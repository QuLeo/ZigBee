/**************************************
* ���׷� ����ֱ�߷�
* ��ȡTGSϵ�д�����������
**************************************/
#include "TGS813.h"

#if (defined Channel_0_For_TGS813) || (defined Channel_1_For_TGS813)   //����궨����TGS813��ͨ���ű�������ĳ���

//��¼figure2����ʪ������
int TGS813GasSen_T[]={-10,0,10,20,30,40};
char TGS813GasSen_R[]={0,20,40,65,100};
unsigned char TGS813GasSen_Flag=0;
//Methane ������figure1�еĽ�������
float TGS813Fig1Data[][2]=
{
	{500,1.5f},
	{600,1.4f},
	{700,1.3f},
	{800,1.2f},
	{900,1.1f},
	{1000,1.0f},
	{1300,0.9f},
	{1700,0.8f},
	{2000,0.73f},
	{2100,0.7f},
	{3000,0.6f},
	{4000,0.53f},
	{4400,0.5f},
	{5000,0.47f},
	{6000,0.43f},
	{7000,0.41f},
	{8000,0.38f},
	{9000,0.36f},
	{10000,0.35f},
};

float TGS813Fig2Data[TGS813GasSen_R_Leghth][TGS813GasSen_T_Leghth][2]=
{
	//0% RH
	{
		{-10,1.87f},{0,1.79f},{10,1.74f},{20,1.68f},{30,1.65f},{40,1.61f},
	},
	//20% RH
	{
		{-10,1.73f},{0,1.52f},{10,1.35f},{20,1.21f},{30,1.12f},{40,1.07f},
	},
	//40% RH
	{
		{-10,1.68f},{0,1.45f},{10,1.25f},{20,1.1f},{30,0.98f},{40,0.92f},
	},
	//65% RH
	{
		{-10,1.61f},{0,1.35f},{10,1.15f},{20,1.0f},{30,0.9f},{40,0.86f},
	},
	//100% RH
	{
		{-10,1.55f},{0,1.3f},{10,1.1f},{20,0.96f},{30,0.86f},{40,0.83f},
	},
};
//������RL�����ϵĵ�ѹֵ DHT11��ʪ��
//������Ũ��ֵ��500-10000ppm֮��
//ע�⣺TGS813ֻ�ܲ���500-10000ppmŨ�ȵļ���
//	��Ũ�ȸ���10000ppmm,TGS813GasSen_Flag��λ0����1;
//	��Ũ�ȵ���500ppm,TGS813GasSen_Flag��λ1����1;
int TGS813GetConcentration(float RL_Vol,char DHT11_T,unsigned char DHT11_R)
{
	float Rs=0;				
	float RsR0_Ratio1=0;							//Rs��R0�ı���1,����1�������ռ���Ũ�Ƚ��
	int concentration=0;							//����Ũ��ֵ
	char i=0;                                                               //ѭ���õ�
        TGS813GasSen_Flag = 0;                                                      //��ձ�־λ

	RL_Vol += (float)TGS813GasSen_Wire_Compensation;			//������·��ѹ��,����ò������ߵ�ѹ��
	Rs = ((TGS813GasSen_Vc/RL_Vol)-1)*TGS813GasSen_RL;			//�ó�Rs��ֵ
	RsR0_Ratio1 = Rs / TGS813GetCookedRo(DHT11_T,DHT11_R);		        //�õ�����1,����1��ͨ��Rs�벹�����R0�ı�ֵ��õġ�


//	printf("Rs:%f\n",Rs);							//���Rs
//	printf("Ratio:%f\n",RsR0_Ratio1);					//�������
//	RsR0_Ratio1 = 1.38;						        //����趨���ʣ�Ϊ�˲���

	//�жϱ��ʣ�Ҫ����ʶ�Ӧ��Ũ��ֵ�ڴ����������ķ�Χ������ڣ���λ��־λ
	if(RsR0_Ratio1 > TGS813Fig1Data[0][1])
	{
		//Ŀ��������0-500ppm֮�䣬���ڲ��Է�Χ����0
		TGS813GasSen_Flag |= (1<<0);
		return 0;
	}
	else if(RsR0_Ratio1 < TGS813Fig1Data[TGS813NumOfFig1Coor-1][1])
	{
		//Ŀ������Ũ�ȴ���10000ppm�����ڲ��Է�Χ������10000
		TGS813GasSen_Flag |= (1<<1);
		return (int)TGS813Fig1Data[TGS813NumOfFig1Coor-1][0];
	}
	//ѭ�����жϱ������ڵ����䣬�õ���iֵΪ���䣨������������ʽ���±�
	for(i=0;i<TGS813NumOfFig1Coor;i++)				
	{	
		if((RsR0_Ratio1 <= TGS813Fig1Data[i][1]) && (RsR0_Ratio1 >TGS813Fig1Data[i+1][1]))	
		break;
	}
	//��ñ��ʶ�Ӧ��Ũ��ֵ
	concentration = (int)GetXFromTwoPoint(TGS813Fig1Data[i][0],TGS813Fig1Data[i][1],TGS813Fig1Data[i+1][0],TGS813Fig1Data[i+1][1],RsR0_Ratio1);	
	return concentration;
}



//��������DHT11��ȡ����ʪ�����ݣ��ֱ���Ϊ1�����ʹ�ø��Ӿ�׼����ʪ�ȴ���������Ҫ�ı��������������
//�������������R0ֵ
float TGS813GetCookedRo(char DHT11_T,unsigned char DHT11_R)
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

	if(DHT11_T > TGS813GasSen_T[TGS813GasSen_T_Leghth-1])	//��õ��¶�ֵ���ڷ�Χ����λ��־λ��λ2
	{
          DHT11_T = TGS813GasSen_TEMP_MAX;  //������ʪ��ΪĬ��ֵ
		TGS813GasSen_Flag |=(1<<2);
	}
	else if(DHT11_T < TGS813GasSen_T[0])			//��õ��¶�ֵ���ڷ�Χ����λ��־λ��λ3
	{
          DHT11_T = (char)TGS813GasSen_TEMP_MIN;   //������ʪ��ΪĬ��ֵ
	  TGS813GasSen_Flag |=(1<<3);
	}

	if(DHT11_R > TGS813GasSen_R[TGS813GasSen_R_Leghth-1])	//��õ�ʪ��ֵ���ڷ�Χ����λ��־λ��λ4
	{
           DHT11_R =  TGS813GasSen_HUMI_MAX;
	   TGS813GasSen_Flag |=(1<<4);
	}
	else if(DHT11_R < TGS813GasSen_R[0])			//��õ�ʪ��ֵ���ڷ�Χ,��λ��־λ��λ5
	{
          DHT11_R =  TGS813GasSen_HUMI_MIN;
	  TGS813GasSen_Flag |=(1<<5);
	}

	//ѭ�����ж����ĸ��¶ȵ����䣬�õ�������±�
	for(sub_T_Arr=0; sub_T_Arr<TGS813GasSen_T_Leghth; sub_T_Arr++)				
	{	
		if((DHT11_T >= TGS813GasSen_T[sub_T_Arr]) && (DHT11_T <= TGS813GasSen_T[sub_T_Arr+1]))	
		break;
	}
	//ѭ�����ж����ĸ�ʪ�ȵ����䣬�õ�������±�
	for(sub_R_Arr=0; sub_R_Arr<TGS813GasSen_R_Leghth; sub_R_Arr++)				
	{	
		if((DHT11_R >= TGS813GasSen_R[sub_R_Arr]) && (DHT11_R <= TGS813GasSen_R[sub_R_Arr+1]))	
		break;
	}
	
	//������������
	TwoCoordinate[0][0] = TGS813Fig2Data[sub_R_Arr][sub_T_Arr][0];
	TwoCoordinate[1][0] = TGS813Fig2Data[sub_R_Arr][sub_T_Arr+1][0];
	TwoCoordinate[0][1] = GetYFromTwoPoint(TGS813GasSen_R[sub_R_Arr] , TGS813Fig2Data[sub_R_Arr][sub_T_Arr][1] , TGS813GasSen_R[sub_R_Arr+1] , TGS813Fig2Data[sub_R_Arr+1][sub_T_Arr][1],DHT11_R);
	TwoCoordinate[1][1] = GetYFromTwoPoint(TGS813GasSen_R[sub_R_Arr] , TGS813Fig2Data[sub_R_Arr][sub_T_Arr+1][1] , TGS813GasSen_R[sub_R_Arr+1] , TGS813Fig2Data[sub_R_Arr+1][sub_T_Arr+1][1],DHT11_R);
	
	//�õ������Rs��R0�ı�ֵ
	RsR0_Ratio0 = GetYFromTwoPoint(TwoCoordinate[0][0],TwoCoordinate[0][1],TwoCoordinate[1][0],TwoCoordinate[1][1],DHT11_T);

	return RsR0_Ratio0 * TGS813GasSen_Standard_R0;
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
	printf("Gas concertation:%dppm\r\n",TGS813GetConcentration(0.16129f,20,50));
}*/
#endif