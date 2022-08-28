/*****************************************************************
 ** 文件名 : usart.h
 ** 2020-2022 珠海禾田电子科技有限公司 Co.Ltd
 
 ** 创建人: 骆军城
 ** 日  期: 2022-01
 ** 描  述: linux串口头文件
 ** 版  本: V1.0

 ** 修改人:
 ** 日  期:
 ** 修改描述:
 ** 版  本: 
******************************************************************/
#ifndef _USART_H_
#define _USART_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************
* 包含头文件(如非特殊需要，H文件不建议包含其它H文件)
******************************************************************/
#include <stdint.h>

/*****************************************************************
* 宏定义
******************************************************************/


/*****************************************************************
* 结构定义
******************************************************************/
/*存放串口信息结构*/
typedef struct _uartCfg
{
	int 		fd;
	int 		init_speed;
	char 		flow_ctl;
	char 		stopbit;
	char 		databit;
	char 		parity;
	char 		uartName[32];
}stUartCfg;


/*****************************************************************
* 函数原型声明
******************************************************************/

int UART_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,char parity);
int UART_Recv(int fd, void *rcv_buf,int data_len,int mTimeout);
int UART_Send(int fd,const void *send_buf,int data_len, int mTimeout);

int UART_Open(char *uartName);
int UART_Close(int fd);

int UART_Init(stUartCfg *pstUartCfg);

#ifdef __cplusplus
}
#endif

#endif


