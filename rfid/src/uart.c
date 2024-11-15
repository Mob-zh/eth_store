#define _GNU_SOURCE //在源文件开头定义_GNU_SOURCE宏
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <sys/poll.h>
#include "uart.h"

/**
 ** 串口初始化操作
 ** 参数device表示串口终端的设备节点
 **/
int uart_init(const char *device, struct termios *cfg)
{
    /* 打开串口终端 */
    int uart_fd = open(device, O_RDWR | O_NOCTTY);
    if (0 > uart_fd)
    {
        fprintf(stderr, "open error: %s: %s\n", device, strerror(errno));
        return -1;
    }

    /* 获取串口当前的配置参数 */
    if (0 > tcgetattr(uart_fd, cfg))
    {
        fprintf(stderr, "tcgetattr error: %s\n", strerror(errno));
        close(uart_fd);
        return -1;
    }

    return uart_fd;
}

/**
 ** 串口配置
 ** 参数cfg指向一个uart_cfg_t结构体对象
 **  --brate=9600  --dbit=8  --parity=N --sbit=1  --type=read
 **/
int uart_cfg(int uart_fd, const uart_cfg_t *cfg)
{
    struct termios new_cfg = {0}; //将new_cfg对象清零
    speed_t speed;

    /* 设置为原始模式 */
    cfmakeraw(&new_cfg);

    /* 使能接收 */
    new_cfg.c_cflag |= CREAD;

    /* 设置波特率 */
    switch (cfg->baudrate)
    {
    case 1200:
        speed = B1200;
        break;
    case 1800:
        speed = B1800;
        break;
    case 2400:
        speed = B2400;
        break;
    case 4800:
        speed = B4800;
        break;
    case 9600:
        speed = B9600;
        break;
    case 19200:
        speed = B19200;
        break;
    case 38400:
        speed = B38400;
        break;
    case 57600:
        speed = B57600;
        break;
    case 115200:
        speed = B115200;
        break;
    case 230400:
        speed = B230400;
        break;
    case 460800:
        speed = B460800;
        break;
    case 500000:
        speed = B500000;
        break;
    default: //默认配置为9600
        speed = B9600;
        printf("default baud rate: 9600\n");
        break;
    }

    if (0 > cfsetspeed(&new_cfg, speed))
    {
        fprintf(stderr, "cfsetspeed error: %s\n", strerror(errno));
        return -1;
    }

    /* 设置数据位大小 */
    new_cfg.c_cflag &= ~CSIZE; //将数据位相关的比特位清零
    switch (cfg->dbit)
    {
    case 5:
        new_cfg.c_cflag |= CS5;
        break;
    case 6:
        new_cfg.c_cflag |= CS6;
        break;
    case 7:
        new_cfg.c_cflag |= CS7;
        break;
    case 8:
        new_cfg.c_cflag |= CS8;
        break;
    default: //默认数据位大小为8
        new_cfg.c_cflag |= CS8;
        printf("default data bit size: 8\n");
        break;
    }

    /* 设置奇偶校验 */
    switch (cfg->parity)
    {
    case 'N': //无校验
        new_cfg.c_cflag &= ~PARENB;
        new_cfg.c_iflag &= ~INPCK;
        break;
    case 'O': //奇校验
        new_cfg.c_cflag |= (PARODD | PARENB);
        new_cfg.c_iflag |= INPCK;
        break;
    case 'E': //偶校验
        new_cfg.c_cflag |= PARENB;
        new_cfg.c_cflag &= ~PARODD; /* 清除PARODD标志，配置为偶校验 */
        new_cfg.c_iflag |= INPCK;
        break;
    default: //默认配置为无校验
        new_cfg.c_cflag &= ~PARENB;
        new_cfg.c_iflag &= ~INPCK;
        printf("default parity: N\n");
        break;
    }

    /* 设置停止位 */
    switch (cfg->sbit)
    {
    case 1: //1个停止位
        new_cfg.c_cflag &= ~CSTOPB;
        break;
    case 2: //2个停止位
        new_cfg.c_cflag |= CSTOPB;
        break;
    default: //默认配置为1个停止位
        new_cfg.c_cflag &= ~CSTOPB;
        printf("default stop bit size: 1\n");
        break;
    }

    /* 将MIN和TIME设置为0 */
    new_cfg.c_cc[VTIME] = 0;
    new_cfg.c_cc[VMIN] = 0;

    /* 清空缓冲区 */
    if (0 > tcflush(uart_fd, TCIOFLUSH))
    {
        fprintf(stderr, "tcflush error: %s\n", strerror(errno));
        return -1;
    }

    /* 写入配置、使配置生效 */
    if (0 > tcsetattr(uart_fd, TCSANOW, &new_cfg))
    {
        fprintf(stderr, "tcsetattr error: %s\n", strerror(errno));
        return -1;
    }

    /* 配置OK 退出 */
    return 0;
}

int uart_destroy(int uart_fd, const struct termios *old_cfg)
{
    tcsetattr(uart_fd, TCSANOW, old_cfg); //恢复到之前的配置
    close(uart_fd);

    return 0;
}
