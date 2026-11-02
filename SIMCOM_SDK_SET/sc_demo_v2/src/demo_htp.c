/*
*@Author: Yanan Peng
*@Date: 2023-12-13
*@Last Modified by: Yanan Peng
*@Last Modified Date: 2024-03-07
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "simcom_htp_client.h"
#include "simcom_common.h"


#define LOG_ERROR(...) sal_log_error("[DEMO_HTP]"__VA_ARGS__)
#define LOG_INFO(...) sal_log_info("[DEMO_HTP]"__VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO_HTP]"__VA_ARGS__)

#define ARGHTP_HOST "host"
#define ARGHTP_INDEX "index"
#define ARGHTP_PORT "port"
#define ARGHTP_HTP_VERSION "version"
#define ARGHTP_CONFIG_RESULT "configResult"
#define ARGHTP_UPDATE_RESULT "updateResult"
#define SIZE_OF_CMD 4
#define SIZE_OF_HOST_OR_IDX SAL_256


void htp_param_invalid_handle(ret_msg_t *ret);
int htp_config(SChtpOperationType op, char *config_info, const char *cmd, const char* host_or_idx, int host_port, int http_version);
int htp_update_async(void);

static ret_msg_t htp_add_config_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t htp_del_config_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t htp_get_config_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t htp_update_cmd_proc(op_t op, arg_t *argv, int argn);


value_t htp_host_value =
{
    .bufp = "www.baidu.com",
    .size = 13,
};

value_t htp_port_value =
{
    .val = 80,
};
const value_list_t htp_port_range =
{
    .isrange = true,
    .min = 1,
    .max = 65535,
};

value_t htp_version_value =
{
    .val = 1,
};
const value_list_t htp_version_range =
{
    .isrange = true,
    .min = 0,
    .max = 1,
};

value_t htp_index_value =
{
    .bufp = "0",
    .size = 1,
};

arg_t htp_add_config[3] =
{
    {
        .type = TYPE_STR,
        .arg_name =ARGHTP_HOST,
        .msg = "HTP server address, length is 1-255.",
        .value = &htp_host_value,
    },
    {
        .type = TYPE_INT,
        .arg_name =ARGHTP_PORT,
        .msg = "The HTP server port, the range is (1-65535).",
        .value_range = &htp_port_range,
        .value = &htp_port_value,
    },
    {
        .type = TYPE_INT,
        .arg_name =ARGHTP_HTP_VERSION,
        .msg = "The HTTP version of the HTP server: 0(HTTP 1.0) or 1(HTTP 1.1).",
        .value_range = &htp_version_range,
        .value = &htp_version_value,
    }
};

arg_t htp_del_config[1] =
{
    {
        .type = TYPE_STR,
        .arg_name =ARGHTP_INDEX,
        .msg = "The index of the HTP server item to be deleted from the list.(range: 0-9)",
        .value = &htp_index_value,
    },
};

const menu_list_t func_htp_add_config =
{
    .menu_name = "Add Config",
    .menu_type = TYPE_FUNC,
    .parent = &htp_menu,
    .function = {
        .argv = htp_add_config,
        .argn = 3,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = htp_add_config_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    }
};

const menu_list_t func_htp_get_config =
{
    .menu_name = "Get Config",
    .menu_type = TYPE_FUNC,
    .parent = &htp_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = htp_get_config_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    }
};

const menu_list_t func_htp_del_config =
{
    .menu_name = "Delete Config",
    .menu_type = TYPE_FUNC,
    .parent = &htp_menu,
    .function = {
        .argv = htp_del_config,
        .argn = 1,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = htp_del_config_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    }
};

const menu_list_t func_htp_update =
{
    .menu_name = "Update",
    .menu_type = TYPE_FUNC,
    .parent = &htp_menu,
    .function = {
        .argv = NULL,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = htp_update_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    }
};

const menu_list_t *htp_menu_sub[] =
{
    &func_htp_add_config,
    &func_htp_get_config,
    &func_htp_del_config,
    &func_htp_update,
    NULL
};

const menu_list_t htp_menu =
{
    .menu_name = "htp",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = htp_menu_sub,
    .parent = &root_menu
};


sMsgQRef g_htpUIResp_msgq;


void htp_param_invalid_handle(ret_msg_t *ret)
{
    LOG_ERROR("param invalid");
    ret->errcode = ERRCODE_DEFAULT;
    ret->msg = "param invalid";
}

int htp_config(SChtpOperationType op, char *config_info, const char *cmd, const char* host_or_idx, int host_port, int http_version)
{
	int ret = -1;
    if (op == SC_HTP_OP_SET)
    {
        char cmdStr[SIZE_OF_CMD];
        char hostOrIdxStr[SIZE_OF_HOST_OR_IDX];

        memset(cmdStr, 0, sizeof(cmdStr));
        memset(hostOrIdxStr, 0, sizeof(hostOrIdxStr));
        snprintf(cmdStr, sizeof(cmdStr), cmd);
        snprintf(hostOrIdxStr, sizeof(hostOrIdxStr), host_or_idx);

        ret = sAPI_HtpSrvConfig(SC_HTP_OP_SET, NULL, cmdStr, hostOrIdxStr, host_port, http_version, NULL, 0);
    }
    else if (op == SC_HTP_OP_GET)
    {
        ret = sAPI_HtpSrvConfig(SC_HTP_OP_GET, config_info, NULL, NULL, -1, -1, NULL, 0);
    }
    else
    {
        LOG_INFO("htp config wrong opration type: %d", op);
    }

    LOG_INFO("htp config result: %d", ret);

    return ret;
}

int htp_update_async(void)
{
    int ret;
    SIM_MSG_T htp_result = {SC_SRV_NONE, -1, 0, NULL};

    if (NULL == g_htpUIResp_msgq)
    {
        SC_STATUS status;
        status = sAPI_MsgQCreate(&g_htpUIResp_msgq, "htpUIResp_msgq", sizeof(SIM_MSG_T), 4, SC_FIFO);
        if (SC_SUCCESS != status)
        {
            LOG_ERROR("[CHTP]msgQ create fail");
        }
    }

    ret = sAPI_HtpUpdate(g_htpUIResp_msgq);
    LOG_INFO("htp update ret = %d", ret);

    do
    {
        sAPI_MsgQRecv(g_htpUIResp_msgq, &htp_result, SC_SUSPEND);

        if (SC_SRV_HTP != htp_result.msg_id)
        {
            LOG_INFO("[CHTP] htp_result.msg_id =[%d]", htp_result.msg_id);
            htp_result.msg_id = SC_SRV_NONE;
            htp_result.arg1 = -1;
            htp_result.arg3 = NULL;
            continue;
        }
        if (SC_HTP_OK == htp_result.arg1)
        {
            LOG_INFO("[CHTP] successfully update time! ");
            break;
        }
        else
        {
            LOG_INFO("[CHTP] failed to update time! result code: %d", htp_result.arg1);
            break;
        }
    } while (1);

    return ret;
}

static ret_msg_t htp_add_config_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int htp_ret;
    const char *host = NULL;
    int port = -1;
    int version = -1;
    arg_t out_arg[1] = { 0 };

    pl_demo_get_data(&host, argv, argn, ARGHTP_HOST);
    pl_demo_get_value(&port, argv, argn, ARGHTP_PORT);
    pl_demo_get_value(&version, argv, argn, ARGHTP_HTP_VERSION);
    if(NULL == host ||
        port < 1 || port >65535||
        (version != 0 && version != 1))
    {
        htp_param_invalid_handle(&ret);
        goto EXIT;
    }

    htp_ret = htp_config(SC_HTP_OP_SET, NULL, "ADD", host, port, version);

    out_arg[0].arg_name = ARGHTP_CONFIG_RESULT;
    out_arg[0].type = TYPE_STR;
    if(SC_HTP_OK == htp_ret)
    {
        LOG_INFO("ADD SERVER SUCCESSFUL");
        out_arg[0].value = pl_demo_make_value(-1, "OK", 2);
    }
    else
    {
        LOG_INFO("ADD SERVER ERROR");
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

static ret_msg_t htp_get_config_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int htp_ret;
    arg_t out_arg[1] = { 0 };
    char config_buf[SAL_256] =  { 0 };
    char buf[SAL_256] =  { 0 };
    int bufLen = 0;

    htp_ret = htp_config(SC_HTP_OP_GET, config_buf, NULL, NULL, -1, -1);

    if(SC_HTP_OK == htp_ret)
    {
        bufLen = snprintf(buf, sizeof(buf), "The first parameter is index. %s", config_buf);

        LOG_INFO("config_buf: %s, config_info: %s, config_info_len: %d", config_buf, buf, bufLen);
        LOG_INFO("GET CONFIG SUCCESSFUL");

        out_arg[0].arg_name = ARGHTP_CONFIG_RESULT;
        out_arg[0].type = TYPE_RAW;
        out_arg[0].value = pl_demo_make_value(-1, NULL, bufLen);

        ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
        pl_demo_post_raw(buf, bufLen);
    }
    else
    {
        LOG_INFO("GET CONFIG ERROR");

        out_arg[0].arg_name = ARGHTP_CONFIG_RESULT;
        out_arg[0].type = TYPE_STR;
        out_arg[0].value = pl_demo_make_value(-1, "ERROR", 5);

        ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    }

    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
        goto EXIT;
    }

EXIT:
    pl_demo_release_value(out_arg[0].value);

    return ret;
}

static ret_msg_t htp_del_config_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int htp_ret;
    const char *idx = NULL;
    arg_t out_arg[1] = { 0 };

    pl_demo_get_data(&idx, argv, argn, ARGHTP_INDEX);
    if(NULL == idx)
    {
        htp_param_invalid_handle(&ret);
        goto EXIT;
    }

    htp_ret = htp_config(SC_HTP_OP_SET, NULL, "DEL", idx, -1, -1);

    out_arg[0].arg_name = ARGHTP_CONFIG_RESULT;
    out_arg[0].type = TYPE_STR;
    if(SC_HTP_OK == htp_ret)
    {
        LOG_INFO("DELETE SERVER SUCCESSFUL");
        out_arg[0].value = pl_demo_make_value(-1, "OK", 2);
    }
    else
    {
        LOG_INFO("DELETE SERVER ERROR");
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

static ret_msg_t htp_update_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    int htp_ret;
    arg_t out_arg[1] = { 0 };

    htp_ret = htp_update_async();

    out_arg[0].arg_name = ARGHTP_UPDATE_RESULT;
    out_arg[0].type = TYPE_STR;
    if(SC_HTP_OK == htp_ret)
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

