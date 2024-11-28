/** \file
 *  brief Example code for Simple Open EtherCAT master
 *
 * Usage: simple_ng IFNAME1
 * IFNAME1 is the NIC interface name, e.g. 'eth0'
 *
 * This is a minimal test.
 */

#include "ethercat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 功能： Fieldbus 是一个封装 EtherCAT 通信相关数据的结构体，用于方便操作。
// 成员变量：
// ecx_contextt context：SOEM 主上下文，用于存储通信会话的所有全局信息。
// char *iface：指定的网络接口名（如 "eth0"）。
// uint8 group：从站组索引，默认为 0。
// int roundtrip_time：存储往返时间（微秒）。
// uint8 map[4096]：I/O 映射的缓冲区，最大 4KB。
// ec_slavet slavelist[EC_MAXSLAVE]：存储检测到的所有从站信息。
// int slavecount：从站数量。
// 其他变量如 ec_groupt grouplist、ec_eringt elist、int64 DCtime 等，用于从站配置和通信管理。
typedef struct
{
    ecx_contextt context;
    char *iface;
    uint8 group;
    int roundtrip_time;

    /* Used by the context */
    uint8 map[4096];
    ecx_portt port;
    ec_slavet slavelist[EC_MAXSLAVE];
    int slavecount;
    ec_groupt grouplist[EC_MAXGROUP];
    uint8 esibuf[EC_MAXEEPBUF];
    uint32 esimap[EC_MAXEEPBITMAP];
    ec_eringt elist;
    ec_idxstackT idxstack;
    boolean ecaterror;
    int64 DCtime;
    ec_SMcommtypet SMcommtype[EC_MAX_MAPT];
    ec_PDOassignt PDOassign[EC_MAX_MAPT];
    ec_PDOdesct PDOdesc[EC_MAX_MAPT];
    ec_eepromSMt eepSM;
    ec_eepromFMMUt eepFMMU;
} Fieldbus;

// 功能： 初始化 Fieldbus 结构体，清零数据并设置上下文指针。
// 关键逻辑：
// 使用 memset 将结构体初始化为 0。
// 将 iface（接口名）赋值给 fieldbus->iface。
// 将上下文指针绑定到 Fieldbus 结构体的成员变量上，确保在整个程序中共享状态信息。
// 其他成员（如 grouplist 和 slavelist）初始化为 Fieldbus 的内存区域。
static void
fieldbus_initialize(Fieldbus *fieldbus, char *iface)
{
    ecx_contextt *context;

    /* Let's start by 0-filling `fieldbus` to avoid surprises */

    memset(fieldbus, 0, sizeof(*fieldbus));

    fieldbus->iface = iface;
    fieldbus->group = 0;
    fieldbus->roundtrip_time = 0;
    fieldbus->ecaterror = FALSE;

    /* Initialize the ecx_contextt data structure */

    context = &fieldbus->context;
    context->port = &fieldbus->port;
    context->slavelist = fieldbus->slavelist;
    context->slavecount = &fieldbus->slavecount;
    context->maxslave = EC_MAXSLAVE;
    context->grouplist = fieldbus->grouplist;
    context->maxgroup = EC_MAXGROUP;
    context->esibuf = fieldbus->esibuf;
    context->esimap = fieldbus->esimap;
    context->esislave = 0;
    context->elist = &fieldbus->elist;
    context->idxstack = &fieldbus->idxstack;
    context->ecaterror = &fieldbus->ecaterror;
    context->DCtime = &fieldbus->DCtime;
    context->SMcommtype = fieldbus->SMcommtype;
    context->PDOassign = fieldbus->PDOassign;
    context->PDOdesc = fieldbus->PDOdesc;
    context->eepSM = &fieldbus->eepSM;
    context->eepFMMU = &fieldbus->eepFMMU;
    context->FOEhook = NULL;
    context->EOEhook = NULL;
    context->manualstatechange = 0;
}

// 功能： 执行一次数据交换，包括发送和接收过程数据。
// 关键逻辑：
// osal_current_time：
// 获取当前时间，用于计算往返时间。
// ecx_send_processdata 和 ecx_receive_processdata：
// 分别发送和接收过程数据。
// 往返时间计算：
// 使用起始时间和结束时间的差值计算整个数据交换的耗时。
// 返回值：
// 返回工作计数器（WKC），用于验证过程数据的正确性。
static int
fieldbus_roundtrip(Fieldbus *fieldbus)
{
    ecx_contextt *context;
    ec_timet start, end, diff;
    int wkc;

    context = &fieldbus->context;

    start = osal_current_time();
    ecx_send_processdata(context);
    wkc = ecx_receive_processdata(context, EC_TIMEOUTRET);
    end = osal_current_time();
    osal_time_diff(&start, &end, &diff);
    fieldbus->roundtrip_time = diff.sec * 1000000 + diff.usec;

    return wkc;
}

// 功能： 启动 EtherCAT 通信，并确保所有从站进入操作模式（Operational）。
// 关键逻辑：
// ecx_init：
// 初始化SOEM库，绑定网络接口。
// 如果初始化失败，说明网络接口不可用。
// ecx_config_init：
// 扫描 EtherCAT 总线，找到所有从站。
// 如果返回值为 0，说明没有找到从站。
// ecx_config_map_group：
// 将从站的 I/O 过程数据映射到本地缓冲区。
// ecx_configdc：
// 配置分布式时钟（Distributed Clocks）。
// 状态检查和切换：
// 循环检查从站是否进入操作模式（EC_STATE_OPERATIONAL）。
// 如果不成功，尝试发送过程数据和再次检查状态。
static boolean
fieldbus_start(Fieldbus *fieldbus)
{
    ecx_contextt *context;
    ec_groupt *grp;
    ec_slavet *slave;
    int i;

    context = &fieldbus->context;
    grp = fieldbus->grouplist + fieldbus->group;

    printf("Initializing SOEM on '%s'... ", fieldbus->iface);
    if (!ecx_init(context, fieldbus->iface))
    {
        printf("no socket connection\n");
        return FALSE;
    }
    printf("done\n");

    printf("Finding autoconfig slaves... ");
    if (ecx_config_init(context, FALSE) <= 0)
    {
        printf("no slaves found\n");
        return FALSE;
    }
    printf("%d slaves found\n", fieldbus->slavecount);

    printf("Sequential mapping of I/O... ");
    ecx_config_map_group(context, fieldbus->map, fieldbus->group);
    printf("mapped %dO+%dI bytes from %d segments",
           grp->Obytes, grp->Ibytes, grp->nsegments);
    if (grp->nsegments > 1)
    {
        /* Show how slaves are distrubuted */
        for (i = 0; i < grp->nsegments; ++i)
        {
            printf("%s%d", i == 0 ? " (" : "+", grp->IOsegment[i]);
        }
        printf(" slaves)");
    }
    printf("\n");

    printf("Configuring distributed clock... ");
    ecx_configdc(context);
    printf("done\n");

    printf("Waiting for all slaves in safe operational... ");
    ecx_statecheck(context, 0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);
    printf("done\n");

    printf("Send a roundtrip to make outputs in slaves happy... ");
    fieldbus_roundtrip(fieldbus);
    printf("done\n");

    printf("Setting operational state..");
    /* Act on slave 0 (a virtual slave used for broadcasting) */
    slave = fieldbus->slavelist;
    slave->state = EC_STATE_OPERATIONAL;
    ecx_writestate(context, 0);
    /* Poll the result ten times before giving up */
    for (i = 0; i < 10; ++i)
    {
        printf(".");
        fieldbus_roundtrip(fieldbus);
        ecx_statecheck(context, 0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE / 10);
        if (slave->state == EC_STATE_OPERATIONAL)
        {
            printf(" all slaves are now operational\n");
            return TRUE;
        }
    }

    printf(" failed,");
    ecx_readstate(context);
    for (i = 1; i <= fieldbus->slavecount; ++i)
    {
        slave = fieldbus->slavelist + i;
        if (slave->state != EC_STATE_OPERATIONAL)
        {
            printf(" slave %d is 0x%04X (AL-status=0x%04X %s)",
                   i, slave->state, slave->ALstatuscode,
                   ec_ALstatuscode2string(slave->ALstatuscode));
        }
    }
    printf("\n");

    return FALSE;
}

//停止函数
static void
fieldbus_stop(Fieldbus *fieldbus)
{
    ecx_contextt *context;
    ec_slavet *slave;

    context = &fieldbus->context;
    /* Act on slave 0 (a virtual slave used for broadcasting) */
    slave = fieldbus->slavelist;

    printf("Requesting init state on all slaves... ");
    slave->state = EC_STATE_INIT;
    ecx_writestate(context, 0);
    printf("done\n");

    printf("Close socket... ");
    ecx_close(context);
    printf("done\n");
}

/*
功能：
fieldbus_dump 函数用于执行一次 EtherCAT 数据交换（或称“轮询”），并打印输出/输入的过程数据、往返时间（roundtrip_time）、工作计数器（WKC）以及 EtherCAT 的分布式时钟（DCtime）。
该函数用于诊断或监测 EtherCAT 网络状态，确保过程数据的有效性和 WKC 的正确性。
*/
static boolean
fieldbus_dump(Fieldbus *fieldbus)
{
    ec_groupt *grp;
    uint32 n;
    int wkc, expected_wkc;

    grp = fieldbus->grouplist + fieldbus->group;

    wkc = fieldbus_roundtrip(fieldbus);
    expected_wkc = grp->outputsWKC * 2 + grp->inputsWKC;
    printf("%6d usec  WKC %d", fieldbus->roundtrip_time, wkc);
    if (wkc < expected_wkc)
    {
        printf(" wrong (expected %d)\n", expected_wkc);
        return FALSE;
    }

    printf("  O:");
    for (n = 0; n < grp->Obytes; ++n)
    {
        printf(" %02X", grp->outputs[n]);
    }
    printf("  I:");
    for (n = 0; n < grp->Ibytes; ++n)
    {
        printf(" %02X", grp->inputs[n]);
    }
    printf("  T: %lld\r", (long long)fieldbus->DCtime);
    return TRUE;
}

/*
功能： 检查从站状态并尝试恢复异常的从站。
关键逻辑：
ecx_readstate：
获取所有从站的当前状态。
状态处理：
如果从站处于错误状态（SAFE_OP + ERROR），尝试通过写入状态进行恢复。
如果从站状态丢失，则调用 ecx_reconfig_slave 尝试重新配置。
*/
static void
fieldbus_check_state(Fieldbus *fieldbus)
{
    ecx_contextt *context;
    ec_groupt *grp;
    ec_slavet *slave;
    int i;

    context = &fieldbus->context;
    grp = context->grouplist + fieldbus->group;
    grp->docheckstate = FALSE;
    ecx_readstate(context);
    for (i = 1; i <= fieldbus->slavecount; ++i)
    {
        slave = context->slavelist + i;
        if (slave->group != fieldbus->group)
        {
            /* This slave is part of another group: do nothing */
        }
        else if (slave->state != EC_STATE_OPERATIONAL)
        {
            grp->docheckstate = TRUE;
            if (slave->state == EC_STATE_SAFE_OP + EC_STATE_ERROR)
            {
                printf("* Slave %d is in SAFE_OP+ERROR, attempting ACK\n", i);
                slave->state = EC_STATE_SAFE_OP + EC_STATE_ACK;
                ecx_writestate(context, i);
            }
            else if (slave->state == EC_STATE_SAFE_OP)
            {
                printf("* Slave %d is in SAFE_OP, change to OPERATIONAL\n", i);
                slave->state = EC_STATE_OPERATIONAL;
                ecx_writestate(context, i);
            }
            else if (slave->state > EC_STATE_NONE)
            {
                if (ecx_reconfig_slave(context, i, EC_TIMEOUTRET))
                {
                    slave->islost = FALSE;
                    printf("* Slave %d reconfigured\n", i);
                }
            }
            else if (!slave->islost)
            {
                ecx_statecheck(context, i, EC_STATE_OPERATIONAL, EC_TIMEOUTRET);
                if (slave->state == EC_STATE_NONE)
                {
                    slave->islost = TRUE;
                    printf("* Slave %d lost\n", i);
                }
            }
        }
        else if (slave->islost)
        {
            if (slave->state != EC_STATE_NONE)
            {
                slave->islost = FALSE;
                printf("* Slave %d found\n", i);
            }
            else if (ecx_recover_slave(context, i, EC_TIMEOUTRET))
            {
                slave->islost = FALSE;
                printf("* Slave %d recovered\n", i);
            }
        }
    }

    if (!grp->docheckstate)
    {
        printf("All slaves resumed OPERATIONAL\n");
    }
}

int main(int argc, char *argv[])
{
    Fieldbus fieldbus;

    if (argc != 2)
    {
        ec_adaptert *adapter = NULL;
        printf("Usage: simple_ng IFNAME1\n"
               "IFNAME1 is the NIC interface name, e.g. 'eth0'\n");

        printf("\nAvailable adapters:\n");
        adapter = ec_find_adapters();
        while (adapter != NULL)
        {
            printf("    - %s  (%s)\n", adapter->name, adapter->desc);
            adapter = adapter->next;
        }
        ec_free_adapters(adapter);
        return 1;
    }

    fieldbus_initialize(&fieldbus, argv[1]);
    if (fieldbus_start(&fieldbus))
    {
        int i, min_time, max_time;
        min_time = max_time = 0;
        for (i = 1; i <= 10; ++i)
        {
            printf("Iteration %4d:", i);
            if (!fieldbus_dump(&fieldbus))
            {
                fieldbus_check_state(&fieldbus);
            }
            else if (i == 1)
            {
                min_time = max_time = fieldbus.roundtrip_time;
            }
            else if (fieldbus.roundtrip_time < min_time)
            {
                min_time = fieldbus.roundtrip_time;
            }
            else if (fieldbus.roundtrip_time > max_time)
            {
                max_time = fieldbus.roundtrip_time;
            }
            osal_usleep(5000);
        }
        printf("\nRoundtrip time (usec): min %d max %d\n", min_time, max_time);
        fieldbus_stop(&fieldbus);
    }

    return 0;
}
