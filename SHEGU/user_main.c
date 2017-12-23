#include "usart.h"
#include "IO_control.h"

const uchar *ITFR_CMD[4+2+3+3+2] = {

	"res_fengshan.fs_res1.txt=\"0\"",			//1
	"res_fengshan.fs_res2.txt=\"0\"",
	"res_fengshan.fs_res3.txt=\"0\"",
	"res_fengshan.fs_res4.txt=\"0\"",
	
	"res_dianfanbao.dfb_res1.txt=\"0\"",		//5
	"res_dianfanbao.dfb_res2.txt=\"0\"",
	"res_dianfanbao.dfb_res3.txt=\"0\"",
	
	"res_weibolu.wbl_res1.txt=\"0\"",			//8
	"res_weibolu.wbl_res2.txt=\"0\"",
	
	"res_xiaodugui.xdg_res1.txt=\"0\"",			//10
	"res_xiaodugui.xdg_res2.txt=\"0\"",
	
	"res_xiyiji.xyj_res1.txt=\"0\"",			//12
	"res_xiyiji.xyj_res2.txt=\"0\"",
	"res_xiyiji.xyj_res3.txt=\"0\"",	
};

const uchar *WIFI_CMD[17] = {

	"at+netmode=2",
	"at+dhcpd=0",
	"at+wifi_conf="WIFI_INFORMATION,
	"at+dhcpc=0",
//	"at+net_ip="device_IP",255.255.255.0,10.2.8.254",
	"at+net_ip="device_IP",255.255.255.0,192.168.0.1",
//	"at+net_dns=202.96.128.86,202.96.134.33",
	"at+net_dns=192.168.0.1,0.0.0.0",
	"at+remotepro=tcp",
	"at+mode=client",
//	"at+remoteip=10.2.8.139",
	"at+remoteip=192.168.0.103",
	"at+remoteport=8088",
	"at+CLport=1234",
	"at+timeout=0",
	"at+uart=9600,8,n,1",
	"at+uartpacklen=64",
	"at+uartpacktimeout=10",
	"at+net_commit=1",
	"at+reconn=1"
};

const char cntConfirm[8] = {0x5a,device_ID,0x03,0x40,0x40,0x40,0x40};

extern uchar WIFI_BUF_QUE[BUFFER0_QLENTH][UART0_RX_BUFFER_SIZE];	
extern uchar ITFR_BUF_QUE[BUFFER1_QLENTH][UART1_RX_BUFFER_SIZE];	

extern infoMal infoMal_teacher;
extern infoMal infoMal_student;

extern uchar resIOJudge[9];	   		//��ʦ�����ѧ����ʱȽϽ�����

infoUser userMe 	 = {"00000000","00000000"};
infoUser userMe_temp = {"00000000","00000000"};

uchar UI_pt = 0x00;	   		//UI�����αָ꣬��ǰ����ʾ����

void delay_3us(void);
void wifiConnecting(void);

void delay_3us(void)  //3us��ʱ����,,���ظ����ò�Ӱ�쾫�� 
{
   asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
   asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
   asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
}

void delay_10us(void)  //10us��ʱ����,���ظ����ò�Ӱ�쾫��
{
 	delay_3us();
 	delay_3us();
 	delay_3us();
}

void delay_50us(void)  //48us��ʱ����,���ظ����ò�Ӱ�쾫��
{      
  	delay_10us();    
  	delay_10us();    
  	delay_10us();   
  	delay_10us();    
  	delay_10us();
}
 
void delay_100us(void)//exactly 98us��ʱ����,���ظ����ò�Ӱ�쾫��
{
  delay_50us();
  delay_50us();
  delay_3us();
}
 
void delay_1ms(void)            
{
  delay_100us(); delay_100us(); delay_100us(); delay_100us(); delay_100us();
  delay_100us(); delay_100us(); delay_100us(); delay_100us(); delay_100us();
}
  
 void delay_ms(uint ms )
{
    while(ms--)  
		delay_1ms();//8MHZ��Ƶʱ��25�������С��1��
}

void EEPROM_write(uchar addr,uchar data){

	while(EECR & BIT(EEWE));
	EEAR=addr;
	EEDR=data;
	EECR|=BIT(EEMWE);
	EECR|=BIT(EEWE);
}

uchar EEPROM_read(uchar addr){

 	while(EECR & BIT(EEWE));
	EEAR=addr;
	EECR|=BIT(EERE);
	return EEDR;
}

void userID_read(uchar Dats[8]){
	 
	uchar loop;
	
	for(loop = 0;loop < 8;loop ++)
		Dats[loop] = EEPROM_read(loop+0x10);
}

void userID_write(uchar Dats[8]){		   //eeprom���û���
	 
	uchar loop;
	
	for(loop = 0;loop < 8;loop ++)
		EEPROM_write(loop+0x10,Dats[loop]);
}


void password_read(uchar Dats[8]){		   //eeprom������
	 
	uchar loop;
	
	for(loop = 0;loop < 8;loop ++)
		Dats[loop] = EEPROM_read(loop+0x20);
}

void password_write(uchar Dats[8]){
	 
	uchar loop;
	
	for(loop = 0;loop < 8;loop ++)
		EEPROM_write(loop+0x20,Dats[loop]);
}

void reaptRes(void){					   //�ύ���

	 uchar loop;
	 
	 for(loop = 0;loop < 4;loop ++)
	 	if(resIOJudge[loop] == 0)resIOJudge[loop] = 0x40;

	 WIFI_PUTC(0x5a);
	 WIFI_PUTC(device_ID);
	 WIFI_PUTC(0x20);
	 WIFI_PUTS(resIOJudge);
}

void bspInit(void)		   				   //��ʼ��
{	
 	usart_init();						   //���ڳ�ʼ��
	
	ITFR_PUTS("page connecting");		   //��ҳ
	
	IOReinstate();						   //�˿ڸ�λ����Ϣ���
	
	wifiConnecting();
	
	userID_read(userMe.userID);			   //���û�
	password_read(userMe.password);		   //������
}

void getConnect(void){

	uchar loopa;
	
	for(loopa = 0;loopa < 30;loopa ++){
				  
		WIFI_PUTC('+');
		delay_ms(100);
	}
		
	for(loopa = 0;loopa < 17;loopa ++)
		WIFI_PUTS((uchar*)WIFI_CMD[loopa]);
		
	delay_ms(500);
	WIFI_PUTS("at+reconn=1");
	delay_ms(500);
	WIFI_PUTS("at+reconn=1");
	delay_ms(500);
}

void wifiConnecting(void){	 			   //��ʼ������
	
	ITFR_PUTS("page connecting");
	UI_pt = 0x00;
	
	while(1)
	if(!ATTX_M1(WIFI,(char*)cntConfirm,(char*)cntConfirm,0,1500,10))
	{
	
		ITFR_PUTS("page main");
		UI_pt = 0x01;
		return;
	}
	else 
	{
	 	 
	 	getConnect();
		ITFR_PUTS("page connecting");
	}
}

void wifiReconnect(void){			 		//��;�Ͽ�����
	
	ITFR_PUTS("page reconnect");
	
	while(1)
	if(!ATTX_M1(WIFI,(char*)cntConfirm,(char*)cntConfirm,0,1000,6))
	{
	
		switch(UI_pt){
		
			case 0x01:	ITFR_PUTS("page main");					   break;
			case 0x02:	ITFR_PUTS("page device_select");		   break;
			default:	break;
		}
		return;
	}
	else 
	{
	 	 
	 	getConnect();
		ITFR_PUTS("page reconnect");
	}
}

void thread_main(void){

	 uchar loopa,loopb;
	 
	 unsigned long  heartBeat_A,heartBeat_B;
	 
	 heartBeat_A = heartBeat_B = 0;

	 while(1){
	 
	 /*********************************************PROCESSING_WIFI*********************************************/
		 
		 //putchar0(0x30+strcmp(userMe_temp.userID,userMe.userID));				//�û����ȽϽ������
		 //putchar0(0x30+strcmp(userMe_temp.password,userMe.password));			//����ȽϽ������
		 
		 if(WIFI_BUF[0] == 0x5a){				//��ͷ����
		 
		 	 if((WIFI_BUF[1] == device_ID) || (WIFI_BUF[1] == 0x06)){
			 
			 	 switch(WIFI_BUF[2]){
				  
				  	 case 0x01:		 strcpy(userMe.userID,&(WIFI_BUF[3]));userID_write(userMe.userID);	//��ʦ�����û���
					 	  			 ITFR_PUTS("page main");
					 	  			 //WIFI_PUTS("userID set");WIFI_PUTS(userMe.userID);		//����wifi��ȡ��ʦ���õ��û���
						  			 break;
									 
					 case 0x02:		 strcpy(userMe.password,&(WIFI_BUF[3]));password_write(userMe.password);	//��ʦ��������
					 	  			 ITFR_PUTS("page main");
					 	  			 //WIFI_PUTS("password set");WIFI_PUTS(userMe.password);	//���Բ���wifi��ȡ��ʦ���õ�����
									 break;
									 
					 case 0x10:		 IOReinstate();			   									//�����ظ���ʣ������һ��Ϊ׼������һ�θ�λ
					 	  			 ITFR_PUTS("device_select.dfs.txt=\"0\"");
									 ITFR_PUTS("device_select.dfb.txt=\"0\"");
									 ITFR_PUTS("device_select.wbl.txt=\"0\"");
									 ITFR_PUTS("device_select.xdg.txt=\"0\"");
									 ITFR_PUTS("device_select.xyj.txt=\"0\"");
									 
					 	  			 for(loopa = 3;loopa < 17;loopa ++){						//��ʦ��ʽ���,ע��˴��ֽ���������
					 
					 	  			 	switch(WIFI_BUF[loopa]){   								//��ʰ���Ӧ���趨����ʦ���Ҫ��
					 	  			 					 
					 	  			 	 	case  1:	dianfengshan_IO('T',1);	break;
										 	case  2:	dianfengshan_IO('T',2); break;
										 	case  3:	dianfengshan_IO('T',3);	break;
										 	case  4:	dianfengshan_IO('T',4);	break;
										 	case  5:	dianfanbao_IO('T',1);	break;
				 	  			 	 	 	case  6:	dianfanbao_IO('T',2);	break;
										 	case  7:	dianfanbao_IO('T',3);	break;
										 	case  8:	weibolu_IO('T',1);		break;
										 	case  9:	weibolu_IO('T',2);		break;
										 	case 10:	xiaodugui_IO('T',1);	break;
										 	case 11:	xiaodugui_IO('T',2);	break;
										 	case 12:	xiyiji_IO('T',1);		break;
										 	case 13:	xiyiji_IO('T',2);		break;
										 	case 14:	xiyiji_IO('T',3);		break;
											default:	break;
					 	  			 	 }
					 	  			 }
									 WIFI_BUF[1] = device_ID; 					//�޸�ID��ԭ��������Ӧ����
				 					 WIFI_PUTS(WIFI_BUF);
									 
									 heartBeat_B = 0;	 						//����
									 
									 break;
									 
					 case 0x30:		 IOReinstate(); 	 						//�豸��λ
					 	  			 ITFR_PUTS("page device_select");			//���¿���ѡ�����
									 UI_pt = 0x02;
					 	  			 WIFI_BUF[1] = device_ID; 		 			//�޸�ID��ԭ��������Ӧ����
				 					 WIFI_PUTS(WIFI_BUF);
					 	  			 break;
									 
					 case 0x03:		 heartBeat_B = 0; 		 					//ȷ�����ӣ������������
					 	  			 break;
									 
					 default:		 break;
				 }
				 
		 		 if(infoMal_teacher.dianfengshan)ITFR_PUTS("device_select.dfs.txt=\"1\"");	//�ⶳ�豸ѡ��				 
				 if(infoMal_teacher.dianfanbao)ITFR_PUTS("device_select.dfb.txt=\"1\"");
				 if(infoMal_teacher.weibolu)ITFR_PUTS("device_select.wbl.txt=\"1\"");
				 if(infoMal_teacher.xiaodugui)ITFR_PUTS("device_select.xdg.txt=\"1\"");
				 if(infoMal_teacher.xiyiji)ITFR_PUTS("device_select.xyj.txt=\"1\"");
			 	 memset(WIFI_BUF,0,sizeof(uchar)*UART0_RX_BUFFER_SIZE);
			 }
		 }
		 //delay_ms(1500);			//������ʱ
		 //WIFI_PUTS(WIFI_BUF);		//����WIFI����
		 
		 
	  /*********************************************PROCESSING_ITFR*********************************************/

		 if(ITFR_BUF[0] == 0x5a){		//��ͷ����
		 
		 	 switch(ITFR_BUF[1]){
			 
			 	  	case 0x01:		strcpy(userMe_temp.userID,&(ITFR_BUF[2]));							//ѧ�������û���
						 			//ITFR_PUTS("GOT userID");ITFR_PUTS(userMe_temp.userID);			//����ѧ��������û���
						  			break;
					case 0x02:		strcpy(userMe_temp.password,&(ITFR_BUF[2]));						//ѧ����������
						 			//ITFR_PUTS("GOT password set");ITFR_PUTS(userMe_temp.password);	//����ѧ�����������
						 			break;
									
					case 0x10:		for(loopa = 0;loopa < 4;loopa ++)										   //ѧ���ύ��ʽ������������ȣ�4����ʵ�
						 				if(ITFR_BUF[loopa+2] == '1')dianfengshan_IO('S',loopa+1);			   //��ʰ���Ӧ���趨��ѧ���ύ���
										
									OPJudje(infoMal_teacher.dianfengshan,infoMal_student.dianfengshan,1); 	   //�Ƚ�ѧ���ύ����������趨���õ��ٲý��		
									for(loopa = 0;loopa < 4;loopa ++)										   //ÿһλ���Ƚϣ������α��λ
									    for(loopb = 1;loopb <= 4;loopb ++)		  							   //�򱾵��û��ύ�ٲý����ֻ�ύ������Ϣ��loopb�α��1-4����ʾ����1-4Ϊ����ȹ���
										    if(resIOJudge[loopa] == loopb)ITFR_PUTS((char*)ITFR_CMD[loopb-1]); //�򱾵��û��ύ�����ֻ�ύ�������ʱ�ţ�[loopb-1]Ϊ���ϱ��������������ֵ
									
									memset(resIOJudge,0,sizeof(uchar)*9);		  //����Ƚ���Ϣ�������
									OPJudje(0x00,infoMal_student.dianfengshan,1); //�õ�ѧ�����ı䶯�Ĺ��϶�Ӧ�Ĺ��ϱ����Ϣ�����ٲ�
									reaptRes();				//ѧ���䶯�ύ��ʦ
										
									break;
									
					case 0x20:		for(loopa = 0;loopa < 2;loopa ++)		  						//΢��¯��2����ʵ�
						 				if(ITFR_BUF[loopa+2] == '1')weibolu_IO('S',loopa+1);		//loop+2Ϊ��������ͷ�����ֽ�
										
									OPJudje(infoMal_teacher.weibolu,infoMal_student.weibolu,8);		
									for(loopa = 0;loopa < 2;loopa ++)				
									    for(loopb = 8;loopb <= 9;loopb ++)										//8-9
										    if(resIOJudge[loopa] == loopb)ITFR_PUTS((char*)ITFR_CMD[loopb-1]);
										
									memset(resIOJudge,0,sizeof(uchar)*9);
									OPJudje(0x00,infoMal_student.weibolu,8);
									reaptRes();
																		
									break;	
									
					case 0x30:		for(loopa = 0;loopa < 3;loopa ++)						   		 //�緹�ң�3����ʵ�
						 				if(ITFR_BUF[loopa+2] == '1')dianfanbao_IO('S',loopa+1);
										
									OPJudje(infoMal_teacher.dianfanbao,infoMal_student.dianfanbao,5);	
									for(loopa = 0;loopa < 3;loopa ++)							
									    for(loopb = 5;loopb <= 7;loopb ++)										//5-7
										    if(resIOJudge[loopa] == loopb)ITFR_PUTS((char*)ITFR_CMD[loopb-1]);
									
									memset(resIOJudge,0,sizeof(uchar)*9);	
									OPJudje(0x00,infoMal_student.dianfanbao,5);
									reaptRes();
				
									break;
									
					case 0x40:		for(loopa = 0;loopa < 3;loopa ++)								 //ϴ�»���3����ʵ�
						 				if(ITFR_BUF[loopa+2] == '1')xiyiji_IO('S',loopa+1);
										
									OPJudje(infoMal_teacher.xiyiji,infoMal_student.xiyiji,12);		
									for(loopa = 0;loopa < 3;loopa ++)								
									    for(loopb = 12;loopb <= 14;loopb ++)									//12-14
										    if(resIOJudge[loopa] == loopb)ITFR_PUTS((char*)ITFR_CMD[loopb-1]);
										
									memset(resIOJudge,0,sizeof(uchar)*9);
									OPJudje(0x00,infoMal_student.xiyiji,12);
									reaptRes();
									
									break;
									
					case 0x50:		for(loopa = 0;loopa < 2;loopa ++)						  		 //������2����ʵ�
						 				if(ITFR_BUF[loopa+2] == '1')xiaodugui_IO('S',loopa+1);
										
									OPJudje(infoMal_teacher.xiaodugui,infoMal_student.xiaodugui,10);		
									for(loopa = 0;loopa < 2;loopa ++)				
										for(loopb = 10;loopb <= 11;loopb ++)									//10-11
											if(resIOJudge[loopa] == loopb)ITFR_PUTS((char*)ITFR_CMD[loopb-1]);
										
									memset(resIOJudge,0,sizeof(uchar)*9);
									OPJudje(0x00,infoMal_student.xiaodugui,10);
									reaptRes();

									//ITFR_PUTS("Result xiaodugui hand up");		//�������
									//putchar1(resIOJudge[0]+0x30);	   				//���ԱȽϽ�����
									//putchar1(resIOJudge[1]+0x30);
									//putchar1(resIOJudge[2]+0x30);
									//putchar1(resIOJudge[3]+0x30);
									//putchar1(resIOJudge[6]+0x30);
									//putchar1(resIOJudge[7]+0x30);
									//putchar1(resIOJudge[8]+0x30);										
									break;
									
					case 0x0f:		IOReinstate();									//����ύ��ɣ��豸��λ�����¿���ѡ�����
						 			//ITFR_PUTS("device_select.dfs.txt=\"0\"");		//��������ѡ��ȴ���ʦ�������
									//ITFR_PUTS("device_select.wbl.txt=\"0\"");
									//ITFR_PUTS("device_select.dfb.txt=\"0\"");
									//ITFR_PUTS("device_select.xyj.txt=\"0\"");
									//ITFR_PUTS("device_select.xdg.txt=\"0\"");								
						 			//ITFR_PUTS("page device_select");				//�������ѡ�����
									
					default:		break;
			 }
			 
			 if(!strcmp(userMe_temp.userID,userMe.userID) && !strcmp(userMe_temp.password,userMe.password)){	//�û�����������֤
		 			
		 			ITFR_PUTS("page device_select");	//�������ѡ�����
					UI_pt = 0x02;
		 	 }
		 	 memset(ITFR_BUF,0,sizeof(uchar)*UART1_RX_BUFFER_SIZE);		  //��Ļ�����������
			 memset(userMe_temp.password,0,sizeof(uchar)*9);			  //�û������������
		 	 memset(resIOJudge,0,sizeof(uchar)*9);						  //ѧ���ύ����ʦ����ٲý�����
		}
		
	  /*********************************************PROCESSING_HEARTBEAT*******************************************/
	  	if(heartBeat_A > 300000){					//���ڷ���������
		
			heartBeat_A = 0;	   
			WIFI_PUTS((char*)cntConfirm);
		}else heartBeat_A ++;
		
		if(heartBeat_B > 2000000){		 			//���ڽ��������ظ�   ��������һ���յ�����
			
			heartBeat_B = 0;
			wifiReconnect();
		}else heartBeat_B ++;
	}
}

void main(void){

	
	
	bspInit();
	
	//WIFI_PUTS("i'm WIFI test start!!!");
	//ITFR_PUTS("i'm ITFR test start!!!");
	//IO_test();
	
	thread_main();

}