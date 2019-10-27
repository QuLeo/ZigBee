#include <CO.h>//也可以用于测量HCL气体模块输出的模拟量

float ValreadP0_0()//P0_0作为测量电压的引脚
{
  int i;
  int read_value;
  int read_sum=0;
  float read_aver;
  float val_aver;
  for(i=0;i<10;i++)
  {
    P0DIR &=~(1<<0);//P0_0设置为输入
    P0SEL |=(1<<0);//P0_0复用为外设功能
    ADCCON3=0xb0;//参考电压为AVDD5引脚电压，512抽取率，通道为AIN0
    ADCCON1 |=0x40;//开始转换
    while(!(ADCCON1 & 0x80));     //等待 AD 转换完成 
    read_value =  ADCL >> 4;           //ADCL 寄存器低 2 位无效，由于他只有12位有效(最高位是符号位），ADCL寄存器低4位无效。网络上很多代码这里都是右移两位，那是不对的
    read_value |= (((uint)ADCH) << 4);
    if(read_value&(1<<11))  //判断第十二位是否为1
    {
      read_value&=(0x07FF);   //清符号位
      read_value-=1;       //补码减一
      read_value^=(0x07FF);   //低十一位取反
      read_value=(-1)*read_value;   //真实返回值
    }
    read_sum = read_sum + read_value;
  }
  read_aver = ((float)read_sum)/10;
  val_aver = read_aver*3.3/2048;
  return val_aver;
}

float ValreadP0_4()//P0_4作为测量电压的引脚
{
  int i;
  int read_value;
  int read_sum=0;
  float read_aver;
  float val_aver;
    for(i=0;i<10;i++)
    {
      P0DIR &=~(1<<4);//P0_4设置为输入
      P0SEL |=(1<<4);//P0_4复用为外设功能
      ADCCON3=0xb4;//参考电压为AVDD5引脚电压，512抽取率，通道为AIN4
      ADCCON1 |=0x40;//开始转换
      while(!(ADCCON1 & 0x80));     //等待 AD 转换完成 
      read_value =  ADCL >> 4;      //ADCL 寄存器低 2 位无效，由于他只有12位有效(最高位是符号位），ADCL寄存器低4位无效。网络上很多代码这里都是右移两位，那是不对的
      read_value |= (((uint)ADCH) << 4);
      if(read_value&(1<<11))  //判断第十二位是否为1
      {
        read_value&=(0x07FF);   //清符号位
        read_value-=1;       //补码减一
        read_value^=(0x07FF);   //低十一位取反
        read_value=(-1)*read_value;   //真实返回值
      }
      read_sum = read_sum + read_value; 
    }
    read_aver = ((float)read_sum)/10;
  val_aver = read_aver*3.3/2048;
  return val_aver;
}