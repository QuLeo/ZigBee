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
  PROVIDED �AS IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
//�˴����ڴ���
#include  "MT_UART.h"
//��ʪ�ȴ�����
#include "dht11.h"
//C���Ա�׼��
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//GasSensor ��ȡ���ݵ�ͷ�ļ�
#include "MQ3.h"
#include "TGS822.h"

//CO
#include <CO.h>

/*********************************************************************
 * MACROS
 */


#define AMS1117_3_ADC_VOL     3.28   //�궨��AMS1117 3.3 ADC�ο���ѹ����ֵ������3.2-3.4����
//���������channel0����channel1�����崫��������
#if (defined Channel_0_For_TGS813) || (defined Channel_0_For_TGS822)
#define Channel_0_GasSensor_Busy
#endif
#if (defined Channel_1_For_TGS813) || (defined Channel_1_For_TGS822)
#define Channel_1_GasSensor_Busy
#endif
//���������崫����ͬʱռ��ͬһ��channel������ʾ
#if (defined Channel_0_For_TGS813) && (defined Channel_0_For_TGS822)
#error "Channel0 is just for one gas sensor.Check the micro!"
#endif
#if (defined Channel_1_For_TGS813) && (defined Channel_1_For_TGS822)
#error "Channel1 is just for one gas sensor.Check the micro!"

#endif
#if (!defined Channel_0_GasSensor_Busy) && (!defined Channel_1_GasSensor_Busy)
#warning "Do you want to use the Gas Sensor? if yes,please define it,if not please ignore this message."
#endif

// ���ò�ͬ��������ѹ��ֵ
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

afAddrType_t SampleApp_Flash_DstAddr;//�鲥

afAddrType_t SampleApp_Point_To_Point_DstAddr;

aps_Group_t SampleApp_Group;//��������

uint8 SampleAppPeriodicCounter = 0;
uint8 SampleAppFlashCounter = 0;


//@huang
//����channel0��channel1��ȡ����Ũ��ָ��ĺ���
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
void SampleApp_SendPeriodicMessage( void );//�㲥����
void SampleApp_SendFlashMessage( uint16 flashTime );//�鲥����
void SampleApp_SendPointMessage( void );//�㲥����

//@huang
void SampleAPP_SendGasSensorMessage(void);//�Լ���������ݷ��ͺ���
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

  MT_UartInit();                //���ڳ�ʼ��
  MT_UartRegisterTaskID(task_id);//�Ǽ������
  HalUARTWrite(0,"Hello World\n",12); //������0��'�ַ�'���ַ���������
  HalAdcInit();                 //ADC�ο���ѹ��ʼ����Ĭ��Ϊ�ڲ��ο���ѹ��
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

  // Setup for the flash command's destination address - Group 1 //�鲥����
  SampleApp_Flash_DstAddr.addrMode = (afAddrMode_t)afAddrGroup;
  SampleApp_Flash_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_Flash_DstAddr.addr.shortAddr = SAMPLEAPP_FLASH_GROUP;//�鲥��
  //�㲥�ṹ��ĳ�ʼ��
   // Setup for the point command's destination address
  SampleApp_Point_To_Point_DstAddr.addrMode = (afAddrMode_t)Addr16Bit; //�㲥
  SampleApp_Point_To_Point_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_Point_To_Point_DstAddr.addr.shortAddr = 0x0000;           //0x0000 Ĭ����Э�����ĵ�ַ
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

  //���û��ʹ��PA�������ƣ����������������ʹ����PA����...
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
            //����ָʾ��
           //HAL_TOGGLE_LED2();
          break;

        // Received whenever the device changes state in the network
        case ZDO_STATE_CHANGE:
          SampleApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          if ( //(SampleApp_NwkState == DEV_ZB_COORD)||       //Э������������Լ��㲥��ֻ����·�������ն˶�Э�����㲥
              (SampleApp_NwkState == DEV_ROUTER)
              || (SampleApp_NwkState == DEV_END_DEVICE) )
          {
            // Start sending the periodic message in a regular interval.
            osal_start_timerEx( SampleApp_TaskID,                    //�ú�����ʱ�����¼�
                              SAMPLEAPP_SEND_PERIODIC_MSG_EVT,        //���������ǡ�����ID��
                              SAMPLEAPP_SEND_PERIODIC_MSG_TIMEOUT );  //��ע���¼�����ѭ��ʱ�䡱
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
    return (events ^ SYS_EVENT_MSG);                        //����û�д�����¼�
  }

  // Send a message out - This event is generated by a timer
  //  (setup in SampleApp_Init()).
  if ( events & SAMPLEAPP_SEND_PERIODIC_MSG_EVT )
  {
    // Send the periodic message
     SampleAPP_SendGasSensorMessage();      //���ʹ��������ݵĺ���
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
  //�����������ֽڴ������ݣ�ֻ�õ����������Ժ����ĸ������ֽ���0xff��ʾ
  convert_u16[2]   = 0xff;
  convert_u16[3]   = 0xff;
  convert_u16[4]   = 0xff;
  convert_u16[5]   = 0xff;  
  uint8 add[]={0xfe,0x10,0x69,0x01,0x83,0xf2,0x2e,0x01,0x00,0x4b,0x12,0x01};//��ʼ��Ϊ01����ַ01��֡ͷ
  uint8 end[]={0x77,0x88,0x12};  //֡β
  uint8 i=0;
  uint16   sensor_value=0;
  char count;

  switch ( pkt->clusterId )
  {
  
  case SAMPLEAPP_GAS_SENSOR_CLUSTERID:
      i=0;//���ڼ�¼���ݰ��ļ�����
      char EDid=0;
      float p=0;
      EDid=(pkt->cmd.Data)[27];
#if defined (ASCII_Printf) //����Ϊ�ַ����,ȡ������ͼӸ�_not
      if(EDid==1&&contin==0)
      //if(1)
      {
        /*��ʾ�յ���������
          sprintf(convert_arr,"DataLength:%d\r\n",pkt->cmd.DataLength);
          HalUARTWrite(0,convert_arr,strlen(convert_arr));*/
      
          //�������ݰ�����ʪ������+״̬λ
          sprintf(convert_arr,"TEMP��%d C\r\n",(pkt->cmd.Data)[i]),i++; //����TEMP
          HalUARTWrite(0,convert_arr,strlen(convert_arr));
          sprintf(convert_arr,"HUMI��%dRH\r\n",(pkt->cmd.Data)[i]),i++;//����HUMI
          HalUARTWrite(0,convert_arr,strlen(convert_arr));
      
          //�жϱ�־λ����������ʾ��Ϣ
          //DHT����
          if((pkt->cmd.Data)[i++] & (1<<0))
          {
            HalUARTWrite(0,"DHT11 Error!\r\n",strlen("DHT11 Error!\r\n"));
          }
      
      #if (defined Channel_0_GasSensor_Busy)
          //ͨ��0��P0.0�����ݰ�(��ѹֵ��Ũ��ֵ)+��־λ
          Channel0_VolSave =  ((float)((pkt->cmd.Data)[i+1]+(pkt->cmd.Data)[i]*256))/1000;
          sprintf(convert_arr,"vol0��%.3fV\r\n",Channel0_VolSave),i+=2;
          HalUARTWrite(0,convert_arr,strlen(convert_arr));
          if(Channel0_VolSave < TGS822_Safe)
            sprintf(convert_arr,"Level: û����Ⱦ\r\n");
          else if(Channel0_VolSave < TGS822_Mildly)
            sprintf(convert_arr,"Level: �����Ⱦ\r\n");
          else if(Channel0_VolSave < TGS822_Moderate)
            sprintf(convert_arr,"Level: �ж���Ⱦ\r\n");
          else if(Channel0_VolSave < TGS822_Severe)
            sprintf(convert_arr,"Level: �ض���Ⱦ\r\n");
      //      sprintf(convert_arr,"Level: �ض���Ⱦ\r\n");
       //   HalUARTWrite(0,convert_arr,strlen(convert_arr));
          //    sprintf(convert_arr,"concen0��%dppm\r\n",BUILD_UINT16((pkt->cmd.Data)[i+1],(pkt->cmd.Data)[i])),i+=2;
      //    HalUARTWrite(0,convert_arr,strlen(convert_arr));
      //    sprintf(convert_arr,"Flag0:0x%x\r\n",(pkt->cmd.Data)[i]),i++;
      //    HalUARTWrite(0,convert_arr,strlen(convert_arr));
      #endif
        
         //��ӡCO��HCL���ƾ�����
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
        HalUARTWrite(0, send_data_CO, 7); //������յ�������
        HalUARTWrite(0, "ppm", 3);
        HalUARTWrite(0, "\r\n", 2);         //�س�����
        
        HalUARTWrite(0, "HCL:", 4);
        HalUARTWrite(0, send_data_HCL, 6); //������յ�������
        HalUARTWrite(0, "ppm", 3);
        HalUARTWrite(0, "\r\n", 2);         //�س�����
        
        HalUARTWrite(0, "AOL:", 4);
        HalUARTWrite(0, send_data_MQ3, 6); //������յ�������
        HalUARTWrite(0, "ppm", 3);
        HalUARTWrite(0, "\r\n", 2);         //�س�����
        HalUARTWrite(0, "\r\n", 2);         //�س�����
        HalUARTWrite(0, "\r\n", 2);
        
    if((Channel0_VolSave>0.15)&&((atof(send_data_MQ3))>150)&&((Channel0_VolSave*1000)>atof(send_data_MQ3)))
          flag_tgs++;
        
    if((Channel0_VolSave>0.15)&&((atof(send_data_MQ3))>150)&&((Channel0_VolSave*1000)<atof(send_data_MQ3)))
          flag_aol++;
    
        
    if(++flag==5)
        
      {
           
           flag=0;      
           
           if(flag_tgs>=3)
              HalUARTWrite(0, "����й©\r\n", 10);
                  
           if(flag_aol>=3)
             HalUARTWrite(0, "�ƾ�й©\r\n", 10);
           flag_tgs=0;flag_aol=0;       
       
      }        
        contin=1;
                  //�������ݰ�����ʪ������+״̬λ

      }
      if(EDid==2&&contin==1)
      {
        for(count=0;count<6;count++)
        {
          send_data_PM25[count]=(pkt->cmd.Data)[count];
        }
        HalUARTWrite(0, "PM2.5:", 6);
        HalUARTWrite(0, send_data_PM25, 6); //������յ�������
        HalUARTWrite(0, "ug/m3", 5);
        HalUARTWrite(0, "\r\n", 2);         //�س�����

        contin=0;
      }
#endif

#if defined (uint8_Printf_not) //����Ϊ16������ʾ���,ȡ������ͼӸ�_not
      if(EDid==1&&contin1==0)
      {
      
          i=0;
          add[3] =0x01;
          add[11]=0x01; //�����¶������ַΪ01
          convert_u16[0]   = 0x00;
          convert_u16[1]   = (pkt->cmd.Data)[i++]; 
          HalUARTWrite(0,add,12);
          HalUARTWrite(0,convert_u16,6);   //�¶�
          HalUARTWrite(0,end,3);
          add[3] =0x02;
          add[11]=0x02; //����ʪ�������ַΪ02         
          convert_u16[0]   = 0x00;
          convert_u16[1]   = (pkt->cmd.Data)[i++];   
          HalUARTWrite(0,add,12);
          HalUARTWrite(0,convert_u16,6);   //ʪ��
          HalUARTWrite(0,end,3);
          i++;  //������ʪ�ȴ������ݵı�־λ
      
      #if (defined Channel_0_GasSensor_Busy)
          //ͨ��0��P0.0�����ݰ�(��ѹֵ��Ũ��ֵ)+��־λ
          Channel0_VolSave =  (float)BUILD_UINT16((pkt->cmd.Data)[i+1],(pkt->cmd.Data)[i]);  //������1000������ֵû�и��㣬���ǹ���Ҫ����1000
          sensor_value     =   Channel0_VolSave;
          convert_u16[0]   =   sensor_value>>8;
          convert_u16[1]   =   (sensor_value&0x00ff);
          add[3] =0x03;
          add[11]=0x03;   //�����л��ܼ������ַΪ03
          HalUARTWrite(0,add,12);
          HalUARTWrite(0,convert_u16,6); 
          HalUARTWrite(0,end,3);
      #endif
        
         //��ӡCO��HCL���ƾ�����
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
       
        sensor_value = atof(send_data_CO)*10;  //��������ת��������
        if(atof(send_data_CO)<0) sensor_value=0;
        convert_u16[0]   =   sensor_value>>8;
        convert_u16[1]   =   (sensor_value&0x00ff);
        add[3] =0x04;
        add[11]=0x04;   
        HalUARTWrite(0,add,12);
        HalUARTWrite(0,convert_u16,6);  //������յ�������
        HalUARTWrite(0,end,3);  

        sensor_value = atof(send_data_HCL)*10;
        if(atof(send_data_HCL)<0) sensor_value=0;
        convert_u16[0]   =   sensor_value>>8;
        convert_u16[1]   =   (sensor_value&0x00ff);
        add[3] =0x05;
        add[11]=0x05;
        HalUARTWrite(0,add,12);
        HalUARTWrite(0,convert_u16,6);  //������յ�������
        HalUARTWrite(0,end,3);     

        sensor_value = atof(send_data_MQ3)*10;
        if(atof(send_data_MQ3)<0) sensor_value=0;
        convert_u16[0]   =   sensor_value>>8;
        convert_u16[1]   =   (sensor_value&0x00ff);
        add[3] =0x06;       
        add[11]=0x06;
        HalUARTWrite(0,add,12);
        HalUARTWrite(0,convert_u16,6);  //������յ�������
        HalUARTWrite(0,end,3);
     
        /*�ж����� ����ȼ��*/
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
        
        /*�жϾƾ�������й©*/   
         if((Channel0_VolSave>150)&&((atof(send_data_MQ3))>150)&&((Channel0_VolSave)>atof(send_data_MQ3)))
            flag_tgs++;        
         if((Channel0_VolSave>150)&&((atof(send_data_MQ3))>150)&&((Channel0_VolSave)<atof(send_data_MQ3)))
            flag_aol++;              
         if(++flag==5)        
          {               
               flag=0;              //��־λ����       
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
        /*����PM25֡*/
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
        HalUARTWrite(0,convert_u16,6);  //������յ�������
        HalUARTWrite(0,end,3);
        
        if(flag_ch==0)
        {
             convert_u16[0]   =   0x00;   //���ֽ�
             convert_u16[1]   =   0x00;   //���ֽ�
             add[3] =0x08;                    //���̵���ȼ�ձ�־λ
             add[11]=0x08;                    //���̵���ȼ�ձ�־λ
             HalUARTWrite(0,add,12);
             HalUARTWrite(0,convert_u16,6);  //������յ�������
             HalUARTWrite(0,end,3);
        }
        else if(flag_ch==1)
        {
             convert_u16[0]   =   0x00;   //���ֽ�
             convert_u16[1]   =   0x01;   //������һ
             add[3] =0x08;                    //���̵���ȼ�ձ�־λ
             add[11]=0x08;                    //���̵���ȼ�ձ�־λ
             HalUARTWrite(0,add,12);
             HalUARTWrite(0,convert_u16,6);  //������յ�������
             HalUARTWrite(0,end,3);
             flag_ch=0;
        }
        else if(flag_ch==2)
        {
             convert_u16[0]   =   0x00;   //���ֽ�
             convert_u16[1]   =   0x02;   //�����ö�
             add[3] =0x08;                    //���̵���ȼ�ձ�־λ
             add[11]=0x08;                    //���̵���ȼ�ձ�־λ
             HalUARTWrite(0,add,12);
             HalUARTWrite(0,convert_u16,6);  //������յ�������
             HalUARTWrite(0,end,3);
             flag_ch=0;
        }
        
        /*�жϾƾ�������й©*/
        if(flag_ta==0)
        {
             convert_u16[0]   =   0x00;   //���ֽ�
             convert_u16[1]   =   0x00;   //���ֽ�
             add[3] =0x09;                    //�ƾ�������й©��־λ
             add[11]=0x09;                    //�ƾ�������й©��־λ
             HalUARTWrite(0,add,12);
             HalUARTWrite(0,convert_u16,6);  //������յ�������
             HalUARTWrite(0,end,3);
        }
        else if(flag_ta==1)   //����й©
        {
             convert_u16[0]   =   0x00;   //���ֽ�
             convert_u16[1]   =   0x01;   //����й©��һ
             add[3] =0x09;                    //�ƾ�������й©��־λ
             add[11]=0x09;                    //�ƾ�������й©��־λ
             HalUARTWrite(0,add,12);
             HalUARTWrite(0,convert_u16,6);  //������յ�������
             HalUARTWrite(0,end,3);
             flag_ta=0;
        }
        else if(flag_ta==2)  //�ƾ�й©
        {
             convert_u16[0]   =   0x00;   //���ֽ�
             convert_u16[1]   =   0x02;   //�ƾ�й©�ö�
             add[3] =0x09;                    //�ƾ�������й©��־λ
             add[11]=0x09;                    //�ƾ�������й©��־λ
             HalUARTWrite(0,add,12);
             HalUARTWrite(0,convert_u16,6);  //������յ�������
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
  
  uint8 Transmit_data[28];              //��Ҫ���͵�����    ��=��������(��ʪ�����ݺͱ�־λ)+ͨ��0���ݺͱ�־+ͨ��1���ݺͱ�־�������������ݿ�ѡ
  float val,val1,co_value,HCL_value,mq3_value;
  //�ڵ�1--��ʪ�Ⱥ;ƾ���һ����̼���Ȼ��⣬
  
  char temp;                   //�¶�
  unsigned char humi;          //ʪ��
  uint8 PublicFlag=0;         //�ñ�����¼��DHT11��״̬���Ƿ��ȡ�ɹ�)�������Լ�¼����״̬
  Transmit_data[27]=1;//�ڵ�1���
#if (defined Channel_0_GasSensor_Busy)
  float  ADC_Channel_0_Vol=0;             //ADC��ѹ
  uint16 ADC_Channel_0_Vol_ForTx = 0;    //����ѹֵ����1000�����ڷ��ͳ�ȥ
  uint16 ADC_Channel_0_RegData;          //ADC�Ĵ�������
  uint16 Channel_0_GasSenDataSave=0;     //�������崫����������
#endif

  uint8 i=0;                            //ѭ��Ҫ�õ�
  uint8 TxPackLength=0;                 //��¼������
  PublicFlag =0;                        //��ձ�־λ

  uint8 ADC_Result_ASCII[20];
  //ADC���Խ����ʹ���ڲ��ο���ѹ����׼ȷ��2017 2 19
  //ע��Ű��ߵ�ѹ����Ҫ������ʵ�ʵ�·�У���ز��Դ��������ŵ�ADC���ŵ�ѹ����Ȼ��������Բ�����2017 2 19
  //�¶ȴ�����  DHT11
  //DHT11��ȡʧ�ܣ�����DHT11�𻵣�����DHT11���γ����������ɶ�
    if(dht11_value(&temp , &humi , DHT11_UINT8) != 0)
    {
      temp = DHT11_TEMP_DEFAULT;  //������ʪ��ΪĬ��ֵ
      humi = DHT11_HUMI_DEFAULT;
      PublicFlag |= (1<<0);     //��λ��־λ
    }
    
    if(temp != 0)
        
      temp_f = temp;
    
    if(humi != 0)
        
      humi_f = humi;
    
    Transmit_data[TxPackLength] = (uint8)temp_f , TxPackLength++;                     //װ���¶�,TxPackLength=0
    Transmit_data[TxPackLength] = (uint8)humi_f , TxPackLength++;                     //װ��ʪ��
    Transmit_data[TxPackLength] =  PublicFlag , TxPackLength++;                    //װ�빫����־λ

//����õ���channe0����룬ʹ��P0.0����ͨ��0���ݺͱ�־λװ�뷢�Ͱ���
#if (defined Channel_0_GasSensor_Busy)
    //��ζ���ADC��ֵ��Ȼ��ȡƽ��ֵ
    ADC_Channel_0_RegData = HalAdcRead(HAL_ADC_CHANNEL_2,HAL_ADC_RESOLUTION_14);    //ѡ��P0_1��14λ�ֱ��ʣ�13λ��Ч����
    for(i=0;i<10;i++)
    {
      ADC_Channel_0_RegData += HalAdcRead(HAL_ADC_CHANNEL_2,HAL_ADC_RESOLUTION_14);  //ѡ��P0_1,14λ�ֱ���,13λ��Ч����
      ADC_Channel_0_RegData >>=1;                                                    //����2
    }
    ADC_Channel_0_Vol =  (float)ADC_Channel_0_RegData*AMS1117_3_ADC_VOL/8192;         //����õ�ѹֵ
    ADC_Channel_0_Vol-=0.79;   //ȥ�����Ư��
    ADC_Channel_0_Vol_ForTx = (ADC_Channel_0_Vol * 1000);
    if(ADC_Channel_0_Vol_ForTx<0) ADC_Channel_0_Vol_ForTx=0;
    
   // ADC_Channel_0_Vol_ForTx =3215;//����text

    //Gas sensor��ȡ����,��������
    Channel_0_GasSenDataSave = (uint16)PointGetConcentrationForChannel_0(ADC_Channel_0_Vol,temp,humi);

    //Transmit_data[TxPackLength] = HI_UINT16(ADC_Channel_0_Vol_ForTx) , TxPackLength++;       //ȡ�����ĵ�ѹ��ֵ�߰�λ
    //Transmit_data[TxPackLength] = LO_UINT16(ADC_Channel_0_Vol_ForTx) , TxPackLength++;       //ȡ�����ĵ�ѹֵ�ĵͰ�λ
    Transmit_data[TxPackLength] = ADC_Channel_0_Vol_ForTx>>8 , TxPackLength++;       //ȡ�����ĵ�ѹ��ֵ�߰�λ
    Transmit_data[TxPackLength] = (ADC_Channel_0_Vol_ForTx&0x00ff) , TxPackLength++;       //ȡ�����ĵ�ѹֵ�ĵͰ�λ
    Transmit_data[TxPackLength] = HI_UINT16(Channel_0_GasSenDataSave) , TxPackLength++;       //װ�����������Ũ�ȸ߰�λ
    Transmit_data[TxPackLength] = LO_UINT16(Channel_0_GasSenDataSave) , TxPackLength++;       //װ�����������Ũ�ȵͰ�λ
    Transmit_data[TxPackLength] = *GasSensorFlagForChannel_0, TxPackLength++;                  //װ���־λ
#endif
    
    //CO���ݲɼ� 
    uint8 data[7];
    char num;
    ADC_Channel_0_RegData = HalAdcRead(HAL_ADC_CHANNEL_0,HAL_ADC_RESOLUTION_14);    //ѡ��P0_0��14λ�ֱ��ʣ�13λ��Ч����
    for(i=0;i<10;i++)
    {
      ADC_Channel_0_RegData += HalAdcRead(HAL_ADC_CHANNEL_0,HAL_ADC_RESOLUTION_14);  //ѡ��P0_0,14λ�ֱ���,13λ��Ч����
      ADC_Channel_0_RegData >>=1;                                                    //����2
    }
    co_value =  (float)ADC_Channel_0_RegData*AMS1117_3_ADC_VOL/8192;         //����õ�ѹֵ*/
    //co_value -=1.292;//ȥ�����
    //co_value  =(210.0/221.0)*co_value*1000-1050/221;
    if(co_value<0) co_value=0;
    memset(data,0,7);
    sprintf(data,"%.1f",co_value);
    for(num=0;num<=6;num++)//װCO������
    {
      Transmit_data[num+8]=data[num];
    }
    
    //HCL���ݲɼ�
    //val1=ValreadP0_0();//��ȡHCL�ĵ�ѹֵ
    //if(val1>3.6)//�����Ų�
    //    val1=0;
    //HCL_value = (val1*1000-330)*20/1427;
    //HCL_value=val;  //�˾����ڲ��ԣ��ǵ�ɾ��
    ADC_Channel_0_RegData = HalAdcRead(HAL_ADC_CHANNEL_4,HAL_ADC_RESOLUTION_14);    //ѡ��P0_4��14λ�ֱ��ʣ�13λ��Ч����
    for(i=0;i<10;i++)
    {
      ADC_Channel_0_RegData += HalAdcRead(HAL_ADC_CHANNEL_4,HAL_ADC_RESOLUTION_14);  //ѡ��P0_4,14λ�ֱ���,13λ��Ч����
      ADC_Channel_0_RegData >>=1;                                                    //����2
    }
    HCL_value =  (float)ADC_Channel_0_RegData*AMS1117_3_ADC_VOL/8192;         //����õ�ѹֵ*/
   // HCL_value-=0.04;  //ȥ�����Ư��
    //HCL_value=(1.0/30.0)*HCL_value*1000-8;
    if(HCL_value<0) HCL_value=0;
    memset(data,0,7);//�������
    sprintf(data,"%.1f",HCL_value);//������ת�����ַ���������װ����������
    
    for(num=0;num<=5;num++)//װHCL������
    {
      Transmit_data[num+15]=data[num];
    }
    
    //�ƾ�mq3���ݲɼ�
    ADC_Channel_0_RegData = HalAdcRead(HAL_ADC_CHANNEL_5,HAL_ADC_RESOLUTION_14);    //ѡ��P0_5��14λ�ֱ��ʣ�13λ��Ч����
    for(i=0;i<10;i++)
    {
      ADC_Channel_0_RegData += HalAdcRead(HAL_ADC_CHANNEL_5,HAL_ADC_RESOLUTION_14);  //ѡ��P0_5,14λ�ֱ���,13λ��Ч����
      ADC_Channel_0_RegData >>=1;                                                    //����2
    }
    val =  (float)ADC_Channel_0_RegData*AMS1117_3_ADC_VOL/8192-0.4;         //����õ�ѹֵ
    
    //mq3_value=val*(8.0/500.0)-1.6/500.0;
    mq3_value=val;
    
    if(((uint16)PointGetConcentrationForChannel_0(val,temp,humi))!=0)
        mq3_value = (uint16)PointGetConcentrationForChannel_0(val,temp,humi);
    
    memset(data,0,7);//�������
    sprintf(data,"%.1f",mq3_value);//������ת�����ַ���������װ����������
    for(num=0;num<=5;num++)//װMQ3������
    {
      Transmit_data[num+21]=data[num];
    }
    
    //������õ����ݰ������ȥ
    if ( AF_DataRequest( &SampleApp_Flash_DstAddr, &SampleApp_epDesc,//�㲥���õ�ID
                     SAMPLEAPP_GAS_SENSOR_CLUSTERID, //����շ�������ϵ�Ĳ�������1����ʾ�������Թ㲥��ʽ���͹���������
                     28,                   //�����������ȣ�
                     Transmit_data,                  //�����׵�ַ
                     &SampleApp_TransID,
                     AF_DISCV_ROUTE,
                     AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
    {
    }
}

/*********************************************************************
*********************************************************************/
