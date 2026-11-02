/**
  ******************************************************************************
  * @file    cus_usb_vcom.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of USB Virtual port comunication,it is for USB AT port read\write.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "userspaceConfig.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "stdlib.h"
#include "simcom_usb_vcom.h"
#include "stdio.h"
#include "string.h"
#include "simcom_at.h"

#define USB_VCOM_RX_BUFFER_SIZE     512

#define SCFW_ATHEAD              ("AT")
#define SCFW_ATTAIL              ("\r\n")

extern void CacheDataToUIDemo(char *data, int len);

/**
  * @brief  Determine whether the input data is an AT command.
  */
int isATCommand(void* data, int length)
{
    char *ret;

    if(strncmp((char *)data, SCFW_ATHEAD, 2) == 0)
    {
        ret = strstr(data, SCFW_ATTAIL);
        if(ret != NULL)
        {
            return 1;
        }
    }

    return 0;
}

/**
  * @brief  Callback for USB VCOM receiving event,customer could read out the data with pointer usbVCcomData.If define SIMCOM_UI_DEMO_TO_USB_AT_PORT then send message to UI demo task.
  * @param  Pointer para.
  * @note   In real application please loop reading by sAPI_UsbVcomRead until return value of sAPI_UsbVcomRead is 0.
  * @retval void
  */
void UsbVcomCBFunc(void *para)
{
    int readLen = 0;
    char *usbVCcomData = malloc(USB_VCOM_RX_BUFFER_SIZE);

    memset(usbVCcomData,0,USB_VCOM_RX_BUFFER_SIZE);
    readLen = sAPI_UsbVcomRead((UINT8 *)usbVCcomData, USB_VCOM_RX_BUFFER_SIZE);
    sAPI_Debug("%s, usbVCcomData = %s,readLen[%d]",__func__,usbVCcomData,readLen);

#ifdef SIMCOM_UI_DEMO_TO_USB_AT_PORT
    CacheDataToUIDemo((char *)usbVCcomData, readLen);
#endif

    free((void*)usbVCcomData);

    return;
}

/**
  * @brief  Extension Callback for USB VCOM receiving event with fixed data length.If define SIMCOM_UI_DEMO_TO_USB_AT_PORT then send message to UI demo task.
  * @param  Pointer para.
  * @note   In real application please loop reading by sAPI_UsbVcomRead until return value of sAPI_UsbVcomRead is 0.
  * @retval void
  */
void UsbVcomCBFuncEx(int len, void *para)
{
    int readLen = 0;
    unsigned char *usbVCcomData = malloc(len);

    memset(usbVCcomData,0,len);
    readLen = sAPI_UsbVcomRead((UINT8 *)usbVCcomData, len);
    if(sAPI_UsbVbusDetect() == 1)
    {
        sAPI_Debug("%s, usbVCcomData = %s,readLen[%d]",__func__,usbVCcomData,readLen);
        if( isATCommand(usbVCcomData,readLen) == 1)
        {
            sAPI_AtSend(usbVCcomData,readLen);//send AT command
        }
        else
        {
#ifdef SIMCOM_UI_DEMO_TO_USB_AT_PORT
            CacheDataToUIDemo((char *)usbVCcomData, readLen);
#endif            
        }
    }


    free((void*)usbVCcomData);

    return;
}

/**
  * @brief  Create USB VCOM task and register receiving callback.
  * @param  void
  * @note   Default with UsbVcomCBFuncEx callback.
  * @retval void
  */
void sAPP_UsbVcomTask(void)
{
    sAPI_UsbVcomRegisterCallbackEX(UsbVcomCBFuncEx, (void *)"UsbVcomCBFuncEx");
    sAPI_Debug("sAPP_UsbVcomTask Succeed!\n");
}


