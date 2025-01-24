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
#include "rfid_db.h"

#define UART2 "/dev/ttySTM1"

struct termios old_cfg; //用于保存终端的配置参数
int rfid_fd;

/**
 ** 信号处理函数，当串口有数据可读时，会跳转到该函数执行
 **/
char rfid_data[RFIDDATASIZE];
int pos;
struct element em;
sqlite3 *db;
void io_handler(int s)
{
    unsigned char buf[10] = {0};
    int ret;
    int n;

    /* 判断串口是否有数据可读 */
    ret = read(rfid_fd, buf, 8); //一次最多读8个字节数据
    // printf("[ ");
    for (n = 0; n < ret; n++)
    {
        // printf("0x%hhx ", buf[n]);
        rfid_data[pos++] = buf[n];
    }
    // printf("]\n");

    if (pos >= 28)
    {
        //检测rfid数据
        printf("[ ");
        for (int i = 0; i < pos; i++)
        {
            printf("0x%x ", rfid_data[i]);
        }
        printf("]\n");
        //提取rfid数据
        //memcpy(&em.id, &rfid_data[7], 4); 会导致字节序反转
        em.id = (rfid_data[7] << 24) + (rfid_data[8] << 16) + (rfid_data[9] << 8) + rfid_data[10];
        printf("card_id = %x ", em.id);
        set_emtype(&em, rfid_data[11]);
        printf("elem_type = %s ", em.type);
        printf("\n");
        em.temp_min = rfid_data[12];
        em.temp_max = rfid_data[13];
        em.wet_min = rfid_data[14];
        em.wet_max = rfid_data[15];
        printf("temp[%d,%d], wet[%d,%d]\n", em.temp_min, em.temp_max, em.wet_min, em.wet_max);

        switch (rfid_data[11])
        {
        case 0:
        case 1:
        case 2:
            em.value = (rfid_data[16] << 8) + rfid_data[17];
            int unit = rfid_data[18];
            rfid_utos(unit, em.unit);

            printf("%d%s", em.value, em.unit);
            break;
        case 3:
            memcpy(em.desc, &rfid_data[16], 11);
            printf("desc:%s", em.desc);
        default:
            break;
        }
        printf("\n");

        Fetch_Elem(db, em);
        //重置数据写入位
        pos = 0;
    }
}

/**
 ** 信号处理函数，当进程被杀死时调用
 **/
void rfid_exit(int s)
{
    /* 退出 */
    fprintf(stderr, "exit process\n");
    sqlite3_close(db);
    rfid_destroy(rfid_fd, &old_cfg);
    exit(EXIT_SUCCESS);
}

/**
 ** 异步I/O初始化函数
 **/
int async_io_init()
{
    struct sigaction sa;

    if (fcntl(rfid_fd, F_SETOWN, getpid()) < 0)
    {
        perror("fcntl()");
        close(rfid_fd);
        return -1;
    }

    int flags = fcntl(rfid_fd, F_GETFL);
    if (fcntl(rfid_fd, F_SETFL, flags | O_ASYNC) < 0)
    {
        perror("fcntl F_SETFL O_ASYNC failed");
        close(rfid_fd);
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

    struct sigaction sa;
    sa.sa_handler = rfid_exit;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGQUIT);
    sigaddset(&sa.sa_mask, SIGTERM);

    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    char *device = UART2;

    /*数据库初始化*/
    int rc;
    rc = sqlite3_open(STORE_DB, &db);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(0);
    }
    else
    {
        fprintf(stdout, "Opened database successfully\n");
    }

    /* 串口初始化 */
    rfid_fd = rfid_init(device, &old_cfg);
    if (rfid_fd < 0)
    {
        fprintf(stderr, "rfid_init():%s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //读串口数据
    async_io_init(); //我们使用异步I/O方式读取串口的数据，调用该函数去初始化串口的异步I/O
    while (1)
        pause(); //进入休眠、等待有数据可读，有数据可读之后就会跳转到io_handler()函数
}
