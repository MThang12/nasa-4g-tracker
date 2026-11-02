/*
 * @Author: yangkui.zhang
 * @Date: 2023-12-22 16:45:37
 * @LastEditTime: 2024-04-28 10:34:57
 * @LastEditors: yangkui.zhang
 * @Description: 
 * @FilePath: \ASR160X\simcom\userspace\sc_demo_v2\src\demo_gpio.c
 * no error,no warning
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "simcom_demo_init.h"
#include "demo_gpio.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "simcom_os.h"
#include "simcom_gpio.h"

#define LOG_ERROR(...) sal_log_error("[DEMO-GPIO] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-GPIO] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-GPIO] " __VA_ARGS__)

int *p = all_gpio;

static void GPIO_IntHandler(void)
{
#ifdef GPIO_INT_WAKEUP_TEST
    static unsigned int i = 0;

    if (i % 2 == 0)
        sAPI_GpioSetValue(SC_MODULE_GPIO_10, 1);
    else
        sAPI_GpioSetValue(SC_MODULE_GPIO_10, 0);

    i++;
#endif
}

void GPIO_WakeupHandler(void)
{
#ifdef GPIO_INT_WAKEUP_TEST
    static unsigned int i = 0;

    if (i % 2 == 0)
        sAPI_GpioSetValue(SC_MODULE_GPIO_09, 1);
    else
        sAPI_GpioSetValue(SC_MODULE_GPIO_09, 0);

    i++;
#endif
    /* If you want to leave sleep after wake up, please disable system sleep ! */
    //sAPI_SystemSleepSet(0);
}

static ret_msg_t set_gpio_direction_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_gpio_direction_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_gpio_level_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_gpio_level_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_gpio_interrupt_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_gpio_wakeupenable_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_gpio_config_cmd_proc(op_t op, arg_t *argv, int argn);
//static ret_msg_t set_gpio_turn_on_test_cmd_proc(op_t op, arg_t *argv, int argn);
//static ret_msg_t set_gpio_turn_off_test_cmd_proc(op_t op, arg_t *argv, int argn);


const value_t directions[] =
{
    {
        .bufp = "input",
        .size = 5,
    },
    {
        .bufp = "output",
        .size = 6,
    }
};
const value_list_t direction_range =
{
    .isrange = false,
    .list_head = directions,
    .count = 2,
};

arg_t setgpiodirection_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGGPIO_NUMBER,
        .msg = "Please input gpio number:"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGGPIO_DIRECTION,
        .value_range = &direction_range,
    }
};

arg_t getgpiodirection_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGGPIO_NUMBER,
        .msg = "Please input gpio number:"
    }
};

const value_t levels[] =
{
    {
        .bufp = "low",
        .size = 3,
    },
    {
        .bufp = "high",
        .size = 4,
    }
};
const value_list_t level_range =
{
    .isrange = false,
    .list_head = levels,
    .count = 2,
};

arg_t setgpiolevel_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGGPIO_NUMBER,
        .msg = "Please input gpio number:"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGGPIO_LEVEL,
        .value_range = &level_range,
    }
};

arg_t getgpiolevel_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGGPIO_NUMBER,
        .msg = "Please input gpio number:"
    }
};

const value_t interrupts[] =
{
    {
        .bufp = "noedge",
        .size = 6,
    },
    {
        .bufp = "rise",
        .size = 4,
    },
    {
        .bufp = "fall",
        .size = 4,
    },
    {
        .bufp = "twoedge",
        .size = 7,
    }
};
const value_list_t interrupt_range =
{
    .isrange = false,
    .list_head = interrupts,
    .count = 4,
};

arg_t setgpiointerrupt_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGGPIO_NUMBER,
        .msg = "Please input gpio number:"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGGPIO_INTERRUPT,
        .value_range = &interrupt_range,
    }
};

const value_t wakeupenables[] =
{
    {
        .bufp = "noedge",
        .size = 6,
    },
    {
        .bufp = "rise",
        .size = 4,
    },
    {
        .bufp = "fall",
        .size = 4,
    },
    {
        .bufp = "twoedge",
        .size = 7,
    }
};
const value_list_t wakeupenable_range =
{
    .isrange = false,
    .list_head = wakeupenables,
    .count = 4,
};

arg_t setgpiowakeupenable_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGGPIO_NUMBER,
        .msg = "Please input gpio number:"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGGPIO_WAKEUPENABLE,
        .value_range = &wakeupenable_range,
    }
};

const value_t pullupdown[] =
{
    {
        .bufp = "disable",
        .size = 7,
    },
    {
        .bufp = "up",
        .size = 2,
    },
    {
        .bufp = "down",
        .size = 4,
    }
};
const value_list_t pullupdown_range =
{
    .isrange = false,
    .list_head = pullupdown,
    .count = 3,
};

arg_t setgpioconfig_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGGPIO_NUMBER,
        .msg = "Please input gpio number:"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGGPIO_DIRECTION,
        .value_range = &direction_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGGPIO_LEVEL,
        .value_range = &level_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGGPIO_PULLUPDOWN,
        .value_range = &pullupdown_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGGPIO_INTERRUPT,
        .value_range = &interrupt_range,
    }
};

const menu_list_t func_setgpiodirection =
{
    .menu_name = "Set gpio direction",
    .menu_type = TYPE_FUNC,
    .parent = &gpio_menu,
    .function = {
        .argv = setgpiodirection_args,
        .argn = sizeof(setgpiodirection_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_gpio_direction_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_getgpiodirection =
{
    .menu_name = "Get gpio direction",
    .menu_type = TYPE_FUNC,
    .parent = &gpio_menu,
    .function = {
        .argv = getgpiodirection_args,
        .argn = sizeof(getgpiodirection_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = get_gpio_direction_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_setgpiolevel =
{
    .menu_name = "Set gpio level",
    .menu_type = TYPE_FUNC,
    .parent = &gpio_menu,
    .function = {
        .argv = setgpiolevel_args,
        .argn = sizeof(setgpiolevel_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_gpio_level_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_getgpiolevel =
{
    .menu_name = "Get gpio level",
    .menu_type = TYPE_FUNC,
    .parent = &gpio_menu,
    .function = {
        .argv = getgpiolevel_args,
        .argn = sizeof(getgpiolevel_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = get_gpio_level_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_setgpiointerrupt =
{
    .menu_name = "Set gpio interrupt",
    .menu_type = TYPE_FUNC,
    .parent = &gpio_menu,
    .function = {
        .argv = setgpiointerrupt_args,
        .argn = sizeof(setgpiointerrupt_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_gpio_interrupt_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_setgpiowakeupenable =
{
    .menu_name = "Set gpio wakeup enable",
    .menu_type = TYPE_FUNC,
    .parent = &gpio_menu,
    .function = {
        .argv = setgpiowakeupenable_args,
        .argn = sizeof(setgpiowakeupenable_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_gpio_wakeupenable_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_setgpioconfig =
{
    .menu_name = "Set gpio config",
    .menu_type = TYPE_FUNC,
    .parent = &gpio_menu,
    .function = {
        .argv = setgpioconfig_args,
        .argn = sizeof(setgpioconfig_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_gpio_config_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

#if 0
const menu_list_t func_setgpioturnontest =
{
    .menu_name = "Set gpio turn on test",
    .menu_type = TYPE_FUNC,
    .parent = &gpio_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_gpio_turn_on_test_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_setgpioturnofftest =
{
    .menu_name = "Set gpio turn off test",
    .menu_type = TYPE_FUNC,
    .parent = &gpio_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_gpio_turn_off_test_cmd_proc,
        .methods.needRAWData = NULL,
    },
};
#endif

const menu_list_t *gpio_menu_sub[] =
{
    &func_setgpiodirection,
    &func_getgpiodirection,
    &func_setgpiolevel,
    &func_getgpiolevel,
    &func_setgpiointerrupt,
    &func_setgpiowakeupenable,
    &func_setgpioconfig,
    //&func_setgpioturnontest,
    //&func_setgpioturnofftest,

    NULL  // MUST end by NULL
};

const menu_list_t gpio_menu =
{
    .menu_name = "GPIO",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = gpio_menu_sub,
    .parent = &root_menu,
};

static ret_msg_t set_gpio_direction_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int gpio_num = 0;
    int direction = 0;
    const char *gpio = NULL;

    pl_demo_get_value(&gpio_num, argv, argn, ARGGPIO_NUMBER);
    pl_demo_get_data(&gpio, argv, argn, ARGGPIO_DIRECTION);
    if (!gpio)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    if(!strncmp(gpio,"input",5))
    {
        direction = 0;
    }
    else
    {
        direction = 1;
    }
    LOG_INFO("%s,gpionumber:%d , set_gpiodirection:[%d]", __func__, gpio_num, direction);

    if (sAPI_GpioSetDirection(gpio_num, direction) != SC_GPIORC_OK)
    {
        LOG_ERROR("sAPI_GpioSetDirection fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_GpioSetDirection fail!!!";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t get_gpio_direction_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char statusbuf[64] = {0};
    int gpio_num = 0;
    int direction = 0;
    arg_t out_arg[1] = {0};

    pl_demo_get_value(&gpio_num, argv, argn, ARGGPIO_NUMBER);

    LOG_INFO("%s,gpionumber:%d", __func__, gpio_num);

    memset(statusbuf, 0, sizeof(statusbuf));

    direction = sAPI_GpioGetDirection(gpio_num);
    if(1 == direction)
    {
        snprintf(statusbuf, sizeof(statusbuf),"direction is output.\r\n");
    }
    else
    {
        snprintf(statusbuf, sizeof(statusbuf),"direction is input.\r\n");
    }

    LOG_INFO("gpio%d,direction:%d!!!",gpio_num,direction);

    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARGGPIO_DIRECTION;
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

static ret_msg_t set_gpio_level_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int gpio_num = 0;
    int level = 0;
    const char *gpio = NULL;

    pl_demo_get_value(&gpio_num, argv, argn, ARGGPIO_NUMBER);
    pl_demo_get_data(&gpio, argv, argn, ARGGPIO_LEVEL);
    if (!gpio)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    if(!strncmp(gpio,"low",3))
    {
        level = 0;
    }
    else
    {
        level = 1;
    }
    LOG_INFO("%s,gpionumber:%d , set_gpiolevel:[%d]", __func__, gpio_num, level);

    if (sAPI_GpioSetValue(gpio_num, level) != SC_GPIORC_OK)
    {
        LOG_ERROR("sAPI_GpioSetValue fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_GpioSetValue fail!!!";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t get_gpio_level_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char statusbuf[64] = {0};
    int gpio_num = 0;
    int level = 0;
    arg_t out_arg[1] = {0};

    pl_demo_get_value(&gpio_num, argv, argn, ARGGPIO_NUMBER);

    LOG_INFO("%s,gpionumber:%d", __func__, gpio_num);

    memset(statusbuf, 0, sizeof(statusbuf));

    level = sAPI_GpioGetValue(gpio_num);
    if(1 == level)
    {
        snprintf(statusbuf, sizeof(statusbuf),"level is high.\r\n");
    }
    else
    {
        snprintf(statusbuf, sizeof(statusbuf),"level is low.\r\n");
    }

    LOG_INFO("gpio%d,level:%d!!!",gpio_num,level);

    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARGGPIO_LEVEL;
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

static ret_msg_t set_gpio_interrupt_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int gpio_num = 0;
    int interrupt = 0;
    const char *gpio = NULL;

    pl_demo_get_value(&gpio_num, argv, argn, ARGGPIO_NUMBER);
    pl_demo_get_data(&gpio, argv, argn, ARGGPIO_INTERRUPT);
    if (!gpio)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    if(!strncmp(gpio,"noedge",6))
    {
        interrupt = 0;
    }
    else if(!strncmp(gpio,"rise",4))
    {
        interrupt = 1;
    }
    else if(!strncmp(gpio,"fall",4))
    {
        interrupt = 2;
    }
    else
    {
        interrupt = 3;
    }

    LOG_INFO("%s,gpionumber:%d , set_gpiointerrupt:[%d]", __func__, gpio_num, interrupt);

    if (sAPI_GpioSetDirection(gpio_num, SC_GPIO_IN_PIN) != SC_GPIORC_OK)
    {
        LOG_ERROR("sAPI_GpioSetDirection fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_GpioSetDirection fail!!!";
        goto EXIT;
    }

    if (sAPI_GpioConfigInterrupt(gpio_num, interrupt, GPIO_IntHandler) != SC_GPIORC_OK)
    {
        LOG_ERROR("sAPI_GpioConfigInterrupt fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_GpioConfigInterrupt fail!!!";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t set_gpio_wakeupenable_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int gpio_num = 0;
    int wakeupenable = 0;
    const char *gpio = NULL;

    pl_demo_get_value(&gpio_num, argv, argn, ARGGPIO_NUMBER);
    pl_demo_get_data(&gpio, argv, argn, ARGGPIO_WAKEUPENABLE);
    if (!gpio)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    if(!strncmp(gpio,"noedge",6))
    {
        wakeupenable = 0;
    }
    else if(!strncmp(gpio,"rise",4))
    {
        wakeupenable = 1;
    }
    else if(!strncmp(gpio,"fall",4))
    {
        wakeupenable = 2;
    }
    else
    {
        wakeupenable = 3;
    }
    LOG_INFO("%s,gpionumber:%d , set_gpiowakeupenable:[%d]", __func__, gpio_num, wakeupenable);

    if (sAPI_GpioWakeupEnable(gpio_num, wakeupenable) != SC_GPIORC_OK)
    {
        LOG_ERROR("sAPI_GpioWakeupEnable fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_GpioWakeupEnable fail!!!";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t set_gpio_config_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int gpio_num = 0;
    int direction = 0;
    int intlevel = 0;
    int pulltype = 0;
    int interrupt = 0;
    const char *gpio = NULL;

    SC_GPIOConfiguration pinconfig;

    pl_demo_get_value(&gpio_num, argv, argn, ARGGPIO_NUMBER);
    pl_demo_get_data(&gpio, argv, argn, ARGGPIO_DIRECTION);
    if (!gpio)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    if(!strncmp(gpio,"input",5))
    {
        direction = 0;
    }
    else
    {
        direction = 1;
    }
    pl_demo_get_data(&gpio, argv, argn, ARGGPIO_LEVEL);
    if (!gpio)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    if(!strncmp(gpio,"low",3))
    {
        intlevel = 0;
    }
    else
    {
        intlevel = 1;
    }
    pl_demo_get_data(&gpio, argv, argn, ARGGPIO_PULLUPDOWN);
    if (!gpio)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    if(!strncmp(gpio,"disable",7))
    {
        pulltype = 0;
    }
    else if(!strncmp(gpio,"up",2))
    {
        pulltype = 1;
    }
    else
    {
        pulltype = 2;
    }
    pl_demo_get_data(&gpio, argv, argn, ARGGPIO_INTERRUPT);
    if (!gpio)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    if(!strncmp(gpio,"noedge",6))
    {
        interrupt = 0;
    }
    else if(!strncmp(gpio,"rise",4))
    {
        interrupt = 1;
    }
    else if(!strncmp(gpio,"fall",4))
    {
        interrupt = 2;
    }
    else
    {
        interrupt = 3;
    }
    LOG_INFO("%s,gpionumber:%d , direction:[%d] , intlevel:[%d] , pulltype:[%d] , interrupt:[%d]", __func__, gpio_num, direction,intlevel,pulltype,interrupt);

    pinconfig.pinDir = direction;
    pinconfig.initLv = intlevel;
    pinconfig.pinPull = pulltype;
    pinconfig.pinEd = interrupt;
    if (pinconfig.pinEd == SC_GPIO_RISE_EDGE || pinconfig.pinEd == SC_GPIO_FALL_EDGE || pinconfig.pinEd == SC_GPIO_TWO_EDGE)
    {
        pinconfig.isr = GPIO_IntHandler;
    }
    else
    {
        pinconfig.isr = NULL;
    }
    pinconfig.wu = GPIO_WakeupHandler;

    if (sAPI_GpioConfig(gpio_num, pinconfig) != SC_GPIORC_OK)
    {
        LOG_ERROR("sAPI_GpioConfig fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_GpioConfig fail!!!";
        goto EXIT;
    }

EXIT:
    return ret;
}

#if 0
static ret_msg_t set_gpio_turn_on_test_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0;
    char statusbuf[128] = {0};
    arg_t out_arg[1] = {0};

    SC_GPIOConfiguration gpio_cfg =
    {
        .initLv = 1,
        .isr = NULL,
        .pinDir = 1,
        .pinEd = 0,
        .wu = NULL,
        .pinPull = 0,
    };
    int *p = all_gpio;

    memset(statusbuf, 0, sizeof(statusbuf));
    do
    {

        result = sAPI_GpioConfig(*p, gpio_cfg);
        if (result)
        {
            snprintf(statusbuf,sizeof(statusbuf),"gpio [%d] cfg fail\r\n", *p);
        }

        if (sAPI_GpioGetValue(*p) != 1)
        {

            snprintf(statusbuf,sizeof(statusbuf),"gpio [%d] is not high\r\n",*p);
        }
    } while (*p ++ != -1);

    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARGGPIO_CONFIG;
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

static ret_msg_t set_gpio_turn_off_test_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0;
    char statusbuf[128] = {0};
    arg_t out_arg[1] = {0};

    SC_GPIOConfiguration gpio_cfg =
    {
        .initLv = 0,
        .isr = NULL,
        .pinDir = 1,
        .pinEd = 0,
        .wu = NULL,
        .pinPull = 0,
    };
    int *p = all_gpio;

    memset(statusbuf, 0, sizeof(statusbuf));
    do
    {
        result = sAPI_GpioConfig(*p, gpio_cfg);
        if (result)
        {
            snprintf(statusbuf,sizeof(statusbuf),"gpio[%d] cfg fail\r\n", *p);
        }

        if (sAPI_GpioGetValue(*p) != 1)
        {
            snprintf(statusbuf,sizeof(statusbuf),"gpio[%d] is not low\r\n", *p);
        }
    } while (*p ++ != -1);

    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARGGPIO_CONFIG;
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

#endif