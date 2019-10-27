/**************************************
* 网孔法 区间直线法
* 获取TGS系列传感器的数据
**************************************/
#include "TGS813.h"

#if (defined Channel_0_For_TGS813) || (defined Channel_1_For_TGS813)   //必须宏定义了TGS813的通道才编译下面的程序

//记录figure2中温湿度数据
int TGS813GasSen_T[]={-10,0,10,20,30,40};
char TGS813GasSen_R[]={0,20,40,65,100};
unsigned char TGS813GasSen_Flag=0;
//Methane 甲烷在figure1中的交点坐标
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
//参数：RL电阻上的电压值 DHT11温湿度
//返还：浓度值，500-10000ppm之间
//注意：TGS813只能测量500-10000ppm浓度的甲烷
//	当浓度高于10000ppmm,TGS813GasSen_Flag的位0被置1;
//	当浓度低于500ppm,TGS813GasSen_Flag的位1被置1;
int TGS813GetConcentration(float RL_Vol,char DHT11_T,unsigned char DHT11_R)
{
	float Rs=0;				
	float RsR0_Ratio1=0;							//Rs与R0的比率1,比率1用于最终计算浓度结果
	int concentration=0;							//最后的浓度值
	char i=0;                                                               //循环用到
        TGS813GasSen_Flag = 0;                                                      //清空标志位

	RL_Vol += (float)TGS813GasSen_Wire_Compensation;			//补偿线路的压降,这个得测量导线的压降
	Rs = ((TGS813GasSen_Vc/RL_Vol)-1)*TGS813GasSen_RL;			//得出Rs的值
	RsR0_Ratio1 = Rs / TGS813GetCookedRo(DHT11_T,DHT11_R);		        //得到比率1,比率1是通过Rs与补偿后的R0的比值获得的。


//	printf("Rs:%f\n",Rs);							//输出Rs
//	printf("Ratio:%f\n",RsR0_Ratio1);					//输出比率
//	RsR0_Ratio1 = 1.38;						        //软件设定比率，为了测试

	//判断比率，要求比率对应的浓度值在传感器测量的范围里，若不在，置位标志位
	if(RsR0_Ratio1 > TGS813Fig1Data[0][1])
	{
		//目标气体在0-500ppm之间，不在测试范围返还0
		TGS813GasSen_Flag |= (1<<0);
		return 0;
	}
	else if(RsR0_Ratio1 < TGS813Fig1Data[TGS813NumOfFig1Coor-1][1])
	{
		//目标气体浓度大于10000ppm，不在测试范围，返还10000
		TGS813GasSen_Flag |= (1<<1);
		return (int)TGS813Fig1Data[TGS813NumOfFig1Coor-1][0];
	}
	//循环，判断比率所在的区间，得到的i值为区间（以坐标数组形式）下标
	for(i=0;i<TGS813NumOfFig1Coor;i++)				
	{	
		if((RsR0_Ratio1 <= TGS813Fig1Data[i][1]) && (RsR0_Ratio1 >TGS813Fig1Data[i+1][1]))	
		break;
	}
	//获得比率对应的浓度值
	concentration = (int)GetXFromTwoPoint(TGS813Fig1Data[i][0],TGS813Fig1Data[i][1],TGS813Fig1Data[i+1][0],TGS813Fig1Data[i+1][1],RsR0_Ratio1);	
	return concentration;
}



//参数：从DHT11获取的温湿度数据，分辨率为1，如果使用更加精准的温湿度传感器，需要改变输入参数的类型
//返还：补偿后的R0值
float TGS813GetCookedRo(char DHT11_T,unsigned char DHT11_R)
{
	char sub_T_Arr=0;
	char sub_R_Arr=0;		//记录温湿度数据在温度，湿度数组中的下标（即在数组中的位置）
	float RsR0_Ratio0=0;		//最终需要得到的结果
	float TwoCoordinate[][2]=	//用于记录求目标数据所在直线的两个点的坐标
	{
		{0,0},			//靠左的点
		{0,0},			//靠右的点
	};
        char TEMP_ARR[20];

	if(DHT11_T > TGS813GasSen_T[TGS813GasSen_T_Leghth-1])	//测得的温度值高于范围，置位标志位的位2
	{
          DHT11_T = TGS813GasSen_TEMP_MAX;  //设置温湿度为默认值
		TGS813GasSen_Flag |=(1<<2);
	}
	else if(DHT11_T < TGS813GasSen_T[0])			//测得的温度值低于范围，置位标志位的位3
	{
          DHT11_T = (char)TGS813GasSen_TEMP_MIN;   //设置温湿度为默认值
	  TGS813GasSen_Flag |=(1<<3);
	}

	if(DHT11_R > TGS813GasSen_R[TGS813GasSen_R_Leghth-1])	//测得的湿度值高于范围，置位标志位的位4
	{
           DHT11_R =  TGS813GasSen_HUMI_MAX;
	   TGS813GasSen_Flag |=(1<<4);
	}
	else if(DHT11_R < TGS813GasSen_R[0])			//测得的湿度值高于范围,置位标志位的位5
	{
          DHT11_R =  TGS813GasSen_HUMI_MIN;
	  TGS813GasSen_Flag |=(1<<5);
	}

	//循环，判断在哪个温度的区间，得到数组的下标
	for(sub_T_Arr=0; sub_T_Arr<TGS813GasSen_T_Leghth; sub_T_Arr++)				
	{	
		if((DHT11_T >= TGS813GasSen_T[sub_T_Arr]) && (DHT11_T <= TGS813GasSen_T[sub_T_Arr+1]))	
		break;
	}
	//循环，判断在哪个湿度的区间，得到数组的下标
	for(sub_R_Arr=0; sub_R_Arr<TGS813GasSen_R_Leghth; sub_R_Arr++)				
	{	
		if((DHT11_R >= TGS813GasSen_R[sub_R_Arr]) && (DHT11_R <= TGS813GasSen_R[sub_R_Arr+1]))	
		break;
	}
	
	//计算两点坐标
	TwoCoordinate[0][0] = TGS813Fig2Data[sub_R_Arr][sub_T_Arr][0];
	TwoCoordinate[1][0] = TGS813Fig2Data[sub_R_Arr][sub_T_Arr+1][0];
	TwoCoordinate[0][1] = GetYFromTwoPoint(TGS813GasSen_R[sub_R_Arr] , TGS813Fig2Data[sub_R_Arr][sub_T_Arr][1] , TGS813GasSen_R[sub_R_Arr+1] , TGS813Fig2Data[sub_R_Arr+1][sub_T_Arr][1],DHT11_R);
	TwoCoordinate[1][1] = GetYFromTwoPoint(TGS813GasSen_R[sub_R_Arr] , TGS813Fig2Data[sub_R_Arr][sub_T_Arr+1][1] , TGS813GasSen_R[sub_R_Arr+1] , TGS813Fig2Data[sub_R_Arr+1][sub_T_Arr+1][1],DHT11_R);
	
	//得到结果，Rs和R0的比值
	RsR0_Ratio0 = GetYFromTwoPoint(TwoCoordinate[0][0],TwoCoordinate[0][1],TwoCoordinate[1][0],TwoCoordinate[1][1],DHT11_T);

	return RsR0_Ratio0 * TGS813GasSen_Standard_R0;
}

//对于一条通过两点确定的直线，给定一个y值，返还x值
float GetXFromTwoPoint(float x1,float y1,float x2,float y2,float y)
{
	if(y1 == y2)			
	{
		if(x1 == x2)
		{
			return 0;		//如果输入的两点重合了，则返回0
		}				
		return y1;			//如果两点的y坐标相等，则返还y坐标
	}
	
	else
		return ((x2-x1)*(y-y1))/(y2-y1) + x1;
}
//对于一条通过两点确定的直线，给定一个x值，返还y值
float GetYFromTwoPoint(float x1,float y1,float x2,float y2,float x)
{
	if(x1 == x2)
	{
		if(y1 == y2)
		{
			return 0;		//如果输入的两点重合了，则返回0
		}	
		return x1;			//如果两点的x坐标相等，则返还x坐标
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