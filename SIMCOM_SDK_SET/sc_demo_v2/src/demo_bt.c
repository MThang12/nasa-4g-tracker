#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "demo_bt.h"
#include "simcom_bt.h"
#include "simcom_os.h"


#define LOG_ERROR(...) sal_log_error("[DEMO-BT] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-BT] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-BT] " __VA_ARGS__)
#define BT_SPP_MAX_FRAME_SIZE 200

static ret_msg_t bt_set_addr_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_get_addr_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_set_name_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_get_name_cmd_proc(op_t op, arg_t *argv, int argn);
//static ret_msg_t bt_get_ioCap_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_open_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_close_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_inquiry_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_inquiry_cancel_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_pair_request_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_pair_response_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_unpair_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_get_pairing_info_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_spp_connect_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_spp_disconnect_cmd_proc(op_t op, arg_t *argv, int argn);
//static ret_msg_t spp_send_data_init(void);
//static ret_msg_t spp_send_data_uninit(void);
static int bt_spp_need_size(void);
static ret_msg_t bt_spp_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms));
static ret_msg_t bt_spp_send_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t bt_rssi_read_cmd_proc(op_t op, arg_t *argv, int argn);

arg_t set_addr_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGBT_ADDR,
        .msg = "bt address,like XXXXXXXXXXXX",
    }
};

const menu_list_t func_set_addr =
{
    .menu_name = "set bt address",
    .menu_type = TYPE_FUNC,
    .parent = &bt_menu,
    .function = {
        .argv = set_addr_args,
        .argn = sizeof(set_addr_args) / sizeof(arg_t),
        .methods.cmd_handler = bt_set_addr_cmd_proc,
    },
};

const menu_list_t func_get_addr =
{
    .menu_name = "get bt address",
    .menu_type = TYPE_FUNC,
    .parent = &bt_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = bt_get_addr_cmd_proc,
    },
};

arg_t set_name_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGBT_NAME,
        .msg = "bt name string",
    }
};

const menu_list_t func_set_name =
{
    .menu_name = "set bt name",
    .menu_type = TYPE_FUNC,
    .parent = &bt_menu,
    .function = {
        .argv = set_name_args,
        .argn = sizeof(set_name_args) / sizeof(arg_t),
        .methods.cmd_handler = bt_set_name_cmd_proc,
    },
};

const menu_list_t func_get_name =
{
    .menu_name = "get bt name",
    .menu_type = TYPE_FUNC,
    .parent = &bt_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = bt_get_name_cmd_proc,
    },
};

#if 0
const menu_list_t func_get_ioCap =
{
    .menu_name = "get bt io_capbility",
    .menu_type = TYPE_FUNC,
    .parent = &bt_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = bt_get_ioCap_cmd_proc,
    },
};
#endif
arg_t open_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGBT_IOCAP,
        .msg = "Support DISPLAY_ONLY,DISPLAY_AND_YES_OR_NO,KEYBOARD_ONLY,NO_DISPALY_AND_NO_KEYBOARD",
    }
};

const menu_list_t func_bt_open =
{
    .menu_name = "open bluetooth",
    .menu_type = TYPE_FUNC,
    .parent = &bt_menu,
    .function = {
        .argv = open_args,
        .argn = 1,
        .methods.cmd_handler = bt_open_cmd_proc,
    },
};

const menu_list_t func_bt_close =
{
    .menu_name = "close bluetooth",
    .menu_type = TYPE_FUNC,
    .parent = &bt_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = bt_close_cmd_proc,
    },
};

arg_t inquiry_args[2] =
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
    .parent = &bt_menu,
    .function = {
        .argv = inquiry_args,
        .argn = sizeof(inquiry_args) / sizeof(arg_t),
        .methods.cmd_handler = bt_inquiry_cmd_proc,
    },
};

const menu_list_t func_bt_inquiry_cancel =
{
    .menu_name = "cancel to inquiry",
    .menu_type = TYPE_FUNC,
    .parent = &bt_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = bt_inquiry_cancel_cmd_proc,
    },
};

arg_t pair_req_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGBT_ADDR,
        .msg = "bt address,like XXXXXXXXXXXX",
    },
};

const menu_list_t func_bt_pair_request =
{
    .menu_name = "bt pairing request",
    .menu_type = TYPE_FUNC,
    .parent = &bt_menu,
    .function = {
        .argv = pair_req_args,
        .argn = sizeof(pair_req_args) / sizeof(arg_t),
        .methods.cmd_handler = bt_pair_request_cmd_proc,
    },
};

arg_t pair_resp_args[4] =
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
        .type = TYPE_INT,
        .arg_name = ARGBT_VERIFICATION,
        .msg = "bt passkey",
    }
};

const menu_list_t func_bt_pair_response =
{
    .menu_name = "bt pairing response",
    .menu_type = TYPE_FUNC,
    .parent = &bt_menu,
    .function = {
        .argv = pair_resp_args,
        .argn = sizeof(pair_resp_args) / sizeof(arg_t),
        .methods.cmd_handler = bt_pair_response_cmd_proc,
    },
};

arg_t unpair_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGBT_ADDR,
        .msg = "bt address,like XXXXXXXXXXXX",
    }
};

const menu_list_t func_bt_unpair =
{
    .menu_name = "bt unpair",
    .menu_type = TYPE_FUNC,
    .parent = &bt_menu,
    .function = {
        .argv = unpair_args,
        .argn = sizeof(unpair_args) / sizeof(arg_t),
        .methods.cmd_handler = bt_unpair_cmd_proc,
    },
};

const menu_list_t func_bt_get_pairing_info =
{
    .menu_name = "bt get pairing information",
    .menu_type = TYPE_FUNC,
    .parent = &bt_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = bt_get_pairing_info_cmd_proc,
    },
};

const menu_list_t func_bt_spp_connect =
{
    .menu_name = "bt spp connect",
    .menu_type = TYPE_FUNC,
    .parent = &bt_menu,
    .function = {
        .argv = unpair_args,
        .argn = sizeof(unpair_args) / sizeof(arg_t),
        .methods.cmd_handler = bt_spp_connect_cmd_proc,
    },
};

arg_t spp_disc_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGBT_SPP_PORT,
        .msg = "bt spp port from connection event",
    }
};

const menu_list_t func_bt_spp_disconnect =
{
    .menu_name = "bt spp connect",
    .menu_type = TYPE_FUNC,
    .parent = &bt_menu,
    .function = {
        .argv = spp_disc_args,
        .argn = sizeof(spp_disc_args)/sizeof(arg_t),
        .methods.cmd_handler = bt_spp_disconnect_cmd_proc,
    },
};

arg_t spp_send_args[2] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGBT_SPP_DATA_LEN,
        .msg = "data length",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGBT_SPP_PORT,
        .msg = "spp port from connection event",
    }
};

const menu_list_t func_bt_spp_send =
{
    .menu_name = "bt spp send data",
    .menu_type = TYPE_FUNC,
    .parent = &bt_menu,
    .function = {
        .argv = spp_send_args,
        .argn = sizeof(spp_send_args)/sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = bt_spp_send_cmd_proc,
        .methods.raw_handler = bt_spp_raw_proc,
        .methods.needRAWData = bt_spp_need_size,
    },
};

const menu_list_t func_bt_read_rssi =
{
    .menu_name = "bt read rssi",
    .menu_type = TYPE_FUNC,
    .parent = &bt_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = bt_rssi_read_cmd_proc,
    },
};

const menu_list_t *bt_menu_sub[] =
{
    &func_set_addr,
    &func_get_addr,
    &func_set_name,
    &func_get_name,
    //&func_get_ioCap,
    &func_bt_open,
    &func_bt_close,
    &func_bt_inquiry,
    &func_bt_inquiry_cancel,
    &func_bt_pair_request,
    &func_bt_pair_response,
    &func_bt_unpair,
    &func_bt_get_pairing_info,
    &func_bt_spp_connect,
    &func_bt_spp_disconnect,
    &func_bt_spp_send,
    &func_bt_read_rssi,
    NULL  // MUST end by NULL
};

const menu_list_t bt_menu =
{
    .menu_name = "Classic BT",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = bt_menu_sub,
    .parent = &root_menu,
};

static unsigned char bt_spp_port            = 0;
static unsigned short bt_spp_max_frame_size = 0;
static const bt_pair_mode_enum g_bt_pairing_mode[5][5] =
{
    {JUST_WORK_MODE,     JUST_WORK_MODE,          PASSKEY_ENTRY_MODE, JUST_WORK_MODE, PASSKEY_ENTRY_MODE},
    {JUST_WORK_MODE,     NUMERIC_COMPARISON_MODE, PASSKEY_ENTRY_MODE, JUST_WORK_MODE, NUMERIC_COMPARISON_MODE},
    {PASSKEY_ENTRY_MODE, PASSKEY_ENTRY_MODE,      PASSKEY_ENTRY_MODE, JUST_WORK_MODE, PASSKEY_ENTRY_MODE},
    {JUST_WORK_MODE,     JUST_WORK_MODE,          JUST_WORK_MODE,     JUST_WORK_MODE, JUST_WORK_MODE},
    {PASSKEY_ENTRY_MODE, NUMERIC_COMPARISON_MODE, PASSKEY_ENTRY_MODE, JUST_WORK_MODE, NUMERIC_COMPARISON_MODE},
};

int g_data_size;
int g_spp_port;
sTaskRef g_spp_data_task_ref;
pub_cache_p g_data_cache;
ret_msg_t g_data_result;

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
    char buffer[500] = {0};
    static struct sc_bt_addr temp_bt_address = {0};
    struct sc_bt_addr temp_bt_address_full_zero = {0};
    struct sc_bt_event_pairing_request *pairing_request = NULL;
    bt_pair_mode_enum pair_mode = JUST_WORK_MODE;
    unsigned char local_io_capability = 0;
    unsigned char peer_io_capability = 0;
    unsigned short spp_allow_receive_max = 0;

    struct sc_bt_task_event *msg = (struct sc_bt_task_event *)argv;

    LOG_INFO("event_type:%d event_id:%d", msg->event_type, msg->event_id);
    switch(msg->event_type)
    {
        case SC_BTTASK_IND_TYPE_COMMON:
        {
            switch(msg->event_id)
            {
                case SC_BTTASK_IND_INQUIRY_RESULT:
                {
                    struct sc_bt_event_inquiry *inquiry = (struct sc_bt_event_inquiry *)msg->payload;

                    LOG_INFO("inquiry result:");
                    LOG_INFO("peer bt address(%02X:%02X:%02X:%02X:%02X:%02X) RSSI:%d",
                            inquiry->addr[5],inquiry->addr[4],inquiry->addr[3],
                            inquiry->addr[2],inquiry->addr[1],inquiry->addr[0],inquiry->rssi);
                    temp_bt_address.bytes[0] = inquiry->addr[5];
                    temp_bt_address.bytes[1] = inquiry->addr[4];
                    temp_bt_address.bytes[2] = inquiry->addr[3];
                    temp_bt_address.bytes[3] = inquiry->addr[2];
                    temp_bt_address.bytes[4] = inquiry->addr[1];
                    temp_bt_address.bytes[5] = inquiry->addr[0];
                    sprintf(buffer,"inquiry result: %s \"%02X:%02X:%02X:%02X:%02X:%02X\" RSSI:%d",
                            inquiry->name,
                            inquiry->addr[5],inquiry->addr[4],inquiry->addr[3],
                            inquiry->addr[2],inquiry->addr[1],inquiry->addr[0],
                            inquiry->rssi);
                    demo_bt_send_buf(buffer,strlen(buffer));
                }
                break;
                case SC_BTTASK_IND_INQUIRY_COMPLETE:
                {
                    LOG_INFO("inquiry complete");
                    if(memcmp(temp_bt_address.bytes, temp_bt_address_full_zero.bytes, 6) != 0)
                    {
                        memset(temp_bt_address.bytes, 0, 6);
                    }

                    memcpy(buffer, "INQUIRY COMPLETE",16);
                    demo_bt_send_buf(buffer,strlen(buffer));
                }
                break;
                case SC_BTTASK_IND_PAIRING_REQUEST:
                {
                    pairing_request = (struct sc_bt_event_pairing_request *)msg->payload;
                    sAPI_BTGetIOCap(&local_io_capability);
                    peer_io_capability = pairing_request->io_capability;
                    pair_mode = g_bt_pairing_mode[local_io_capability][peer_io_capability];

                    LOG_INFO("pair request");
                    LOG_INFO("pair mode(%d)", pair_mode);

                    sprintf(buffer, "PAIRING MODE[%d]",pair_mode);
                    demo_bt_send_buf(buffer,strlen(buffer));

#if 1
                    if(pair_mode == JUST_WORK_MODE)
                    {
                        sAPI_TaskSleep(1000 / 5);//avoid to fast
                        sAPI_BTPairAccept(JUST_WORK_MODE, pairing_request->addr, 0);
                    }

                    if(pair_mode == NUMERIC_COMPARISON_MODE)
                    {
                        sAPI_TaskSleep(1000 / 5);//avoid to fast
                        sAPI_BTPairAccept(NUMERIC_COMPARISON_MODE, pairing_request->addr, 0);
                    }
#endif
                }
                break;
                case SC_BTTASK_IND_PAIRED:
                {
                    struct sc_bt_event_paired *paired = (struct sc_bt_event_paired *)msg->payload;
                    sprintf(buffer,"paired with %s(%02X:%02X:%02X:%02X:%02X:%02X)",paired->name,
                        paired->addr[5],paired->addr[4],paired->addr[3],
                        paired->addr[2],paired->addr[1],paired->addr[0]);

                    demo_bt_send_buf(buffer,strlen(buffer));
                    break;
                }
                case SC_BTTASK_IND_SHUTDOWN_COMPLETE:
                {
                    LOG_INFO("bluetooth close success.");
                    bt_spp_port = 0;
                    bt_spp_max_frame_size = 0;

                    memcpy(buffer, "BT CLOSE", 8);
                    demo_bt_send_buf(buffer,strlen(buffer));
                }
                break;
                case SC_BTTASK_IND_RSSI:
                {
                    signed char *value_rssi = (signed char *)msg->payload;
                    LOG_INFO("demo_bt:handle Read RSSI message %d\r\n", *value_rssi );
                    sprintf(buffer,"RSSI[%d]",*value_rssi);
                    demo_bt_send_buf(buffer,strlen(buffer));
                }
                break;
                default:break;
            }
        }
        break;
        case SC_BTTASK_IND_TYPE_SPP:
        {
            switch(msg->event_id)
            {
                case SC_BTTASK_IND_SPP_CONNECT_IND:
                {
                    struct sc_bt_event_spp_event *connect = (struct sc_bt_event_spp_event *)msg->payload;
                    if(connect->result == 0)
                    {
                        LOG_INFO("spp connect success");
                        LOG_INFO("peer bt address(%02X:%02X:%02X:%02X:%02X:%02X)",
                                    connect->addr[5],connect->addr[4],connect->addr[3],
                                    connect->addr[2],connect->addr[1],connect->addr[0]);
                        LOG_INFO("spp_port(%d) spp_max_frame_size(%d)", connect->port, connect->max_frame_size);
                        bt_spp_port = connect->port;
                        bt_spp_max_frame_size = connect->max_frame_size;
                        //sAPI_BTSPPSend(bt_spp_device_b_send_data, strlen((const char *)bt_spp_device_b_send_data), bt_spp_port);
                    }
                    sprintf(buffer,"SPP_CONNECT_IND result= %d,port=%d,spp_max_frame_size = %d address(%02X:%02X:%02X:%02X:%02X:%02X)",
                                connect->result,connect->port, connect->max_frame_size,
                                connect->addr[5],connect->addr[4],connect->addr[3],
                                connect->addr[2],connect->addr[1],connect->addr[0]);
                    demo_bt_send_buf(buffer,strlen(buffer));
                }
                break;
                case SC_BTTASK_IND_SPP_CONNECT_CNF:
                {
                    struct sc_bt_event_spp_event *connect = (struct sc_bt_event_spp_event *)msg->payload;
                    if(connect->result == 0)
                    {
                        LOG_INFO("spp connect success");
                        LOG_INFO("peer bt address(%02X:%02X:%02X:%02X:%02X:%02X)",
                                    connect->addr[5],connect->addr[4],connect->addr[3],
                                    connect->addr[2],connect->addr[1],connect->addr[0]);
                        LOG_INFO("spp_port(%d) spp_max_frame_size(%d)", connect->port, connect->max_frame_size);
                        bt_spp_port = connect->port;
                        bt_spp_max_frame_size = connect->max_frame_size;
                        //sAPI_BTSPPSend(bt_spp_device_a_send_data, strlen((const char *)bt_spp_device_a_send_data), bt_spp_port);
                    }
                    sprintf(buffer,"SPP_CONNECT_CNF result= %d,port=%d,spp_max_frame_size = %d address(%02X:%02X:%02X:%02X:%02X:%02X)",
                                connect->result,connect->port, connect->max_frame_size,
                                connect->addr[5],connect->addr[4],connect->addr[3],
                                connect->addr[2],connect->addr[1],connect->addr[0]);
                    demo_bt_send_buf(buffer,strlen(buffer));
                }
                break;
                case SC_BTTASK_IND_SPP_DISCONNECT_IND:
                {
                    struct sc_bt_event_spp_event *connect = (struct sc_bt_event_spp_event *)msg->payload;
                    if(connect->result == 0)
                    {
                        LOG_INFO("spp disconnect success");
                        bt_spp_port = 0;
                        bt_spp_max_frame_size = 0;
                    }
                    sprintf(buffer,"SPP_DISCONNECT_IND result= %d", connect->result);
                    demo_bt_send_buf(buffer,strlen(buffer));
                }
                break;
                case SC_BTTASK_IND_SPP_DISCONNECT_CNF:
                {
                    struct sc_bt_event_spp_event *connect = (struct sc_bt_event_spp_event *)msg->payload;
                    if(connect->result == 0)
                    {
                        LOG_INFO("spp disconnect success");
                        bt_spp_port = 0;
                        bt_spp_max_frame_size = 0;
                    }
                    sprintf(buffer,"SPP_DISCONNECT_CNF result= %d", connect->result);
                    demo_bt_send_buf(buffer,strlen(buffer));
                }
                break;
                case SC_BTTASK_IND_SPP_DATA_IND:
                {
                    struct sc_bt_event_spp_event *connect = (struct sc_bt_event_spp_event *)msg->payload;
                    spp_allow_receive_max = (bt_spp_max_frame_size <= BT_SPP_MAX_FRAME_SIZE) ? bt_spp_max_frame_size : BT_SPP_MAX_FRAME_SIZE;
                    LOG_INFO("spp receive data");
                    LOG_INFO("the length of received data is (%d)", connect->data_len);
                    if(connect->data_len <= spp_allow_receive_max)
                    {
                        char *urc = sal_malloc(connect->data_len + 1);
                        snprintf(urc, connect->data_len + 1, "%s", connect->data);
                        LOG_INFO("%s", urc);
                        sprintf(buffer,"SPP_DATA_IND result= %d,data=%s",connect->result,urc);
                    }
                    else
                    {
                        LOG_INFO("received data too much, has exceeded the cache");
                        strcpy(buffer,"SPP_DISCONNECT_CNF received data too much");
                    }
                    demo_bt_send_buf(buffer,strlen(buffer));
                }
                break;
                case SC_BTTASK_IND_SPP_DATA_CNF:
                {
                    struct sc_bt_event_spp_event *connect = (struct sc_bt_event_spp_event *)msg->payload;
                    sprintf(buffer,"\r\nSPP send data result= %d\r\n",connect->result);
                    if(connect->result == 0)
                        LOG_INFO("spp send data success");
                    else
                        LOG_INFO("spp send data failure");
                    sprintf(buffer,"SPP_DATA_CNF result= %d",connect->result);
                    demo_bt_send_buf(buffer,strlen(buffer));
                }
                break;
                default:break;
            }
        }
        break;

        default:break;
    }
}

static ret_msg_t bt_set_addr_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *addrStr = NULL;
    unsigned char addr[6] = {0};
    int j = 6;

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
        addr[--j] = strtoul(tmp,NULL,16);
    }
    LOG_INFO("addr %02X:%02X:%02X:%02X:%02X:%02X",
                addr[0],addr[1],addr[2],
                addr[3],addr[4],addr[5]);
    ret.errcode = sAPI_BTSetAddress(addr);
    if (0 != ret.errcode)
    {
        LOG_ERROR("bt set address failed");
        ret.msg = "bt set address failed";
        return ret;
    }

    return ret;
}

static ret_msg_t bt_get_addr_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    arg_t out_arg[1] = {0};
    int result = 0;
    unsigned char addr[6] = {0};
    char buffer[32] = {0};

    result = sAPI_BTGetAddress(addr);
    if(result != 0)
    {
        LOG_ERROR("bt get address failed");
        ret.msg = "bt get address failed";
        ret.errcode = result;
        return ret;
    }
    sprintf(buffer,"%02X:%02X:%02X:%02X:%02X:%02X",
            addr[5],addr[4],addr[3], addr[2],addr[1],addr[0]);
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
    ret.errcode = sAPI_BTSetName(name,strlen(name));
    if (0 != ret.errcode)
    {
        LOG_ERROR("bt set name failed");
        ret.msg = "bt set name failed";
        return ret;
    }

    return ret;
}

static ret_msg_t bt_get_name_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    arg_t out_arg[1] = {0};
    int result = 0;
    char name[255] = {0};
    unsigned char len;

    result = sAPI_BTGetName(name, &len);
    if(result != 0)
    {
        LOG_ERROR("bt get name failed");
        ret.msg = "bt get name failed";
        ret.errcode = result;
        return ret;
    }
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARGBT_NAME;
    out_arg[0].value = pl_demo_make_value(0, name, len);

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);

    return ret;
}

static ret_msg_t bt_open_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0;
    int io_cap = 0;
    pl_demo_get_value(&io_cap, argv, argn, ARGBT_IOCAP);

    if(io_cap < DISPLAY_ONLY || io_cap > NO_DISPALY_AND_NO_KEYBOARD)
    {
        LOG_ERROR("io_capability is error,%d", io_cap);
        ret.errcode = -1;
        return ret;
    }
    sAPI_BTRegisterEventHandle(bt_event_handle_cb);
    sAPI_BTSetIOCap(io_cap);

    result = sAPI_BTOpen();
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

static ret_msg_t bt_close_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0;

    result = sAPI_BTClose();
    if(result == 0)
    {
        LOG_INFO("close bt device success.");
        ret.msg = "close bt device success.";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("close bt device fail. error code = %d", result);
        ret.msg = "close bt device fail";
    }
    return ret;
}

static ret_msg_t bt_inquiry_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0;
    int duration = 0, scan_resp_num = 0;

    pl_demo_get_value(&duration, argv, argn, ARGBT_SCAN_TIME);
    pl_demo_get_value(&scan_resp_num, argv, argn, ARGBT_RESP_NUM);
    LOG_INFO("duration[%d],scan_resp_num[%d]",duration,scan_resp_num);
    result = sAPI_BTScanStart(duration, scan_resp_num);
    if(result != 0)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("start to inquiry failed. error code = %d", result);
        ret.msg = "start to inquiry failed";
    }
    else
    {
        LOG_INFO("start to inquiry");
        ret.msg = "start to inquiry";
    }
    return ret;
}

static ret_msg_t bt_inquiry_cancel_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0;

    result = sAPI_BTScanStop();
    if(result != 0)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("stop to inquiry failed. error code = %d", result);
        ret.msg = "stop to inquiry failed";
    }
    else
    {
        LOG_INFO("stop to inquiry");
        ret.msg = "stop to inquiry";
    }
    return ret;
}
static ret_msg_t bt_pair_request_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0, j = 6;
    const char *addrStr = NULL;
    struct sc_bt_addr addr = {0};

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
    LOG_INFO("addr[%02X:%02X:%02X:%02X:%02X:%02X]",
                addr.bytes[0],addr.bytes[1],addr.bytes[2],
                addr.bytes[3],addr.bytes[4],addr.bytes[5]);

    result = sAPI_BTPairStart(addr);
    if(result != 0)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("pair to device failed. error code = %d", result);
        ret.msg = "pair to device failed";
    }
    else
    {
        LOG_INFO("start to pair");
        ret.msg = "start to pair";
    }
    return ret;
}

static ret_msg_t bt_pair_response_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0, j = 6;
    const char *addrStr = NULL;
    int accept = 0, mode = 0, passkey = 0;
    struct sc_bt_addr addr = {0};

    pl_demo_get_value(&accept, argv, argn, ARGBT_PAIR_RESP);
    pl_demo_get_value(&mode, argv, argn, ARGBT_PAIR_MODE);
    pl_demo_get_data(&addrStr, argv, argn, ARGBT_ADDR);
    pl_demo_get_value(&passkey, argv, argn, ARGBT_VERIFICATION);
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
    LOG_INFO("mode[%d],passkey[%d] addr[%02X:%02X:%02X:%02X:%02X:%02X]",
                mode,passkey,
                addr.bytes[0],addr.bytes[1],addr.bytes[2],
                addr.bytes[3],addr.bytes[4],addr.bytes[5]);

    if(accept == 0)
        result = sAPI_BTPairAccept(mode,addr,passkey);
    else
        result = sAPI_BTPairRefuse(mode,addr);
    if(result != 0)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("pairing response failed. error code = %d", result);
        ret.msg = "pairing response failed";
    }
    else
    {
        LOG_INFO("pairing response OK");
        ret.msg = "pairing response OK";
    }
    return ret;
}

static ret_msg_t bt_unpair_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0, j = 6;
    const char *addrStr = NULL;
    struct sc_bt_addr addr = {0};

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
    LOG_INFO("addr[%02X:%02X:%02X:%02X:%02X:%02X]",
                addr.bytes[0],addr.bytes[1],addr.bytes[2],
                addr.bytes[3],addr.bytes[4],addr.bytes[5]);

    result = sAPI_BTUnpair(addr);

    if(result != 0)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("unpair failed. error code = %d", result);
        ret.msg = "unpair failed";
    }
    else
    {
        LOG_INFO("unpair OK");
        ret.msg = "unpair OK";
    }
    return ret;
}

static ret_msg_t bt_get_pairing_info_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    arg_t out_arg[3] = {0};
    unsigned char bt_device_record_num = 0;
    struct sc_bt_device_record bt_device_record_array[10] = {0};
    char buffer[32] = {0};

    memset(bt_device_record_array, 0, sizeof(struct sc_bt_device_record) * 10);
    sAPI_BTPaired(bt_device_record_array, &bt_device_record_num);
    for(int i = 0; i < bt_device_record_num; i++)
    {
        pl_demo_release_value(out_arg[0].value);
        pl_demo_release_value(out_arg[1].value);
        pl_demo_release_value(out_arg[2].value);

        LOG_INFO("bt device record[%d] name[%s] addr(%02X:%02X:%02X:%02X:%02X:%02X)", 
                    i, bt_device_record_array[i].name,
                    bt_device_record_array[i].addr[5], bt_device_record_array[i].addr[4],
                    bt_device_record_array[i].addr[3], bt_device_record_array[i].addr[2],
                    bt_device_record_array[i].addr[1], bt_device_record_array[i].addr[0]);

        out_arg[0].arg_name = "record_index";
        out_arg[0].type = TYPE_INT;
        out_arg[0].value = pl_demo_make_value(i, NULL, 0);

        out_arg[1].arg_name = ARGBT_NAME;
        out_arg[1].type = TYPE_STR;
        out_arg[1].value = pl_demo_make_value(0, bt_device_record_array[i].name, strlen(bt_device_record_array[i].name));

        sprintf(buffer,"%02X:%02X:%02X:%02X:%02X:%02X",bt_device_record_array[i].addr[5],bt_device_record_array[i].addr[4],
                    bt_device_record_array[i].addr[3],bt_device_record_array[i].addr[2],
                    bt_device_record_array[i].addr[1],bt_device_record_array[i].addr[0]);
        out_arg[2].arg_name = ARGBT_ADDR;
        out_arg[2].type = TYPE_STR;
        out_arg[2].value = pl_demo_make_value(0, (const char *)buffer, strlen(buffer));

        pl_demo_post_data(OP_POST, out_arg, 3, NULL, 0);
    }

    pl_demo_release_value(out_arg[0].value);
    pl_demo_release_value(out_arg[1].value);
    pl_demo_release_value(out_arg[2].value);

    return ret;
}

static ret_msg_t bt_spp_connect_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0, j = 6;
    const char *addrStr = NULL;
    struct sc_bt_addr addr = {0};

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
    LOG_INFO("addr[%02X:%02X:%02X:%02X:%02X:%02X]",
                addr.bytes[0],addr.bytes[1],addr.bytes[2],
                addr.bytes[3],addr.bytes[4],addr.bytes[5]);

    result = sAPI_BTSPPConnect(addr);
    if(result != 0)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("spp connect to device failed. error code = %d", result);
        ret.msg = "spp connect to device failed";
    }
    else
    {
        LOG_INFO("spp start to connect");
        ret.msg = "spp start to connect";
    }
    return ret;

}

static ret_msg_t bt_spp_disconnect_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = 0;
    int spp_port = 0;

    pl_demo_get_value(&spp_port, argv, argn, ARGBT_SPP_PORT);
    if (spp_port < 0)
    {
        LOG_ERROR("spp_port invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "spp_port invalid";
        return ret;
    }

    result = sAPI_BTSPPDisconnect(spp_port);
    if(result != 0)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("spp disconnect to device failed. error code = %d", result);
        ret.msg = "spp disconnect to device failed";
    }
    else
    {
        LOG_INFO("spp start to disconnect");
        ret.msg = "spp start to disconnect";
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
    return ret;
}

static int bt_spp_need_size(void)
{
    return g_data_size;
}

static ret_msg_t bt_spp_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms))
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

        result = sAPI_BTSPPSend((unsigned char *)buf, read_size, g_spp_port);
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

static ret_msg_t bt_rssi_read_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int result = sAPI_BTReadRssi();
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

