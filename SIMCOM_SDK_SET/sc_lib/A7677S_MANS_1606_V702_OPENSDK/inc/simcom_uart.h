/**
* @file         simcom_uart.h
* @brief        SIMCom OpenLinux UART API
* @author       HT
* @date         2019/2/13
* @version      V1.0.0
* @par Copyright (c):
*               SIMCom Co.Ltd 2003-2019
* @par History: 1:Create
*
*/

#ifndef __SIMCOM_UART_H__
#define __SIMCOM_UART_H__

#include "simcom_os.h"
#include "simcom_gpio.h"

/****************************************************************************
    Define enum
*****************************************************************************/
typedef enum
{
    SC_UART_RETURN_CODE_OK = 0,
    SC_UART_RETURN_CODE_ERROR = -1
}SC_Uart_Return_Code;

typedef enum
{
    SC_UART  = 1,  //Full Function UART,except for A7680C module
    SC_UART2 = 2,  //Debug UART
    SC_UART3 = 3,  //UART3
    SC_UART4 = 4,  //UART4,full function UART port of A7680C module
    SC_UART_MAX
}SC_Uart_Port_Number;

typedef enum
{
    SC_UART_READY_READ = 1,
    SC_UART_READY_WRITE ,
    SC_UART_READ_DONE ,
    SC_UART_WRITE_DONE ,
    SC_UART_FIFO_ERROR ,
    SC_UART_TRANSMIT_ERROR ,
    SC_UART_EVENT_MAX
}SC_Uart_Event;

typedef enum  /* The status of Rx */
{
    SC_UART_RX_IDEL  = 0,
    SC_UART_RX_BUSY  = 1,
    SC_UART_RX_STATUS_MAX = -1
}SC_Uart_Rx_Status;

typedef enum
{
    SC_UART_MODE_FIFO = 0,
    SC_UART_MODE_DMA  = 1,
    SC_UART_MODE_MAX  = -1
}SC_UART_MODE;

typedef enum
{
    SC_UART_OPEN,
    SC_UART_CLOSE,
}SC_Uart_Control;

typedef enum   /* All the UART Baud Rate that the UART Package supplay */
{
    SC_UART_BAUD_300       = 300,
    SC_UART_BAUD_600       = 600,
    SC_UART_BAUD_1200      = 1200,
    SC_UART_BAUD_2400      = 2400,
    SC_UART_BAUD_3600      = 3600,
    SC_UART_BAUD_4800      = 4800,
    SC_UART_BAUD_9600      = 9600,
    SC_UART_BAUD_19200     = 19200,
    SC_UART_BAUD_38400     = 38400,
    SC_UART_BAUD_57600     = 57600,
    SC_UART_BAUD_115200    = 115200,
    SC_UART_BAUD_230400    = 230400,
    SC_UART_BAUD_460800    = 460800,
    SC_UART_BAUD_921600    = 921600,
    SC_UART_BAUD_1842000   = 1842000,
    SC_UART_BAUD_3686400   = 3686400,
    SC_UART_MAX_NUM_BAUD
}SC_UART_BaudRates;

typedef enum  /* The Word Len of the UART Frame Format  */
{
    SC_UART_WORD_LEN_5,                /* set Word Lengto to 5 Bits         */
    SC_UART_WORD_LEN_6,                /* set Word Lengto to 6 Bits         */
    SC_UART_WORD_LEN_7,                /* set Word Lengto to 7 Bits         */
    SC_UART_WORD_LEN_8                 /* set Word Lengto to 8 Bits         */
}SC_UART_WordLen;

typedef enum   /* The Stop Bits of the UART Frame Format */
{
    SC_UART_ONE_STOP_BIT,
    SC_UART_ONE_HALF_OR_TWO_STOP_BITS
}SC_UART_StopBits;

typedef enum  /* The Parity Bits of the UART Frame Format */
{
    SC_UART_NO_PARITY_BITS,
    SC_UART_ODD_PARITY_SELECT,
    SC_UART_EVEN_PARITY_SELECT = 3,
}SC_UART_ParityTBits;

/****************************************************************************
    Define struct
*****************************************************************************/
typedef struct SCuartConfiguration
{
    SC_UART_BaudRates BaudRate;  /* the baudrate of the uart(default - 115200)*/
    SC_UART_WordLen DataBits;   /* 5, 6, 7, or 8 number of data bits in the UART data frame (default - 8). */
    SC_UART_StopBits StopBits;  /* 1, 1.5 or 2 stop bits in the UART data frame (default - 1).   */
    SC_UART_ParityTBits ParityBit; /* Even, Odd or no-parity bit type in the UART data frame (default - Non). */
}SCuartConfiguration;

typedef struct {
	int msgID;
    int ticks;
	void *pArgs;
}SCclearTimerMsgq;

typedef enum
{
    CLEAR_TIMER_INFO_MSG        = 1,
    //INFO_MSG_END
}clear_timer_Info_type;

typedef enum
{
    SC_UART_RS485_DE_LOW_LEVEL   = 0,
    SC_UART_RS485_DE_HIGH_LEVEL
}SC_UartRs485DeActiveLevel;

typedef void (*SC_Uart_Callback)(SC_Uart_Port_Number port, void *para);
typedef void (*SC_Uart_CallbackEX)(SC_Uart_Port_Number port, int len, void *reserve);
typedef void (* SC_Uart_Event_Callback) (SC_Uart_Port_Number port, SC_Uart_Event events);

/*****************************************************************************
 * FUNCTION
 *  sAPI_UartWrite
 *
 * DESCRIPTION
 *  Send data to Tx
 *
 * PARAMETERS
 *  port   the port number of UART
 *  *data & length
 *
 * RETURNS
 *  SC_UART_RETURN_CODE_OK: write done        SC_UART_RETURN_CODE_ERROR: fail
 *
 * NOTE
 *
 *****************************************************************************/
SC_Uart_Return_Code sAPI_UartWrite(SC_Uart_Port_Number port, UINT8 *data, UINT32 length);

/*****************************************************************************
 * FUNCTION
 *  sAPI_UartWriteString
 *
 * DESCRIPTION
 *  Send string to Tx
 *
 * PARAMETERS
 *  port  the port number of UART
 *  data  the header address of the string
 *
 * RETURNS
 *  SC_UART_RETURN_CODE_OK: write done        SC_UART_RETURN_CODE_ERROR: fail
 *
 * NOTE
 *
 *****************************************************************************/
SC_Uart_Return_Code sAPI_UartWriteString(SC_Uart_Port_Number port, UINT8 *data);

/*****************************************************************************
 * FUNCTION
 *  sAPI_UartSetConfig
 *
 * DESCRIPTION
 *  Set the configuration of the UART, that includes baud-rate and
 *the format of the frame.
 *
 * PARAMETERS
 *  [in]port the port number of UART
 *  [in]config Pointer to UART configuration block
 *
 * RETURNS
 *  SC_UART_RETURN_CODE_OK: set done        SC_UART_RETURN_CODE_ERROR: fail
 *
 * NOTE
 *
 *****************************************************************************/
SC_Uart_Return_Code sAPI_UartSetConfig(SC_Uart_Port_Number port,const SCuartConfiguration *config);

/*****************************************************************************
 * FUNCTION
 *  sAPI_UartGetConfig
 *
 * DESCRIPTION
 *  Get the configuration of the UART, that includes baud-rate and
 *the format of the frame.
 *
 * PARAMETERS
 *  [in]port the port number of UART
 *  [out]config Pointer to UART configuration block
 *
 * RETURNS
 *  SC_UART_RETURN_CODE_OK: get done        SC_UART_RETURN_CODE_ERROR: get fail
 *
 * NOTE
 *
 *****************************************************************************/
SC_Uart_Return_Code sAPI_UartGetConfig(SC_Uart_Port_Number port,SCuartConfiguration *config);

/*****************************************************************************
 * FUNCTION
 *  sAPI_UartPrintf
 *
 * DESCRIPTION
 *  uart debug function
 *
 * PARAMETERS
 *  None
 *
 * RETURNS
 *
 *
 * NOTE
 *
 *****************************************************************************/
int sAPI_UartPrintf(const char *fmt, ...);

int baudRateIsValid(SC_UART_BaudRates rate);
int dataBitIsValid(SC_UART_WordLen databit);
int stopBitIsValid(SC_UART_StopBits stopbit);
int parityBitIsValid(SC_UART_ParityTBits paritybit);

int sAPI_UartRead(SC_Uart_Port_Number port, unsigned char *data, int len);

SC_Uart_Return_Code sAPI_UartRegisterCallbackEX(SC_Uart_Port_Number port, SC_Uart_CallbackEX cb, void *reserve);
SC_Uart_Return_Code sAPI_UartRegisterCallback(SC_Uart_Port_Number port, SC_Uart_Callback cb);

/*****************************************************************************
 * FUNCTION
 *  sAPI_UartControl
 *
 * DESCRIPTION
 *  Control UART opening or closing.
 *
 * PARAMETERS
 *  [in]port, the port number of UART.
 *  [in]ctl, the type of control.
 *
 * RETURNS
 *  SC_UART_RETURN_CODE_OK: set done        SC_UART_RETURN_CODE_ERROR: fail
 *
 * NOTE
 *
 *****************************************************************************/
SC_Uart_Return_Code sAPI_UartControl(SC_Uart_Port_Number port, SC_Uart_Control ctl);

/*****************************************************************************
 * FUNCTION
 *  sAPI_SendATCMDWaitResp
 *
 * DESCRIPTION
 *  This API used to send AT command and get response
 *
 * PARAMETERS
 *  sATPInd: channel index, Channel 10 is recommended.
 *  inStr: a string for AT command
 *  timeOut: time
 *  okFmt: The return format of the instruction when executed correctly
 *  okFlag: flag
 *  errFmt: The return format of the instruction when executed incorrectly
 *  outStr: return string about AT command
 *  outLen: the length of the string
 *
 * RETURNS
 *  0: success        other: fail
 *
 * NOTE
 *  the demo about this API is in demo_uart.c
 *
 *****************************************************************************/
int sAPI_SendATCMDWaitResp(int sATPInd,char *in_str, int timeout, char *ok_fmt,
                                    int ok_flag,char *err_fmt, char *out_str, int resplen);

/*****************************************************************************
 * FUNCTION
 *  sAPI_UartRxStatus
 *
 * DESCRIPTION
 *  Get the status of Rx, either idle or busy.
 *
 * PARAMETERS
 *  [in]port, the port number of UART
 *  [in]timeout, a threshold that If Rx exceeds the threshold and no data is received,
 *Rx is idle, otherwise Rx is busy.
 *
 * RETURNS
 *  SC_UART_RX_IDEL: 0       SC_UART_RX_BUSY: 1
 *
 * NOTE
 *
 *****************************************************************************/
SC_Uart_Rx_Status sAPI_UartRxStatus(SC_Uart_Port_Number port, int timeout);

/*****************************************************************************
 * FUNCTION
 *  sAPI_UartModeControl
 *
 * DESCRIPTION
 *  Select DMA or FIFO mode. Used before sAPI_UartSetConfig. Defaut is DMA Mode.
 *
 * PARAMETERS
 *  [in]mode, the mode of UART.  SC_UART_MODE_DMA: 1 , SC_UART_MODE_FIFO: 0.
 *
 * RETURNS
 *  SC_UART_RETURN_CODE_OK:0 set done        SC_UART_RETURN_CODE_ERROR:-1 fail
 *
 * NOTE
 *
 *****************************************************************************/
SC_Uart_Return_Code sAPI_UartModeControl(SC_UART_MODE mode);


/*****************************************************************************
 * FUNCTION
 *  sAPI_UartGetStatus
 *
 * DESCRIPTION
 *  Get the UART status.
 *
 * PARAMETERS
 *  [in]port the port number of UART.
 *
 * RETURNS
 *  SC_UART_RETURN_CODE_OK:0 UART_OPEN        SC_UART_RETURN_CODE_ERROR:-1 UART_CLOSE
 *
 * NOTE
 *
 *****************************************************************************/
SC_Uart_Return_Code sAPI_UartGetStatus(SC_Uart_Port_Number port);

/*****************************************************************************
 * FUNCTION
 *  sAPI_UartEventCallback
 *
 * DESCRIPTION
 *  bind UART event callback.
 *
 * PARAMETERS
 *  [in]Callback of UART response event.
 *
 * RETURNS
 *  SC_UART_RETURN_CODE_OK:0 UART_OPEN        SC_UART_RETURN_CODE_ERROR:-1 UART_CLOSE
 *
 * NOTE
 *
 *****************************************************************************/
SC_Uart_Return_Code sAPI_UartEventCallback(SC_Uart_Event_Callback cb);

SC_GPIOReturnCode sAPI_UartRs485DePinAssign(SC_Uart_Port_Number port, unsigned int GpinNum);
SC_GPIOReturnCode sAPI_UartRs485DePinAssignEx(SC_Uart_Port_Number port, unsigned int GpinNum, SC_UartRs485DeActiveLevel DeActiveLevel);

#endif