#include "IO_control.h"

infoMal infoMal_teacher = {0,0,0,0,0};
infoMal infoMal_student = {0,0,0,0,0};

uchar resIOJudge[9] = {0};

void IOInit(void){

	DDRA  |= 0x55; 	DDRB  |= 0x55;	DDRC  |= 0x55;	DDRF  |= 0x05;
	PORTA |= 0x55; 	PORTB |= 0x55;	PORTC |= 0x55;	PORTF |= 0x05;
}

void delayTest(void){

	 uint a = 65535;
	 while(--a){
	 	asm("nop");asm("nop");asm("nop");
		asm("nop");asm("nop");asm("nop");	
		asm("nop");asm("nop");asm("nop");	
	 }
}

void IO_test(void){

	const uint count  = 5;
	uint a = count;
	uchar	 loop;
	
	DDRA = DDRB = DDRC = DDRF = 0xff;

	while(1){
 
 		PORTA = 0x01;
 		for(loop = 0;loop < 5;loop++){
		
			PORTA = PORTA << 2;
			while(--a)delayTest();
			a = count;
		}
 		PORTB = 0x01;
 		for(loop = 0;loop < 5;loop++){
		
			PORTB = PORTB << 2;
			while(--a)delayTest();
			a = count;
		}	
 		PORTC = 0x01;
 		for(loop = 0;loop < 5;loop++){
		
			PORTC = PORTC << 2;
			while(--a)delayTest();
			a = count;
		}	
 		PORTF = 0x01;
 		for(loop = 0;loop < 5;loop++){
		
			PORTF = PORTF << 2;
			while(--a)delayTest();
			a = count;
		}	
 	}
}

void IOReinstate(void){

	infoMal_teacher.dianfengshan =\
	infoMal_teacher.weibolu =\
	infoMal_teacher.dianfanbao =\
	infoMal_teacher.xiyiji =\
	infoMal_teacher.xiaodugui \
	= 0;
	
	infoMal_student.dianfengshan =\
	infoMal_student.weibolu =\
	infoMal_student.dianfanbao =\
	infoMal_student.xiyiji =\
	infoMal_student.xiaodugui \
	= 0;
	
	IOInit();
}

void OPJudje(uchar T,uchar S,uchar Base){ 		  //直接异或比较输出结果

	uchar temp,loop,pt;
	pt = 0;
	
	temp = T ^ S;
	for(loop = 0;loop < 8;loop ++){
			 
		if((temp >> loop) & 0x01)resIOJudge[pt++] = Base + loop;
	}
	
	//resIOJudge[6] = temp;						  		 //测试比较结果输出
	//resIOJudge[7] = infoMal_teacher.xiaodugui;
	//resIOJudge[8] = infoMal_student.xiaodugui;
}

//单字节从右到左分别表示设故编号
//1-4号电风扇、5-6号电饭煲、7-9号微波炉、10-12号消毒柜、13-14号洗衣机
void dianfengshan_IO(uchar obj,uchar a){	   //1-4号电风扇

	if(obj == 'T'){		   			 		   //教师端设故信息更新
	
		infoMal_teacher.dianfengshan |= (1 << (a-1));
		//IO Opreation here
		switch(a){
	
			case  1:	   PORTA &= (~0x01);  break;
			case  2:   	   PORTA &= (~0x04);  break;
			case  3:   	   PORTA &= (~0x10);  break;
			case  4:   	   PORTA &= (~0x40);  break;
			default:   	   break;
		}
	} 
	if(obj == 'S'){		   		 			  //学生端设故信息更新
	
		infoMal_student.dianfengshan |= (1 << (a-1));
		//IO Opreation here
		switch(a){
	
			case  1:	   PORTA |= (0x01);  break;
			case  2:   	   PORTA |= (0x04);  break;
			case  3:   	   PORTA |= (0x10);  break;
			case  4:   	   PORTA |= (0x40);  break;
			default:   	   break;
		}	
	}
}

void dianfanbao_IO(uchar obj,uchar a){	  	   //5-7号电饭煲

	if(obj == 'T'){		 		   			   //教师端设故信息更新
	
		infoMal_teacher.dianfanbao |= (1 << (a-1));
		//IO Opreation here
		switch(a){
	
			case  1:	   PORTB &= (~0x01);  break;
			case  2:   	   PORTB &= (~0x04);  break;
			case  3:   	   PORTB &= (~0x10);  break;
			case  4:   	   break;
			default:   	   break;
		}
	} 	
	if(obj == 'S'){				   	  	 	   //学生端设故信息更新
	
		infoMal_student.dianfanbao |= (1 << (a-1));
		//IO Opreation here
		switch(a){
	
			case  1:	   PORTB |= (0x01);  break;
			case  2:   	   PORTB |= (0x04);  break;
			case  3:   	   PORTB |= (0x10);  break;
			default:   	   break;
		}
	}	
}

void weibolu_IO(uchar obj,uchar a){			   //8-9号微波炉

	if(obj == 'T'){	  						   //教师端设故信息更新
	
		infoMal_teacher.weibolu |= (1 << (a-1));
		//IO Opreation here
		switch(a){
	
			case  1:	   PORTB &= (~0x40);  break;
			case  2:   	   PORTC &= (~0x01);  break;
			default:   	   break;
		}
	} 	
	if(obj == 'S'){				   	  	 	   //学生端设故信息更新
	
		infoMal_student.weibolu |= (1 << (a-1));
		//IO Opreation here
		switch(a){
	
			case  1:	   PORTB |= (0x40);  break;
			case  2:   	   PORTC |= (0x01);  break;
			default:   	   break;
		}
	}	
}

void xiaodugui_IO(uchar obj,uchar a){	 	   //10-11号消毒柜

	if(obj == 'T'){				  			   //教师端设故信息更新
	
		infoMal_teacher.xiaodugui |= (1 << (a-1));
		//IO Opreation here
		switch(a){
	
			case  1:	   PORTC &= (~0x04);  break;
			case  2:   	   PORTC &= (~0x10);  break;
			default:   	   break;
		}
	} 
	if(obj == 'S'){				  	 	   	   //学生端设故信息更新
	
		infoMal_student.xiaodugui |= (1 << (a-1));
		//IO Opreation here
		switch(a){
	
			case  1:	   PORTC |= (0x04);  break;
			case  2:   	   PORTC |= (0x10);  break;
			default:   	   break;
		}
	}	
}

void xiyiji_IO(uchar obj,uchar a){			   //12-14号洗衣机

	if(obj == 'T'){	 		   				   //教师端设故信息更新
	
		infoMal_teacher.xiyiji |= (1 << (a-1));
		//IO Opreation here
		switch(a){
	
			case  1:	   PORTC &= (~0x40);  break;
			case  2:   	   PORTF &= (~0x01);  break;
			case  3:   	   PORTF &= (~0x04);  break;
			default:   	   break;
		}
	} 	
	if(obj == 'S'){			   	  	 		   //学生端设故信息更新
	
		infoMal_student.xiyiji |= (1 << (a-1));
		//IO Opreation here
		switch(a){
	
			case  1:	   PORTC |= (0x40);  break;
			case  2:   	   PORTF |= (0x01);  break;
			case  3:   	   PORTF |= (0x04);  break;
			default:   	   break;
		}
	}	
}
