/**
  ******************************************************************************
  * @file    demo_ping.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of ping operation.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "simcom_ping.h"
#include "simcom_debug.h"
#include <string.h>

/**
  * @brief  ping operation callback
  * @param  size---packet of response.
  * @param  statistics
  * @note   
  * @retval void
  */
void pingResult(UINT32 size,INT8 *statistics)
{
    if(statistics != NULL)
        sAPI_Debug("size[%d] statistics[%s]",size,statistics);
}

/**
  * @brief  PING an IP or Domain name with API
  * @param  void
  * @note   
  * @retval void
  */
void PingDemo(void)
{
    SCPingResultCode ret ;
    SCping ctx;

    memset(&ctx,0,sizeof(ctx));
    ctx.destination = (INT8 *)"www.baidu.com";
    ctx.count = 20;
    ctx.interval = 10;
    ctx.resultcb = pingResult;

    ret = sAPI_Ping(&ctx);

    sAPI_Debug("sAPI_Ping ret[%d]",ret);
}

