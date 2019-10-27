#ifndef _GAS_SENSOR_TGS822_H
#define _GAS_SENSOR_TGS822_H

#if (defined Channel_0_For_TGS822) || (defined Channel_1_For_TGS822)   //����궨����TGS813����822��ͨ���ű�������ĳ���
#include <stdio.h>
#include <string.h>

#define TGS822GasSen_RL 1				//���ص���Ϊ1K
#define TGS822GasSen_Vc 4.88				//��Դ��ѹΪ5V
#define TGS822GasSen_Standard_R0 1.7			//��׼����£�300ppm�Ҵ���25�㣬65%RH�´���������ֵ��
#define TGS822GasSen_Wire_Compensation (-0.01)          //���������������ߵĲ���ѹ������Ϊ����������˿�����ܴ��������ϲ����˲��ܱ����Ե�ѹ�������ﵥλ��v
							//�������Ҳ��ͨ����������˿���裬�������ߵĵ�����
//�궨�����崫��������ʪ�ȷ�Χ
#define TGS822GasSen_TEMP_MAX 40
#define TGS822GasSen_TEMP_MIN (-10)
#define TGS822GasSen_HUMI_MAX 100
#define TGS822GasSen_HUMI_MIN 35

#define TGS822NumOfFig1Coor sizeof(TGS822Fig1Data)/sizeof(TGS822Fig1Data[0]) //Figure1����������
#define TGS822GasSen_T_Leghth (sizeof(TGS822GasSen_T)/sizeof(TGS822GasSen_T[0]))
#define TGS822GasSen_R_Leghth (sizeof(TGS822GasSen_R)/sizeof(TGS822GasSen_R[0]))

//λ0��������Ũ����0-50ppmʱ��1
//λ1��������Ũ�ȴ���5000ppmʱ��1
//λ2: ��ʪ�ȴ�������õ��¶ȸ��ڷ�Χ
//λ3����ʪ�ȴ�������õ��¶ȵ��ڷ�Χ
//λ4����ʪ�ȴ�������õ�ʪ�ȸ��ڷ�Χ
//λ5����ʪ�ȴ�������õ�ʪ�ȵ��ڷ�Χ
extern unsigned char TGS822GasSen_Flag;                                                                //GasSenor�ı�־λ��ÿ�ε��û�ȡŨ��ֵ����ʱ����־λ�����㣬Ȼ��鿴������״̬������Ӧ��λ

extern int TGS822GetConcentration(float RL_Vol,char DHT11_T,unsigned char DHT11_R);	//�ⲿ���õĺ���������������RL�ĵ�ѹ��ADC�ķ���ֵ�������������ص��¶�ֵ�����������ص�ʪ��ֵ����Ũ��ֵ
static float TGS822GetCookedRo(char DHT11_T,unsigned char DHT11_R);			        //�ڲ����������ظ���fig2�������R0ֵ
static float GetXFromTwoPoint(float x1,float y1,float x2,float y2,float y);		//����һ��ͨ������ȷ����ֱ�ߣ�����һ��yֵ������xֵ
static float GetYFromTwoPoint(float x1,float y1,float x2,float y2,float x);		//����һ��ͨ������ȷ����ֱ�ߣ�����һ��xֵ������yֵ


#endif
#endif