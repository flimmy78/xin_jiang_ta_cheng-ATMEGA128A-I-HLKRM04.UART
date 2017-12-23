#include "usart.h"


const uchar *WIFICmd[] = {

	0  
};

const uchar *ITFRCmd[] = {

	0  
};

static volatile uchar UART0_RxHead,UART1_RxHead;		//�������ݻ����α�

uchar UART0_RxBuff[UART0_RX_BUFFER_SIZE];				//����0����
uchar UART1_RxBuff[UART1_RX_BUFFER_SIZE];				//����1����

uchar UART0_RxBuff_Queue[BUFFER0_QLENTH][UART0_RX_BUFFER_SIZE];	  //����0�������
uchar UART1_RxBuff_Queue[BUFFER1_QLENTH][UART1_RX_BUFFER_SIZE];	  //����1�������

void delay(unsigned int ms){

 	 unsigned int i,j;
	 for( i=0;i<ms;i++)
	 for(j=0;j<1141;j++); //1141����8MHz�����£�ͨ���������淴��ʵ��õ�����ֵ
}

void usart0Init(void){

	 UART0_RxHead = 0;

	 UCSR0B = 0x00;                   	//�ر�UART00
	 UCSR0A =0x00;                     	//��ʹ�ñ��ٷ��ͣ��첽��
	 UCSR0C =(1<<UCSZ01)|(1<<UCSZ00);   //����λΪ8λ
	 UBRR0L=(FOSC/16/(BAUD0+1))%256;    //�첽��������µļ��㹫ʽ
	 UBRR0H=(FOSC/16/(BAUD0+1))/256;
	 UCSR0B =(1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);     //����ʹ�ܺͷ���ʹ���Լ��ж�
}

void usart1Init(void){

	 UART1_RxHead = 0;

 	 UCSR1B = 0x00;   					//�ر�USART1
 	 UCSR1A = 0x00;   					//����ʹ�ñ��ٷ���
 	 UCSR1C = (1<<UCSZ11)|(1<<UCSZ10);	//����λΪ��λ
 	 UBRR1L=(FOSC/16/(BAUD1+1))%256;	//�첽����ģʽ�£�UBRR�ļ��㹫ʽ
 	 UBRR1H=(FOSC/16/(BAUD1+1))/256;
 	 UCSR1B =(1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1); 	//����ʹ�ܣ�����ʹ���Լ��ж�
}

void putchar0(unsigned char c){  

     while (!(UCSR0A&(1<<UDRE0)));		//�����������Ѿ�׼������
     UDR0=c;    						//��Ҫ���͵�����װ��UDR0�Ĵ���
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
	
	putchar0(0xff);	  //��β��ʶ
 	putchar0(0xff);
	putchar0(0xff);
}
 
void puts1(char *s){

	 while (*s){
	 
  	 	 putchar1(*s);
		 s++;
	}
	
	putchar1('\r');   //��β��ʶ
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
			memset(UART0_RxBuff,0,sizeof(uchar)*UART0_RX_BUFFER_SIZE);	//�������
	
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
			memset(UART1_RxBuff,0,sizeof(uchar)*UART1_RX_BUFFER_SIZE);	//�������
			
		}else UART1_RxBuff[UART1_RxHead++] = data;		
	}
}


//���룺
//		interface���ӿ����ͣ�'A'��ʾ����0��'B'��ʾ����1
//		command  ������
//		rec		 ������
//		crp		 : �����α�
//		wait_time�����εȴ�ʱ��
//		rep		 ���ȴ�����
//������أ�
//		0���ɹ���1��ʧ��
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
		memset(UART0_RxBuff_Queue[crp],0,sizeof(uchar)*UART0_RX_BUFFER_SIZE);	//�������
	}
		
	if(interface == 'B'){
		
		while(strcmp(rec,UART1_RxBuff_Queue[crp])){
		
			memset(UART1_RxBuff_Queue[crp],0,sizeof(uchar)*UART1_RX_BUFFER_SIZE);
			puts1(command);
			delay(wait_time);
			time_point++;
			if(time_point > rep)return 1;
		}
		memset(UART1_RxBuff_Queue[crp],0,sizeof(uchar)*UART1_RX_BUFFER_SIZE);	//�������
	}
		
	delay(100);
	return 0;
}

//���룺
//		interface���ӿ����ͣ�'A'��ʾ����0��'B'��ʾ����1
//		command  ������
//		rec		 ������
//		crp		 : �����α�
//		over_time���ȴ�ʱ��
//������أ�
//		0���ɹ���1��ʧ��
uchar ATTX_M2(uchar interface,uchar *command,uchar *rec,uchar crp,uint over_time)
{
		
	uint time_point = 1;
	const uint freq = 200;			//��ѯƵ���趨��200ms��ѯһ��
	
	delay(100);
		
	if(interface == 'A'){
	
		puts0(command);
		while(strcmp(rec,UART0_RxBuff_Queue[crp])){
			
				delay(freq);
				if(time_point > over_time/freq)return 1;
				time_point++;
		}
		memset(UART0_RxBuff_Queue[crp],0,sizeof(uchar)*UART0_RX_BUFFER_SIZE);	//�������
	}
	
	if(interface == 'B'){
	
		puts1(command);
		while(strcmp(rec,UART1_RxBuff_Queue[crp])){
			
				delay(freq);
				if(time_point > over_time/freq)return 1;
				time_point++;
		}
		memset(UART1_RxBuff_Queue[crp],0,sizeof(uchar)*UART1_RX_BUFFER_SIZE);	//�������
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