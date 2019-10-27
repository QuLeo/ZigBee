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
  T3CTL |= 0x08 ;          //������ж�     
  T3IE = 1;                //�����жϺ�T3�ж�
  T3CTL |= 0xE0;           //128��Ƶ,128/16000000*N=0.5S,N=62500
  T3CTL &= ~0x03;          //�Զ���װ 00��>0xff  62500/255=245(��)
  T3CTL |= 0x10;           //����
  EA = 1;                  //�����ж�,һ���жϼ�ʱ0.002048��
}
void InitClock(void)
{   
    CLKCONCMD &= ~0x40;              //����ϵͳʱ��ԴΪ 32MHZ����
    while(CLKCONSTA & 0x40);         //�ȴ������ȶ� 
    CLKCONCMD &= ~0x47;              //����ϵͳ��ʱ��Ƶ��Ϊ 32MHZ
}


/****************************************************************************
* ��    ��: InitUart()
* ��    ��: ���ڳ�ʼ������
* ��ڲ���: ��
* ���ڲ���: ��
****************************************************************************/
void InitUART(void)
{
    /*PERCFG = 0x00;                   //λ��1 P0��
    P0SEL = 0x3C;                    //P0��������
    
    P2DIR &= ~0xC0;                  //P0������ΪUART0    
    U0CSR |= 0x80;                   //��������ΪUART��ʽ
    U0GCR |= 11;				
    U0BAUD |= 216;                   //��������Ϊ115200
    
    UTX0IF = 1;                      //UART0 TX�жϱ�־��ʼ��λ1  
    U0CSR |= 0x40;                   //�������
    IEN0 |= 0x84;                    //�����жϣ������ж�*/
    
    //**PERCFG &= 0xfe;           //������ƼĴ��� USART 0��IOλ��:0ΪP0��λ��1 
    PERCFG =0xbe;//��ʱ��1����λ��1����ʱ��1����λ��1,UART0����λ��1
    P0SEL |= 0x0c;            //P0_2,P0_3�������ڣ����蹦�ܣ�
    P2DIR &= ~0xC0;          //P0������ΪUART0
    
    U0CSR |= 0x80;           //����ΪUART��ʽ
    U0GCR |= 11;				       
    U0BAUD |= 216;           //��������Ϊ115200
    UTX0IF = 0;              //UART0 TX�жϱ�־��ʼ��λ0
    U0CSR |= 0x40;           //������� 
    IEN0 |= 0x84;            //�����ж���������ж� 
}

void Init_capture_Gpio()
{
  P0SEL |= (1<<4);//P0_4���ù���
  P0DIR &=~(1<<4);//����GPIOΪ����
 // P0_4=0;
  //P2INP |=0x20;//P0�˿�ȫ����������
 // P0INP=0xff;//P0�˿�ȫ��������̬
  //**  PERCFG &=0xbf;//��ʱ��1����λ��1
  PERCFG =0xbe;//��ʱ��1����λ��1,UART0����λ��1
  //P2DIR |=0xc0;//��ʱ��1ͨ��2����  *****
  P2DIR &= ~0xC0;          //P0������ΪUART0  ****
}

void T1_init3()//���ö�ʱ��1Ϊ�����ش���
{
//  T1CTL=0x0d;
  T1CCTL2 = 0x43;//���жϣ������������ز���
  //T1CCTL2 = 0x41;//���жϣ���������������
 // PICTL &=0xfe;//P0_0-P0_7�����ش���
 // PICTL=0xfe;//P0_0-P0_7�����ش���
 // PICTL |=0x01;//P0_0-P0_7�½��ش���
  IRCON=0;//����жϱ�־
  P0IFG =0x00;//����жϱ�־

  T1IE=1;//ʹ�ܶ�ʱ��1�ж�  
  EA=1;
}

void T1_Capture_r_f_init()
{
  Init_capture_Gpio();
  T1_init3();
}


void PM2_5Init()
{ 
 // InitClock(); //ʱ����Ƶ������Ϊ32M
  //InitUART();//��ʼ�����ڣ����Է�������
  CLKCONCMD |= (1<<3);          // OKC����ʱ��1Ƶ��Ϊ16MHZ
  CLKCONCMD &= ~(3<<4);
  T1_Capture_r_f_init();//�����ش���
  IP1 |= (1<<1);              // ���ö�ʱ��1Ϊ�ڶ����ж����ȼ�
  IP0 |= (1<<1);
  IP1 |= (1<<3);              // ���ö�ʱ��3Ϊ��һ���ж����ȼ�
  IP0 |= (1<<3);
  //while(1);
}



#pragma vector = T1_VECTOR 
__interrupt void T1_ISR(void)
{
  EA=0;
  T1IF=0;
  if(T1STAT & (1<<2))//ͨ��2�жϱ�־
  {
    if(P0_4==1)
    {
      //EA=0;
     //** LED1=0;
    //  T1IF=0;
      T1STAT=0;
      P0IFG =0x00;//����жϱ�־
     // EA=1;
     // InitUART();
    //**  UartSendString(start,5);
      count=0;
      T3init();//������ʱ��3��ʱ
    //  T4_int();
     //*** T1_Capture_fall_init();
    }
    if(P0_4==0)
    {
      
    //  EA=0;
    //  LED1=1;
    //  T1IF=0;
      T1STAT=0;
      P0IFG =0x00;//����жϱ�־
     // memset(PM_data, 0, 6);
      count = count*2.048;
      count=(count-2)/2;//�õ�PM2.5��ֵ  �������
     // count=0-0.0003*count*count+1.331*count-13.19;//��Ϻ���
      countsum = count + countsum;
      i++;
      if(i==5)
      {
        i=0;
        count_aver=countsum/5;
        sprintf(PM_data,"%.2f", count_aver);//��������ת���ַ���
        pm_data=(int)count_aver*100;
       //** SampleApp_Send_P2P_Message();
        countsum=0;
        count_aver=0;
      }
      T3CTL |=0x04;//�����ʱ��3�ļ�����
      count=0;
    }
  }
  EA=1;
  T1STAT &= ~0x3f;             // ��ն�ʱ���ж�״̬λ
}


//��ʱ��T3�жϴ�����
#pragma vector = T3_VECTOR 
__interrupt void T3_ISR(void) 
{ 
    T3IF = 0;            //���жϱ�־, Ҳ����Ӳ���Զ���� 
    count++;
}
