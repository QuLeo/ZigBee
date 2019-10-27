#ifndef __PM25_H__
#define __PM25_H__
#include <ioCC2530.h>
#define uint unsigned int 
#define uchar unsigned char
#define u16 unsigned short

extern void InitUART(void);
extern void Init_capture_Gpio();
extern void T1_init3();
extern void T1_Capture_r_f_init();
extern void PM2_5Init(void);
extern void T3init();
extern void InitClock(void);
#endif