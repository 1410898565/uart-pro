#include<stdio.h>      /*��׼�����������*/
#include<stdlib.h>     /*��׼�����ⶨ��*/
#include<unistd.h>     /*Unix ��׼��������*/
#include<sys/types.h> 
#include<sys/stat.h>   
#include<fcntl.h>      /*�ļ����ƶ���*/
#include<termios.h>    /*PPSIX �ն˿��ƶ���*/
#include<errno.h>      /*����Ŷ���*/
#include<string.h>
#include "usart.h"

//�궨��
#define FALSE  -1
#define TRUE   0
#define TTYSACn "/dev/ttyS2"

/*******************************************************************
* ���ƣ�               UART0_Open
* ���ܣ�           �򿪴��ڲ����ش����豸�ļ�����
* ��ڲ�����     fd    :�ļ�������     port :���ں�(ttyS0,ttyS1,ttyS2)
* ���ڲ�����     ��ȷ����Ϊ1�����󷵻�Ϊ0
*******************************************************************/
int UART_Open(void)
{
   int fd;
   //1���򿪴���
   fd = open(TTYSACn, O_RDWR|O_NOCTTY|O_NDELAY);
   if (FALSE == fd)
   {
		perror("Open Serial Port Failed!\n");
		return(FALSE);
   }
   //�ָ�����Ϊ����״̬                               
   if(fcntl(fd, F_SETFL, 0) < 0)
   {
        printf("fcntl failed!\n");
        return(FALSE);
   }     
   else
   {
        printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
   }
    //�����Ƿ�Ϊ�ն��豸,��1����0    
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
* ���ƣ�                UART0_Close
* ���ܣ�                �رմ��ڲ����ش����豸�ļ�����
* ��ڲ�����        fd    :�ļ�������     port :���ں�(ttyS0,ttyS1,ttyS2)
* ���ڲ�����        void
*******************************************************************/
void UART_Close(int fd)
{
    close(fd);
}
 
/*******************************************************************
* ���ƣ�                UART0_Set
* ���ܣ�                ���ô�������λ��ֹͣλ��Ч��λ
* ��ڲ�����        fd        �����ļ�������
*                              speed     �����ٶ�
*                              flow_ctrl   ����������
*                           databits   ����λ   ȡֵΪ 7 ����8
*                           stopbits   ֹͣλ   ȡֵΪ 1 ����2
*                           parity     Ч������ ȡֵΪN,E,O,,S
*���ڲ�����          ��ȷ����Ϊ1�����󷵻�Ϊ0
*******************************************************************/
int UART_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,char parity)
{
    int   i;
    int   status;
    int   speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300};
    int   name_arr[] = {115200,  19200,  9600,  4800,  2400,  1200,  300};
         
    struct termios options;
   
    /*tcgetattr(fd,&options)�õ���fdָ��������ز���
	�������Ǳ�����options,�ú��������Բ��������Ƿ���ȷ��
	�ô����Ƿ���õȡ������óɹ�����������ֵΪ0��
	������ʧ�ܣ���������ֵΪ1.
    */
    if (tcgetattr( fd,&options)  !=  0)
    {
        perror("SetupSerial 1");    
        return(FALSE); 
    }
  
    //���ô������벨���ʺ����������
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
    {
        if  (speed == name_arr[i])
        {             
                cfsetispeed(&options, speed_arr[i]); 
                cfsetospeed(&options, speed_arr[i]);  
        }
     }     

    //�޸Ŀ���ģʽ����֤���򲻻�ռ�ô���
    options.c_cflag |= CLOCAL;
    //�޸Ŀ���ģʽ��ʹ���ܹ��Ӵ����ж�ȡ��������
    options.c_cflag |= CREAD;
    //��������������
    switch(flow_ctrl)
    {
       case 0 ://��ʹ��������
              options.c_cflag &= ~CRTSCTS;
              break;   
      
       case 1 ://ʹ��Ӳ��������
              options.c_cflag |= CRTSCTS;
              break;
       case 2 ://ʹ�����������
              options.c_cflag |= IXON | IXOFF | IXANY;
              break;
    }
    //��������λ
    //����������־λ
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
    //����У��λ
    switch (parity)
    {  
       case 'n':
       case 'N': //����żУ��λ��
                 options.c_cflag &= ~PARENB; 
                 options.c_iflag &= ~INPCK;    
                 break; 
       case 'o':  
       case 'O'://����Ϊ��У��    
                 options.c_cflag |= (PARODD | PARENB); 
                 options.c_iflag |= INPCK;             
                 break; 
       case 'e': 
       case 'E'://����ΪżУ��  
                 options.c_cflag |= PARENB;       
                 options.c_cflag &= ~PARODD;       
                 options.c_iflag |= INPCK;      
                 break;
       case 's':
       case 'S': //����Ϊ�ո� 
                 options.c_cflag &= ~PARENB;
                 options.c_cflag &= ~CSTOPB;
                 break; 
        default:  
                 fprintf(stderr,"Unsupported parity\n");    
                 return (FALSE); 
    } 
    // ����ֹͣλ 
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
   
   //�޸����ģʽ��ԭʼ�������
   options.c_oflag &= ~OPOST;
  
   options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);//�Ҽӵ�
 //options.c_lflag &= ~(ISIG | ICANON);
   
    //���õȴ�ʱ�����С�����ַ�
    options.c_cc[VTIME] = 1; /* ��ȡһ���ַ��ȴ�1*(1/10)s */  
    options.c_cc[VMIN] = 1; /* ��ȡ�ַ������ٸ���Ϊ1 */
   
    //�����������������������ݣ����ǲ��ٶ�ȡ ˢ���յ������ݵ��ǲ���
    tcflush(fd,TCIFLUSH);
   
    //�������� (���޸ĺ��termios�������õ������У�
   if (tcsetattr(fd,TCSANOW,&options) != 0)  
   {
       perror("com set error!\n");  
      return (FALSE); 
   }
    return (TRUE); 
}
/*******************************************************************
* ���ƣ�                UART0_Init()
* ���ܣ�                ���ڳ�ʼ��
* ��ڲ�����        fd       :  �ļ�������   
*               speed  :  �����ٶ�
*                              flow_ctrl  ����������
*               databits   ����λ   ȡֵΪ 7 ����8
*                           stopbits   ֹͣλ   ȡֵΪ 1 ����2
*                           parity     Ч������ ȡֵΪN,E,O,,S
*                      
* ���ڲ�����        ��ȷ����Ϊ1�����󷵻�Ϊ0
*******************************************************************/
int UART_Init(Uart_Src *myuart_src)
{
    int err;
	/*�򿪴���*/
	myuart_src->fd	= UART_Open();
	/*����Ĭ�ϲ�����ʼ��*/
	myuart_src->init_speed	=115200; //������
	myuart_src->flow_ctl	= 0;
	myuart_src->stopbit		= 1;
	myuart_src->databit		= 8;
	myuart_src->parity		='N';

    //���ô�������֡��ʽ
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
*������������Ϣ�ṹ
*���ܣ��޸Ĵ��ڲ���
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

		//���ô�������֡��ʽ
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
* ���ƣ�                  UART0_Send
* ���ܣ�                ��������
* ��ڲ�����        
*fd           :�ļ�������    
* send_buf    :��Ŵ��ڷ�������
* data_len    :һ֡���ݵĸ���
* ���ڲ�����   ��ȷ����Ϊ1�����󷵻�Ϊ0
*******************************************************************/
int UART_Send(int fd,const void *send_buf,int data_len)
{
    int len = 0;
    len = write(fd,send_buf,data_len);
    return len;
}


/*******************************************************************
* ���ƣ�                  UART0_Recv
* ���ܣ�                ���մ�������
* ��ڲ����� 
*fd           :�ļ�������    
*rcv_buf     :���մ��������ݴ���rcv_buf��������
*data_len    :һ֡���ݵĳ���
* ���ڲ����� ��ȷ����Ϊ1�����󷵻�Ϊ0
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
   
    //ʹ��selectʵ�ִ��ڵĶ�·ͨ��
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




