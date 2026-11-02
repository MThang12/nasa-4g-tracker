/*
 * @Author: junjie.peng
 * @Date: 2023-12-20
 * @Last Modified by: junjie.peng
 * @Last Modified time: 2023-12-20
 */

#include "simcom_common.h"
// #include "simcom_os.h"
#include "simcom_system.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "simcom_wtd.h"
#include "simcom_debug.h"
#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"

#define LOG(...)       sal_log(__VA_ARGS__)
#define LOG_ERROR(...) sal_log_error("[DEMO-WTD] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-WTD] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-WTD] " __VA_ARGS__)

static ret_msg_t open_wtd(op_t op, arg_t *argv, int argn);
static ret_msg_t close_wtd(op_t op, arg_t *argv, int argn);
static ret_msg_t feed_wtd(op_t op, arg_t *argv, int argn);


const menu_list_t wtdopen =
{
    .menu_name = "wtdopen",
    .menu_type = TYPE_FUNC,
    .parent = &WTD_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = open_wtd,
        .methods.needRAWData = NULL,
    },
};
const menu_list_t wtdclose =
{
    .menu_name = "wtdclose",
    .menu_type = TYPE_FUNC,
    .parent = &WTD_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = close_wtd,
        .methods.needRAWData = NULL,
    },
};
const menu_list_t feeddog =
{
    .menu_name = "feeddog",
    .menu_type = TYPE_FUNC,
    .parent = &WTD_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = feed_wtd,
        .methods.needRAWData = NULL,
    },
};



const menu_list_t *WTD_menu_sub[] =
{
    &wtdopen,
    &wtdclose,
    &feeddog,
    NULL  // MUST end by NULL
};

const menu_list_t WTD_menu =
{
    .menu_name = "WTD",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = WTD_menu_sub,
    .parent = &root_menu,
};

static ret_msg_t open_wtd(op_t op, arg_t *argv, int argn)
{
    int ret = 1;
    ret_msg_t ret_msg = {0};
    ret = sAPI_SetWtdTimeOutPeriod(0x04);//feed dog 16S Period

    ret |= sAPI_FalutWakeEnable(1);

    ret |= sAPI_SoftWtdEnable(1);

    ret |= sAPI_FeedWtd(); //After starting the WDT,the dog needs to be fed once before the timeout set by the register 0x11 will load

    if (ret)
    {
        LOG_ERROR("open watchdog failed");
        ret_msg.errcode = ERRCODE_DEFAULT;
        ret_msg.msg = "open watchdog failed";
        goto EXIT;
    }
    else
    {
        LOG_INFO("open watchdog successful!");
        ret_msg.errcode = ERRCODE_OK;
        ret_msg.msg = "open watchdog successful!";
        goto EXIT;
    }
    EXIT:
    return ret_msg;
}


static ret_msg_t close_wtd(op_t op, arg_t *argv, int argn)
{
    int ret = 1;
    ret_msg_t ret_msg = {0};
    ret  = sAPI_FalutWakeEnable(0);
    ret |= sAPI_SoftWtdEnable(0);

    if (ret)
    {
        LOG_ERROR("close watchdog failed");
        ret_msg.errcode = ERRCODE_DEFAULT;
        ret_msg.msg = "close watchdog failed";
        goto EXIT;
    }
    else
    {
        LOG_INFO("close watchdog successful!");
        ret_msg.errcode = ERRCODE_OK;
        ret_msg.msg = "close watchdog successful!";
        goto EXIT;
    }
    EXIT:
    return ret_msg;
}



static ret_msg_t feed_wtd(op_t op, arg_t *argv, int argn)
{
    int ret = 1;
    ret_msg_t ret_msg = {0};
    ret = sAPI_FeedWtd();

    if (ret)
    {
        LOG_ERROR("feed watchdog failed");
        ret_msg.errcode = ERRCODE_DEFAULT;
        ret_msg.msg = "feed watchdog failed";
        goto EXIT;
    }
    else
    {
        LOG_INFO("feed watchdog successful!");
        ret_msg.errcode = ERRCODE_OK;
        ret_msg.msg = "feed watchdog successful!";
        goto EXIT;
    }
    EXIT:
    return ret_msg;
}

