#ifndef _IO_CONTROL_H__
#define _IO_CONTROL_H__

#include<iom128v.h>
#include<macros.h>
#include"__info_dats.h"

void dianfengshan_IO(uchar obj,uchar a);
void weibolu_IO(uchar obj,uchar a);
void dianfanbao_IO(uchar obj,uchar a);
void xiyiji_IO(uchar obj,uchar a);
void xiaodugui_IO(uchar obj,uchar a);

void IOInit(void);
void IOReinstate(void);
void OPJudje(uchar T,uchar S,uchar Base);

void IO_test(void);

#endif