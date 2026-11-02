/*
*@Author: Yanan Peng
*@Date: 2023-12-12
*@Last Modified by: Yanan Peng
*@Last Modified Date: 2024-03-11
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "simcom_ntp_client.h"
#include "simcom_common.h"


#define LOG_ERROR(...) sal_log_error("[DEMO_NTP]"__VA_ARGS__)
#define LOG_INFO(...) sal_log_info("[DEMO_NTP]"__VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO_NTP]"__VA_ARGS__)

#define ARGNTP_SERVER "server"
#define ARGNTP_TIMEZONE "timeZone"
#define ARGNTP_CONFIG_RESULT "configResult"
#define ARGNTP_UPDATE_RESULT "updateResult"

int ntp_config(const char * server, int timeZone);
static ret_msg_t ntp_set_config_cmd_proc(op_t op, arg_t *argv, int argn);
int ntp_update_async(SCsysTime_t *currUtcTime);
static ret_msg_t ntp_update_cmd_proc(op_t op, arg_t *argv, int argn);


value_t ntp_server_value =
{
    .bufp = "ntp.aliyun.com",
    .size = 14,
};

value_t ntp_timezone_value =
{
    .val = 32,
};
const value_list_t ntp_timezone_range =
{
    .isrange = true,
    .min = -96,
    .max = 96,
};

arg_t ntp_set_config[2] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGNTP_SERVER,
        .msg = "NTP server address, length is 0-255",
        .value = &ntp_server_value,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGNTP_TIMEZONE,
        .msg = "Local time zone, the range is (-96 to 96)",
        .value = &ntp_timezone_value,
        .value_range = &ntp_timezone_range,
    }
};

const menu_list_t func_ntp_set_config =
{
    .menu_name = "Set Config",
    .menu_type = TYPE_FUNC,
    .parent = &ntp_menu,
    .function = {
        .argv = ntp_set_config,
        .argn = 2,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = ntp_set_config_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    }
};




const menu_list_t func_ntp_update =
{
    .menu_name = "Update",
    .menu_type = TYPE_FUNC,
    .parent = &ntp_menu,
    .function = {
        .argv = NULL,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = ntp_update_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    }
};

const menu_list_t *ntp_menu_sub[] =
{
    &func_ntp_set_config,
    &func_ntp_update,
    NULL
};

const menu_list_t ntp_menu =
{
    .menu_name = "ntp",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = ntp_menu_sub,
    .parent = &root_menu
};


sMsgQRef g_ntpUIResp_msgq;


int ntp_config(const char *server, int timeZone)
{
	int ret;
    char buf[SAL_256] ;

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), server);

    ret = sAPI_NtpUpdate(SC_NTP_OP_SET, buf, timeZone, NULL);
    LOG_INFO("ntp config result: %d", ret);

    return ret;
}


static ret_msg_t ntp_set_config_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int ntp_ret;
    const char *server = NULL;
    int timeZone = 0;
    arg_t out_arg[1] = { 0 };

    pl_demo_get_data(&server, argv, argn, ARGNTP_SERVER);
    pl_demo_get_value(&timeZone, argv, argn, ARGNTP_TIMEZONE);
    if(NULL == server ||
        timeZone < -96 || timeZone > 96)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        goto EXIT;
    }

    ntp_ret = ntp_config(server, timeZone);

    out_arg[0].arg_name = ARGNTP_CONFIG_RESULT;
    out_arg[0].type = TYPE_STR;
    if(SC_NTP_OK == ntp_ret)
    {
        out_arg[0].value = pl_demo_make_value(-1, "OK", 2);
    }
    else
    {
        out_arg[0].value = pl_demo_make_value(-1, "ERROR", 5);
    }

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
        goto EXIT;
    }


EXIT:
    pl_demo_release_value(out_arg[0].value);

    return ret;
}

int ntp_update_async(SCsysTime_t *currUtcTime)
{
    int ret;
    SIM_MSG_T ntp_result = {SC_SRV_NONE, -1, 0, NULL};

    if (NULL == g_ntpUIResp_msgq)
    {
        SC_STATUS status;
        status = sAPI_MsgQCreate(&g_ntpUIResp_msgq, "htpUIResp_msgq", sizeof(SIM_MSG_T), 4, SC_FIFO);
        if (SC_SUCCESS != status)
        {
            LOG_ERROR("[CNTP]msgQ create fail");
        }
    }

    ret = sAPI_NtpUpdate(SC_NTP_OP_EXC, NULL, 0, g_ntpUIResp_msgq);
    LOG_INFO("ntp update ret = %d", ret);

    do
    {
        sAPI_MsgQRecv(g_ntpUIResp_msgq, &ntp_result, SC_SUSPEND);

        if (SC_SRV_NTP != ntp_result.msg_id)
        {
            LOG_INFO("[CNTP] ntp_result.msg_id =[%d]", ntp_result.msg_id );
            ntp_result.msg_id = SC_SRV_NONE;
            ntp_result.arg1 = -1;
            ntp_result.arg3 = NULL;
            continue;
        }
        if (SC_NTP_OK == ntp_result.arg1)
        {
            LOG_INFO("[CNTP] successfully update time! ");
            break;
        }
        else
        {
            LOG_INFO("[CNTP] failed to update time! result code: %d", ntp_result.arg1);
            break;
        }
    } while (1);

    memset(currUtcTime, 0, sizeof(SCsysTime_t));
    sAPI_GetSysLocalTime(currUtcTime);
    LOG_INFO("local time after ntp update:  %d - %d - %d - %d : %d : %d   %d",
    currUtcTime->tm_year, currUtcTime->tm_mon, currUtcTime->tm_mday, currUtcTime->tm_hour, currUtcTime->tm_min, currUtcTime->tm_sec, currUtcTime->tm_wday);

    return ret;
}


static ret_msg_t ntp_update_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int ntp_ret;
    arg_t out_arg_1[1] = { 0 };
    SCsysTime_t currUtcTime;
    char buf[SAL_256];//Is 256 bytes enough?
    int bufSize = 0;

    memset(buf, 0, sizeof(buf));

    ntp_ret = ntp_update_async(&currUtcTime);

    bufSize = snprintf(buf, sizeof(buf), "updateResult: %d.  local time after ntp update: %d-%d-%d  %d:%d:%d  %d",
    ntp_ret, currUtcTime.tm_year, currUtcTime.tm_mon, currUtcTime.tm_mday, currUtcTime.tm_hour, currUtcTime.tm_min, currUtcTime.tm_sec, currUtcTime.tm_wday);

    LOG_INFO("update_info: %s, update_info_len: %d", buf, bufSize);

    out_arg_1[0].arg_name = ARGNTP_UPDATE_RESULT;
    out_arg_1[0].type = TYPE_STR;
    out_arg_1[0].value = pl_demo_make_value(-1, buf, bufSize);

    ret.errcode = pl_demo_post_data(OP_POST, out_arg_1, 1, NULL, 0);
    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
        goto EXIT;
    }


EXIT:
    pl_demo_release_value(out_arg_1[0].value);

    return ret;
}

