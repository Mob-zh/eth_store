#ifndef _RFID_M4255_H_
#define _RFID_M4255_H_
#include <stdint.h>

#define RFIDSCANER_DEV "/dev/ttySTM1"
#define RFIDDATASIZE 28
#define MAX_ELEM_TYPE_SIZE 30
#define DEFAULT_STORE_TYPE "混合"
#define STORE_TYPE_MIX "混合"
#define STORE_TYPE_O "电阻"
#define STORE_TYPE_F "电容"
#define STORE_TYPE_H "电感"
#define STORE_TYPE_C "芯片"

enum ELEM_TYPE
{
    RESISTANCE = 0, //  电阻
    CAPACITANCE,    //  电容
    INDUCTANCE,     //   电感
    CHIP            //    芯片
};
enum UNITS_TYPE
{
    mO,
    O,
    kO,
    uF,
    mF,
    F,
    uH,
    mH,
    H
};

struct rfid_cfg
{
    int stat;
    int def_block;
};

struct element
{
    uint32_t id;
    char type[MAX_ELEM_TYPE_SIZE];
    uint8_t temp_min; //最小温度
    uint8_t temp_max;
    uint8_t wet_min; //最小湿度
    uint8_t wet_max;
    uint16_t value; //元器件参数
    char unit[4];   //参数单位

    uint16_t shelf_id; //所属货架id
    uint16_t slot_id;  //所属货架格id
    char desc[15];     //型号
};

struct shelf
{
    uint16_t shelf_id; //所属货架id
    char store_type[MAX_ELEM_TYPE_SIZE];
    int capacity;
};

/**
 ** rfid的串口初始化
 ** 参数device表示串口终端的设备节点
 ** 参数old_cfg 记录原始的串口配置
 **/
int rfid_init(const char *device, struct termios *old_cfg);

/**
 ** rfid的销毁
 ** 参数device表示串口终端的设备节点
 ** 参数old_cfg 记录原始的串口配置
 **/
int rfid_destroy(int rfid_fd, struct termios *old_cfg);

//自动根据接收到的字节给type赋字符值
int set_emtype(struct element *em, uint8_t btype);

void rfid_utos(int units, char *buf);
#endif
