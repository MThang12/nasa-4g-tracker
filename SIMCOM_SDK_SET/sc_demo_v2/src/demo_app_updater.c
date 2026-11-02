#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "simcom_os.h"
#include "simcom_common.h"
#include "sc_app_update.h"
#include "userspaceConfig.h"
#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "simcom_os.h"
#include "demo_app_updater.h"
#include "simcom_file.h"

#define APP_FILE_NAME   "c:/customer_app.bin"

#define LOG_ERROR(...) sal_log_error("[DEMO-APP-UPDATER] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-APP-UPDATER] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-APP-UPDATER] " __VA_ARGS__)

extern void sAPI_FBF_Disable(void);
extern void sAPI_SysReset(void);


static ret_msg_t set_openAppPackage_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_file_to_reserver_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_closeAppPackage_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_crcAppPackage_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_resetToUpgrade_proc(op_t op, arg_t *argv, int argn);

const value_t rw_upgrade_mode[2] =
{
    {
        .bufp = R_MODE,
        .size = sizeof(R_MODE)-1,
    },

    {
        .bufp = W_MODE,
        .size = sizeof(W_MODE)-1,
    },

};

const value_list_t RW_mode_range =
{
    .isrange = false,
    .list_head = rw_upgrade_mode,
    .count = 2,
};

arg_t setopenAppPackage_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = RW_MODE,
        .value_range = &RW_mode_range
    }
};

const menu_list_t func_openAppPackage =
{
    .menu_name = "open and init AppPackage",
    .menu_type = TYPE_FUNC,
    .parent = &app_updater_menu,
    .function = {
        .argv = setopenAppPackage_args,
        .argn = 1,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_openAppPackage_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_file_to_reserver =
{
    .menu_name = "file is writen into reserver from FS",
    .menu_type = TYPE_FUNC,
    .parent = &app_updater_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_file_to_reserver_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_closeAppPackage =
{
    .menu_name = "close AppPackage",
    .menu_type = TYPE_FUNC,
    .parent = &app_updater_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_closeAppPackage_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_crcAppPackage =
{
    .menu_name = "crc AppPackage",
    .menu_type = TYPE_FUNC,
    .parent = &app_updater_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_crcAppPackage_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_resetToUpgrade =
{
    .menu_name = "reset to upgrade",
    .menu_type = TYPE_FUNC,
    .parent = &app_updater_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_resetToUpgrade_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t *app_updater_sub[] =
{
    &func_openAppPackage,
    &func_file_to_reserver,
    &func_closeAppPackage,
    &func_crcAppPackage,
    &func_resetToUpgrade,

    NULL  // MUST end by NULL
};

const menu_list_t app_updater_menu =
{
    .menu_name = "LOCAL_APP_UPDATER",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = app_updater_sub,
    .parent = &root_menu,
};


static ret_msg_t set_openAppPackage_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *mode = NULL;

    pl_demo_get_data(&mode, argv, argn, RW_MODE);
    if(strncmp(mode,R_MODE,sizeof(R_MODE)-1) == 0)
    {
        if(sAPI_AppPackageOpen("r") != 0)
        {
            if(sAPI_AppPackageOpen("r") == 1)
            {
                LOG_ERROR("set R_MODE fail and already opened");
                ret.errcode = ERRCODE_DEFAULT;
                ret.msg = "set R_MODE fail and already opened";
            }
            return ret;
        }
    }
    if(strncmp(mode,W_MODE,sizeof(W_MODE)-1) == 0)
    {
        if(sAPI_AppPackageOpen("w") != 0)
        {
            if(sAPI_AppPackageOpen("w") == 1)
            {
                LOG_ERROR("set w MODE fail and already opened");
                ret.errcode = ERRCODE_DEFAULT;
                ret.msg = "set w MODE fail and already opened";
            }
            return ret;
        }
    }

    return ret;
}

static SCFILE *fp = NULL;
static ret_msg_t set_file_to_reserver_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int flag = 0; 
    size_t read_len = 0;

    sAPI_FBF_Disable();  //must unlock fbf befor download
    //sAPI_remove();
    flag = sAPI_FsSwitchDir(APP_FILE_NAME, DIR_C_TO_SIMDIR);
    if(0 != flag)
    {
        LOG_ERROR("sAPI_FsSwitchDir fail,flag = %d",flag);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_FsSwitchDir fail";
        return ret;
    }

    fp = sAPI_fopen("c:/customer_app.bin", "rb");
    if (fp == NULL) {
        LOG_ERROR("sAPI_fopen fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_fopen fail";
        return ret;
    }

    void *read_data = (void *)sal_malloc(1024);
    if (!read_data) {
        LOG_ERROR("sal_malloc fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sal_malloc data fail";
        if(fp)
        {
            sAPI_fclose(fp);
        }
        return ret;
    }

    while (1) {
        memset(read_data, 0, 1024);
        read_len = sAPI_fread(read_data, 1024, 1, fp);
        if (read_len == 0)
        {
            break;
        }
        if (sAPI_AppPackageWrite(read_data, read_len) != 0) {
            LOG_ERROR("sAPI_AppPackageWrite fail");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "sAPI_AppPackageWrite fail";
            sal_free(read_data);
            if(fp)
            {
                sAPI_fclose(fp);
            }
            return ret;
        }
    }
    sal_free(read_data);
    return ret;
}

static ret_msg_t set_closeAppPackage_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    if (sAPI_AppPackageClose() != 0) {
        LOG_ERROR("NO OPEN");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "NO OPEN";
        if(fp)
        {
            sAPI_fclose(fp);
        }
        return ret;
    }
    return ret;
}

static ret_msg_t set_crcAppPackage_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SCAppPackageInfo pInfo = {0};
    if (sAPI_AppPackageCrc(&pInfo) != 0) {
        LOG_ERROR("app crc fail.\r\n");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "file crc fail.";
        if(fp)
        {
            sAPI_fclose(fp);
        }
        return ret;
    }
    LOG_ERROR("app crc success!!!.\r\n");
    ret.errcode = ERRCODE_OK;
    ret.msg = "app crc success!!!.";
    return ret;
}

static ret_msg_t set_resetToUpgrade_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    //sAPI_TaskSleep(20);
    if(fp)
    {
        sAPI_fclose(fp);
    }
    sAPI_SysReset();
    return ret;
}



#if 0
static ret_msg_t set_app_updater_cb(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
#ifdef HAS_DEMO_FS
    int flag = -1;
    SCFILE *fp = NULL;
    size_t read_len = 0;
    SCAppPackageInfo pInfo = {0};
    arg_t out_arg[1] = {0};

    void *read_data = (void *)malloc(1024);
    if (!read_data) {
        LOG_ERROR("malloc fail!!!");
        goto error;
    }

    sAPI_FBF_Disable();//must unlock fbf befor download

    flag = sAPI_FsSwitchDir(APP_FILE_NAME, DIR_C_TO_SIMDIR); /* swich the customwer_app.bin path from c:/ to c:/simdir, or the API of the file system cannoperateit*/
    if (flag) {
        LOG_ERROR("sAPI_FsSwitchDir fail !!!");
        goto error;
    }

    fp = sAPI_fopen("c:/customer_app.bin", "rb");
    if (fp == NULL) {
        LOG_ERROR();
        goto error;
    }

    if (sAPI_AppPackageOpen("w") != 0) {
        LOG_ERROR();
        goto error;
    }

    while (1) {
        memset(read_data, 0, 1024);
        read_len = sAPI_fread(read_data, 1024, 1, fp);
        if (read_len == 0)
            break;
        if (sAPI_AppPackageWrite(read_data, read_len) != 0) {
            LOG_ERROR();
            goto error;
        }
    }

    if (sAPI_AppPackageClose() != 0) {
        LOG_ERROR();
        goto error;
    }
    if (sAPI_AppPackageCrc(&pInfo) != 0) {
        LOG_ERROR("app crc fail.\r\n");
        goto error;
    }
    else
    {
        char buf[50]={0};
        snprintf(buf,sizeof(buf)-1,"app crc success --> reset!\n");
        LOG_INFO("app crc success --> reset!");
        out_arg[0].arg_name = "local_app_update";
        out_arg[0].type = TYPE_RAW;
        out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
        pl_demo_post_urc(&func_app_updater,TYPE_URC_DATA, out_arg, 1, buf, strlen(buf));
        pl_demo_release_value(out_arg[0].value);
        sAPI_TaskSleep(20);
        sAPI_SysReset();
    }

error:
    if (fp)
        sAPI_fclose(fp);
    if (read_data)
        free(read_data);
#else
    LOG_INFO("NO support HAS_DEMO_FS");
#endif
    return ret;
}
#endif