#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "simcom_os.h"
#include "simcom_fota_download.h"
#include "demo_fota.h"

#define LOG_ERROR(...) sal_log_error("[DEMO-FOTA] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-FOTA] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-FOTA] " __VA_ARGS__)

static ret_msg_t set_fotabynet_cmd_proc(op_t op, arg_t *argv, int argn);

const value_t fota_upgrade_mode[2] =
{
    {
        .bufp = FOTA_UPGRADE_BY_FTP,
        .size = sizeof(FOTA_UPGRADE_BY_FTP)-1,
    },

    {
        .bufp = FOTA_UPGRADE_BY_HTTP,
        .size = sizeof(FOTA_UPGRADE_BY_HTTP)-1,
    },

};

const value_list_t upgrade_mode_range =
{
    .isrange = false,
    .list_head = fota_upgrade_mode,
    .count = 2,
};


arg_t setfotaupgrade_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGFOTA_MODE,
        .value_range = &upgrade_mode_range
    },
    {
        .type = TYPE_STR,
        .arg_name = UPGRADE_URL,
    },
    {
        .type = TYPE_STR,
        .arg_name = UPGRADE_USERNAME,
    },
    {
        .type = TYPE_STR,
        .arg_name = UPGRADE_PASSWORD,
    }
};

const menu_list_t func_fotabynet =
{
    .menu_name = "fota upgrade by net",
    .menu_type = TYPE_FUNC,
    .parent = &fota_menu,
    .function = {
        .argv = setfotaupgrade_args,
        .argn = sizeof(setfotaupgrade_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_fotabynet_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t *fota_menu_sub[] =
{
    &func_fotabynet,

    NULL  // MUST end by NULL
};

const menu_list_t fota_menu =
{
    .menu_name = "FOTA",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = fota_menu_sub,
    .parent = &root_menu,
};


static ret_msg_t set_fotabynet_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    struct SC_FotaApiParam param = {0};
    const char *upgrdade_mode = NULL;
    const char *url = NULL;
    const char *username = NULL;
    const char *password = NULL;
    pl_demo_get_data(&upgrdade_mode, argv, argn, ARGFOTA_MODE);
    if (!upgrdade_mode)
    {
        LOG_ERROR("upgrdade_mode invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "upgrdade_mode invalid";
        return ret;
    }
    if(strncmp(upgrdade_mode,FOTA_UPGRADE_BY_FTP,sizeof(FOTA_UPGRADE_BY_FTP)-1) == 0)
    {
       param.mode = 0;
    }
    if(strncmp(upgrdade_mode,FOTA_UPGRADE_BY_HTTP,sizeof(FOTA_UPGRADE_BY_HTTP)-1) == 0)
    {
        param.mode = 1;
    }
    LOG_INFO("fota upgrade mode = %d", __func__,param.mode);

    pl_demo_get_data(&url, argv, argn, UPGRADE_URL);
    if (!url)
    {
        LOG_ERROR("url is null");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "url is null";
        return ret;
    }
    else{
        strncpy(param.host, url,sizeof(param.host));
    }

    pl_demo_get_data(&username, argv, argn, UPGRADE_USERNAME);
    if (!username)
    {
        LOG_ERROR("username is null");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "username is null";
        return ret;
    }
    else{
        strncpy(param.username, username,sizeof(param.username));
    }

    pl_demo_get_data(&password, argv, argn, UPGRADE_PASSWORD);
    if (!password)
    {
        LOG_ERROR("password is null");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "password is null";
        return ret;
    }
    else{
        strncpy(param.password, password,sizeof(param.password));
    }

    param.sc_fota_cb = NULL;
    sAPI_FotaServiceBegin((void *)&param);
    return ret;
}
