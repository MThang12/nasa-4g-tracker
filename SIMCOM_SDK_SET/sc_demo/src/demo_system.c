/**
  ******************************************************************************
  * @file    demo_system.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of system operation.
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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "simcom_common.h"
#include "uart_api.h"

typedef enum
{
    GETTASKSTACKINFO = 1,
    BACK = 99,
} SYS_OPS;

#ifdef SIMCOM_GET_TASKSTACK_INFO
extern sTaskRef simcomUIProcesser;
//extern sTaskRef gUrcProcessTask;
extern sTaskRef helloWorldProcesser;
#endif
extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);

void SysDemo(void)
{
#ifdef SIMCOM_GET_TASKSTACK_INFO
    UINT32 opt = 0;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] =
    {
        "1. GetTaskStackInfo",
        "99. back",
    };
    unsigned long stackSize, stackInuse, stackPeak;
    char *taskName;
    char stackInfo[80] = {0};
    static UINT8 taskRefListIdx = 0;
    sTaskRef taskRefList[] = {
        helloWorldProcesser,
        simcomUIProcesser,
        //gUrcProcessTask,
    };
    while (1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list, sizeof(options_list) / sizeof(options_list[0]));

        opt = UartReadValue();

        switch (opt)
        {

            case GETTASKSTACKINFO:
            {
                PrintfResp("\r\n get task stack info test start\r\n ");
                for(taskRefListIdx = 0;taskRefListIdx<(sizeof(taskRefList)/sizeof(sTaskRef));taskRefListIdx++)
                {
                    sAPI_GetTaskStackInfo(taskRefList[taskRefListIdx], &taskName, &stackSize, &stackInuse, &stackPeak);
                    memset(stackInfo, 0, 80);
                    snprintf(stackInfo, 80, "%s_stack(S:%ld U:%ld P:%ld)", taskName, stackSize, stackInuse, stackPeak);
                    PrintfResp(stackInfo);
                    sAPI_TaskSleep(200*2);
                }
                PrintfResp("\r\n get task stack info test finish \r\n");
                break;
            }
            case BACK:
                return;
        }
    }
#else
    PrintfResp("\r\n GetTaskStackInfo funtion is not open!!!\r\n");
    return;
#endif
}

