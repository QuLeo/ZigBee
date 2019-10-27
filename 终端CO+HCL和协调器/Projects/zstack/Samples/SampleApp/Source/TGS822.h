#ifndef _GAS_SENSOR_TGS822_H
#define _GAS_SENSOR_TGS822_H

#if (defined Channel_0_For_TGS822) || (defined Channel_1_For_TGS822)   //必须宏定义了TGS813或者822的通道才编译下面的程序
#include <stdio.h>
#include <string.h>

#define TGS822GasSen_RL 1				//负载电阻为1K
#define TGS822GasSen_Vc 4.88				//电源电压为5V
#define TGS822GasSen_Standard_R0 1.7			//标准情况下（300ppm乙醇，25°，65%RH下传感器电阻值）
#define TGS822GasSen_Wire_Compensation (-0.01)          //传感器与主板连线的补偿压降，因为传感器加热丝电流很大，在连线上产生了不能被忽略的压降，这里单位是v
							//这个参数也能通过测量发热丝电阻，测量导线的电阻获得
//宏定义气体传感器的温湿度范围
#define TGS822GasSen_TEMP_MAX 40
#define TGS822GasSen_TEMP_MIN (-10)
#define TGS822GasSen_HUMI_MAX 100
#define TGS822GasSen_HUMI_MIN 35

#define TGS822NumOfFig1Coor sizeof(TGS822Fig1Data)/sizeof(TGS822Fig1Data[0]) //Figure1的坐标数量
#define TGS822GasSen_T_Leghth (sizeof(TGS822GasSen_T)/sizeof(TGS822GasSen_T[0]))
#define TGS822GasSen_R_Leghth (sizeof(TGS822GasSen_R)/sizeof(TGS822GasSen_R[0]))

//位0：当气体浓度在0-50ppm时置1
//位1：当气体浓度大于5000ppm时置1
//位2: 温湿度传感器测得的温度高于范围
//位3：温湿度传感器测得的温度低于范围
//位4：温湿度传感器测得的湿度高于范围
//位5：温湿度传感器测得的湿度低于范围
extern unsigned char TGS822GasSen_Flag;                                                                //GasSenor的标志位，每次调用获取浓度值函数时，标志位表清零，然后查看传感器状态并置相应的位

extern int TGS822GetConcentration(float RL_Vol,char DHT11_T,unsigned char DHT11_R);	//外部调用的函数，参数依次是RL的电压（ADC的返回值），传感器返回的温度值，传感器返回的湿度值返回浓度值
static float TGS822GetCookedRo(char DHT11_T,unsigned char DHT11_R);			        //内部函数，返回根据fig2补偿后的R0值
static float GetXFromTwoPoint(float x1,float y1,float x2,float y2,float y);		//对于一条通过两点确定的直线，给定一个y值，返还x值
static float GetYFromTwoPoint(float x1,float y1,float x2,float y2,float x);		//对于一条通过两点确定的直线，给定一个x值，返还y值


#endif
#endif