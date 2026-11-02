/*
 * @Author: yangkui.zhang
 * @Date: 2023-12-22 16:45:37
 * @LastEditTime: 2024-01-10 15:01:52
 * @LastEditors: yangkui.zhang
 * @Description: 
 * @FilePath: \ASR160X\simcom\userspace\sc_demo_v2\src\demo_pwm.c
 * no error,no warning
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "simcom_demo_init.h"
#include "demo_pwm.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "simcom_os.h"
#include "simcom_pwm.h"


#define LOG_ERROR(...) sal_log_error("[DEMO-PWM] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-PWM] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-PWM] " __VA_ARGS__)


static ret_msg_t set_pwm_config_cmd_proc(op_t op, arg_t *argv, int argn);

const value_t devices[] =
{
    {
        .bufp = "device1",
        .size = 7,
    },
    {
        .bufp = "device2",
        .size = 7,
    },
    {
        .bufp = "device3",
        .size = 7,
    },
    {
        .bufp = "device4",
        .size = 7,
    }
};
const value_list_t device_range =
{
    .isrange = false,
    .list_head = devices,
    .count = 4,
};

const value_t switchs[] =
{
    {
        .bufp = "off",
        .size = 3,
    },
    {
        .bufp = "on",
        .size = 2,
    }
};
const value_list_t switch_range =
{
    .isrange = false,
    .list_head = switchs,
    .count = 2,
};

arg_t setpwmconfig_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGPWM_NUMBER,
        .value_range= &device_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGPWM_SWITCH,
        .value_range= &switch_range,
    }
};


const menu_list_t func_setpwmconfig =
{
    .menu_name = "Set pwm config",
    .menu_type = TYPE_FUNC,
    .parent = &pwm_menu,
    .function = {
        .argv = setpwmconfig_args,
        .argn = sizeof(setpwmconfig_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_pwm_config_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t *pwm_menu_sub[] =
{
    &func_setpwmconfig,

    NULL  // MUST end by NULL
};

const menu_list_t pwm_menu =
{
    .menu_name = "PWM",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = pwm_menu_sub,
    .parent = &root_menu,
};


static ret_msg_t set_pwm_config_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int pwm_number = 0;
    int pwm_switch = 0;
    const char *pwm = NULL;

    pl_demo_get_data(&pwm, argv, argn, ARGPWM_NUMBER);
    if (!pwm)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    if(!strncmp(pwm,"device1",7))
    {
        pwm_number = 1;
    }
    else if(!strncmp(pwm,"device2",7))
    {
        pwm_number = 2;
    }
    else if(!strncmp(pwm,"device3",7))
    {
        pwm_number = 3;
    }
    else
    {
        pwm_number = 4;
    }

    pl_demo_get_data(&pwm, argv, argn, ARGPWM_SWITCH);
    if (!pwm)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    if(!strncmp(pwm,"off",3))
    {
        pwm_switch = 1;
    }
    else
    {
        pwm_switch = 0;
    }

    LOG_INFO("%s,pwmnumber:%d , set_pwmconfig:[%d]", __func__, pwm_number, pwm_switch);

    if (sAPI_PWMConfig(pwm_number, pwm_switch, 64, 7, 14) != PWM_RC_OK)
    {
        LOG_ERROR("sAPI_PWMConfig fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_PWMConfig fail!!!";
        goto EXIT;
    }

EXIT:
    return ret;
}
