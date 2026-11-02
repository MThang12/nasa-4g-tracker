/*
*@Author: Simcom yi.chen
*@Date: 2023-12-19
*@Last Modified by: Simcom yi.chen
*@Last Modified Date: 2023-12-25
*/

/*------------------------------------*/
/*             header file            */
/*------------------------------------*/
#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"

#include <string.h>
#include <stdlib.h>
#include "simcom_simcard.h"
#include "simcom_os.h"
#include "simcom_debug.h"
#include "simcom_uart.h"
#include "simcom_common.h"
#include "stdio.h"
#include "uart_api.h"

/*------------------------------------*/
/*          macro definition          */
/*------------------------------------*/
#define LOG_ERROR(...) sal_log_error("[DEMO_SIMCARD]"__VA_ARGS__)
#define LOG_INFO(...) sal_log_info("[DEMO_SIMCARD]"__VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO_SIMCARD]"__VA_ARGS__)

#define SIMCARD_URC_RECIVE_TIME_OUT 2000
#define BUFF_LEN 120
#define BUFF_LARGE 650

#define ARGSIM_OUT              "pro_res"
#define ARGSIM_SWITCHSIM        "SIMID"
#define ARGSIM_SETPLUGIN        "plugIn"
#define ARGSIM_SETPLUGLE        "plugLevel"
#define ARGSIM_SETPLUGOUT       "plugOut"
#define ARGSIM_SETQUERY         "QUERY_OPT"
#define ARGSIM_SETSTATEOPT      "SET_OPT"
#define ARGSIM_SETSTATE         "SET_TYPE"
#define ARGSIM_PINOPT           "OPT"
#define ARGSIM_PINOLD           "PIN"
#define ARGSIM_PINNEW           "NEW_PIN"
#define ARGSIM_BINDSIM          "AT_channel"
#define ARGSIM_Restric_cmd      "CMD_Name"
#define ARGSIM_Restric_fID      "FileID"
#define ARGSIM_Restric_p1       "P1"
#define ARGSIM_Restric_p2       "P2"
#define ARGSIM_Restric_p3       "P3"
#define ARGSIM_Restric_data     "Data"
#define ARGSIM_Restric_pID      "PathID"
#define ARGSIM_DUALSIM          "DualSIMType"
#define ARGSIM_SIMLOCK          "OPT"
#define ARGSIM_LCKPIN           "PIN"
#define ARGSIM_Generic_cmdL     "CMD_Len"
#define ARGSIM_Generic_cmd      "CMD_Name"

#define MSGSIM_FLAG             "g_simcard_demo_msgQ"

/*------------------------------------*/
/*        function declaration        */
/*------------------------------------*/
#if defined (LWIP_IPNETBUF_SUPPORT) && defined (FEATURE_SIMCOM_DUALSIM)
static ret_msg_t SwitchSIM_proc(op_t op, arg_t *argv, int argn);
#if !(defined SINGLE_SIM) && !(defined FEATURE_SIMCOM_DSSS)
static ret_msg_t getBindsim_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t setBindsim_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_DualsimType_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_DualsimType_proc(op_t op, arg_t *argv, int argn);
#endif
#endif

#ifdef FEATURE_SIMCOM_HOTPLUG
static ret_msg_t Query_plugin_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t Query_pluglevel_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_plugin_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_pluglevel_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t Query_plugout_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_plugout_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_Query_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_STATE_proc(op_t op, arg_t *argv, int argn);
#endif
static ret_msg_t setPin_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t getPin_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t getPinEX_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t getICCID_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t getIMSI_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t getHPLMN_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t SIM_Restricted_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_SIMlock_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t SIM_Generic_proc(op_t op, arg_t *argv, int argn);

/*------------------------------------*/
/*           global variable          */
/*------------------------------------*/
#ifdef FEATURE_SIMCOM_HOTPLUG
sMsgQRef g_simcard_demo_msgQ;
const menu_list_t Hotplug_menu;
const menu_list_t HotplugEX_menu;
#endif

/*------------------------------------*/
/*              switchsim             */
/*------------------------------------*/
//param config
const value_t SIM_ID[2] =
{
    {
        .bufp = "0:SIM1",
        .size = 7,
    },
    {
        .bufp = "1:SIM2",
        .size = 7,
    },
};

const value_list_t SWITCHSIM =
{
    .isrange = false, .list_head = SIM_ID, .count = 2,
};

#if defined (LWIP_IPNETBUF_SUPPORT) && defined (FEATURE_SIMCOM_DUALSIM)
arg_t SwitchSim_args =
{
    .type = TYPE_STR, .arg_name = ARGSIM_SWITCHSIM, .value_range =&SWITCHSIM
};

//func menu config
const menu_list_t SwitchSim_menu =
{
    .menu_name = "Switch SIM",
    .menu_type = TYPE_FUNC,
    .parent = &Simcard_menu,
    .function =
    {
        .argv = &SwitchSim_args,
        .argn = sizeof(SwitchSim_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = SwitchSIM_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t SwitchSIM_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter!", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    const char *SIMID = NULL;
    arg_t out = { 0 };

    //get input and print LOG for debug
    pl_demo_get_data(&SIMID, argv, argn, ARGSIM_SWITCHSIM);
    sAPI_Debug("%s Info: get SIMID: %s", __func__, SIMID);

    //process input, run SAPI, get result
    if (strcmp(SIMID, "0:SIM1") == 0)
        ret.errcode = sAPI_SimcardSwitchMsg(0, NULL); //This API only return 0--success, 1--fail
    else if (strcmp(SIMID, "1:SIM2") == 0)
        ret.errcode = sAPI_SimcardSwitchMsg(1, NULL);
    else
        ret.errcode = 1;

    //process result
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if (ret.errcode) //result == 1 means fail
    {
        snprintf(rsp_buff, BUFF_LEN, "Failure!");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        ret.msg = "Change MasterSIM Failure!";
    }
    else
    {
        snprintf(rsp_buff, BUFF_LEN, "Success!");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        ret.msg = "Change MasterSIM Success!";
    }

    //post result msg
    if ( pl_demo_post_data(OP_POST, &out, 1, NULL, 0) )
        LOG_ERROR("%s Error: POST msg fail!", __func__);

    sAPI_Debug("[SIM_Demo] %s leave with retcode: %d", __func__, ret.errcode);
    //must free before leave
    pl_demo_release_value(out.value);
    return ret;
}

#if !(defined SINGLE_SIM) && !(defined FEATURE_SIMCOM_DSSS)
/*------------------------------------*/
/*             Bindsim get            */
/*------------------------------------*/
//param config
    //NULL

//func menu config
const menu_list_t Get_Bindsim_menu =
{
    .menu_name = "get Bindsim",
    .menu_type = TYPE_FUNC,
    .parent = &Simcard_menu,
    .function =
    {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = getBindsim_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t getBindsim_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t out = { 0 };
    UINT8 bindsim = 255;

    //operate
    ret.errcode = sAPI_SysGetBindSim(&bindsim);
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if (ret.errcode)
    {
        snprintf(rsp_buff, BUFF_LEN, "Get Bindsim Falied!");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }
    else
    {
        if (bindsim == 0)
            snprintf(rsp_buff, BUFF_LEN, "0:SIM1");
        else if (bindsim == 1)
            snprintf(rsp_buff, BUFF_LEN, "1:SIM2");
        else
            snprintf(rsp_buff, BUFF_LEN, "error");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }

    if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
        LOG_ERROR("%s Error: POST data fail!", __func__);
    sAPI_Debug("[SIM_Demo] %s leave with retcode :%d", __func__, ret.errcode);
    pl_demo_release_value(out.value);
    return ret;
}

/*------------------------------------*/
/*             Bindsim set            */
/*------------------------------------*/
//param config
const value_t Channal_Type[2] =
{
    {
        .bufp = "0:SIM1 AT Channal",
        .size = 18,
    },
    {
        .bufp = "1:SIM2 AT Channal",
        .size = 18,
    },
};

const value_list_t SETATChannal =
{
    .isrange = false, .list_head = Channal_Type, .count = 2,
};

arg_t set_Bindsim_args =
{
    .type = TYPE_STR, .arg_name = ARGSIM_BINDSIM, .value_range =&SETATChannal
};

//func menu config
const menu_list_t Set_Bindsim_menu =
{
    .menu_name = "set Bindsim",
    .menu_type = TYPE_FUNC,
    .parent = &Simcard_menu,
    .function =
    {
        .argv = &set_Bindsim_args,
        .argn = sizeof(set_Bindsim_args)/sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = setBindsim_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t setBindsim_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t out = { 0 };
    const char *opt = NULL;

    //get input
    pl_demo_get_data(&opt, argv, argn, ARGSIM_BINDSIM);
    sAPI_Debug("%s Info: get opt: %s", __func__, opt);

    //process input, run SAPI, get result
    if (strcmp(opt, "0:SIM1 AT Channal") == 0)
        ret.errcode = sAPI_SysSetBindSim(0); //This API only result 0--success, 1--fail
    else if (strcmp(opt, "1:SIM2 AT Channal") == 0)
        ret.errcode = sAPI_SysSetBindSim(1);
    else
        ret.errcode = 1;

    //process result
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if (ret.errcode) //result == 1 means fail
    {
        snprintf(rsp_buff, BUFF_LEN, "Failure");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        ret.msg = "Change SIM AT Channal Failed!";
    }
    else
    {
        snprintf(rsp_buff, BUFF_LEN, "Success");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        ret.msg = "Change SIM AT Channal Success!";
    }

    if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
        LOG_ERROR("%s Error: POST data fail!", __func__);
    sAPI_Debug("[SIM_Demo] %s leave with retcode :%d", __func__, ret.errcode);
    pl_demo_release_value(out.value);
    return ret;
}

/*------------------------------------*/
/*           DualsimType get          */
/*------------------------------------*/
//param config
    //NULL

//func menu config
const menu_list_t Get_DualsimType_menu =
{
    .menu_name = "get DualsimType",
    .menu_type = TYPE_FUNC,
    .parent = &Simcard_menu,
    .function =
    {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = get_DualsimType_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t get_DualsimType_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t out = { 0 };
    UINT8 type = 255;

    //operate
    ret.errcode = sAPI_SysGetDualSimType(&type); // This API only return 0--success, 1--fail
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if (ret.errcode)
    {
        snprintf(rsp_buff, BUFF_LEN, "Get DualSim Type Faild!");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }
    else
    {
        if (type == 0)
            snprintf(rsp_buff, BUFF_LEN, "0: DUAL SIM DUAL STANDBY");
        else if (type == 1)
            snprintf(rsp_buff, BUFF_LEN, "1: DUAL SIM SINGLE STANDBY");
        else if (type == 3)
            snprintf(rsp_buff, BUFF_LEN, "3: DUAL SIM DUAL STANDBY FOR FP");
        else
            snprintf(rsp_buff, BUFF_LEN, "error!");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }

    if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
        LOG_ERROR("%s Error: POST data fail!", __func__);
    sAPI_Debug("[SIM_Demo] %s leave with retcode :%d", __func__, ret.errcode);
    pl_demo_release_value(out.value);
    return ret;
}

/*------------------------------------*/
/*           DualsimType set          */
/*------------------------------------*/
//param config
const value_t DUAL_Type[3] =
{
    {
        .bufp = "0:DUAL SIM DUAL STANDBY",
        .size = 24,
    },
    {
        .bufp = "1:DUAL SIM SINGLE STANDBY",
        .size = 26,
    },
    {
        .bufp = "3:DUAL SIM DUAL STANDBY FOR FP",
        .size = 31,
    },
};

const value_list_t SETDUALSIMTYPE =
{
    .isrange = false, .list_head = DUAL_Type, .count = 3,
};

arg_t set_Dualsim_args =
{
    .type = TYPE_STR, .arg_name = ARGSIM_DUALSIM, .value_range =&SETDUALSIMTYPE
};

//func menu config
const menu_list_t Set_DualsimType_menu =
{
    .menu_name = "set DualsimType",
    .menu_type = TYPE_FUNC,
    .parent = &Simcard_menu,
    .function =
    {
        .argv = &set_Dualsim_args,
        .argn = sizeof(set_Dualsim_args)/sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_DualsimType_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t set_DualsimType_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t out = { 0 };
    const char * opt = NULL;

    //getinput
    pl_demo_get_data(&opt, argv, argn, ARGSIM_DUALSIM);
    sAPI_Debug("%s Info: get opt: %s", __func__, opt);

    //process input, run SAPI, get result
    if (strcmp(opt, "0:DUAL SIM DUAL STANDBY") == 0)
        ret.errcode = sAPI_SysSetDualSimType(0); //This API only return 0--success, 1--fail
    else if (strcmp(opt, "1:DUAL SIM SINGLE STANDBY") == 0)
        ret.errcode = sAPI_SysSetDualSimType(1);
    else if (strcmp(opt, "3:DUAL SIM DUAL STANDBY FOR FP") == 0)
        ret.errcode = sAPI_SysSetDualSimType(3);
    else
        ret.errcode = 1;

    //process result
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if (ret.errcode) //result == 1 means fial
    {
        snprintf(rsp_buff, BUFF_LEN, "Failure");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        ret.msg = "Change DUALSIM Type Failed!";
    }
    else
    {
        snprintf(rsp_buff, BUFF_LEN, "Success");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        ret.msg = "Change DUALSIM Type Success!";
    }

    if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
        LOG_ERROR("%s Error: POST data fail!", __func__);
    sAPI_Debug("[SIM_Demo] %s leave with retcode :%d", __func__, ret.errcode);
    pl_demo_release_value(out.value);
    return ret;
}
#endif
#endif

#ifdef FEATURE_SIMCOM_HOTPLUG
/*------------------------------------*/
/*         Query plugIn state         */
/*------------------------------------*/
//param config
    // NULL

//func menu config
const menu_list_t Query_plugIn_menu =
{
    .menu_name = "plugIn query",
    .menu_type = TYPE_FUNC,
    .parent = &Hotplug_menu,
    .function =
    {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = Query_plugin_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t Query_plugin_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter!", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t out;
    SC_HotSwapCmdType_e opt = SC_HOTSWAP_QUERY_STATE;
    __attribute__((__unused__)) SC_STATUS status;
    SIM_MSG_T msg = { 0 };

    //create message flag
    status = sAPI_MsgQCreate(&g_simcard_demo_msgQ, MSGSIM_FLAG, sizeof(SIM_MSG_T), 4, SC_FIFO);
    LOG_INFO("%s Info: create status: %d", __func__, status);
    if (status)
        goto END;

    //operate and get result
    ret.errcode = sAPI_SimcardHotSwapMsg(opt, 0, g_simcard_demo_msgQ);
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if (ret.errcode)
    {
        //API get error
        snprintf(rsp_buff, BUFF_LEN, "process sAPI_SimcardHotSwapMsg Fail!");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }
    else
    {
        status = sAPI_MsgQRecv(g_simcard_demo_msgQ, &msg, SIMCARD_URC_RECIVE_TIME_OUT); //This API will return 0--RHINO_SUCCESS when process success
        LOG_INFO("%s Info: get status: %d", __func__, status);
        if (status)
        {
        //receive msg fail
            snprintf(rsp_buff, BUFF_LEN, "get sAPI_SimcardHotSwapMsg return msg Fail!");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        }
        else
        {
            if (msg.arg2 == 1)
            {
                snprintf(rsp_buff, BUFF_LEN, "1:ENABLE");
                out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            }
            else if (msg.arg2 == 0)
            {
                snprintf(rsp_buff, BUFF_LEN, "0:DISABLE");
                out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            }
            else
            {
                snprintf(rsp_buff, BUFF_LEN, "ERROR");
                out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            }
        }
    }

END:
    if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
        LOG_ERROR("%s Error: POST data fail!", __func__);
    sAPI_Debug("[SIM_Demo] %s leave with retcode: %d", __func__, ret.errcode);
    sAPI_MsgQDelete(g_simcard_demo_msgQ);
    pl_demo_release_value(out.value);
    return ret;
}

/*------------------------------------*/
/*       Query plugLevel state        */
/*------------------------------------*/
//param config
    //NULL

//func menu config
const menu_list_t Query_plugLevel_menu =
{
    .menu_name = "plugLevel query",
    .menu_type = TYPE_FUNC,
    .parent = &Hotplug_menu,
    .function =
    {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = Query_pluglevel_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t Query_pluglevel_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter!", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t out;
    SC_HotSwapCmdType_e opt = SC_HOTSWAP_QUERY_LEVEL;
    __attribute__((__unused__)) SC_STATUS status;
    SIM_MSG_T msg = { 0 };

    //create flag
    status = sAPI_MsgQCreate(&g_simcard_demo_msgQ, MSGSIM_FLAG, sizeof(SIM_MSG_T), 4, SC_FIFO);
    LOG_INFO("%s Info: create status: %d", __func__, status);
    if (status)
        goto END;

    //operate and get result
    ret.errcode = sAPI_SimcardHotSwapMsg(opt, 0, g_simcard_demo_msgQ);
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if (ret.errcode)
    {
        //API get error
        snprintf(rsp_buff, BUFF_LEN, "process sAPI_SimcardHotSwapMsg Fail!");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }
    else
    {
        status = sAPI_MsgQRecv(g_simcard_demo_msgQ, &msg, SIMCARD_URC_RECIVE_TIME_OUT); //This API will return 0--RHINO_SUCCESS when process success
        LOG_INFO("%s Info: get status: %d", __func__, status);
        if (status)
        {
        //receive msg fail
            snprintf(rsp_buff, BUFF_LEN, "get sAPI_SimcardHotSwapMsg return msg Fail!");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        }
        else
        {
            if (msg.arg2 == 1)
            {
                snprintf(rsp_buff, BUFF_LEN, "1:HIGH Level");
                out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            }
            else if (msg.arg2 == 0)
            {
                snprintf(rsp_buff, BUFF_LEN, "0:LOW Level");
                out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            }
            else
            {
                snprintf(rsp_buff, BUFF_LEN, "ERROR");
                out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            }
        }
    }

END:
    if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
        LOG_ERROR("%s Error: POST data fail!", __func__);
    sAPI_Debug("[SIM_Demo] %s leave with retcode: %d", __func__, ret.errcode);
    sAPI_MsgQDelete(g_simcard_demo_msgQ);
    pl_demo_release_value(out.value);
    return ret;
}

/*------------------------------------*/
/*          set plugIn state          */
/*------------------------------------*/
//param config
const value_t STATUS_TYPE[2] =
{
    {
        .bufp = "0:DISABLE",
        .size = 10,
    },
    {
        .bufp = "1:ENABLE",
        .size = 9,
    },
};

const value_list_t SWITCHSTATUS =
{
    .isrange = false, .list_head = STATUS_TYPE, .count = 2,
};

arg_t setPlugIn_args =
{
    .type = TYPE_STR, .arg_name = ARGSIM_SETPLUGIN, .value_range = &SWITCHSTATUS
};

//func menu config
const menu_list_t Set_plugIn_menu =
{
    .menu_name = "Set plugIn state",
    .menu_type = TYPE_FUNC,
    .parent = &Hotplug_menu,
    .function =
    {
        .argv = &setPlugIn_args,
        .argn = sizeof(setPlugIn_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_plugin_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t set_plugin_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t out = { 0 };
    SC_HotSwapCmdType_e opt = SC_HOTSWAP_SET_SWITCH;
    const char *input;
    int state = 255;

    //get input
    pl_demo_get_data(&input, argv, argn, ARGSIM_SETPLUGIN);
    LOG_INFO("%s Info: get input: %s", __func__, input);

    //process input
    if (strcmp(input, "0:DISABLE") == 0)
        state = 0;
    else if (strcmp(input, "1:ENABLE") == 0)
        state = 1;
    else
        state = -1;

    //run SAPI, get result
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if ((state == 0) || (state == 1))
    {
        ret.errcode = sAPI_SimcardHotSwapMsg(opt, state, g_simcard_demo_msgQ);
        if (ret.errcode)
        {
        //API get error
            snprintf(rsp_buff, BUFF_LEN, "Failure!");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            ret.msg = "sAPI_SimcardHotSwapMsg set plugin fail";
        }
        else
        {
            snprintf(rsp_buff, BUFF_LEN, "Success!");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            ret.msg = "sAPI_SimcardHotSwapMsg set plugin success";
        }
    }
    else
    {
    //Invalid Parameter
        snprintf(rsp_buff, BUFF_LEN, "Invalid Parameter");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }

    if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
        LOG_ERROR("%s Error: POST data fail!", __func__);
    sAPI_Debug("[SIM_Demo] %s leave with retcode :%d", __func__, ret.errcode);
    pl_demo_release_value(out.value);
    return ret;
}

/*------------------------------------*/
/*        set plugLevel state         */
/*------------------------------------*/
//param config
const value_t LEVEL_TYPE[2] =
{
    {
        .bufp = "0:LOW LEVEL",
        .size = 12,
    },
    {
        .bufp = "1:HIGH LEVEL",
        .size = 13,
    },
};

const value_list_t SWITCHLEVEL =
{
    .isrange = false, .list_head = LEVEL_TYPE, .count = 2,
};

arg_t setPlugLevel_args =
{
    .type = TYPE_STR, .arg_name = ARGSIM_SETPLUGLE, .value_range = &SWITCHLEVEL
};

//func menu config
const menu_list_t set_plugLevel_menu =
{
    .menu_name = "Set plugLevel state",
    .menu_type = TYPE_FUNC,
    .parent = &Hotplug_menu,
    .function =
    {
        .argv = &setPlugLevel_args,
        .argn = sizeof(setPlugLevel_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_pluglevel_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t set_pluglevel_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t out = { 0 };
    SC_HotSwapCmdType_e opt = SC_HOTSWAP_SET_LEVEL;
    const char *input;
    int state = 255;

    //get input
    pl_demo_get_data(&input, argv, argn, ARGSIM_SETPLUGLE);
    LOG_INFO("%s Info: get state: %s", __func__, input);

    //process input
    if (strcmp(input, "0:LOW LEVEL") == 0)
        state = 0;
    else if (strcmp(input, "1:HIGH LEVEL") == 0)
        state = 1;
    else
        state = -1;

    //run SAPI, get result
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if ((state == 0) || (state == 1))
    {
        ret.errcode = sAPI_SimcardHotSwapMsg(opt, state, g_simcard_demo_msgQ);
        if (ret.errcode)
        {
        //API get error
            snprintf(rsp_buff, BUFF_LEN, "Failure!");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            ret.msg = "sAPI_SimcardHotSwapMsg set pluglevel fail";
        }
        else
        {
            snprintf(rsp_buff, BUFF_LEN, "Suceess!");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            ret.msg = "sAPI_SimcardHotSwapMsg Set Pluglevel Succeed";
        }
    }
    else
    {
    //Invalid Parameter
        snprintf(rsp_buff, BUFF_LEN, "Invalid Parameter");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }

    if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
        LOG_ERROR("%s Error: POST data fail!", __func__);
    sAPI_Debug("[SIM_Demo] %s leave with retcode :%d", __func__, ret.errcode);
    pl_demo_release_value(out.value);
    return ret;

}


/*------------------------------------*/
/*        Query plugOut state         */
/*------------------------------------*/
//param config
    //NULL

//func menu config
const menu_list_t Query_plugOut_menu =
{
    .menu_name = "plugOut query",
    .menu_type = TYPE_FUNC,
    .parent = &Hotplug_menu,
    .function =
    {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = Query_plugout_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t Query_plugout_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter!", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t out;
    SC_HotSwapCmdType_e opt = SC_HOTSWAP_QUERY_OUTSTATE;
    __attribute__((__unused__)) SC_STATUS status;
    SIM_MSG_T msg = { 0 };

    //create flag
    status = sAPI_MsgQCreate(&g_simcard_demo_msgQ, MSGSIM_FLAG, sizeof(SIM_MSG_T), 4, SC_FIFO);
    LOG_INFO("%s Info: create status: %d", __func__, status);
    if (status)
        goto END;

    //operate and get result
    ret.errcode = sAPI_SimcardHotSwapMsg(opt, 0, g_simcard_demo_msgQ);
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if (ret.errcode)
    {
        //API get error
        snprintf(rsp_buff, BUFF_LEN, "process sAPI_SimcardHotSwapMsg Fail!");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }
    else
    {
        status = sAPI_MsgQRecv(g_simcard_demo_msgQ, &msg, SIMCARD_URC_RECIVE_TIME_OUT); //This API will return 0--RHINO_SUCCESS when process success
        LOG_INFO("%s Info: get status: %d", __func__, status);
        if (status)
        {
        //receive msg fail
            snprintf(rsp_buff, BUFF_LEN, "get sAPI_SimcardHotSwapMsg return msg Fail!");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            ret.msg = "sAPI_SimcardHotSwapMsg set plugout fail";
        }
        else
        {
            if (msg.arg2 == 1)
            {
                snprintf(rsp_buff, BUFF_LEN, "1:ENABLE");
                out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            }
            else if (msg.arg2 == 0)
            {
                snprintf(rsp_buff, BUFF_LEN, "0:DISABLE");
                out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            }
            else
            {
                snprintf(rsp_buff, BUFF_LEN, "ERROR");
                out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            }
        }
    }

END:
    if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
        LOG_ERROR("%s Error: POST data fail!", __func__);
    sAPI_Debug("[SIM_Demo] %s leave with retcode: %d", __func__, ret.errcode);
    sAPI_MsgQDelete(g_simcard_demo_msgQ);
    pl_demo_release_value(out.value);
    return ret;
}

/*------------------------------------*/
/*        set plugLevel state         */
/*------------------------------------*/
//param config
arg_t setPlugOut_args =
{
    .type = TYPE_STR, .arg_name = ARGSIM_SETPLUGOUT, .value_range = &SWITCHSTATUS
};

//func menu config
const menu_list_t set_plugOut_menu =
{
    .menu_name = "Set plugOut state",
    .menu_type = TYPE_FUNC,
    .parent = &Hotplug_menu,
    .function =
    {
        .argv = &setPlugOut_args,
        .argn = sizeof(setPlugOut_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_plugout_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t set_plugout_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t out = { 0 };
    SC_HotSwapCmdType_e opt = SC_HOTSWAP_SET_OUTSWITCH;
    const char* input;
    int state = 255;

    //get input
    pl_demo_get_data(&input, argv, argn, ARGSIM_SETPLUGOUT);
    LOG_INFO("%s Info: get state: %s", __func__, state);

    //process input
    if (strcmp(input, "0:DISABLE") == 0)
        state = 0;
    else if (strcmp(input, "1:ENABLE") == 0)
        state = 1;
    else
        state = -1;

    //run sAPI, get return
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if ((state == 0) || (state == 1))
    {
        ret.errcode = sAPI_SimcardHotSwapMsg(opt, state, g_simcard_demo_msgQ);
        if (ret.errcode)
        {
        //API get error
            snprintf(rsp_buff, BUFF_LEN, "Failure!");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            ret.msg = "sAPI_SimcardHotSwapMsg set plugout fail";
        }
        else
        {
            snprintf(rsp_buff, BUFF_LEN, "Success!");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            ret.msg = "sAPI_SimcardHotSwapMsg set plugout success";
        }
    }
    else
    {
    //Invalid Parameter
        snprintf(rsp_buff, BUFF_LEN, "Invalid Parameter");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }

    if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
        LOG_ERROR("%s Error: POST data fail!", __func__);
    sAPI_Debug("[SIM_Demo] %s leave with retcode :%d", __func__, ret.errcode);
    pl_demo_release_value(out.value);
    return ret;
}

/*------------------------------------*/
/*            plugEX Query            */
/*------------------------------------*/
//param config
const value_t QUERY_OPT[3] =
{
    {
        .bufp = "1:QUERY STATE",
        .size = 14,
    },
    {
        .bufp = "2:QUERY LEVEL",
        .size = 14,
    },
    {
        .bufp = "5:QUERY OUTSTATE",
        .size = 16,
    }
};

const value_list_t QuerySTATUS =
{
    .isrange = false, .list_head = QUERY_OPT, .count = 3,
};

arg_t setQuery_args[2] =
{
    {
        .type = TYPE_STR, .arg_name = ARGSIM_SWITCHSIM, .value_range = &SWITCHSIM
    },
    {
        .type = TYPE_STR, .arg_name = ARGSIM_SETQUERY, .value_range = &QuerySTATUS
    }
};

//func menu config
const menu_list_t Set_Query_menu =
{
    .menu_name = "HotPlug(EX) state Query",
    .menu_type = TYPE_FUNC,
    .parent = &HotplugEX_menu,
    .function =
    {
        .argv = setQuery_args,
        .argn = sizeof(setQuery_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_Query_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t set_Query_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t out = { 0 };
    const char *input;
    BOOL SIMID = 255;
    SC_HotSwapCmdType_e opt = SC_HOTSWAP_TYPE_MAX;
    UINT8 state = 255;

    //get input(ARGSIM_SWITCHSIM)
    pl_demo_get_data(&input, argv, argn, ARGSIM_SWITCHSIM);
    LOG_INFO("%s Info: get SIMID: %s", __func__, input);
    if (strcmp(input, "0:SIM1") == 0)
        SIMID = 0;
    else if (strcmp(input, "1:SIM2") == 0)
        SIMID = 1;
    else
        SIMID = -1;

    //get input(ARGSIM_SETQUERY)
    pl_demo_get_data(&input, argv, argn, ARGSIM_SETQUERY);
    LOG_INFO("%s Info: get Quety_TYPE: %s", __func__, input);
    if (strcmp(input, "1:QUERY STATE") == 0)
        opt = SC_HOTSWAP_QUERY_STATE;
    else if (strcmp(input, "2:QUERY LEVEL") == 0)
        opt = SC_HOTSWAP_QUERY_LEVEL;
    else if (strcmp(input, "5:QUERY OUTSTATE") == 0)
        opt = SC_HOTSWAP_QUERY_OUTSTATE;
    else
        opt = SC_HOTSWAP_INVALID;

    //run SAPI, get result
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if ((SIMID == 0) || (SIMID == 1))
    {
        if (opt != SC_HOTSWAP_INVALID)
            ret.errcode = sAPI_SimcardHotSwap_Ex(SIMID, opt, &state);
        else
            ret.errcode = -1;

        if (ret.errcode)
        {
        //API get error
            snprintf(rsp_buff, BUFF_LEN, "Failure!");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            ret.msg = "sAPI_SimcardHotSwap_Ex Query fail";
        }
        else
        {
            if (state == 1)
            {
                snprintf(rsp_buff, BUFF_LEN, "1:ENABLE/HIGH Level");
                out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
                ret.msg = "sAPI_SimcardHotSwap_Ex Query success";
            }
            else if (state == 0)
            {
                snprintf(rsp_buff, BUFF_LEN, "0:DISABLE/LOW Level");
                out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
                ret.msg = "sAPI_SimcardHotSwap_Ex Query success";
            }
            else
            {
                snprintf(rsp_buff, BUFF_LEN, "ERROR");
                out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
                ret.msg = "sAPI_SimcardHotSwap_Ex Query fail";
            }
        }
    }
    else
    {
    //Invalid Parameter
        snprintf(rsp_buff, BUFF_LEN, "Invalid Parameter");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }

    if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
        LOG_ERROR("%s Error: POST data fail!", __func__);
    sAPI_Debug("[SIM_Demo] %s leave with retcode :%d", __func__, ret.errcode);
    pl_demo_release_value(out.value);
    return ret;
}

/*------------------------------------*/
/*             plugEX SET             */
/*------------------------------------*/
//param config
const value_t PLUGOPT_TYPE[3] =
{
    {
        .bufp = "3:SET STATE",
        .size = 14,
    },
    {
        .bufp = "4:SET LEVEL",
        .size = 14,
    },
    {
        .bufp = "6:SET OUTSTATE",
        .size = 16,
    }
};

const value_list_t PLUGoptSTATUS =
{
    .isrange = false, .list_head = PLUGOPT_TYPE, .count = 3,
};

const value_t PLUGSET_TYPE[2] =
{
    {
        .bufp = "0:DISABLE/LOW LEVEL",
        .size = 20,
    },
    {
        .bufp = "1:ENABLE/HIGH LEVEL",
        .size = 20,
    }
};

const value_list_t PLUGsetSTATUS =
{
    .isrange = false, .list_head = PLUGSET_TYPE, .count = 2,
};


arg_t setPLUGset_args[3] =
{
    {
        .type = TYPE_STR, .arg_name = ARGSIM_SWITCHSIM, .value_range = &SWITCHSIM
    },
    {
        .type = TYPE_STR, .arg_name = ARGSIM_SETSTATEOPT, .value_range = &PLUGoptSTATUS
    },
    {
        .type = TYPE_STR, .arg_name = ARGSIM_SETSTATE, .value_range = &PLUGsetSTATUS
    }
};

//func menu config
const menu_list_t Set_State_menu =
{
    .menu_name = "HotPlug(EX) state Query",
    .menu_type = TYPE_FUNC,
    .parent = &HotplugEX_menu,
    .function =
    {
        .argv = setPLUGset_args,
        .argn = sizeof(setPLUGset_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_STATE_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t set_STATE_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t out = { 0 };
    const char *input;
    BOOL SIMID = 255;
    SC_HotSwapCmdType_e opt = SC_HOTSWAP_TYPE_MAX;
    UINT8 state = 255;

    //get input(ARGSIM_SWITCHSIM)
    pl_demo_get_data(&input, argv, argn, ARGSIM_SWITCHSIM);
    LOG_INFO("%s Info: get SIMID: %s", __func__, input);
    if (strcmp(input, "0:SIM1") == 0)
        SIMID = 0;
    else if (strcmp(input, "1:SIM2") == 0)
        SIMID = 1;
    else
        SIMID = -1;

    //get input(ARGSIM_SETSTATEOPT)
    pl_demo_get_data(&input, argv, argn, ARGSIM_SETSTATEOPT);
    LOG_INFO("%s Info: get Quety_TYPE: %s", __func__, input);
    if (strcmp(input, "3:SET STATE") == 0)
        opt = SC_HOTSWAP_SET_SWITCH;
    else if (strcmp(input, "4:SET LEVEL") == 0)
        opt = SC_HOTSWAP_SET_LEVEL;
    else if (strcmp(input, "6:SET OUTSTATE") == 0)
        opt = SC_HOTSWAP_SET_OUTSWITCH;
    else
        opt = SC_HOTSWAP_INVALID;

    //get input(ARGSIM_SETSTATE)
    pl_demo_get_data(&input, argv, argn, ARGSIM_SETSTATE);
    LOG_INFO("%s Info: get Quety_TYPE: %s", __func__, input);
    if (strcmp(input, "0:DISABLE/LOW LEVEL") == 0)
        state = 0;
    else if (strcmp(input, "1:ENABLE/HIGH LEVEL") == 0)
        state = 1;
    else
        state = -1;

    //run SAPI, get result
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if ((SIMID == 0) || (SIMID == 1))
    {
        if (opt != SC_HOTSWAP_INVALID)
            ret.errcode = sAPI_SimcardHotSwap_Ex(SIMID, opt, &state);
        else
            ret.errcode = -1;

        if (ret.errcode)
        {
        //API get error
            snprintf(rsp_buff, BUFF_LEN, "Failure!");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            ret.msg = "sAPI_SimcardHotSwap_Ex set fail";
        }
        else
        {
            snprintf(rsp_buff, BUFF_LEN, "Success!");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            ret.msg = "sAPI_SimcardHotSwap_Ex set success";
        }
    }
    else
    {
    //Invalid Parameter
        snprintf(rsp_buff, BUFF_LEN, "Invalid Parameter");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }

    if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
        LOG_ERROR("%s Error: POST data fail!", __func__);
    sAPI_Debug("[SIM_Demo] %s leave with retcode :%d", __func__, ret.errcode);
    pl_demo_release_value(out.value);
    return ret;
}

/*------------------------------------*/
/*         Hotplug menu items         */
/*------------------------------------*/
const menu_list_t *SIM_hotplug_sub[] =
{
    &Query_plugIn_menu,
    &Query_plugLevel_menu,
    &Set_plugIn_menu,
    &set_plugLevel_menu,
    &Query_plugOut_menu,
    &set_plugOut_menu,

    NULL  //must end bu NULL
};

const menu_list_t Hotplug_menu =
{
    .menu_name = "Hotplug OPTION",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = SIM_hotplug_sub,
    .parent = &Simcard_menu,
};

/*------------------------------------*/
/*         HotplugEX menu items         */
/*------------------------------------*/
const menu_list_t *SIM_hotplugEX_sub[] =
{
    &Set_Query_menu,
    &Set_State_menu,

    NULL  //must end bu NULL
};

const menu_list_t HotplugEX_menu =
{
    .menu_name = "Hotplug(EX) OPTION",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = SIM_hotplugEX_sub,
    .parent = &Simcard_menu,
};
#endif

/*------------------------------------*/
/*              Pin set               */
/*------------------------------------*/
//param config
const value_t PIN_TYPE[2] =
{
    {
        .bufp = "0:No Need New PIN",
        .size = 18,
    },
    {
        .bufp = "1:Need New PIN",
        .size = 15,
    },
};

const value_list_t SET_PINTYPE =
{
    .isrange = false, .list_head = PIN_TYPE, .count = 2,
};

arg_t setPin_args[3] =
{
    {
        .type = TYPE_STR, .arg_name = ARGSIM_PINOPT, .value_range = &SET_PINTYPE
    },
    {
        .type = TYPE_STR, .arg_name = ARGSIM_PINOLD, .msg = "Please enter the PIN:"
    },
    {
        .type = TYPE_STR, .arg_name = ARGSIM_PINNEW, .msg = "Please enter the PIN(new):"
    }
};

//func menu config
const menu_list_t Set_pin_menu =
{
    .menu_name = "set sim pin",
    .menu_type = TYPE_FUNC,
    .parent = &Simcard_menu,
    .function =
    {
        .argv = setPin_args,
        .argn = sizeof(setPin_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = setPin_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t setPin_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t out = { 0 };
    const char *input = NULL;
    const char *getpin = NULL;
    char oldpass[16] = { 0 }, newpass[16] = { 0 };
    int opt = 255;

    //get input, process input
    pl_demo_get_data(&input, argv, argn, ARGSIM_PINOPT);
    LOG_INFO("%s Info: get opt: %s", __func__, input);
    if (strcmp(input, "0:No Need New PIN"))
        opt = 0;
    else if (strcmp(input, "1:Need New PIN"))
        opt = 1;
    else
        opt = -1;
    //according to opt, get pass
    switch (opt)
    {
    case 0://NO need new pin
        pl_demo_get_data(&getpin, argv, argn, ARGSIM_PINOLD);
        LOG_INFO("%s Info: get PIN: %s", __func__, getpin);
        //delete CR LF
        if ((getpin[strlen(getpin) - 1] == '\n') && (getpin[strlen(getpin) - 2] == '\r'))
        {
            sAPI_Debug("[SIM_Demo] %d delet CR LF\r\n", __LINE__);
            memcpy(oldpass, getpin, strlen(getpin) - 2);
        }
        else
            memcpy(oldpass, getpin, strlen(getpin));
        break;
    case 1://Need new pin
        pl_demo_get_data(&getpin, argv, argn, ARGSIM_PINOLD);
        LOG_INFO("%s Info: get PIN: %s", __func__, getpin);
        //delete CR LF for oldpin
        if ((getpin[strlen(getpin) - 1] == '\n') && (getpin[strlen(getpin) - 2] == '\r'))
        {
            sAPI_Debug("[SIM_Demo] %d delet CR LF\r\n", __LINE__);
            memcpy(oldpass, getpin, strlen(getpin) - 2);
        }
        else
            memcpy(oldpass, getpin, strlen(getpin));
        pl_demo_get_data(&getpin, argv, argn, ARGSIM_PINNEW);
        LOG_INFO("%s Info: get PIN(new): %s", __func__, getpin);
        //delete CR LF for newpin
        if ((getpin[strlen(getpin) - 1] == '\n') && (getpin[strlen(getpin) - 2] == '\r'))
        {
            sAPI_Debug("[SIM_Demo] %d delet CR LF\r\n", __LINE__);
            memcpy(newpass, getpin, strlen(getpin) - 2);
        }
        else
            memcpy(newpass, getpin, strlen(getpin));
        break;
    default:
        out.arg_name = ARGSIM_OUT;
        out.type = TYPE_STR;
        snprintf(rsp_buff, BUFF_LEN, "Invalid Operate!");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        goto END;
    }

    //operate
    ret.errcode = sAPI_SimcardPinSet(oldpass, newpass, opt);
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if (ret.errcode)
    {
    //SAPI get error
        snprintf(rsp_buff, BUFF_LEN, "Failure!");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        ret.msg = "sAPI_SimcardPinSet PIN Set fail";
    }
    else
    {
        snprintf(rsp_buff, BUFF_LEN, "Success!");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        ret.msg = "sAPI_SimcardPinSet PIN Set Success";
    }

END:
    if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
        LOG_ERROR("%s Error: POST data fail!", __func__);
    sAPI_Debug("[SIM_Demo] %s leave with retcode :%d", __func__, ret.errcode);
    pl_demo_release_value(out.value);
    return ret;
}

/*------------------------------------*/
/*              Pin get               */
/*------------------------------------*/
//param config
    //NULL

//func menu config
const menu_list_t Get_pin_menu =
{
    .menu_name = "get sim pinstate",
    .menu_type = TYPE_FUNC,
    .parent = &Simcard_menu,
    .function =
    {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = getPin_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t getPin_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t out = { 0 };
    UINT8 cpin = 255;

    //operate
    ret.errcode = sAPI_SimcardPinGet(&cpin);
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if (ret.errcode)
    {
        snprintf(rsp_buff, BUFF_LEN, "Get Pin State Falied!");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }
    else
    {
        switch (cpin)
        {
        case 0:
            snprintf(rsp_buff, BUFF_LEN, "0:READY");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        case 1:
            snprintf(rsp_buff, BUFF_LEN, "1:PIN NEED");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        case 2:
            snprintf(rsp_buff, BUFF_LEN, "2:PUK NEED");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        case 3:
            snprintf(rsp_buff, BUFF_LEN, "3:BLK");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        case 4:
            snprintf(rsp_buff, BUFF_LEN, "4:REMOVE");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        case 5:
            snprintf(rsp_buff, BUFF_LEN, "5:CRASH");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        case 6:
            snprintf(rsp_buff, BUFF_LEN, "6:NOINSRT");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        case 7:
            snprintf(rsp_buff, BUFF_LEN, "7:UNKN");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        default:
            snprintf(rsp_buff, BUFF_LEN, "ERROR");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        }
    }

    if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
        LOG_ERROR("%s Error: POST data fail!", __func__);
    sAPI_Debug("[SIM_Demo] %s leave with retcode :%d", __func__, ret.errcode);
    pl_demo_release_value(out.value);
    return ret;
}

/*------------------------------------*/
/*             Pin_EX get             */
/*------------------------------------*/
//param config
arg_t Get_PINEX_args =
{
    .type = TYPE_STR, .arg_name = ARGSIM_SWITCHSIM, .value_range =&SWITCHSIM
};

//func menu config
const menu_list_t Get_PINEX_menu =
{
    .menu_name = "get sim pinstate(EX)",
    .menu_type = TYPE_FUNC,
    .parent = &Simcard_menu,
    .function =
    {
        .argv = &Get_PINEX_args,
        .argn = sizeof(Get_PINEX_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = getPinEX_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t getPinEX_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t out = { 0 };
    UINT8 cpin = 255;
    const char *input = NULL;
    BOOL SIMID = 255;

    //get input
    pl_demo_get_data(&input, argv, argn, ARGSIM_SWITCHSIM);
    if (strcmp(input, "0:SIM1") == 0)
        SIMID = 0;
    else if (strcmp(input, "1:SIM2") == 0)
        SIMID = 1;
    else
        SIMID = -1;

    //operate
    if ((SIMID == 0) || (SIMID == 1))
        ret.errcode = sAPI_SimcardPinGet_Ex(SIMID, &cpin);
    else
        ret.errcode = -1;

    //process result
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if (ret.errcode)
    {
        snprintf(rsp_buff, BUFF_LEN, "Get Pin State Falied!");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }
    else
    {
        switch (cpin)
        {
        case 0:
            snprintf(rsp_buff, BUFF_LEN, "0:READY");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        case 1:
            snprintf(rsp_buff, BUFF_LEN, "1:PIN NEED");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        case 2:
            snprintf(rsp_buff, BUFF_LEN, "2:PUK NEED");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        case 3:
            snprintf(rsp_buff, BUFF_LEN, "3:BLK");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        case 4:
            snprintf(rsp_buff, BUFF_LEN, "4:REMOVE");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        case 5:
            snprintf(rsp_buff, BUFF_LEN, "5:CRASH");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        case 6:
            snprintf(rsp_buff, BUFF_LEN, "6:NOINSRT");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        case 7:
            snprintf(rsp_buff, BUFF_LEN, "7:UNKN");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        default:
            snprintf(rsp_buff, BUFF_LEN, "ERROR");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        }
    }

    if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
        LOG_ERROR("%s Error: POST data fail!", __func__);
    sAPI_Debug("[SIM_Demo] %s leave with retcode :%d", __func__, ret.errcode);
    pl_demo_release_value(out.value);
    return ret;
}

/*------------------------------------*/
/*              ICCID get             */
/*------------------------------------*/
//param config
    //NULL

//func menu config
const menu_list_t Get_ICCID_menu =
{
    .menu_name = "get sim ICCID",
    .menu_type = TYPE_FUNC,
    .parent = &Simcard_menu,
    .function =
    {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = getICCID_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t getICCID_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t out = { 0 };
    char iccid[32] = { 0 };

    //operate
    ret.errcode = sAPI_SysGetIccid(iccid);
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if (ret.errcode)
    {
        snprintf(rsp_buff, BUFF_LEN, "Get ICCID Falied!");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }
    else
    {
        snprintf(rsp_buff, BUFF_LEN, "%s", iccid);
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }

    if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
        LOG_ERROR("%s Error: POST data fail!", __func__);
    sAPI_Debug("[SIM_Demo] %s leave with retcode :%d", __func__, ret.errcode);
    pl_demo_release_value(out.value);
    return ret;
}

/*------------------------------------*/
/*              IMSI get              */
/*------------------------------------*/
//param config
    //NULL

//func menu config
const menu_list_t Get_IMSI_menu =
{
    .menu_name = "get sim IMSI",
    .menu_type = TYPE_FUNC,
    .parent = &Simcard_menu,
    .function =
    {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = getIMSI_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t getIMSI_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t out = { 0 };
    char imsi[32] = { 0 };

    //operate
    ret.errcode = sAPI_SysGetImsi(imsi);
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if (ret.errcode)
    {
        snprintf(rsp_buff, BUFF_LEN, "Get IMSI Falied!");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }
    else
    {
        snprintf(rsp_buff, BUFF_LEN, "%s", imsi);
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }

    if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
        LOG_ERROR("%s Error: POST data fail!", __func__);
    sAPI_Debug("[SIM_Demo] %s leave with retcode :%d", __func__, ret.errcode);
    pl_demo_release_value(out.value);
    return ret;
}

/*------------------------------------*/
/*              HPLMN get             */
/*------------------------------------*/
//param config
    //NULL

//func menu config
const menu_list_t Get_HPLMN_menu =
{
    .menu_name = "get sim HPLMN",
    .menu_type = TYPE_FUNC,
    .parent = &Simcard_menu,
    .function =
    {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = getHPLMN_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t getHPLMN_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t spn = { 0 }, plmn = { 0 };
    Hplmn_st hplmn = { 0 };

    //operate
    ret.errcode = sAPI_SysGetHplmn(&hplmn);

    if (ret.errcode)
    {
        spn.arg_name = ARGSIM_OUT;
        spn.type = TYPE_STR;
        snprintf(rsp_buff, BUFF_LEN, "Get HPLMN Falied!");
        spn.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        if (pl_demo_post_data(OP_POST, &spn, 1, NULL, 0))
            LOG_ERROR("%s Error: POST spn fail!", __func__);
        pl_demo_release_value(spn.value);
    }
    else
    {
        spn.arg_name = "SPN";
        spn.type = TYPE_STR;
        plmn.arg_name = "HPLMN";
        plmn. type = TYPE_STR;
        snprintf(rsp_buff, BUFF_LEN, "%s", hplmn.spn);
        spn.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        snprintf(rsp_buff, BUFF_LEN, "%s-%s", hplmn.mcc, hplmn.mnc);
        plmn.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        if (pl_demo_post_data(OP_POST, &spn, 1, NULL, 0))
            LOG_ERROR("%s Error: POST spn fail!", __func__);
        if (pl_demo_post_data(OP_POST, &plmn, 1, NULL, 0))
            LOG_ERROR("%s Error: POST plmn fail!", __func__);
        pl_demo_release_value(spn.value);
        pl_demo_release_value(plmn.value);
    }

    sAPI_Debug("[SIM_Demo] %s leave with retcode :%d", __func__, ret.errcode);
    return ret;
}

/*------------------------------------*/
/*             SIMlock set            */
/*------------------------------------*/
//param config
const value_t SIMlock_Type[2] =
{
    {
        .bufp = "0:UNLOCK SIMCARD",
        .size = 17,
    },
    {
        .bufp = "1:LOCK SIMCARD",
        .size = 15,
    },
};

const value_list_t SETSIMLOCK =
{
    .isrange = false, .list_head = SIMlock_Type, .count = 2,
};

arg_t set_SIMlock_args[2] =
{
    {
        .type = TYPE_STR, .arg_name = ARGSIM_SIMLOCK, .value_range = &SETSIMLOCK
    },
    {
        .type = TYPE_STR, .arg_name = ARGSIM_LCKPIN, .msg = "Please enter the password(PIN):"
    }
};

//func menu config
const menu_list_t Set_SIMLock_menu =
{
    .menu_name = "set SIMlock",
    .menu_type = TYPE_FUNC,
    .parent = &Simcard_menu,
    .function =
    {
        .argv = set_SIMlock_args,
        .argn = sizeof(set_SIMlock_args)/sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_SIMlock_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t set_SIMlock_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t out = { 0 };
    const char *input = NULL;
    const char *getpin;
    int opt = 255;
    char pass[16] = { 0 };

    //getinput
    pl_demo_get_data(&input, argv, argn, ARGSIM_SIMLOCK);
    LOG_INFO("%s Info: get opt: %d", __func__, input);
    if (strcmp(input, "0:UNLOCK SIMCARD") == 0)
        opt = 0;
    else if (strcmp(input, "1:LOCK SIMCARD") == 0)
        opt = 1;
    else
        opt = -1;
    pl_demo_get_data(&getpin, argv, argn, ARGSIM_LCKPIN);
    LOG_INFO("%s Info: get PIN: %s", __func__, getpin);
    if ((getpin[strlen(getpin) - 1] == '\n') && (getpin[strlen(getpin) - 2] == '\r'))
    {
        sAPI_Debug("[SIM_Demo] %d delet CR LF\r\n", __LINE__);
        memcpy(pass, getpin, strlen(getpin) - 2);
    }
    else
        memcpy(pass, getpin, strlen(getpin));

    //operate
    ret.errcode = sAPI_SimLockSet(opt, pass);
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if(ret.errcode)
    {
    //SAPI get error
        switch (opt)
        {
        case 1:
            snprintf(rsp_buff, BUFF_LEN, "Simcard lock Fail!");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        case 2:
            snprintf(rsp_buff, BUFF_LEN, "Simcard unlock Fail!");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        }
    }
    else
    {
        switch (opt)
        {
        case 1:
            snprintf(rsp_buff, BUFF_LEN, "Simcard lock success!");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        case 2:
            snprintf(rsp_buff, BUFF_LEN, "Simcard unlock success!");
            out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
            break;
        }
    }

    if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
        LOG_ERROR("%s Error: POST data fail!", __func__);
    sAPI_Debug("[SIM_Demo] %s leave with retcode :%d", __func__, ret.errcode);
    pl_demo_release_value(out.value);
    return ret;
}

/*------------------------------------*/
/*           SIM Restricted           */
/*------------------------------------*/
//param config
const value_t CMD_Type[5] =
{
    {
        .bufp = "176:READ BINARY",
        .size = 16,
    },
    {
        .bufp = "178:READ RECORD",
        .size = 16,
    },
    {
        .bufp = "192:GET RESPONSE",
        .size = 17,
    },
    {
        .bufp = "214:UPDATE BINARY",
        .size = 18,
    },
    {
        .bufp = "220:UPDATE RECORD",
        .size = 18,
    },
};

const value_list_t SET_Restricted_Name =
{
    .isrange = false, .list_head = CMD_Type, .count = 5,
};

arg_t SIM_Restricted_args[7] =
{
    {
        .type = TYPE_STR, .arg_name = ARGSIM_Restric_cmd, .value_range = &SET_Restricted_Name
    },
    {
        .type = TYPE_INT, .arg_name = ARGSIM_Restric_fID, .msg = "Please enter the file ID(dec):"
    },
    {
        .type = TYPE_INT, .arg_name = ARGSIM_Restric_p1, .msg = "Please enter the p1(0-255):"
    },
    {
        .type = TYPE_INT, .arg_name = ARGSIM_Restric_p2, .msg = "Please enter the p2(0-255):"
    },
    {
        .type = TYPE_INT, .arg_name = ARGSIM_Restric_p3, .msg = "Please enter the p3(Length 0-255):"
    },
    {
        .type = TYPE_STR, .arg_name = ARGSIM_Restric_data, .msg = "Please enter the data(hex):"
    },
    {
        .type = TYPE_STR, .arg_name = ARGSIM_Restric_pID, .msg = "Please enter the pathid(hex):"
    }
};

//func menu config
const menu_list_t SIM_Restricted_menu =
{
    .menu_name = "set SIM Restricted",
    .menu_type = TYPE_FUNC,
    .parent = &Simcard_menu,
    .function =
    {
        .argv = SIM_Restricted_args,
        .argn = sizeof(SIM_Restricted_args)/sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = SIM_Restricted_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t SIM_Restricted_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LEN] = { 0 };
    arg_t out = { 0 };
    const char *input = NULL;
    int cmd=0, fileId=0, p1=0, p2=0, p3=0;
    const char *data = NULL, *pathid = NULL;
    CrsmResponse_st response = { 0 };
    arg_t sw1 = { 0 }, sw2 = { 0 };

    //get input
    pl_demo_get_data(&input, argv, argn, ARGSIM_Restric_cmd);
    LOG_INFO("%s Info: get cmd: %s", input);
    if (strcmp(input, "176:READ BINARY") == 0)
        cmd = 176;
    else if (strcmp(input, "178:READ RECORD") == 0)
        cmd = 178;
    else if (strcmp(input, "192:GET RESPONSE") == 0)
        cmd = 192;
    else if (strcmp(input, "214:UPDATE BINARY") == 0)
        cmd = 214;
    else if (strcmp(input, "220:UPDATE RECORD") == 0)
        cmd = 220;
    else
        cmd = 0;

    switch (cmd)
    {
    case 176:
    case 178:
    case 192:
        pl_demo_get_value(&fileId, argv, argn, ARGSIM_Restric_fID);
        pl_demo_get_value(&p1, argv, argn, ARGSIM_Restric_p1);
        pl_demo_get_value(&p2, argv, argn, ARGSIM_Restric_p2);
        pl_demo_get_value(&p3, argv, argn, ARGSIM_Restric_p3);
        LOG_INFO("%s Info: get fileID: %d, p1: %d, p2: %d, p3: %d", __func__, fileId, p1, p2, p3);
        break;
    case 214:
    case 220:
        pl_demo_get_value(&fileId, argv, argn, ARGSIM_Restric_fID);
        pl_demo_get_value(&p1, argv, argn, ARGSIM_Restric_p1);
        pl_demo_get_value(&p2, argv, argn, ARGSIM_Restric_p2);
        pl_demo_get_value(&p3, argv, argn, ARGSIM_Restric_p3);
        LOG_INFO("%s Info: get fileID: %d, p1: %d, p2: %d, p3: %d", __func__, fileId, p1, p2, p3);
        pl_demo_get_data(&data, argv, argn, ARGSIM_Restric_data);
        LOG_INFO("%s Info: get data: %s", data);
        pl_demo_get_data(&pathid, argv , argn, ARGSIM_Restric_pID);
        LOG_INFO("%s Info: get pathid: %s", pathid);
        break;
    default:
        break;
    }

    //operate
    if (cmd)
        ret.errcode = sAPI_SimRestrictedAccess(cmd,fileId,p1,p2,p3,(char*)data,(char*)pathid,&response);
    else
        ret.errcode = -255;

    if (ret.errcode == -255)
    {
    //invalid param
        out.arg_name = ARGSIM_OUT;
        out.type = TYPE_STR;
        sAPI_Debug("[SIM_Demo] %s sAPI_SimRestrictedAccess get invalid param!", __func__);
        snprintf(rsp_buff,BUFF_LEN,"Invalid param!");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
            LOG_ERROR("%s Error: POST data fail!", __func__);
        pl_demo_release_value(out.value);
    }
    else if (ret.errcode)
    {
    //SAPI get error
        out.arg_name = ARGSIM_OUT;
        out.type = TYPE_STR;
        sAPI_Debug("[SIM_Demo] %s sAPI_SimRestrictedAccess is error!", __func__);
        snprintf(rsp_buff,BUFF_LEN,"Failure!");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
            LOG_ERROR("%s Error: POST data fail!", __func__);
        pl_demo_release_value(out.value);
    }
    else
    {
        sw1.arg_name = "sw1";
        sw1.type = TYPE_STR;
        sw2.arg_name = "sw2";
        sw2.type = TYPE_STR;
        out.arg_name = "data";
        out.type = TYPE_STR;
        snprintf(rsp_buff,BUFF_LEN,"%d", response.sw1);
        sw1.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        snprintf(rsp_buff,BUFF_LEN,"%d", response.sw2);
        sw2.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
        if (strlen(response.data) == 0)
            out.value = pl_demo_make_value(-1, "NULL", 5);
        else
            out.value = pl_demo_make_value(-1, response.data, strlen(response.data));

        if (pl_demo_post_data(OP_POST, &sw1, 1, NULL, 0))
            LOG_ERROR("%s Error: %d POST data fail!", __func__, __LINE__);
        if (pl_demo_post_data(OP_POST, &sw2, 1, NULL, 0))
            LOG_ERROR("%s Error: %d POST data fail!", __func__, __LINE__);
        if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
            LOG_ERROR("%s Error: %d POST data fail!", __func__, __LINE__);

        pl_demo_release_value(sw1.value);
        pl_demo_release_value(sw2.value);
        pl_demo_release_value(out.value);
    }

    sAPI_Debug("[SIM_Demo] %s leave with retcode :%d", __func__, ret.errcode);
    return ret;
}

/*------------------------------------*/
/*             SIM Generic            */
/*------------------------------------*/
//param config
arg_t SIM_Generic_args[2] =
{
    {
        .type = TYPE_INT, .arg_name = ARGSIM_Generic_cmdL, .msg = "Please enter the cmdLen(0-255):"
    },
    {
        .type = TYPE_STR, .arg_name = ARGSIM_Generic_cmd, .msg = "Please enter the cmd:"
    }
};

//func menu config
const menu_list_t SIM_Generic_menu =
{
    .menu_name = "set SIM Generic",
    .menu_type = TYPE_FUNC,
    .parent = &Simcard_menu,
    .function =
    {
        .argv = SIM_Generic_args,
        .argn = sizeof(SIM_Generic_args)/sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = SIM_Generic_proc,
        .methods.needRAWData = NULL,
    },
};

//func
static ret_msg_t SIM_Generic_proc(op_t op, arg_t *argv, int argn)
{
    sAPI_Debug("[SIM_Demo] %s enter", __func__);
    ret_msg_t ret = { 0 };
    char rsp_buff[BUFF_LARGE] = { 0 };
    arg_t out = { 0 };
    int cmdLen = 0;
    const char *cmd = NULL;
    CsimResponse_st response = { 0 };

    //getinput
    pl_demo_get_value(&cmdLen, argv, argn, ARGSIM_Generic_cmdL);
    pl_demo_get_data(&cmd, argv, argn, ARGSIM_Generic_cmd);
    LOG_INFO("%s Info: get cmdLen: %d, cmd: %s]", __func__, cmdLen, cmd);

    //operate
    ret.errcode = sAPI_SimGenericAccess(cmdLen, (char*)cmd, &response);
    out.arg_name = ARGSIM_OUT;
    out.type = TYPE_STR;
    if(ret.errcode)
    {
    //SAPI get error
        snprintf(rsp_buff,BUFF_LARGE,"sAPI_SimGenericAccess error!");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }
    else
    {
        if (response.length == 0)
            snprintf(rsp_buff,5,"NULL");
        else if (response.length < BUFF_LARGE)
            snprintf(rsp_buff,response.length," \"%s\" ", response.data);
        else
            snprintf(rsp_buff,7,"ERROR!");
        out.value = pl_demo_make_value(-1, rsp_buff, strlen(rsp_buff));
    }

    if (pl_demo_post_data(OP_POST, &out, 1, NULL, 0))
        LOG_ERROR("%s Error: POST data fail!", __func__);
    sAPI_Debug("[SIM_Demo] %s leave with retcode :%d", __func__, ret.errcode);
    pl_demo_release_value(out.value);
    return ret;
}

/*------------------------------------*/
/*             menu items             */
/*------------------------------------*/
const menu_list_t *SIM_menu_sub[] =
{
#if defined (LWIP_IPNETBUF_SUPPORT) && defined (FEATURE_SIMCOM_DUALSIM)
    &SwitchSim_menu,
#if !(defined SINGLE_SIM) && !(defined FEATURE_SIMCOM_DSSS)
    &Get_Bindsim_menu,
    &Set_Bindsim_menu,
    &Get_DualsimType_menu,
    &Set_DualsimType_menu,
#endif
#endif

#ifdef FEATURE_SIMCOM_HOTPLUG
    &Hotplug_menu,
    &HotplugEX_menu,
#endif
    &Set_pin_menu,
    &Get_pin_menu,
    &Get_PINEX_menu,
    &Get_ICCID_menu,
    &Get_IMSI_menu,
    &Get_HPLMN_menu,
    &Set_SIMLock_menu,
    &SIM_Restricted_menu,
    &SIM_Generic_menu,

    NULL  // MUST end by NULL
};

const menu_list_t Simcard_menu =
{
    .menu_name = "SIMCARD",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = SIM_menu_sub,
    .parent = &root_menu,
};
