#include "usart.h"


const uchar *WIFICmd[] = {

	0  
};

const uchar *ITFRCmd[] = {

	0  
};

static volatile uchar UART0_RxHead,UART1_RxHead;		//串口数据缓存游标

uchar UART0_RxBuff[UART0_RX_BUFFER_SIZE];				//串口0缓存
uchar UART1_RxBuff[UART1_RX_BUFFER_SIZE];				//串口1缓存

uchar UART0_RxBuff_Queue[BUFFER0_QLENTH][UART0_RX_BUFFER_SIZE];	  //串口0缓存队列
uchar UART1_RxBuff_Queue[BUFFER1_QLENTH][UART1_RX_BUFFER_SIZE];	  //串口1缓存队列

void delay(unsigned int ms){

 	 unsigned int i,j;
	 for( i=0;i<ms;i++)
	 for(j=0;j<1141;j++); //1141是在8MHz晶振下，通过软件仿真反复实验得到的数值
}

void usart0Init(void){

	 UART0_RxHead = 0;

	 UCSR0B = 0x00;                   	//关闭UART00
	 UCSR0A =0x00;                     	//不使用倍速发送（异步）
	 UCSR0C =(1<<UCSZ01)|(1<<UCSZ00);   //数据位为8位
	 UBRR0L=(FOSC/16/(BAUD0+1))%256;    //异步正常情况下的计算公式
	 UBRR0H=(FOSC/16/(BAUD0+1))/256;
	 UCSR0B =(1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);     //接收使能和发送使能以及中断
}

void usart1Init(void){

	 UART1_RxHead = 0;

 	 UCSR1B = 0x00;   					//关闭USART1
 	 UCSR1A = 0x00;   					//不适使用倍速发送
 	 UCSR1C = (1<<UCSZ11)|(1<<UCSZ10);	//数据位为八位
 	 UBRR1L=(FOSC/16/(BAUD1+1))%256;	//异步正常模式下，UBRR的计算公式
 	 UBRR1H=(FOSC/16/(BAUD1+1))/256;
 	 UCSR1B =(1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1); 	//接收使能，传送使能以及中断
}

void putchar0(unsigned char c){  

     while (!(UCSR0A&(1<<UDRE0)));		//表明发送器已经准备就绪
     UDR0=c;    						//将要发送的数据装入UDR0寄存器
}

void putchar1(unsigned char c){  

     while (!(UCSR1A&(1<<UDRE1)));
  	 UDR1=c;    
}

void puts0(char *s){
	 
	 while (*s){
	 
  	 	 putchar0(*s);
		 s++;
    } 
	
	putchar0(0xff);	  //包尾标识
 	putchar0(0xff);
	putchar0(0xff);
}
 
void puts1(char *s){

	 while (*s){
	 
  	 	 putchar1(*s);
		 s++;
	}
	
	putchar1('\r');   //包尾标识
 	putchar1('\n');
} 

#pragma interrupt_handler uart0_rx_isr:iv_USART0_RXC
void uart0_rx_isr(void){
	 
	uchar data,status,loop;
	uchar crp = UART0_RxHead;
	
	data   = UDR0;
	status = UCSR0A;

	if(!(status & 0x18)){		
	
		if((UART0_RxHead > (UART0_RX_BUFFER_SIZE-2)) || ((data == '\n') && (UART0_RxBuff[crp-1] == '\r'))){
			
			UART0_RxHead = 0;
			
			UART0_RxBuff[crp-1] = 0;
				
			for(loop = BUFFER0_QLENTH;loop > 1;loop --)			
				strcpy(UART0_RxBuff_Queue[loop - 1],UART0_RxBuff_Queue[loop - 2]);
				
			strcpy(UART0_RxBuff_Queue[0],UART0_RxBuff);
			memset(UART0_RxBuff,0,sizeof(uchar)*UART0_RX_BUFFER_SIZE);	//缓存清空
	
		}else UART0_RxBuff[UART0_RxHead++] = data;
	}
}

#pragma interrupt_handler uart1_rx_isr:iv_USART1_RXC
void uart1_rx_isr(void){
	 
	uchar data,status,loop;
	uchar crp = UART1_RxHead;
	
	data   = UDR1;
	status = UCSR1A;

	if(!(status & 0x18)){		
	
		if((UART1_RxHead > (UART1_RX_BUFFER_SIZE-2)) || ((data == '\n') && (UART1_RxBuff[crp-1] == '\r'))){
		
			UART1_RxHead = 0;
			
			UART1_RxBuff[crp-1] = 0;
				
			for(loop = BUFFER1_QLENTH;loop > 1;loop --)			
				strcpy(UART1_RxBuff_Queue[loop - 1],UART1_RxBuff_Queue[loop - 2]);
				
			strcpy(UART1_RxBuff_Queue[0],UART1_RxBuff);
			memset(UART1_RxBuff,0,sizeof(uchar)*UART1_RX_BUFFER_SIZE);	//缓存清空
			
		}else UART1_RxBuff[UART1_RxHead++] = data;		
	}
}


//输入：
//		interface：接口类型，'A'表示串口0，'B'表示串口1
//		command  ：命令
//		rec		 ：反馈
//		crp		 : 缓存游标
//		wait_time：单次等待时间
//		rep		 ：等待次数
//输出返回：
//		0：成功，1：失败
uchar ATTX_M1(uchar interface,uchar *command,uchar *rec,uchar crp,uint wait_time,uchar rep)
{
	uchar time_point = 1;
	
	delay(100);

	if(interface == 'A'){
		
		while(strcmp(rec,UART0_RxBuff_Queue[crp])){
		
			memset(UART0_RxBuff_Queue[crp],0,sizeof(uchar)*UART0_RX_BUFFER_SIZE);
			puts0(command);
			delay(wait_time);
			time_point++;
			if(time_point > rep)return 1;
		}
		memset(UART0_RxBuff_Queue[crp],0,sizeof(uchar)*UART0_RX_BUFFER_SIZE);	//缓存清空
	}
		
	if(interface == 'B'){
		
		while(strcmp(rec,UART1_RxBuff_Queue[crp])){
		
			memset(UART1_RxBuff_Queue[crp],0,sizeof(uchar)*UART1_RX_BUFFER_SIZE);
			puts1(command);
			delay(wait_time);
			time_point++;
			if(time_point > rep)return 1;
		}
		memset(UART1_RxBuff_Queue[crp],0,sizeof(uchar)*UART1_RX_BUFFER_SIZE);	//缓存清空
	}
		
	delay(100);
	return 0;
}

//输入：
//		interface：接口类型，'A'表示串口0，'B'表示串口1
//		command  ：命令
//		rec		 ：反馈
//		crp		 : 缓存游标
//		over_time：等待时间
//输出返回：
//		0：成功，1：失败
uchar ATTX_M2(uchar interface,uchar *command,uchar *rec,uchar crp,uint over_time)
{
		
	uint time_point = 1;
	const uint freq = 200;			//轮询频次设定，200ms轮询一次
	
	delay(100);
		
	if(interface == 'A'){
	
		puts0(command);
		while(strcmp(rec,UART0_RxBuff_Queue[crp])){
			
				delay(freq);
				if(time_point > over_time/freq)return 1;
				time_point++;
		}
		memset(UART0_RxBuff_Queue[crp],0,sizeof(uchar)*UART0_RX_BUFFER_SIZE);	//缓存清空
	}
	
	if(interface == 'B'){
	
		puts1(command);
		while(strcmp(rec,UART1_RxBuff_Queue[crp])){
			
				delay(freq);
				if(time_point > over_time/freq)return 1;
				time_point++;
		}
		memset(UART1_RxBuff_Queue[crp],0,sizeof(uchar)*UART1_RX_BUFFER_SIZE);	//缓存清空
	}

	delay(100);
	return 0;
}

void usart_init(void){

  	SEI();
	  
	usart0Init();
	usart1Init();	 
}

void usart_test(void)
{
 	  uchar temp = UART1_RxHead;

  	  SEI();
	  
	  usart0Init();
	  usart1Init();
	  
	  ATTX_M2('B',"abc","bcd",0,9000);  
	  
	  while(1){
	  		   temp = UART0_RxHead;
	  
	  		putchar1(UART1_RxBuff[temp-2]);
			puts1(UART1_RxBuff_Queue[4]);
			delay(500);
			if(UART1_RxBuff[0]){
								
				//puts0(UART0_RxBuff_Queue[0]);
				//CLI();
				//memset(UART0_RxBuff,0,sizeof(uchar)*UART0_RX_BUFFER_SIZE);
				//SEI();
			}
	  }
}