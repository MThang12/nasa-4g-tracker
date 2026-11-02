/*
 * @Author: caican.peng
 * @Date: 2023-11-24 11:15:41
 * @Last Modified by: caican.peng
 * @Last Modified time: 2023-12-5 11:02:20
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "demo_ftps.h"
#include "simcom_ftps.h"
#include "simcom_os.h"
#include "simcom_uart.h"
#include "simcom_common.h"


#define LOG_ERROR(...) sal_log_error("[DEMO-FTPS] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-FTPS] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-FTPS] " __VA_ARGS__)

static ret_msg_t ftps_init_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ftps_login_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ftps_list_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ftps_logout_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ftps_mkd_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ftps_pwd_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ftps_cwd_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ftps_download_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ftps_downloadfile_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ftps_uploadfile_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ftps_deletefile_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ftps_deletedir_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ftps_settype_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ftps_setmode_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ftps_size_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t ftps_deinit_cmd_proc(op_t op, arg_t *argv, int argn);
/**/
enum appChannelType channeltype;
extern int ftpsTestTime;
extern int ftpsCurrentTime;
sMsgQRef ftpsUIResp_msgq;
SIM_MSG_T ftpsMsg = {0, 0, 0, NULL};
SCapiFtpsData *ftpsData;
SC_STATUS status;
char *dir = NULL;
extern void FtpsTestDemoInit(void);
/**/
/*ARGS END*/

/*RANGE Start*/
const value_t type[2] =
{
    {
        .bufp = "A",
        .size = 1,
    },
    {
        .bufp = "I",
        .size = 1,
    },
};
const value_t mode[2] =
{
    {
        .bufp = "0",
        .size = 1,
    },
    {
        .bufp = "1",
        .size = 1,
    },
};
const value_t server[4] =
{
    {
        .bufp = "0",
        .size = 1,
    },
    {
        .bufp = "1",
        .size = 1,
    },
        {
        .bufp = "2",
        .size = 1,
    },
        {
        .bufp = "3",
        .size = 1,
    },
};
/*RANGE End*/
/*ARGS START*/
const value_list_t type_range =
{
    .isrange = false,
    .list_head = type,
    .count = 2,
};
const value_list_t server_range =
{
    .isrange = false,
    .list_head = server,
    .count = 4,
};
const value_list_t mode_range =
{
    .isrange = false,
    .list_head = mode,
    .count = 2,
};
arg_t login_args[5] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGFTPS_HOST,
        .msg ="maximum length is 128",
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGFTPS_PORT,
        .msg ="the range is from 1 to 65535",
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGFTPS_USERNAME,
        .msg ="maximum length is 128",
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGFTPS_PASSWORD,
        .msg ="maximum length is 128",
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGFTPS_SERVER_TYPE,
        .value_range =&server_range,
    }
};
arg_t file_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGFTPS_FILENAME,
    }
};
arg_t mode_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGFTPS_MODE,
        .value_range =&mode_range,
    }
};

arg_t type_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGFTPS_TYPE,
        .value_range =&type_range,
    }
};
arg_t demotest_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGFTPS_DEMO_TEST_TIME,
    }
};
/*MENU LIST START*/
const menu_list_t func_init =
{
    .menu_name = "FTP Init",
    .menu_type = TYPE_FUNC,
    .parent = &ftps_menu,
    .function = {
        .methods.cmd_handler = ftps_init_cmd_proc,
    },
};

const menu_list_t func_deinit =
{
    .menu_name = "FTP Deinit",
    .menu_type = TYPE_FUNC,
    .parent = &ftps_menu,
    .function = {
        .methods.cmd_handler = ftps_deinit_cmd_proc,
    },
};
const menu_list_t func_login =
{
    .menu_name = "FTP Login",
    .menu_type = TYPE_FUNC,
    .parent = &ftps_menu,
    .function = {
        .argv = login_args,
        .argn = sizeof(login_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_login_cmd_proc,
    },
};
const menu_list_t func_logout =
{
    .menu_name = "FTP Logout",
    .menu_type = TYPE_FUNC,
    .parent = &ftps_menu,
    .function = {
        .methods.cmd_handler = ftps_logout_cmd_proc,
    },
};
const menu_list_t func_list =
{
    .menu_name = "FTP List",
    .menu_type = TYPE_FUNC,
    .parent = &ftps_menu,
    .function = {
        .argv = file_args,
        .argn = sizeof(file_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_list_cmd_proc,
    },
};
const menu_list_t func_mkd =
{
    .menu_name = "FTP Mkd",
    .menu_type = TYPE_FUNC,
    .parent = &ftps_menu,
    .function = {
        .argv = file_args,
        .argn = sizeof(file_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_mkd_cmd_proc,
    },
};
const menu_list_t func_pwd =
{
    .menu_name = "FTP Pwd",
    .menu_type = TYPE_FUNC,
    .parent = &ftps_menu,
    .function = {
        .methods.cmd_handler = ftps_pwd_cmd_proc,
    },
};
const menu_list_t func_cwd =
{
    .menu_name = "FTP Cwd",
    .menu_type = TYPE_FUNC,
    .parent = &ftps_menu,
    .function = {
        .argv = file_args,
        .argn = sizeof(file_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_cwd_cmd_proc,
    },
};
const menu_list_t func_download =
{
    .menu_name = "FTP download",
    .menu_type = TYPE_FUNC,
    .parent = &ftps_menu,
    .function = {
        .argv = file_args,
        .argn = sizeof(file_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_download_cmd_proc,
    },
};
const menu_list_t func_downloadfile =
{
    .menu_name = "FTP download file",
    .menu_type = TYPE_FUNC,
    .parent = &ftps_menu,
    .function = {
        .argv = file_args,
        .argn = sizeof(file_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_downloadfile_cmd_proc,
    },
};
const menu_list_t func_uploadfile =
{
    .menu_name = "FTP upload file",
    .menu_type = TYPE_FUNC,
    .parent = &ftps_menu,
    .function = {
        .argv = file_args,
        .argn = sizeof(file_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_uploadfile_cmd_proc,
    },
};
const menu_list_t func_deletefile =
{
    .menu_name = "FTP delete file",
    .menu_type = TYPE_FUNC,
    .parent = &ftps_menu,
    .function = {
        .argv = file_args,
        .argn = sizeof(file_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_deletefile_cmd_proc,
    },
};
const menu_list_t func_deletedir =
{
    .menu_name = "FTP RMD",
    .menu_type = TYPE_FUNC,
    .parent = &ftps_menu,
    .function = {
        .argv = file_args,
        .argn = sizeof(file_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_deletedir_cmd_proc,
    },
};
const menu_list_t func_setmode =
{
    .menu_name = "FTP set mode",
    .menu_type = TYPE_FUNC,
    .parent = &ftps_menu,
    .function = {
        .argv = mode_args,
        .argn = sizeof(mode_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_setmode_cmd_proc,
    },
};
const menu_list_t func_settype =
{
    .menu_name = "FTP set type",
    .menu_type = TYPE_FUNC,
    .parent = &ftps_menu,
    .function = {
        .argv = type_args,
        .argn = sizeof(type_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_settype_cmd_proc,
    },
};
const menu_list_t func_size =
{
    .menu_name = "FTP size",
    .menu_type = TYPE_FUNC,
    .parent = &ftps_menu,
    .function = {
        .argv = file_args,
        .argn = sizeof(file_args) / sizeof(arg_t),
        .methods.cmd_handler = ftps_size_cmd_proc,
    },
};
const menu_list_t *ftps_menu_sub[]=
 {
    &func_init,
    &func_deinit,
    &func_login,
    &func_logout,
    &func_list,
    &func_mkd,
    &func_pwd,
    &func_cwd,
    &func_download,
    &func_downloadfile,
    &func_uploadfile,
    &func_deletefile,
    &func_deletedir,
    &func_setmode,
    &func_settype,
    &func_size,
    NULL  // MUST end by NULL
 };
 const menu_list_t ftps_menu =
 {
    .menu_name = "ftps",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = ftps_menu_sub,
    .parent = &root_menu,
 };
/*MENU LIST END*/
/*CMD PROC START*/
static ret_msg_t ftps_init_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    if (NULL == ftpsUIResp_msgq)
    {
        SC_STATUS status;
        status = sAPI_MsgQCreate(&ftpsUIResp_msgq, "ftpsUIResp_msgq", sizeof(SIM_MSG_T), 20, SC_FIFO);
        if (SC_SUCCESS != status)
        {
            LOG_ERROR("msgQ create fail");
            ret.msg ="FTP Init Fail!";
        }
    }
    sAPI_FtpsInit(SC_FTPS_USB, ftpsUIResp_msgq);

    sAPI_MsgQRecv(ftpsUIResp_msgq, &ftpsMsg, 12000);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    LOG_INFO("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]", channeltype, ftpsMsg.msg_id, ftpsMsg.arg1,
                ftpsMsg.arg2);
    ret.errcode = ftpsMsg.arg2;
    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        LOG_INFO("Init SUCCESS");
        ret.msg = "FTP Init Successful!";
    }
    else
    {
        LOG_INFO("Init FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP Init Fail!";
    }
    return ret;
}

static ret_msg_t ftps_login_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    LOG_INFO("FTPS_login");
    SCftpsLoginMsg msg = {0};
    const char *in_host = NULL;
    int in_port = 0;
    const char *in_username = NULL;
    const char *in_password = NULL;
    const char *servertype = NULL;
    int server_type;
    pl_demo_get_data(&in_host, argv, argn, ARGFTPS_HOST);
    memcpy(msg.host, in_host, sizeof(msg.host));
    pl_demo_get_value(&in_port, argv, argn, ARGFTPS_PORT);
    msg.port = in_port;
    pl_demo_get_data(&in_username, argv, argn, ARGFTPS_USERNAME);
    memcpy(msg.username,in_username,sizeof(msg.username));
    pl_demo_get_data(&in_password, argv, argn, ARGFTPS_PASSWORD);
    memcpy(msg.password,in_password,sizeof(msg.password));
    pl_demo_get_data(&servertype, argv, argn, ARGFTPS_SERVER_TYPE);
    server_type = atoi(servertype);
    msg.serverType = server_type;

    LOG_INFO("host = [%s],user = [%s],pass = [%s],servertype = [%d]", msg.host, msg.username, msg.password);

    sAPI_FtpsLogin(msg);
    sAPI_MsgQRecv(ftpsUIResp_msgq, &ftpsMsg, 12000);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    LOG_INFO("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",
                channeltype, ftpsMsg.msg_id, ftpsMsg.arg1, ftpsMsg.arg2);
    ret.errcode = ftpsMsg.arg2;
    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        LOG_INFO("LOGIN SUCCESSFUL");
        ret.msg = "FTP Login Successful!";
        return ret;
    }
    else
    {
        LOG_ERROR("LOGIN ERROR,ERROR CODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP Login Fail!";
        return ret;
    }
    return ret;
}

static ret_msg_t ftps_logout_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    LOG_INFO("FTPS_logout");
    sAPI_FtpsLogout();
    sAPI_MsgQRecv(ftpsUIResp_msgq, &ftpsMsg, 12000);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    LOG_INFO("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]", channeltype, ftpsMsg.msg_id, ftpsMsg.arg1,
                ftpsMsg.arg2);
    ret.errcode = ftpsMsg.arg2;
    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        LOG_INFO("LOGOUT SUCCESSFUL");
        ret.msg = "FTP Logout Successful!";
        return ret;
    }
    else
    {
        LOG_ERROR("LOGOUT FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP Logout Fail!";
        return ret;
    }
    return ret;
}

static ret_msg_t ftps_list_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *filePath = NULL;
    int ret_value = 0;
    arg_t out_arg[1] = {0};
    LOG_INFO("FTPS_list");
    pl_demo_get_data(&filePath, argv, argn, ARGFTPS_FILENAME);

    ret_value = sAPI_FtpsList((char *)filePath);
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        LOG_INFO("LIST FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP LIST Fail!";
        ret.errcode = ftpsMsg.arg2;
        return ret;
    }

    while (1)
    {
        LOG_INFO("COMING TO THE RECV");
        ftpsMsg.arg3 = NULL;
        status = sAPI_MsgQRecv(ftpsUIResp_msgq, &ftpsMsg, 12000);
        LOG_INFO("status = [%d]", status);
        channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
        LOG_INFO("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]", channeltype, ftpsMsg.msg_id, ftpsMsg.arg1,
                    ftpsMsg.arg2);
        if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
        {
            ftpsData = (SCapiFtpsData *)ftpsMsg.arg3;
            LOG_INFO("flag = [%d]", ftpsData->flag);
            if (SC_DATA_COMPLETE == ftpsData->flag)
            {
                LOG_INFO("get data complete!");
                LOG_INFO("flag = [%d],len = [%d]", ftpsData->flag, ftpsData->len);
                free(ftpsData);
                break;
            }
            else if (SC_DATA_RESUME == ftpsData->flag)
            {
                LOG_INFO("get data successful!");
                LOG_INFO("flag = [%d],len = [%d]", ftpsData->flag, ftpsData->len);

                //just for waiting uart ok now
                out_arg[0].arg_name = pre_arg_name[ARG_ID_DATA];
                out_arg[0].type = TYPE_RAW;
                out_arg[0].value = pl_demo_make_value(0, ftpsData->data, ftpsData->len);
                pl_demo_post_data(OP_POST, out_arg,1,NULL, 0);

                pl_demo_release_value(out_arg[0].value);
                free(ftpsData->data);
                free(ftpsData);
            }
            else
            {
                LOG_ERROR("ERROR HAPPEN!");
                break;
            }
        }
        else
        {
            LOG_ERROR("error code = [%d]", ftpsMsg.arg2);
            ret.msg = "Ftp list fail!";
            break;
        }
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}

    
static ret_msg_t ftps_mkd_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *filePath = NULL;
    LOG_INFO("sAPI_FtpsCreateDirectory");
    pl_demo_get_data(&filePath, argv, argn, ARGFTPS_FILENAME);
    LOG_INFO("filePath = [%s]", filePath);

    sAPI_FtpsCreateDirectory((char *)filePath);
    sAPI_MsgQRecv(ftpsUIResp_msgq, &ftpsMsg, 12000);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    LOG_INFO("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]", channeltype, ftpsMsg.msg_id, ftpsMsg.arg1,
                ftpsMsg.arg2);
    ret.errcode = ftpsMsg.arg2;
    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        LOG_INFO("Create Directory Successful");
        ret.msg = "Create Directory Successful!";
    }
    else
    {
        LOG_ERROR("Create Directory FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "Craete Directory Fail!";
    }
    return ret;
}
    
static ret_msg_t ftps_pwd_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char path[256] = {0};
    LOG_INFO("FTPS_getCurrentDirectory");
    sAPI_FtpsGetCurrentDirectory();
    sAPI_MsgQRecv(ftpsUIResp_msgq, &ftpsMsg, 12000);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    LOG_INFO("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]", channeltype, ftpsMsg.msg_id, ftpsMsg.arg1,
                ftpsMsg.arg2);

    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        dir = (char *)ftpsMsg.arg3;
        LOG_INFO("GET CURRENT DIRECTORY SUCCESS = [%s]", dir);
        snprintf(path, sizeof(path), "current path = %s", dir);
        ret.msg= path;
    }
    else
    {
        LOG_ERROR("GET CURRENT DIRECTORY FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "Get Directory Fail";
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}
    
static ret_msg_t ftps_cwd_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *filePath = NULL;
    LOG_INFO("sAPI_FtpsChangeDirectory");
    pl_demo_get_data(&filePath, argv, argn, ARGFTPS_FILENAME);
    LOG_INFO("filePath = [%s]", filePath);

    sAPI_FtpsChangeDirectory((char *)filePath);
    sAPI_MsgQRecv(ftpsUIResp_msgq, &ftpsMsg, 12000);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    LOG_INFO("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]", channeltype, ftpsMsg.msg_id, ftpsMsg.arg1,
                ftpsMsg.arg2);

    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        LOG_INFO("Change Directory Successful");
        ret.msg = "Change Directory Successful!";
    }
    else
    {
        LOG_ERROR("Change Directory FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "Change Directory Fail!";
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}
    
static ret_msg_t ftps_download_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *filePath = NULL;
    int ret_value = 0;
    arg_t out_arg[1] = {0};
    LOG_INFO("sAPI_FtpsDownloadFileToBuffer");
    pl_demo_get_data(&filePath, argv, argn, ARGFTPS_FILENAME);
    LOG_INFO("filePath = [%s]", filePath);

    ret_value = sAPI_FtpsDownloadFileToBuffer((char *)filePath, 0);
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        LOG_ERROR("DownloadFileToBuffer FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP DownloadFileToBuffer Fail!";
        ret.errcode = ftpsMsg.arg2;
        return ret;
    }

    while (1)
    {
        LOG_INFO("COMING TO THE RECV");
        ftpsMsg.arg3 = NULL;
        status = sAPI_MsgQRecv(ftpsUIResp_msgq, &ftpsMsg, 12000);
        LOG_INFO("status = [%d]", status);
        channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
        LOG_INFO("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]", channeltype, ftpsMsg.msg_id, ftpsMsg.arg1,
                    ftpsMsg.arg2);
        if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
        {
            ftpsData = (SCapiFtpsData *)ftpsMsg.arg3;
            LOG_INFO("flag = [%d]", ftpsData->flag);
            if (SC_DATA_COMPLETE == ftpsData->flag)
            {
                LOG_INFO("get data complete!");
                LOG_INFO("flag = [%d],len = [%d]", ftpsData->flag, ftpsData->len);
                free(ftpsData);
                break;
            }
            else if (SC_DATA_RESUME == ftpsData->flag)
            {
                LOG_INFO("get data successful!");
                LOG_INFO("flag = [%d],len = [%d]", ftpsData->flag, ftpsData->len);

                //just for waiting uart ok now
                out_arg[0].arg_name = pre_arg_name[ARG_ID_DATA];
                out_arg[0].type = TYPE_RAW;
                out_arg[0].value = pl_demo_make_value(0, ftpsData->data, ftpsData->len);
                pl_demo_post_data(OP_POST, out_arg,1,NULL, 0);

                pl_demo_release_value(out_arg[0].value);

                free(ftpsData->data);
                free(ftpsData);
            }
            else
            {
                LOG_ERROR("ERROR HAPPEN!");
                break;
            }
        }
        else
        {
            LOG_ERROR("error code = [%d]", ftpsMsg.arg2);
            ret.msg="FTP DOWNLOAD FILE TO BUFFER FAIL!";
            break;
        }

    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}
    
static ret_msg_t ftps_downloadfile_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *filePath = NULL;
    int ret_value = 0;
    LOG_INFO("sAPI_FtpsDownloadFile");
    pl_demo_get_data(&filePath, argv, argn, ARGFTPS_FILENAME);
    LOG_INFO("filePath = [%s]", filePath);

    ret_value = sAPI_FtpsDownloadFile((char *)filePath, SC_FTPS_FILE_FLASH);
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        LOG_ERROR("DownloadFile FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP DownloadFile Fail!";
        ret.errcode = ftpsMsg.arg2;
        return ret;
    }
    sAPI_MsgQRecv(ftpsUIResp_msgq, &ftpsMsg, 12000);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    LOG_INFO("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]", channeltype, ftpsMsg.msg_id, ftpsMsg.arg1,
                ftpsMsg.arg2);

    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        LOG_INFO("DOWNLOAD FILE SUCCESSFUL");
        ret.msg = "Download File Successful";
    }
    else
    {
        LOG_ERROR("DOWNLOAD FILE FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "Download File Fail";
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}
    

static ret_msg_t ftps_uploadfile_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *filePath = NULL;
    int ret_value = 0;
    LOG_INFO("sAPI_FtpsUploadFile");
    pl_demo_get_data(&filePath, argv, argn, ARGFTPS_FILENAME);
    LOG_INFO("filePath = [%s]", filePath);

    ret_value = sAPI_FtpsUploadFile((char *)filePath, SC_FTPS_FILE_FLASH, 0);
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        LOG_ERROR("UploadFile FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP UploadFile Fail!";
        ret.errcode = ftpsMsg.arg2;
        return ret;
    }
    sAPI_MsgQRecv(ftpsUIResp_msgq, &ftpsMsg, 12000);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    LOG_INFO("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]", channeltype, ftpsMsg.msg_id, ftpsMsg.arg1,
                ftpsMsg.arg2);

    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        LOG_INFO("UPLOAD FILE SUCCESSFUL");
        ret.msg = "Upload File Successful";
    }
    else
    {
        LOG_ERROR("UPLOAD FILE FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "Upload File Fail";
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}
    
static ret_msg_t ftps_deletefile_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *filePath = NULL;
    int ret_value = 0;
    LOG_INFO("sAPI_FtpsDeleteFile");
    pl_demo_get_data(&filePath, argv, argn, ARGFTPS_FILENAME);
    LOG_INFO("filePath = [%s]", filePath);

    ret_value = sAPI_FtpsDeleteFile((char *)filePath);
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        LOG_ERROR("DeleteFile FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP DeleteFile Fail!";
        ret.errcode = ftpsMsg.arg2;
        return ret;
    }
    sAPI_MsgQRecv(ftpsUIResp_msgq, &ftpsMsg, 12000);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    LOG_INFO("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]", channeltype, ftpsMsg.msg_id, ftpsMsg.arg1,
                ftpsMsg.arg2);

    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        LOG_INFO("Delete File Successful");
        ret.msg = "Delete File Successful!";
    }
    else
    {
        LOG_ERROR("Delete File Fail");
        ret.msg = "Delete File Fail!";
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}
    
static ret_msg_t ftps_deletedir_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *filePath = NULL;
    int ret_value = 0;
    LOG_INFO("sAPI_FtpsDeleteDirectory");
    pl_demo_get_data(&filePath, argv, argn, ARGFTPS_FILENAME);
    LOG_INFO("filePath = [%s]", filePath);

    ret_value = sAPI_FtpsDeleteDirectory((char *)filePath);
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        LOG_ERROR("DeleteFile FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP DeleteFile Fail!";
        ret.errcode = ftpsMsg.arg2;
        return ret;
    }
    sAPI_MsgQRecv(ftpsUIResp_msgq, &ftpsMsg, 12000);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    LOG_INFO("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]", channeltype, ftpsMsg.msg_id, ftpsMsg.arg1,
                ftpsMsg.arg2);

    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        LOG_INFO("Delete Directory Successful");
        ret.msg = "Delete Directory Successful!";
    }
    else
    {
        LOG_ERROR("Delete Directory Fail");
        ret.msg = "Delete Directory Fail!";
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}
    
static ret_msg_t ftps_settype_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *type = NULL;
    int ret_value = 0;
    LOG_INFO("sAPI_FtpsTransferType");
    pl_demo_get_data(&type, argv, argn, ARGFTPS_TYPE);
    LOG_INFO("arg3 = [%s]",type);

    ret_value = sAPI_FtpsTransferType((char *)type);
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        LOG_ERROR("SetType FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP Settype Fail!";
        ret.errcode = ftpsMsg.arg2;
        return ret;
    }
    sAPI_MsgQRecv(ftpsUIResp_msgq,&ftpsMsg,12000);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    LOG_INFO("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);
    
    if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        LOG_INFO("SETTYPE SUCCESSFUL");
        ret.msg = "SETTYPE Successful"; 
    }
    else
    {
        LOG_ERROR("SETTYPE FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
        ret.msg = "SETTYPE Fail"; 
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}
    
static ret_msg_t ftps_setmode_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int mode = 0;
    int ret_value = 0;
    pl_demo_post_comments("Please input mdoe.0:PORT 1:PASV");
    LOG_INFO("sAPI_FtpsSetMode");
    pl_demo_get_value(&mode, argv, argn, ARGFTPS_MODE);
    LOG_INFO("arg3 = [%s]",mode);

    ret_value = sAPI_FtpsSetMode(mode);
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        LOG_ERROR("SetMode FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP SetMode Fail!";
        ret.errcode = ftpsMsg.arg2;
        return ret;
    }
    sAPI_MsgQRecv(ftpsUIResp_msgq,&ftpsMsg,12000);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    LOG_INFO("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);
    
    if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        LOG_INFO("SETMODE SUCCESSFUL");
        ret.msg = "SETMODE Successful"; 
    }
    else
    {
        LOG_ERROR("SETMODE FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
        ret.msg = "SETMODE Fail"; 
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}
    
static ret_msg_t ftps_size_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *filePath = NULL;
    int ret_value = 0;
    char path[256] = {0};
    int *size = NULL;
    pl_demo_post_comments("Please input File name.");
    LOG_INFO("sAPI_FtpsGetFileSize");
    pl_demo_get_data(&filePath, argv, argn, ARGFTPS_FILENAME);
    LOG_INFO("filePath = [%s]", filePath);

    ret_value = sAPI_FtpsGetFileSize((char *)filePath);
    if (ret_value != SC_FTPS_RESULT_OK)
    {
        LOG_INFO("DeleteFile FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "FTP DeleteFile Fail!";
        ret.errcode = ftpsMsg.arg2;
        return ret;
    }
    sAPI_MsgQRecv(ftpsUIResp_msgq, &ftpsMsg, 12000);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    LOG_INFO("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]", channeltype, ftpsMsg.msg_id, ftpsMsg.arg1,
                ftpsMsg.arg2);

    if (SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        size = (int *)ftpsMsg.arg3;
        LOG_INFO("GET SIZE SUCESS,SIZE = [%d]", *size);
        snprintf(path, sizeof(path), "file size = %d", *size);
        ret.msg= path;
        free(size);
    }
    else
    {
        LOG_ERROR("GET SIZE FAIL,ERRCODE = [%d]", ftpsMsg.arg2);
        ret.msg = "Get File Size Fail!";
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}
static ret_msg_t ftps_deinit_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    LOG_INFO("FTPS_DeInit");
    sAPI_FtpsDeInit(SC_FTPS_USB);
    status = sAPI_MsgQRecv(ftpsUIResp_msgq,&ftpsMsg,12000);
    LOG_INFO("status = [%d]", status);
    channeltype = GET_SRV_FROM_MSG_ID(ftpsMsg.msg_id);
    LOG_INFO("SRV = [%d],msgId = [%d],api = [%d],errCode = [%d]",channeltype,ftpsMsg.msg_id,ftpsMsg.arg1,ftpsMsg.arg2);

    if(SC_FTPS_RESULT_OK == ftpsMsg.arg2)
    {
        LOG_INFO("DEINIT SUCCESS");
        ret.msg = "FTP DeInit Successful!";
    }
    else
    {
        LOG_ERROR("DEINIT FAIL,ERRCODE = [%d]",ftpsMsg.arg2);
        ret.msg = "FTP DeInit Fail!";
    }
    ret.errcode = ftpsMsg.arg2;
    return ret;
}

/*CMD PROC END*/

