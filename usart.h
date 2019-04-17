#ifndef _USART_H
#define _USART_H
/*存放串口信息结构*/
typedef struct uart_src
{
	int fd;
	int init_speed;
	int flow_ctl;
	int stopbit;
	int databit;
	char parity;
}Uart_Src;


int UART_Reset(Uart_Src *myuart_src);
int UART_Init(Uart_Src *myuart_src);
int UART_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,char parity);
void UART_Close(int fd);
int UART_Open(void);
int UART_Recv(int fd, void *rcv_buf,int data_len);
int UART_Send(int fd,const void *send_buf,int data_len);
#endif
