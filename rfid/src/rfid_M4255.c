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
#include <syslog.h>

#include "rfid_M4255.h"
#include "uart.h"

/**
 ** rfid的串口初始化
 ** 参数device表示串口终端的设备节点
 ** 参数old_cfg 记录原始的串口配置
 **/
int rfid_init(const char *device, struct termios *old_cfg)
{
	int rfid_fd; //rfid文件描述符
	struct uart_hardware_cfg ucfg;
	rfid_fd = uart_init(RFIDSCANER_DEV, old_cfg);
	if (rfid_fd < 0)
	{
		return -1;
	}

	if (uart_cfg(rfid_fd, &ucfg) < 0)
	{
		return -1;
	}

	return rfid_fd;
}

/**
 ** rfid的销毁
 ** 参数device表示串口终端的设备节点
 ** 参数old_cfg 记录原始的串口配置
 **/
int rfid_destroy(int rfid_fd, struct termios *old_cfg)
{
	uart_destroy(rfid_fd, old_cfg);
	return 0;
}

int set_emtype(struct element *em, uint8_t btype)
{
	switch (btype)
	{
	case RESISTANCE:
		strcpy(em->type, "电阻");
		break;
	case CAPACITANCE:
		strcpy(em->type, "电容");
		break;
	case INDUCTANCE:
		strcpy(em->type, "电感");
		break;
	case CHIP:
		strcpy(em->type, "芯片");
		break;
	default:
		return -1;
		break;
	}
	return 0;
}

void rfid_utos(int units, char *buf)
{
	switch (units)
	{
	case mO:
		strcpy(buf, "mO");
		break;
	case O:
		strcpy(buf, "O");
		break;
	case kO:
		strcpy(buf, "kO");
		break;
	case uF:
		strcpy(buf, "uF");
		break;
	case mF:
		strcpy(buf, "mF");
		break;
	case F:
		strcpy(buf, "F");
		break;
	case uH:
		strcpy(buf, "uH");
		break;
	case mH:
		strcpy(buf, "mH");
		break;
	case H:
		strcpy(buf, "H");
		break;
	default:
		break;
	}
}