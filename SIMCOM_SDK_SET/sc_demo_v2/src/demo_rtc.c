/*
 * @Author: junjie.peng
 * @Date: 2023-12-20
 * @Last Modified by: junjie.peng
 * @Last Modified time: 2023-12-20
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "simcom_rtc.h"
#include "simcom_gpio.h"
#include "simcom_common.h"
#include "demo_rtc.h"
#include "simcom_uart.h"


#define LOG(...)       sal_log(__VA_ARGS__)
#define LOG_ERROR(...) sal_log_error("[DEMO-RTC] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-RTC] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-RTC] " __VA_ARGS__)
char buff[100] = {0};
t_rtc timeval;
static ret_msg_t rtcSettime(op_t op, arg_t *argv, int argn);
static ret_msg_t rtcGettime(op_t op, arg_t *argv, int argn);
static ret_msg_t rtcSetalarm(op_t op, arg_t *argv, int argn);
static ret_msg_t rtcGetalarm(op_t op, arg_t *argv, int argn);
static ret_msg_t rtcenablealarm(op_t op, arg_t *argv, int argn);
static ret_msg_t rtcgetutctime(op_t op, arg_t *argv, int argn);
static ret_msg_t callback_test(op_t op, arg_t *argv, int argn);

void cbfunc(void)
{
    sAPI_UartPrintf("alarm callback !");
}


arg_t inputtime_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = INPUT_TIME_DAY,
        .msg = "Please input time such as: 2024/03/08"
    },
    {
        .type = TYPE_STR,
        .arg_name = INPUT_TIME_CLOCK,
        .msg = "Please input time such as: 09:36:00"
    }

};

arg_t inputalarm_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = INPUT_ALARM_DAY,
        .msg = "Please input time such as: 2024/03/08"
    },
    {
        .type = TYPE_STR,
        .arg_name = INPUT_ALARM_CLOCK,
        .msg = "Please input time such as: 09:36:00"
    }
};
const value_t enable[] =
{
    {
        .bufp = "enable",
        .size = 5,
    },
    {
        .bufp = "disable",
        .size = 6,
    }
};
const value_list_t enable_range =
{
    .isrange = false,
    .list_head = enable,
    .count = 2,
};



arg_t inputenable_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = INPUT_ENABLE,
        .value_range = &enable_range,
    }
};


const menu_list_t rtcSettime_menu =
{
    .menu_name = "Set time",
    .menu_type = TYPE_FUNC,
    .parent = &rtc_menu,
    .function = {
        .argv = inputtime_args,
        .argn = sizeof(inputtime_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = rtcSettime,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t rtcGettime_menu =
{
    .menu_name = "Get time",
    .menu_type = TYPE_FUNC,
    .parent = &rtc_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = rtcGettime,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t rtcSetalarm_menu =
{
    .menu_name = "Set alarm",
    .menu_type = TYPE_FUNC,
    .parent = &rtc_menu,
    .function = {
        .argv = inputalarm_args,
        .argn = sizeof(inputalarm_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = rtcSetalarm,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t rtcGetalarm_menu =
{
    .menu_name = "Get alarm",
    .menu_type = TYPE_FUNC,
    .parent = &rtc_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = rtcGetalarm,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t rtcregisteralarmcb_menu =
{
    .menu_name = "callback_test",
    .menu_type = TYPE_FUNC,
    .parent = &rtc_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler =callback_test,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t rtcenablealarm_menu =
{
    .menu_name = "enable alarm",
    .menu_type = TYPE_FUNC,
    .parent = &rtc_menu,
    .function = {
        .argv = inputenable_args,
        .argn = sizeof(inputenable_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = rtcenablealarm,
        .methods.needRAWData = NULL,
    },
};





const menu_list_t rtcgetutctime_menu =
{
    .menu_name = "get utc time",
    .menu_type = TYPE_FUNC,
    .parent = &rtc_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = rtcgetutctime,
        .methods.needRAWData = NULL,
    },
};


const menu_list_t *rtc_menu_sub[] =
{
    &rtcSettime_menu,
    &rtcGettime_menu,
    &rtcSetalarm_menu,
    &rtcGetalarm_menu,
    &rtcenablealarm_menu,
    &rtcregisteralarmcb_menu,
    &rtcgetutctime_menu,

    NULL  // MUST end by NULL
};

const menu_list_t rtc_menu =
{
    .menu_name = "rtc",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = rtc_menu_sub,
    .parent = &root_menu,
};

static ret_msg_t rtcSettime(op_t op, arg_t *argv, int argn)
{
    const char *timeStrday =NULL;
    const char *timeStrclock =NULL;
    memset(&timeval, 0, sizeof(t_rtc));
    int monthLen[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int retval1 = -1;
    int retval2 = -1;
    int timezone = 0;
    int retset=0;
    ret_msg_t ret_msg= {0};
    pl_demo_get_data(&timeStrday, argv, argn, INPUT_TIME_DAY);
    pl_demo_get_data(&timeStrclock, argv, argn, INPUT_TIME_CLOCK);

    retval1 = sscanf((char *)timeStrday, "%4d/%2d/%2d", &timeval.tm_year, &timeval.tm_mon,&timeval.tm_mday);
    retval2 = sscanf((char *)timeStrclock, "%2d:%2d:%2d", &timeval.tm_hour, &timeval.tm_min,&timeval.tm_sec);
    if (retval1+retval2 < 6 )
    {
        LOG_INFO("Please input correct value !");
        ret_msg.errcode = ERRCODE_DEFAULT;
        ret_msg.msg = "Please input correct value !";
        goto EXIT;
    }
      if (timeval.tm_year <1970 || timeval.tm_year > 2070  )
    {
        LOG_INFO("The year should be greater than 1970 and less than 2070!");
        ret_msg.errcode = ERRCODE_DEFAULT;
        ret_msg.msg = "The year should be greater than 1970 and less than 2070!";
        goto EXIT;
    }

    if (((timeval.tm_year % 4 == 0) && (timeval.tm_year % 100 != 0)) || (timeval.tm_year % 400 == 0))
        monthLen[1] += 1;

    if (((timeval.tm_year >= 1970) && (timeval.tm_year < 2070)) && ((timeval.tm_mon > 0) && (timeval.tm_mon <= 12))
        && ((timeval.tm_mday > 0) && (timeval.tm_mday <= monthLen[timeval.tm_mon - 1]))   \
        && ((timeval.tm_hour >= 0) && (timeval.tm_hour < 24)) && ((timeval.tm_min >= 0) && (timeval.tm_min < 60))
        && ((timeval.tm_sec >= 0) && (timeval.tm_sec < 60))   \
        && ((timezone >= -96) && (timezone <= 96)))
    {
        retset = sAPI_SetRealTimeClock(&timeval);
        if (retset < 0)
        {
            LOG_INFO("set time failed 1 !");
            ret_msg.errcode = ERRCODE_DEFAULT;
            ret_msg.msg = "set time failed 1 !";
            goto EXIT;

        }
        else
        {
            LOG_INFO("set time successed !!");
            ret_msg.errcode = ERRCODE_OK;
            ret_msg.msg = "set time successed ! !";
            goto EXIT;

        }
    }
    else
    {
        LOG_INFO("set time failed 2 !");
        ret_msg.errcode = ERRCODE_DEFAULT;
        ret_msg.msg = "set time failed 2 !";
        goto EXIT;
    }

    EXIT:
    return ret_msg;
}

static ret_msg_t rtcGettime(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret_msg = {0};
    memset(&timeval, 0, sizeof(t_rtc));
    sAPI_GetRealTimeClock(&timeval);
    sprintf(buff, "Get RTC time: %d/%02d/%02d %02d:%02d:%02d", timeval.tm_year, timeval.tm_mon, timeval.tm_mday,
            timeval.tm_hour, timeval.tm_min, timeval.tm_sec);
    LOG(buff);
    ret_msg.errcode = ERRCODE_OK;
    ret_msg.msg = buff;
    return  ret_msg;
}

static ret_msg_t rtcSetalarm(op_t op, arg_t *argv, int argn)
{
    const char *timeStrday = NULL;
    const char *timeStrclock = NULL;
    int monthLen[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int retval1 = -1;
    int retval2 = -1;
    ret_msg_t ret_msg= {0};
    memset(&timeval, 0, sizeof(t_rtc));
    pl_demo_get_data(&timeStrday, argv, argn, INPUT_ALARM_DAY);
    pl_demo_get_data(&timeStrclock, argv, argn, INPUT_ALARM_CLOCK);

    retval1 = sscanf((char *)timeStrday, "%4d/%2d/%2d", &timeval.tm_year, &timeval.tm_mon,&timeval.tm_mday);
    retval2 = sscanf((char *)timeStrclock, "%2d:%2d:%2d", &timeval.tm_hour, &timeval.tm_min,&timeval.tm_sec);
    if (retval1+retval2 < 6)
    {
        LOG_INFO("Please input correct value !");
        ret_msg.errcode = ERRCODE_DEFAULT;
        ret_msg.msg = "Please input correct value !";
        goto EXIT;
    }
    if (timeval.tm_year <1970 || timeval.tm_year > 2070  )
    {
        LOG_INFO("The year should be greater than 1970 and less than 2070!");
        ret_msg.errcode = ERRCODE_DEFAULT;
        ret_msg.msg = "The year should be greater than 1970 and less than 2070!";
        goto EXIT;
    }
    if (((timeval.tm_year % 4 == 0) && (timeval.tm_year % 100 != 0)) || (timeval.tm_year % 400 == 0))
        monthLen[1] += 1;
    if (((timeval.tm_year >= 1970) && (timeval.tm_year < 2070)) && ((timeval.tm_mon > 0) && (timeval.tm_mon <= 12))
    && ((timeval.tm_mday > 0) && (timeval.tm_mday <= monthLen[timeval.tm_mon - 1]))   \
    && ((timeval.tm_hour >= 0) && (timeval.tm_hour < 24)) && ((timeval.tm_min >= 0) && (timeval.tm_min < 60))
    && ((timeval.tm_sec >= 0) && (timeval.tm_sec < 60)))
    {
        sAPI_RtcSetAlarm(&timeval);
        memset(&timeval, 0, sizeof(t_rtc));
        LOG("set Alarm OK");
        ret_msg.errcode = ERRCODE_OK;
        ret_msg.msg = "OK";
        goto EXIT;
    }
    else
    {
        LOG_INFO("Please input correct value !");
        ret_msg.errcode = ERRCODE_DEFAULT;
        ret_msg.msg = "Please input correct value !";
        goto EXIT;
    }


    EXIT:
    return ret_msg;

}


static ret_msg_t rtcGetalarm(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret_msg = {0};
    memset(&timeval, 0, sizeof(t_rtc));
    sAPI_RtcGetAlarm(&timeval);
    sprintf(buff, "AlarmTime: %d/%.2d/%.2d %.2d:%.2d:%.2d ", timeval.tm_year, \
        timeval.tm_mon, timeval.tm_mday, timeval.tm_hour, timeval.tm_min, timeval.tm_sec);
    LOG(buff);
    ret_msg.errcode = ERRCODE_OK;
    ret_msg.msg = buff;
    return  ret_msg;

}

static ret_msg_t callback_test(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret_msg = {0};
    sAPI_RtcRegisterCB(cbfunc);
    sAPI_RtcEnableAlarm(1);
    sAPI_GetRealTimeClock(&timeval);
    timeval.tm_sec+=10;
    sAPI_RtcSetAlarm(&timeval);
    ret_msg.errcode = ERRCODE_OK;
    ret_msg.msg = "callback_test ok";
    return  ret_msg;
}

static ret_msg_t rtcenablealarm(op_t op, arg_t *argv, int argn)
{
    const char *onoff = NULL;
    unsigned int enable=0;
    ret_msg_t ret_msg = {0};
    pl_demo_get_data(&onoff, argv, argn, INPUT_ENABLE);
    if (!onoff)
    {
        LOG_ERROR("param invalid");
        ret_msg.errcode = ERRCODE_DEFAULT;
        ret_msg.msg = "param invalid";
        goto EXIT;
    }
    if(!strcmp(onoff,"enable"))
    {
        enable = 1;
        ret_msg.msg = "enable alarm ok!";
    }
    else
    {
        enable =0;
        ret_msg.msg = "disable alarm ok!";
    }
    sAPI_RtcEnableAlarm(enable);
    ret_msg.errcode = ERRCODE_OK;
    goto EXIT;

EXIT:
    return ret_msg;
}



static ret_msg_t rtcgetutctime(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret_msg = {0};
    memset(&timeval, 0, sizeof(t_rtc));
    sAPI_GetUtcTime(&timeval);
    sprintf(buff, "Get UTC time: %d/%02d/%02d %02d:%02d:%02d ", timeval.tm_year, timeval.tm_mon, timeval.tm_mday,
            timeval.tm_hour, timeval.tm_min, timeval.tm_sec);
    LOG(buff);
    ret_msg.errcode = ERRCODE_OK;
    ret_msg.msg = buff;
    return  ret_msg;
}


