/** \file
 * \brief Example code for Simple Open EtherCAT master
 *
 * Usage : slaveinfo [ifname] [-sdo] [-map]
 * Ifname is NIC interface, f.e. eth0.
 * Optional -sdo to display CoE object dictionary.
 * Optional -map to display slave PDO mapping
 *
 * This shows the configured slave data.
 *
 * (c)Arthur Ketels 2010 - 2011
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "ethercat.h"

char IOmap[4096];
ec_ODlistt ODlist;
ec_OElistt OElist;
boolean printSDO = FALSE;
boolean printMAP = FALSE;
char usdo[128];

#define OTYPE_VAR 0x0007
#define OTYPE_ARRAY 0x0008
#define OTYPE_RECORD 0x0009

#define ATYPE_Rpre 0x01
#define ATYPE_Rsafe 0x02
#define ATYPE_Rop 0x04
#define ATYPE_Wpre 0x08
#define ATYPE_Wsafe 0x10
#define ATYPE_Wop 0x20

//判断ethercat的datatype
char *dtype2string(uint16 dtype, uint16 bitlen)
{
    static char str[32] = {0};

    switch (dtype)
    {
    case ECT_BOOLEAN:
        sprintf(str, "BOOLEAN");
        break;
    case ECT_INTEGER8:
        sprintf(str, "INTEGER8");
        break;
    case ECT_INTEGER16:
        sprintf(str, "INTEGER16");
        break;
    case ECT_INTEGER32:
        sprintf(str, "INTEGER32");
        break;
    case ECT_INTEGER24:
        sprintf(str, "INTEGER24");
        break;
    case ECT_INTEGER64:
        sprintf(str, "INTEGER64");
        break;
    case ECT_UNSIGNED8:
        sprintf(str, "UNSIGNED8");
        break;
    case ECT_UNSIGNED16:
        sprintf(str, "UNSIGNED16");
        break;
    case ECT_UNSIGNED32:
        sprintf(str, "UNSIGNED32");
        break;
    case ECT_UNSIGNED24:
        sprintf(str, "UNSIGNED24");
        break;
    case ECT_UNSIGNED64:
        sprintf(str, "UNSIGNED64");
        break;
    case ECT_REAL32:
        sprintf(str, "REAL32");
        break;
    case ECT_REAL64:
        sprintf(str, "REAL64");
        break;
    case ECT_BIT1:
        sprintf(str, "BIT1");
        break;
    case ECT_BIT2:
        sprintf(str, "BIT2");
        break;
    case ECT_BIT3:
        sprintf(str, "BIT3");
        break;
    case ECT_BIT4:
        sprintf(str, "BIT4");
        break;
    case ECT_BIT5:
        sprintf(str, "BIT5");
        break;
    case ECT_BIT6:
        sprintf(str, "BIT6");
        break;
    case ECT_BIT7:
        sprintf(str, "BIT7");
        break;
    case ECT_BIT8:
        sprintf(str, "BIT8");
        break;
    case ECT_VISIBLE_STRING:
        sprintf(str, "VISIBLE_STR(%d)", bitlen);
        break;
    case ECT_OCTET_STRING:
        sprintf(str, "OCTET_STR(%d)", bitlen);
        break;
    default:
        sprintf(str, "dt:0x%4.4X (%d)", dtype, bitlen);
    }
    return str;
}

//判断otype的datatype
char *otype2string(uint16 otype)
{
    static char str[32] = {0};

    switch (otype)
    {
    case OTYPE_VAR:
        sprintf(str, "VAR");
        break;
    case OTYPE_ARRAY:
        sprintf(str, "ARRAY");
        break;
    case OTYPE_RECORD:
        sprintf(str, "RECORD");
        break;
    default:
        sprintf(str, "ot:0x%4.4X", otype);
    }
    return str;
}

//判断状态是什么
char *access2string(uint16 access)
{
    static char str[32] = {0};

    sprintf(str, "%s%s%s%s%s%s",
            ((access & ATYPE_Rpre) != 0 ? "R" : "_"),
            ((access & ATYPE_Wpre) != 0 ? "W" : "_"),
            ((access & ATYPE_Rsafe) != 0 ? "R" : "_"),
            ((access & ATYPE_Wsafe) != 0 ? "W" : "_"),
            ((access & ATYPE_Rop) != 0 ? "R" : "_"),
            ((access & ATYPE_Wop) != 0 ? "W" : "_"));
    return str;
}

char *SDO2string(uint16 slave, uint16 index, uint8 subidx, uint16 dtype)
{
    int l = sizeof(usdo) - 1, i;
    uint8 *u8;
    int8 *i8;
    uint16 *u16;
    int16 *i16;
    uint32 *u32;
    int32 *i32;
    uint64 *u64;
    int64 *i64;
    float *sr;
    double *dr;
    char es[32];

    memset(&usdo, 0, 128);
    ec_SDOread(slave, index, subidx, FALSE, &l, &usdo, EC_TIMEOUTRXM);
    if (EcatError)
    {
        return ec_elist2string();
    }
    else
    {
        static char str[64] = {0};
        switch (dtype)
        {
        case ECT_BOOLEAN:
            u8 = (uint8 *)&usdo[0];
            if (*u8)
                sprintf(str, "TRUE");
            else
                sprintf(str, "FALSE");
            break;
        case ECT_INTEGER8:
            i8 = (int8 *)&usdo[0];
            sprintf(str, "0x%2.2x / %d", *i8, *i8);
            break;
        case ECT_INTEGER16:
            i16 = (int16 *)&usdo[0];
            sprintf(str, "0x%4.4x / %d", *i16, *i16);
            break;
        case ECT_INTEGER32:
        case ECT_INTEGER24:
            i32 = (int32 *)&usdo[0];
            sprintf(str, "0x%8.8x / %d", *i32, *i32);
            break;
        case ECT_INTEGER64:
            i64 = (int64 *)&usdo[0];
            sprintf(str, "0x%16.16" PRIx64 " / %" PRId64, *i64, *i64);
            break;
        case ECT_UNSIGNED8:
            u8 = (uint8 *)&usdo[0];
            sprintf(str, "0x%2.2x / %u", *u8, *u8);
            break;
        case ECT_UNSIGNED16:
            u16 = (uint16 *)&usdo[0];
            sprintf(str, "0x%4.4x / %u", *u16, *u16);
            break;
        case ECT_UNSIGNED32:
        case ECT_UNSIGNED24:
            u32 = (uint32 *)&usdo[0];
            sprintf(str, "0x%8.8x / %u", *u32, *u32);
            break;
        case ECT_UNSIGNED64:
            u64 = (uint64 *)&usdo[0];
            sprintf(str, "0x%16.16" PRIx64 " / %" PRIu64, *u64, *u64);
            break;
        case ECT_REAL32:
            sr = (float *)&usdo[0];
            sprintf(str, "%f", *sr);
            break;
        case ECT_REAL64:
            dr = (double *)&usdo[0];
            sprintf(str, "%f", *dr);
            break;
        case ECT_BIT1:
        case ECT_BIT2:
        case ECT_BIT3:
        case ECT_BIT4:
        case ECT_BIT5:
        case ECT_BIT6:
        case ECT_BIT7:
        case ECT_BIT8:
            u8 = (uint8 *)&usdo[0];
            sprintf(str, "0x%x / %u", *u8, *u8);
            break;
        case ECT_VISIBLE_STRING:
            strcpy(str, "\"");
            strcat(str, usdo);
            strcat(str, "\"");
            break;
        case ECT_OCTET_STRING:
            str[0] = 0x00;
            for (i = 0; i < l; i++)
            {
                sprintf(es, "0x%2.2x ", usdo[i]);
                strcat(str, es);
            }
            break;
        default:
            sprintf(str, "Unknown type");
        }
        return str;
    }
}

/** Read PDO assign structure */
/** 读取PDO在从站中的分配结构
 *
 * @param[in]  slave      = 从站号
 * @param[in]  PDOassign  = PDO分配索引
 * @param[in]  mapoffset  = 映射
 * @param[in]  bitoffset  = 位偏移
 * @return Offset to data in rx frame, usefull to retrieve data after RX.
 */
int si_PDOassign(uint16 slave, uint16 PDOassign, int mapoffset, int bitoffset)
{
    uint16 idxloop, nidx, subidxloop, rdat, idx, subidx;
    uint8 subcnt;
    int wkc, bsize = 0, rdl;
    int32 rdat2;
    uint8 bitlen, obj_subidx;
    uint16 obj_idx;
    int abs_offset, abs_bit;

    rdl = sizeof(rdat);
    rdat = 0;
    /* read PDO assign subindex 0 ( = number of PDO's) */
    /* read PDO assign subindex 0 ( = number of PDO's) */
    // 读取PDO分配的子索引0位置的数据（即PDO的数量）
    // 推测返回值有三个wkc（工作计数），rdl（数据长度），rdat（数据，在这里为PDO的数量）
    wkc = ec_SDOread(slave, PDOassign, 0x00, FALSE, &rdl, &rdat, EC_TIMEOUTRXM);
    rdat = etohs(rdat);
    /* positive result from slave ? */
    //判断工作计数和PDO数据长度是否大于零
    if ((wkc > 0) && (rdat > 0))
    {
        /* number of available sub indexes */
        //记录可用子索引（PDO）的数量
        nidx = rdat;
        bsize = 0;
        /* 依次读取所有PDO */
        for (idxloop = 1; idxloop <= nidx; idxloop++)
        {
            rdl = sizeof(rdat);
            rdat = 0;
            /* 读取PDO */
            wkc = ec_SDOread(slave, PDOassign, (uint8)idxloop, FALSE, &rdl, &rdat, EC_TIMEOUTRXM);
            /* 返回值是PDO的索引值 */
            idx = etohs(rdat);
            //idx > 0说明PDO索引有效
            if (idx > 0)
            {
                rdl = sizeof(subcnt);
                subcnt = 0;
                /* 读取PDO子索引的数量*/
                wkc = ec_SDOread(slave, idx, 0x00, FALSE, &rdl, &subcnt, EC_TIMEOUTRXM);
                subidx = subcnt;
                /* 遍历每个子索引 */
                for (subidxloop = 1; subidxloop <= subidx; subidxloop++)
                {
                    rdl = sizeof(rdat2);
                    rdat2 = 0;
                    /* 读取映射到PDO的SDO */
                    wkc = ec_SDOread(slave, idx, (uint8)subidxloop, FALSE, &rdl, &rdat2, EC_TIMEOUTRXM);
                    rdat2 = etohl(rdat2);
                    /* 提取SDO的位长度 */
                    bitlen = LO_BYTE(rdat2);
                    bsize += bitlen;

                    //计算各种数据的值
                    obj_idx = (uint16)(rdat2 >> 16);                 //rdat2的高16位，表示对象索引
                    obj_subidx = (uint8)((rdat2 >> 8) & 0x000000ff); //rdat2的8-15位
                    abs_offset = mapoffset + (bitoffset / 8);        //根据参数算出字节偏移量
                    abs_bit = bitoffset % 8;                         //算出位偏移量
                    ODlist.Slave = slave;                            //
                    ODlist.Index[0] = obj_idx;
                    OElist.Entries = 0;
                    wkc = 0;
                    /* 如果不是填充对象(0x0000:0x00) ，从字典中读取对象条目*/
                    if (obj_idx || obj_subidx)
                        wkc = ec_readOEsingle(0, obj_subidx, &ODlist, &OElist);
                    //打印偏移和对象信息
                    printf("  [0x%4.4X.%1d] 0x%4.4X:0x%2.2X 0x%2.2X", abs_offset, abs_bit, obj_idx, obj_subidx, bitlen);

                    if ((wkc > 0) && OElist.Entries)
                    {
                        //获取数据类型并打印
                        printf(" %-12s %s\n", dtype2string(OElist.DataType[obj_subidx], bitlen), OElist.Name[obj_subidx]);
                    }
                    else
                        printf("\n");
                    //累计上当前SDO的位长度
                    bitoffset += bitlen;
                };
            };
        };
    };
    /* return total found bitlength (PDO) */
    return bsize;
}

/** Read PDO assign structure */
/** 读取PDO在从站中的分配结构
 *
 * @param[in]  slave      = 从站号
 * @return Offset to data in rx frame, usefull to retrieve data after RX.
 */
int si_map_sdo(int slave)
{
    int wkc, rdl;
    int retVal = 0;
    uint8 nSM, iSM, tSM;
    int Tsize, outputs_bo, inputs_bo;
    uint8 SMt_bug_add;

    printf("PDO mapping according to CoE :\n");
    SMt_bug_add = 0;
    outputs_bo = 0;
    inputs_bo = 0;
    rdl = sizeof(nSM);
    nSM = 0;
    /* read SyncManager Communication Type object count */
    wkc = ec_SDOread(slave, ECT_SDO_SMCOMMTYPE, 0x00, FALSE, &rdl, &nSM, EC_TIMEOUTRXM);
    /* positive result from slave ? */
    if ((wkc > 0) && (nSM > 2))
    {
        /* make nSM equal to number of defined SM */
        nSM--;
        /* limit to maximum number of SM defined, if true the slave can't be configured */
        if (nSM > EC_MAXSM)
            nSM = EC_MAXSM;
        /* iterate for every SM type defined */
        for (iSM = 2; iSM <= nSM; iSM++)
        {
            rdl = sizeof(tSM);
            tSM = 0;
            /* read SyncManager Communication Type */
            wkc = ec_SDOread(slave, ECT_SDO_SMCOMMTYPE, iSM + 1, FALSE, &rdl, &tSM, EC_TIMEOUTRXM);
            if (wkc > 0)
            {
                if ((iSM == 2) && (tSM == 2)) // SM2 has type 2 == mailbox out, this is a bug in the slave!
                {
                    SMt_bug_add = 1; // try to correct, this works if the types are 0 1 2 3 and should be 1 2 3 4
                    printf("Activated SM type workaround, possible incorrect mapping.\n");
                }
                if (tSM)
                    tSM += SMt_bug_add; // only add if SMt > 0

                if (tSM == 3) // outputs
                {
                    /* read the assign RXPDO */
                    printf("  SM%1d outputs\n     addr b   index: sub bitl data_type    name\n", iSM);
                    Tsize = si_PDOassign(slave, ECT_SDO_PDOASSIGN + iSM, (int)(ec_slave[slave].outputs - (uint8 *)&IOmap[0]), outputs_bo);
                    outputs_bo += Tsize;
                }
                if (tSM == 4) // inputs
                {
                    /* read the assign TXPDO */
                    printf("  SM%1d inputs\n     addr b   index: sub bitl data_type    name\n", iSM);
                    Tsize = si_PDOassign(slave, ECT_SDO_PDOASSIGN + iSM, (int)(ec_slave[slave].inputs - (uint8 *)&IOmap[0]), inputs_bo);
                    inputs_bo += Tsize;
                }
            }
        }
    }

    /* found some I/O bits ? */
    if ((outputs_bo > 0) || (inputs_bo > 0))
        retVal = 1;
    return retVal;
}

int si_siiPDO(uint16 slave, uint8 t, int mapoffset, int bitoffset)
{
    uint16 a, w, c, e, er;
    uint8 eectl;
    uint16 obj_idx;
    uint8 obj_subidx;
    uint8 obj_name;
    uint8 obj_datatype;
    uint8 bitlen;
    int totalsize;
    ec_eepromPDOt eepPDO;
    ec_eepromPDOt *PDO;
    int abs_offset, abs_bit;
    char str_name[EC_MAXNAME + 1];

    eectl = ec_slave[slave].eep_pdi;

    totalsize = 0;
    PDO = &eepPDO;
    PDO->nPDO = 0;
    PDO->Length = 0;
    PDO->Index[1] = 0;
    for (c = 0; c < EC_MAXSM; c++)
        PDO->SMbitsize[c] = 0;
    if (t > 1)
        t = 1;
    PDO->Startpos = ec_siifind(slave, ECT_SII_PDO + t);
    if (PDO->Startpos > 0)
    {
        a = PDO->Startpos;
        w = ec_siigetbyte(slave, a++);
        w += (ec_siigetbyte(slave, a++) << 8);
        PDO->Length = w;
        c = 1;
        /* traverse through all PDOs */
        do
        {
            PDO->nPDO++;
            PDO->Index[PDO->nPDO] = ec_siigetbyte(slave, a++);
            PDO->Index[PDO->nPDO] += (ec_siigetbyte(slave, a++) << 8);
            PDO->BitSize[PDO->nPDO] = 0;
            c++;
            /* number of entries in PDO */
            e = ec_siigetbyte(slave, a++);
            PDO->SyncM[PDO->nPDO] = ec_siigetbyte(slave, a++);
            a++;
            obj_name = ec_siigetbyte(slave, a++);
            a += 2;
            c += 2;
            if (PDO->SyncM[PDO->nPDO] < EC_MAXSM) /* active and in range SM? */
            {
                str_name[0] = 0;
                if (obj_name)
                    ec_siistring(str_name, slave, obj_name);
                if (t)
                    printf("  SM%1d RXPDO 0x%4.4X %s\n", PDO->SyncM[PDO->nPDO], PDO->Index[PDO->nPDO], str_name);
                else
                    printf("  SM%1d TXPDO 0x%4.4X %s\n", PDO->SyncM[PDO->nPDO], PDO->Index[PDO->nPDO], str_name);
                printf("     addr b   index: sub bitl data_type    name\n");
                /* read all entries defined in PDO */
                for (er = 1; er <= e; er++)
                {
                    c += 4;
                    obj_idx = ec_siigetbyte(slave, a++);
                    obj_idx += (ec_siigetbyte(slave, a++) << 8);
                    obj_subidx = ec_siigetbyte(slave, a++);
                    obj_name = ec_siigetbyte(slave, a++);
                    obj_datatype = ec_siigetbyte(slave, a++);
                    bitlen = ec_siigetbyte(slave, a++);
                    abs_offset = mapoffset + (bitoffset / 8);
                    abs_bit = bitoffset % 8;

                    PDO->BitSize[PDO->nPDO] += bitlen;
                    a += 2;

                    /* skip entry if filler (0x0000:0x00) */
                    if (obj_idx || obj_subidx)
                    {
                        str_name[0] = 0;
                        if (obj_name)
                            ec_siistring(str_name, slave, obj_name);

                        printf("  [0x%4.4X.%1d] 0x%4.4X:0x%2.2X 0x%2.2X", abs_offset, abs_bit, obj_idx, obj_subidx, bitlen);
                        printf(" %-12s %s\n", dtype2string(obj_datatype, bitlen), str_name);
                    }
                    bitoffset += bitlen;
                    totalsize += bitlen;
                }
                PDO->SMbitsize[PDO->SyncM[PDO->nPDO]] += PDO->BitSize[PDO->nPDO];
                c++;
            }
            else /* PDO deactivated because SM is 0xff or > EC_MAXSM */
            {
                c += 4 * e;
                a += 8 * e;
                c++;
            }
            if (PDO->nPDO >= (EC_MAXEEPDO - 1))
                c = PDO->Length; /* limit number of PDO entries in buffer */
        } while (c < PDO->Length);
    }
    if (eectl)
        ec_eeprom2pdi(slave); /* if eeprom control was previously pdi then restore */
    return totalsize;
}

int si_map_sii(int slave)
{
    int retVal = 0;
    int Tsize, outputs_bo, inputs_bo;

    printf("PDO mapping according to SII :\n");

    outputs_bo = 0;
    inputs_bo = 0;
    /* read the assign RXPDOs */
    Tsize = si_siiPDO(slave, 1, (int)(ec_slave[slave].outputs - (uint8 *)&IOmap), outputs_bo);
    outputs_bo += Tsize;
    /* read the assign TXPDOs */
    Tsize = si_siiPDO(slave, 0, (int)(ec_slave[slave].inputs - (uint8 *)&IOmap), inputs_bo);
    inputs_bo += Tsize;
    /* found some I/O bits ? */
    if ((outputs_bo > 0) || (inputs_bo > 0))
        retVal = 1;
    return retVal;
}

void si_sdo(int cnt)
{
    int i, j;

    ODlist.Entries = 0;
    memset(&ODlist, 0, sizeof(ODlist));
    if (ec_readODlist(cnt, &ODlist))
    {
        printf(" CoE Object Description found, %d entries.\n", ODlist.Entries);
        for (i = 0; i < ODlist.Entries; i++)
        {
            uint8_t max_sub;
            char name[128] = {0};

            ec_readODdescription(i, &ODlist);
            while (EcatError)
                printf(" - %s\n", ec_elist2string());
            snprintf(name, sizeof(name) - 1, "\"%s\"", ODlist.Name[i]);
            if (ODlist.ObjectCode[i] == OTYPE_VAR)
            {
                printf("0x%04x      %-40s      [%s]\n", ODlist.Index[i], name,
                       otype2string(ODlist.ObjectCode[i]));
            }
            else
            {
                printf("0x%04x      %-40s      [%s  maxsub(0x%02x / %d)]\n",
                       ODlist.Index[i], name, otype2string(ODlist.ObjectCode[i]),
                       ODlist.MaxSub[i], ODlist.MaxSub[i]);
            }
            memset(&OElist, 0, sizeof(OElist));
            ec_readOE(i, &ODlist, &OElist);
            while (EcatError)
                printf("- %s\n", ec_elist2string());

            if (ODlist.ObjectCode[i] != OTYPE_VAR)
            {
                int l = sizeof(max_sub);
                ec_SDOread(cnt, ODlist.Index[i], 0, FALSE, &l, &max_sub, EC_TIMEOUTRXM);
            }
            else
            {
                max_sub = ODlist.MaxSub[i];
            }

            for (j = 0; j < max_sub + 1; j++)
            {
                if ((OElist.DataType[j] > 0) && (OElist.BitLength[j] > 0))
                {
                    snprintf(name, sizeof(name) - 1, "\"%s\"", OElist.Name[j]);
                    printf("    0x%02x      %-40s      [%-16s %6s]      ", j, name,
                           dtype2string(OElist.DataType[j], OElist.BitLength[j]),
                           access2string(OElist.ObjAccess[j]));
                    if ((OElist.ObjAccess[j] & 0x0007))
                    {
                        printf("%s", SDO2string(cnt, ODlist.Index[i], j, OElist.DataType[j]));
                    }
                    printf("\n");
                }
            }
        }
    }
    else
    {
        while (EcatError)
            printf("%s", ec_elist2string());
    }
}

/** 读取从站详细信息
 *
 * @param[in]  ifname     = 借口名 
 */
void slaveinfo(char *ifname)
{
    int cnt, i, j;
    uint16 ssigen;
    int expectedWKC;

    printf("Starting slaveinfo\n");

    /* initialise SOEM, bind socket to ifname */
    if (ec_init(ifname))
    {
        printf("ec_init on %s succeeded.\n", ifname);
        /* 自动配置从站并返回从站数 */
        if (ec_config(FALSE, &IOmap) > 0)
        {
            //配置时钟
            ec_configdc();
            while (EcatError)
            {
                printf("%s", ec_elist2string());
            }
            //ec_slavecount 全局变量，当使用config函数后就会自动赋值（很重要）
            printf("%d slaves found and configured.\n", ec_slavecount);
            //计算expectedWKC的公式
            expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
            printf("Calculated workcounter %d\n", expectedWKC);
            /* 阻塞切换所有从站的状态 */
            ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 3);
            //若有从站未SAFE_OP，则打印其信息
            if (ec_slave[0].state != EC_STATE_SAFE_OP)
            {
                printf("Not all slaves reached safe operational state.\n");
                ec_readstate();
                for (i = 1; i <= ec_slavecount; i++)
                {
                    if (ec_slave[i].state != EC_STATE_SAFE_OP)
                    {
                        printf("Slave %d State=%2x StatusCode=%4x : %s\n",
                               i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
                    }
                }
            }

            ec_readstate();
            for (cnt = 1; cnt <= ec_slavecount; cnt++)
            {

                printf("\nSlave:%d\n Name:%s\n Output size: %dbits\n Input size: %dbits\n State: %d\n Delay: %d[ns]\n Has DC: %d\n",
                       cnt, ec_slave[cnt].name, ec_slave[cnt].Obits, ec_slave[cnt].Ibits,
                       ec_slave[cnt].state, ec_slave[cnt].pdelay, ec_slave[cnt].hasdc);
                if (ec_slave[cnt].hasdc)
                    printf(" DCParentport:%d\n", ec_slave[cnt].parentport);
                //输出从站的活动端口
                printf(" Activeports:%d.%d.%d.%d\n", (ec_slave[cnt].activeports & 0x01) > 0,
                       (ec_slave[cnt].activeports & 0x02) > 0,
                       (ec_slave[cnt].activeports & 0x04) > 0,
                       (ec_slave[cnt].activeports & 0x08) > 0);

                //遍历从站未使用的FMMU（现场总线内存管理单元）
                for (j = 0; j < ec_slave[cnt].FMMUunused; j++)
                {
                    //打印详细信息：逻辑地址起始地址，长度，～起始位，～结束位，物理起始地址，～起始位，类型，是否激活
                    printf(" FMMU%1d Ls:%8.8x Ll:%4d Lsb:%d Leb:%d Ps:%4.4x Psb:%d Ty:%2.2x Act:%2.2x\n", j,
                           etohl(ec_slave[cnt].FMMU[j].LogStart), etohs(ec_slave[cnt].FMMU[j].LogLength), ec_slave[cnt].FMMU[j].LogStartbit,
                           ec_slave[cnt].FMMU[j].LogEndbit, etohs(ec_slave[cnt].FMMU[j].PhysStart), ec_slave[cnt].FMMU[j].PhysStartBit,
                           ec_slave[cnt].FMMU[j].FMMUtype, ec_slave[cnt].FMMU[j].FMMUactive);
                }
                //显示从站FMMU功能映射
                printf(" FMMUfunc 0:%d 1:%d 2:%d 3:%d\n",
                       ec_slave[cnt].FMMU0func, ec_slave[cnt].FMMU1func, ec_slave[cnt].FMMU2func, ec_slave[cnt].FMMU3func);
                //显示从站的邮箱写入和读取长度，以及支持的邮箱协议
                printf(" MBX length wr: %d rd: %d MBX protocols : %2.2x\n", ec_slave[cnt].mbx_l, ec_slave[cnt].mbx_rl, ec_slave[cnt].mbx_proto);
                //查找并存储从站的SII（Slave Information Interface）常规段地址
                ssigen = ec_siifind(cnt, ECT_SII_GENERAL);
                /* 检查从站支持的接口协议类型 */
                if (ssigen)
                {
                    ec_slave[cnt].CoEdetails = ec_siigetbyte(cnt, ssigen + 0x07);
                    ec_slave[cnt].FoEdetails = ec_siigetbyte(cnt, ssigen + 0x08);
                    ec_slave[cnt].EoEdetails = ec_siigetbyte(cnt, ssigen + 0x09);
                    ec_slave[cnt].SoEdetails = ec_siigetbyte(cnt, ssigen + 0x0a);
                    //满足这个条件则启用LRW功能
                    if ((ec_siigetbyte(cnt, ssigen + 0x0d) & 0x02) > 0)
                    {
                        ec_slave[cnt].blockLRW = 1;
                        ec_slave[0].blockLRW++;
                    }
                    //读取从站总线电流信息并加入主站的总电流中
                    ec_slave[cnt].Ebuscurrent = ec_siigetbyte(cnt, ssigen + 0x0e);
                    ec_slave[cnt].Ebuscurrent += ec_siigetbyte(cnt, ssigen + 0x0f) << 8;
                    ec_slave[0].Ebuscurrent += ec_slave[cnt].Ebuscurrent;
                }
                // //打印协议细节
                // printf(" CoE details: %2.2x FoE details: %2.2x EoE details: %2.2x SoE details: %2.2x\n",
                //        ec_slave[cnt].CoEdetails, ec_slave[cnt].FoEdetails, ec_slave[cnt].EoEdetails, ec_slave[cnt].SoEdetails);
                // printf(" Ebus current: %d[mA]\n only LRD/LWR:%d\n",
                //        ec_slave[cnt].Ebuscurrent, ec_slave[cnt].blockLRW);

                if ((ec_slave[cnt].mbx_proto & ECT_MBXPROT_COE) && printSDO)
                    si_sdo(cnt);
                if (printMAP)
                {
                    if (ec_slave[cnt].mbx_proto & ECT_MBXPROT_COE)
                        si_map_sdo(cnt);
                    else
                        si_map_sii(cnt);
                }
            }
        }
        else
        {
            printf("No slaves found!\n");
        }
        printf("End slaveinfo, close socket\n");
        /* stop SOEM, close socket */
        ec_close();
    }
    else
    {
        printf("No socket connection on %s\nExcecute as root\n", ifname);
    }
}

char ifbuf[1024];

int main(int argc, char *argv[])
{
    ec_adaptert *adapter = NULL;
    printf("SOEM (Simple Open EtherCAT Master)\nSlaveinfo\n");

    if (argc > 1)
    {
        if ((argc > 2) && (strncmp(argv[2], "-sdo", sizeof("-sdo")) == 0))
            printSDO = TRUE;
        if ((argc > 2) && (strncmp(argv[2], "-map", sizeof("-map")) == 0))
            printMAP = TRUE;
        /* start slaveinfo */
        strcpy(ifbuf, argv[1]);
        slaveinfo(ifbuf);
    }
    else
    {
        //找可用网卡
        printf("Usage: slaveinfo ifname [options]\nifname = eth0 for example\nOptions :\n -sdo : print SDO info\n -map : print mapping\n");

        printf("Available adapters\n");
        adapter = ec_find_adapters();
        while (adapter != NULL)
        {
            printf("Description : %s, Device to use for wpcap: %s\n", adapter->desc, adapter->name);
            adapter = adapter->next;
        }
        ec_free_adapters(adapter);
    }

    printf("End program\n");
    return (0);
}
