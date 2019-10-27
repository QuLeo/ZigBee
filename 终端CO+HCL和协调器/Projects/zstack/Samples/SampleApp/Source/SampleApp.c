/**************************************************************************************************
  Filename:       SampleApp.c
  Revised:        $Date: 2009-03-18 15:56:27 -0700 (Wed, 18 Mar 2009) $
  Revision:       $Revision: 19453 $

  Description:    Sample Application (no Profile).


  Copyright 2007 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
  This application isn't intended to do anything useful, it is
  intended to be a simple example of an application's structure.

  This application sends it's messages either as broadcast or
  broadcast filtered group messages.  The other (more normal)
  message addressing is unicast.  Most of the other sample
  applications are written to support the unicast message model.

  Key control:
    SW1:  Sends a flash command to all devices in Group 1.
    SW2:  Adds/Removes (toggles) this device in and out
          of Group 1.  This will enable and disable the
          reception of the flash command.
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "OSAL.h"
#include "ZGlobals.h"
#include "AF.h"
#include "aps_groups.h"
#include "ZDApp.h"

#include "SampleApp.h"
#include "SampleAppHw.h"

#include "OnBoard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_adc.h"
//此处用于串口
#include  "MT_UART.h"
//温湿度传感器
#include "dht11.h"
//C语言标准库
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//GasSensor 获取数据的头文件
#include "MQ3.h"
#include "TGS822.h"

//CO
#include <CO.h>

/*********************************************************************
 * MACROS
 */


#define AMS1117_3_ADC_VOL     3.28   //宏定义AMS1117 3.3 ADC参考电压，该值可能在3.2-3.4浮动
//如果定义了channel0或者channel1给气体传感器，则
#if (defined Channel_0_For_TGS813) || (defined Channel_0_For_TGS822)
#define Channel_0_GasSensor_Busy
#endif
#if (defined Channel_1_For_TGS813) || (defined Channel_1_For_TGS822)
#define Channel_1_GasSensor_Busy
#endif
//如果多个气体传感器同时占用同一个channel，则提示
#if (defined Channel_0_For_TGS813) && (defined Channel_0_For_TGS822)
#error "Channel0 is just for one gas sensor.Check the micro!"
#endif
#if (defined Channel_1_For_TGS813) && (defined Channel_1_For_TGS822)
#error "Channel1 is just for one gas sensor.Check the micro!"

#endif
#if (!defined Channel_0_GasSensor_Busy) && (!defined Channel_1_GasSensor_Busy)
#warning "Do you want to use the Gas Sensor? if yes,please define it,if not please ignore this message."
#endif

// 设置不同传感器电压阈值
// TGS813
#define TGS813_Safe 0.8
#define TGS813_Mildly 1.2
#define TGS813_Moderate 1.8
#define TGS813_Severe 2.3
// TGS822
#define TGS822_Safe 0.8
#define TGS822_Mildly 1.2
#define TGS822_Moderate 1.8
#define TGS822_Severe 2.3


char contin=0;
char contin1=0;
extern uint8 PM_data[6];
extern unsigned short pm_data;
uint8 send_data_PM25[8];
uint8 send_data_CO[7];
uint8 send_data_HCL[6];
uint8 send_data_MQ3[6];
uint8  temp_f;
uint8  humi_f;
char flag=0,flag1=0,flag_co=0,flag_hcl=0,flag_aol=0,flag_tgs=0,flag_ch=0,flag_ta=0;
float p=0;

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// This list should be filled with Application specific Cluster IDs.
const cId_t SampleApp_ClusterList[SAMPLEAPP_MAX_CLUSTERS] =
{
  SAMPLEAPP_PERIODIC_CLUSTERID,
  SAMPLEAPP_FLASH_CLUSTERID
};

const SimpleDescriptionFormat_t SampleApp_SimpleDesc =
{
  SAMPLEAPP_ENDPOINT,              //  int Endpoint;
  SAMPLEAPP_PROFID,                //  uint16 AppProfId[2];
  SAMPLEAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  SAMPLEAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  SAMPLEAPP_FLAGS,                 //  int   AppFlags:4;
  SAMPLEAPP_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
  (cId_t *)SampleApp_ClusterList,  //  uint8 *pAppInClusterList;
  SAMPLEAPP_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
  (cId_t *)SampleApp_ClusterList   //  uint8 *pAppInClusterList;
};

// This is the Endpoint/Interface description.  It is defined here, but
// filled-in in SampleApp_Init().  Another way to go would be to fill
// in the structure here and make it a "const" (in code space).  The
// way it's defined in this sample app it is define in RAM.
endPointDesc_t SampleApp_epDesc;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
uint8 SampleApp_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // SampleApp_Init() is called.
devStates_t SampleApp_NwkState;

uint8 SampleApp_TransID;  // This is the unique message ID (counter)

afAddrType_t SampleApp_Periodic_DstAddr;

afAddrType_t SampleApp_Flash_DstAddr;//组播

afAddrType_t SampleApp_Point_To_Point_DstAddr;

aps_Group_t SampleApp_Group;//分组内容

uint8 SampleAppPeriodicCounter = 0;
uint8 SampleAppFlashCounter = 0;


//@huang
//设置channel0和channel1获取气体浓度指向的函数
#if (defined Channel_0_For_TGS822)
int (*PointGetConcentrationForChannel_0)(float ,char ,unsigned char ) = TGS822GetConcentration;
uint8 *GasSensorFlagForChannel_0 = &TGS822GasSen_Flag;
#elif (defined Channel_1_For_TGS822)
int (*PointGetConcentrationForChannel_1)(float ,char ,unsigned char ) = TGS822GetConcentration;
uint8 *GasSensorFlagForChannel_1 = &TGS822GasSen_Flag;
#endif


/*********************************************************************
 * LOCAL FUNCTIONS
 */
void SampleApp_HandleKeys( uint8 shift, uint8 keys );
void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void SampleApp_SendPeriodicMessage( void );//广播组网
void SampleApp_SendFlashMessage( uint16 flashTime );//组播组网
void SampleApp_SendPointMessage( void );//点播组网

//@huang
void SampleAPP_SendGasSensorMessage(void);//自己定义的数据发送函数
void GetChannel1ADC(void);
void GetChannel2ADC(void);
/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SampleApp_Init
 *
 * @brief   Initialization function for the Generic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SampleApp_Init( uint8 task_id )
{
  SampleApp_TaskID = task_id;
  SampleApp_NwkState = DEV_INIT;
  SampleApp_TransID = 0;

  MT_UartInit();                //串口初始化
  MT_UartRegisterTaskID(task_id);//登记任务号
  HalUARTWrite(0,"Hello World\n",12); //（串口0，'字符'，字符个数。）
  HalAdcInit();                 //ADC参考电压初始化（默认为内部参考电压）
  // Device hardware initialization can be added here or in main() (Zmain.c).
  // If the hardware is application specific - add it here.
  // If the hardware is other parts of the device add it in main().

 #if defined ( BUILD_ALL_DEVICES )
  // The "Demo" target is setup to have BUILD_ALL_DEVICES and HOLD_AUTO_START
  // We are looking at a jumper (defined in SampleAppHw.c) to be jumpered
  // together - if they are - we will start up a coordinator. Otherwise,
  // the device will start as a router.
  if ( readCoordinatorJumper() )
    zgDeviceLogicalType = ZG_DEVICETYPE_COORDINATOR;
  else
    zgDeviceLogicalType = ZG_DEVICETYPE_ROUTER;
#endif // BUILD_ALL_DEVICES

#if defined ( HOLD_AUTO_START )
  // HOLD_AUTO_START is a compile option that will surpress ZDApp
  //  from starting the device and wait for the application to
  //  start the device.
  ZDOInitDevice(0);
#endif

  // Setup for the periodic message's destination address
  // Broadcast to everyone
  SampleApp_Periodic_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;
  SampleApp_Periodic_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_Periodic_DstAddr.addr.shortAddr = 0xFFFF;

  // Setup for the flash command's destination address - Group 1 //组播设置
  SampleApp_Flash_DstAddr.addrMode = (afAddrMode_t)afAddrGroup;
  SampleApp_Flash_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_Flash_DstAddr.addr.shortAddr = SAMPLEAPP_FLASH_GROUP;//组播号
  //点播结构体的初始化
   // Setup for the point command's destination address
  SampleApp_Point_To_Point_DstAddr.addrMode = (afAddrMode_t)Addr16Bit; //点播
  SampleApp_Point_To_Point_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_Point_To_Point_DstAddr.addr.shortAddr = 0x0000;           //0x0000 默认是协调器的地址
  // Fill out the endpoint description.
  SampleApp_epDesc.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_epDesc.task_id = &SampleApp_TaskID;
  SampleApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&SampleApp_SimpleDesc;
  SampleApp_epDesc.latencyReq = noLatencyReqs;

  // Register the endpoint description with the AF
  afRegister( &SampleApp_epDesc );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( SampleApp_TaskID );

  // By default, all devices start out in Group 1
  SampleApp_Group.ID = 0x0001;
  osal_memcpy( SampleApp_Group.name, "Group 1", 7  );
  aps_AddGroup( SAMPLEAPP_ENDPOINT, &SampleApp_Group );

#if defined ( LCD_SUPPORTED )
  HalLcdWriteString( "SampleApp", HAL_LCD_LINE_1 );
#endif

  //如果没有使用PA，则亮灯（或者其他），如果使用了PA，则...
#if defined (HAL_PA_LNA)
  HAL_TURN_ON_LED2();
#endif
}

/*********************************************************************
 * @fn      SampleApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
uint16 SampleApp_ProcessEvent( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SampleApp_TaskID );
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {
        /* Received when a key is pressed
        case KEY_CHANGE:
          SampleApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;*/

        // Received when a messages is received (OTA) for this endpoint
        case AF_INCOMING_MSG_CMD:
          SampleApp_MessageMSGCB( MSGpkt );
            //工作指示灯
           //HAL_TOGGLE_LED2();
          break;

        // Received whenever the device changes state in the network
        case ZDO_STATE_CHANGE:
          SampleApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          if ( //(SampleApp_NwkState == DEV_ZB_COORD)||       //协调器不允许对自己点播，只允许路由器和终端对协调器点播
              (SampleApp_NwkState == DEV_ROUTER)
              || (SampleApp_NwkState == DEV_END_DEVICE) )
          {
            // Start sending the periodic message in a regular interval.
            osal_start_timerEx( SampleApp_TaskID,                    //该函数定时触发事件
                              SAMPLEAPP_SEND_PERIODIC_MSG_EVT,        //参数依次是“任务ID”
                              SAMPLEAPP_SEND_PERIODIC_MSG_TIMEOUT );  //“注册事件”“循环时间”
          }
          else
          {
            // Device is no longer in the network
          }
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );

      // Next - if one is available
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SampleApp_TaskID );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);                        //返回没有处理的事件
  }

  // Send a message out - This event is generated by a timer
  //  (setup in SampleApp_Init()).
  if ( events & SAMPLEAPP_SEND_PERIODIC_MSG_EVT )
  {
    // Send the periodic message
     SampleAPP_SendGasSensorMessage();      //发送传感器数据的函数
    // Setup to send message again in normal period (+ a little jitter)
    osal_start_timerEx( SampleApp_TaskID, SAMPLEAPP_SEND_PERIODIC_MSG_EVT,
        (SAMPLEAPP_SEND_PERIODIC_MSG_TIMEOUT + (osal_rand() & 0x00FF)) );

    // return unprocessed events
    return (events ^ SAMPLEAPP_SEND_PERIODIC_MSG_EVT);
  }

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      SampleApp_MessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */
void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  float Channel0_VolSave = 0.0f;
  char  convert_arr[20];
  uint8 convert_u16[20];
  //由于有六个字节传输数据，只用到两个，所以后面四个数据字节用0xff表示
  convert_u16[2]   = 0xff;
  convert_u16[3]   = 0xff;
  convert_u16[4]   = 0xff;
  convert_u16[5]   = 0xff;  
  uint8 add[]={0xfe,0x10,0x69,0x01,0x83,0xf2,0x2e,0x01,0x00,0x4b,0x12,0x01};//初始类为01，地址01，帧头
  uint8 end[]={0x77,0x88,0x12};  //帧尾
  uint8 i=0;
  uint16   sensor_value=0;
  char count;

  switch ( pkt->clusterId )
  {
  
  case SAMPLEAPP_GAS_SENSOR_CLUSTERID:
      i=0;//用于记录数据包的计数子
      char EDid=0;
      float p=0;
      EDid=(pkt->cmd.Data)[27];
#if defined (ASCII_Printf) //设置为字符输出,取消输出就加个_not
      if(EDid==1&&contin==0)
      //if(1)
      {
        /*显示收到的数据量
          sprintf(convert_arr,"DataLength:%d\r\n",pkt->cmd.DataLength);
          HalUARTWrite(0,convert_arr,strlen(convert_arr));*/
      
          //公共数据包，温湿度数据+状态位
          sprintf(convert_arr,"TEMP：%d C\r\n",(pkt->cmd.Data)[i]),i++; //隐藏TEMP
          HalUARTWrite(0,convert_arr,strlen(convert_arr));
          sprintf(convert_arr,"HUMI：%dRH\r\n",(pkt->cmd.Data)[i]),i++;//隐藏HUMI
          HalUARTWrite(0,convert_arr,strlen(convert_arr));
      
          //判断标志位，并给出提示信息
          //DHT出错
          if((pkt->cmd.Data)[i++] & (1<<0))
          {
            HalUARTWrite(0,"DHT11 Error!\r\n",strlen("DHT11 Error!\r\n"));
          }
      
      #if (defined Channel_0_GasSensor_Busy)
          //通道0（P0.0）数据包(电压值加浓度值)+标志位
          Channel0_VolSave =  ((float)((pkt->cmd.Data)[i+1]+(pkt->cmd.Data)[i]*256))/1000;
          sprintf(convert_arr,"vol0：%.3fV\r\n",Channel0_VolSave),i+=2;
          HalUARTWrite(0,convert_arr,strlen(convert_arr));
          if(Channel0_VolSave < TGS822_Safe)
            sprintf(convert_arr,"Level: 没有污染\r\n");
          else if(Channel0_VolSave < TGS822_Mildly)
            sprintf(convert_arr,"Level: 轻度污染\r\n");
          else if(Channel0_VolSave < TGS822_Moderate)
            sprintf(convert_arr,"Level: 中度污染\r\n");
          else if(Channel0_VolSave < TGS822_Severe)
            sprintf(convert_arr,"Level: 重度污染\r\n");
      //      sprintf(convert_arr,"Level: 重度污染\r\n");
       //   HalUARTWrite(0,convert_arr,strlen(convert_arr));
          //    sprintf(convert_arr,"concen0：%dppm\r\n",BUILD_UINT16((pkt->cmd.Data)[i+1],(pkt->cmd.Data)[i])),i+=2;
      //    HalUARTWrite(0,convert_arr,strlen(convert_arr));
      //    sprintf(convert_arr,"Flag0:0x%x\r\n",(pkt->cmd.Data)[i]),i++;
      //    HalUARTWrite(0,convert_arr,strlen(convert_arr));
      #endif
        
         //打印CO、HCL、酒精数据
        for(count=0;count<=6;count++)
        {
          send_data_CO[count]=(pkt->cmd.Data)[count+8];
        }
        for(count=0;count<=5;count++)
        {
          send_data_HCL[count]=(pkt->cmd.Data)[count+15];
        }
        for(count=0;count<=5;count++)
        {
          send_data_MQ3[count]=(pkt->cmd.Data)[count+21];
        }
        HalUARTWrite(0, "CO:", 3);
        HalUARTWrite(0, send_data_CO, 7); //输出接收到的数据
        HalUARTWrite(0, "ppm", 3);
        HalUARTWrite(0, "\r\n", 2);         //回车换行
        
        HalUARTWrite(0, "HCL:", 4);
        HalUARTWrite(0, send_data_HCL, 6); //输出接收到的数据
        HalUARTWrite(0, "ppm", 3);
        HalUARTWrite(0, "\r\n", 2);         //回车换行
        
        HalUARTWrite(0, "AOL:", 4);
        HalUARTWrite(0, send_data_MQ3, 6); //输出接收到的数据
        HalUARTWrite(0, "ppm", 3);
        HalUARTWrite(0, "\r\n", 2);         //回车换行
        HalUARTWrite(0, "\r\n", 2);         //回车换行
        HalUARTWrite(0, "\r\n", 2);
        
    if((Channel0_VolSave>0.15)&&((atof(send_data_MQ3))>150)&&((Channel0_VolSave*1000)>atof(send_data_MQ3)))
          flag_tgs++;
        
    if((Channel0_VolSave>0.15)&&((atof(send_data_MQ3))>150)&&((Channel0_VolSave*1000)<atof(send_data_MQ3)))
          flag_aol++;
    
        
    if(++flag==5)
        
      {
           
           flag=0;      
           
           if(flag_tgs>=3)
              HalUARTWrite(0, "汽油泄漏\r\n", 10);
                  
           if(flag_aol>=3)
             HalUARTWrite(0, "酒精泄漏\r\n", 10);
           flag_tgs=0;flag_aol=0;       
       
      }        
        contin=1;
                  //公共数据包，温湿度数据+状态位

      }
      if(EDid==2&&contin==1)
      {
        for(count=0;count<6;count++)
        {
          send_data_PM25[count]=(pkt->cmd.Data)[count];
        }
        HalUARTWrite(0, "PM2.5:", 6);
        HalUARTWrite(0, send_data_PM25, 6); //输出接收到的数据
        HalUARTWrite(0, "ug/m3", 5);
        HalUARTWrite(0, "\r\n", 2);         //回车换行

        contin=0;
      }
#endif

#if defined (uint8_Printf_not) //设置为16进制显示输出,取消输出就加个_not
      if(EDid==1&&contin1==0)
      {
      
          i=0;
          add[3] =0x01;
          add[11]=0x01; //设置温度物理地址为01
          convert_u16[0]   = 0x00;
          convert_u16[1]   = (pkt->cmd.Data)[i++]; 
          HalUARTWrite(0,add,12);
          HalUARTWrite(0,convert_u16,6);   //温度
          HalUARTWrite(0,end,3);
          add[3] =0x02;
          add[11]=0x02; //设置湿度物理地址为02         
          convert_u16[0]   = 0x00;
          convert_u16[1]   = (pkt->cmd.Data)[i++];   
          HalUARTWrite(0,add,12);
          HalUARTWrite(0,convert_u16,6);   //湿度
          HalUARTWrite(0,end,3);
          i++;  //跳过温湿度传感数据的标志位
      
      #if (defined Channel_0_GasSensor_Busy)
          //通道0（P0.0）数据包(电压值加浓度值)+标志位
          Channel0_VolSave =  (float)BUILD_UINT16((pkt->cmd.Data)[i+1],(pkt->cmd.Data)[i]);  //不除以1000，让其值没有浮点，但是过后要除以1000
          sensor_value     =   Channel0_VolSave;
          convert_u16[0]   =   sensor_value>>8;
          convert_u16[1]   =   (sensor_value&0x00ff);
          add[3] =0x03;
          add[11]=0x03;   //设置有机溶剂物理地址为03
          HalUARTWrite(0,add,12);
          HalUARTWrite(0,convert_u16,6); 
          HalUARTWrite(0,end,3);
      #endif
        
         //打印CO、HCL、酒精数据
        for(count=0;count<=6;count++)
        {
          send_data_CO[count]=(pkt->cmd.Data)[count+8];
        }
        for(count=0;count<=5;count++)
        {
          send_data_HCL[count]=(pkt->cmd.Data)[count+15];
        }
        for(count=0;count<=5;count++)
        {
          send_data_MQ3[count]=(pkt->cmd.Data)[count+21];
        }
       
        sensor_value = atof(send_data_CO)*10;  //将浮点数转换成整数
        if(atof(send_data_CO)<0) sensor_value=0;
        convert_u16[0]   =   sensor_value>>8;
        convert_u16[1]   =   (sensor_value&0x00ff);
        add[3] =0x04;
        add[11]=0x04;   
        HalUARTWrite(0,add,12);
        HalUARTWrite(0,convert_u16,6);  //输出接收到的数据
        HalUARTWrite(0,end,3);  

        sensor_value = atof(send_data_HCL)*10;
        if(atof(send_data_HCL)<0) sensor_value=0;
        convert_u16[0]   =   sensor_value>>8;
        convert_u16[1]   =   (sensor_value&0x00ff);
        add[3] =0x05;
        add[11]=0x05;
        HalUARTWrite(0,add,12);
        HalUARTWrite(0,convert_u16,6);  //输出接收到的数据
        HalUARTWrite(0,end,3);     

        sensor_value = atof(send_data_MQ3)*10;
        if(atof(send_data_MQ3)<0) sensor_value=0;
        convert_u16[0]   =   sensor_value>>8;
        convert_u16[1]   =   (sensor_value&0x00ff);
        add[3] =0x06;       
        add[11]=0x06;
        HalUARTWrite(0,add,12);
        HalUARTWrite(0,convert_u16,6);  //输出接收到的数据
        HalUARTWrite(0,end,3);
     
        /*判断香烟 电缆燃烧*/
        p=atof(send_data_HCL)/atof(send_data_CO);
        if((p>0.1&&p<0.2)&&(atof(send_data_HCL))>1.0)
            flag_co++;       
        if(p>0.4&&p<3.0&&(atof(send_data_HCL))>1.0)
            flag_hcl++;        
        if(++flag==5)            
          {               
               flag=0;                    
               if(flag_co>=3)                  
                   flag_ch=1;
                      
               if(flag_hcl>=3)
                   flag_ch=2;
               flag_co=0;
               flag_hcl=0;                  
          }      
        
        /*判断酒精、汽油泄漏*/   
         if((Channel0_VolSave>150)&&((atof(send_data_MQ3))>150)&&((Channel0_VolSave)>atof(send_data_MQ3)))
            flag_tgs++;        
         if((Channel0_VolSave>150)&&((atof(send_data_MQ3))>150)&&((Channel0_VolSave)<atof(send_data_MQ3)))
            flag_aol++;              
         if(++flag==5)        
          {               
               flag=0;              //标志位清零       
               if(flag_tgs>=3)
                  flag_ta=1;                      
               if(flag_aol>=3)
                  flag_ta=2;
               flag_tgs=0;
               flag_aol=0;                  
          }  
        
        contin1=1;
      }
      if(EDid==2&&contin1==1)
      {  
        /*构建PM25帧*/
        for(count=0;count<6;count++)
        {
          send_data_PM25[count]=(pkt->cmd.Data)[count];
        }
        sensor_value = atof(send_data_PM25)*100;
        if(atof(send_data_PM25)<0) sensor_value=0;
        convert_u16[0]   =   sensor_value>>8;
        convert_u16[1]   =   (sensor_value&0x00ff);
        add[3] =0x07;
        add[11]=0x07;
        HalUARTWrite(0,add,12);
        HalUARTWrite(0,convert_u16,6);  //输出接收到的数据
        HalUARTWrite(0,end,3);
        
        if(flag_ch==0)
        {
             convert_u16[0]   =   0x00;   //高字节
             convert_u16[1]   =   0x00;   //低字节
             add[3] =0x08;                    //香烟电缆燃烧标志位
             add[11]=0x08;                    //香烟电缆燃烧标志位
             HalUARTWrite(0,add,12);
             HalUARTWrite(0,convert_u16,6);  //输出接收到的数据
             HalUARTWrite(0,end,3);
        }
        else if(flag_ch==1)
        {
             convert_u16[0]   =   0x00;   //高字节
             convert_u16[1]   =   0x01;   //香烟置一
             add[3] =0x08;                    //香烟电缆燃烧标志位
             add[11]=0x08;                    //香烟电缆燃烧标志位
             HalUARTWrite(0,add,12);
             HalUARTWrite(0,convert_u16,6);  //输出接收到的数据
             HalUARTWrite(0,end,3);
             flag_ch=0;
        }
        else if(flag_ch==2)
        {
             convert_u16[0]   =   0x00;   //高字节
             convert_u16[1]   =   0x02;   //电缆置二
             add[3] =0x08;                    //香烟电缆燃烧标志位
             add[11]=0x08;                    //香烟电缆燃烧标志位
             HalUARTWrite(0,add,12);
             HalUARTWrite(0,convert_u16,6);  //输出接收到的数据
             HalUARTWrite(0,end,3);
             flag_ch=0;
        }
        
        /*判断酒精、汽油泄漏*/
        if(flag_ta==0)
        {
             convert_u16[0]   =   0x00;   //高字节
             convert_u16[1]   =   0x00;   //低字节
             add[3] =0x09;                    //酒精、汽油泄漏标志位
             add[11]=0x09;                    //酒精、汽油泄漏标志位
             HalUARTWrite(0,add,12);
             HalUARTWrite(0,convert_u16,6);  //输出接收到的数据
             HalUARTWrite(0,end,3);
        }
        else if(flag_ta==1)   //汽油泄漏
        {
             convert_u16[0]   =   0x00;   //高字节
             convert_u16[1]   =   0x01;   //汽油泄漏置一
             add[3] =0x09;                    //酒精、汽油泄漏标志位
             add[11]=0x09;                    //酒精、汽油泄漏标志位
             HalUARTWrite(0,add,12);
             HalUARTWrite(0,convert_u16,6);  //输出接收到的数据
             HalUARTWrite(0,end,3);
             flag_ta=0;
        }
        else if(flag_ta==2)  //酒精泄漏
        {
             convert_u16[0]   =   0x00;   //高字节
             convert_u16[1]   =   0x02;   //酒精泄漏置二
             add[3] =0x09;                    //酒精、汽油泄漏标志位
             add[11]=0x09;                    //酒精、汽油泄漏标志位
             HalUARTWrite(0,add,12);
             HalUARTWrite(0,convert_u16,6);  //输出接收到的数据
             HalUARTWrite(0,end,3);
             flag_ta=0;
        }
        
        contin1=0;
      }      
#endif
      break;
  }
}


/*********************************************************************
 * @fn      SampleAPP_SendGasSensorMessage
 *
 * @brief   Send the flash message to group 1.
 *
 * @param   flashTime - in milliseconds
 *
 * @return  none
 */

void SampleAPP_SendGasSensorMessage(void)
{
  
  uint8 Transmit_data[28];              //需要发送的数据    包=公共数据(温湿度数据和标志位)+通道0数据和标志+通道1数据和标志，后面两个数据可选
  float val,val1,co_value,HCL_value,mq3_value;
  //节点1--温湿度和酒精，一氧化碳，氯化氢，
  
  char temp;                   //温度
  unsigned char humi;          //湿度
  uint8 PublicFlag=0;         //该变量记录了DHT11的状态（是否读取成功)，还可以记录其他状态
  Transmit_data[27]=1;//节点1标记
#if (defined Channel_0_GasSensor_Busy)
  float  ADC_Channel_0_Vol=0;             //ADC电压
  uint16 ADC_Channel_0_Vol_ForTx = 0;    //将电压值乘上1000，用于发送出去
  uint16 ADC_Channel_0_RegData;          //ADC寄存器数据
  uint16 Channel_0_GasSenDataSave=0;     //保存气体传感器的数据
#endif

  uint8 i=0;                            //循环要用到
  uint8 TxPackLength=0;                 //记录包长度
  PublicFlag =0;                        //清空标志位

  uint8 ADC_Result_ASCII[20];
  //ADC测试结果，使用内部参考电压是最准确的2017 2 19
  //注意杜邦线的压降需要补偿，实际电路中，务必测试传感器引脚到ADC引脚的压降，然后软件加以补偿。2017 2 19
  //温度传感器  DHT11
  //DHT11读取失败，可能DHT11损坏，可能DHT11被拔出或者连线松动
    if(dht11_value(&temp , &humi , DHT11_UINT8) != 0)
    {
      temp = DHT11_TEMP_DEFAULT;  //设置温湿度为默认值
      humi = DHT11_HUMI_DEFAULT;
      PublicFlag |= (1<<0);     //置位标志位
    }
    
    if(temp != 0)
        
      temp_f = temp;
    
    if(humi != 0)
        
      humi_f = humi;
    
    Transmit_data[TxPackLength] = (uint8)temp_f , TxPackLength++;                     //装入温度,TxPackLength=0
    Transmit_data[TxPackLength] = (uint8)humi_f , TxPackLength++;                     //装入湿度
    Transmit_data[TxPackLength] =  PublicFlag , TxPackLength++;                    //装入公共标志位

//如果用到了channe0则编译，使用P0.0，将通道0数据和标志位装入发送包中
#if (defined Channel_0_GasSensor_Busy)
    //多次读出ADC的值，然后取平均值
    ADC_Channel_0_RegData = HalAdcRead(HAL_ADC_CHANNEL_2,HAL_ADC_RESOLUTION_14);    //选择P0_1，14位分辨率，13位有效数据
    for(i=0;i<10;i++)
    {
      ADC_Channel_0_RegData += HalAdcRead(HAL_ADC_CHANNEL_2,HAL_ADC_RESOLUTION_14);  //选择P0_1,14位分辨率,13位有效数据
      ADC_Channel_0_RegData >>=1;                                                    //除以2
    }
    ADC_Channel_0_Vol =  (float)ADC_Channel_0_RegData*AMS1117_3_ADC_VOL/8192;         //计算得电压值
    ADC_Channel_0_Vol-=0.79;   //去除零点漂移
    ADC_Channel_0_Vol_ForTx = (ADC_Channel_0_Vol * 1000);
    if(ADC_Channel_0_Vol_ForTx<0) ADC_Channel_0_Vol_ForTx=0;
    
   // ADC_Channel_0_Vol_ForTx =3215;//用于text

    //Gas sensor获取数据,保存起来
    Channel_0_GasSenDataSave = (uint16)PointGetConcentrationForChannel_0(ADC_Channel_0_Vol,temp,humi);

    //Transmit_data[TxPackLength] = HI_UINT16(ADC_Channel_0_Vol_ForTx) , TxPackLength++;       //取处理后的电压的值高八位
    //Transmit_data[TxPackLength] = LO_UINT16(ADC_Channel_0_Vol_ForTx) , TxPackLength++;       //取处理后的电压值的低八位
    Transmit_data[TxPackLength] = ADC_Channel_0_Vol_ForTx>>8 , TxPackLength++;       //取处理后的电压的值高八位
    Transmit_data[TxPackLength] = (ADC_Channel_0_Vol_ForTx&0x00ff) , TxPackLength++;       //取处理后的电压值的低八位
    Transmit_data[TxPackLength] = HI_UINT16(Channel_0_GasSenDataSave) , TxPackLength++;       //装入计算后的气体浓度高八位
    Transmit_data[TxPackLength] = LO_UINT16(Channel_0_GasSenDataSave) , TxPackLength++;       //装入计算后的气体浓度低八位
    Transmit_data[TxPackLength] = *GasSensorFlagForChannel_0, TxPackLength++;                  //装入标志位
#endif
    
    //CO数据采集 
    uint8 data[7];
    char num;
    ADC_Channel_0_RegData = HalAdcRead(HAL_ADC_CHANNEL_0,HAL_ADC_RESOLUTION_14);    //选择P0_0，14位分辨率，13位有效数据
    for(i=0;i<10;i++)
    {
      ADC_Channel_0_RegData += HalAdcRead(HAL_ADC_CHANNEL_0,HAL_ADC_RESOLUTION_14);  //选择P0_0,14位分辨率,13位有效数据
      ADC_Channel_0_RegData >>=1;                                                    //除以2
    }
    co_value =  (float)ADC_Channel_0_RegData*AMS1117_3_ADC_VOL/8192;         //计算得电压值*/
    //co_value -=1.292;//去除零点
    //co_value  =(210.0/221.0)*co_value*1000-1050/221;
    if(co_value<0) co_value=0;
    memset(data,0,7);
    sprintf(data,"%.1f",co_value);
    for(num=0;num<=6;num++)//装CO的数据
    {
      Transmit_data[num+8]=data[num];
    }
    
    //HCL数据采集
    //val1=ValreadP0_0();//读取HCL的电压值
    //if(val1>3.6)//数据排查
    //    val1=0;
    //HCL_value = (val1*1000-330)*20/1427;
    //HCL_value=val;  //此句用于测试，记得删除
    ADC_Channel_0_RegData = HalAdcRead(HAL_ADC_CHANNEL_4,HAL_ADC_RESOLUTION_14);    //选择P0_4，14位分辨率，13位有效数据
    for(i=0;i<10;i++)
    {
      ADC_Channel_0_RegData += HalAdcRead(HAL_ADC_CHANNEL_4,HAL_ADC_RESOLUTION_14);  //选择P0_4,14位分辨率,13位有效数据
      ADC_Channel_0_RegData >>=1;                                                    //除以2
    }
    HCL_value =  (float)ADC_Channel_0_RegData*AMS1117_3_ADC_VOL/8192;         //计算得电压值*/
   // HCL_value-=0.04;  //去除零点漂移
    //HCL_value=(1.0/30.0)*HCL_value*1000-8;
    if(HCL_value<0) HCL_value=0;
    memset(data,0,7);//清除数组
    sprintf(data,"%.1f",HCL_value);//将数据转化成字符串，并且装进数组里面
    
    for(num=0;num<=5;num++)//装HCL的数据
    {
      Transmit_data[num+15]=data[num];
    }
    
    //酒精mq3数据采集
    ADC_Channel_0_RegData = HalAdcRead(HAL_ADC_CHANNEL_5,HAL_ADC_RESOLUTION_14);    //选择P0_5，14位分辨率，13位有效数据
    for(i=0;i<10;i++)
    {
      ADC_Channel_0_RegData += HalAdcRead(HAL_ADC_CHANNEL_5,HAL_ADC_RESOLUTION_14);  //选择P0_5,14位分辨率,13位有效数据
      ADC_Channel_0_RegData >>=1;                                                    //除以2
    }
    val =  (float)ADC_Channel_0_RegData*AMS1117_3_ADC_VOL/8192-0.4;         //计算得电压值
    
    //mq3_value=val*(8.0/500.0)-1.6/500.0;
    mq3_value=val;
    
    if(((uint16)PointGetConcentrationForChannel_0(val,temp,humi))!=0)
        mq3_value = (uint16)PointGetConcentrationForChannel_0(val,temp,humi);
    
    memset(data,0,7);//清除数组
    sprintf(data,"%.1f",mq3_value);//将数据转化成字符串，并且装进数组里面
    for(num=0;num<=5;num++)//装MQ3的数据
    {
      Transmit_data[num+21]=data[num];
    }
    
    //将打包好的数据包发射出去
    if ( AF_DataRequest( &SampleApp_Flash_DstAddr, &SampleApp_epDesc,//点播设置的ID
                     SAMPLEAPP_GAS_SENSOR_CLUSTERID, //与接收方建立联系的参数，如1，表示由周期性广播方式发送过来的数据
                     28,                   //数据量（长度）
                     Transmit_data,                  //数据首地址
                     &SampleApp_TransID,
                     AF_DISCV_ROUTE,
                     AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
    {
    }
}

/*********************************************************************
*********************************************************************/
