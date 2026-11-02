/**
  ******************************************************************************
  * @file    demo_fota.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of fota function,  this is used to update core FW.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "simcom_fota_download.h"
#include "simcom_uart.h"
#include "simcom_common.h"
#ifndef LTEONLY_THIN_SINGLE_SIM_2MFLASH
#include "simcom_file.h"
#endif
#include "uart_api.h"
#ifdef FEATURE_SIMCOM_NORFLASH_MINI_LFOTA
#include "local_fota.h"
#endif

#define LOG_ERR()         do{printf("%s error, line: %d\r\n", __func__, __LINE__);}while (0)


extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);


enum SC_FOTA_SWITCH
{
    SC_FTP_FOTA = 1,

    SC_HTTP_FOTA,

#ifndef CRANEL_OTA
    SC_LOCAL_FOTA,
#endif

#ifdef FEATURE_SIMCOM_NORFLASH_MINI_LFOTA
    SC_MINI_LOCAL_FOTA,
#endif

    SC_FOTA_BACK = 99
};

/**
  * @brief  Fota operation callback to indicated the downloading finishing percentage.
  * @param  void
  * @note   isok will be used to indicate the download result.
  * @retval void
  */
int fotacb(int isok)
{
    if(isok ==100)
        PrintfResp("fota download is successful, Please reset the module.\r\n");
    else
        PrintfResp("fota download failed, try again\r\n");
    return 0;
}

/**
  * @brief  Fota demo
  * @param  void
  * @note   suppor HTTP and FTP\local update. For local update need sAPI_FsSwitchDir to change director
  * @retval void
  */
void FotaDemo(void)
{
    UINT32 opt = 0;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    #ifndef CRANEL_OTA
    char *options_list[] =
    {
        "1. FTP fota",

        "2. HTTP fota",

        "3. LOCAL fota",
#ifndef FALCON_1803_PLATFORM
        "8. Check Current mini sys Status",
#endif
        "99. back",
    };
    #else
        char *options_list[] =
    {
        "1. FTP fota",

        "2. HTTP fota",

    #ifdef FEATURE_SIMCOM_NORFLASH_MINI_LFOTA
        "3. MINI LOCAL fota",
    #endif
#ifndef FALCON_1803_PLATFORM
        "8. Check Current mini sys Status",
#endif
        "99. back",
    };
    #endif

    while (1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list, sizeof(options_list) / sizeof(options_list[0]));

        opt = UartReadValue();

        switch (opt)
        {

            case SC_FTP_FOTA:
            {
                struct SC_FotaApiParam param = {0};
                strcpy(param.host, "183.230.174.137:6121/zyk_fota/system_patch.bin");
                strcpy(param.username, "yangkui.zhang");
                strcpy(param.password, "simcom");
                param.mode = 0;//ftp
                param.sc_fota_cb = fotacb;

                sAPI_FotaServiceBegin((void *)&param);
                break;
            }

            case SC_HTTP_FOTA:
            {
                struct SC_FotaApiParam param = {0};
                strcpy(param.host, "47.108.134.22/zyk_fota/system_patch.bin");
                strcpy(param.username, "root");
                strcpy(param.password, "Simcom1234!");
                param.mode = 1;//http
                param.sc_fota_cb = fotacb;

                sAPI_FotaServiceBegin((void *)&param);
                break;
            }

#ifndef CRANEL_OTA
#ifndef LTEONLY_THIN_SINGLE_SIM_2MFLASH
            case SC_LOCAL_FOTA:
            {
                SCFILE *fp = NULL;
                size_t read_len = 0;
                int filesize = 0;

                void *read_data = (void *)malloc(1024);
                if (!read_data)
                {
                    LOG_ERR();
                    goto error;
                }

/*1803S没有simdir目录，不需要切换到simdir路径*/
#ifndef FALCON_1803_PLATFORM
                int ret = -1;
                ret = sAPI_FsSwitchDir("sys_patch.bin", DIR_C_TO_SIMDIR);//change dir to simdir
                if (ret)
                {
                    LOG_ERR();
                    goto error;
                }
#endif

                fp = sAPI_fopen("c:/syspatch.bin", "rb");
                if (fp == NULL)
                {
                    LOG_ERR();
                    goto error;
                }
               filesize = sAPI_fsize((SCFILE *)fp);
                //filesize = 848396;
                printf("fota file size : [%d]", filesize);

                while (1)
                {
                    memset(read_data, 0, 1024);
                    read_len = sAPI_fread(read_data, 1, 1024, fp);
                    printf("fota read len: [%d]", read_len);

                    if (read_len == 0)
                        break;
                    if (sAPI_FotaImageWrite(read_data, read_len, filesize) < 0)
                    {
                        LOG_ERR();
                        goto error;
                    }
                }

                printf("sAPI_FotaImageWrite sucess!!!");
#ifndef FALCON_1803_PLATFORM
                if (sAPI_FotaImageVerify(filesize) != 0)
#else
                if (sAPI_FotaImageVerify(filesize) != 100)
#endif
                {
                    LOG_ERR();
                    printf("sAPI_FotaImageVerify fail!!!");
                    goto error;
                }
                printf("first fota image write suc, verify suc");
error:
                if (fp)
                    sAPI_fclose(fp);
                if (read_data)
                    free(read_data);

                break;
            }
#endif
#endif

#ifdef FEATURE_SIMCOM_NORFLASH_MINI_LFOTA
            case SC_MINI_LOCAL_FOTA:
            {
                struct SC_LFotaApiParam param = {0};
                strcpy(param.filepath, "D:/system_patch.bin");
                param.mode = 1;/*1: norflash*/
                if(!sAPI_LFotaPreVerify((void *)&param))
                {
                    printf("sAPI_LFotaPreVerify sucess!");
                    sAPI_LFotaServiceBegin((void *)&param);
                }
                else
                {
                    printf("sAPI_LFotaPreVerify fail!");
                }
                break;
            }
#endif

#ifndef FALCON_1803_PLATFORM
            case 8:
            {
                char tmp[60];
                SC_MiniSysStatus MiniSysStatus = {0, 0};
                sAPI_GetMiniSysStatus(&MiniSysStatus);
                sprintf(tmp, "\r\nCurrent minisys enable.enable %d stage %d\r\n", MiniSysStatus.enable, MiniSysStatus.stage);

                PrintfResp(tmp);
                break;
            }
#endif

            case SC_FOTA_BACK:
                return;
        }
    }
}
