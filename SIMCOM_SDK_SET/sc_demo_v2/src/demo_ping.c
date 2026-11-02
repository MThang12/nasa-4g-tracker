#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "simcom_ping.h"
#include "simcom_debug.h"

#define LOG_ERROR(...) sal_log_error("[DEMO_PING]"__VA_ARGS__)
#define LOG_INFO(...) sal_log_info("[DEMO_PING]"__VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO_PING]"__VA_ARGS__)

#define ARGPING_SERVER "destination"
#define ARGPING_COUNT  "count"

static ret_msg_t ping_cmd_proc(op_t op, arg_t *argv, int argn);

value_t serve_value =
{
    .bufp = "www.baidu.com",
    .size = 13,
};
value_t count_value =
{
    .val = 4,
};
const value_list_t count_range =
{
    .isrange = true,
    .min = 1,
    .max = 50
};

arg_t ping_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGPING_SERVER,
        .msg = "PING destination address, length is 0-128",
        .value = &serve_value
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGPING_COUNT,
        .msg = "PING count, Maximum support is 50",
        .value_range = &count_range,
        .value = &count_value
    }
};

const menu_list_t func_ping =
{
    .menu_name = "Ping",
    .menu_type = TYPE_FUNC,
    .parent = &ping_menu,
    .function = {
        .argv = ping_args,
        .argn = sizeof(ping_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = ping_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    }
};

const menu_list_t *ping_menu_list[] =
{
    &func_ping,
    NULL
};

const menu_list_t ping_menu =
{
    .menu_name = "ping",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = ping_menu_list,
    .parent = &root_menu
};



/*ping init */
static void pingResult(UINT32 size,INT8 *statistics)
{
    arg_t out_arg[1] = { 0 };

    if(statistics != NULL)
    {
        sAPI_Debug("size[%d] statistics[%s]",size,statistics);

        out_arg[0].arg_name = "pingResult";
        out_arg[0].type = TYPE_STR;
        out_arg[0].value = pl_demo_make_value(0, (char *)statistics, size);

        pl_demo_post_urc(&func_ping,TYPE_URC_NOTICE,out_arg, 1, NULL, 0);
        pl_demo_release_value(out_arg[0].value);
    }
}
static SCping g_ctx = {NULL, 10, 20, pingResult};


static ret_msg_t ping_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    const char *server = NULL;
    int count = 0;

    pl_demo_get_data(&server, argv, argn, ARGPING_SERVER);
    pl_demo_get_value(&count, argv, argn, ARGPING_COUNT);

    g_ctx.destination = (INT8 *)server;
    g_ctx.count = count;

    ret.errcode = sAPI_Ping(&g_ctx);
    sal_log_info("simcom_ping_test ret[%d]",ret.errcode);

    if(SC_PING_SUCESSED == ret.errcode)
    {
        ret.msg = "ping success";
    }
    else
    {
        ret.msg = "ping fail";
    }

    return ret;
}

