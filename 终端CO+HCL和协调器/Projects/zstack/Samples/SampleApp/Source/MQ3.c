#include <MQ3.h>//Ҳ�������ڲ���HCL����ģ�������ģ����

float ValreadP0_5()//P0_5��Ϊ������ѹ������
{
  int i;
  int read_value;
  int read_sum=0;
  float read_aver;
  float val_aver;
  for(i=0;i<10;i++)
  {
    P0DIR &=~(1<<5);//P0_5����Ϊ����
    P0SEL |=(1<<5);//P0_5����Ϊ���蹦��
    ADCCON3=0xb5;//�ο���ѹΪAVDD5���ŵ�ѹ��512��ȡ�ʣ�ͨ��ΪAIN5,����Ҫ��Ϊ5V�ο���ѹ
    ADCCON1 |=0x40;//��ʼת��
    while(!(ADCCON1 & 0x80));     //�ȴ� AD ת����� 
    read_value =  ADCL >> 4;           //ADCL �Ĵ����� 2 λ��Ч��������ֻ��12λ��Ч(���λ�Ƿ���λ����ADCL�Ĵ�����4λ��Ч�������Ϻܶ�������ﶼ��������λ�����ǲ��Ե�
    read_value |= (((uint)ADCH) << 4);
    if(read_value&(1<<11))  //�жϵ�ʮ��λ�Ƿ�Ϊ1
    {
      read_value&=(0x07FF);   //�����λ
      read_value-=1;       //�����һ
      read_value^=(0x07FF);   //��ʮһλȡ��
      read_value=(-1)*read_value;   //��ʵ����ֵ
    }
    read_sum = read_sum + read_value;
  }
  read_aver = ((float)read_sum)/10;
  val_aver = read_aver*3.3/2048;
  return val_aver;
}