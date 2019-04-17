#include "usart.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void printHex(char *pBuff, int nlen, char *pTempBuf)
{
	 int i = 0;

     if(pTempBuf != NULL)
     {
        printf("%s", pTempBuf);
     }
     
	 for(i=0; i<nlen; i++)
	 {
		printf("%02x ", pBuff[i]);
	 }
     printf("\r\n\r\n");

}
int main()
{
	int nRet = 0;
	char buf[256]={0};
	Uart_Src myuart_src; 
	UART_Init(&myuart_src);
	char cmd[3]={0};
	cmd[0] =0x7f;
	cmd[1] =0x01;
	cmd[2] =0x00;

	while(1)
	{
		memset(buf,0,sizeof(buf));
		nRet=UART_Send(myuart_src.fd,cmd,sizeof(cmd));
		printHex(cmd,sizeof(cmd),"cmd:");

		UART_Recv(myuart_src.fd,buf,64);
		printf("nRet=%d\n",nRet);
		printHex(buf,64,"RxData:");
		sleep(1);

	}
	return 0;
}

