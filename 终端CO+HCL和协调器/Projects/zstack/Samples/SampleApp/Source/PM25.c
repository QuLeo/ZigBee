#include <PM25.h>
#include <string.h>
#include <stdio.h>
typedef unsigned char   uint8;
#define uint unsigned int 
#define uchar unsigned char
uint8 PM_data[6];
uchar i=0;
float count=0;
float countsum=0;
float count_aver=0;
unsigned short pm_data=0;


void T3init()
{ 
  T3CTL |= 0x08 ;          //开溢出中断     
  T3IE = 1;                //开总中断和T3中断
  T3CTL |= 0xE0;           //128分频,128/16000000*N=0.5S,N=62500
  T3CTL &= ~0x03;          //自动重装 00－>0xff  62500/255=245(次)
  T3CTL |= 0x10;           //启动
  EA = 1;                  //开总中断,一次中断计时0.002048秒
}
void InitClock(void)
{   
    CLKCONCMD &= ~0x40;              //设置系统时钟源为 32MHZ晶振
    while(CLKCONSTA & 0x40);         //等待晶振稳定 
    CLKCONCMD &= ~0x47;              //设置系统主时钟频率为 32MHZ
}


/****************************************************************************
* 名    称: InitUart()
* 功    能: 串口初始化函数
* 入口参数: 无
* 出口参数: 无
****************************************************************************/
void InitUART(void)
{
    /*PERCFG = 0x00;                   //位置1 P0口
    P0SEL = 0x3C;                    //P0用作串口
    
    P2DIR &= ~0xC0;                  //P0优先作为UART0    
    U0CSR |= 0x80;                   //串口设置为UART方式
    U0GCR |= 11;				
    U0BAUD |= 216;                   //波特率设为115200
    
    UTX0IF = 1;                      //UART0 TX中断标志初始置位1  
    U0CSR |= 0x40;                   //允许接收
    IEN0 |= 0x84;                    //开总中断，接收中断*/
    
    //**PERCFG &= 0xfe;           //外设控制寄存器 USART 0的IO位置:0为P0口位置1 
    PERCFG =0xbe;//定时器1备用位置1，定时器1备用位置1,UART0备用位置1
    P0SEL |= 0x0c;            //P0_2,P0_3用作串口（外设功能）
    P2DIR &= ~0xC0;          //P0优先作为UART0
    
    U0CSR |= 0x80;           //设置为UART方式
    U0GCR |= 11;				       
    U0BAUD |= 216;           //波特率设为115200
    UTX0IF = 0;              //UART0 TX中断标志初始置位0
    U0CSR |= 0x40;           //允许接收 
    IEN0 |= 0x84;            //开总中断允许接收中断 
}

void Init_capture_Gpio()
{
  P0SEL |= (1<<4);//P0_4复用功能
  P0DIR &=~(1<<4);//设置GPIO为输入
 // P0_4=0;
  //P2INP |=0x20;//P0端口全部引脚下拉
 // P0INP=0xff;//P0端口全部引脚三态
  //**  PERCFG &=0xbf;//定时器1备用位置1
  PERCFG =0xbe;//定时器1备用位置1,UART0备用位置1
  //P2DIR |=0xc0;//定时器1通道2优先  *****
  P2DIR &= ~0xC0;          //P0优先作为UART0  ****
}

void T1_init3()//配置定时器1为所有沿触发
{
//  T1CTL=0x0d;
  T1CCTL2 = 0x43;//开中断，并设置所有沿捕获
  //T1CCTL2 = 0x41;//开中断，并设置上升捕获
 // PICTL &=0xfe;//P0_0-P0_7上升沿触发
 // PICTL=0xfe;//P0_0-P0_7上升沿触发
 // PICTL |=0x01;//P0_0-P0_7下降沿触发
  IRCON=0;//清除中断标志
  P0IFG =0x00;//清除中断标志

  T1IE=1;//使能定时器1中断  
  EA=1;
}

void T1_Capture_r_f_init()
{
  Init_capture_Gpio();
  T1_init3();
}


void PM2_5Init()
{ 
 // InitClock(); //时钟主频率设置为32M
  //InitUART();//初始化串口，用以发送数据
  CLKCONCMD |= (1<<3);          // OKC：定时器1频率为16MHZ
  CLKCONCMD &= ~(3<<4);
  T1_Capture_r_f_init();//所有沿触发
  IP1 |= (1<<1);              // 设置定时器1为第二高中断优先级
  IP0 |= (1<<1);
  IP1 |= (1<<3);              // 设置定时器3为第一高中断优先级
  IP0 |= (1<<3);
  //while(1);
}



#pragma vector = T1_VECTOR 
__interrupt void T1_ISR(void)
{
  EA=0;
  T1IF=0;
  if(T1STAT & (1<<2))//通道2中断标志
  {
    if(P0_4==1)
    {
      //EA=0;
     //** LED1=0;
    //  T1IF=0;
      T1STAT=0;
      P0IFG =0x00;//清除中断标志
     // EA=1;
     // InitUART();
    //**  UartSendString(start,5);
      count=0;
      T3init();//启动定时器3计时
    //  T4_int();
     //*** T1_Capture_fall_init();
    }
    if(P0_4==0)
    {
      
    //  EA=0;
    //  LED1=1;
    //  T1IF=0;
      T1STAT=0;
      P0IFG =0x00;//清除中断标志
     // memset(PM_data, 0, 6);
      count = count*2.048;
      count=(count-2)/2;//得到PM2.5的值  粗略拟合
     // count=0-0.0003*count*count+1.331*count-13.19;//拟合函数
      countsum = count + countsum;
      i++;
      if(i==5)
      {
        i=0;
        count_aver=countsum/5;
        sprintf(PM_data,"%.2f", count_aver);//将浮点数转成字符串
        pm_data=(int)count_aver*100;
       //** SampleApp_Send_P2P_Message();
        countsum=0;
        count_aver=0;
      }
      T3CTL |=0x04;//清除定时器3的计数器
      count=0;
    }
  }
  EA=1;
  T1STAT &= ~0x3f;             // 清空定时器中断状态位
}


//定时器T3中断处理函数
#pragma vector = T3_VECTOR 
__interrupt void T3_ISR(void) 
{ 
    T3IF = 0;            //清中断标志, 也可由硬件自动完成 
    count++;
}
