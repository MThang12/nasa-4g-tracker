/**
  ******************************************************************************
  * @file    demo_exfs.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of SPI operation.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "simcom_os.h"
#include "simcom_debug.h"
#include "sc_spi.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "simcom_common.h"
#include "uart_api.h"
#include "simcom_exfsmount.h"

extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);

typedef enum
{

    EXNORFLASH_MOUNT_LFS = 1,
    EXNANDFLASH_MOUNT_YAFFS = 2,
    EXFLASH_MOUNT_STATUS = 3,
    EXFLASH_MOUNT_BACK = 99,
} EXFLASH_MOUNT_FS;

sTaskRef norflashmountlfsProcesser;
sTaskRef nandflashmountlfsProcesser;


void sTASK_norflashmountlfsProcesser(void *arg)
{
    SC_ExFsMount_ReturnCode ret;
    ret = sAPI_NorFlashMountLfs();
    sAPI_Debug("extern flash mount lfs ret:%d",ret);
}


void sAPI_NorflashmountLFS_test(void)
{
    SC_STATUS status;
    void *norflashmountlfsStack = (void *)malloc(60 * 1024);
    if (!norflashmountlfsStack)
    {
        printf("malloc norflashmountlfsStack fail!");
        return;
    }
    status = sAPI_TaskCreate(&norflashmountlfsProcesser,
                             norflashmountlfsStack,
                             1024 * 60,
                             101,
                             "norflashmountlfsProcesser",
                             sTASK_norflashmountlfsProcesser,
                             (void *)0);
    if (SC_SUCCESS != status)
    {
        sAPI_Debug("task create fail");
    }
}

void sTASK_nandflashmountlfsProcesser(void *arg)
{
    SC_ExFsMount_ReturnCode ret;
    ret = sAPI_NandFlashMountYaffs();
    sAPI_Debug("extern nand flash mount yaffs ret:%d",ret);
}


void sAPI_NandflashmountYaffs_test(void)
{
    SC_STATUS status;
    void *nandflashmountlfsStack = (void *)malloc(60 * 1024);
    if (!nandflashmountlfsStack)
    {
        printf("malloc nandflashmountlfsStack fail!");
        return;
    }
    status = sAPI_TaskCreate(&nandflashmountlfsProcesser,
                             nandflashmountlfsStack,
                             1024 * 60,
                             101,
                             "nandflashmountlfsProcesser",
                             sTASK_nandflashmountlfsProcesser,
                             (void *)0);
    if (SC_SUCCESS != status)
    {
        sAPI_Debug("task create fail");
    }
}





/**
 * @brief  This demo show that how to use  extern flash mount fs
 * @param  void
 * @note
 * @retval void
 *
 * Demo describe:
 * This demo show that how to use extern flash mount fs
 */
void exflashmountfsDemo(void)
{
    UINT32 opt = 0;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] =
    {
        "1. Extern NorFlash mount LFS",
        "2. Extern NandFLash mount YAFFS ",
        "3. Obtain the current mount status "
        "99. back",
    };

    while (1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list, sizeof(options_list) / sizeof(options_list[0]));

        opt = UartReadValue();
        char disBuff[256] = {0};
        memset(disBuff, 0, sizeof(disBuff));
        switch (opt)
        {
            case EXNORFLASH_MOUNT_LFS:
            {
                PrintfResp("\r\nstart mount LFS for NorFlash....\r\n");
                sAPI_NorflashmountLFS_test();
                PrintfResp("\r\nLFS mount finish....\r\n");
                break;
            }
            case EXNANDFLASH_MOUNT_YAFFS:
            {
                //PrintfResp("\r\nThis feature is not yet developed, so stay tuned!\r\n");
                PrintfResp("\r\nstart mount Yaffs for NandFlash....\r\n");
                sAPI_NandflashmountYaffs_test();
                PrintfResp("\r\nYaffs mount finish....\r\n");
                break;
            }
            case EXFLASH_MOUNT_STATUS:
            {
                SC_ExFsMount_State state;
                state = sAPI_ExFlashMountState();
                sprintf(disBuff, "\r\ncurrent mount state:%d\r\n",state);
                PrintfResp(disBuff); 
                break;
            }
            case EXFLASH_MOUNT_BACK:
                return;
        }
    }

}
