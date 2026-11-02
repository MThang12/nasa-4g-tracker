/*
 * @Author: Minghao.zhang
 * @Date: 2023-12-12 16:07:01
 * @Last Modified by: Minghao.zhang
 * @Last Modified time: 2023-12-12 16:07:08
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "demo_uart.h"
#include "simcom_uart.h"
#include "simcom_system.h"
#include "simcom_os.h"
#include "uart_api.h"


#define LOG_ERROR(...) sal_log_error("[DEMO-UART] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-UART] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-UART] " __VA_ARGS__)

SCuartConfiguration config = {SC_UART_BAUD_115200, SC_UART_WORD_LEN_8, SC_UART_ONE_STOP_BIT, SC_UART_NO_PARITY_BITS};

static ret_msg_t set_baud_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_databits_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_stopbits_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_paritybit_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_uartconfig_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_uartstatus_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t uartctl_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t sleepset_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t sleepget_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t alarmwakeup_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t Internal_AT_cmd_proc(op_t op, arg_t *argv, int argn);

const value_t uartport[] =
{
    {
        .bufp = "SC_UART",
        .size = 7,
    },
    {
        .bufp = "SC_UART2",
        .size = 8,
    },
    {
        .bufp = "SC_UART3",
        .size = 8,
    },
    {
        .bufp = "SC_UART4",
        .size = 8,
    }
};
const value_list_t uartport_range =
{
    .isrange = false,
    .list_head = uartport,
    .count = 4,
};

const value_t baudrate[] =
{
    {
        .bufp = "300",
        .size = 3,
    },
    {
        .bufp = "600",
        .size = 3,
    },
    {
        .bufp = "1200",
        .size = 4,
    },
    {
        .bufp = "2400",
        .size = 4,
    },
    {
        .bufp = "3600",
        .size = 4,
    },
    {
        .bufp = "4800",
        .size = 4,
    },
    {
        .bufp = "9600",
        .size = 4,
    },
    {
        .bufp = "19200",
        .size = 5,
    },
    {
        .bufp = "38400",
        .size = 5,
    },
    {
        .bufp = "57600",
        .size = 5,
    },
    {
        .bufp = "115200",
        .size = 6,
    },
    {
        .bufp = "230400",
        .size = 6,
    },
    {
        .bufp = "460800",
        .size = 6,
    },
    {
        .bufp = "921600",
        .size = 6,
    },
    {
        .bufp = "1842000",
        .size = 7,
    },
    {
        .bufp = "3686400",
        .size = 7,
    }
};
const value_list_t baudrate_range =
{
    .isrange = false,
    .list_head = baudrate,
    .count = 16,
};

arg_t setbaud_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGUART_UARTPORT,
        .value_range = &uartport_range,
        .msg = "port"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGUART_BAUDRATE,
        .value_range = &baudrate_range,
        .msg = "baudrate"
    },
};

const menu_list_t func_setbaud =
{
    .menu_name = "Set baud rate",
    .menu_type = TYPE_FUNC,
    .parent = &uart_menu,
    .function = {
        .argv = setbaud_args,
        .argn = sizeof(setbaud_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_baud_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const value_t databits[] =
{
    {
        .bufp = "5",
        .size = 1,
    },
    {
        .bufp = "6",
        .size = 1,
    },
    {
        .bufp = "7",
        .size = 1,
    },
    {
        .bufp = "8",
        .size = 1,
    }
};
const value_list_t databits_range =
{
    .isrange = false,
    .list_head = databits,
    .count = 4,
};

arg_t setdatabits_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGUART_UARTPORT,
        .value_range = &uartport_range,
        .msg = "port"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGUART_DATABITS,
        .value_range = &databits_range,
        .msg = "databits"
    },
};

const menu_list_t func_setdatabits =
{
    .menu_name = "Set data bits",
    .menu_type = TYPE_FUNC,
    .parent = &uart_menu,
    .function = {
        .argv = setdatabits_args,
        .argn = sizeof(setdatabits_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_databits_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const value_t stopbits[] =
{
    {
        .bufp = "1bits",
        .size = 5,
    },
    {
        .bufp = "1.5bits_or_2bits",
        .size = 16,
    }
};
const value_list_t stopbits_range =
{
    .isrange = false,
    .list_head = stopbits,
    .count = 2,
};

arg_t setstopbits_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGUART_UARTPORT,
        .value_range = &uartport_range,
        .msg = "port"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGUART_STOPBITS,
        .value_range = &stopbits_range,
        .msg = "stopbits"
    },
};

const menu_list_t func_setstopbits =
{
    .menu_name = "Set stop bits",
    .menu_type = TYPE_FUNC,
    .parent = &uart_menu,
    .function = {
        .argv = setstopbits_args,
        .argn = sizeof(setstopbits_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_stopbits_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const value_t paritybits[] =
{
    {
        .bufp = "no_parity_bit",
        .size = 13,
    },
    {
        .bufp = "odd_parity_bit",
        .size = 14,
    },
    {
        .bufp = "even_parity_bit",
        .size = 15,
    }
};
const value_list_t paritybits_range =
{
    .isrange = false,
    .list_head = paritybits,
    .count = 3,
};

arg_t setparitybit_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGUART_UARTPORT,
        .value_range = &uartport_range,
        .msg = "port"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGUART_PARITYBIT,
        .value_range = &paritybits_range,
        .msg = "paritybits"
    },
};

const menu_list_t func_setparitybit =
{
    .menu_name = "Set parity rate",
    .menu_type = TYPE_FUNC,
    .parent = &uart_menu,
    .function = {
        .argv = setparitybit_args,
        .argn = sizeof(setparitybit_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_paritybit_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

arg_t getconfig_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGUART_UARTPORT,
        .value_range = &uartport_range,
        .msg = "port"
    }
};

const menu_list_t func_getconfig =
{
    .menu_name = "Get UART configration",
    .menu_type = TYPE_FUNC,
    .parent = &uart_menu,
    .function = {
        .argv = getconfig_args,
        .argn = sizeof(getconfig_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = get_uartconfig_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

arg_t getstatus_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGUART_UARTPORT,
        .value_range = &uartport_range,
        .msg = "port"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGUART_RXTIMEOUT,
        .msg = "a threshold that If Rx exceeds the threshold and no data is received, Rx is idle, otherwise Rx is busy.  99(back)"
    }
};

const menu_list_t func_getstatus =
{
    .menu_name = "Get UART status",
    .menu_type = TYPE_FUNC,
    .parent = &uart_menu,
    .function = {
        .argv = getstatus_args,
        .argn = sizeof(getstatus_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = get_uartstatus_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const value_t uartctl[] =
{
    {
        .bufp = "open",
        .size = 4,
    },
    {
        .bufp = "close",
        .size = 5,
    }
};
const value_list_t uartctl_range =
{
    .isrange = false,
    .list_head = uartctl,
    .count = 2,
};

arg_t uartctl_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGUART_UARTPORT,
        .value_range = &uartport_range,
        .msg = "port"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGUART_UARTCTL,
        .value_range = &uartctl_range,
        .msg = "uartctl"
    }
};

const menu_list_t func_uartctl =
{
    .menu_name = "UART open/close",
    .menu_type = TYPE_FUNC,
    .parent = &uart_menu,
    .function = {
        .argv = uartctl_args,
        .argn = sizeof(uartctl_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = uartctl_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const value_t sleepctl[] =
{
    {
        .bufp = "sleep",
        .size = 5,
    },
    {
        .bufp = "wakeup",
        .size = 6,
    }
};
const value_list_t sleepctl_range =
{
    .isrange = false,
    .list_head = sleepctl,
    .count = 2,
};

arg_t sleepctl_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGUART_SLEEPCTL,
        .value_range = &sleepctl_range,
        .msg = "sleepctl"
    }
};

const menu_list_t func_sleepset =
{
    .menu_name = "Set sleep",
    .menu_type = TYPE_FUNC,
    .parent = &uart_menu,
    .function = {
        .argv = sleepctl_args,
        .argn = sizeof(sleepctl_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = sleepset_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_sleepget =
{
    .menu_name = "Get sleep status",
    .menu_type = TYPE_FUNC,
    .parent = &uart_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = sleepget_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

arg_t alarmwakeup_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGUART_ALARMCLOCKTIME,
        .msg = "Please input alarmclockwakeuptime: /ms"
    }
};

const menu_list_t func_alarmwakeup =
{
    .menu_name = "alarm clock to wakeup",
    .menu_type = TYPE_FUNC,
    .parent = &uart_menu,
    .function = {
        .argv = alarmwakeup_args,
        .argn = sizeof(alarmwakeup_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = alarmwakeup_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

arg_t InternalAT_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGUART_INTERNALAT,
        .msg = "Input AT cmd"
    }
};

const menu_list_t func_Internal_AT_cmd =
{
    .menu_name = "Internal AT command",
    .menu_type = TYPE_FUNC,
    .parent = &uart_menu,
    .function = {
        .argv = InternalAT_args,
        .argn = sizeof(InternalAT_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = Internal_AT_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t *uart_menu_sub[] =
{
    &func_setbaud,
    &func_setdatabits,
    &func_setstopbits,
    &func_setparitybit,
    &func_getconfig,
    &func_getstatus,
    &func_uartctl,
    &func_sleepset,
    &func_sleepget,
    &func_alarmwakeup,
    &func_Internal_AT_cmd,

    NULL  // MUST end by NULL
};

const menu_list_t uart_menu =
{
    .menu_name = "UART",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = uart_menu_sub,
    .parent = &root_menu,
};

void GetUartPort(int *port, arg_t *argv, int argn)
{
    const char *portnum = NULL;
    int uartport = 0;

    pl_demo_get_data(&portnum, argv, argn, ARGUART_UARTPORT);

    if(!strncmp(portnum,"SC_UART",8))
    {
        uartport = 1;
    }
    else if(!strncmp(portnum,"SC_UART2",8))
    {
        uartport = 2;
    }
    else if(!strncmp(portnum,"SC_UART3",8))
    {
        uartport = 3;
    }
    else if(!strncmp(portnum,"SC_UART4",8))
    {
        uartport = 4;
    }

    *port = uartport;
}

static ret_msg_t set_baud_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *baudrate = NULL;
    int port = 0;

    pl_demo_get_data(&baudrate, argv, argn, ARGUART_BAUDRATE);
    GetUartPort(&port, argv, argn);
    LOG_INFO("%s,portnumber:%d , set_baudrate:[%d]", __func__, port, atoi(baudrate));
    if (baudRateIsValid(atoi(baudrate)))
    {
        LOG_ERROR("baudrate invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "baudrate invalid";
        goto EXIT;
    }

    config.BaudRate = (SC_UART_BaudRates)atoi(baudrate);

    if (sAPI_UartSetConfig(port, &config) == SC_UART_RETURN_CODE_ERROR)
    {
        LOG_ERROR("sAPI_UartSetConfig fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_UartSetConfig fail!!!";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t set_databits_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *databits = NULL;
    int port = 0;

    pl_demo_get_data(&databits, argv, argn, ARGUART_DATABITS);
    GetUartPort(&port, argv, argn);
    LOG_INFO("%s,portnumber:%d , set_databits:[%d]", __func__, port, atoi(databits));
    if (dataBitIsValid(atoi(databits)))
    {
        LOG_ERROR("databits invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "databits invalid";
        goto EXIT;
    }

    config.DataBits = (SC_UART_WordLen)atoi(databits);

    if (sAPI_UartSetConfig(port, &config) == SC_UART_RETURN_CODE_ERROR)
    {
        LOG_ERROR("sAPI_UartSetConfig fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_UartSetConfig fail!!!";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t set_stopbits_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *stopbits = 0;
    int port = 0;

    pl_demo_get_data(&stopbits, argv, argn, ARGUART_STOPBITS);
    GetUartPort(&port, argv, argn);
    
    if(!strncmp(stopbits,"1bits",5))
    {
        config.StopBits = (SC_UART_StopBits)0;
    }
    else if(!strncmp(stopbits,"1.5bits_or_2bits",16))
    {
        config.StopBits = (SC_UART_StopBits)1;
    }
    
    LOG_INFO("%s,portnumber:%d , set_stopbits:[%d]", __func__, port, config.StopBits);

    if (stopBitIsValid(config.StopBits))
    {
        LOG_ERROR("stopbits invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "stopbits invalid";
        goto EXIT;
    }

    if (sAPI_UartSetConfig(port, &config) == SC_UART_RETURN_CODE_ERROR)
    {
        LOG_ERROR("sAPI_UartSetConfig fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_UartSetConfig fail!!!";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t set_paritybit_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *paritybit = NULL;
    int port = 0;

    pl_demo_get_data(&paritybit, argv, argn, ARGUART_PARITYBIT);
    GetUartPort(&port, argv, argn);

    if(!strncmp(paritybit,"no_parity_bit",13))
    {
        config.ParityBit = (SC_UART_ParityTBits)0;
    }
    else if(!strncmp(paritybit,"odd_parity_bit",14))
    {
        config.ParityBit = (SC_UART_ParityTBits)1;
    }
    else if(!strncmp(paritybit,"even_parity_bit",15))
    {
        config.ParityBit = (SC_UART_ParityTBits)3;
    }
    LOG_INFO("%s,portnumber:%d , set_paritybit:[%d]", __func__, port, config.ParityBit);
    if (parityBitIsValid(config.ParityBit))
    {
        LOG_ERROR("paritybit invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "paritybit invalid";
        goto EXIT;
    }

    if (sAPI_UartSetConfig(port, &config) == SC_UART_RETURN_CODE_ERROR)
    {
        LOG_ERROR("sAPI_UartSetConfig fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_UartSetConfig fail!!!";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t get_uartconfig_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char configbuf[256] = {0};
    int port = 0;
    arg_t out_arg[1] = {0};

    GetUartPort(&port, argv, argn);
    LOG_INFO("%s,portnumber:%d ", __func__, port);

    if (sAPI_UartGetConfig(port, &config) == SC_UART_RETURN_CODE_ERROR)
    {
        LOG_ERROR("sAPI_UartGetConfig fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_UartGetConfig fail!!!";
        goto EXIT;
    }

    memset(configbuf, 0, sizeof(configbuf));

    snprintf(configbuf, sizeof(configbuf),"baud rate is: %d  ",config.BaudRate);

    switch (config.DataBits)
    {
        case SC_UART_WORD_LEN_5:
            strcat(configbuf, "data bit is: 5bits  ");
            break;
        case SC_UART_WORD_LEN_6:
            strcat(configbuf, "data bit is: 6bits  ");
            break;
        case SC_UART_WORD_LEN_7:
            strcat(configbuf, "data bit is: 7bits  ");
            break;
        case SC_UART_WORD_LEN_8:
            strcat(configbuf, "data bit is: 8bits  ");
        default:
            break;
    }
    switch (config.ParityBit)
    {
        case SC_UART_NO_PARITY_BITS:
            strcat(configbuf, "parity bit is: no parity  ");
            break;
        case SC_UART_EVEN_PARITY_SELECT:
            strcat(configbuf, "parity bit is: even parity  ");
            break;
        case SC_UART_ODD_PARITY_SELECT:
            strcat(configbuf, "parity bit is: odd parity  ");
            break;
        default:
            break;
    }
    switch (config.StopBits)
    {
        case SC_UART_ONE_STOP_BIT:
            strcat(configbuf, "stop bits is: 1bits  ");
            break;
        case SC_UART_ONE_HALF_OR_TWO_STOP_BITS:
            strcat(configbuf, "stop bits is: 1.5bits or 2bits  ");
            break;
        default:
            break;
    }

    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARGUART_UARTCONFIG;
    out_arg[0].value = pl_demo_make_value(0, configbuf, strlen(configbuf));

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

static ret_msg_t get_uartstatus_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char statusbuf[64] = {0};
    int port = 0;
    int timeout = 0;
    arg_t out_arg[1] = {0};

    GetUartPort(&port, argv, argn);
    pl_demo_get_value(&timeout, argv, argn, ARGUART_RXTIMEOUT);
    LOG_INFO("%s,portnumber:%d timeout:%d", __func__, port, timeout);

    memset(statusbuf, 0, sizeof(statusbuf));

    switch (sAPI_UartRxStatus(port, timeout))
    {
    case SC_UART_RX_IDEL:
        snprintf(statusbuf, sizeof(statusbuf),"UART3 is idel.\r\n");
        break;
    case SC_UART_RX_BUSY:
        snprintf(statusbuf, sizeof(statusbuf),"UART3 is busy.\r\n");
        break;
    case SC_UART_RX_STATUS_MAX:
        LOG_ERROR("sAPI_UartRxStatus fail,Port is invalid!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_UartRxStatus fail,Port is invalid!!!";
        goto EXIT;
        break;
    default:
        break;
    }

    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARGUART_UARTSTATUS;
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

static ret_msg_t uartctl_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int port = 0;
    const char *option = NULL;
    int uartctl = 0;

    GetUartPort(&port, argv, argn);
    pl_demo_get_data(&option, argv, argn, ARGUART_UARTCTL);
    LOG_INFO("%s,portnumber:%d option:%s", __func__, port, option);

    if(!strncmp(option,"open",4))
    {
        uartctl = (SC_Uart_Control)0;
    }
    else if(!strncmp(option,"close",5))
    {
        uartctl = (SC_Uart_Control)1;
    }

    if (sAPI_UartControl(port, uartctl) == SC_UART_RETURN_CODE_ERROR)
    {
        LOG_ERROR("sAPI_UartControl fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_UartControl fail!!!";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t sleepset_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char buf[128] = {0};
    arg_t out_arg[1] = {0};
    const char *sleepset = NULL;
    int sleepctl = 0;

    pl_demo_get_data(&sleepset, argv, argn, ARGUART_SLEEPCTL);

    if(!strncmp(sleepset,"sleep",5))
    {
        sleepctl = (SC_SYSTEM_SLEEP_FLAG)1;
    }
    else if(!strncmp(sleepset,"wakeup",6))
    {
        sleepctl = (SC_SYSTEM_SLEEP_FLAG)0;
    }

    memset(buf, 0, sizeof(buf));

    snprintf(buf, sizeof(buf),"Please unplug the USB to put the module into sleep mode. Pull down the DTR pin to wake up the module.");

    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "Note";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

    pl_demo_release_value(out_arg[0].value);

    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
        goto EXIT;
    }

    if ((SC_Uart_Return_Code)sAPI_SystemSleepSet(sleepctl) == SC_UART_RETURN_CODE_ERROR)
    {
        LOG_ERROR("sAPI_SystemSleepSet fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_SystemSleepSet fail!!!";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t sleepget_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char buf[64] = {0};
    arg_t out_arg[1] = {0};

    memset(buf, 0, sizeof(buf));

    snprintf(buf, sizeof(buf),"Sleep status is %d \r\n", sAPI_SystemSleepGet());

    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "Sleep status";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

    pl_demo_release_value(out_arg[0].value);

    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
    }

    return ret;
}

static ret_msg_t alarmwakeup_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char buf[128] = {0};
    int time = 0;
    arg_t out_arg[1] = {0};

    pl_demo_get_value(&time, argv, argn, ARGUART_ALARMCLOCKTIME);

    memset(buf, 0, sizeof(buf));

    snprintf(buf, sizeof(buf),"Please unplug the USB to put the module into sleep mode. Pull down the DTR pin to wake up the module.");

    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "Note";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

    pl_demo_release_value(out_arg[0].value);

    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
        goto EXIT;
    }

    if ((SC_Uart_Return_Code)sAPI_SystemAlarmClock2Wakeup(time) == SC_UART_RETURN_CODE_ERROR)
    {
        LOG_ERROR("sAPI_SystemAlarmClock2Wakeup fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_SystemAlarmClock2Wakeup fail!!!";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t Internal_AT_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char respStr[64];
    arg_t out_arg[1] = {0};
    const char *ATcmd = NULL;
    char AT[32];

    memset(respStr, 0, sizeof(respStr));
    memset(AT, 0, sizeof(AT));

    pl_demo_get_data(&ATcmd, argv, argn, ARGUART_INTERNALAT);

    snprintf(AT, sizeof(AT),"%s\r",ATcmd);

    LOG_INFO("%s,Internal_AT: %s",__func__,AT);

    if (sAPI_SendATCMDWaitResp(10, AT, 150, NULL, 1, NULL, respStr, sizeof(respStr)))
    {
        LOG_ERROR("ATCMD NOT SUPPORT!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "ATCMD NOT SUPPORT!";
        goto EXIT;
    }

    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "Result";
    out_arg[0].value = pl_demo_make_value(0, respStr, strlen(respStr));

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
