/*****************************************************************
 ** 文件名:  HTMngr.c
 ** 2020-2022 珠海禾田电子科技有限公司 Co.Ltd
 
 ** 创建人: 骆军城
 ** 日  期: 2022-01
 ** 描  述: 程序系统管理模块
 ** 版  本: V1.0

 ** 修改人:
 ** 日  期:
 ** 修改描述:
 ** 版  本: 
******************************************************************/

/*****************************************************************
* 包含头文件
******************************************************************/
#include <stdio.h>      /*标准输入输出定义*/
#include <stdlib.h>     /*标准函数库定义*/
#include <unistd.h>     /*Unix 标准函数定义*/
#include <sys/types.h> 
#include <sys/stat.h>   
#include <sys/select.h>

#include <sys/time.h>

#include <fcntl.h>      /*文件控制定义*/
#include <termios.h>    /*PPSIX 终端控制定义*/
#include <errno.h>      /*错误号定义*/
#include <string.h>
#include "usart.h"

/*****************************************************************
* 宏定义(仅在当前C文件使用的宏定义写在当前C文件中，否则需写在H文件中)
******************************************************************/
#define FALSE  						-1
#define TRUE   						0

/*****************************************************************
* 结构定义(仅在当前C文件使用的结构体写在当前C文件中，否则需写在H文件中)
******************************************************************/

/*****************************************************************
* 全局变量定义
******************************************************************/

/*****************************************************************
* 静态变量定义
******************************************************************/

/*****************************************************************
* 外部变量声明（如果全局变量没有在其它的H文件声明，引用时需在此处声明，
*如果已在其它H文件声明，则只需包含此H文件即可）
******************************************************************/

/*****************************************************************
* 函数原型声明
******************************************************************/

/*****************************************************************
* 函数定义
*注意，编写函数需首先定义所有的局部变量等，不允许在
*函数的中间出现新的变量的定义。
******************************************************************/

/*************************************************
** Function：	UART0_Set
** Description：	串口参数设置
** Input:	none
** Output：	none

** Return：	0-成功；-1-失败
** Author：骆军城
** Date：2022-01-25

** Modification History：
** Author：
** Date：
** Description：
*************************************************/
int UART_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,char parity)
{
    int   i = 0;
    int   status = 0;
    int   speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300};
    int   name_arr[] = {115200,  19200,  9600,  4800,  2400,  1200,  300};
         
    struct termios options = {0};
   
    /*tcgetattr(fd,&options)得到与fd指向对象的相关参数
	并将它们保存于options,该函数还可以测试配置是否正确，
	该串口是否可用等。若调用成功，函数返回值为0，
	若调用失败，函数返回值为1.
    */
    if (tcgetattr( fd,&options)  !=  0)
    {
        perror("SetupSerial 1");    
        return(FALSE); 
    }
  
    //设置串口输入波特率和输出波特率
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
    {
        if  (speed == name_arr[i])
        {             
                cfsetispeed(&options, speed_arr[i]); 
                cfsetospeed(&options, speed_arr[i]);  
        }
     }     

    //修改控制模式，保证程序不会占用串口
    options.c_cflag |= CLOCAL;
    //修改控制模式，使得能够从串口中读取输入数据
    options.c_cflag |= CREAD;
    //设置数据流控制
    switch(flow_ctrl)
    {
       case 0 ://不使用流控制
              options.c_cflag &= ~CRTSCTS;
              break;   
      
       case 1 ://使用硬件流控制
              options.c_cflag |= CRTSCTS;
              break;
       case 2 ://使用软件流控制
              options.c_cflag |= IXON | IXOFF | IXANY;
              break;
    }
    //设置数据位
    //屏蔽其他标志位
    options.c_cflag &= ~CSIZE;
    switch (databits)
    {  
       case 5    :
                 options.c_cflag |= CS5;
                 break;
       case 6    :
                 options.c_cflag |= CS6;
                 break;
       case 7    :    
                 options.c_cflag |= CS7;
                 break;
       case 8:    
                 options.c_cflag |= CS8;
                 break;  
       default:   
                 fprintf(stderr,"Unsupported data size\n");
                 return (FALSE); 
    }
    //设置校验位
    switch (parity)
    {  
       case 'n':
       case 'N': //无奇偶校验位。
                 options.c_cflag &= ~PARENB; 
                 options.c_iflag &= ~INPCK;    
                 break; 
       case 'o':  
       case 'O'://设置为奇校验    
                 options.c_cflag |= (PARODD | PARENB); 
                 options.c_iflag |= INPCK;             
                 break; 
       case 'e': 
       case 'E'://设置为偶校验  
                 options.c_cflag |= PARENB;       
                 options.c_cflag &= ~PARODD;       
                 options.c_iflag |= INPCK;      
                 break;
       case 's':
       case 'S': //设置为空格 
                 options.c_cflag &= ~PARENB;
                 options.c_cflag &= ~CSTOPB;
                 break; 
        default:  
                 fprintf(stderr,"Unsupported parity\n");    
                 return (FALSE); 
    } 
    // 设置停止位 
    switch (stopbits)
    {  
       case 1:   
                 options.c_cflag &= ~CSTOPB; break; 
       case 2:   
                 options.c_cflag |= CSTOPB; break;
       default:   
                       fprintf(stderr,"Unsupported stop bits\n"); 
                       return (FALSE);
    }
   
   //修改输出模式，原始数据输出
   options.c_oflag &= ~OPOST;
   options.c_oflag &= ~(ONLCR | OCRNL);
  
   options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);//我加的
   
   options.c_iflag &= ~(ICRNL | INLCR);
   options.c_iflag &= ~(IXON | IXOFF | IXANY);

   
    //设置等待时间和最小接收字符
    options.c_cc[VTIME] = 1; /* 读取一个字符等待1*(1/10)s */  
    options.c_cc[VMIN] = 1; /* 读取字符的最少个数为1 */
   
    //如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读
    tcflush(fd,TCIFLUSH);
   
    //激活配置 (将修改后的termios数据设置到串口中）
   if (tcsetattr(fd,TCSANOW,&options) != 0)  
   {
		perror("com set error!\n");  
		return (FALSE); 
   }
   
	return (TRUE); 
}

/*************************************************
** Function：	UART_Send
** Description：	   发送数据
** Input:	none
** Output：	none

** Return：	0-成功；-1-失败
** Author：骆军城
** Date：2022-01-25

** Modification History：
** Author：
** Date：
** Description：
*************************************************/
int UART_Send(int fd,const void *send_buf,int data_len, int mTimeout)
{
	int ret 	= 0;
	struct timeval t;
	fd_set write_fds;
	
	if(fd<0) return 0;
	
	t.tv_sec	= 0;
	t.tv_usec	= mTimeout*1000;
 
	FD_ZERO(&write_fds);
	FD_SET(fd, &write_fds);
 
	ret = select(fd + 1, NULL, &write_fds, NULL, &t);
	if(ret < 0)
	{
		perror("select failed\r\n");	 
		return 0;
	}
	
	if(FD_ISSET(fd, &write_fds))
	{
		ret = write(fd, send_buf, data_len);
		if(ret > 0)
		{
			return ret;
		}
		else
		{
			return 0;
		}
	}
 
	return 0;
}


/*************************************************
** Function：	UART0_Recv
** Description：	   接收串口数据
** Input:	none
** Output：	none

** Return：	0-成功；-1-失败
** Author：骆军城
** Date：2022-01-25

** Modification History：
** Author：
** Date：
** Description：
*************************************************/
int UART_Recv(int fd, void *rcv_buf,int data_len, int mTimeout)
{
	int ret 		= 0;
	int readSize	= 0;
	fd_set read_fds = {0};
 
	struct timeval t = {0};

	if(fd<0) return 0;

	mTimeout = mTimeout/10;

	do
	{
		t.tv_sec	= 0;
		t.tv_usec	= 10*1000; //ms阻塞
		FD_ZERO(&read_fds);
		FD_SET(fd, &read_fds);
 
		ret = select(fd + 1, &read_fds, NULL, NULL, &t);
		if(ret < 0)
		{
			perror("select failed!\r\n");
			return -1;
		}
		
		if(FD_ISSET(fd, &read_fds))
		{
			ret = read(fd, (uint8_t*)(rcv_buf+readSize), (data_len-readSize));
			if(ret > 0)
			{
				readSize += ret;
				//数据读完了
				if(readSize >= data_len)
				{						
					break;
				}
			}
			else if(ret == 0)
			{
				perror("rcv occur error\r\n");	 
				break;
			}
		}
 
		if(mTimeout >=1)
			mTimeout--;
 
	}while(mTimeout>0);
	
	return readSize;
}


/*************************************************
** Function：	UART0_Open
** Description：	   打开串口并返回串口设备文件描述
** Input:	uartName :串口号(ttyS0,ttyS1,ttyS2)
** Output：	none

** Return：	0-成功；-1-失败
** Author：骆军城
** Date：2022-01-25

** Modification History：
** Author：
** Date：
** Description：
*************************************************/
int UART_Open(char *uartName)
{
   int fd = -1;

	if(strlen(uartName)<3 || strlen(uartName)>24)
	{
		perror("pUartName error\n");
		return(FALSE);
   }
   
   //1、打开串口
   fd = open(uartName, O_RDWR|O_NOCTTY|O_NDELAY);
   if (FALSE == fd)
   {
		perror("Open Port Failed!\n");
		return(FALSE);
   }
   
   //恢复串口为阻塞状态                               
   if(fcntl(fd, F_SETFL, 0) < 0)
   {
        perror("fcntl failed!\n");
		goto iExit;
   }     
   
	//测试是否为终端设备,是1，否0    
	if(0 == isatty(fd))
	{
		fprintf(stderr, "%s Serial is not a terminal device\n", uartName);
		goto iExit;
	}           

	return fd;
iExit:
	UART_Close(fd);
	return(FALSE);
}

/*************************************************
** Function：	UART_Close
** Description：	串口去初始化
** Input:	none
** Output：	none

** Return：	0-成功；-1-失败
** Author：骆军城
** Date：2022-01-25

** Modification History：
** Author：
** Date：
** Description：
*************************************************/
int UART_Close(int fd)
{
	if(fd>0)
	    close(fd);

	return 0;
}


/*************************************************
** Function：	UART_Init
** Description：	串口初始化
** Input:	none
** Output：	none

** Return：	0-成功；-1-失败
** Author：骆军城
** Date：2022-01-25

** Modification History：
** Author：
** Date：
** Description：
*************************************************/
int UART_Init(stUartCfg *pStUartCfg)
{
    int err = -1;
	
	/*打开串口*/
	pStUartCfg->fd	= UART_Open(pStUartCfg->uartName);
	
	/*串口默认参数初始化*/
	pStUartCfg->stopbit		= 1;
	pStUartCfg->databit		= 8;
	pStUartCfg->flow_ctl	= 0;
	pStUartCfg->parity		= 'N';

    //设置串口数据帧格式
    err = UART_Set(pStUartCfg->fd,pStUartCfg->init_speed,pStUartCfg->flow_ctl,\
		pStUartCfg->databit,pStUartCfg->stopbit,pStUartCfg->parity);
	
	if(err == FALSE)
    {                                                         
        return FALSE;
    }
    else
    {
        return  TRUE;
    }
}



