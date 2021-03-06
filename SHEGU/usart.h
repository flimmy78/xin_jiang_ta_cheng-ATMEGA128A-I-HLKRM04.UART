#ifndef _UART_H__
#define _UART_H__

#include<iom128v.h>
#include<macros.h>
#include"__info_dats.h"
#include"string.h"

#define	 FOSC	   8000000		//外部晶振设定
#define	 BAUD0	   9600			//串口0波特率设定
#define	 BAUD1	   9600			//串口1波特率设定

#define	 UART0_RX_BUFFER_SIZE	30			//串口0接收缓存尺寸
#define	 UART1_RX_BUFFER_SIZE	30			//串口1接收缓存尺寸

#define	 BUFFER0_QLENTH			5			//串口0接收缓存队列长度
#define	 BUFFER1_QLENTH			5			//串口1接收缓存队列长度

#define	 ITFR_BUF_QUE			UART0_RxBuff_Queue
#define	 WIFI_BUF_QUE			UART1_RxBuff_Queue

#define	 ITFR_BUF				ITFR_BUF_QUE[0]
#define	 WIFI_BUF				WIFI_BUF_QUE[0]

#define	 ITFR					'A'
#define	 WIFI					'B'

#define	 ITFR_PUTS				puts0	
#define	 WIFI_PUTS				puts1	

#define	 ITFR_PUTC				putchar0	
#define	 WIFI_PUTC				putchar1	

void delay(unsigned int ms);
void usart0Init(void);
void usart1Init(void);
void putchar0(unsigned char c);
void putchar1(unsigned char c);
void puts0(char *s);
void puts1(char *s);
uchar ATTX_M1(uchar interface,uchar *command,uchar *rec,uchar crp,uint wait_time,uchar rep);
uchar ATTX_M2(uchar interface,uchar *command,uchar *rec,uchar crp,uint over_time);
void usart_init(void);

void usart_test(void);

#endif