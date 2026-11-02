/*
 * @Author: yangkui.zhang
 * @Date: 2023-12-22 16:45:37
 * @LastEditTime: 2024-01-10 15:53:09
 * @LastEditors: yangkui.zhang
 * @Description: 
 * @FilePath: \ASR160X\simcom\userspace\sc_demo_v2\src\demo_pm.c
 * no error,no warning
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "simcom_demo_init.h"
#include "demo_pm.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "simcom_os.h"
#include "sc_power.h"

#define LOG_ERROR(...) sal_log_error("[DEMO-PMU] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-PMU] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-PMU] " __VA_ARGS__)


static ret_msg_t detect_power_key_status_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t onoff_power_key_shutdown_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t power_updown_event_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_adc_votalge_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_vbat_votalge_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_vbat_votalge_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_syspoweroff_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_sysreset_cmd_proc(op_t op, arg_t *argv, int argn);

const value_t powerswitchs[] =
{
    {
        .bufp = "disable",
        .size = 7,
    },
    {
        .bufp = "open",
        .size = 4,
    }
};
const value_list_t powerswitch_range =
{
    .isrange = false,
    .list_head = powerswitchs,
    .count = 2,
};

arg_t onoff_power_key_shutdown_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGPMU_POWERSWITCH,
        .value_range= &powerswitch_range,
    }
};

value_t vbat_value =
{
    .val = 2500,
};
const value_list_t vbat_range =
{
    .isrange = true,
    .min = 2500,
    .max = 3000
};
arg_t set_vdd_aux_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARPMU_SETVBAT,
        .msg = "(2500--3000)",
        .value_range = &vbat_range,
        .value = &vbat_value
    }
};


const menu_list_t func_detectpowerkey =
{
    .menu_name = "Detect_power_key_status",
    .menu_type = TYPE_FUNC,
    .parent = &pmu_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = detect_power_key_status_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_onoffpowerkey =
{
    .menu_name = "Onoff_power_key",
    .menu_type = TYPE_FUNC,
    .parent = &pmu_menu,
    .function = {
        .argv = onoff_power_key_shutdown_args,
        .argn = sizeof(onoff_power_key_shutdown_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = onoff_power_key_shutdown_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_powerupdownevent =
{
    .menu_name = "Power_updown_event",
    .menu_type = TYPE_FUNC,
    .parent = &pmu_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = power_updown_event_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_getadcvotalge =
{
    .menu_name = "Get_adc_votalge",
    .menu_type = TYPE_FUNC,
    .parent = &pmu_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = get_adc_votalge_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_getvbatvotalge =
{
    .menu_name = "Get_vbat_votalge",
    .menu_type = TYPE_FUNC,
    .parent = &pmu_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = get_vbat_votalge_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_setvddaux =
{
    .menu_name = "Set VDD_AUX votalge",
    .menu_type = TYPE_FUNC,
    .parent = &pmu_menu,
    .function = {
        .argv = set_vdd_aux_args,
        .argn = sizeof(set_vdd_aux_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_vbat_votalge_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_setsyspoweroff =
{
    .menu_name = "Set_syspoweroff",
    .menu_type = TYPE_FUNC,
    .parent = &pmu_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_syspoweroff_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_setsysreset =
{
    .menu_name = "Set_sysreset",
    .menu_type = TYPE_FUNC,
    .parent = &pmu_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_sysreset_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t *pmu_menu_sub[] =
{
    &func_detectpowerkey,
    &func_onoffpowerkey,
    &func_powerupdownevent,
    &func_getadcvotalge,
    &func_getvbatvotalge,
    &func_setvddaux,
    &func_setsyspoweroff,
    &func_setsysreset,

    NULL  // MUST end by NULL
};

const menu_list_t pmu_menu =
{
    .menu_name = "PMU",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = pmu_menu_sub,
    .parent = &root_menu,
};

static ret_msg_t detect_power_key_status_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int onoff = -1;
    char statusbuf[64] = {0};
    arg_t out_arg[1] = {0};

    memset(statusbuf, 0, sizeof(statusbuf));

    onoff = sAPI_GetPowerKeyStatus();
    snprintf(statusbuf, sizeof(statusbuf),"onoff is %d.\r\n",onoff);

    LOG_INFO("onoff:%d!!!",onoff);

    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARGPMU_POWERSWITCHSTATUS;
    out_arg[0].value = pl_demo_make_value(0, statusbuf, strlen(statusbuf));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

    pl_demo_release_value(out_arg[0].value);

    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t onoff_power_key_shutdown_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int onoff = -1;
    char statusbuf[64] = {0};
    arg_t out_arg[1] = {0};
    int result = 0;
    const char *pmu = NULL;

    pl_demo_get_data(&pmu, argv, argn, ARGPMU_POWERSWITCH);
    if (!pmu)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    if(!strncmp(pmu,"open",4))
    {
        onoff = 1;
    }
    else
    {
        onoff = 0;
    }

    LOG_INFO("%s,gpionumber:%d", __func__, onoff);


    memset(statusbuf, 0, sizeof(statusbuf));

    result = sAPI_SetPowerKeyOffFunc(onoff, 50);   //50 x 100ms: about 5S
    snprintf(statusbuf, sizeof(statusbuf),"set onoff is %d.\r\n",result);

    LOG_INFO("onoff:%d!!!",result);

    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARGPMU_POWERSWITCH;
    out_arg[0].value = pl_demo_make_value(0, statusbuf, strlen(statusbuf));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

    pl_demo_release_value(out_arg[0].value);

    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t power_updown_event_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int powerup = -1;
    int powerdown = -1;
    char statusbuf[64] = {0};
    arg_t out_arg[1] = {0};


    memset(statusbuf, 0, sizeof(statusbuf));

    powerup = sAPI_GetPowerUpEvent();
    powerdown = sAPI_GetPowerDownEvent();

    snprintf(statusbuf, sizeof(statusbuf),"powerup is %d,powerdown is %d.\r\n",powerup,powerdown);

    LOG_INFO("powerup:%d,powerdown:%d!!!",powerup,powerdown);

    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARGPMU_POWEREVENT;
    out_arg[0].value = pl_demo_make_value(0, statusbuf, strlen(statusbuf));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

    pl_demo_release_value(out_arg[0].value);

    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t get_adc_votalge_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int ADC1 = -1;
    int ADC2 = -1;
    char statusbuf[64] = {0};
    arg_t out_arg[1] = {0};


    memset(statusbuf, 0, sizeof(statusbuf));

    ADC1 = sAPI_ReadAdc(1);
    ADC2 = sAPI_ReadAdc(2);

    snprintf(statusbuf, sizeof(statusbuf),"ADC1 VOL = %dmv\r\nADC2 VOL = %dmv.\r\n",ADC1,ADC2);

    LOG_INFO("ADC1:%d,ADC2:%d!!!",ADC1,ADC2);

    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARPMU_ADCVOTALGE;
    out_arg[0].value = pl_demo_make_value(0, statusbuf, strlen(statusbuf));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

    pl_demo_release_value(out_arg[0].value);

    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t get_vbat_votalge_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int VBAT = -1;
    char statusbuf[64] = {0};
    arg_t out_arg[1] = {0};


    memset(statusbuf, 0, sizeof(statusbuf));

    VBAT = sAPI_ReadVbat();

    snprintf(statusbuf, sizeof(statusbuf),"VBAT VOL =  %d.%03dV.\r\n",VBAT/1000, VBAT%1000);

    LOG_INFO("VBAT VOL =  %d.%03dV.\r\n",VBAT/1000, VBAT%1000);

    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARPMU_VBATVOTALGE;
    out_arg[0].value = pl_demo_make_value(0, statusbuf, strlen(statusbuf));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

    pl_demo_release_value(out_arg[0].value);

    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t set_vbat_votalge_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int voltage = 0;

    pl_demo_get_value(&voltage, argv, argn, ARPMU_SETVBAT);

    LOG_INFO("%s,voltage:%d", __func__, voltage);

    if (voltage > 3000 || voltage < 2500)
    {
        LOG_ERROR("param error!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param error fail!!!";
        goto EXIT;
    }

    if (sAPI_SetVddAux(voltage) != 0)
    {
        LOG_ERROR("sAPI_SetVddAux fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_SetVddAux fail!!!";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t set_syspoweroff_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    sAPI_SysPowerOff();

    return ret;
}

static ret_msg_t set_sysreset_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    sAPI_SysReset();

    return ret;
}