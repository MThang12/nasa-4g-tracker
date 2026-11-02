/**
  ******************************************************************************
  * @file    sc_application.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source code for SIMCom OpenSDK application, void userSpace_Main(void * arg) is the app entry for OpenSDK application,customer should start application from this call.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "userspaceConfig.h"
#include "simcom_debug.h"
#include "simcom_os.h"
#include "sal_os.h"

#ifdef HAS_DEMO
#include "simcom_demo_init.h"
#endif  // HAS_DEMO


// for userSpace_Main
#define MAIN_STACK_SIZE (1024 * 4)
char stack_main[MAIN_STACK_SIZE];

char *mainStack = stack_main;
int mainStackSize = MAIN_STACK_SIZE;
enum sal_task_priority mainPriority = sal_task_priority_low_1;

extern char UserSpaceVersion[40];
// for userSpace_Main end



/**
  * @brief  OpenSDK app entry.
  * @param  Pointer arg
  * @note   This is the app entry,like main(),all functions must start from here!!!!!!
  * @retval void
  */
void userSpace_Main(void *arg)
{
#ifdef APP_VERSION
    strncpy(UserSpaceVersion, APP_VERSION, sizeof(UserSpaceVersion));
#endif
    sAPI_enableDUMP();
    /*  UI demo task for customer with CLI method for all demo running,
        customer need to define SIMCOM_UI_DEMO_TO_USB_AT_PORT or
        SIMCOM_UI_DEMO_TO_UART1_PORT to select hardware interface.
    */
#ifdef HAS_DEMO
    sAPI_TaskSleep(200);//Wait for USB initialization to complete and print catstudio log.

    simcom_demo_init();
#endif

    printf("user app is running...");
}
