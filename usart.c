#include<stdio.h>      /*标准输入输出定义*/
#include<stdlib.h>     /*标准函数库定义*/
#include<unistd.h>     /*Unix 标准函数定义*/
#include<sys/types.h> 
#include<sys/stat.h>   
#include<fcntl.h>      /*文件控制定义*/
#include<termios.h>    /*PPSIX 终端控制定义*/
#include<errno.h>      /*错误号定义*/
#include<string.h>
#include "usart.h"

//宏定义
#define FALSE  -1
#define TRUE   0
#define TTYSACn "/dev/ttyS2"

/*******************************************************************
* 名称：               UART0_Open
* 功能：           打开串口并返回串口设备文件描述
* 入口参数：     fd    :文件描述符     port :串口号(ttyS0,ttyS1,ttyS2)
* 出口参数：     正确返回为1，错误返回为0
*******************************************************************/
int UART_Open(void)
{
   int fd;
   //1、打开串口
   fd = open(TTYSACn, O_RDWR|O_NOCTTY|O_NDELAY);
   if (FALSE == fd)
   {
		perror("Open Serial Port Failed!\n");
		return(FALSE);
   }
   //恢复串口为阻塞状态                               
   if(fcntl(fd, F_SETFL, 0) < 0)
   {
        printf("fcntl failed!\n");
        return(FALSE);
   }     
   else
   {
        printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
   }
    //测试是否为终端设备,是1，否0    
   if(0 == isatty(fd))
    {
        printf("%s Serial is not a terminal device\n",TTYSACn);
        return(FALSE);
    }
   else
    {
            printf("%s is terminal device!\n",TTYSACn);
    }              
	//printf("fd->open=%d\n",fd);
	return fd;
}
/*******************************************************************
* 名称：                UART0_Close
* 功能：                关闭串口并返回串口设备文件描述
* 入口参数：        fd    :文件描述符     port :串口号(ttyS0,ttyS1,ttyS2)
* 出口参数：        void
*******************************************************************/
void UART_Close(int fd)
{
    close(fd);
}
 
/*******************************************************************
* 名称：                UART0_Set
* 功能：                设置串口数据位，停止位和效验位
* 入口参数：        fd        串口文件描述符
*                              speed     串口速度
*                              flow_ctrl   数据流控制
*                           databits   数据位   取值为 7 或者8
*                           stopbits   停止位   取值为 1 或者2
*                           parity     效验类型 取值为N,E,O,,S
*出口参数：          正确返回为1，错误返回为0
*******************************************************************/
int UART_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,char parity)
{
    int   i;
    int   status;
    int   speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300};
    int   name_arr[] = {115200,  19200,  9600,  4800,  2400,  1200,  300};
         
    struct termios options;
   
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
  
   options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);//我加的
 //options.c_lflag &= ~(ISIG | ICANON);
   
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
/*******************************************************************
* 名称：                UART0_Init()
* 功能：                串口初始化
* 入口参数：        fd       :  文件描述符   
*               speed  :  串口速度
*                              flow_ctrl  数据流控制
*               databits   数据位   取值为 7 或者8
*                           stopbits   停止位   取值为 1 或者2
*                           parity     效验类型 取值为N,E,O,,S
*                      
* 出口参数：        正确返回为1，错误返回为0
*******************************************************************/
int UART_Init(Uart_Src *myuart_src)
{
    int err;
	/*打开串口*/
	myuart_src->fd	= UART_Open();
	/*串口默认参数初始化*/
	myuart_src->init_speed	=115200; //波特率
	myuart_src->flow_ctl	= 0;
	myuart_src->stopbit		= 1;
	myuart_src->databit		= 8;
	myuart_src->parity		='N';

    //设置串口数据帧格式
    err = UART_Set(myuart_src->fd,myuart_src->init_speed,myuart_src->flow_ctl,\
		myuart_src->databit,myuart_src->stopbit,myuart_src->parity);
	if(err == FALSE)
    {                                                         
        return FALSE;
    }
    else
    {
        return  TRUE;
    }
}

/*********************************************************************
*参数：串口信息结构
*功能：修改串口参数
********************************************************************/
int UART_Reset(Uart_Src *myuart_src)
{
	int err;
	char input='n';

	printf("Continue to Modify Uart_Param, input: y\n");

	scanf("%c",&input);

	if('y'==input)
	{
		printf("please input baud:");
		scanf("%d",&myuart_src->init_speed);
		printf("please input databit:");
		scanf("%d",&myuart_src->databit);
		printf("please input stopbit:");
		scanf("%d",&myuart_src->stopbit);
		printf("please input parity:");
		scanf("%c",&myuart_src->parity);
		getchar();

		//设置串口数据帧格式
		err = UART_Set(myuart_src->fd,myuart_src->init_speed,myuart_src->flow_ctl,\
			myuart_src->databit,myuart_src->stopbit,myuart_src->parity); 
		if(err == FALSE)
		{                
			printf("Set Uart failed!\n");
			return FALSE;
		}
		else
		{
			printf("Set Uart successfully!\n");
			return  TRUE;
		}

	}
	else
		printf("quit!");
		return 0;
}


/********************************************************************
* 名称：                  UART0_Send
* 功能：                发送数据
* 入口参数：        
*fd           :文件描述符    
* send_buf    :存放串口发送数据
* data_len    :一帧数据的个数
* 出口参数：   正确返回为1，错误返回为0
*******************************************************************/
int UART_Send(int fd,const void *send_buf,int data_len)
{
    int len = 0;
    len = write(fd,send_buf,data_len);
    return len;
}


/*******************************************************************
* 名称：                  UART0_Recv
* 功能：                接收串口数据
* 入口参数： 
*fd           :文件描述符    
*rcv_buf     :接收串口中数据存入rcv_buf缓冲区中
*data_len    :一帧数据的长度
* 出口参数： 正确返回为1，错误返回为0
*******************************************************************/
int UART_Recv(int fd, void *rcv_buf,int data_len)
{
    int len,fd_ret;
    fd_set fds;
    struct timeval time;
   
    FD_ZERO(&fds);
    FD_SET(fd,&fds);
   
    time.tv_sec = 10;
    time.tv_usec = 0;
   
    //使用select实现串口的多路通信
    fd_ret = select(fd+1,&fds,NULL,NULL,&time);
    if(fd_ret)
    {
        len = read(fd,rcv_buf,data_len);
	printf("read length = %d\n",len);
        return len;
    }
    else
    {
	    printf("Time Out!!");
        return FALSE;
    }     
}




