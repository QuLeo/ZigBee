#ifndef __PM25_H__
#define __PM25_H__
#include <ioCC2530.h>

#define uint unsigned int 
#define uchar unsigned char
#define u16 unsigned short


#define UART0_RX    1
#define UART0_TX    2
#define SIZE       51

extern void DelayMS(uint msec);
extern void InitUart(void);
extern void UartSendString(char *Data, int len);

#endif