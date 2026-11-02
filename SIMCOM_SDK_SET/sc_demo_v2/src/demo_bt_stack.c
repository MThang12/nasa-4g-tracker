#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "simcom_bt_stack.h"
#include "simcom_os.h"
#include "demo_bt.h"
#include "ble_gatt_demo.h"

#define LOG_ERROR(...) sal_log_error("[DEMO-BTSTACK] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-BTSTACK] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-BTSTACK] " __VA_ARGS__)

static ret_msg_t bt_open_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_close_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_set_addr_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_get_addr_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_set_name_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_get_name_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_inquiry_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_inquiry_cancel_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_pair_request_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_pair_response_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_unpair_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_get_pairing_info_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_spp_connect_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_spp_disconnect_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_spp_send_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_rssi_read_cmd_proc(op_t op, arg_t *argv, int argn);

static ret_msg_t le_set_random_addr_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_set_adv_param_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_set_adv_data_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_set_scan_resp_data_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_set_ext_adv_param_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_set_ext_adv_data_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_set_ext_scan_resp_data_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_register_service_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_enable_adv_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_disable_adv_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_indicate_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_notify_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_connect_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_disconnect_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_mtu_request_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_att_scan_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_client_scan_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_client_scan_stop_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_client_read_request_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_client_write_request_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_client_write_command_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_pairing_request_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_get_pairing_info_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_remove_pairing_info_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_set_white_list_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t le_clear_white_list_cmd_proc(op_t op, arg_t *argv, int argn);

static int bt_get_data_size(void);
static ret_msg_t bt_recv_data_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms));

arg_t bt_open_para[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGBT_IOCAP,
        .msg = "Support DISPLAY_ONLY,DISPLAY_AND_YES_OR_NO,KEYBOARD_ONLY,NO_DISPALY_AND_NO_KEYBOARD",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGBT_BTMODE,
        .msg = "0:br,1:bt_le,2:le",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGBT_PAIR_MODE,
        .msg = "0:pin code,1:ssp(default to set)",
    }
};

const menu_list_t func_bt_open =
{
    .menu_name = "open bluetooth",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = bt_open_para,
        .argn = sizeof(bt_open_para)/sizeof(arg_t),
        .methods.cmd_handler = bt_open_cmd_proc,
    },
};

const menu_list_t func_bt_close =
{
    .menu_name = "close bluetooth",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = bt_close_cmd_proc,
    },
};

arg_t set_addr_para[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGBT_ADDR,
        .msg = "bt address,like XXXXXXXXXXXX",
    }
};

const menu_list_t func_bt_set_addr =
{
    .menu_name = "set bt address",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = set_addr_para,
        .argn = sizeof(set_addr_para)/sizeof(arg_t),
        .methods.cmd_handler = bt_set_addr_cmd_proc,
    },
};

const menu_list_t func_bt_get_addr =
{
    .menu_name = "get bt address",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = bt_get_addr_cmd_proc,
    },
};

arg_t set_name_para[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGBT_NAME,
        .msg = "bt device name",
    }
};

const menu_list_t func_bt_set_name =
{
    .menu_name = "set bt name",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = set_name_para,
        .argn = sizeof(set_name_para)/sizeof(arg_t),
        .methods.cmd_handler = bt_set_name_cmd_proc,
    },
};

const menu_list_t func_bt_get_name =
{
    .menu_name = "get bt name",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = bt_get_name_cmd_proc,
    },
};

arg_t inquiry_para[2] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGBT_SCAN_TIME,
        .msg = "range is (1-48)",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGBT_RESP_NUM,
        .msg = "range >= 0",
    }
};

const menu_list_t func_bt_inquiry =
{
    .menu_name = "inquiry bt device",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = inquiry_para,
        .argn = sizeof(inquiry_para) / sizeof(arg_t),
        .methods.cmd_handler = bt_inquiry_cmd_proc,
    },
};

const menu_list_t func_bt_inquiry_cancel =
{
    .menu_name = "cancel to inquiry",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = bt_inquiry_cancel_cmd_proc,
    },
};

arg_t pair_req_para[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGBT_ADDR,
        .msg = "bt address,like XXXXXXXXXXXX",
    },
};

const menu_list_t func_bt_pair_device =
{
    .menu_name = "bt pairing request",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = pair_req_para,
        .argn = sizeof(pair_req_para) / sizeof(arg_t),
        .methods.cmd_handler = bt_pair_request_cmd_proc,
    },
};

arg_t pair_resp_para[4] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGBT_PAIR_RESP,
        .msg = "bt accept/refuse pairing(1/0)",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGBT_PAIR_MODE,
        .msg = "bt pairing mode",
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGBT_ADDR,
        .msg = "bt address,like XXXXXXXXXXXX"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGBT_VERIFICATION,
        .msg = "bt passkey/pin",
    }
};

const menu_list_t func_bt_pair_response =
{
    .menu_name = "bt pairing response",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = pair_resp_para,
        .argn = sizeof(pair_resp_para) / sizeof(arg_t),
        .methods.cmd_handler = bt_pair_response_cmd_proc,
    },
};

arg_t unpair_para[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGBT_ADDR,
        .msg = "bt address,like XXXXXXXXXXXX",
    }
};

const menu_list_t func_bt_unpair_device =
{
    .menu_name = "bt unpair",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = unpair_para,
        .argn = sizeof(unpair_para) / sizeof(arg_t),
        .methods.cmd_handler = bt_unpair_cmd_proc,
    },
};

const menu_list_t func_bt_get_pairing_info =
{
    .menu_name = "bt get pairing info",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = bt_get_pairing_info_cmd_proc,
    },
};

arg_t spp_connect_para[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGBT_ADDR,
        .msg = "bt address,like XXXXXXXXXXXX",
    }
};

const menu_list_t func_bt_spp_connect =
{
    .menu_name = "bt SPP connect",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = spp_connect_para,
        .argn = sizeof(spp_connect_para)/sizeof(arg_t),
        .methods.cmd_handler = bt_spp_connect_cmd_proc,
    },
};

arg_t spp_disconnect_para[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGBT_SPP_PORT,
        .msg = "spp port from connection",
    }
};

const menu_list_t func_bt_spp_disconnect =
{
    .menu_name = "bt SPP connect",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = spp_disconnect_para,
        .argn = sizeof(spp_disconnect_para)/sizeof(arg_t),
        .methods.cmd_handler = bt_spp_disconnect_cmd_proc,
    },
};

arg_t spp_send_para[2] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGBT_SPP_DATA_LEN,
        .msg = "data length",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGBT_SPP_PORT,
        .msg = "spp port from connection",
    }
};

const menu_list_t func_bt_spp_send_data =
{
    .menu_name = "bt SPP send data",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = spp_send_para,
        .argn = sizeof(spp_send_para)/sizeof(arg_t),
        .methods.cmd_handler = bt_spp_send_cmd_proc,
        .methods.raw_handler = bt_recv_data_proc,
        .methods.needRAWData = bt_get_data_size,
    },
};

const menu_list_t func_bt_get_rssi =
{
    .menu_name = "bt read rssi",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = bt_rssi_read_cmd_proc,
    },
};

arg_t le_set_random_addr_para[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGBT_ADDR,
        .msg = "le random address,like XXXXXXXXXXXX",
    }
};

const menu_list_t func_le_set_random_addr =
{
    .menu_name = "le set random addr",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = le_set_random_addr_para,
        .argn = sizeof(le_set_random_addr_para)/sizeof(arg_t),
        .methods.cmd_handler = le_set_random_addr_cmd_proc,
    },
};

arg_t le_adv_param[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_ADV_INTERVAL_MIN,
        .msg = "interval_min"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_ADV_INTERVAL_MAX,
        .msg = "interval_max"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_ADV_TYPE,
        .msg = "LE_ADV_TYPE_XXX",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_ADDR_TYPE,
        .msg = "own address type",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_ADDR_TYPE,
        .msg = "peer address type",
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGBT_ADDR,
        .msg = "peer address",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_ADV_FILTER,
        .msg = "filter",
    }
};

const menu_list_t func_le_set_adv_param =
{
    .menu_name = "set adv param",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = le_adv_param,
        .argn = sizeof(le_adv_param) / sizeof(arg_t),
        .methods.cmd_handler = le_set_adv_param_cmd_proc,
    },
};

arg_t le_set_adv_data_para[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGLE_ADV_DATA,
        .msg = "hex string",
    }
};

const menu_list_t func_le_set_adv_data =
{
    .menu_name = "set adv data",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = le_set_adv_data_para,
        .argn = sizeof(le_set_adv_data_para) / sizeof(arg_t),
        .methods.cmd_handler = le_set_adv_data_cmd_proc,
    },
};

arg_t le_set_scan_resp_para[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGLE_SCAN_RESP,
        .msg = "hex string",
    }
};

const menu_list_t func_le_set_scan_resp_data =
{
    .menu_name = "set scan_resp data",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = le_set_scan_resp_para,
        .argn = sizeof(le_set_scan_resp_para) / sizeof(arg_t),
        .methods.cmd_handler = le_set_scan_resp_data_cmd_proc,
    },
};

arg_t le_ext_adv_param[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_PROPERTIES,
        .msg = "properties"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_ADV_INTERVAL_MIN,
        .msg = "interval_min"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_ADV_INTERVAL_MAX,
        .msg = "interval_max"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_CHANNEL,
        .msg = "adv channel",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_ADDR_TYPE,
        .msg = "own address type",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_TX_POWER,
        .msg = "tx_power",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_PRIMARY_PHY,
        .msg = "primary_phy",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_SECONDARY_PHY,
        .msg = "secondary_phy",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_ADV_MODE,
        .msg = "adv mode",
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGBT_ADDR,
        .msg = "random address",
    }
};

const menu_list_t func_le_set_ext_adv_param =
{
    .menu_name = "set externed adv param",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = le_ext_adv_param,
        .argn = sizeof(le_ext_adv_param) / sizeof(arg_t),
        .methods.cmd_handler = le_set_ext_adv_param_cmd_proc,
    },
};

arg_t le_set_ext_adv_data_para[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGLE_ADV_DATA,
        .msg = "hex string",
    }
};

const menu_list_t func_le_set_ext_adv_data =
{
    .menu_name = "set externed adv data",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = le_set_ext_adv_data_para,
        .argn = sizeof(le_set_ext_adv_data_para) / sizeof(arg_t),
        .methods.cmd_handler = le_set_ext_adv_data_cmd_proc,
    },
};

arg_t le_set_ext_scan_resp_para[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGLE_SCAN_RESP,
        .msg = "hex string",
    }
};

const menu_list_t func_le_set_ext_scan_resp_data =
{
    .menu_name = "set externed scan_resp data",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = le_set_ext_scan_resp_para,
        .argn = sizeof(le_set_ext_scan_resp_para) / sizeof(arg_t),
        .methods.cmd_handler = le_set_ext_scan_resp_data_cmd_proc,
    },
};

const menu_list_t func_le_register_service =
{
    .menu_name = "le register service",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = le_register_service_cmd_proc,
    },
};

arg_t adv_type_para[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_ADV_TYPE,
        .msg = "0 is legacy adv, 1 is externed adv",
    }
};

const menu_list_t func_le_enable_adv =
{
    .menu_name = "enable adv",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = adv_type_para,
        .argn = sizeof(adv_type_para)/sizeof(arg_t),
        .methods.cmd_handler = le_enable_adv_cmd_proc,
    },
};

const menu_list_t func_le_disable_adv =
{
    .menu_name = "disable adv",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = adv_type_para,
        .argn = sizeof(adv_type_para)/sizeof(arg_t),
        .methods.cmd_handler = le_disable_adv_cmd_proc,
    },
};

arg_t indicate_para[3] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_CONN_HANDLE,
        .msg = "conn handle",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_ATT_HANDLE,
        .msg = "att handle",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_DATA_LEN,
        .msg = "data size",
    }
};

const menu_list_t func_le_indicate =
{
    .menu_name = "ble indicate",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = indicate_para,
        .argn = sizeof(indicate_para)/sizeof(arg_t),
        .methods.cmd_handler = le_indicate_cmd_proc,
        .methods.raw_handler = bt_recv_data_proc,
        .methods.needRAWData = bt_get_data_size,
    },
};

const menu_list_t func_le_notify =
{
    .menu_name = "ble notify",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = indicate_para,
        .argn = sizeof(indicate_para)/sizeof(arg_t),
        .methods.cmd_handler = le_notify_cmd_proc,
        .methods.raw_handler = bt_recv_data_proc,
        .methods.needRAWData = bt_get_data_size,
    },
};

arg_t connect_para[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGBT_ADDR,
        .msg = "bt address",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGBT_ADDR_TYPE,
        .msg = "peer addr type",
    }
};

const menu_list_t func_le_connect =
{
    .menu_name = "ble connect",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = connect_para,
        .argn = sizeof(connect_para)/sizeof(arg_t),
        .methods.cmd_handler = le_connect_cmd_proc,
    },
};

arg_t disconnect_para[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_CONN_HANDLE,
        .msg = "conn handle",
    }
};

const menu_list_t func_le_disconnect =
{
    .menu_name = "ble disconnect",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = disconnect_para,
        .argn = sizeof(disconnect_para)/sizeof(arg_t),
        .methods.cmd_handler = le_disconnect_cmd_proc,
    },
};

arg_t mtu_req_para[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_CONN_HANDLE,
        .msg = "conn handle",
    },

    {
        .type = TYPE_INT,
        .arg_name = ARGLE_MTU_SIZE,
        .msg = "mtu size",
    }
};

const menu_list_t func_le_mtu_request =
{
    .menu_name = "ble mtu request",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = mtu_req_para,
        .argn = sizeof(mtu_req_para)/sizeof(arg_t),
        .methods.cmd_handler = le_mtu_request_cmd_proc,
    },
};

arg_t att_scan_para[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_CONN_HANDLE,
        .msg = "conn handle",
    }
};

const menu_list_t func_le_att_scan =
{
    .menu_name = "ble mtu request",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = att_scan_para,
        .argn = sizeof(att_scan_para)/sizeof(arg_t),
        .methods.cmd_handler = le_att_scan_cmd_proc,
    },
};

arg_t client_scan_para[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_SCAN_MODE,
        .msg = "scan_mode",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_SCAN_INTERVAL,
        .msg = "interval",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_SCAN_WINDOW,
        .msg = "window",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGBT_ADDR_TYPE,
        .msg = "own addr type",
    }
};

const menu_list_t func_le_client_scan =
{
    .menu_name = "ble client scan",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = client_scan_para,
        .argn = sizeof(client_scan_para)/sizeof(arg_t),
        .methods.cmd_handler = le_client_scan_cmd_proc,
    },
};

const menu_list_t func_le_client_scan_stop =
{
    .menu_name = "ble client stop to scan",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = le_client_scan_stop_cmd_proc,
    },
};

arg_t client_read_req_para[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_CONN_HANDLE,
        .msg = "conn handle",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_ATT_HANDLE,
        .msg = "att handle",
    },
};

const menu_list_t func_le_client_read_requset =
{
    .menu_name = "ble client read request",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = client_read_req_para,
        .argn = sizeof(client_read_req_para)/sizeof(arg_t),
        .methods.cmd_handler = le_client_read_request_cmd_proc,
    },
};

arg_t client_write_req_para[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_CONN_HANDLE,
        .msg = "conn handle",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_ATT_HANDLE,
        .msg = "att handle",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_DATA_LEN,
        .msg = "data size",
    }
};

const menu_list_t func_le_client_write_requset =
{
    .menu_name = "ble client write request",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = client_write_req_para,
        .argn = sizeof(client_write_req_para)/sizeof(arg_t),
        .methods.cmd_handler = le_client_write_request_cmd_proc,
        .methods.raw_handler = bt_recv_data_proc,
        .methods.needRAWData = bt_get_data_size,
    },
};

const menu_list_t func_le_client_write_command =
{
    .menu_name = "ble client write command",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = client_write_req_para,
        .argn = sizeof(client_write_req_para)/sizeof(arg_t),
        .methods.cmd_handler = le_client_write_command_cmd_proc,
        .methods.raw_handler = bt_recv_data_proc,
        .methods.needRAWData = bt_get_data_size,
    },
};

arg_t le_pairing_enable_para[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_PAIR_ENABLE,
        .msg = "enable/disable pairing(default enable)",
    }
};

const menu_list_t func_le_pairing_enable =
{
    .menu_name = "ble set to enable pairing",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = le_pairing_enable_para,
        .argn = sizeof(le_pairing_enable_para)/sizeof(arg_t),
        .methods.cmd_handler = le_pairing_request_cmd_proc,
    },
};

arg_t le_pairing_para[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_CONN_HANDLE,
        .msg = "conn handle",
    }
};

const menu_list_t func_le_pairing_request =
{
    .menu_name = "ble pair to device",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = le_pairing_para,
        .argn = sizeof(le_pairing_para)/sizeof(arg_t),
        .methods.cmd_handler = le_pairing_request_cmd_proc,
    }
};

arg_t le_get_pairing_para[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_PAIR_INDEX,
        .msg = "record index",
    }
};

const menu_list_t func_le_get_pairing_info =
{
    .menu_name = "ble get pairing information",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = le_get_pairing_para,
        .argn = sizeof(le_get_pairing_para)/sizeof(arg_t),
        .methods.cmd_handler = le_get_pairing_info_cmd_proc,
    },
};

const menu_list_t func_le_remove_pairing_info =
{
    .menu_name = "ble remove pairing information",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = le_get_pairing_para,
        .argn = sizeof(le_get_pairing_para)/sizeof(arg_t),
        .methods.cmd_handler = le_remove_pairing_info_cmd_proc,
    },
};

const menu_list_t func_le_set_white_list =
{
    .menu_name = "ble set white list",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = connect_para,
        .argn = sizeof(connect_para)/sizeof(arg_t),
        .methods.cmd_handler = le_set_white_list_cmd_proc,
    },
};

const menu_list_t func_le_clear_white_list =
{
    .menu_name = "ble clear white list",
    .menu_type = TYPE_FUNC,
    .parent = &bt_stack_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = le_clear_white_list_cmd_proc,
    },
};

const menu_list_t *bt_menu_sub[] =
{
    &func_bt_open,
    &func_bt_close,
    &func_bt_set_addr,
    &func_bt_get_addr,
    &func_bt_set_name,
    &func_bt_get_name,
    &func_bt_inquiry,
    &func_bt_inquiry_cancel,
    &func_bt_pair_device,
    &func_bt_pair_response,
    &func_bt_unpair_device,
    &func_bt_get_pairing_info,
    &func_bt_spp_connect,
    &func_bt_spp_disconnect,
    &func_bt_spp_send_data,
    &func_bt_get_rssi,

    &func_le_set_random_addr,
    &func_le_set_adv_param,
    &func_le_set_adv_data,
    &func_le_set_scan_resp_data,
    &func_le_set_ext_adv_param,
    &func_le_set_ext_adv_data,
    &func_le_set_ext_scan_resp_data,
    &func_le_register_service,
    &func_le_enable_adv,
    &func_le_disable_adv,
    &func_le_indicate,
    &func_le_notify,
    &func_le_connect,
    &func_le_disconnect,
    &func_le_mtu_request,
    &func_le_att_scan,
    &func_le_client_scan,
    &func_le_client_scan_stop,
    &func_le_client_read_requset,
    &func_le_client_write_requset,
    &func_le_client_write_command,
    &func_le_pairing_enable,
    &func_le_pairing_request,
    &func_le_get_pairing_info,
    &func_le_remove_pairing_info,
    &func_le_set_white_list,
    &func_le_clear_white_list,
    NULL  // MUST end by NULL
};

const menu_list_t bt_stack_menu =
{
    .menu_name = "BT/BLE",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = bt_menu_sub,
    .parent = &root_menu,
};

static const SC_BT_PAIR_MODE_T g_bt_pairing_mode[5][5] =
{
    {SC_BT_PAIR_MODE_JUST_WORK_MODE,    SC_BT_PAIR_MODE_JUST_WORK_MODE,          SC_BT_PAIR_MODE_PASSKEY_ENTRY_MODE, SC_BT_PAIR_MODE_JUST_WORK_MODE, SC_BT_PAIR_MODE_PASSKEY_ENTRY_MODE},
    {SC_BT_PAIR_MODE_JUST_WORK_MODE,    SC_BT_PAIR_MODE_NUMERIC_COMPARISON_MODE, SC_BT_PAIR_MODE_PASSKEY_ENTRY_MODE, SC_BT_PAIR_MODE_JUST_WORK_MODE, SC_BT_PAIR_MODE_NUMERIC_COMPARISON_MODE},
    {SC_BT_PAIR_MODE_PASSKEY_ENTRY_MODE,SC_BT_PAIR_MODE_PASSKEY_ENTRY_MODE,      SC_BT_PAIR_MODE_PASSKEY_ENTRY_MODE, SC_BT_PAIR_MODE_JUST_WORK_MODE, SC_BT_PAIR_MODE_PASSKEY_ENTRY_MODE},
    {SC_BT_PAIR_MODE_JUST_WORK_MODE,    SC_BT_PAIR_MODE_JUST_WORK_MODE,          SC_BT_PAIR_MODE_JUST_WORK_MODE,     SC_BT_PAIR_MODE_JUST_WORK_MODE, SC_BT_PAIR_MODE_JUST_WORK_MODE},
    {SC_BT_PAIR_MODE_PASSKEY_ENTRY_MODE,SC_BT_PAIR_MODE_NUMERIC_COMPARISON_MODE, SC_BT_PAIR_MODE_PASSKEY_ENTRY_MODE, SC_BT_PAIR_MODE_JUST_WORK_MODE, SC_BT_PAIR_MODE_NUMERIC_COMPARISON_MODE},
};

unsigned short custom_value_client_configuration1 = 0;
static unsigned char scan_num = 0;

static int g_spp_port = 0;
static int g_data_size = 0;
static int g_conn_handle = 0,g_att_handle = 0;
static int g_data_type = 0;//0:indicate,1:notify,2:write command,3 write reqeust,4:spp send


static int bt_get_data_size(void)
{
    return g_data_size;
}
static ret_msg_t bt_recv_data_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms))
{
    ret_msg_t ret = {0};
    int read_size = 0;
    int process_size = 0;
    int remain_size = size;
    char buf[SAL_256] = {0};
    int result = 0;

    if (g_data_size != size)
    {
        LOG_ERROR("raw data size invalid");
        LOG_ERROR("size:%d, need:%d", size, g_data_size);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "raw data size invalid";
        return ret;
    }

    while (remain_size > 0)
    {
        read_size = remain_size <= SAL_128 ? remain_size : SAL_128;
        read_size = raw_read(buf, read_size, 10000);
        if (read_size < 0)
        {
            LOG_ERROR("raw data read fail");
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "raw data read fail";
            goto EXIT;
        }
        else if (0 == read_size)
        {
            LOG_ERROR("raw data read timeout");
            ret.errcode = ERRCODE_RAW_RECV_TIMEOUT;
            ret.msg = "raw data read timeout";
            goto EXIT;
        }

        if(g_data_type == 0)
            result = sAPI_BleIndicate(g_conn_handle, g_att_handle, (unsigned char *)buf, read_size);
        else if( g_data_type == 1)
            result = sAPI_BleNotify(g_conn_handle, g_att_handle, (unsigned char *)buf, read_size);
        else if(g_data_type == 2)
            result = sAPI_BleWriteCommand(g_conn_handle, g_att_handle, (unsigned char *)buf, read_size);
        else if(g_data_type == 3)
            result = sAPI_BleWriteRequest(g_conn_handle, g_att_handle, (unsigned char *)buf, read_size);
        else if(g_data_type == 4)
            result = sAPI_BtSppSend((unsigned char *)buf, read_size, g_spp_port);
        if (result < 0)
        {
            LOG_ERROR("raw data cache fail");
            LOG_ERROR("read size:%d, cache size:%d", read_size, process_size);
            ret.errcode = process_size < 0 ? process_size : ERRCODE_DEFAULT;
            ret.msg = "raw data cache fail";
            goto EXIT;
        }

        remain_size -= read_size;
    }

EXIT:
    /* wait file write to fs. */

    return ret;
}

uint32_t ble_demo_min(uint32_t a, uint32_t b)
{
    return (a < b) ? a : b;
}

uint16_t ble_demo_little_endian_read_16(const uint8_t *buffer, int position)
{
    return (uint16_t)(((uint16_t) buffer[position]) | (((uint16_t)buffer[position + 1]) << 8));
}

uint32_t ble_demo_little_endian_read_32(const uint8_t * buffer, int position){
    return ((uint32_t) buffer[position]) | (((uint32_t)buffer[position+1]) << 8) | (((uint32_t)buffer[position+2]) << 16) | (((uint32_t) buffer[position+3]) << 24);
}

// att_read_callback helpers
uint16_t ble_demo_att_read_callback_handle_blob(const uint8_t *blob, uint16_t blob_size, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    if (buffer != NULL) {
        uint16_t bytes_to_copy = 0;
        if (blob_size >= offset) {
            bytes_to_copy = ble_demo_min(blob_size - offset, buffer_size);
            (void)memcpy(buffer, &blob[offset], bytes_to_copy);
        }
        return bytes_to_copy;
    } else {
        return blob_size;
    }
}

char *ble_demo_uuid128_to_str(const uint8_t *uuid)
{
    static char appbt_uuid128_to_str_buffer[32 + 4 + 1];
    int i;
    int j = 0;
    // after 4, 6, 8, and 10 bytes = XYXYXYXY-XYXY-XYXY-XYXY-XYXYXYXYXYXY, there's a dash
    const int dash_locations = (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9);
    for (i = 0; i < 16; i++)
    {
        snprintf(appbt_uuid128_to_str_buffer + i * 2 + j, 4, "%02x", uuid[i]);
        if (dash_locations & (1 << i))
        {
            snprintf(appbt_uuid128_to_str_buffer + i * 2 + 2 + j, 4, "-");
            j++;
        }
    }
    return appbt_uuid128_to_str_buffer;
}

char *ble_demo_print_properties(uint16_t properties)
{
    static char properties_log[100] = {0};
    int offset = 0;
    if ((properties & SC_BLE_ATT_CHARC_PROP_READ) == SC_BLE_ATT_CHARC_PROP_READ)
    {
        snprintf(properties_log + offset, strlen("read | ") + 1, "read | ");
        offset += strlen("read | ");
    }
    if ((properties & SC_BLE_ATT_CHARC_PROP_WWP) == SC_BLE_ATT_CHARC_PROP_WWP)
    {
        snprintf(properties_log + offset, strlen("write_without_response | ") + 1, "write_without_response | ");
        offset += strlen("write_without_response | ");
    }
    if ((properties & SC_BLE_ATT_CHARC_PROP_WRITE) == SC_BLE_ATT_CHARC_PROP_WRITE)
    {
        snprintf(properties_log + offset, strlen("write | ") + 1, "write | ");
        offset += strlen("write | ");
    }
    if ((properties & SC_BLE_ATT_CHARC_PROP_NOTIFY) == SC_BLE_ATT_CHARC_PROP_NOTIFY)
    {
        snprintf(properties_log + offset, strlen("notify | ") + 1, "notify | ");
        offset += strlen("notify | ");
    }
    if ((properties & SC_BLE_ATT_CHARC_PROP_INDICATE) == SC_BLE_ATT_CHARC_PROP_INDICATE)
    {
        snprintf(properties_log + offset, strlen("indicate | ") + 1, "indicate | ");
        offset += strlen("indicate | ");
    }
    snprintf(properties_log + offset - 3, 2, "~");
    return properties_log;
}

static unsigned short sc_demo_att_read_callback(
                                            unsigned short connection_handle,
                                            unsigned short att_handle,
                                            unsigned short offset,
                                            unsigned char *buffer,
                                            unsigned short buffer_size)
{

    LOG_INFO("[DEMO]connection_handle: %d, att_handle: 0x%04x, offset: %d, buffer: 0x%x, buffer_size: %d", connection_handle, att_handle, offset, buffer, buffer_size);

    if (buffer == NULL)
    {
        LOG_INFO("[DEMO][%s] used for check value len");
    }
    char buff[10] = "SIMCOM";
    return ble_demo_att_read_callback_handle_blob((const uint8_t *)buff, sizeof(buff), offset, buffer, buffer_size);
    //return 0;
}

static int sc_demo_att_write_callback(
                                   unsigned short connection_handle,
                                   unsigned short att_handle,
                                   unsigned short transaction_mode,
                                   unsigned short offset,
                                   unsigned char *buffer,
                                   unsigned short buffer_size)
{

    LOG_INFO("[DEMO]connection_handle: %d, att_handle: 0x%04x, offset: %d, buffer_size: %d", connection_handle, att_handle, offset, buffer_size);
    custom_value_client_configuration1 = ble_demo_little_endian_read_16(buffer, 0);
    LOG_INFO("[DEMO]custom_value_client_configuration1 %d", custom_value_client_configuration1);
    return 0;
}

void demo_bt_send_buf(char *buffer,int len)
{
    
    arg_t out_arg[1] = {0};
    out_arg[0].arg_name = ARGBT_RESULT;
    out_arg[0].type = TYPE_STR;
    out_arg[0].value = pl_demo_make_value(0, buffer, len);
    pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);
}

static void bt_event_handle_cb(void *argv)
{
    char buffer[512] = {0};
    static SC_BT_ADDR_T temp_bt_address = {0};
    SC_BT_ADDR_T temp_bt_address_full_zero = {0};
    SC_BT_TASK_EVENT_T *msg = (SC_BT_TASK_EVENT_T *)argv;

    LOG_INFO("event_type:%d event_id:%d", msg->event_type, msg->event_id);
    switch(msg->event_type)
    {
        case SC_BT_EVENT_TYPE_COMMON:
        {
            switch(msg->event_id)
            {
                case SC_COMMON_INQUIRY_RESULT:
                {
                    SC_BT_EVENT_INQUIRY_T *inquiry = (SC_BT_EVENT_INQUIRY_T *)msg->payload;
                    sprintf(buffer,"[COMMON_INQUIRY_RESULT] name[%s] address(%02X:%02X:%02X:%02X:%02X:%02X) RSSI:%d",
                            inquiry->name,
                            inquiry->addr[5],inquiry->addr[4],inquiry->addr[3],
                            inquiry->addr[2],inquiry->addr[1],inquiry->addr[0],
                            inquiry->rssi);
                    temp_bt_address.bytes[0] = inquiry->addr[5];
                    temp_bt_address.bytes[1] = inquiry->addr[4];
                    temp_bt_address.bytes[2] = inquiry->addr[3];
                    temp_bt_address.bytes[3] = inquiry->addr[2];
                    temp_bt_address.bytes[4] = inquiry->addr[1];
                    temp_bt_address.bytes[5] = inquiry->addr[0];
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
                case SC_COMMON_INQUIRY_COMPLETE:
                {
                    sprintf(buffer,"[COMMON_INQUIRY_COMPLETE]");
                    if(memcmp(temp_bt_address.bytes, temp_bt_address_full_zero.bytes, 6) != 0)
                    {
                        memset(temp_bt_address.bytes, 0, 6);
                    }
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
                case SC_COMMON_PAIRING_REQUEST:
                {
                    SC_BT_EVENT_PAIRING_REQUEST_T *pairing_request = (SC_BT_EVENT_PAIRING_REQUEST_T *)msg->payload;
                    SC_BT_IOCAP_T local_io_capability = 0;
                    SC_BT_IOCAP_T peer_io_capability = 0;
                    SC_BT_PAIR_MODE_T pair_mode;

                    sAPI_BtGetIoCap(&local_io_capability);
                    peer_io_capability = pairing_request->io_capability;
                    pair_mode = g_bt_pairing_mode[local_io_capability][peer_io_capability];
                    sprintf(buffer,"[COMMON_PAIRING_REQUEST] pairing request mode(%d)",pair_mode);
                    LOG_INFO("%s",buffer);

                    if(pair_mode == SC_BT_PAIR_MODE_JUST_WORK_MODE)
                    {
                        char result[100] = "[COMMON_PAIRING_REQUEST] SC_BT_PAIR_MODE_JUST_WORK_MODE";
                        sAPI_TaskSleep(500 / 5);//avoid to fast
                        sAPI_BtPairAccept(SC_BT_PAIR_MODE_JUST_WORK_MODE, pairing_request->addr, 0, NULL);
                        demo_bt_send_buf(result, strlen(result));
                    }
                    else if(pair_mode == SC_BT_PAIR_MODE_NUMERIC_COMPARISON_MODE)
                    {
                        char result[100] = "[COMMON_PAIRING_REQUEST] SC_BT_PAIR_MODE_NUMERIC_COMPARISON_MODE accpetted";
                        sAPI_TaskSleep(500 / 5);//avoid to fast
                        sAPI_BtPairAccept(SC_BT_PAIR_MODE_NUMERIC_COMPARISON_MODE, pairing_request->addr, 0, NULL);
                        demo_bt_send_buf(result, strlen(result));
                    }
                    else
                    {
                        demo_bt_send_buf(buffer, strlen(buffer));
                    }
                    break;
                }
                case SC_COMMON_PIN_REQUEST:
                {
                    char *pin = "1234";
                    int ret;
                    SC_BT_ADDR_T addr = {0};
                    SC_BT_EVENT_PIN_REQUEST_T *request = (SC_BT_EVENT_PIN_REQUEST_T *)msg->payload;
                    sprintf(buffer,"[COMMON_PIN_REQUEST]");
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    sAPI_TaskSleep(500 / 5);//avoid to fast
                    addr.bytes[5] = request->addr.bytes[0];
                    addr.bytes[4] = request->addr.bytes[1];
                    addr.bytes[3] = request->addr.bytes[2];
                    addr.bytes[2] = request->addr.bytes[3];
                    addr.bytes[1] = request->addr.bytes[4];
                    addr.bytes[0] = request->addr.bytes[5];
                    ret = sAPI_BtPairAccept(SC_BT_PAIR_MODE_PIN_CODE, addr, 0, pin);
                    LOG_INFO("ret is %d",ret);
                    break;
                }
                case SC_COMMON_USER_PASSKEY_NOTIFICATION:
                {
                    SC_BT_EVENT_USER_PASSKEY_NOTIFICATION_T *event = (SC_BT_EVENT_USER_PASSKEY_NOTIFICATION_T *)msg->payload;
                    sprintf(buffer,"[COMMON_USER_PASSKEY_NOTIFICATION] address(%02X:%02X:%02X:%02X:%02X:%02X) passkey:%d",
                            event->addr.bytes[5],event->addr.bytes[4],event->addr.bytes[3],
                            event->addr.bytes[2],event->addr.bytes[1],event->addr.bytes[0],
                            event->passkey);
                    sAPI_TaskSleep(500 / 5);//avoid to fast
                    sAPI_BtPairAccept(SC_BT_PAIR_MODE_JUST_WORK_MODE, event->addr, 1 , NULL);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
                case SC_COMMON_USER_PASSKEY_REQUEST:
                {
                    SC_BT_EVENT_USER_PASSKEY_REQUEST_T *event = (SC_BT_EVENT_USER_PASSKEY_REQUEST_T *)msg->payload;
                    sprintf(buffer,"[COMMON_USER_PASSKEY_REQUEST] request passky(%02X:%02X:%02X:%02X:%02X:%02X)",
                            event->addr.bytes[5],event->addr.bytes[4],event->addr.bytes[3],
                            event->addr.bytes[2],event->addr.bytes[1],event->addr.bytes[0]);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
                case SC_COMMON_POWERUP_COMPLETE:
                {
                    sprintf(buffer,"[COMMON_POWERUP_COMPLETE]");
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
                case SC_COMMON_POWERUP_FAILED:
                {
                    sprintf(buffer,"[COMMON_POWERUP_FAILED]");
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
                case SC_COMMON_PAIRED:
                {
                    SC_BT_EVENT_PAIRED_T *paired = (SC_BT_EVENT_PAIRED_T *)msg->payload;
                    sprintf(buffer,"[COMMON_PAIRED] paired with name[%s] address(%02X:%02X:%02X:%02X:%02X:%02X)",
                        paired->name,
                        paired->addr[5],paired->addr[4],paired->addr[3],
                        paired->addr[2],paired->addr[1],paired->addr[0]);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
                case SC_COMMON_SHUTDOWN_COMPLETE:
                {
                    sprintf(buffer,"[COMMON_SHUTDOWN_COMPLETE]");
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
                case SC_COMMON_RSSI:
                {
                    signed char *value_rssi = (signed char *)msg->payload;
                    sprintf(buffer,"[COMMON_RSSI] Read RSSI(%d)",*value_rssi);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
                default:
                {
                    sprintf(buffer,"[SC_BT_EVENT_TYPE_COMMON_%d]",msg->event_id);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
            }
        }
        break;

        case SC_BT_EVENT_TYPE_SPP:
        {
            switch(msg->event_id)
            {
                case SC_SPP_CONNECT_IND:
                {
                    SC_BT_EVENT_SPP_EVENT_T *connect = (SC_BT_EVENT_SPP_EVENT_T *)msg->payload;
                    if(connect->result == 0)
                    {
                        sprintf(buffer,"[SPP_CONNECT_IND] SPP connect success, address(%02X:%02X:%02X:%02X:%02X:%02X),spp_port(%d) spp_max_frame_size(%d)",
                                    connect->addr[5],connect->addr[4],connect->addr[3],
                                    connect->addr[2],connect->addr[1],connect->addr[0],
                                    connect->port, connect->max_frame_size);
                    }
                    else
                    {
                        sprintf(buffer,"[SPP_CONNECT_IND] SPP connect result= %d", connect->result);
                    }
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
                case SC_SPP_CONNECT_CNF:
                {
                    SC_BT_EVENT_SPP_EVENT_T *connect = (SC_BT_EVENT_SPP_EVENT_T *)msg->payload;
                    if(connect->result == 0)
                    {
                        sprintf(buffer,"[SPP_CONNECT_CNF] SPP connect success, address(%02X:%02X:%02X:%02X:%02X:%02X),spp_port(%d) spp_max_frame_size(%d)",
                                    connect->addr[5],connect->addr[4],connect->addr[3],
                                    connect->addr[2],connect->addr[1],connect->addr[0],
                                    connect->port, connect->max_frame_size);
                    }
                    else
                    {
                        sprintf(buffer,"[SPP_CONNECT_CNF] SPP connect result= %d",connect->result);
                    }
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
                case SC_SPP_DISCONNECT_IND:
                {
                    SC_BT_EVENT_SPP_EVENT_T *connect = (SC_BT_EVENT_SPP_EVENT_T *)msg->payload;
                    sprintf(buffer,"[SPP_DISCONNECT_IND] SPP disconnect IND result= %d",connect->result);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
                case SC_SPP_DISCONNECT_CNF:
                {
                    SC_BT_EVENT_SPP_EVENT_T *connect = (SC_BT_EVENT_SPP_EVENT_T *)msg->payload;
                    sprintf(buffer,"[SPP_DISCONNECT_CNF] SPP disconnect CNF result= %d",connect->result);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
                case SC_SPP_DATA_IND:
                {
                    SC_BT_EVENT_SPP_EVENT_T *connect = (SC_BT_EVENT_SPP_EVENT_T *)msg->payload;
                    char urc[400] = {0};
                    snprintf(urc, connect->data_len + 1, "%s", connect->data);
                    sprintf(buffer,"[SPP_DATA_IND] SPP recieve data result= %d,length[%d] data[%s]",connect->result,connect->data_len,urc);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
                case SC_SPP_DATA_CNF:
                {
                    SC_BT_EVENT_SPP_EVENT_T *connect = (SC_BT_EVENT_SPP_EVENT_T *)msg->payload;
                    sprintf(buffer,"[SPP_DATA_CNF] SPP send data result= %d",connect->result);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
                default:
                {
                    sprintf(buffer,"[SC_BT_EVENT_TYPE_SPP_%d]",msg->event_id);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
            }
        }
        break;
        case SC_BT_EVENT_TYPE_LE:
        {
            switch(msg->event_id)
            {
                case SC_BLE_SCAN_EVENT:
                {
                    SC_BLE_EVENT_SCAN_EVENT_T *scan = (SC_BLE_EVENT_SCAN_EVENT_T *)(msg->payload);
                    int i = 0,j = 0;
                    char advData[63] = {0};

                    for(i = 0; i < scan->length; i ++)
                    {
                        j += sprintf(advData + j, "%02x",scan->data[i]);
                    }
                    if(scan->event_type != 0x04)
                    {
                        scan_num ++;
                        sprintf(buffer,"[LE_SCAN_EVENT] remote device addr(%02x:%02x:%02x:%02x:%02x:%02x),addrType(%d),advData[%s],rssi(%d)",
                                        scan->address.bytes[5],scan->address.bytes[4],scan->address.bytes[3],
                                        scan->address.bytes[2],scan->address.bytes[1],scan->address.bytes[0],
                                        scan->address_type, advData, scan->rssi);
                        LOG_INFO("%s",buffer);
                        demo_bt_send_buf(buffer, strlen(buffer));
                        if(scan_num >= 60)
                        {
                            sAPI_BleScanStop();
                            scan_num = 0;
                        }
                    }
                    else
                    {
                        sprintf(buffer,"[LE_SCAN_EVENT] remote device addr(%02x:%02x:%02x:%02x:%02x:%02x),ScanRespData[%s],rssi(%d)",
                                        scan->address.bytes[5],scan->address.bytes[4],scan->address.bytes[3],
                                        scan->address.bytes[2],scan->address.bytes[1],scan->address.bytes[0],
                                        advData, scan->rssi);
                        LOG_INFO("%s",buffer);
                        demo_bt_send_buf(buffer, strlen(buffer));
                    }
                    break;
                }
                case SC_BLE_GATT_CONNECTED:
                {
                    SC_BLE_EVENT_ATT_CONNECTED_T *info = (SC_BLE_EVENT_ATT_CONNECTED_T *)msg->payload;
                    sprintf(buffer,"[LE_GATT_CONNECTED] connect with (%02x:%02x:%02x:%02x:%02x:%02x) acl_handle:0x%x",
                            info->addr.bytes[5],info->addr.bytes[4],info->addr.bytes[3],
                            info->addr.bytes[2],info->addr.bytes[1],info->addr.bytes[0],
                            info->acl_handle);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    sAPI_BleGattScanStart(info->acl_handle);
                    break;
                }
                case SC_BLE_GATT_DISCONNECTED:
                {
                    int *conn_handle = (int *)msg->payload;
                    sprintf(buffer,"[LE_GATT_DISCONNECTED] disconnect with device handler 0x%x",*conn_handle);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
                case SC_BLE_MTU_EXCHANGED:
                {
                    SC_BLE_EVENT_MTU_EXCHANGE_EVENT_T *mtu = (SC_BLE_EVENT_MTU_EXCHANGE_EVENT_T *)msg->payload;
                    sprintf(buffer,"[LE_MTU_EXCHANGED] handle 0x%x,mtu %d",mtu->acl_handle,mtu->mut);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
                case SC_BLE_CLIENT_HANDLE_NOTIFY:
                {
                    SC_BLE_EVENT_CLIENT_HANDLE_NOTIFY_T *notify = (SC_BLE_EVENT_CLIENT_HANDLE_NOTIFY_T *)msg->payload;
                    char data[256] = {0};
                    memcpy(data, notify->value, 255);
                    sprintf(buffer,"[LE_CLIENT_HANDLE_NOTIFY] connection_handle 0x%x,value_handle %d,data[%d][%s]",
                        notify->connection_handle,notify->value_handle,notify->value_length,data);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
                case SC_BLE_CLIENT_HANDLE_INDIATION:
                {
                    SC_BLE_EVENT_CLIENT_HANDLE_INDICATION_T *event = (SC_BLE_EVENT_CLIENT_HANDLE_INDICATION_T *)msg->payload;
                    char data[256] = {0};
                    memcpy(data, event->value, 255);
                    sprintf(buffer,"[LE_CLIENT_HANDLE_INDIATION] connection_handle 0x%x,value_handle %d,data[%d][%s]",
                        event->connection_handle,event->value_handle,event->value_length,data);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer, strlen(buffer));
                    break;
                }
                case SC_BLE_WHITE_LIST_SIZE:
                {
                    LOG_INFO("[LE_WHITE_LIST_SIZE]");
                    demo_bt_send_buf("[LE_WHITE_LIST_SIZE]", strlen("[LE_WHITE_LIST_SIZE]"));
                    break;
                }
                case SC_BLE_SMP_PASSKEY:
                {
                    SC_BLE_EVENT_SMP_PASSKEY_T *passkey = (SC_BLE_EVENT_SMP_PASSKEY_T *)msg->payload;
                    sprintf(buffer,"[LE_SMP_PASSKEY] passkey_value:%d %02X%02X%02X%02X%02X%02X",
                                passkey->passkey_value,
                                passkey->addr.bytes[5], passkey->addr.bytes[4],
                                passkey->addr.bytes[3], passkey->addr.bytes[2],
                                passkey->addr.bytes[1], passkey->addr.bytes[0]);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                }
                case SC_BLE_ADV_PHY_TXPOWER:
                {
                    signed char *tx_power = (signed char *)msg->payload;
                    sprintf(buffer,"[LE_ADV_PHY_TXPOWER] handle TX power %d",*tx_power);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                }
                case SC_BLE_SLAVE_LE_BOND_COMPLETE:
                {
                    SC_BLE_EVENT_BOND_COMPLETE_T *event = (SC_BLE_EVENT_BOND_COMPLETE_T *)msg->payload;
                    sprintf(buffer,"[LE_BOND_COMPLETE] type: %d, address: %02x%02x%02x%02x%02x%02x",
                                event->address_type,
                                event->address.bytes[5], event->address.bytes[4], event->address.bytes[3],
                                event->address.bytes[2], event->address.bytes[1], event->address.bytes[0]);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                }
                case SC_BLE_ATT_EVENT_CAN_SEND_NOW:
                {
                    LOG_INFO("[LE_ATT_EVENT_CAN_SEND_NOW]");
                    demo_bt_send_buf("[LE_ATT_EVENT_CAN_SEND_NOW]",strlen("[LE_ATT_EVENT_CAN_SEND_NOW]"));
                    break;
                }
                case SC_BLE_ATT_EVENT_CAN_WRITE_NOW:
                {
                    SC_BLE_EVENT_ATT_CAN_WRITE_NOW_T *event = (SC_BLE_EVENT_ATT_CAN_WRITE_NOW_T *)msg->payload;
                    sprintf(buffer,"[LE_ATT_EVENT_CAN_WRITE_NOW] conn: 0x%x, write_type: %d", event->connection_handle, event->write_type);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                }
                case SC_BLE_IDENTITY_INFO:
                {
                    SC_BLE_EVENT_IDENTITY_INFO_EVENT_T *info = (SC_BLE_EVENT_IDENTITY_INFO_EVENT_T *)msg->payload;
                    if (info->success_flag == 1)
                    {
                        sprintf(buffer,"[LE_IDENTITY_INFO] identity success, index: %d, connection_handle: 0x%x addr: %02x%02x%02x%02x%02x%02x, addr_type: %d identity_addr: %02x%02x%02x%02x%02x%02x, identity_addr_type: %d",
                            info->index, info->connection_handle,
                            info->address.bytes[5], info->address.bytes[4],
                            info->address.bytes[3], info->address.bytes[2],
                            info->address.bytes[1], info->address.bytes[0],
                            info->address_type,
                            info->identity_address.bytes[5], info->identity_address.bytes[4],
                            info->identity_address.bytes[3], info->identity_address.bytes[2],
                            info->identity_address.bytes[1], info->identity_address.bytes[0],
                            info->identity_address_type);

                    }
                    else
                    {
                        sprintf(buffer,"[LE_IDENTITY_INFO] identity fail, index: %d, connection_handle: 0x%x addr: %02x%02x%02x%02x%02x%02x, addr_type: %d",
                            info->index, info->connection_handle,
                            info->address.bytes[5], info->address.bytes[4],
                            info->address.bytes[3], info->address.bytes[2],
                            info->address.bytes[1], info->address.bytes[0],
                            info->address_type);
                    }
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                }
                case SC_BLE_DB_CONTROL_GET_INFO:
                {
                    SC_BLE_EVENT_GET_PAIR_INFO_T *info = (SC_BLE_EVENT_GET_PAIR_INFO_T *)msg->payload;
                    sprintf(buffer,"[LE_DB_CONTROL_GET_INFO] index: %d, seq: %d, valid count: %d, max count: %d,addr: %02x%02x%02x%02x%02x%02x, addr_type: %d",
                        info->index, info->seq, info->valid_count, info->max_count,
                        info->identity_address.bytes[5], info->identity_address.bytes[4],
                        info->identity_address.bytes[3], info->identity_address.bytes[2],
                        info->identity_address.bytes[1], info->identity_address.bytes[0],
                        info->identity_address_type);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                }
                case SC_BLE_ATT_EVENT_READ_DATA_RESULT:
                {
                    SC_BLE_EVENT_ATT_READ_DATA_RESULT_T *event = (SC_BLE_EVENT_ATT_READ_DATA_RESULT_T *)msg->payload;
                    char *buf = NULL;
                    if(event->value_len > 0)
                    {
                        buf = sAPI_Malloc(event->value_len + 1);
                        memset(buf, 0, event->value_len + 1);
                        memcpy(buf, event->value, event->value_len);
                    }
                    sprintf(buffer,"[LE_ATT_EVENT_READ_DATA_RESULT] connection_handle: 0x%x, value_handle: 0x%x, value_offset: %d, value_length: %d data[%s]",
                                event->connection_handle, event->value_handle, event->value_offset, event->value_len, buf);
                    sAPI_Free(buf);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                }
                case SC_BLE_ATT_EVENT_READ_OVER_RESULT:
                {
                    SC_BLE_EVENT_ATT_READ_OVER_RESULT_T *event = (SC_BLE_EVENT_ATT_READ_OVER_RESULT_T *)msg->payload;
                    sprintf(buffer,"[LE_ATT_EVENT_READ_OVER_RESULT] connection_handle: 0x%x, att_status: %d",event->connection_handle, event->att_status);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                }
                case SC_BLE_GATT_SCAN_RESULT:
                {
                    SC_BLE_EVENT_GATT_SCAN_RESULT_T *result = (SC_BLE_EVENT_GATT_SCAN_RESULT_T *)msg->payload;
                    sprintf(buffer,"[LE_GATT_SCAN_RESULT] connection_handle: 0x%x, status: %d", result->connection_handle, result->status);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                }
                case SC_BLE_GATT_SCAN_DUMP_SERVICE:
                {
                    SC_BLE_EVENT_GATT_SCAN_DUMP_SERVICE_T *result = (SC_BLE_EVENT_GATT_SCAN_DUMP_SERVICE_T *)msg->payload;
                    sprintf(buffer,"[LE_GATT_SCAN_RESULT] connection_handle: 0x%x", result->connection_handle);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer,strlen(buffer));

                    SC_GATT_CLIENT_SERVICE_T *service = &result->service;
                    memset(buffer, 0 ,sizeof(buffer));
                    if (service->uuid16)
                    {
                        sprintf(buffer,"service[0x%x-0x%x], uuid[0x%x]",
                                    service->start_group_handle, service->end_group_handle, service->uuid16);
                    }
                    else
                    {
                        sprintf(buffer,"service[0x%x-0x%x], uuid[%s]", service->start_group_handle, service->end_group_handle, ble_demo_uuid128_to_str(service->uuid128));
                    }
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                }
                case SC_BLE_GATT_SCAN_DUMP_CHARACTERISTIC:
                {
                    SC_BLE_EVENT_GATT_SCAN_DUMP_CHARACTERISTIC_T *result = (SC_BLE_EVENT_GATT_SCAN_DUMP_CHARACTERISTIC_T *)msg->payload;
                    sprintf(buffer,"[LE_GATT_SCAN_DUMP_CHARACTERISTIC] connection_handle: 0x%x", result->connection_handle);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    memset(buffer, 0 ,sizeof(buffer));
                    SC_GATT_CLIENT_CHARACTERISTIC_T *characteristic = &result->characteristic;
                    if (characteristic->uuid16)
                    {
                        sprintf(buffer,"characteristic: [0x%x-0x%x-0x%x], properties 0x%x(%s), uuid 0x%x value_handle(0x%x)",
                                    characteristic->start_handle, characteristic->value_handle, characteristic->end_handle, characteristic->properties,
                                    ble_demo_print_properties(characteristic->properties), characteristic->uuid16, characteristic->value_handle);
                    }
                    else
                    {
                        sprintf(buffer,"characteristic: [0x%x-0x%x-0x%x], properties 0x%x(%s), uuid %s value_handle(0x%x)",
                                    characteristic->start_handle, characteristic->value_handle, characteristic->end_handle, characteristic->properties,
                                    ble_demo_print_properties(characteristic->properties), 
                                    ble_demo_uuid128_to_str(characteristic->uuid128), characteristic->value_handle);
                    }
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                }
                case SC_BLE_GATT_SCAN_DUMP_DESCRIPTOR:
                {
                    SC_BLE_EVENT_GATT_SCAN_DUMP_DESCRIPTOR_T *result = (SC_BLE_EVENT_GATT_SCAN_DUMP_DESCRIPTOR_T *)msg->payload;
                    sprintf(buffer,"[LE_GATT_SCAN_DUMP_DESCRIPTOR] connection_handle: 0x%x", result->connection_handle);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    memset(buffer, 0 ,sizeof(buffer));
                    SC_GATT_CLIENT_CHARACTERISTIC_DESCRIPTOR_T *descriptor = &result->descriptor;
                    if (descriptor->uuid16)
                    {
                        sprintf(buffer,"descriptor, handle: 0x%x, uuid: 0x%x", descriptor->handle, 
                        descriptor->uuid16);
                    }
                    else
                    {
                       sprintf(buffer,"descriptor, handle: 0x%x, uuid: %s", descriptor->handle, ble_demo_uuid128_to_str(descriptor->uuid128));
                    }
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                }
                case SC_BLE_CONNECTION_UPDATE_COMPLETE:
                {
                    SC_BLE_EVENT_CONNECTION_PARAMETER_UPDATE_T *param = (SC_BLE_EVENT_CONNECTION_PARAMETER_UPDATE_T *)msg->payload;
                    sprintf(buffer,"[LE_CONNECTION_UPDATE_COMPLETE] status: %d, conn_handle: 0x%x, interval: %d, latency: %d, timeout: %d",
                                 param->status, param->conn_handle, param->conn_interval, param->slave_latency, param->supervision_timeout);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                }
                case SC_BLE_ATT_INDICATION_COMPLETE:
                {
                    sprintf(buffer,"[LE_ATT_INDICATION_COMPLETE]");
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                }
                case SC_BLE_ENCRYPTION_CHANGE:
                {
                    sprintf(buffer,"[LE_ENCRYPTION_CHANGE]");
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                }
                default:
                {
                    sprintf(buffer,"[SC_BT_EVENT_TYPE_LE_%d]",msg->event_id);
                    LOG_INFO("%s",buffer);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                }
            }
            break;
        }
        default:break;
    }
}

static ret_msg_t bt_open_cmd_proc(op_t op, arg_t *argv, int argn)
{
    int pair_mode = 0, io_cap = 0, bt_mode = 0;
    ret_msg_t ret = {0};


    pl_demo_get_value(&io_cap, argv, argn, ARGBT_IOCAP);
    pl_demo_get_value(&bt_mode, argv, argn, ARGBT_BTMODE);
    pl_demo_get_value(&pair_mode, argv, argn, ARGBT_PAIR_MODE);

    if(io_cap < SC_BT_IO_DISPLAYONLY || io_cap > SC_BT_IO_NOINPUTNOOUTPUT || sAPI_BtSetIoCap(io_cap) != 0)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "io_cap is error";
        LOG_ERROR("io_cap[%d] is error",io_cap);
        return ret;
    }

    if(bt_mode < 0 || bt_mode > SC_BT_LE)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "bt_mode is error";
        LOG_ERROR("bt_mode[%d] is error",bt_mode);
        return ret;
    }
    if(pair_mode < 0 || pair_mode > 1 || sAPI_BtPairingMode(pair_mode))
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "pair_mode is error";
        LOG_ERROR("pair_mode[%d] is error",pair_mode);
        return ret;
    }
    if(sAPI_BtOpenStaus())
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "bt is opened";
        LOG_ERROR("bt is opened");
        return ret;
    }
    sAPI_BtRegisterEventHandle(bt_event_handle_cb);
    ret.errcode = sAPI_BtOpen(bt_mode);
    if(ret.errcode == 0)
    {
        ret.msg = "bt open succ";
        LOG_INFO("bt open succ");
    }
    else
    {
        ret.msg = "bt open failed";
        LOG_ERROR("bt open failed[%d]",ret.errcode);
    }
    return ret;
}

static ret_msg_t bt_close_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    ret.errcode = sAPI_BtClose();
    if(ret.errcode == 0)
    {
        ret.msg = "bt close";
        LOG_INFO("bt close");
    }
    else
    {
        ret.msg = "bt close failed";
        LOG_ERROR("bt close failed[%d]",ret.errcode);
    }
    return ret;

}

static ret_msg_t bt_set_addr_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *addrStr = NULL;
    int j = 0;
    SC_BT_ADDR_T addr = {0};

    pl_demo_get_data(&addrStr, argv, argn, ARGBT_ADDR);
    if (!addrStr)
    {
        LOG_ERROR("address invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "address invalid";
        return ret;
    }
    for(int i = 0;i < 12;i += 2)
    {
        char tmp[4] = {0};
        memcpy(tmp,&addrStr[i],2);
        addr.bytes[j++] = strtoul(tmp,NULL,16);
    }
    LOG_INFO("addr %02X:%02X:%02X:%02X:%02X:%02X",
                addr.bytes[5],addr.bytes[4],addr.bytes[3],
                addr.bytes[2],addr.bytes[1],addr.bytes[0]);
    ret.errcode = sAPI_BtSetAddress(addr);
    if (0 != ret.errcode)
    {
        LOG_ERROR("bt set address failed");
        ret.msg = "bt set address failed";
    }
    return ret;
}

static ret_msg_t bt_get_addr_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    arg_t out_arg[1] = {0};
    int result = 0;
    SC_BT_ADDR_T addr = {0};
    char buffer[64] = {0};

    result = sAPI_BtGetAddress(&addr);
    if(result != 0)
    {
        LOG_ERROR("bt get address failed");
        ret.msg = "bt get address failed";
        ret.errcode = result;
        return ret;
    }
    sprintf(buffer,"%02X:%02X:%02X:%02X:%02X:%02X",
            addr.bytes[5],addr.bytes[4],addr.bytes[3],
            addr.bytes[2],addr.bytes[1],addr.bytes[0]);
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARGBT_ADDR;
    out_arg[0].value = pl_demo_make_value(0, (const char *)buffer, strlen(buffer));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);

    return ret;
}

static ret_msg_t bt_set_name_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *nameStr = NULL;
    char *name = NULL;

    pl_demo_get_data(&nameStr, argv, argn, ARGBT_NAME);
    if (!nameStr)
    {
        LOG_ERROR("name string invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "name string invalid";
        return ret;
    }
    name = sal_malloc(strlen(nameStr) + 1);
    strcpy(name, nameStr);
    LOG_INFO("name[%s]",name);
    ret.errcode = sAPI_BtSetName(name, strlen(name));
    if (0 != ret.errcode)
    {
        LOG_ERROR("bt set name failed");
        ret.msg = "bt set name failed";
    }
    return ret;
}

static ret_msg_t bt_get_name_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    arg_t out_arg[1] = {0};
    int result = 0;
    char name[255] = {0};
    unsigned char nameLen = 0;

    result = sAPI_BtGetName(name,&nameLen);
    if(result != 0)
    {
        LOG_ERROR("ble get name failed");
        ret.msg = "ble get name failed";
        ret.errcode = result;
        return ret;
    }
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARGBT_NAME;
    out_arg[0].value = pl_demo_make_value(0, (const char *)name, nameLen);

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);

    return ret;

}
static ret_msg_t bt_inquiry_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int duration = 0;
    int num = 0;

    pl_demo_get_value(&duration, argv, argn, ARGBT_SCAN_TIME);
    pl_demo_get_value(&num, argv, argn, ARGBT_RESP_NUM);

    LOG_INFO("duration[%d],num[%d]",duration,num);
    ret.errcode = sAPI_BtScanStart(duration, num);
    if (0 != ret.errcode)
    {
        LOG_ERROR("bt start to inquiry device failed");
        ret.msg = "bt start to inquiry device failed";
    }
    else
    {
        LOG_ERROR("bt start to inquiry device");
        ret.msg = "bt start to inquiry device";
    }
    return ret;
}

static ret_msg_t bt_inquiry_cancel_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    ret.errcode = sAPI_BtScanStop();
    if (0 != ret.errcode)
    {
        LOG_ERROR("bt stop to inquiry device failed");
        ret.msg = "bt stop to inquiry device failed";
    }
    else
    {
        LOG_ERROR("bt stop to inquiry device");
        ret.msg = "bt stop to inquiry device";
    }
    return ret;
}

static ret_msg_t bt_pair_request_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *addrStr = NULL;
    int j = 0;
    SC_BT_ADDR_T addr = {0};

    pl_demo_get_data(&addrStr, argv, argn, ARGBT_ADDR);
    if (!addrStr)
    {
        LOG_ERROR("address invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "address invalid";
        return ret;
    }
    for(int i = 0;i < 12;i += 2)
    {
        char tmp[4] = {0};
        memcpy(tmp,&addrStr[i],2);
        addr.bytes[j++] = strtoul(tmp,NULL,16);
    }
    LOG_INFO("addr %02X:%02X:%02X:%02X:%02X:%02X",
                addr.bytes[5],addr.bytes[4],addr.bytes[3],
                addr.bytes[2],addr.bytes[1],addr.bytes[0]);

    ret.errcode = sAPI_BtPairRequest(addr);
    if (0 != ret.errcode)
    {
        LOG_ERROR("bt request to pair device failed");
        ret.msg = "bt request to pair device failed";
    }
    else
    {
        LOG_ERROR("bt request to pair device");
        ret.msg = "bt request to pair device";
    }
    return ret;

}

static ret_msg_t bt_pair_response_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int mode = 0, accept = 0;
    int j = 0;
    SC_BT_ADDR_T addr = {0};
    const char *addrStr = NULL;
    const char *verification_code = NULL;

    pl_demo_get_value(&accept, argv, argn, ARGBT_PAIR_RESP);
    pl_demo_get_value(&mode, argv, argn, ARGBT_PAIR_MODE);
    pl_demo_get_data(&addrStr, argv, argn, ARGBT_ADDR);
    pl_demo_get_data(&verification_code, argv, argn, ARGBT_VERIFICATION);
    if (!addrStr)
    {
        LOG_ERROR("address invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "address invalid";
        return ret;
    }
    for(int i = 0;i < 12;i += 2)
    {
        char tmp[4] = {0};
        memcpy(tmp,&addrStr[i],2);
        addr.bytes[j++] = strtoul(tmp,NULL,16);
    }
    LOG_INFO("accept[%d] mode[%d],addr %02X:%02X:%02X:%02X:%02X:%02X, code/pin[%s]",
                accept, mode,
                addr.bytes[5],addr.bytes[4],addr.bytes[3],
                addr.bytes[2],addr.bytes[1],addr.bytes[0],
                verification_code);

    if(accept == 1)
    {
        if(mode == SC_BT_PAIR_MODE_PIN_CODE)
        {
            char pin[64] = {0};
            memcpy(pin, verification_code, strlen(verification_code));
            ret.errcode = sAPI_BtPairAccept(mode, addr, 0, pin);
        }
        else
        {
            int code = strtoul(verification_code, NULL,10 );
            ret.errcode = sAPI_BtPairAccept(mode, addr, code, NULL);
        }
        if(0 != ret.errcode)
        {
            LOG_ERROR("bt accpet to pair failed");
            ret.msg = "bt accpet to pair failed";
        }
    }
    else if(accept == 0)
    {
        ret.errcode = sAPI_BtPairRefuse(mode, addr);
        if(0 != ret.errcode)
        {
            LOG_ERROR("bt refuse to pair failed");
            ret.msg = "bt refuse to pair failed";
        }
    }
    else
    {
        LOG_ERROR("address invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "address invalid";
    }
    return ret;
}

static ret_msg_t bt_unpair_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int j = 0;
    SC_BT_ADDR_T addr = {0};
    const char *addrStr = NULL;

    pl_demo_get_data(&addrStr, argv, argn, ARGBT_ADDR);
    for(int i = 0;i < 12;i += 2)
    {
        char tmp[4] = {0};
        memcpy(tmp,&addrStr[i],2);
        addr.bytes[j++] = strtoul(tmp,NULL,16);
    }
    LOG_INFO("addr %02X:%02X:%02X:%02X:%02X:%02X, code/pin[%s]",
                addr.bytes[5],addr.bytes[4],addr.bytes[3],
                addr.bytes[2],addr.bytes[1],addr.bytes[0]);

    ret.errcode = sAPI_BtUnpair(addr);
    if(ret.errcode != 0)
    {
        LOG_ERROR("bt upair device failed");
        ret.msg = "bt upair device failed";
    }
    return ret;

}
static ret_msg_t bt_get_pairing_info_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_BT_DEVICE_RECORD_T record = {0};
    int i;
    char buffer[512] = {0};
    arg_t out_arg[1] = {0};

    for(i = 0; i < 10; i++)
    {
        int result = sAPI_BtPairedQueryByIndex(i, &record);
        if(result != 0)
            break;
        LOG_INFO("bt device record index(%d) peer name[%s] address(%02X:%02X:%02X:%02X:%02X:%02X)",
                i, record.name,
                record.addr[5],record.addr[4],
                record.addr[3],record.addr[2],
                record.addr[1],record.addr[0]);
        sprintf(buffer, "bt device record index(%d) peer name[%s] address(%02X:%02X:%02X:%02X:%02X:%02X)",
                i, record.name,
                record.addr[5],record.addr[4],
                record.addr[3],record.addr[2],
                record.addr[1],record.addr[0]);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = ARGBT_RESULT;
        out_arg[0].value = pl_demo_make_value(0, (const char *)buffer, strlen(buffer));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
        pl_demo_release_value(out_arg[0].value);
    }
    return ret;

}

static ret_msg_t bt_spp_connect_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int j = 0;
    SC_BT_ADDR_T addr = {0};
    const char *addrStr = NULL;

    pl_demo_get_data(&addrStr, argv, argn, ARGBT_ADDR);
    for(int i = 0;i < 12;i += 2)
    {
        char tmp[4] = {0};
        memcpy(tmp,&addrStr[i],2);
        addr.bytes[j++] = strtoul(tmp,NULL,16);
    }
    LOG_INFO("addr %02X:%02X:%02X:%02X:%02X:%02X, code/pin[%s]",
                addr.bytes[5],addr.bytes[4],addr.bytes[3],
                addr.bytes[2],addr.bytes[1],addr.bytes[0]);

    ret.errcode = sAPI_BtSppConnect(addr);
    if(ret.errcode != 0)
    {
        LOG_ERROR("bt spp connect failed");
        ret.msg = "bt spp connect failed";
    }
    return ret;
}

static ret_msg_t bt_spp_disconnect_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int port;
    pl_demo_get_value(&port, argv, argn, ARGBT_SPP_PORT);

    ret.errcode = sAPI_BtSppDisconnect(port);
    if(ret.errcode != 0)
    {
        LOG_ERROR("bt spp disconnect failed");
        ret.msg = "bt spp disconnect failed";
    }
    return ret;
}

static ret_msg_t bt_spp_send_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int sppport = 0;
    int datasize = 0;

    pl_demo_get_value(&sppport, argv, argn, ARGBT_SPP_PORT);
    pl_demo_get_value(&datasize, argv, argn, ARGBT_SPP_DATA_LEN);
    if (datasize < 0 || sppport < 0)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    g_data_size = datasize;
    g_spp_port = sppport;
    g_data_type = 4;
    return ret;

}

static ret_msg_t bt_rssi_read_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int conn_handle;
    pl_demo_get_value(&conn_handle, argv, argn, ARGLE_CONN_HANDLE);

    ret.errcode = sAPI_BtReadRssi(conn_handle);
    if(ret.errcode != 0)
    {
        LOG_ERROR("bt start to read rssi failed");
        ret.msg = "bt start to read rssi failed";
    }
    return ret;
}

static ret_msg_t le_set_random_addr_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *addrStr = NULL;
    int j = 0;
    SC_BT_ADDR_T addr = {0};

    pl_demo_get_data(&addrStr, argv, argn, ARGBT_ADDR);
    if (!addrStr)
    {
        LOG_ERROR("address invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "address invalid";
        return ret;
    }
    for(int i = 0;i < 12;i += 2)
    {
        char tmp[4] = {0};
        memcpy(tmp,&addrStr[i],2);
        addr.bytes[j++] = strtoul(tmp,NULL,16);
    }
    LOG_INFO("addr %02X:%02X:%02X:%02X:%02X:%02X",
                addr.bytes[5],addr.bytes[4],addr.bytes[3],
                addr.bytes[2],addr.bytes[1],addr.bytes[0]);
    ret.errcode = sAPI_BleSetRandomAddress(&addr);
    if (0 != ret.errcode)
    {
        LOG_ERROR("ble set random address failed");
        ret.msg = "ble set arndom address failed";
    }
    return ret;

}

static ret_msg_t le_set_adv_param_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int interval_min = 0, interval_max = 0, adv_type = 0;
    int own_addr_type = 0, peer_addr_type = 0, filter = 0;
    const char *addrStr = NULL;
    SC_BLE_TASK_ADV_PARAMETERS_T adv_param = {0};
    int j = 0;
    SC_BT_ADDR_T addr = {0};

    pl_demo_get_value(&interval_min, argv, argn, ARGLE_ADV_INTERVAL_MIN);
    pl_demo_get_value(&interval_max, argv, argn, ARGLE_ADV_INTERVAL_MAX);
    pl_demo_get_value(&adv_type, argv, argn, ARGLE_ADV_TYPE);
    pl_demo_get_value(&own_addr_type, argv, argn, ARGLE_ADDR_TYPE);
    pl_demo_get_value(&peer_addr_type, argv, argn, ARGLE_ADDR_TYPE);
    pl_demo_get_data(&addrStr, argv, argn, ARGBT_ADDR);
    pl_demo_get_value(&filter, argv, argn, ARGLE_ADV_FILTER);
    if (!addrStr)
    {
        LOG_ERROR("address invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "address invalid";
        return ret;
    }
    for(int i = 0;i < 12;i += 2)
    {
        char tmp[4] = {0};
        memcpy(tmp,&addrStr[i],2);
        addr.bytes[j++] = strtoul(tmp,NULL,16);
    }
    LOG_INFO("interval_min[%d],interval_max[%d],adv_type[%d],own_addr_type[%d],peer_addr_type[%d],addr %02X:%02X:%02X:%02X:%02X:%02X,filter[%d]",
                interval_min,interval_max,adv_type,own_addr_type,peer_addr_type,
                addr.bytes[5],addr.bytes[4],addr.bytes[3],
                addr.bytes[2],addr.bytes[1],addr.bytes[0],
                filter);
    adv_param.interval_max = interval_max;
    adv_param.interval_min = interval_min;
    adv_param.own_address_type = own_addr_type;
    adv_param.peer_address_type = peer_addr_type;
    adv_param.peer_address = addr;
    adv_param.filter = filter;
    ret.errcode = sAPI_BleSetAdvParam(&adv_param);
    if (0 != ret.errcode)
    {
        LOG_ERROR("ble set adv parameters failed");
        ret.msg = "ble set adv parameters failed";
    }
    return ret;
}

static ret_msg_t le_set_adv_data_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *adv_data = NULL;
    char buff[32] = {0};
    int i,j = 0;

    pl_demo_get_data(&adv_data, argv, argn, ARGLE_ADV_DATA);
    for(i = 0;i < 62;i += 2)
    {
        char tmp[4] = {0};
        memcpy(tmp,&adv_data[i],2);
        buff[j ++] = strtoul(tmp,NULL,16);
    }
    LOG_INFO("adv_data[%s] len:%d",buff,strlen(buff));
    ret.errcode = sAPI_BleSetAdvData((unsigned char *)buff, strlen(buff));
    if (0 != ret.errcode)
    {
        LOG_ERROR("ble set adv data failed");
        ret.msg = "ble set adv data failed";
    }
    return ret;
}

static ret_msg_t le_set_scan_resp_data_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *scan_resp_data = NULL;
    char buff[32] = {0};
    int i,j = 0;

    pl_demo_get_data(&scan_resp_data, argv, argn, ARGLE_SCAN_RESP);
    for(i = 0;i < 62;i += 2)
    {
        char tmp[4] = {0};
        memcpy(tmp,&scan_resp_data[i],2);
        buff[j ++] = strtoul(tmp,NULL,16);
    }
    LOG_INFO("scan_resp_data[%s] len:%d",buff,strlen(buff));
    ret.errcode = sAPI_BleSetScanResponseData((unsigned char *)buff, strlen(buff));
    if (0 != ret.errcode)
    {
        LOG_ERROR("ble set scan_resp data failed");
        ret.msg = "ble set scan_resp data failed";
    }
    return ret;
}

static ret_msg_t le_set_ext_adv_param_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int properties = 0,interval_min = 0, interval_max = 0, adv_channel = 0;
    int own_addr_type = 0, tx_power = 0, primary_phy = 0, secondary_phy = 0, adv_mode = 0;
    const char *addrStr = NULL;
    SC_BLE_TASK_EXT_ADV_PARAMETERS_T adv_param = {0};
    int j = 0;
    SC_BT_ADDR_T addr = {0};

    pl_demo_get_value(&properties, argv, argn, ARGLE_PROPERTIES);
    pl_demo_get_value(&interval_min, argv, argn, ARGLE_ADV_INTERVAL_MIN);
    pl_demo_get_value(&interval_max, argv, argn, ARGLE_ADV_INTERVAL_MAX);
    pl_demo_get_value(&adv_channel, argv, argn, ARGLE_CHANNEL);
    pl_demo_get_value(&own_addr_type, argv, argn, ARGLE_ADDR_TYPE);
    pl_demo_get_value(&tx_power, argv, argn, ARGLE_ADV_TYPE);
    pl_demo_get_value(&primary_phy, argv, argn, ARGLE_ADDR_TYPE);
    pl_demo_get_value(&secondary_phy, argv, argn, ARGLE_ADV_FILTER);
    pl_demo_get_value(&adv_mode, argv, argn, ARGLE_ADV_FILTER);
    pl_demo_get_data(&addrStr, argv, argn, ARGBT_ADDR);
    if (!addrStr)
    {
        LOG_ERROR("address invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "address invalid";
        return ret;
    }
    for(int i = 0;i < 12;i += 2)
    {
        char tmp[4] = {0};
        memcpy(tmp,&addrStr[i],2);
        addr.bytes[j++] = strtoul(tmp,NULL,16);
    }
    LOG_INFO("properties[%d],interval_min[%d],interval_max[%d],adv_channel[%d],own_addr_type[%d],tx_power[%d],primary_phy[%d],secondary_phy[%d],adv_mode[%d],addr %02X:%02X:%02X:%02X:%02X:%02X",
                properties,interval_min,interval_max,
                adv_channel,own_addr_type,tx_power,
                primary_phy,secondary_phy,adv_mode,
                addr.bytes[5],addr.bytes[4],addr.bytes[3],
                addr.bytes[2],addr.bytes[1],addr.bytes[0]);
    adv_param.properties = properties;
    adv_param.interval_max = interval_max;
    adv_param.interval_min = interval_min;
    adv_param.channel = adv_channel;
    adv_param.own_address_type = own_addr_type;
    adv_param.tx_power = tx_power;
    adv_param.primary_phy = primary_phy;
    adv_param.secondary_phy = secondary_phy;
    adv_param.adv_mode = adv_mode;
    adv_param.peer_address = addr;
    ret.errcode = sAPI_BleSetExtAdvParam(&adv_param);
    if (0 != ret.errcode)
    {
        LOG_ERROR("ble set ext_adv parameters failed");
        ret.msg = "ble set ext_adv parameters failed";
    }
    return ret;
}

static ret_msg_t le_set_ext_adv_data_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *ext_adv_data = NULL;
    char buff[256] = {0};
    int i,j = 0;

    pl_demo_get_data(&ext_adv_data, argv, argn, ARGLE_ADV_DATA);
    for(i = 0;i < 510;i += 2)
    {
        char tmp[4] = {0};
        memcpy(tmp,&ext_adv_data[i],2);
        buff[j ++] = strtoul(tmp,NULL,16);
    }
    LOG_INFO("ext_adv_data[%s] len:%d",buff,strlen(buff));
    ret.errcode = sAPI_BleSetExtAdvData((unsigned char *)buff, strlen(buff));
    if (0 != ret.errcode)
    {
        LOG_ERROR("ble set ext_adv data failed");
        ret.msg = "ble set ext_adv data failed";
    }
    return ret;
}

static ret_msg_t le_set_ext_scan_resp_data_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *ext_scan_resp_data = NULL;
    char buff[256] = {0};
    int i,j = 0;

    pl_demo_get_data(&ext_scan_resp_data, argv, argn, ARGLE_ADV_DATA);
    for(i = 0;i < 510;i += 2)
    {
        char tmp[4] = {0};
        memcpy(tmp,&ext_scan_resp_data[i],2);
        buff[j ++] = strtoul(tmp,NULL,16);
    }
    LOG_INFO("ext_scan_resp_data[%s] len:%d",buff,strlen(buff));
    ret.errcode = sAPI_BleSetExtAdvData((unsigned char *)buff, strlen(buff));
    if (0 != ret.errcode)
    {
        LOG_ERROR("ble set ext_scan_resp data failed");
        ret.msg = "ble set ext_scan_resp data failed";
    }
    return ret;
}

static ret_msg_t le_register_service_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    ret.errcode = sAPI_BleRegisterAttServer(profile_data,sizeof(profile_data),sc_demo_att_read_callback,sc_demo_att_write_callback);
    if (0 != ret.errcode)
    {
        LOG_ERROR("ble register att_server failed");
        ret.msg = "ble register att_server failed";
    }
    return ret;
}

static ret_msg_t le_enable_adv_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int adv_type = 0;

    pl_demo_get_value(&adv_type, argv, argn, ARGLE_ADV_TYPE);
    if(adv_type < 0 || adv_type > 1)
    if (0 != ret.errcode)
    {
        LOG_ERROR("ble adv_type error");
        ret.msg = "ble adv_type error";
        return ret;
    }

    if(adv_type)
    {
        ret.errcode = sAPI_BleEnableExtAdv();
        if (0 != ret.errcode)
        {
            LOG_ERROR("ble enable ext_adv failed");
            ret.msg = "ble enable ext_adv failed";
        }
    }
    else
    {
        ret.errcode = sAPI_BleEnableAdv();
        if (0 != ret.errcode)
        {
            LOG_ERROR("ble enable adv failed");
            ret.msg = "ble enable adv failed";
        }
    }
    if (0 == ret.errcode)
    {
        LOG_INFO("ble enable adv");
        ret.msg = "ble enable adv";
    }
    return ret;

}

static ret_msg_t le_disable_adv_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int adv_type = 0;

    pl_demo_get_value(&adv_type, argv, argn, ARGLE_ADV_TYPE);
    if(adv_type < 0 || adv_type > 1)
    if (0 != ret.errcode)
    {
        LOG_ERROR("ble adv_type error");
        ret.msg = "ble adv_type error";
        return ret;
    }

    if(adv_type)
    {
        ret.errcode = sAPI_BleDisableExtAdv();
        if (0 != ret.errcode)
        {
            LOG_ERROR("ble disable ext_adv failed");
            ret.msg = "ble disable ext_adv failed";
        }
    }
    else
    {
        ret.errcode = sAPI_BleDisableAdv();
        if (0 != ret.errcode)
        {
            LOG_ERROR("ble disable adv failed");
            ret.msg = "ble disable adv failed";
        }
    }
    if (0 == ret.errcode)
    {
        LOG_INFO("ble disable adv");
        ret.msg = "ble disable adv";
    }
    return ret;
}

static ret_msg_t le_indicate_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int conn_handle = 0, att_handle = 0, data_len;

    pl_demo_get_value(&conn_handle, argv, argn, ARGLE_CONN_HANDLE);
    pl_demo_get_value(&att_handle, argv, argn, ARGLE_ATT_HANDLE);
    pl_demo_get_value(&data_len, argv, argn, ARGLE_DATA_LEN);
    if(data_len <= 0)
    {
        LOG_ERROR("ble indicater data len error");
        ret.msg = "ble indicater data len error";
        return ret;
    }
    g_data_size = data_len;
    g_conn_handle = conn_handle;
    g_att_handle = att_handle;
    g_data_type = 0;
    return ret;
}

static ret_msg_t le_notify_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int conn_handle = 0, att_handle = 0, data_len;

    pl_demo_get_value(&conn_handle, argv, argn, ARGLE_CONN_HANDLE);
    pl_demo_get_value(&att_handle, argv, argn, ARGLE_ATT_HANDLE);
    pl_demo_get_value(&data_len, argv, argn, ARGLE_DATA_LEN);
    if(data_len <= 0)
    {
        LOG_ERROR("ble indicater data len error");
        ret.msg = "ble indicater data len error";
        return ret;
    }
    g_data_size = data_len;
    g_conn_handle = conn_handle;
    g_att_handle = att_handle;
    g_data_type = 1;
    return ret;
}

static ret_msg_t le_connect_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *addrStr = NULL;
    int addr_type = 0;
    int j = 0;
    SC_BT_ADDR_T addr = {0};

    pl_demo_get_data(&addrStr, argv, argn, ARGBT_ADDR);
    pl_demo_get_value(&addr_type, argv, argn, ARGBT_ADDR_TYPE);

    if (!addrStr)
    {
        LOG_ERROR("address invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "address invalid";
        return ret;
    }
    for(int i = 0;i < 12;i += 2)
    {
        char tmp[4] = {0};
        memcpy(tmp,&addrStr[i],2);
        addr.bytes[j++] = strtoul(tmp,NULL,16);
    }
    LOG_INFO("addr %02X:%02X:%02X:%02X:%02X:%02X",
                addr.bytes[5],addr.bytes[4],addr.bytes[3],
                addr.bytes[2],addr.bytes[1],addr.bytes[0]);

    ret.errcode = sAPI_BleConnect(&addr, addr_type);
    if(ret.errcode != 0)
    {
        LOG_ERROR("ble connect failed");
        ret.msg = "ble connect failed";
    }
    return ret;
}

static ret_msg_t le_disconnect_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int conn_handle = 0;

    pl_demo_get_value(&conn_handle, argv, argn, ARGLE_CONN_HANDLE);
    ret.errcode = sAPI_BleDisconnect(conn_handle);
    if(ret.errcode != 0)
    {
        LOG_ERROR("ble disconnect failed");
        ret.msg = "ble disconnect failed";
    }
    return ret;
}

static ret_msg_t le_mtu_request_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int conn_handle = 0, mtu_size = 0;

    pl_demo_get_value(&conn_handle, argv, argn, ARGLE_CONN_HANDLE);
    pl_demo_get_value(&mtu_size, argv, argn, ARGLE_MTU_SIZE);

    if(mtu_size <= 0)
    {
        LOG_ERROR("ble mtu_size error");
        ret.msg = "ble mtu_size error";
        return ret;
    }
    LOG_INFO("conn_handle[%d],mtu_size[%d]",conn_handle,mtu_size);

    ret.errcode = sAPI_BleMtuRequest(conn_handle, mtu_size);
    if(ret.errcode != 0)
    {
        LOG_ERROR("ble mtu request failed");
        ret.msg = "ble mtu request failed";
    }
    return ret;
}

static ret_msg_t le_att_scan_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int conn_handle = 0;

    pl_demo_get_value(&conn_handle, argv, argn, ARGLE_CONN_HANDLE);
    LOG_INFO("conn_handle[%d]",conn_handle);

    ret.errcode = sAPI_BleGattScanStart(conn_handle);
    if(ret.errcode != 0)
    {
        LOG_ERROR("ble att_scan failed");
        ret.msg = "ble att_scan failed";
    }
    return ret;
}

static ret_msg_t le_client_scan_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int type = 0, interval = 0, window = 0, own_addr_type = 0;

    pl_demo_get_value(&type, argv, argn, ARGLE_SCAN_MODE);
    pl_demo_get_value(&interval, argv, argn, ARGLE_SCAN_INTERVAL);
    pl_demo_get_value(&window, argv, argn, ARGLE_SCAN_WINDOW);
    pl_demo_get_value(&own_addr_type, argv, argn, ARGBT_ADDR_TYPE);
    LOG_INFO("type[%d] interval[%d],window[%d],own_addr_type[%d]",
                type,interval,window,own_addr_type);

    ret.errcode = sAPI_BleScan(type,interval,window,own_addr_type);
    if(ret.errcode != 0)
    {
        LOG_ERROR("ble client scan failed");
        ret.msg = "ble clinet scan failed";
    }
    return ret;
}

static ret_msg_t le_client_scan_stop_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    ret.errcode = sAPI_BleScanStop();
    if(ret.errcode != 0)
    {
        LOG_ERROR("ble stop scanning failed");
        ret.msg = "ble stop scanning failed";
    }
    return ret;
}

static ret_msg_t le_client_read_request_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int conn_handle = 0, att_handle = 0;

    pl_demo_get_value(&conn_handle, argv, argn, ARGLE_CONN_HANDLE);
    pl_demo_get_value(&att_handle, argv, argn, ARGLE_ATT_HANDLE);

    LOG_INFO("conn_handle[%d] att_handle[%d]",conn_handle,att_handle);
    ret.errcode = sAPI_BleReadRequest(conn_handle,att_handle);
    if(ret.errcode != 0)
    {
        LOG_ERROR("ble request to read failed");
        ret.msg = "ble request to read failed";
    }
    return ret;

}

static ret_msg_t le_client_write_request_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int conn_handle = 0, att_handle = 0, data_len;

    pl_demo_get_value(&conn_handle, argv, argn, ARGLE_CONN_HANDLE);
    pl_demo_get_value(&att_handle, argv, argn, ARGLE_ATT_HANDLE);
    pl_demo_get_value(&data_len, argv, argn, ARGLE_DATA_LEN);
    if(data_len <= 0)
    {
        LOG_ERROR("ble write_request data len error");
        ret.msg = "ble write_request data len error";
        return ret;
    }
    g_data_size = data_len;
    g_conn_handle = conn_handle;
    g_att_handle = att_handle;
    g_data_type = 2;
    return ret;

}

static ret_msg_t le_client_write_command_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int conn_handle = 0, att_handle = 0, data_len;

    pl_demo_get_value(&conn_handle, argv, argn, ARGLE_CONN_HANDLE);
    pl_demo_get_value(&att_handle, argv, argn, ARGLE_ATT_HANDLE);
    pl_demo_get_value(&data_len, argv, argn, ARGLE_DATA_LEN);
    if(data_len <= 0)
    {
        LOG_ERROR("ble write_command data len error");
        ret.msg = "ble write_command data len error";
        return ret;
    }
    g_data_size = data_len;
    g_conn_handle = conn_handle;
    g_att_handle = att_handle;
    g_data_type = 3;
    return ret;
}

static ret_msg_t le_pairing_request_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int conn_handle = 0;

    pl_demo_get_value(&conn_handle, argv, argn, ARGLE_CONN_HANDLE);

    ret.errcode = sAPI_BlePairRequest(conn_handle);
    if(ret.errcode != 0)
    {
        LOG_ERROR("ble request to pair device failed");
        ret.msg = "ble request to pair device failed";
    }
    return ret;
}

static ret_msg_t le_get_pairing_info_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int index = 0;
    
    pl_demo_get_value(&index, argv, argn, ARGLE_PAIR_INDEX);
    
    ret.errcode = sAPI_BleGetPairInfo(index);
    if(ret.errcode != 0)
    {
        LOG_ERROR("ble request to get pairing information failed");
        ret.msg = "ble request to get pairing information failed";
    }
    return ret;
}

static ret_msg_t le_remove_pairing_info_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int index = 0;
    
    pl_demo_get_value(&index, argv, argn, ARGLE_PAIR_INDEX);
    
    sAPI_BleRemovePairInfo(index);

    LOG_INFO("ble request to remove pairing information");
    ret.msg = "ble request to remove pairing information";
    return ret;

}

static ret_msg_t le_set_white_list_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *addrStr = NULL;
    int addr_type = 0;
    int j = 0;
    SC_BT_ADDR_T addr = {0};

    pl_demo_get_data(&addrStr, argv, argn, ARGBT_ADDR);
    pl_demo_get_value(&addr_type, argv, argn, ARGBT_ADDR_TYPE);

    if (!addrStr)
    {
        LOG_ERROR("address invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "address invalid";
        return ret;
    }
    for(int i = 0;i < 12;i += 2)
    {
        char tmp[4] = {0};
        memcpy(tmp,&addrStr[i],2);
        addr.bytes[j++] = strtoul(tmp,NULL,16);
    }
    LOG_INFO("addr %02X:%02X:%02X:%02X:%02X:%02X",
                addr.bytes[5],addr.bytes[4],addr.bytes[3],
                addr.bytes[2],addr.bytes[1],addr.bytes[0]);

    ret.errcode = sAPI_BleSetWhiteList(addr, addr_type);
    if(ret.errcode != 0)
    {
        LOG_ERROR("ble set white list failed");
        ret.msg = "ble set white list failed";
    }
    return ret;
}
static ret_msg_t le_clear_white_list_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
        
    ret.errcode = sAPI_BleClearWhiteList();
    if(ret.errcode != 0)
    {
        LOG_ERROR("ble request to clear white list failed");
        ret.msg = "ble request to clear white list failed";
    }
    return ret;

}

