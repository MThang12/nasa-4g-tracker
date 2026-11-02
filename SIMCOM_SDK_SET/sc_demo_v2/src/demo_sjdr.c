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
#include "sal_os.h"
#include "sal_log.h"
#include "simcom_demo_init.h"



#define JAMMING_NO_DETECT_ID     0
#define JAMMING_DETECT_ID        1
#define ARGSJDR_UPDATE_RESULT "updateResult"

#define LOG_ERROR(...) sal_log_error("[DEMO-SJDR] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-SJDR] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-SJDR] " __VA_ARGS__)
static ret_msg_t func_sjdr_config_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t func_sjdr_start_proc(op_t op, arg_t *argv, int argn);

const menu_list_t func_sjdr_config =
{
    .menu_name = "Config JamDect",
    .menu_type = TYPE_FUNC,
    .parent = &sjdr_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = func_sjdr_config_proc,
    },
};
const menu_list_t func_sjdr_start =
{
    .menu_name = "Enable JamDect",
    .menu_type = TYPE_FUNC,
    .parent = &sjdr_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = func_sjdr_start_proc,
    },
};

const menu_list_t *sjdr_menu_sub[] =
{
    &func_sjdr_config,
    &func_sjdr_start,
    NULL  // MUST end by NULL
};

const menu_list_t sjdr_menu =
{
    .menu_name = "JAMMING DETECT",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = sjdr_menu_sub,
    .parent = &root_menu,
};
/**
  * @brief	sjdr callback
  * @param	int flag
  * @note	This demo shows how to get sjdr status.
  * @retval char
  */

char getJamDectStatusDemo(UINT32 Id)
{
  arg_t out_arg[1] = { 0 };
  int ret = 0;
  out_arg[0].arg_name = ARGSJDR_UPDATE_RESULT;
  out_arg[0].type = TYPE_URC_DATA;
  int bufsize = 0;
  char buf[25] ={0};
  LOG_TRACE("%s: jzr jiaming %d\n", __func__,Id);
  if(Id == JAMMING_NO_DETECT_ID)
  {
	LOG_TRACE("%s,report no jamming!!",__func__);
	bufsize = snprintf(buf, sizeof(buf), "report no jamming!");
	out_arg[0].value = pl_demo_make_value(-1,buf, bufsize);
  }
  else if(Id == JAMMING_DETECT_ID)
  {
	LOG_TRACE("%s,report jamming!!",__func__);
	bufsize = snprintf(buf, sizeof(buf), "report jamming detect!");
	out_arg[0].value = pl_demo_make_value(-1, buf, bufsize);
  }
  ret = pl_demo_post_urc(&func_sjdr_start,TYPE_URC_DATA, out_arg, 1, buf, bufsize);

    pl_demo_release_value(out_arg[0].value);
  return ret;
}

static ret_msg_t func_sjdr_config_proc(op_t op, arg_t *argv, int argn)
{
	ret_msg_t JDret = {0};
    sAPI_JDConfig(5, 17, 5, 1, getJamDectStatusDemo);
    return JDret;
}

static ret_msg_t func_sjdr_start_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
	int sjdr_ret ;
    sjdr_ret = sAPI_JDSet(1);
    if(sjdr_ret == SC_SUCCESS)
    {
        LOG_INFO("enable jd success.");
        ret.msg = "enable jd ok!";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("enable jd falied!");
        ret.msg = "enable jd falied!";
    }
    return ret;
}



