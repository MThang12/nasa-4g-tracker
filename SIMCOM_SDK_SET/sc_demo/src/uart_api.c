/**
  ******************************************************************************
  * @file    cus_uart.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of uart task.
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
#include "simcom_uart.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "string.h"
#include "stdlib.h"
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>


#define UART_RX_BUFFER_SIZE     256 // RX buffer can not more than 2048
#define UART_TX_BUFFER_SIZE     128

static struct
{
    char *cache;
    char *cacheEnd;
    char *head;
    char *tail;
    int freelen;
    sFlagRef waitflag;
    sMutexRef lock;
} UIDemoDatacache;
#define UIDEMO_CACHE_MAXLEN (10240)

#ifdef HAS_DEMO
void PrintfRespData(char *buff, UINT32 length);
#endif

//#define SC_DEMO_UART3_CALLBACK_WRITE
#ifdef SC_DEMO_UART3_CALLBACK_WRITE
sTaskRef sc_uartWriteTask;
static uint8_t sc_uartWriteProcesserStack[SC_DEFAULT_THREAD_STACKSIZE];
sMsgQRef sc_uartmsgq;
#endif

static void UIDemoDAtacacheInit(void)
{
    UIDemoDatacache.cache = (char *)malloc(UIDEMO_CACHE_MAXLEN);
    UIDemoDatacache.cacheEnd = UIDemoDatacache.cache + UIDEMO_CACHE_MAXLEN;
    UIDemoDatacache.head = UIDemoDatacache.cache;
    UIDemoDatacache.tail = UIDemoDatacache.cache;
    UIDemoDatacache.freelen = UIDEMO_CACHE_MAXLEN;

    sAPI_FlagCreate(&UIDemoDatacache.waitflag);
    sAPI_MutexCreate(&UIDemoDatacache.lock, SC_FIFO);
}

void CacheDataToUIDemo(char *data, int len)
{
    int copylen = 0;
    char *from = data;

    char buf[512] = {0};

    if (NULL == UIDemoDatacache.cache)
    {
        sAPI_Debug("cache not init!!");
        return;
    }

    memset(buf, 0, 512);
    memcpy(buf, data, len);
    sAPI_Debug("cache data = [%s], len = [%d]", buf, len);

    if (len > UIDemoDatacache.freelen)
    {
        sAPI_Debug("cache overflow, drop data len:%d\n", len - UIDemoDatacache.freelen);
    }

    sAPI_MutexLock(UIDemoDatacache.lock, SC_SUSPEND);
    copylen = len <= UIDemoDatacache.freelen ? len : UIDemoDatacache.freelen;
    while (copylen > 0)
    {
#ifdef HAS_DEMO
#ifndef UART_NEW_VERSION
        PrintfRespData(from, 1);
#endif
#endif
        *UIDemoDatacache.tail++ = *from++;
        if (UIDemoDatacache.tail >= UIDemoDatacache.cacheEnd)
        {
            UIDemoDatacache.tail = UIDemoDatacache.cache;
        }
        copylen--;
        UIDemoDatacache.freelen--;
        sAPI_FlagSet(UIDemoDatacache.waitflag, 1, SC_FLAG_OR);
    }
    sAPI_MutexUnLock(UIDemoDatacache.lock);
}

int UartRead(char *buf, int len, int suspend)
{
    int ret = 0;
    int copylen = 0;
    UINT32 flag;
    char *to = buf;

    if (NULL == UIDemoDatacache.cache)
    {
        sAPI_Debug("cache not init!!");
        return 0;
    }

    if (suspend)
    {
        while (len > (UIDEMO_CACHE_MAXLEN - UIDemoDatacache.freelen))
        {
            sAPI_FlagWait(UIDemoDatacache.waitflag, 1, SC_FLAG_OR_CLEAR, &flag, SC_NO_SUSPEND);
            sAPI_FlagWait(UIDemoDatacache.waitflag, 1, SC_FLAG_OR_CLEAR, &flag, SC_SUSPEND);
        }
    }

    memset(buf, 0, len);
    sAPI_MutexLock(UIDemoDatacache.lock, SC_SUSPEND);
    copylen = len <= (UIDEMO_CACHE_MAXLEN - UIDemoDatacache.freelen) ?
              len : (UIDEMO_CACHE_MAXLEN - UIDemoDatacache.freelen);
    ret = copylen;
    while (copylen > 0)
    {
        *to++ = *UIDemoDatacache.head++;
        if (UIDemoDatacache.head >= UIDemoDatacache.cacheEnd)
        {
            UIDemoDatacache.head = UIDemoDatacache.cache;
        }
        UIDemoDatacache.freelen++;
        copylen--;
    }
    sAPI_MutexUnLock(UIDemoDatacache.lock);

    return ret;
}

int UartReadLine(char *buf, int maxLen)
{
    UINT32 flag;
    int remainLen = maxLen;
    char *to = buf;

    if (NULL == UIDemoDatacache.cache)
    {
        sAPI_Debug("cache not init!!");
        return 0;
    }

    memset(buf, 0, maxLen);
    sAPI_MutexLock(UIDemoDatacache.lock, SC_SUSPEND);
    while (remainLen > 1)
    {
        if (UIDemoDatacache.freelen >= UIDEMO_CACHE_MAXLEN)
        {
            sAPI_MutexUnLock(UIDemoDatacache.lock);
            sAPI_FlagWait(UIDemoDatacache.waitflag, 1, SC_FLAG_OR_CLEAR, &flag, SC_NO_SUSPEND);
            sAPI_FlagWait(UIDemoDatacache.waitflag, 1, SC_FLAG_OR_CLEAR, &flag, SC_SUSPEND);
            sAPI_MutexLock(UIDemoDatacache.lock, SC_SUSPEND);
        }
        *to++ = *UIDemoDatacache.head++;
        if (UIDemoDatacache.head >= UIDemoDatacache.cacheEnd)
        {
            UIDemoDatacache.head = UIDemoDatacache.cache;
        }
        UIDemoDatacache.freelen++;
        remainLen--;
        if ('\r' == *(to - 1) || '\n' == *(to - 1))
        {
            *(to - 1) = '\0';
            remainLen += 1;
            if ('\r' == *(to - 2) || '\n' == *(to - 2))
            {
                *(to - 2) = '\0';
                remainLen += 1;
            }

            // clean '\r' and '\n' from data start.
            while (UIDemoDatacache.freelen < UIDEMO_CACHE_MAXLEN)
            {
                if ('\r' == *UIDemoDatacache.head || '\n' == *UIDemoDatacache.head)
                {
                    UIDemoDatacache.freelen++;
                    UIDemoDatacache.head++;
                    if (UIDemoDatacache.head == UIDemoDatacache.cacheEnd)
                    {
                        UIDemoDatacache.head = UIDemoDatacache.cache;
                    }
                }
                else
                {
                    break;
                }
            }
            break;
        }
    }
    sAPI_MutexUnLock(UIDemoDatacache.lock);

    return (maxLen - remainLen);
}

bool isdigits(char *buf, int len)
{
    int i = 0;

    do
    {
        if (!isdigit((int)buf[i]))
        {
            return false;
        }
    }while(++i < len);

    return true;
}

int UartReadValue(void)
{
    char buf[50] = {0};
    int len = 0;

    do
    {
        len = UartReadLine(buf, 50);
        sAPI_Debug("read line buf = [%s], len = [%d]", buf, len);
    } while ((len <= 0) || !isdigits(buf, len));

    return atoi(buf);
}

/**
  * @brief  Callback for uart1 receiving event, read received data, if define SIMCOM_UI_DEMO_TO_UART1_PORT then send received data to UI demo task.
  * @param  portNumber
  * @param  pointer para
  * @note   Please do not run delay\sleep or blocking API inside this callback.
  * @retval void
  */
void UartCBFunc(SC_Uart_Port_Number portNumber, void *para)
{
    int readLen = 0;
    char *uartData = malloc(UART_RX_BUFFER_SIZE);

    readLen = sAPI_UartRead(portNumber, (UINT8 *)uartData, UART_RX_BUFFER_SIZE);
    sAPI_Debug("%s, portNumber is %d, readLen[%d].", __func__, portNumber, readLen);

#if !defined (SIMCOM_A7680C_V5_01) || !defined (SIMCOM_A7680C_V506) || !defined (SIMCOM_A7670C_V701) || !defined (SIMCOM_A7670C_V702) || !defined (SIMCOM_A7680C_V603) || !defined (SIMCOM_A7605C1_V201)
#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    CacheDataToUIDemo(uartData, readLen);
#endif
#endif

    free((void *)uartData);
    return;
}

/**
  * @brief  Extension Callback for uart1 receiving event, read fixed length of data, if define SIMCOM_UI_DEMO_TO_UART1_PORT then send received data to UI demo task.
  * @param  portNumber
  * @param  len for data to receive
  * @param  pointer para
  * @note   Please do not run delay\sleep or blocking API inside this callback.
  * @retval void
  */
void UartCBFuncEx(SC_Uart_Port_Number portNumber, int len, void *para)
{
    int readLen = 0;
    char *uartData = malloc(len);

    readLen = sAPI_UartRead(portNumber, (UINT8 *)uartData, len);
    sAPI_Debug("%s, portNumber is %d, readLen[%d].", __func__, portNumber, readLen);

#if !defined (SIMCOM_A7680C_V5_01) || !defined (SIMCOM_A7680C_V506) || !defined (SIMCOM_A7670C_V701) || !defined (SIMCOM_A7670C_V702) || !defined (SIMCOM_A7680C_V603) || !defined (SIMCOM_A7605C1_V201)
#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    CacheDataToUIDemo(uartData, readLen);
#endif
#endif

    free((void *)uartData);

    return;
}

/**
  * @brief  Callback for uart2 receiving event, read received data.
  * @param  portNumber
  * @param  pointer para
  * @note   Please do not run delay\sleep or blocking API inside this callback.
  * @retval void
  */
void Uart2CBFunc(SC_Uart_Port_Number portNumber, void *para)
{
    int readLen = 0;
    UINT8 *uart2data = malloc(UART_RX_BUFFER_SIZE);

    readLen = sAPI_UartRead(portNumber, uart2data, UART_RX_BUFFER_SIZE);
    sAPI_Debug("%s, portNumber is %d, readLen[%d].", __func__, portNumber, readLen);

    free((void *)uart2data);
    return;
}

/**
  * @brief  Extension Callback for uart2 receiving event, read fixed length of data.
  * @param  portNumber
  * @param  len for data to receive
  * @param  pointer para
  * @note   Please do not run delay\sleep or blocking API inside this callback.
  * @retval void
  */
void Uart2CBFuncEx(SC_Uart_Port_Number portNumber, int len, void *para)
{
    int readLen = 0;
    UINT8 *uart2data = malloc(len);

    readLen = sAPI_UartRead(portNumber, uart2data, len);
    sAPI_Debug("%s, portNumber is %d, readLen[%d].", __func__, portNumber, readLen);

    free((void *)uart2data);
    return;
}

/**
  * @brief  Callback for uart3 receiving event, read received data.
  * @param  portNumber
  * @param  pointer para
  * @note   Please do not run delay\sleep or blocking API inside this callback.
  * @retval void
  */
void Uart3CBFunc(SC_Uart_Port_Number portNumber, void *reserve)
{
    int readLen = 0;
    UINT8 *uart3data = malloc(UART_RX_BUFFER_SIZE);

    readLen = sAPI_UartRead(portNumber, uart3data, UART_RX_BUFFER_SIZE);
    sAPI_Debug("%s, portNumber is %d, readlen[%d].", __func__, portNumber, readLen);

    free((void *)uart3data);
    return;
}

/**
  * @brief  Extension Callback for uart3 receiving event, read fixed length of data.
  * @param  portNumber
  * @param  len for data to receive
  * @param  pointer para
  * @note   Please do not run delay\sleep or blocking API inside this callback.
  * @retval void
  */
#ifdef SC_DEMO_UART3_CALLBACK_WRITE
void sendMsgToUartMsg(SIM_MSG_T UartMsg)
{
    SC_STATUS status = SC_SUCCESS;
    SIM_MSG_T optionMsg = {0,0,0,NULL};
    optionMsg.msg_id = UartMsg.msg_id;
    optionMsg.arg1 = UartMsg.arg1;
    optionMsg.arg2 = UartMsg.arg2;

    optionMsg.arg3 = sAPI_Malloc(optionMsg.arg2+1);
    memset(optionMsg.arg3, 0, optionMsg.arg2+1);
    memcpy(optionMsg.arg3, UartMsg.arg3, UartMsg.arg2);

    status = sAPI_MsgQSend(sc_uartmsgq,&optionMsg);
    if(status != SC_SUCCESS)
    {
        sAPI_Free(optionMsg.arg3);
        sAPI_Debug("send msg error,status = [%d]",status);
    }
}

void sTask_uartWriteProcesser(void* arg)
{
    SIM_MSG_T optionMsg ={0,0,0,NULL};
    int ret = SC_UART_RETURN_CODE_OK;
    while(1)
    {
        sAPI_MsgQRecv(sc_uartmsgq,&optionMsg,SC_SUSPEND);
        if(SRV_UART != optionMsg.msg_id)
        {
            sAPI_Debug("%s,msg_id is error!!",__func__);
            break;
        }
        //sAPI_Debug("%s,port: %d length: %d",__func__,optionMsg.arg1,optionMsg.arg2);
        ret = sAPI_UartWrite(optionMsg.arg1, (UINT8 *)optionMsg.arg3, optionMsg.arg2);
        if(SC_UART_RETURN_CODE_OK != ret)
            sAPI_Debug("%s,sAPI_UartWrite is error!!",__func__);
        sAPI_Free(optionMsg.arg3);
    }

}

void sTask_uartWriteProExit(void* arg)
{
    /* delect the sc_uartmsgq */
    sAPI_MsgQDelete(sc_uartmsgq);
    /* destroy the thread sc_uartWriteTask */
    sAPI_TaskDelete(sc_uartWriteTask);
}
#endif

void Uart3CBFuncEx(SC_Uart_Port_Number portNumber, int len, void *reserve)
{
    int readLen = 0;
    UINT8 *uart3data = malloc(len);

    readLen = sAPI_UartRead(portNumber, uart3data, len);
    sAPI_Debug("%s, portNumber is %d, readlen[%d].", __func__, portNumber, readLen);

#ifdef SC_DEMO_UART3_CALLBACK_WRITE
    /*get data send to  sc_uartmsgq*/
    SIM_MSG_T uartMsg = {0,0,0,NULL};
    uartMsg.msg_id = SRV_UART;
    uartMsg.arg1 = portNumber;
    uartMsg.arg2 = readLen;
    uartMsg.arg3 = uart3data;
    sendMsgToUartMsg(uartMsg);
#endif

    free((void *)uart3data);
    return;
}

void Uart4CBFunc(SC_Uart_Port_Number portNumber, void *para)
{
#if (defined SIMCOM_A7680C_V5_01) || (defined SIMCOM_A7670C_V701) || (defined SIMCOM_A7670C_V702) || defined (SIMCOM_A7680C_V506) || defined (SIMCOM_A7680C_V801) || defined (SIMCOM_A7680C_V603) || defined (SIMCOM_A7605C1_V201) || defined (SIMCOM_A7673G_V201)
    int readLen = 0;
    char *uart4Data = malloc(UART_RX_BUFFER_SIZE);

    readLen = sAPI_UartRead(portNumber, (UINT8 *)uart4Data, UART_RX_BUFFER_SIZE);
    sAPI_Debug("%s, portNumber is %d, readLen[%d].", __func__, portNumber, readLen);

#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    CacheDataToUIDemo(uart4Data, readLen);
#endif

    free((void *)uart4Data);
#endif
    return;
}

void Uart4CBFuncEx(SC_Uart_Port_Number portNumber, int len, void *para)
{
#if (defined SIMCOM_A7680C_V5_01) || (defined SIMCOM_A7670C_V701) || (defined SIMCOM_A7670C_V702) || defined (SIMCOM_A7680C_V506) || defined (SIMCOM_A7680C_V801) || defined (SIMCOM_A7680C_V603) || defined (SIMCOM_A7605C1_V201) || defined (SIMCOM_A7673G_V201)
    int readLen = 0;
    char *uart4Data = malloc(len);

    readLen = sAPI_UartRead(portNumber, (UINT8 *)uart4Data, len);
    sAPI_Debug("%s, portNumber is %d, readLen[%d].", __func__, portNumber, readLen);

#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    CacheDataToUIDemo(uart4Data, readLen);
#endif

    free((void *)uart4Data);
#endif
    return;
}

/**
  * @brief  Configure 3 Uarts and register related callback.
  * @param  void
  * @note   SC_UART is from UART1 of module, it will be the CLI interface for the SIMCom UI demo
  * @retval void
  */
void sDemo_UartInit(void)
{
#ifndef FALCON_1803_PLATFORM
#ifdef PROJECT_TOPFLY
    SCuartConfiguration uart2Config, uart3Config, uart4Config;
#else
    SCuartConfiguration uartConfig, uart2Config, uart3Config, uart4Config;
#endif
#else
    SCuartConfiguration uartConfig, uart2Config, uart3Config;
#endif
    /*************************Configure UART again*********************************/
    /*******The user can modify the initialization configuratin of UART in here.***/
    /******************************************************************************/
#ifndef PROJECT_TOPFLY
    uartConfig.BaudRate  =  SC_UART_BAUD_115200;
    uartConfig.DataBits  =  SC_UART_WORD_LEN_8;
    uartConfig.ParityBit =  SC_UART_NO_PARITY_BITS;
    uartConfig.StopBits  =  SC_UART_ONE_STOP_BIT;
    if (sAPI_UartSetConfig(SC_UART, &uartConfig) == SC_UART_RETURN_CODE_ERROR)
    {
        sAPI_Debug("%s: Configure UART failure!!", __func__);
    }
#endif
    /*************************Configure UART2 again*********************************/
    /*******The user can modify the initialization configuratin of UART2 in here.***/
    /*******************************************************************************/
    uart2Config.BaudRate  =  SC_UART_BAUD_115200;
    uart2Config.DataBits  =  SC_UART_WORD_LEN_8;
    uart2Config.ParityBit =  SC_UART_NO_PARITY_BITS;
    uart2Config.StopBits  =  SC_UART_ONE_STOP_BIT;
    if (sAPI_UartSetConfig(SC_UART2, &uart2Config) == SC_UART_RETURN_CODE_ERROR)
    {
        sAPI_Debug("%s: Configure UART2 failure!!", __func__);
    }
    /*************************Configure UART3 again*********************************/
    /*******The user can modify the initialization configuratin of UART3 in here.***/
    /*******************************************************************************/
    uart3Config.BaudRate  =  SC_UART_BAUD_115200;
    uart3Config.DataBits  =  SC_UART_WORD_LEN_8;
    uart3Config.ParityBit =  SC_UART_NO_PARITY_BITS;
    uart3Config.StopBits  =  SC_UART_ONE_STOP_BIT;
    if (sAPI_UartSetConfig(SC_UART3, &uart3Config) == SC_UART_RETURN_CODE_ERROR)
    {
        sAPI_Debug("%s: Configure UART3 failure!!", __func__);
    }
#ifndef FALCON_1803_PLATFORM
    /*************************Configure UART4 again*********************************/
    /*******The user can modify the initialization configuratin of UART4 in here.***/
    /*******************************************************************************/
    uart4Config.BaudRate  =  SC_UART_BAUD_115200;
    uart4Config.DataBits  =  SC_UART_WORD_LEN_8;
    uart4Config.ParityBit =  SC_UART_NO_PARITY_BITS;
    uart4Config.StopBits  =  SC_UART_ONE_STOP_BIT;
    if (sAPI_UartSetConfig(SC_UART4, &uart4Config) == SC_UART_RETURN_CODE_ERROR)
    {
        sAPI_Debug("%s: Configure UART4 failure!!", __func__);
    }
#endif
    sAPI_Debug("%s: UART Configuration is complete!!\n", __func__);

    UIDemoDAtacacheInit();

#ifdef SC_DEMO_UART3_CALLBACK_WRITE
    /* create the msg fifo for callback tx*/
    SC_STATUS status;
    status = sAPI_TaskCreate(&sc_uartWriteTask,sc_uartWriteProcesserStack,SC_DEFAULT_THREAD_STACKSIZE,SC_DEFAULT_TASK_PRIORITY,"sc_uartWriteTask",sTask_uartWriteProcesser,(void *)0);
    if(SC_SUCCESS != status)
    {
        sAPI_Debug("uart write task create fail");
    }
    status = sAPI_MsgQCreate(&sc_uartmsgq, "sc_uartmsgq", sizeof(SIM_MSG_T), 12, SC_FIFO);
    if(SC_SUCCESS != status)
    {
        sAPI_Debug("msgQ create fail");
    }
#endif

#ifndef PROJECT_TOPFLY
    sAPI_UartRegisterCallback(SC_UART,  UartCBFunc);
#endif
    sAPI_UartRegisterCallback(SC_UART2, Uart2CBFunc);
    sAPI_UartRegisterCallbackEX(SC_UART3, Uart3CBFuncEx, (void *)"Uart3CBFuncEx");
#ifndef FALCON_1803_PLATFORM
    sAPI_UartRegisterCallback(SC_UART4,  Uart4CBFunc);
#endif
    sAPI_Debug("%s Task creation completed!!\n", __func__);
}


