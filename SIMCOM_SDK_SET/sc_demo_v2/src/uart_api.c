/**
  ******************************************************************************
  * @file    uart_api.c
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
#include "sal_os.h"
#include "sal_log.h"
#include "pub_cache.h"
#include "demo_protocol_parser.h"


#define LOG(...)       sal_log(__VA_ARGS__)
#define LOG_ERROR(...) sal_log_error("[DEMO-INIT] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-INIT] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-INIT] " __VA_ARGS__)

#define DEMO_UART_CACHE_SIZE SAL_1K
#define DEMO_UART_READ_BUFFER_SIZE SAL_512
static pub_cache_p g_uart_cache;
static bool g_uart_inited;


int demo_uart_read(char *buf, unsigned int size, int timeout_ms)
{
    if (!g_uart_cache || false == g_uart_inited)
    {
        LOG_ERROR("uart inited:[%d], uart cache:[%p]",
                  g_uart_inited, g_uart_cache);
        return -1;
    }

    return pub_cache_read_ex(g_uart_cache, buf, size, NULL, 0, timeout_ms);
}

int demo_uart_readline(char *buf, unsigned int buf_size, int timeout_ms)
{
    char *p = buf;
    int size = 0;
    int ret = 0;

    if (!g_uart_cache || false == g_uart_inited)
    {
        LOG_ERROR("uart inited:[%d], uart cache:[%p]",
                  g_uart_inited, g_uart_cache);
        return -1;
    }

    while (1)
    {
        ret = pub_cache_read_ex(g_uart_cache, p, 1, NULL, 0, timeout_ms);

        if (ret <= 0) break;
        if ('\n' == *p) break;

        size += ret;
        p += ret;

        if (size >= buf_size) break;
    }

    return size;
}

int demo_uart_send(const char *buf, unsigned int size)
{
    SC_Uart_Return_Code ret = SC_UART_RETURN_CODE_OK;

    if (!g_uart_cache || false == g_uart_inited)
    {
        LOG_ERROR("uart inited:[%d], uart cache:[%p]",
                  g_uart_inited, g_uart_cache);
        return -1;
    }

    LOG_TRACE("uart send -> %d:[%s]", size, buf);

#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    #ifdef HAS_UART
        #if (defined SIMCOM_A7680C_V5_01) || (defined SIMCOM_A7670C_V701) || (defined SIMCOM_A7670C_V702) || defined (SIMCOM_A7680C_V506) || defined (SIMCOM_A7680C_V603) || defined (SIMCOM_A7605C1_V201)
            ret = sAPI_UartWrite(SC_UART4, (UINT8 *)buf, (UINT32)size);
        #else
            ret = sAPI_UartWrite(SC_UART, (UINT8 *)buf, (UINT32)size);
        #endif
    #endif
#elif defined(SIMCOM_UI_DEMO_TO_USB_AT_PORT)
    #ifdef HAS_USB
        sAPI_UsbVcomWrite((UINT8 *)buf, (UINT32)size);
    #endif
#endif

    if (SC_UART_RETURN_CODE_OK != ret)
    {
        LOG_ERROR("UART send fail");
    }

    return SC_UART_RETURN_CODE_OK == ret ? 0 : -1;
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
    int readlen = 0;
    char *uartData = sal_malloc(DEMO_UART_READ_BUFFER_SIZE);

    readlen = sAPI_UartRead(portNumber, (unsigned char *)uartData, DEMO_UART_READ_BUFFER_SIZE);
    LOG_INFO("%s, portNumber is %d, readlen[%d].", __func__, portNumber, readlen);

#if !defined (SIMCOM_A7680C_V5_01) || !defined (SIMCOM_A7680C_V506) || !defined (SIMCOM_A7670C_V701) || !defined (SIMCOM_A7670C_V702) || !defined (SIMCOM_A7680C_V603) || !defined (SIMCOM_A7605C1_V201)
#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    int savelen = 0;
    if (g_uart_cache)
        savelen = pub_cache_write(g_uart_cache, uartData, readlen);

    if (savelen != readlen)
    {
        LOG_ERROR("drop data size: %d, readsize:%d, savesize:%d",
                  readlen - savelen,
                  readlen,
                  savelen);
    }
#endif
#endif

    sal_free(uartData);
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
    int readlen = 0;
    char *uartData = sal_malloc(len);

    readlen = sAPI_UartRead(portNumber, (unsigned char *)uartData, len);
    LOG_INFO("%s, portNumber is %d, readlen[%d].", __func__, portNumber, readlen);

#if !defined (SIMCOM_A7680C_V5_01) || !defined (SIMCOM_A7680C_V506) || !defined (SIMCOM_A7670C_V701) || !defined (SIMCOM_A7670C_V702) || !defined (SIMCOM_A7680C_V603) || !defined (SIMCOM_A7605C1_V201)
#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    int savelen = 0;
    if (g_uart_cache)
        savelen = pub_cache_write(g_uart_cache, uartData, readlen);

    if (savelen != readlen)
    {
        LOG_ERROR("drop data size: %d, readsize:%d, savesize:%d",
                  readlen - savelen,
                  readlen,
                  savelen);
    }
#endif
#endif

    sal_free(uartData);
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
    int readlen = 0;
    UINT8 *uart2data = sal_malloc(DEMO_UART_READ_BUFFER_SIZE);

    readlen = sAPI_UartRead(portNumber, uart2data, DEMO_UART_READ_BUFFER_SIZE);
    LOG_INFO("%s, portNumber is %d, readlen[%d].", __func__, portNumber, readlen);

    sal_free(uart2data);
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
    int readlen = 0;
    UINT8 *uart2data = sal_malloc(len);

    readlen = sAPI_UartRead(portNumber, uart2data, len);
    LOG_INFO("%s, portNumber is %d, readlen[%d].", __func__, portNumber, readlen);

    sal_free(uart2data);
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
    int readlen = 0;
    UINT8 *uart3data = sal_malloc(DEMO_UART_READ_BUFFER_SIZE);

    readlen = sAPI_UartRead(portNumber, uart3data, DEMO_UART_READ_BUFFER_SIZE);
    LOG_INFO("%s, portNumber is %d, readlen[%d].", __func__, portNumber, readlen);

    sal_free(uart3data);
}

/**
  * @brief  Extension Callback for uart3 receiving event, read fixed length of data.
  * @param  portNumber
  * @param  len for data to receive
  * @param  pointer para
  * @note   Please do not run delay\sleep or blocking API inside this callback.
  * @retval void
  */
void Uart3CBFuncEx(SC_Uart_Port_Number portNumber, int len, void *reserve)
{
    int readlen = 0;
    UINT8 *uart3data = sal_malloc(len);

    readlen = sAPI_UartRead(portNumber, uart3data, len);
    LOG_INFO("%s, portNumber is %d, readlen[%d].", __func__, portNumber, readlen);

    sal_free(uart3data);
}

void Uart4CBFunc(SC_Uart_Port_Number portNumber, void *para)
{
    int readlen = 0;
    char *uart4Data = sal_malloc(DEMO_UART_READ_BUFFER_SIZE);

    readlen = sAPI_UartRead(portNumber, (unsigned char *)uart4Data, DEMO_UART_READ_BUFFER_SIZE);
    LOG_INFO("%s, portNumber is %d, readlen[%d].", __func__, portNumber, readlen);

#if (defined SIMCOM_A7680C_V5_01) || (defined SIMCOM_A7670C_V701) || (defined SIMCOM_A7670C_V702) || defined (SIMCOM_A7680C_V506) || defined (SIMCOM_A7680C_V801) || defined (SIMCOM_A7680C_V603) || defined (SIMCOM_A7605C1_V201)
#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    int savelen = 0;
    if (g_uart_cache)
        savelen = pub_cache_write(g_uart_cache, uart4Data, readlen);

    if (savelen != readlen)
    {
        LOG_ERROR("drop data size: %d, readsize:%d, savesize:%d",
                  readlen - savelen,
                  readlen,
                  savelen);
    }
#endif
#endif

    sal_free(uart4Data);
}

void Uart4CBFuncEx(SC_Uart_Port_Number portNumber, int len, void *para)
{
    int readlen = 0;
    char *uart4Data = sal_malloc(len);

    readlen = sAPI_UartRead(portNumber, (UINT8 *)uart4Data, len);
    LOG_INFO("%s, portNumber is %d, readlen[%d].", __func__, portNumber, readlen);

#if (defined SIMCOM_A7680C_V5_01) || (defined SIMCOM_A7670C_V701) || (defined SIMCOM_A7670C_V702) || defined (SIMCOM_A7680C_V506) || defined (SIMCOM_A7680C_V801) || defined (SIMCOM_A7680C_V603) || defined (SIMCOM_A7605C1_V201)
#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    int savelen = 0;
    if (g_uart_cache)
        savelen = pub_cache_write(g_uart_cache, uart4Data, readlen);

    if (savelen != readlen)
    {
        LOG_ERROR("drop data size: %d, readsize:%d, savesize:%d",
                  readlen - savelen,
                  readlen,
                  savelen);
    }
#endif
#endif

    sal_free(uart4Data);
}

/**
  * @brief  Configure 3 Uarts and register related callback.
  * @param  void
  * @note   SC_UART is from UART1 of module, it will be the CLI interface for the SIMCom UI demo
  * @retval void
  */
int demo_uart_init(void)
{
    g_uart_cache = pub_cache_create(DEMO_UART_CACHE_SIZE, CACHE_TYPE_BUFFER, CACHE_SYNC_R);
    if (!g_uart_cache)
    {
        LOG_ERROR("uart cache create fail");
        return -1;
    }

#ifndef FALCON_1803_PLATFORM
    SCuartConfiguration uartConfig, uart2Config, uart3Config, uart4Config;
#else
    SCuartConfiguration uartConfig, uart2Config, uart3Config;
#endif
    /*************************Configure UART again*********************************/
    /*******The user can modify the initialization configuratin of UART in here.***/
    /******************************************************************************/
    uartConfig.BaudRate  =  SC_UART_BAUD_115200;
    uartConfig.DataBits  =  SC_UART_WORD_LEN_8;
    uartConfig.ParityBit =  SC_UART_NO_PARITY_BITS;
    uartConfig.StopBits  =  SC_UART_ONE_STOP_BIT;
    if (sAPI_UartSetConfig(SC_UART, &uartConfig) == SC_UART_RETURN_CODE_ERROR)
    {
        LOG_ERROR("%s: Configure UART failure!!", __func__);
    }

    /*************************Configure UART2 again*********************************/
    /*******The user can modify the initialization configuratin of UART2 in here.***/
    /*******************************************************************************/
    uart2Config.BaudRate  =  SC_UART_BAUD_115200;
    uart2Config.DataBits  =  SC_UART_WORD_LEN_8;
    uart2Config.ParityBit =  SC_UART_NO_PARITY_BITS;
    uart2Config.StopBits  =  SC_UART_ONE_STOP_BIT;
    if (sAPI_UartSetConfig(SC_UART2, &uart2Config) == SC_UART_RETURN_CODE_ERROR)
    {
        LOG_ERROR("%s: Configure UART2 failure!!", __func__);
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
        LOG_ERROR("%s: Configure UART3 failure!!", __func__);
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
        LOG_ERROR("%s: Configure UART4 failure!!", __func__);
    }
#endif
    LOG_INFO("%s: UART Configuration is complete!!\n", __func__);

    sAPI_UartRegisterCallback(SC_UART,  UartCBFunc);
    sAPI_UartRegisterCallback(SC_UART2, Uart2CBFunc);
    sAPI_UartRegisterCallbackEX(SC_UART3, Uart3CBFuncEx, (void *)"Uart3CBFuncEx");
#ifndef FALCON_1803_PLATFORM
    sAPI_UartRegisterCallback(SC_UART4,  Uart4CBFunc);
#endif
    g_uart_inited = true;

    LOG_INFO("%s Task creation completed!!\n", __func__);

    return 0;
}


