#ifndef _UART_H_
#define _UART_H_

typedef struct uart_hardware_cfg
{
    unsigned int baudrate; /* 波特率 */
    unsigned char dbit;    /* 数据位 */
    char parity;           /* 奇偶校验 */
    unsigned char sbit;    /* 停止位 */
} uart_cfg_t;

/**
 ** 串口初始化操作
 ** 参数device表示串口终端的设备节点
 ** 返回uart_fd
 **/
int uart_init(const char *device, struct termios *cfg);

/**
 ** 串口配置
 ** 参数cfg指向一个uart_cfg_t结构体对象
 **/
int uart_cfg(int uart_fd, const uart_cfg_t *cfg);

int uart_destroy(int uart_fd, const struct termios *cfg);

#endif
