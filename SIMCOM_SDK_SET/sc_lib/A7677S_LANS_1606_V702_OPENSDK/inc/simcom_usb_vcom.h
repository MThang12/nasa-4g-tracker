/**
* @file         simcom_usb_vcom.h
* @brief        SIMCom OpenLinux USB EX API
* @author       HT
* @date         2019/2/13
* @version      V1.0.0
* @par Copyright (c):
*               SIMCom Co.Ltd 2003-2019
* @par History: 1:Create
*
*/

#ifndef __SIMCOM_USB_VCOM_H__
#define __SIMCOM_USB_VCOM_H__

#include "simcom_os.h"
/****************************************************************************
    Define enum
*****************************************************************************/
typedef enum  /* The status of Tx */
{
    USB_TX_SUCCEED  = 0,
    USB_TX_FAIL     = -1
}Usb_Tx_Status;

typedef enum  /* The status of USB Vbus */
{
    USB_VBUS_LOW      = 0,
    USB_VBUS_HIGH     = 1,
}SC_Usb_Vbus_Status;

typedef enum
{
    USB_CLOSE      = 0,
    USB_OPEN       = 1,
}SC_Usb_Switch_Status;
/*****************************************************************************
* FUNCTION
*  sAPI_UsbVbusDetect
*
* DESCRIPTION
*  check usb Vbus
*
* PARAMETERS
*  None
*
* RETURNS
*  1 is high(connect).
*  0 is low(disconnect).
*
* NOTE
*
*****************************************************************************/
SC_Usb_Vbus_Status sAPI_UsbVbusDetect(void);

/*****************************************************************************
 * FUNCTION
 *  sAPI_UsbModemPortWrite
 *
 * DESCRIPTION
 *  Send data to usb modem Tx
 *
 * PARAMETERS
 *  *data & length
 *
 * RETURNS
 *  Usb_Tx_Status
 *
 *  USB_TX_SUCCEED  = 0          Sent successfully
 *  USB_TX_FAIL     = -1         fail in send
 *
 * NOTE
 *
 *****************************************************************************/
int sAPI_UsbModemPortWrite(unsigned char* data, unsigned long length);

/*****************************************************************************
 * FUNCTION
 *  sAPI_UsbVcomWrite
 *
 * DESCRIPTION
 *  Send data to usb Tx
 *
 * PARAMETERS
 *  *data & length
 *
 * RETURNS
 *  Usb_Tx_Status
 *
 *  USB_TX_SUCCEED  = 0          Sent successfully
 *  USB_TX_FAIL     = -1         fail in send
 *
 * NOTE
 *
 *****************************************************************************/
int sAPI_UsbVcomWrite(unsigned char* data, unsigned long length);


/*****************************************************************************
 * FUNCTION
 *  sAPI_UsbVcomRead
 *
 * DESCRIPTION
 *  Read USB data.
 *
 * PARAMETERS
 *  *data & length
 *
 * RETURNS
 *  Read data size
 *
 * NOTE
 *
 *****************************************************************************/
int sAPI_UsbVcomRead(unsigned char* data, unsigned long length);

/*****************************************************************************
 * FUNCTION
 *  sAPI_UsbSwitchFunction
 *
 * DESCRIPTION
 *  control usb switch
 *
 * PARAMETERS
 *  switch:USB_CLOSE    0
 *         USB_OPEN     1
 *
 * RETURNS
 *  int
 *
 *   0          Execution succeeded
 *  -1          Execution failed
 *
 * NOTE
 * USB is turned on by default.
 *
 *****************************************************************************/
int sAPI_UsbSwitchFunction(SC_Usb_Switch_Status swt);

typedef void (*SC_Usb_Callback)(void *para);
typedef void (*SC_Usb_CallbackEX)(int len, void *reserve);

int sAPI_UsbVcomRegisterCallbackEX(SC_Usb_CallbackEX cb, void *reserve);
int sAPI_UsbVcomRegisterCallback(SC_Usb_Callback cb);
int sAPI_ControlRndisFunction(int mode);


#endif


