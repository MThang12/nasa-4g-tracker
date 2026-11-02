#include <stdio.h>
#include <stdlib.h>

#include "simcom_os.h"
#include "simcom_demo_init.h"
#include "demo_protocol_parser.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "cJSON.h"

#define LOG_ERROR(...) sal_log_error("[DEMO-CJSON] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-CJSON] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-CJSON] " __VA_ARGS__)

#define ARGS_CJSON          "JSON:"
#define ARGS_CJSON_COMPANY  "company"
#define ARGS_CJSON_MODULE   "module"
#define ARGS_CJSON_PARSE    "parse"

static ret_msg_t cjson_create(op_t op, arg_t *argv, int argn);
static ret_msg_t cjson_parse(op_t op, arg_t *argv, int argn);

const value_t company[] =
{
    {
        .bufp = "SIMCOM",
        .size = 6,
    },
};
const value_list_t company_range =
{
    .isrange = false,
    .list_head = company,
    .count = 1,
};

// static char json_data[] = "{\"company\":\"simcom\",\"module\":\"A7600E\"}";

// const value_t json[] =
// {
//     {
//         .bufp = json_data,
//         .size = 38,
//     },
// };
// const value_list_t json_range =
// {
//     .isrange = false,
//     .list_head = json,
//     .count = 1,
// };


arg_t args_create[2] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGS_CJSON_COMPANY,
        .msg = "Support string",
        .value_range = &company_range
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGS_CJSON_MODULE,
        .msg = "Support string:"
    }
};

// arg_t args_parse[1] =
// {
//     {
//         .type = TYPE_STR,
//         .arg_name = ARGS_CJSON_COMPANY,
//         .msg = "Support string",
//         .value_range = &json_range
//     }
// };

const menu_list_t func_cjson_cteate =
{
    .menu_name = "cjson create",
    .menu_type = TYPE_FUNC,
    .parent = &menu_cjson,
    .function = {
        .argv = args_create,
        .argn = sizeof(args_create) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = cjson_create,
    },
};

const menu_list_t func_cjson_parse =
{
    .menu_name = "cjson parse",
    .menu_type = TYPE_FUNC,
    .parent = &menu_cjson,
    .function = {
        .argv = NULL,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = cjson_parse,
    },
};

const menu_list_t *cjson_menu_sub[] =
{
    &func_cjson_cteate,
    &func_cjson_parse,
    NULL  // MUST end by NULL
};

const menu_list_t menu_cjson =
{
    .menu_name = "simcom cjson",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = cjson_menu_sub,
    .parent = &root_menu,
};

char* json = NULL;

static ret_msg_t cjson_create(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *company = NULL;
    const char *module = NULL;
    // arg_t out_arg = {0};

    pl_demo_get_data(&company, argv, argn, ARGS_CJSON_COMPANY);
    pl_demo_get_data(&module, argv, argn, ARGS_CJSON_MODULE);

    if (!company || !module)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    cJSON * root =  cJSON_CreateObject();

    cJSON_AddItemToObject(root, "company", cJSON_CreateString(company));
    cJSON_AddItemToObject(root, "module", cJSON_CreateString(module));
    //cJSON_AddItemToObject(root, "number", cJSON_CreateNumber(0xFFFFFFFF));

    if(json)
        free(json);
    json = cJSON_Print(root);
    LOG_INFO("JSON:%s\n", json);

    // pl_demo_release_value(out_arg.value);
    // out_arg.value = pl_demo_make_value(0, json, strlen(json));
    // out_arg.type = TYPE_STR;
    // out_arg.arg_name = ARGS_CJSON;

    // pl_demo_post_data(OP_POST, &out_arg, 1, NULL, 0);

    // pl_demo_release_value(out_arg.value);

    cJSON_Delete(root);

    return ret;
}

static ret_msg_t cjson_parse(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    arg_t out_arg[2] = {0};

    cJSON* root = NULL;
    cJSON* itemName = NULL;
    cJSON* item_module = NULL;
    cJSON_Hooks hooks;
    hooks.malloc_fn = (void *)malloc;
    hooks.free_fn = free;
    cJSON_InitHooks(&hooks);

    if(json == NULL)
    {
        LOG_ERROR("json NULL");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "cjson create first";
        return ret;
    }

    root = cJSON_Parse(json);
    if(root == NULL)
    {
        LOG_ERROR("json parse fail");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "json parse fail";
    }

    itemName = cJSON_GetObjectItem(root, "company");
    if(itemName)
        LOG_ERROR("company[%s]",itemName->valuestring);

    item_module = cJSON_GetObjectItem(root, "module");
    if(item_module)
        LOG_ERROR("module[%s]",item_module->valuestring);

    pl_demo_release_value(out_arg[0].value);
    out_arg[0].value = pl_demo_make_value(0, itemName->valuestring, strlen(itemName->valuestring));
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARGS_CJSON_COMPANY;

    pl_demo_release_value(out_arg[1].value);
    out_arg[1].value = pl_demo_make_value(0, item_module->valuestring, strlen(item_module->valuestring));
    out_arg[1].type = TYPE_STR;
    out_arg[1].arg_name = ARGS_CJSON_MODULE;

    pl_demo_post_data(OP_POST, out_arg, 2, NULL, 0);

    pl_demo_release_value(out_arg[0].value);
    pl_demo_release_value(out_arg[1].value);

    cJSON_Delete(root);

    return ret;
}
