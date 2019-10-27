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

//C语言标准库
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//PM25文件
#include <PM25.h>


char contin=0;
char contin1=0;

float pm_data;
uint8 send_data_PM25[8];
char PM_data[20];
char pm_rxlen;
uchar RXBUF[20];
uint8 Transmit_data[28];         //需要发送的数据    
                                   //包=公共数据(温湿度数据和标志位)+通道0数据和标志+通道1数据和标志，后面两个数据可选

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
  MT_UartRegisterTaskID(SampleApp_TaskID);//登记任务号
  HalUARTWrite(0,"Hello World\n",12); //（串口0，'字符'，字符个数。）
  //IEN0 |= (1<<2);                 //使能USART0 RX
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
  u16   sensor_value=0;
  char count;

  switch ( pkt->clusterId )
  {
  
  case SAMPLEAPP_GAS_SENSOR_CLUSTERID:
      i=0;//用于记录数据包的计数子
      char EDid=0;
      EDid=(pkt->cmd.Data)[27];
#if defined (ASCII_Printf) //设置为字符输出
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
          Channel0_VolSave =  (float)BUILD_UINT16((pkt->cmd.Data)[i+1],(pkt->cmd.Data)[i])/1000;
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
          HalUARTWrite(0,convert_arr,strlen(convert_arr));
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
        //sprintf(text,"HELLO!",(pkt->cmd.Data)[i]);
        
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
          Channel0_VolSave =  (float)BUILD_UINT16((pkt->cmd.Data)[i+1],(pkt->cmd.Data)[i]);  //不乘1000，让其值没有浮点，但是过后要除以1000
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
  //节点2--PM2.5数据采集
    uchar ch;
    uint16 len,time=0;
    uint8 i=0;
    while(!(len=Hal_UART_RxBufLen(0)));
     // if(time++==65530) break;   //等待串口接收到数据,len可以测试Hal_UART_RxBufLen(0)返回的数据长度，测得是1
    while(Hal_UART_RxBufLen(0))
    {
      pm_rxlen=HalUARTRead (0,&ch, 1);
      RXBUF[i]=ch;
      HalUARTWrite(0,&ch,1); //（串口0，'字符'，字符个数。）   
      if(++i == 10) break;
      while(Hal_UART_RxBufLen(0)==0); //继续等待
    }
    if((RXBUF[0]==0XAA)&&(RXBUF[1]==0XC0)&&(RXBUF[9]==0XAB))
    {
        pm_data=((float)(RXBUF[3]*256+RXBUF[2]))/10;
        sprintf(PM_data,"%.1f",pm_data);
    }
  
    
    Transmit_data[27]=2;//节点2标记
    Transmit_data[0] = PM_data[0];
    Transmit_data[1] = PM_data[1];
    Transmit_data[2] = PM_data[2];
    Transmit_data[3] = PM_data[3];
    Transmit_data[4] = PM_data[4];
    Transmit_data[5] = PM_data[5];
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
