/**************************************
* ���׷� ����ֱ�߷�
* ��ȡTGSϵ�д�����������
**************************************/
#include "TGS822.h"

#if (defined Channel_0_For_TGS822) || (defined Channel_1_For_TGS822)   //����궨����TGS822��ͨ���ű�������ĳ���

//��¼figure2����ʪ������
int TGS822GasSen_T[]={-10,0,10,20,30,40};
char TGS822GasSen_R[]={35,50,65,100};
unsigned char TGS822GasSen_Flag=0;
//Methane ������figure1�еĽ�������
float TGS822Fig1Data[][2]=
{
	{50,2.84f},
	{60,2.64f},
	{70,2.39f},
	{80,2.22f},
	{90,2.05f},
	{93,2.0f},
	{100,1.94f},
	{200,1.35f},
	{300,1.0f},
	{400,0.9f},
	{458,0.8f},
	{500,0.74f},
	{530,0.7f},
	{600,0.64f},
	{671,0.6f},
	{700,0.59f},
	{800,0.53f},
	{867,0.5f},
	{900,0.49f},
	{1000,0.46f},
	{1290,0.4f},
	{1896,0.3f},
	{2000,0.29f},
	{3000,0.22f},
	{3309,0.2f},
	{4000,0.18f},
	{5000,0.16f},
};

float TGS822Fig2Data[TGS822GasSen_R_Leghth][TGS822GasSen_T_Leghth][2]=
{
	//35% RH
	{
		{-10,1.69f},{0,1.69f},{10,1.69f},{20,1.34f},{30,1.09f},{40,0.88f},
	},
	//50% RH
	{
		{-10,1.53f},{0,1.53f},{10,1.53f},{20,1.17f},{30,0.92f},{40,0.76f},
	},

	//65% RH
	{
		{-10,1.81f},{0,1.81f},{10,1.38f},{20,1.01f},{30,0.83f},{40,0.68f},
	},
	//100% RH
	{
		{-10,2.28f},{0,1.59f},{10,1.1f},{20,0.87f},{30,0.72f},{40,0.61f},
	},
};
//������RL�����ϵĵ�ѹֵ DHT11��ʪ��
//������Ũ��ֵ��50-5000ppm֮��
//ע�⣺TGS822ֻ�ܲ���50-5000ppmŨ�ȵľƾ�
//	��Ũ�ȸ���5000ppmm,TGS822GasSen_Flag��λ0����1;
//	��Ũ�ȵ���50ppm,TGS822GasSen_Flag��λ1����1;
int TGS822GetConcentration(float RL_Vol,char DHT11_T,unsigned char DHT11_R)
{
	float Rs=0;				
	float RsR0_Ratio1=0;							//Rs��R0�ı���1,����1�������ռ���Ũ�Ƚ��
	int concentration=0;							//����Ũ��ֵ
	char i=0;                                                               //ѭ���õ�

        TGS822GasSen_Flag = 0;                                                  //��ձ�־λ
	
	RL_Vol += (float)TGS822GasSen_Wire_Compensation;			//������·��ѹ��
	Rs = ((TGS822GasSen_Vc/RL_Vol)-1)*TGS822GasSen_RL;			//�ó�Rs��ֵ
	RsR0_Ratio1 = Rs / TGS822GetCookedRo(DHT11_T,DHT11_R);		        //�õ�����1,����1��ͨ��Rs�벹�����R0�ı�ֵ��õġ�

//	printf("Rs:%f\n",Rs);							//���Rs
//	printf("Ratio:%f\n",RsR0_Ratio1);					//�������
//	RsR0_Ratio1 = 0.18;						        //����趨���ʣ�Ϊ�˲���

	//�жϱ��ʣ�Ҫ����ʶ�Ӧ��Ũ��ֵ�ڴ����������ķ�Χ������ڣ���λ��־λ
	if(RsR0_Ratio1 > TGS822Fig1Data[0][1])
	{
		//Ŀ��������0-500ppm֮�䣬���ڲ��Է�Χ����0
		TGS822GasSen_Flag |= (1<<0);
		return 0;
	}
	else if(RsR0_Ratio1 < TGS822Fig1Data[TGS822NumOfFig1Coor-1][1])
	{
		//Ŀ������Ũ�ȴ���10000ppm�����ڲ��Է�Χ������10000
		TGS822GasSen_Flag |= (1<<1);
		return (int)TGS822Fig1Data[TGS822NumOfFig1Coor-1][0];
	}
	//ѭ�����жϱ������ڵ����䣬�õ���iֵΪ���䣨������������ʽ���±�
	for(i=0;i<TGS822NumOfFig1Coor;i++)				
	{	
		if((RsR0_Ratio1 <= TGS822Fig1Data[i][1]) && (RsR0_Ratio1 >TGS822Fig1Data[i+1][1]))	
		break;
	}
	//��ñ��ʶ�Ӧ��Ũ��ֵ
	concentration = (int)GetXFromTwoPoint(TGS822Fig1Data[i][0],TGS822Fig1Data[i][1],TGS822Fig1Data[i+1][0],TGS822Fig1Data[i+1][1],RsR0_Ratio1);	
	return concentration;
}



//��������DHT11��ȡ����ʪ�����ݣ��ֱ���Ϊ1�����ʹ�ø��Ӿ�׼����ʪ�ȴ���������Ҫ�ı��������������
//�������������R0ֵ
float TGS822GetCookedRo(char DHT11_T,unsigned char DHT11_R)
{
	char sub_T_Arr=0;
	char sub_R_Arr=0;		//��¼��ʪ���������¶ȣ�ʪ�������е��±꣨���������е�λ�ã�
	float RsR0_Ratio0=0;		//������Ҫ�õ��Ľ��
	float TwoCoordinate[][2]=	//���ڼ�¼��Ŀ����������ֱ�ߵ������������
	{
		{0,0},			//����ĵ�
		{0,0},			//���ҵĵ�
	};
        char TEMP_ARR[20];

	if(DHT11_T > TGS822GasSen_T[TGS822GasSen_T_Leghth-1])	//��õ��¶�ֵ���ڷ�Χ����λ��־λ��λ2
	{
          DHT11_T = TGS822GasSen_TEMP_MAX;  //������ʪ��ΪĬ��ֵ
		TGS822GasSen_Flag |=(1<<2);
	}
	else if(DHT11_T < TGS822GasSen_T[0])			//��õ��¶�ֵ���ڷ�Χ����λ��־λ��λ3
	{
          DHT11_T = (char)TGS822GasSen_TEMP_MIN;   //������ʪ��ΪĬ��ֵ
	  TGS822GasSen_Flag |=(1<<3);
	}

	if(DHT11_R > TGS822GasSen_R[TGS822GasSen_R_Leghth-1])	//��õ�ʪ��ֵ���ڷ�Χ����λ��־λ��λ4
	{
           DHT11_R =  TGS822GasSen_HUMI_MAX;
	   TGS822GasSen_Flag |=(1<<4);
	}
	else if(DHT11_R < TGS822GasSen_R[0])			//��õ�ʪ��ֵ���ڷ�Χ,��λ��־λ��λ5
	{
          DHT11_R =  TGS822GasSen_HUMI_MIN;
	  TGS822GasSen_Flag |=(1<<5);
	}

	//ѭ�����ж����ĸ��¶ȵ����䣬�õ�������±�
	for(sub_T_Arr=0; sub_T_Arr<TGS822GasSen_T_Leghth; sub_T_Arr++)				
	{	
		if((DHT11_T >= TGS822GasSen_T[sub_T_Arr]) && (DHT11_T <= TGS822GasSen_T[sub_T_Arr+1]))	
		break;
	}
	//ѭ�����ж����ĸ�ʪ�ȵ����䣬�õ�������±�
	for(sub_R_Arr=0; sub_R_Arr<TGS822GasSen_R_Leghth; sub_R_Arr++)				
	{	
		if((DHT11_R >= TGS822GasSen_R[sub_R_Arr]) && (DHT11_R <= TGS822GasSen_R[sub_R_Arr+1]))	
		break;
	}
	
	//������������
	TwoCoordinate[0][0] = TGS822Fig2Data[sub_R_Arr][sub_T_Arr][0];
	TwoCoordinate[1][0] = TGS822Fig2Data[sub_R_Arr][sub_T_Arr+1][0];
	TwoCoordinate[0][1] = GetYFromTwoPoint(TGS822GasSen_R[sub_R_Arr] , TGS822Fig2Data[sub_R_Arr][sub_T_Arr][1] , TGS822GasSen_R[sub_R_Arr+1] , TGS822Fig2Data[sub_R_Arr+1][sub_T_Arr][1],DHT11_R);
	TwoCoordinate[1][1] = GetYFromTwoPoint(TGS822GasSen_R[sub_R_Arr] , TGS822Fig2Data[sub_R_Arr][sub_T_Arr+1][1] , TGS822GasSen_R[sub_R_Arr+1] , TGS822Fig2Data[sub_R_Arr+1][sub_T_Arr+1][1],DHT11_R);
	
	//�õ������Rs��R0�ı�ֵ
	RsR0_Ratio0 = GetYFromTwoPoint(TwoCoordinate[0][0],TwoCoordinate[0][1],TwoCoordinate[1][0],TwoCoordinate[1][1],DHT11_T);

	return RsR0_Ratio0 * TGS822GasSen_Standard_R0;
}

//����һ��ͨ������ȷ����ֱ�ߣ�����һ��yֵ������xֵ
float GetXFromTwoPoint(float x1,float y1,float x2,float y2,float y)
{
	if(y1 == y2)			
	{
		if(x1 == x2)
		{
			return 0;		//�������������غ��ˣ��򷵻�0
		}				
		return y1;			//��������y������ȣ��򷵻�y����
	}
	
	else
		return ((x2-x1)*(y-y1))/(y2-y1) + x1;
}
//����һ��ͨ������ȷ����ֱ�ߣ�����һ��xֵ������yֵ
float GetYFromTwoPoint(float x1,float y1,float x2,float y2,float x)
{
	if(x1 == x2)
	{
		if(y1 == y2)
		{
			return 0;		//�������������غ��ˣ��򷵻�0
		}	
		return x1;			//��������x������ȣ��򷵻�x����
	}
	else
		return ((y2-y1)*(x-x1))/(x2-x1) + y1;
}
/* For Test in VC++
void main()
{
	printf("Gas concertation:%dppm\r\n",TGS822GetConcentration(0.16129f,30,80));
}*/
#endif