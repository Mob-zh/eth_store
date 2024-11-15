//#define _GNU_SOURCE //在源文件开头定义_GNU_SOURCE宏
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
#include <poll.h>
#include "uart.h"
#include "rfid_M4255.h"

#define UART2 "/dev/ttySTM1"

struct termios old_cfg; //用于保存终端的配置参数
int uart_fd;

/**
 ** 信号处理函数，当串口有数据可读时，会跳转到该函数执行
 **/
void io_handler(int s)
{
    unsigned char buf[10] = {0};
    int ret;
    int n;

    /* 判断串口是否有数据可读 */
    ret = read(uart_fd, buf, 8); //一次最多读8个字节数据
    printf("[ ");
    for (n = 0; n < ret; n++)
        printf("0x%hhx ", buf[n]);
    printf("]\n");
}

/**
 ** 异步I/O初始化函数
 **/
int async_io_init()
{
    struct sigaction sa;

    if (fcntl(uart_fd, F_SETOWN, getpid()) < 0)
    {
        perror("fcntl()");
        close(uart_fd);
        return -1;
    }

    int flags = fcntl(uart_fd, F_GETFL);
    if (fcntl(uart_fd, F_SETFL, flags | O_ASYNC) < 0)
    {
        perror("fcntl F_SETFL O_ASYNC failed");
        close(uart_fd);
        return -1;
    }

    sa.sa_handler = io_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGIO, &sa, NULL) < 0)
    {
        perror("sigaction()");
        return -1;
    }
    return 0;
}

int main()
{
    uart_cfg_t cfg = {0};
    char *device = UART2;

    /* 串口初始化 */
    uart_fd = uart_init(device, &old_cfg);
    if (uart_fd < 0)
    {
        exit(EXIT_FAILURE);
    }

    /* 串口配置 */
    if (uart_cfg(uart_fd, &cfg) < 0)
    {
        tcsetattr(uart_fd, TCSANOW, &old_cfg); //恢复到之前的配置
        close(uart_fd);
        exit(EXIT_FAILURE);
    }

    //读串口数据
    async_io_init(); //我们使用异步I/O方式读取串口的数据，调用该函数去初始化串口的异步I/O
    while (1)
        pause(); //进入休眠、等待有数据可读，有数据可读之后就会跳转到io_handler()函数

    /* 退出 */
    tcsetattr(uart_fd, TCSANOW, &old_cfg); //恢复到之前的配置
    close(uart_fd);
    exit(EXIT_SUCCESS);
}
