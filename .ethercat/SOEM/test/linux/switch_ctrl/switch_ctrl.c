#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ethercat.h"

#define SWITCH1_PDO_INDEX 0x0004 // RxPDO  Index
#define LED1_PDO_INDEX 0x0000    // RxPDO  Index
#define SWITCH2_PDO_INDEX 0x0006 // RxPDO  Index
#define LED2_PDO_INDEX 0x0002    // RxPDO  Index
//超时时间
#define EC_TIMEOUTMON 500

//输入/输出数据映射
char IOmap[4096];
//Ethercat 通信状态检查的线程句柄
OSAL_THREAD_HANDLE thread1;
//期待的工作计数
int expectedWKC;
//实际工作计数
volatile int wkc;
//控制是否需要换行符输出
boolean needlf;
//标识从站当前是否OP状态
boolean inOP;
//标识当前从站组
uint8 currentgroup = 0;
//标识是否强制对齐
boolean forceByteAlignment = FALSE;

OSAL_THREAD_FUNC ecatcheck(void *ptr)
{
    int slave;
    (void)ptr; /* Not used */

    while (1)
    {
        //检查条件：全线OP&&（有从站未响应||当前从站组需要检查）
        if (inOP && ((wkc < expectedWKC) || ec_group[currentgroup].docheckstate))
        {
            if (needlf)
            {
                needlf = FALSE;
                printf("\n");
            }
            /* one ore more slaves are not responding */
            ec_group[currentgroup].docheckstate = FALSE;
            //读所有从站的状态并且保存到glob的struct ecx_context中
            ec_readstate();
            //遍历从站
            for (slave = 1; slave <= ec_slavecount; slave++)
            {
                //检查条件：检查的从站所属组为当前从站组&&从站的状态非OP
                if ((ec_slave[slave].group == currentgroup) && (ec_slave[slave].state != EC_STATE_OPERATIONAL))
                {
                    ec_group[currentgroup].docheckstate = TRUE;
                    //对于各种状态的反馈
                    if (ec_slave[slave].state == (EC_STATE_SAFE_OP + EC_STATE_ERROR))
                    {
                        printf("ERROR : slave %d is in SAFE_OP + ERROR, attempting ack.\n", slave);
                        ec_slave[slave].state = (EC_STATE_SAFE_OP + EC_STATE_ACK);
                        ec_writestate(slave);
                    }
                    else if (ec_slave[slave].state == EC_STATE_SAFE_OP)
                    {
                        printf("WARNING : slave %d is in SAFE_OP, change to OPERATIONAL.\n", slave);
                        ec_slave[slave].state = EC_STATE_OPERATIONAL;
                        ec_writestate(slave);
                    }
                    else if (ec_slave[slave].state > EC_STATE_NONE)
                    {
                        if (ec_reconfig_slave(slave, EC_TIMEOUTMON))
                        {
                            ec_slave[slave].islost = FALSE;
                            printf("MESSAGE : slave %d reconfigured\n", slave);
                        }
                    }
                    else if (!ec_slave[slave].islost)
                    {
                        /* re-check state */
                        ec_statecheck(slave, EC_STATE_OPERATIONAL, EC_TIMEOUTRET);
                        if (ec_slave[slave].state == EC_STATE_NONE)
                        {
                            ec_slave[slave].islost = TRUE;
                            printf("ERROR : slave %d lost\n", slave);
                        }
                    }
                }
                if (ec_slave[slave].islost)
                {
                    if (ec_slave[slave].state == EC_STATE_NONE)
                    {
                        if (ec_recover_slave(slave, EC_TIMEOUTMON))
                        {
                            ec_slave[slave].islost = FALSE;
                            printf("MESSAGE : slave %d recovered\n", slave);
                        }
                    }
                    else
                    {
                        ec_slave[slave].islost = FALSE;
                        printf("MESSAGE : slave %d found\n", slave);
                    }
                }
            }
            if (!ec_group[currentgroup].docheckstate)
                printf("OK : all slaves resumed OPERATIONAL.\n");
        }
        osal_usleep(10000);
    }
}

int main()
{
    int8_t switch1 = 1;
    int8_t switch2 = 1;
    int8_t switch3 = 1;
    int8_t switch11 = 1;
    int8_t switch22 = 1;
    int8_t switch33 = 1;
    uint8_t led_state1 = 0;
    uint8_t led_state2 = 0;
    uint8_t switch_state1 = 0;
    uint8_t switch_state2 = 0;

    osal_thread_create(&thread1, 128000, &ecatcheck, NULL);

    if (ec_init("eth1"))
    {
        printf("EtherCAT 初始化成功\n");

        if (ec_config_init(FALSE) > 0)
        {
            printf("%d 个从站被发现\n", ec_slavecount);

            // 自动配置PDO并启用dc
            ec_config_map(&IOmap);
            ec_configdc();

            ec_slave[0].state = EC_STATE_OPERATIONAL;
            ec_writestate(0);

            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);

            // 等待所有总站 OPERATIONAL
            if (ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE) == EC_STATE_OPERATIONAL)
            {
                printf("所有从站 OPERATIONAL \n");

                // 循环控制LED
                while (1)
                {

                    ec_slave[1].outputs[LED1_PDO_INDEX] = led_state1;
                    ec_slave[1].outputs[LED2_PDO_INDEX] = led_state2;
                    if (ec_send_processdata() < 0)
                    {
                        fprintf(stderr, "ec_send_processdata() failed\n");
                        exit(1);
                    }
                    ec_receive_processdata(EC_TIMEOUTRET);
                    switch_state1 = IOmap[SWITCH1_PDO_INDEX];
                    switch_state2 = IOmap[SWITCH2_PDO_INDEX];
                    switch1 = !((switch_state1 >> 0) & 0x01);
                    switch2 = !((switch_state1 >> 1) & 0x01);
                    switch3 = !((switch_state1 >> 2) & 0x01);
                    switch11 = !((switch_state2 >> 0) & 0x01);
                    switch22 = !((switch_state2 >> 1) & 0x01);
                    switch33 = !((switch_state2 >> 2) & 0x01);
                    printf("ledstate1：%d\n", led_state1);
                    printf("switch_state1：%d\n", switch_state1);
                    printf("ledstate2：%d\n", led_state2);
                    printf("switch_state2：%d\n", switch_state2);
                    for (int i = 0; i < 8; i++)
                    {
                        printf("%02X", IOmap[i]);
                    }
                    printf("\n");
                    if (switch1)
                    {
                        printf("switch1:on\n");
                        led_state2 |= 0x01;
                    }
                    else
                    {
                        led_state2 &= ~(0x01);
                    }
                    if (switch2)
                    {
                        printf("switch2:on\n");
                        led_state2 |= 0x02;
                    }
                    else
                    {
                        led_state2 &= ~(0x02);
                    }
                    if (switch3)
                    {
                        printf("switch3:on\n");
                    }
                    if (switch11)
                    {
                        printf("switch11:on\n");
                        led_state1 |= 0x01;
                    }
                    else
                    {
                        led_state1 &= ~(0x01);
                    }
                    if (switch22)
                    {
                        printf("switch22:on\n");
                        led_state1 |= 0x02;
                    }
                    else
                    {
                        led_state1 &= ~(0x02);
                    }
                    if (switch33)
                    {
                        printf("switch33:on\n");
                    }

                    osal_usleep(50000);
                }
                // 关闭EtherCAT
                ec_close();
            }
            else
            {
                printf("从站未进入 OPERATIONAL \n");
            }
        }
        else
        {
            printf("未找到从站\n");
        }
    }
    else
    {
        printf("网卡初始化失败，试使用root用户\n");
    }
    return 0;
}
