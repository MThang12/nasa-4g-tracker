/**
  ******************************************************************************
  * @file    demo_sjdr.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of network management.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "simcom_sjdr.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "simcom_uart.h"
#include "uart_api.h"



#define JAMMING_NO_DETECT_ID     0
#define JAMMING_DETECT_ID        1

extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);

/**
  * @brief  sjdr callback
  * @param  int flag
  * @note   This demo shows how to get sjdr status.
  * @retval char
  */
char getJamDectStatusDemo(UINT32 Id)
{
  if(Id == JAMMING_NO_DETECT_ID)
  {
    sAPI_Debug("%s,report no jamming!!",__func__);
    PrintfResp("\r\nreport no jamming!\r\n");
  }
  else if(Id == JAMMING_DETECT_ID)
  {
    sAPI_Debug("%s,report jamming!!",__func__);
    PrintfResp("\r\nreport jamming detect!\r\n");
  }
  return 0;
}

/**
  * @brief  sjdr management demo
  * @param  void
  * @note   This demo shows how to manage sjdr by API.
  * @retval void
  */
void JamDectDemo(void)
{
    UINT8 ret;
    int opt = 0;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] = {
        "1. Config JamDect",
        "2. Enable JamDect",
        "99.back",
    };

    while(1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
        opt = UartReadValue();

        switch(opt)
        {
            case 1:
            {
                sAPI_Debug("start config jd");
                sAPI_JDConfig(5, 17, 5, 1, getJamDectStatusDemo);
                break;
            }

            case 2:
            {
                ret = sAPI_JDSet(1);
                if(ret == SC_SUCCESS)
                {
                    sAPI_Debug("enable jd success.");
                    PrintfResp("\r\nenable jd ok!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("enable jd falied!");
                    PrintfResp("\r\nenable jd falied!\r\n");
                    break;
                }
            }

            case 99:
            {
                return;
            }
            default :
                break;
        }
    }
}
