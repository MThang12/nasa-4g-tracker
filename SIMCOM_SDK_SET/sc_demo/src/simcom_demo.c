/**
  ******************************************************************************
  * @file    simcom_demo.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source code for all OpenSDK demo task management with UI.
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

#include "simcom_demo.h"
#include "simcom_os.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "simcom_debug.h"
#include "simcom_common.h"
#ifdef HAS_USB
#include "simcom_usb_vcom.h"
#endif
#ifdef HAS_UART
#include "simcom_uart.h"
#include "uart_api.h"
#endif


#ifdef FEATURE_SIMCOM_MQTT
sMsgQRef urc_mqtt_msgq_1;
#endif
sTaskRef simcomUIProcesser;

static SC_DEMO_T gDemoList[SC_DEMO_MAX];

void scDemoListInit(void)
{
#ifdef HAS_DEMO_NETWORK
    gDemoList[SC_DEMO_FOR_NETWORK].description = "NETWORK";
    gDemoList[SC_DEMO_FOR_NETWORK].cb = NetWorkDemo;
#endif
#ifdef HAS_DEMO_SIMCARD
    gDemoList[SC_DEMO_FOR_SIMCARD].description = "SIMCARD";
    gDemoList[SC_DEMO_FOR_SIMCARD].cb = SimcardDemo;
#endif
#ifdef HAS_DEMO_SMS
    gDemoList[SC_DEMO_FOR_SMS].description = "SMS";
    gDemoList[SC_DEMO_FOR_SMS].cb = SMSDemo;
#endif
#ifdef HAS_DEMO_UART
    gDemoList[SC_DEMO_FOR_UART].description = "UART";
    gDemoList[SC_DEMO_FOR_UART].cb = UartDemo;
#endif
#ifdef HAS_DEMO_USB
    gDemoList[SC_DEMO_FOR_USB].description = "USB";
    gDemoList[SC_DEMO_FOR_USB].cb = NULL;
#endif
#ifdef HAS_DEMO_GPIO
    gDemoList[SC_DEMO_FOR_GPIO].description = "GPIO";
    gDemoList[SC_DEMO_FOR_GPIO].cb = GpioDemo;
#endif
#ifdef HAS_DEMO_PMU
    gDemoList[SC_DEMO_FOR_PMU].description = "PMU";
    gDemoList[SC_DEMO_FOR_PMU].cb = PMUDemo;
#endif
#ifdef HAS_DEMO_I2C
    gDemoList[SC_DEMO_FOR_I2C].description = "I2C";
    gDemoList[SC_DEMO_FOR_I2C].cb = I2cDemo;
#endif
#ifdef HAS_DEMO_AUDIO
    gDemoList[SC_DEMO_FOR_AUDIO].description = "AUDIO";
    gDemoList[SC_DEMO_FOR_AUDIO].cb = AudioDemo;
#endif
#ifdef HAS_DEMO_FS
    gDemoList[SC_DEMO_FOR_FILE_SYSTEM].description = "FILE SYSTEM";
    gDemoList[SC_DEMO_FOR_FILE_SYSTEM].cb = FsDemo;
#endif
#ifdef HAS_DEMO_TCPIP
    gDemoList[SC_DEMO_FOR_TCP_IP].description = "TCPIP";
    gDemoList[SC_DEMO_FOR_TCP_IP].cb = TcpipDemo;
#endif
#ifdef HAS_DEMO_HTTPS
    gDemoList[SC_DEMO_FOR_HTTP_HTTPS].description = "HTTPS";
    gDemoList[SC_DEMO_FOR_HTTP_HTTPS].cb = HttpsDemo;
#endif
#ifdef HAS_DEMO_FTPS
    gDemoList[SC_DEMO_FOR_FTP_FTPS].description = "FTPS";
    gDemoList[SC_DEMO_FOR_FTP_FTPS].cb = FtpsDemo;
#endif
#ifdef HAS_DEMO_MQTTS
    gDemoList[SC_DEMO_FOR_MQTT_MQTTS].description = "MQTTS";
    gDemoList[SC_DEMO_FOR_MQTT_MQTTS].cb = MqttDemo;
#endif
#ifdef HAS_DEMO_SSL
    gDemoList[SC_DEMO_FOR_SSL].description = "SSL";
    gDemoList[SC_DEMO_FOR_SSL].cb = SslDemo;
#endif
#ifdef HAS_DEMO_OTA
    gDemoList[SC_DEMO_FOR_OTA].description = "FOTA";
    gDemoList[SC_DEMO_FOR_OTA].cb = FotaDemo;
#endif
#ifdef HAS_DEMO_LBS
    gDemoList[SC_DEMO_FOR_LBS].description = "LBS";
    gDemoList[SC_DEMO_FOR_LBS].cb = LbsDemo;
#endif

#ifdef HAS_DEMO_SJDR
    gDemoList[SC_DEMO_FOR_SJDR].description = "JAMMING DETECT";
    gDemoList[SC_DEMO_FOR_SJDR].cb = JamDectDemo;
#endif

#ifdef HAS_DEMO_NTP
    gDemoList[SC_DEMO_FOR_NTP].description = "NTP";
    gDemoList[SC_DEMO_FOR_NTP].cb = NtpDemo;
#endif
#ifdef HAS_DEMO_HTP
    gDemoList[SC_DEMO_FOR_HTP].description = "HTP";
    gDemoList[SC_DEMO_FOR_HTP].cb = HtpDemo;
#endif
#ifdef HAS_DEMO_INTERNET_SERVICE
    gDemoList[SC_DEMO_FOR_INTERNET_SERVICE].description = "INTERNET_SERVICE";
    gDemoList[SC_DEMO_FOR_INTERNET_SERVICE].cb = NULL;
#endif
#ifdef HAS_DEMO_TTS
    gDemoList[SC_DEMO_FOR_TTS].description = "TTS";
    gDemoList[SC_DEMO_FOR_TTS].cb = TTSDemo;
#endif
#ifdef HAS_DEMO_CALL
    gDemoList[SC_DEMO_FOR_CALL].description = "CALL";
    gDemoList[SC_DEMO_FOR_CALL].cb = CALLDemo;
#endif
#ifdef HAS_DEMO_WIFI
    gDemoList[SC_DEMO_FOR_WIFI].description = "WIFI";
    gDemoList[SC_DEMO_FOR_WIFI].cb = WIFIDemo;
#endif
#ifdef HAS_DEMO_GNSS
    gDemoList[SC_DEMO_FOR_GNSS].description = "GNSS";
    gDemoList[SC_DEMO_FOR_GNSS].cb = GNSSDemo;
#endif
#ifdef HAS_DEMO_LCD
    gDemoList[SC_DEMO_FOR_LCD].description = "LCD";
    gDemoList[SC_DEMO_FOR_LCD].cb = LcdDemo;
#endif
#ifdef HAS_DEMO_RTC
    gDemoList[SC_DEMO_FOR_RTC].description = "RTC";
    gDemoList[SC_DEMO_FOR_RTC].cb = RTCDemo;
#endif
#ifdef HAS_DEMO_FLASH
    gDemoList[SC_DEMO_FOR_FLASH].description = "FLASH";
    gDemoList[SC_DEMO_FOR_FLASH].cb = FlashRWdemo;
#endif
#ifdef HAS_DEMO_FS_OLD
    gDemoList[SC_DEMO_FOR_FILE_SYSTEM_OLD].description = "FILE SYSTEM Compatible API";
    gDemoList[SC_DEMO_FOR_FILE_SYSTEM_OLD].cb = NULL;
#endif
#ifdef HAS_DEMO_SPI
    gDemoList[SC_DEMO_FOR_SPI].description = "SPI";
    gDemoList[SC_DEMO_FOR_SPI].cb = SpiDemo;

    gDemoList[SC_DEMO_FOR_SPI_NOR].description = "SPI_NOR";
    gDemoList[SC_DEMO_FOR_SPI_NOR].cb = SpiNorDemo;

    gDemoList[SC_DEMO_FOR_SPI_NAND].description = "SPI_NAND";
    gDemoList[SC_DEMO_FOR_SPI_NAND].cb = SpiNandDemo;
#endif
#ifdef HAS_DEMO_CAM
    gDemoList[SC_DEMO_FOR_CAM].description = "CAM";
    gDemoList[SC_DEMO_FOR_CAM].cb = CamDemo;
#endif
#ifdef HAS_DEMO_SYS
    gDemoList[SC_DEMO_FOR_SYS].description = "SYS";
    gDemoList[SC_DEMO_FOR_SYS].cb = SysDemo;
#endif
#ifdef HAS_DEMO_BLE
    gDemoList[SC_DEMO_FOR_BLE].description = "BLE";
    gDemoList[SC_DEMO_FOR_BLE].cb = BLEDemo;
#endif
#ifdef HAS_DEMO_BT
    gDemoList[SC_DEMO_FOR_BT].description = "BT";
    gDemoList[SC_DEMO_FOR_BT].cb = BTDemo;
#endif
#ifdef HAS_DEMO_BT_STACK
    gDemoList[SC_DEMO_FOR_BTSTACK].description = "BT/BLE";
    gDemoList[SC_DEMO_FOR_BTSTACK].cb = BTDemo;
#endif

#ifdef HAS_DEMO_APP_DOWNLOAD
    gDemoList[SC_DEMO_FOR_APP_DOWNLOAD].description = "APP_DOWNLOAD";
    gDemoList[SC_DEMO_FOR_APP_DOWNLOAD].cb = AppDownloadDemo;
#endif
#ifdef HAS_DEMO_APP_UPDATE
    gDemoList[SC_DEMO_FOR_APP_UPDATE_FOR_NVM].description = "APP_UPDATE_FOR_NVM";
    gDemoList[SC_DEMO_FOR_APP_UPDATE_FOR_NVM].cb = AppUpdateDemo;
#endif
#ifdef HAS_DEMO_PWM
    gDemoList[SC_DEMO_FOR_PWM].description = "PWM";
    gDemoList[SC_DEMO_FOR_PWM].cb = PwmDemo;
#endif
#ifdef HAS_DEMO_POC
    gDemoList[SC_DEMO_FOR_POC].description = "POC";
    gDemoList[SC_DEMO_FOR_POC].cb = POCDemo;
#endif
#ifdef HAS_DEMO_WTD
    gDemoList[SC_DEMO_FOR_WTD].description = "WTD";
    gDemoList[SC_DEMO_FOR_WTD].cb = WTDDemo;
#endif
#ifdef HAS_DEMO_PING
    gDemoList[SC_DEMO_FOR_PING].description = "PING";
    gDemoList[SC_DEMO_FOR_PING].cb = PingDemo;
#endif
#ifdef HAS_SM2
    gDemoList[SC_DEMO_FOR_SM2].description = "SM2";
    gDemoList[SC_DEMO_FOR_SM2].cb = SM2Demo;
#endif
#ifdef HAS_ZLIB
    gDemoList[SC_DEMO_FOR_ZLIB].description = "ZLIB";
    gDemoList[SC_DEMO_FOR_ZLIB].cb = ZLIBDemo;
#endif
#ifdef HAS_CJSON
    gDemoList[SC_DEMO_FOR_CJSON].description = "CJSON";
    gDemoList[SC_DEMO_FOR_CJSON].cb = CjsonDemo;
#endif
#ifdef HAS_MBEDTLS
    gDemoList[SC_DEMO_FOR_MBEDTLS].description = "MBEDTLS";
    gDemoList[SC_DEMO_FOR_MBEDTLS].cb = MbedTLSDemo;
#endif

#ifdef HAS_DEMO_EXFLASHMOUNTFS
    gDemoList[SC_DEMO_FOR_MBEDTLS].description = "exflashmountfs";
    gDemoList[SC_DEMO_FOR_MBEDTLS].cb = exflashmountfsDemo;
#endif

}

/**
  * @brief  Print string to UART1 or USB AT port.
  * @param  format,data pointer
  * @note   Please define SIMCOM_UI_DEMO_TO_UART1_PORT or SIMCOM_UI_DEMO_TO_USB_AT_PORT in advance.
  * @retval void
  */
void PrintfResp(char *format)
{
#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    #ifdef HAS_UART
        #if (defined SIMCOM_A7680C_V5_01) || (defined SIMCOM_A7670C_V701) || (defined SIMCOM_A7670C_V702) || defined (SIMCOM_A7680C_V506) || defined (SIMCOM_A7680C_V603) || defined (SIMCOM_A7605C1_V201) || defined(SIMCOM_A7673G_V201)
            sAPI_UartWrite(SC_UART4, (UINT8 *)format, strlen(format));
        #else
            sAPI_UartWrite(SC_UART, (UINT8 *)format, strlen(format));
        #endif
    #endif
#else
    #ifdef HAS_USB
        sAPI_UsbVcomWrite((UINT8 *)format, strlen(format));
    #endif
#endif
}

void PrintfRespData(char *buff, UINT32 length)
{
#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    #ifdef HAS_UART
        #if (defined SIMCOM_A7680C_V5_01) || (defined SIMCOM_A7670C_V701) || (defined SIMCOM_A7670C_V702) || defined (SIMCOM_A7680C_V506) || defined (SIMCOM_A7680C_V603) || defined (SIMCOM_A7605C1_V201) || defined (SIMCOM_A7673G_V201)
            sAPI_UartWrite(SC_UART4, (UINT8 *)buff, length);
        #else
            sAPI_UartWrite(SC_UART, (UINT8 *)buff, length);
        #endif
    #endif
#else
    #ifdef HAS_USB
        sAPI_UsbVcomWrite((UINT8 *)buff, length);
    #endif
#endif
}

void PrintfRespHexData(char *buff, UINT32 length)
{
    int i = 0;
    char *buff_hex = NULL;
    char tem[3] = {0};

    buff_hex = malloc(length * 2);
    if (NULL == buff_hex)
    {
        return;
    }
    memset(buff_hex, 0, length * 2);

    for (i = 0; i < length; i++)
    {
        snprintf(tem, 3, "%.2x", buff[i]);
        memcpy(&buff_hex[i * 2], tem, 2);
    }

#ifdef SIMCOM_UI_DEMO_TO_UART1_PORT
    #ifdef HAS_UART
        #if (defined SIMCOM_A7680C_V5_01) || (defined SIMCOM_A7670C_V701) || (defined SIMCOM_A7670C_V702) || defined (SIMCOM_A7680C_V506) || defined (SIMCOM_A7680C_V603) || defined (SIMCOM_A7605C1_V201)
            sAPI_UartWrite(SC_UART4, (UINT8 *)buff_hex, length * 2);
        #else
            sAPI_UartWrite(SC_UART, (UINT8 *)buff_hex, length * 2);
        #endif
    #endif
#else
    #ifdef HAS_USB
        sAPI_UsbVcomWrite((UINT8 *)buff_hex, length * 2);
    #endif
#endif
}

/**
  * @brief  Print operation menu.
  * @param  options_list,operation list
  * @param  array_size,size of options_list
  * @note   Please define SIMCOM_UI_DEMO_TO_UART1_PORT or SIMCOM_UI_DEMO_TO_USB_AT_PORT in advance.
  * @retval void
  */
void PrintfOptionMenu(char *options_list[], int array_size)
{
    UINT32 i = 0;
    sAPI_Debug("array_size = [%d]", array_size);
    char menu[80] = {0};
    PrintfResp("\r\n************************************************************\r\n");
    for (i = 0; i < (array_size / 2); i++)
    {
        memset(menu, 0, 80);
        snprintf(menu, 80, "%-30s%-30s", options_list[2 * i], options_list[2 * i + 1]);
        PrintfResp(menu);
        PrintfResp("\r\n");
    }

    if (array_size % 2 != 0)
    {
        memset(menu, 0, 80);
        snprintf(menu, 80, "%s", options_list[array_size - 1]);
        PrintfResp(menu);
        PrintfResp("\r\n");
    }
    PrintfResp("************************************************************\r\n");

}

void PrintfMainMenu(void)
{
    int i = 0;
    int counter = 0;
    char menu[40] = {0};

    PrintfResp("\r\n************************************************************\r\n");
    for (i = 0; i < SC_DEMO_MAX; i++)
    {
        if (gDemoList[i].cb != NULL)
        {
            memset(menu, 0, 40);
            snprintf(menu, 40, "%2d. %-30s", i, gDemoList[i].description);
            PrintfResp(menu);
            counter++;
        }

        if (counter >= 2)
        {
            PrintfResp("\r\n");
            counter = 0;
        }
    }
    PrintfResp("************************************************************\r\n");
}


/**
  * @brief  SIMCom UI demo processer.
  * @param  arg
  * @note   Please select demo according to CLI.
  * @retval void
  */
void sTask_SimcomUIProcesser(void *arg)
{
    UINT32 opt = 0;
    char *note = "Please select an option to test from the items listed below.\n";

    while (1)
    {
        PrintfResp(note);
        PrintfMainMenu();

        opt = UartReadValue();
        sAPI_Debug("opt = [%d]", opt);

        if (opt >= 0 && opt < SC_DEMO_MAX && gDemoList[opt].cb != NULL)
        {
            sAPI_Debug("call %s demo!!", gDemoList[opt].description);
            gDemoList[opt].cb();
        }
        else
        {
            sAPI_Debug("opt: %d, not support!", opt);
        }
    }
}

/**
  * @brief  Create SIMCom UI demo task.
  * @param  void
  * @note   UI demo based on message(queue) with blocking method.
  * @retval void
  */
void sAPP_SimcomUIDemo(void)
{
    SC_STATUS status;

    scDemoListInit();

#ifdef FEATURE_SIMCOM_MQTT
    status = sAPI_MsgQCreate(&urc_mqtt_msgq_1, "urc_mqtt_msgq_1", (sizeof(SIM_MSG_T)), 4,
                             SC_FIFO);        //msgQ for subscribed data transfer
    if (status != SC_SUCCESS)
    {
        sAPI_Debug("message queue creat err!\n");
    }
    printf("%s,sTask_SimcomUIProcesser", __func__);
#endif

    void *simcomUIProcesserStack = (void *)malloc(60 * 1024);
    if (!simcomUIProcesserStack)
    {
        printf("malloc simcomUIProcesserStack fail!");
        return;
    }
    status = sAPI_TaskCreate(&simcomUIProcesser,
                             simcomUIProcesserStack,
                             1024 * 60,
                             100,
                             "simcomUIProcesser",
                             sTask_SimcomUIProcesser,
                             (void *)0);
    if (SC_SUCCESS != status)
    {
        sAPI_Debug("task create fail");
    }
}

