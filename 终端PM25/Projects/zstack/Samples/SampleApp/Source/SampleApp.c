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

//C���Ա�׼��
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//PM25�ļ�
#include <PM25.h>


char contin=0;
char contin1=0;

float pm_data;
uint8 send_data_PM25[8];
char PM_data[20];
char pm_rxlen;
uchar RXBUF[20];
uint8 Transmit_data[28];         //��Ҫ���͵�����    
                                   //��=��������(��ʪ�����ݺͱ�־λ)+ͨ��0���ݺͱ�־+ͨ��1���ݺͱ�־�������������ݿ�ѡ

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
  MT_UartRegisterTaskID(SampleApp_TaskID);//�Ǽ������
  HalUARTWrite(0,"Hello World\n",12); //������0��'�ַ�'���ַ���������
  //IEN0 |= (1<<2);                 //ʹ��USART0 RX
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
  u16   sensor_value=0;
  char count;

  switch ( pkt->clusterId )
  {
  
  case SAMPLEAPP_GAS_SENSOR_CLUSTERID:
      i=0;//���ڼ�¼���ݰ��ļ�����
      char EDid=0;
      EDid=(pkt->cmd.Data)[27];
#if defined (ASCII_Printf) //����Ϊ�ַ����
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
          Channel0_VolSave =  (float)BUILD_UINT16((pkt->cmd.Data)[i+1],(pkt->cmd.Data)[i])/1000;
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
          HalUARTWrite(0,convert_arr,strlen(convert_arr));
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
        //sprintf(text,"HELLO!",(pkt->cmd.Data)[i]);
        
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
          Channel0_VolSave =  (float)BUILD_UINT16((pkt->cmd.Data)[i+1],(pkt->cmd.Data)[i]);  //����1000������ֵû�и��㣬���ǹ���Ҫ����1000
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
  //�ڵ�2--PM2.5���ݲɼ�
    uchar ch;
    uint16 len,time=0;
    uint8 i=0;
    while(!(len=Hal_UART_RxBufLen(0)));
     // if(time++==65530) break;   //�ȴ����ڽ��յ�����,len���Բ���Hal_UART_RxBufLen(0)���ص����ݳ��ȣ������1
    while(Hal_UART_RxBufLen(0))
    {
      pm_rxlen=HalUARTRead (0,&ch, 1);
      RXBUF[i]=ch;
      HalUARTWrite(0,&ch,1); //������0��'�ַ�'���ַ���������   
      if(++i == 10) break;
      while(Hal_UART_RxBufLen(0)==0); //�����ȴ�
    }
    if((RXBUF[0]==0XAA)&&(RXBUF[1]==0XC0)&&(RXBUF[9]==0XAB))
    {
        pm_data=((float)(RXBUF[3]*256+RXBUF[2]))/10;
        sprintf(PM_data,"%.1f",pm_data);
    }
  
    
    Transmit_data[27]=2;//�ڵ�2���
    Transmit_data[0] = PM_data[0];
    Transmit_data[1] = PM_data[1];
    Transmit_data[2] = PM_data[2];
    Transmit_data[3] = PM_data[3];
    Transmit_data[4] = PM_data[4];
    Transmit_data[5] = PM_data[5];
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
