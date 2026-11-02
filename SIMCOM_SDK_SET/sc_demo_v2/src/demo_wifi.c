#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "demo_wifi.h"
#include "simcom_os.h"
#include "simcom_wifi.h"
#include "simcom_demo_init.h"

#include "sal_os.h"
#include "sal_log.h"

#define CHAR_TO_INT(x) ((unsigned char)(x) - '0')
/**LOG define*/
#define LOG(...)       sal_log(__VA_ARGS__)
#define LOG_ERROR(...) sal_log_error("[DEMO-WIFI] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-WIFI] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-WIFI] " __VA_ARGS__)


static ret_msg_t wifi_start_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t wifi_stop_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t wifi_set_para_cmd_proc(op_t op, arg_t *argv, int argn);

enum event
{
    STAT_INIT_EVENT = 0,
    STAT_EXEC_EVENT = 1
};
static char STAT_MACH = STAT_INIT_EVENT;

/**parameters define*/
const value_list_t round_list =
{
    .isrange = true,
    .min = 1,
    .max = 3
};

const value_list_t bssid_list =
{
    .isrange = true,
    .min = 4,
    .max = 10
};

value_t priority[2] =
{
    {
        .bufp = "0_PS",
        .size = 4,
    },
    {
        .bufp = "1_WIFI",
        .size = 6,
    }
};
const value_list_t priority_list =
{
    .isrange = false,
    .list_head = priority,
    .count = sizeof(priority) / sizeof(value_t),
};


arg_t wifi_args[4] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGWIFI_ROUND_NUM,
        // .value = &round_value,
        .value_range = &round_list,
        .msg = "from 1 to 3. default is 3"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGWIFI_BSSID_NUM,
        .value_range = &bssid_list,
        .msg = "from 4 to 10. default is 5"
    },
    {
        .type = TYPE_INT,
        .arg_name =  ARGWIFI_TIMEOUT,
        .msg = "uint:second. default is 25."
    },
    {
        .type = TYPE_STR,
        .arg_name =  ARGWIFI_PRIORITY,
        .value_range = &priority_list
    }
};

const menu_list_t func_wifi_start =
{
    .menu_name = "WIFI scan start",
    .menu_type = TYPE_FUNC,
    .parent = &wifi_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = wifi_start_cmd_proc,
    }
};

const menu_list_t func_wifi_para_set =
{
    .menu_name = "WIFI parameters",
    .menu_type = TYPE_FUNC,
    .parent = &wifi_menu,
    .function = {
        .argv = wifi_args,
        .argn = sizeof(wifi_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = wifi_set_para_cmd_proc,
    }
};

const menu_list_t func_wifi_stop =
{
    .menu_name = "WIFI scan stop",
    .menu_type = TYPE_FUNC,
    .parent = &wifi_menu,
    .function = {
        .methods.cmd_handler = wifi_stop_cmd_proc,
    }
};

const menu_list_t *wifi_menu_list[] =
{
    &func_wifi_start,
    &func_wifi_para_set,
    &func_wifi_stop,
    NULL
};

const menu_list_t wifi_menu =
{
    .menu_name = "WIFI scan",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = wifi_menu_list,
    .parent = &root_menu,
};

/**
  * @brief  Event handler for wifi scan.
  * @param  param WIFI scan parameters
  * @note   Will output scaned MAC\channel number\rssi.
  * @retval void
  */
static void wifi_handle_event(const SC_WIFI_EVENT_T *param)
{
    char rspBuf[128] = {0};
    arg_t out_cb_arg[1] = {0};

    switch (param->type)
    {
        case SC_WIFI_EVENT_SCAN_INFO:
            snprintf(rspBuf, sizeof(rspBuf),
                    "MAC address(%02x:%02x:%02x:%02x:%02x:%02x), channel(%d), rssi(%d).\n",
                    param->info.mac_addr[5], param->info.mac_addr[4], param->info.mac_addr[3],
                    param->info.mac_addr[2], param->info.mac_addr[1], param->info.mac_addr[0],
                    param->info.channel_number, param->info.rssi);
            break;
        case SC_WIFI_EVENT_SCAN_STOP:
            snprintf(rspBuf, sizeof(rspBuf), "wifi scan stop.\n");
            break;
        default:
            snprintf(rspBuf, sizeof(rspBuf), "invalid wifi event.\n");
    }

    out_cb_arg[0].arg_name = "Scan Result";
    out_cb_arg[0].type = TYPE_RAW;
    out_cb_arg[0].value = pl_demo_make_value(0, NULL, strlen(rspBuf));

    if (0 != pl_demo_post_urc(&func_wifi_start, TYPE_URC_DATA, out_cb_arg, 1, rspBuf, strlen(rspBuf)))
            pl_demo_post_comments("wifi CB post urc failed!!");
    LOG_TRACE("WIFI scan event: %d, satus:%d", param->type, STAT_MACH);
    if ((param->type != SC_WIFI_EVENT_SCAN_INFO) && (STAT_MACH == STAT_EXEC_EVENT))
    {
        LOG_TRACE("WIFI scan event: %d", param->type);
        STAT_MACH = STAT_INIT_EVENT;
    }

    pl_demo_release_value(out_cb_arg[0].value);
}


static ret_msg_t wifi_start_cmd_proc(op_t op, arg_t *argv, int argn)
{
    LOG_TRACE("wifi_start_cmd_proc enter!");
    ret_msg_t ret = {0};
    arg_t out_arg[1] = {0};
    char rspBuf[50];

    if (STAT_MACH == STAT_INIT_EVENT)
    {
        sAPI_WifiSetHandler(wifi_handle_event);
        sAPI_WifiScanStart(); //wifi_scan is retimed if re-executed

        STAT_MACH = STAT_EXEC_EVENT;
        snprintf(rspBuf, sizeof(rspBuf), "Please wait a moment, scanning...\n");
    }
    else
    {
        snprintf(rspBuf, sizeof(rspBuf), "wifi scan is scanning, do not repeat\n");
    }


    out_arg[0].arg_name = "WIFI Status";
    out_arg[0].type = TYPE_RAW;
    out_arg[0].value = pl_demo_make_value(0, NULL, strlen(rspBuf));
    if (0 != pl_demo_post_urc(&func_wifi_start, TYPE_URC_DATA, out_arg, 1, rspBuf, strlen(rspBuf)))
        pl_demo_post_comments("wifi start post urc failed!!");

    pl_demo_release_value(out_arg[0].value);

    return ret;
}

static ret_msg_t wifi_stop_cmd_proc(op_t op, arg_t *argv, int argn)
{
    LOG_TRACE("wifi_stop_cmd_proc enter!");
    ret_msg_t ret = {0};
    arg_t out_arg[1] = {0};
    char rspBuf[50];

    if (STAT_MACH == STAT_EXEC_EVENT)
    {
        LOG_TRACE("wifi scan stop");
        sAPI_WifiScanStop();

        snprintf(rspBuf, sizeof(rspBuf), "Please wait a moment, stopping...\n");
    }
    else
    {
        snprintf(rspBuf, sizeof(rspBuf), "wifi scan is stopped, do not repeat\n");
    }

    for (int i = 10*5; STAT_MACH != STAT_INIT_EVENT; i--)
    {
        sAPI_TaskSleep(20);
        if (i == 0 && STAT_MACH == STAT_EXEC_EVENT) STAT_MACH = STAT_INIT_EVENT;
    }

    out_arg[0].arg_name = "WIFI Status";
    out_arg[0].type = TYPE_RAW;
    out_arg[0].value = pl_demo_make_value(0, NULL, strlen(rspBuf));
    if (0 != pl_demo_post_urc(&func_wifi_stop, TYPE_URC_DATA, out_arg, 1, rspBuf, strlen(rspBuf)))
        pl_demo_post_comments("wifi stop post urc failed!!");

    pl_demo_release_value(out_arg[0].value);

    return ret;
}

static ret_msg_t wifi_set_para_cmd_proc(op_t op, arg_t *argv, int argn)
{
    LOG_TRACE("wifi_set_para_cmd_proc enter!");
    ret_msg_t ret = {0};
    int roundNum, bssidNum, timeout;
    const char *priority = NULL;
    SC_WIFI_SCANPARAM_T param = {0};

    pl_demo_get_value(&roundNum, argv, argn, ARGWIFI_ROUND_NUM);
    pl_demo_get_value(&bssidNum, argv, argn, ARGWIFI_BSSID_NUM);
    pl_demo_get_value(&timeout, argv, argn, ARGWIFI_TIMEOUT);
    pl_demo_get_data(&priority, argv, argn, ARGWIFI_PRIORITY);

#ifdef TTS_TEST_DEBUG
    int i = 0;
    for(i = 0; i < argn; i++) {
        LOG_INFO("param argv[%d].type=%d, argv[%d].value=%d", i, argv[i].type, i, argv[i].value);
    }
#endif

    if (round_list.min > roundNum || round_list.max < roundNum)
    {
        LOG_ERROR("param invalid! round number=%d", roundNum);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    else if (bssid_list.min > bssidNum || bssid_list.max < bssidNum)
    {
        LOG_ERROR("param invalid! bssid number=%d", bssidNum);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    else if (!priority || timeout <= 0)
    {
        LOG_ERROR("param invalid! priority=%d, timeout=%d", priority, timeout);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    else
    {
        param.round_number = roundNum;
        param.max_bssid_number = bssidNum;
        param.timeout = timeout;
        param.priority = CHAR_TO_INT(*priority);

        ret.errcode = ERRCODE_OK;
        ret.msg = "wifi param set success";
    }

    sAPI_WifiSetScanParam((const SC_WIFI_SCANPARAM_T *)&param);

    return ret;
}

