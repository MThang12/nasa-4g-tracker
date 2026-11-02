#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "simcom_ble.h"
#include "simcom_os.h"
#include "demo_bt.h"


#define LOG_ERROR(...) sal_log_error("[DEMO-BLE] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-BLE] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-BLE] " __VA_ARGS__)

static ret_msg_t ble_set_addr_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_get_addr_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_set_name_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_get_name_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_open_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_close_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_set_param_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_set_adv_data_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_set_scan_resp_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_enable_adv_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_disable_adv_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_scan_start_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_scan_stop_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_connect_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_disconnect_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_indicate_cmd_proc(op_t op, arg_t *argv, int argn);
static int ble_indicate_data_size(void);
static ret_msg_t ble_indicate_data_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms));
static ret_msg_t ble_notify_cmd_proc(op_t op, arg_t *argv, int argn);
static int ble_notify_data_size(void);
static ret_msg_t ble_notify_data_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms));
static ret_msg_t ble_read_request_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_write_request_cmd_proc(op_t op, arg_t *argv, int argn);
static int ble_write_data_size(void);
static ret_msg_t ble_write_data_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms));
static ret_msg_t ble_write_command_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_pairing_request_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_enable_pairing_request_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_get_pairing_info_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_remove_pairing_info_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_clear_pairing_info_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_set_white_list_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_clear_white_list_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ble_rssi_read_cmd_proc(op_t op, arg_t *argv, int argn);

arg_t set_le_addr_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGBT_ADDR,
        .msg = "bt address,like XXXXXXXXXXXX",
    }
};

const menu_list_t func_le_set_addr =
{
    .menu_name = "set bt address",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = set_le_addr_args,
        .argn = sizeof(set_le_addr_args) / sizeof(arg_t),
        .methods.cmd_handler = ble_set_addr_cmd_proc,
    },
};

const menu_list_t func_le_get_addr =
{
    .menu_name = "get bt address",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = ble_get_addr_cmd_proc,
    },
};

arg_t set_le_name_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGBT_NAME,
        .msg = "bt name string",
    }
};

const menu_list_t func_le_set_name =
{
    .menu_name = "set ble name",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = set_le_name_args,
        .argn = sizeof(set_le_name_args) / sizeof(arg_t),
        .methods.cmd_handler = ble_set_name_cmd_proc,
    },
};

const menu_list_t func_le_get_name =
{
    .menu_name = "get ble name",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = ble_get_name_cmd_proc,
    },
};

const menu_list_t func_ble_open =
{
    .menu_name = "open bluetooth",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = ble_open_cmd_proc,
    },
};

const menu_list_t func_ble_close =
{
    .menu_name = "close bluetooth",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = ble_close_cmd_proc,
    },
};

arg_t adv_param_args[7] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_ADV_INTERVAL_MIN,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_ADV_INTERVAL_MAX,
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

const menu_list_t func_ble_set_adv_param =
{
    .menu_name = "set adv param",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = adv_param_args,
        .argn = sizeof(adv_param_args) / sizeof(arg_t),
        .methods.cmd_handler = ble_set_param_cmd_proc,
    },
};

arg_t set_adv_data_args[2] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGLE_ADV_DATA,
        .msg = "hex string",
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGLE_SCAN_RESP,
        .msg = "hex string",
    }

};

const menu_list_t func_ble_set_adv_data =
{
    .menu_name = "set adv data",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = set_adv_data_args,
        .argn = sizeof(set_adv_data_args) / sizeof(arg_t),
        .methods.cmd_handler = ble_set_adv_data_cmd_proc,
    },
};

arg_t set_scan_resp_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGLE_SCAN_RESP,
        .msg = "hex string",
    }
};

const menu_list_t func_ble_set_scan_resp =
{
    .menu_name = "set scan_resp data",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = set_scan_resp_args,
        .argn = 1,
        .methods.cmd_handler = ble_set_scan_resp_cmd_proc,
    },
};


const menu_list_t func_ble_enable_adv =
{
    .menu_name = "enable adv",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = ble_enable_adv_cmd_proc,
    },
};

const menu_list_t func_ble_disable_adv =
{
    .menu_name = "disable adv",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = ble_disable_adv_cmd_proc,
    },
};

arg_t scan_args[4] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_SCAN_MODE,
        .msg = "active/passive scan",
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
        .arg_name = ARGLE_ADDR_TYPE,
        .msg = "own address type",
    }
};

const menu_list_t func_ble_scan_start =
{
    .menu_name = "ble scan start",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = scan_args,
        .argn = sizeof(scan_args)/sizeof(arg_t),
        .methods.cmd_handler = ble_scan_start_cmd_proc,
    },
};

const menu_list_t func_ble_scan_stop =
{
    .menu_name = "ble scan stop",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = ble_scan_stop_cmd_proc,
    }
};

arg_t connect_args[2] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGBT_ADDR,
        .msg = "peer address",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_ADDR_TYPE,
        .msg = "own address type",
    }
};

const menu_list_t func_ble_connect =
{
    .menu_name = "ble connect",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = connect_args,
        .argn = sizeof(connect_args)/sizeof(arg_t),
        .methods.cmd_handler = ble_connect_cmd_proc,
    },
};

const menu_list_t func_ble_disconnect =
{
    .menu_name = "ble disconnect",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = ble_disconnect_cmd_proc,
    },
};

arg_t indicate_args[2] =
{
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

const menu_list_t func_ble_indicate =
{
    .menu_name = "ble indicate",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = indicate_args,
        .argn = sizeof(indicate_args)/sizeof(arg_t),
        .methods.cmd_handler = ble_indicate_cmd_proc,
        .methods.raw_handler = ble_indicate_data_proc,
        .methods.needRAWData = ble_indicate_data_size,
    },
};

const menu_list_t func_ble_notify =
{
    .menu_name = "ble notify",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = indicate_args,
        .argn = sizeof(indicate_args)/sizeof(arg_t),
        .methods.cmd_handler = ble_notify_cmd_proc,
        .methods.raw_handler = ble_notify_data_proc,
        .methods.needRAWData = ble_notify_data_size,
    },
};

arg_t read_att_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_ATT_HANDLE,
        .msg = "att handle",
    }
};

const menu_list_t func_ble_read_request =
{
    .menu_name = "ble read request",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = read_att_args,
        .argn = sizeof(read_att_args)/sizeof(arg_t),
        .methods.cmd_handler = ble_read_request_cmd_proc,
    },
};

const menu_list_t func_ble_write_request =
{
    .menu_name = "ble write request",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = indicate_args,
        .argn = sizeof(indicate_args)/sizeof(arg_t),
        .methods.cmd_handler = ble_write_request_cmd_proc,
        .methods.raw_handler = ble_write_data_proc,
        .methods.needRAWData = ble_write_data_size,
    },
};

const menu_list_t func_ble_write_command =
{
    .menu_name = "bt write command",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = indicate_args,
        .argn = sizeof(indicate_args)/sizeof(arg_t),
        .methods.cmd_handler = ble_write_command_cmd_proc,
        .methods.raw_handler = ble_write_data_proc,
        .methods.needRAWData = ble_write_data_size,
    },
};

const menu_list_t func_ble_pairing_request =
{
    .menu_name = "ble pairing request",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = ble_pairing_request_cmd_proc,
    },
};

arg_t enable_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_PAIR_ENABLE,
        .msg = "enable/disable",
    }
};

const menu_list_t func_ble_enable_pairing_request =
{
    .menu_name = "ble enable pairing request",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = enable_args,
        .argn = 1,
        .methods.cmd_handler = ble_enable_pairing_request_cmd_proc,
    },
};

const menu_list_t func_ble_get_pairing_info =
{
    .menu_name = "ble get pairing info",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = ble_get_pairing_info_cmd_proc,
    },
};

arg_t remove_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGLE_PAIR_INDEX,
        .msg = "index",
    }
};

const menu_list_t func_ble_remove_pairing_info =
{
    .menu_name = "ble remove pairing info",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = remove_args,
        .argn = 1,
        .methods.cmd_handler = ble_remove_pairing_info_cmd_proc,
    },
};

const menu_list_t func_ble_clear_pairing_info =
{
    .menu_name = "ble_clear pairing info",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = ble_clear_pairing_info_cmd_proc,
    },
};

const menu_list_t func_ble_set_white_list =
{
    .menu_name = "ble set white list",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = ble_set_white_list_cmd_proc,
    },
};

const menu_list_t func_ble_clear_white_list =
{
    .menu_name = "ble clear white list",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = ble_clear_white_list_cmd_proc,
    },
};

const menu_list_t func_ble_read_rssi =
{
    .menu_name = "ble read rssi",
    .menu_type = TYPE_FUNC,
    .parent = &ble_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = ble_rssi_read_cmd_proc,
    },
};

const menu_list_t *ble_menu_sub[] =
{
    &func_le_set_addr,
    &func_le_get_addr,
    &func_le_set_name,
    &func_le_get_name,
    &func_ble_open,
    &func_ble_close,
    &func_ble_set_adv_param,
    &func_ble_set_adv_data,
    &func_ble_enable_adv,
    &func_ble_disable_adv,
    &func_ble_scan_start,
    &func_ble_scan_stop,
    &func_ble_connect,
    &func_ble_disconnect,
    &func_ble_indicate,
    &func_ble_notify,
    &func_ble_read_request,
    &func_ble_write_request,
    &func_ble_write_command,
    &func_ble_pairing_request,
    &func_ble_enable_pairing_request,
    &func_ble_get_pairing_info,
    &func_ble_remove_pairing_info,
    &func_ble_clear_pairing_info,
    &func_ble_set_white_list,
    &func_ble_clear_white_list,
    &func_ble_read_rssi,
    NULL  // MUST end by NULL
};

const menu_list_t ble_menu =
{
    .menu_name = "BLE",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = ble_menu_sub,
    .parent = &root_menu,
};

int g_data_size;
int g_att_handle;
int g_write_data_type = 0;


#define CHARACTERISTIC_VALUE_LENGTH             512

static char custom_characteristic_value[CHARACTERISTIC_VALUE_LENGTH]     = {"hello world"};
static SC_ATT_CHARA_CCB_EX_T   custom_client_characteristic_config_value        = {0};

SC_UUID_16_T sc_ble_uuid_primary_service =
{
    .type.type = LE_UUID_TYPE_16,
    .value = LE_ATT_UUID_PRIMARY
};

SC_UUID_16_T sc_ble_uuid_client_charc_config =
{
    .type.type = LE_UUID_TYPE_16,
    .value = LE_ATT_UUID_CLIENT_CHARC_CONFIG
};

SC_UUID_16_T sc_ble_uuid_characteristic =
{
    .type.type = LE_UUID_TYPE_16,
    .value = LE_ATT_UUID_CHARC
};

static SC_UUID_16_T custom_service_declare =
{
    .type.type = LE_UUID_TYPE_16,
    .value = 0xFFF0
};

static SC_BLE_CHARACTERISTIC_16_T custom_characteristic_declare =
{
    .uuid = 0xFFF1,
    .properties = LE_ATT_CHARC_PROP_READ | LE_ATT_CHARC_PROP_WWP | LE_ATT_CHARC_PROP_NOTIFY | LE_ATT_CHARC_PROP_INDICATE
};

static SC_UUID_16_T custom_characteristic_attribute_value =
{
    .type.type = LE_UUID_TYPE_16,
    .value = 0xFFF1
};

static int custom_characteristic_write_cb(void *param)
{
    SC_BLE_RW_T *rw = (SC_BLE_RW_T *)param;

    LOG_INFO("%s: handle %04x, length %d, offset %d\r\n", __func__, rw->handle, rw->length, rw->offset);

    if((rw->offset >= CHARACTERISTIC_VALUE_LENGTH) || (rw->length > CHARACTERISTIC_VALUE_LENGTH))
    {
        LOG_INFO("Offset specified was past the end of the attribute.\r\n");
        return SC_BLE_ERR_INVALID_OFFSET;
    }

    memset(custom_characteristic_value, 0, CHARACTERISTIC_VALUE_LENGTH);
    memcpy(custom_characteristic_value, rw->data, rw->length);
    LOG_INFO("%s: receive data:", __func__);
    for(int i=0; i<rw->length; i++)
    {
        LOG_INFO("%02X", custom_characteristic_value[i]);
    }

    return SC_BLE_ERR_OK;
}

static int custom_characteristic_read_cb(void *param)
{
    SC_BLE_RW_T *rw = (SC_BLE_RW_T *)param;

    LOG_INFO("%s: handle %d, length %d, offset %d\r\n", __func__, rw->handle, rw->length, rw->offset);

    if((rw->offset >= CHARACTERISTIC_VALUE_LENGTH) || (rw->length > CHARACTERISTIC_VALUE_LENGTH))
    {
        LOG_INFO("Offset specified was past the end of the attribute.\r\n");
        return SC_BLE_ERR_INVALID_OFFSET;
    }

    unsigned char *p = (unsigned char *)sAPI_Malloc(CHARACTERISTIC_VALUE_LENGTH);
    if(p == NULL)
    {
        LOG_INFO("%s: malloc fail\r\n", __func__);
        return 0x80;
    }

    memset(p, 0, CHARACTERISTIC_VALUE_LENGTH);
    memcpy(p, custom_characteristic_value, strlen(custom_characteristic_value));
    rw->data = p;

    return strlen(custom_characteristic_value);
}


static int custom_client_char_config_write_cb(void *param)
{
    SC_BLE_RW_T *rw = (SC_BLE_RW_T *)param;

    LOG_INFO("%s: handle %d, length %d, offset %d\r\n", __func__, rw->handle, rw->length, rw->offset);

    if(rw->data[0] > 0x03)//bit1:bit0 = indication:notification
    {
        LOG_INFO("%s: written data illegal.\r\n", __func__);
        return 0x81;
    }

    LOG_INFO("%s: receive data:0x%02X", __func__, rw->data[0]);

    return SC_BLE_ERR_OK;
}

static SC_BLE_SERVICE_T custom_attrs[] =
{
    //custom service declare
    SC_BLE_DECLARE_PRIMARY_SERVICE(sc_ble_uuid_primary_service, custom_service_declare, LE_UUID_TYPE_16),

    //custom characteristic declare
    SC_BLE_DECLARE_CHARACTERISTIC(sc_ble_uuid_characteristic, custom_characteristic_declare, custom_characteristic_attribute_value, LE_ATT_PM_READABLE|LE_ATT_PM_WRITEABLE,
                              custom_characteristic_write_cb, custom_characteristic_read_cb,
                              custom_characteristic_value, sizeof(custom_characteristic_value)),

    //custom client characteristic config declare
    SC_BLE_DECLARE_CLINET_CHRAC_CONFIG(sc_ble_uuid_client_charc_config, custom_client_characteristic_config_value, custom_client_char_config_write_cb),
};

extern void demo_bt_send_buf(char *buffer,int len);

static void custom_ble_handle_event(void *argv)
{
    char buffer[200] = {0};
    SC_BLE_EVENT_MSG_T *msg = (SC_BLE_EVENT_MSG_T *)argv;

    LOG_INFO("%s: event_type:%d event_id:%d", __func__, msg->event_type, msg->event_id);
    switch(msg->event_type)
    {
        case EVENT_TYPE_COMMON:
            switch(msg->event_id)
            {
                case COMMON_POWERUP_FAILED:
                    LOG_INFO("COMMON_POWERUP_FAILED.");
                    sprintf(buffer,"COMMON_POWERUP_FAILED");
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                case COMMON_POWERUP_COMPLETE:
                    LOG_INFO("COMMON_POWERUP_COMPLETE.");
                    sprintf(buffer,"COMMON_POWERUP_COMPLETE");
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                case COMMON_SLAVE_LE_BOND_COMPLETE:
                    LOG_INFO("COMMON_SLAVE_LE_BOND_COMPLETE.");
                    sprintf(buffer,"COMMON_SLAVE_LE_BOND_COMPLETE");
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                case COMMON_RSSI:
                {
                    signed char *rssi = (signed char *)msg->payload;
                    LOG_INFO("COMMON_RSSI %d.",*rssi);
                    sprintf(buffer,"COMMON_RSSI rssi:%d.",*rssi);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                }
                default:
                    sprintf(buffer,"EVENT_TYPE_COMMON_ID_%d", msg->event_id);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
            }
            break;
        case EVENT_TYPE_LE:
            switch(msg->event_id)
            {
                case BLE_SCAN_EVENT:
                {
                    SC_BLE_SCAN_EVENT_T *scan = (SC_BLE_SCAN_EVENT_T *)(msg->payload);
                    int i = 0,j = 0;
                    char advData[63] = {0};

                    for(i = 0; i < scan->length; i ++)
                    {
                        j += sprintf(advData + j, "%02x",scan->data[i]);
                    }
                    LOG_INFO("remote device addr:%02x:%02x:%02x:%02x:%02x:%02x,advData[%s],rssi:%d",
                                    scan->address.bytes[5],scan->address.bytes[4],scan->address.bytes[3],
                                    scan->address.bytes[2],scan->address.bytes[1],scan->address.bytes[0],
                                    advData, scan->rssi);
                    sprintf(buffer,"BLE_SCAN_EVENT addr:%02x:%02x:%02x:%02x:%02x:%02x,advData[%s],rssi:%d",
                                    scan->address.bytes[5],scan->address.bytes[4],scan->address.bytes[3],
                                    scan->address.bytes[2],scan->address.bytes[1],scan->address.bytes[0],
                                    advData, scan->rssi);
                    demo_bt_send_buf(buffer,strlen(buffer));
                }
                break;

                case BLE_CONNECT_EVENT:
                {
                    SC_BLE_CONNECT_EVENT_T *connect = (SC_BLE_CONNECT_EVENT_T *)(msg->payload);
                    LOG_INFO("BLE_CONNECT_EVENT .");
                    sprintf(buffer,"BLE_CONNECT_EVENT addr:%02x:%02x:%02x:%02x:%02x:%02x,type[%d],role:%d",
                                    connect->address.bytes[5],connect->address.bytes[4],connect->address.bytes[3],
                                    connect->address.bytes[2],connect->address.bytes[1],connect->address.bytes[0],
                                    connect->address_type, connect->role);
                    demo_bt_send_buf(buffer,strlen(buffer));
                }
                break;

                case BLE_DIS_CONNECT_EVENT:
                    LOG_INFO("BLE_DIS_CONNECT_EVENT.");
                    sprintf(buffer,"BLE_DIS_CONNECT_EVENT");
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;

                case BLE_MTU_EXCHANGED_EVENT:
                    LOG_INFO("BLE_MTU_EXCHANGED_EVENT.");
                    sprintf(buffer,"BLE_MTU_EXCHANGED_EVENT");
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;

                case BLE_INDICATE_EVENT:
                    LOG_INFO("BLE_INDICATE_EVENT.");
                    sprintf(buffer,"BLE_INDICATE_EVENT");
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;

                case BLE_CLIENT_MTU_EXCHANGED_EVENT:
                    LOG_INFO("BLE_CLIENT_MTU_EXCHANGED_EVENT.");
                    sprintf(buffer,"BLE_CLIENT_MTU_EXCHANGED_EVENT");
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;

                case BLE_CLIENT_READ_BY_GROUP_TYPE_RSP_EVENT:
                    LOG_INFO("BLE_CLIENT_READ_BY_GROUP_TYPE_RSP_EVENT.");
                    sprintf(buffer,"BLE_CLIENT_READ_BY_GROUP_TYPE_RSP_EVENT");
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;

                case BLE_CLIENT_WRITE_RSP:
                    LOG_INFO("BLE_CLIENT_WRITE_RSP.");
                    sprintf(buffer,"BLE_CLIENT_WRITE_RSP");
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;

                default:
                    LOG_INFO("%s: default.", __func__);
                    sprintf(buffer,"EVENT_TYPE_LE_ID_%d", msg->event_id);
                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
            }
            break;
        default:
        {
            sprintf(buffer,"EVENT_TYPE_%d_ID_%d",msg->event_type, msg->event_id);
            demo_bt_send_buf(buffer,strlen(buffer));
            break;
        }
    }
}

static ret_msg_t ble_set_addr_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *addrStr = NULL;
    //char addr[6] = {0};
    int j = 6;
    SC_BLE_ADDR_T addr = {0};

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
        addr.bytes[--j] = strtoul(tmp,NULL,16);
    }
    LOG_INFO("addr %02X:%02X:%02X:%02X:%02X:%02X",
                addr.bytes[0],addr.bytes[1],addr.bytes[2],
                addr.bytes[3],addr.bytes[4],addr.bytes[5]);
    ret.errcode = sAPI_BleSetAddress(&addr);
    if (0 != ret.errcode)
    {
        LOG_ERROR("ble set address failed");
        ret.msg = "ble set address failed";
        return ret;
    }

    return ret;
}

static ret_msg_t ble_get_addr_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    arg_t out_arg[1] = {0};
    int result = 0;
    //unsigned char addr[6] = {0};
    SC_BLE_ADDR_T addr = {0};
    char buffer[32] = {0};

    result = sAPI_BleGetAddress(&addr);
    if(result != 0)
    {
        LOG_ERROR("ble get address failed");
        ret.msg = "ble get address failed";
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

static ret_msg_t ble_set_name_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *nameStr = NULL;
    char name[255] = {0};

    pl_demo_get_data(&nameStr, argv, argn, ARGBT_NAME);
    if (!nameStr)
    {
        LOG_ERROR("name invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "name invalid";
        return ret;
    }
    strcpy(name,nameStr);
    LOG_INFO("nameStr %s",name);
    ret.errcode = sAPI_BleSetDeviceName(name);
    if (0 != ret.errcode)
    {
        LOG_ERROR("bt set name failed");
        ret.msg = "bt set name failed";
        return ret;
    }

    return ret;
}

static ret_msg_t ble_get_name_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    arg_t out_arg[1] = {0};
    int result = 0;
    char name[255] = {0};

    strcpy(name,sAPI_BleGetDeviceName());
    if(name == NULL)
    {
        LOG_ERROR("bt get name failed");
        ret.msg = "bt get name failed";
        ret.errcode = result;
        return ret;
    }
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARGBT_NAME;
    out_arg[0].value = pl_demo_make_value(0, name, strlen(name));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);

    return ret;
}


static ret_msg_t ble_open_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0;

    sAPI_BleRegisterEventHandleEx(custom_ble_handle_event);
    result = sAPI_BleOpen(NULL,0);
    if(result == 0)
    {
        LOG_INFO("open bt device success.");
        ret.msg = "open bt device success.";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("open bt device fail. error code = %d", result);
        ret.msg = "open bt device fail";
    }

    return ret;
}

static ret_msg_t ble_close_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    sAPI_BleClose();

    LOG_INFO("close ble success.");
    ret.msg = "close ble success.";
    return ret;
}

static ret_msg_t ble_set_param_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0;
    const char *addrStr = NULL;
    SC_BLE_ADV_PARAM_T param = {0};
    int j = 6;
    int interval_min,interval_max,adv_type,own_addr_type,peer_addr_type,filter;

    pl_demo_get_value(&interval_min, argv, argn, ARGLE_ADV_INTERVAL_MIN);
    pl_demo_get_value(&interval_max, argv, argn, ARGLE_ADV_INTERVAL_MAX);
    pl_demo_get_value(&adv_type, argv, argn, ARGLE_ADV_TYPE);
    pl_demo_get_value(&own_addr_type, argv, argn, ARGLE_ADDR_TYPE);
    pl_demo_get_value(&peer_addr_type, argv, argn, ARGLE_ADDR_TYPE);
    pl_demo_get_data(&addrStr, argv, argn, ARGBT_ADDR);
    pl_demo_get_value(&filter, argv, argn, ARGLE_ADV_FILTER);

    param.interval_min = interval_min;
    param.interval_max = interval_max;
    param.advertising_type = adv_type;
    param.own_address_type = own_addr_type;
    param.peer_address_type = peer_addr_type;
    param.filter = filter;
    for(int i = 0;i < 12;i += 2)
    {
        char tmp[4] = {0};
        memcpy(tmp,&addrStr[i],2);
        param.peer_address.bytes[--j] = strtoul(tmp,NULL,16);
    }
    LOG_INFO("interval_min[%d],interval_max[%d] adv_type[%d] own_addr_type[%d] peer_addr_type[%d] addr(%02X:%02X:%02X:%02X:%02X:%02X) filter[%d]",
        param.interval_min,param.interval_min, param.advertising_type, param.own_address_type, param.peer_address_type,
        param.peer_address.bytes[0],param.peer_address.bytes[1],param.peer_address.bytes[2],
        param.peer_address.bytes[3],param.peer_address.bytes[4],param.peer_address.bytes[5],
        param.filter);
    result = sAPI_BleSetAdvParam(&param);
    if(result != 0)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("set adv parameters failed. error code = %d", result);
        ret.msg = "set adv parameters failed";
    }
    else
    {
        LOG_INFO("set adv parameters success");
        ret.msg = "set adv parameters success";
    }
    return ret;
}

static ret_msg_t ble_set_adv_data_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0;
    const char *adv_data = NULL;
    unsigned char buf[31] = {0};
    int j = 0;
    const char *resp_data = NULL;
    arg_t out_arg[1] = {0};

    pl_demo_get_data(&adv_data, argv, argn, ARGLE_ADV_DATA);
    if(adv_data != NULL)
    {
        for(int i = 0;i <strlen(adv_data) ; i+=2)
        {
            buf[j ++] = adv_data[i] << 1|adv_data[i+1];
        }
    }

    result = sAPI_BleSetAdvData(buf, j);
    if(result != 0)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("set adv data failed. error code = %d", result);
        ret.msg = "set adv data failed";
    }
    else
    {
        LOG_INFO("set adv data success");
        ret.msg = "set adv data success";
    }
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARGBT_RESULT;
    out_arg[0].value = pl_demo_make_value(0, ret.msg, strlen(ret.msg));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);

    pl_demo_get_data(&resp_data, argv, argn, ARGLE_SCAN_RESP);
    if(resp_data != NULL)
    {
        j = 0;
        memset(buf,0,31);
        for(int i = 0;i <strlen(resp_data) ; i+=2)
        {
            buf[j ++] = resp_data[i] << 1|resp_data[i+1];
        }
    }
    result = sAPI_BleSetScanResponseData(buf, j);
    if(result != 0)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("set scan_resp failed. error code = %d", result);
        ret.msg = "set scan_resp failed";
    }
    else
    {
        LOG_INFO("set scan_resp success");
        ret.msg = "set scan_resp success";
    }
    return ret;
}
static ret_msg_t ble_set_scan_resp_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0;
    const char *resp_data = NULL;
    unsigned char buf[31] = {0};
    int j = 0;

    pl_demo_get_data(&resp_data, argv, argn, ARGLE_SCAN_RESP);

    if(resp_data != NULL)
    {
        for(int i = 0;i <strlen(resp_data) ; i+=2)
        {
            buf[j ++] = resp_data[i] << 1|resp_data[i+1];
        }
    }
    result = sAPI_BleSetScanResponseData(buf, j);
    if(result != 0)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("set scan_resp failed. error code = %d", result);
        ret.msg = "set scan_resp failed";
    }
    else
    {
        LOG_INFO("set scan_resp success");
        ret.msg = "set scan_resp success";
    }
    return ret;
}

static ret_msg_t ble_enable_adv_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0;

    result = sAPI_BleRegisterService(custom_attrs, sizeof(custom_attrs) / sizeof(SC_BLE_SERVICE_T));
    result = sAPI_BleEnableAdv();
    if(result == 0)
    {
        LOG_INFO("start to adv success.");
        ret.msg = "start to adv success.";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("start to adv fail. error code = %d", result);
        ret.msg = "start to adv fail";
    }

    return ret;
}

static ret_msg_t ble_disable_adv_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0;

    result = sAPI_BleDisableAdv();
    if(result == 0)
    {
        LOG_INFO("disable adv success.");
        ret.msg = "disable adv success.";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("disable adv fail. error code = %d", result);
        ret.msg = "disable adv fail";
    }

    return ret;
}

static ret_msg_t ble_scan_start_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0;
    int mode = 0, interval = 0, window = 0, addr_type = 0;

    pl_demo_get_value(&mode, argv, argn, ARGLE_SCAN_MODE);
    pl_demo_get_value(&interval, argv, argn, ARGLE_SCAN_INTERVAL);
    pl_demo_get_value(&window, argv, argn, ARGLE_SCAN_WINDOW);
    pl_demo_get_value(&addr_type, argv, argn, ARGLE_ADDR_TYPE);

    LOG_INFO("mode[%d] interval[%d] window[%d] addr_type[%d]", mode,interval,window, addr_type);
    result = sAPI_BleScan(mode, interval, window, addr_type);
    if(result == 0)
    {
        LOG_INFO("start to scan success.");
        ret.msg = "start to scan success.";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("start to scan fail. error code = %d", result);
        ret.msg = "start to scan fail";
    }
    return ret;
}

static ret_msg_t ble_scan_stop_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0;

    result = sAPI_BleScanStop();
    if(result == 0)
    {
        LOG_INFO("stop to scan success.");
        ret.msg = "stop to scan success.";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("stop to scan fail. error code = %d", result);
        ret.msg = "stop to scan fail";
    }
    return ret;

}

static ret_msg_t ble_connect_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0;
    const char *addrStr = NULL;
    SC_BLE_ADDR_T addr = {0};
    int addr_type = 0;
    int j = 6;

    pl_demo_get_data(&addrStr, argv, argn, ARGBT_ADDR);
    pl_demo_get_value(&addr_type, argv, argn, ARGLE_ADDR_TYPE);
    for(int i = 0;i < 12;i += 2)
    {
        char tmp[4] = {0};
        memcpy(tmp,&addrStr[i],2);
        addr.bytes[--j] = strtoul(tmp,NULL,16);
    }

    result = sAPI_BleConnect(&addr, addr_type);
    if(result != 0)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("connect to device failed. error code = %d", result);
        ret.msg = "connect to device failed";
    }
    else
    {
        LOG_INFO("start to connect");
        ret.msg = "start to connect";
    }
    return ret;

}

static ret_msg_t ble_disconnect_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0;

    result = sAPI_BleDisconnect();
    if(result != 0)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("disconnect device failed. error code = %d", result);
        ret.msg = "disconnect device failed";
    }
    else
    {
        LOG_INFO("start to disconnect");
        ret.msg = "start to disconnect";
    }
    return ret;
}

static int ble_indicate_data_size(void)
{
    return g_data_size;
}
static ret_msg_t ble_indicate_data_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms))
{
    ret_msg_t ret = {0};
    int read_size = 0;
    int process_size = 0;
    int remain_size = size;
    char *buf = sal_malloc(g_data_size + 1);
    int result = 0;
    memset(buf, 0 , g_data_size + 1);

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

        remain_size -= read_size;
    }
    result = sAPI_BleIndicate(g_att_handle, (const void *)buf, g_data_size);
    if (result < 0)
    {
        LOG_ERROR("fail to indicate");
        LOG_ERROR("read size:%d, cache size:%d", read_size, process_size);
        ret.errcode = process_size < 0 ? process_size : ERRCODE_DEFAULT;
        ret.msg = "raw data cache fail";
    }
    else
    {
        LOG_INFO("indicate data");
        ret.msg = "indicate data";
    }
EXIT:
    sal_free(buf);
    g_data_size = 0;
    g_att_handle = 0;

    return ret;
}

static ret_msg_t ble_indicate_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int att_handle = 0, data_size = 0;

    pl_demo_get_value(&att_handle, argv, argn, ARGLE_ATT_HANDLE);
    pl_demo_get_value(&data_size, argv, argn, ARGLE_DATA_LEN);

    if(att_handle > 0 && data_size > 0)
    {
        g_att_handle = att_handle;
        g_data_size = data_size;
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("invalid param");
        ret.msg = "invalid paramd";
        return ret;
    }
    return ret;
}

static int ble_notify_data_size(void)
{
    return g_data_size;
}
static ret_msg_t ble_notify_data_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms))
{
    ret_msg_t ret = {0};
    int read_size = 0;
    int process_size = 0;
    int remain_size = size;
    char *buf = sal_malloc(g_data_size + 1);
    int result = 0;
    memset(buf, 0 , g_data_size + 1);

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

        remain_size -= read_size;
    }
    result = sAPI_BleNotify(g_att_handle, (const void *)buf, g_data_size);
    if (result < 0)
    {
        LOG_ERROR("fail to notify");
        LOG_ERROR("read size:%d, cache size:%d", read_size, process_size);
        ret.errcode = process_size < 0 ? process_size : ERRCODE_DEFAULT;
        ret.msg = "raw data cache fail";
    }
    else
    {
        LOG_INFO("notify data");
        ret.msg = "notify data";
    }
EXIT:
    sal_free(buf);
    g_data_size = 0;
    g_att_handle = 0;

    return ret;
}

static ret_msg_t ble_notify_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int att_handle = 0, data_size = 0;

    pl_demo_get_value(&att_handle, argv, argn, ARGLE_ATT_HANDLE);
    pl_demo_get_value(&data_size, argv, argn, ARGLE_DATA_LEN);

    if(att_handle > 0 && data_size > 0)
    {
        g_att_handle = att_handle;
        g_data_size = data_size;
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("invalid param");
        ret.msg = "invalid paramd";
        return ret;
    }
    return ret;
}

static ret_msg_t ble_read_request_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0;
    int att_handle = 0;
    pl_demo_get_value(&att_handle, argv, argn, ARGLE_ATT_HANDLE);

    result = sAPI_BleReadRequest(att_handle);
    if(result == 0)
    {
        LOG_INFO("request to read data with (%d).",att_handle);
        ret.msg = "request to read data";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("request to read data fail. error code = %d", result);
        ret.msg = "request to read data";
    }
    return ret;
}

static ret_msg_t ble_write_request_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int att_handle = 0, data_size = 0;

    pl_demo_get_value(&att_handle, argv, argn, ARGLE_ATT_HANDLE);
    pl_demo_get_value(&data_size, argv, argn, ARGLE_DATA_LEN);

    if(att_handle > 0 && data_size > 0)
    {
        g_att_handle = att_handle;
        g_data_size = data_size;
        g_write_data_type = 0;
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("invalid param");
        ret.msg = "invalid paramd";
        return ret;
    }
    return ret;
}

static int ble_write_data_size(void)
{
    return g_data_size;
}

static ret_msg_t ble_write_data_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms))
{
    ret_msg_t ret = {0};
    int read_size = 0;
    int process_size = 0;
    int remain_size = size;
    char *buf = sal_malloc(g_data_size + 1);
    int result = 0;
    memset(buf, 0 , g_data_size + 1);

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

        remain_size -= read_size;
    }
    if(g_write_data_type)
        result = sAPI_BleWriteCommand(g_att_handle, (const void *)buf, g_data_size);
    else
        result = sAPI_BleWriteRequest(g_att_handle, (const void *)buf, g_data_size);
    if (result < 0)
    {
        LOG_ERROR("fail to write");
        LOG_ERROR("read size:%d, cache size:%d", read_size, process_size);
        ret.errcode = process_size < 0 ? process_size : ERRCODE_DEFAULT;
        ret.msg = "raw data cache fail";
    }
    else
    {
        LOG_INFO("write data");
        ret.msg = "write data";
    }
EXIT:
    sal_free(buf);
    g_data_size = 0;
    g_att_handle = 0;
    g_write_data_type = 0;

    return ret;
}

static ret_msg_t ble_write_command_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int att_handle = 0, data_size = 0;

    pl_demo_get_value(&att_handle, argv, argn, ARGLE_ATT_HANDLE);
    pl_demo_get_value(&data_size, argv, argn, ARGLE_DATA_LEN);

    if(att_handle > 0 && data_size > 0)
    {
        g_att_handle = att_handle;
        g_data_size = data_size;
        g_write_data_type = 1;
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("invalid param");
        ret.msg = "invalid paramd";
    }
    return ret;
}

static ret_msg_t ble_pairing_request_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *addrStr = NULL;
    int j = 6;
    SC_BLE_ADDR_T addr = {0};

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
        addr.bytes[--j] = strtoul(tmp,NULL,16);
    }
    LOG_INFO("addr %02X:%02X:%02X:%02X:%02X:%02X",
                addr.bytes[0],addr.bytes[1],addr.bytes[2],
                addr.bytes[3],addr.bytes[4],addr.bytes[5]);
    ret.errcode = sAPI_BleSmpIniate(addr);
    if (0 != ret.errcode)
    {
        LOG_ERROR("ble fail to pair request");
        ret.msg = "ble fail to pair request";
        return ret;
    }

    return ret;
}

static ret_msg_t ble_enable_pairing_request_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int enable = 0;

    pl_demo_get_value(&enable, argv, argn, ARGLE_PAIR_ENABLE);

    ret.errcode = sAPI_BleSetPairEnable(enable);
    if (0 != ret.errcode)
    {
        LOG_ERROR("ble fail to set pairing request");
        ret.msg = "ble fail to set pairing request";
        return ret;
    }
    return ret;
}

static ret_msg_t ble_get_pairing_info_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_BLE_DEVICE_INFO_RECORD info = {0};
    arg_t out_arg[1] = {0};
    char buffer[128] = {0};

    for(int i = 0;i < 10;i ++)
    {
        if(sAPI_BleGetPairInfo(i,&info) == 0)
        {
            LOG_INFO("pair info[%d] %2X:%2X:%2X:%2X:%2X:%2X",i,
                 info.address[5],info.address[4],info.address[3],
                 info.address[2],info.address[1],info.address[0]);
            sprintf(buffer,"pair info[%d] %2X:%2X:%2X:%2X:%2X:%2X",i,
                 info.address[5],info.address[4],info.address[3],
                 info.address[2],info.address[1],info.address[0]);
            pl_demo_release_value(out_arg[0].value);
            out_arg[0].arg_name = "Le_record_info";
            out_arg[0].type = TYPE_STR;
            out_arg[0].value = pl_demo_make_value(0, buffer, strlen(buffer));
            pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
            pl_demo_release_value(out_arg[0].value);
        }
    }
    return ret;
}

static ret_msg_t ble_remove_pairing_info_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int index = 0;

    pl_demo_get_value(&index, argv, argn, ARGLE_PAIR_INDEX);
    LOG_INFO("index[%d]",index);
    sAPI_BleDeleteSinglePairInfo(index);
    return ret;
}

static ret_msg_t ble_clear_pairing_info_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    //SC_BLE_DEVICE_INFO_RECORD info = {0};
    for(int i = 0;i < 10;i ++)
    {
        sAPI_BleDeleteSinglePairInfo(i);
    }

    return ret;
}

static ret_msg_t ble_set_white_list_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *addrStr = NULL;
    int j = 6;
    int addr_type;
    SC_BLE_ADDR_T addr = {0};

    pl_demo_get_data(&addrStr, argv, argn, ARGBT_ADDR);
    pl_demo_get_value(&addr_type, argv, argn, ARGLE_ADDR_TYPE);
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
        addr.bytes[--j] = strtoul(tmp,NULL,16);
    }
    LOG_INFO("addr %02X:%02X:%02X:%02X:%02X:%02X",
           addr.bytes[0],addr.bytes[1],addr.bytes[2],
           addr.bytes[3],addr.bytes[4],addr.bytes[5]);
    ret.errcode = sAPI_BleSetWhiteList(addr,addr_type);
    if (0 != ret.errcode)
    {
        LOG_ERROR("ble fail to set white list");
        ret.msg = "ble fail to set white list";
        return ret;
    }

    return ret;
}

static ret_msg_t ble_clear_white_list_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    ret.errcode = sAPI_BleClearWhilteList();
    if (0 != ret.errcode)
    {
        LOG_ERROR("ble fail to clear white list");
        ret.msg = "ble fail to clear white list";
        return ret;
    }
    return ret;
}

static ret_msg_t ble_rssi_read_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = sAPI_BleReadRssi();
    if(result != 0)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("request RSSI failed. error code = %d", result);
        ret.msg = "request RSSI failed";
    }
    else
    {
        LOG_INFO("start to get rssi");
        ret.msg = "start to get rssi";
    }
    return ret;
}



