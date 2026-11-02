/*
*@Author: Xiaokai Yang
*@Date: 2023-12-15
*@Last Modified by: Xiaokai Yang
*@Last Modified Date: 2023-12-20
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "simcom_common.h"
#include "simcom_debug.h"
#include "simcom_sms.h"
#include "simcom_uart.h"
#include "uart_api.h"
#include "demo_sms.h"


#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"


#define LOG_ERROR(...) sal_log_error("[DEMO_SMS]"__VA_ARGS__)
#define LOG_INFO(...) sal_log_info("[DEMO_SMS]"__VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO_SMS]"__VA_ARGS__)
#define FALSE 0
#define TRUE 1
#define SMS_URC_RECIVE_TIME_OUT 1000
#define MAX_SMS_INPUT_DATA_LEN 400

sMsgQRef g_sms_demo_msgQ;

static ret_msg_t write_msg_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t read_msg_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t send_msg_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t send_storemsg_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t send_longmsg_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t delete_allmsg_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t delete_onemsg_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_newmsgind_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_newmsgind_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_format_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_format_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t decode_pdu_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t encode_pdu_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_chest_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_chest_cmd_proc(op_t op, arg_t *argv, int argn);
static char smsFormatjudge(const char *formatmodestr);
static char statejudge(const char *statstr);


const value_t smsFormat[] =
{
    {
        .bufp = "0:PDU",
        .size = 5,
    },
    {
        .bufp = "1:TEXT",
        .size = 6,
    },
};

const value_list_t smsFormat_range =
{
    .isrange = false,
    .list_head = smsFormat,
    .count = 2,
};

const value_t statstrs[] =
{
    {
        .bufp = "2:sto unsent",
        .size = 12,
    },
    {
        .bufp = "3:sto sent",
        .size = 10,
    },
};

const value_list_t stat_range =
{
    .isrange = false,
    .list_head = statstrs,
    .count = 2,
};


arg_t writemsg_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_FORMATMODE,
        .value_range = &smsFormat_range,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGSMS_SMSLEN,
        .msg = "Please input WRITE sms para SmsLen: "
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_SMSDATA,
        .msg = "Please input WRITE sms para SmsData: "
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_SRCADDR,
        .msg = "Please input WRITE sms para SrcAddr: "
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_SMSSTAT,
        .value_range = &stat_range,
    },
};

const menu_list_t func_writemsg =
{
    .menu_name = "write a msg to simcard",
    .menu_type = TYPE_FUNC,
    .parent = &sms_menu,
    .function = {
        .argv = writemsg_args,
        .argn = sizeof(writemsg_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = write_msg_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

arg_t readmsg_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_FORMATMODE,
        .value_range = &smsFormat_range,
        
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGSMS_SMSINDEX,
    },
};


const menu_list_t func_readmsg =
{
    .menu_name = "read a msg",
    .menu_type = TYPE_FUNC,
    .parent = &sms_menu,
    .function = {
        .argv = readmsg_args,
        .argn = sizeof(readmsg_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = read_msg_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

arg_t sendmsg_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_FORMATMODE,
        .value_range = &smsFormat_range,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGSMS_SMSLEN,
        .msg = "Please input send sms para SmsLen: "
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_SMSDATA,
        .msg = "Please input send sms para SmsData: "
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_SRCADDR,
        .msg = "Please input send sms para SrcAddr: "
    },
};

const menu_list_t func_sendmsg =
{
    .menu_name = "send a msg",
    .menu_type = TYPE_FUNC,
    .parent = &sms_menu,
    .function = {
        .argv = sendmsg_args,
        .argn = sizeof(sendmsg_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = send_msg_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

arg_t sendstoremsg_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGSMS_SMSINDEX,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_SRCADDR,
        .msg = "Please input send store sms  para addr: "
    },
};


const menu_list_t func_sendstoremsg =
{
    .menu_name = "send store msg",
    .menu_type = TYPE_FUNC,
    .parent = &sms_menu,
    .function = {
        .argv = sendstoremsg_args,
        .argn = sizeof(sendstoremsg_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = send_storemsg_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

arg_t sendlongmsg_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_SMSDATA,
        .msg = "Please input send long sms para smsdata:"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGSMS_SMSLEN,
        .msg = "Please input send long sms para smslen: "
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_SRCADDR,
        .msg = "Please input send long sms para addr: "
    },
};

const menu_list_t func_sendlongmsg =
{
    .menu_name = "send long msg",
    .menu_type = TYPE_FUNC,
    .parent = &sms_menu,
    .function = {
        .argv = sendlongmsg_args,
        .argn = sizeof(sendlongmsg_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = send_longmsg_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_delallmsg =
{
    .menu_name = "delete all msg",
    .menu_type = TYPE_FUNC,
    .parent = &sms_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = delete_allmsg_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

arg_t delonemsg_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGSMS_SMSINDEX,
        .msg = "Please input delete one sms para index:"
    },
};


const menu_list_t func_delonemsg =
{
    .menu_name = "delete one msg",
    .menu_type = TYPE_FUNC,
    .parent = &sms_menu,
    .function = {
        .argv = delonemsg_args,
        .argn = sizeof(delonemsg_args)  / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = delete_onemsg_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const value_t modestr[] =
{
    {
        .bufp = "mode:2",
        .size = 6,
    },
    {
        .bufp = "mode:1",
        .size = 6,
    },
    {
        .bufp = "mode:0",
        .size = 6,
    },
};
const value_list_t modede_range =
{
	.isrange = false,
	.list_head = modestr,
	.count = 3,
};

const value_t mtstr[] =
{
    {
        .bufp = "mt:1",
        .size = 4,
    },
    {
        .bufp = "mt:0",
        .size = 4,
    },
    {
        .bufp = "mt:2",
        .size = 4,
    },
    {
        .bufp = "mt:3",
        .size = 4,
    },
};
const value_list_t mt_range =
{
	.isrange = false,
	.list_head = mtstr,
	.count = 4,
};

const value_t bmstr[] =
{
    {
        .bufp = "bm:0",
        .size = 4,
    },
    {
        .bufp = "bm:2",
        .size = 4,
    },
};
const value_list_t bm_range =
{
    .isrange = false,
    .list_head = bmstr,
    .count = 2,
};
const value_t dsdestr[] =
{
    {
        .bufp = "ds:0",
        .size = 4,
    },
    {
        .bufp = "ds;1",
        .size = 4,
    },
    {
        .bufp = "ds:2",
        .size = 4,
    },
};
const value_list_t ds_range =
{
    .isrange = false,
    .list_head = dsdestr,
    .count = 3,
};
const value_t bfrstr[] =
{
    {
        .bufp = "bfr:0",
        .size = 5,
    },
    {
        .bufp = "bfr:1",
        .size = 5,
    },
};
const value_list_t bfr_range =
{
	.isrange = false,
	.list_head = bfrstr,
	.count = 2,
};

arg_t setnewmsgind_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_FORMATMODE,
        .value_range = &smsFormat_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_SMSMODE,
        .value_range = &modede_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_SMSMT,
        .value_range = &mt_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_SMSBM,
        .value_range = &bm_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_SMSDS,
        .value_range = &ds_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_SMSBFR,
        .value_range = &bfr_range,
    },
};


const menu_list_t func_setnewmsgind =
{
    .menu_name = "set new msg ind",
    .menu_type = TYPE_FUNC,
    .parent = &sms_menu,
    .function = {
        .argv = setnewmsgind_args,
        .argn = sizeof(setnewmsgind_args)  / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_newmsgind_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_getnewmsgind =
{
    .menu_name = "get new msg ind",
    .menu_type = TYPE_FUNC,
    .parent = &sms_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = get_newmsgind_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

arg_t setformat_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_FORMATMODE,
        .value_range = &smsFormat_range,
    },
};


const menu_list_t func_setformat =
{
    .menu_name = "set sms format",
    .menu_type = TYPE_FUNC,
    .parent = &sms_menu,
    .function = {
        .argv = setformat_args,
        .argn = sizeof(setformat_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_format_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_getformat =
{
    .menu_name = "get sms format",
    .menu_type = TYPE_FUNC,
    .parent = &sms_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = get_format_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

arg_t decodepdu_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_SMSPDU,
        .msg = "Please input pdu sms data:"
    },
};


const menu_list_t func_DecodePdu =
{
    .menu_name = "decode pdu sms",
    .menu_type = TYPE_FUNC,
    .parent = &sms_menu,
    .function = {
        .argv = decodepdu_args,
        .argn = sizeof(decodepdu_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = decode_pdu_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

arg_t encodepdu_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_SRCADDR,
        .msg = "Please input text sms addr:"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_SMSDATA,
        .msg = "Please input text sms data:"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGSMS_SMSLEN,
        .msg = "Please input text sms len:"
    },
};


const menu_list_t func_EncodePdu =
{
    .menu_name = "encode pdu sms",
    .menu_type = TYPE_FUNC,
    .parent = &sms_menu,
    .function = {
        .argv = encodepdu_args,
        .argn = sizeof(encodepdu_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = encode_pdu_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const value_t cheststr[] =
{
    {
        .bufp = "GSM:0",
        .size = 5,
    },
    {
        .bufp = "HEX:1",
        .size = 5,
    },
    {
        .bufp = "IRA:2",
        .size = 5,
    },
    {
        .bufp = "UCS2:4",
        .size = 6,
    },
};
const value_list_t chest_range =
{
	.isrange = false,
	.list_head = cheststr,
	.count = 4,
};

arg_t setchest_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGSMS_SMSCHEST,
        .value_range = &chest_range,
    },
};


const menu_list_t func_Setchset =
{
    .menu_name = "set sms chest",
    .menu_type = TYPE_FUNC,
    .parent = &sms_menu,
    .function = {
        .argv = setchest_args,
        .argn = sizeof(setchest_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_chest_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_Getchset =
{
    .menu_name = "get sms chest",
    .menu_type = TYPE_FUNC,
    .parent = &sms_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = get_chest_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t *sms_menu_sub[] =
{
    &func_writemsg,
    &func_readmsg,
    &func_sendmsg,
    &func_sendstoremsg,
    &func_sendlongmsg,
    &func_delallmsg,
    &func_delonemsg,
    &func_setnewmsgind,
    &func_getnewmsgind,
    &func_setformat,
    &func_getformat,
    &func_DecodePdu,
    &func_EncodePdu,
    &func_Setchset,
    &func_Getchset,

    NULL  // MUST end by NULL
};

const menu_list_t sms_menu =
{
    .menu_name = "SMS",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = sms_menu_sub,
    .parent = &root_menu,
};

static char smsFormatjudge(const char *formatmodestr)
{
    char formatmod = 0;
    if (strncmp(formatmodestr,"0:PDU",5) == 0)
        formatmod =0;
    else if(strncmp(formatmodestr,"1:TEXT",6) == 0)
        formatmod =1;

    return formatmod;
}

static char statejudge(const char *statstr)
{
	char stat = 2;
    if(strncmp(statstr,"2:sto unsent",12) == 0)
        stat = 2;
    else if(strncmp(statstr,"3:sto sent",10) == 0)
        stat = 3;

    return stat;
}

static ret_msg_t write_msg_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    arg_t out_arg[1] = {0};
    SC_SMSReturnCode retval = SC_SMS_FAIL;
    const char *formatmodestr = NULL;
	const char *statstr = NULL;
	char stat = 2;
	char formatmod = 0;
    int smslen = 0;
    const char *sms_data = NULL;
    const char *srcAddr = NULL;
    SIM_MSG_T msg;
    int resp_buff_len = 100;
    char rsp_buff[resp_buff_len];

    SC_STATUS status;
    status = sAPI_MsgQCreate(&g_sms_demo_msgQ, "g_sms_demo_msgQ", sizeof(SIM_MSG_T), 4, SC_FIFO);
    if (status != SC_SUCCESS)
    {
        LOG_ERROR("ERROR: message queue creat err!\n");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "ERROR: message queue creat err!";
        goto EXIT;
    }

	pl_demo_get_data(&formatmodestr, argv, argn, ARGSMS_FORMATMODE);
    pl_demo_get_value(&smslen, argv, argn, ARGSMS_SMSLEN);
    pl_demo_get_data(&sms_data,argv,argn,ARGSMS_SMSDATA);
    pl_demo_get_data(&srcAddr,argv,argn,ARGSMS_SRCADDR);
    pl_demo_get_data(&statstr,argv,argn,ARGSMS_SMSSTAT);

	formatmod = smsFormatjudge(formatmodestr);
	stat = statejudge(statstr);

    if(formatmod == 1)
        retval = sAPI_SmsWriteMsg(formatmod, (UINT8 *)sms_data, smslen, (UINT8 *)srcAddr, stat, g_sms_demo_msgQ);
    else
        retval = sAPI_SmsWriteMsg(formatmod, (UINT8 *)sms_data, smslen, NULL, stat, g_sms_demo_msgQ);

    LOG_INFO("%s,fmatmode=%d,smslen=%d,smsdata=%s,addr=%s,stat=%d",__func__,formatmod,smslen,sms_data,srcAddr,stat);

    LOG_INFO("%s, %d, msg write, ret=%d!!", __func__, __LINE__, ret);
    if (retval == SC_SMS_SUCESS)
    {
        memset(&msg, 0, sizeof(msg));
        sAPI_MsgQRecv(g_sms_demo_msgQ, &msg, SMS_URC_RECIVE_TIME_OUT);
        sAPI_Debug("[sms], sAPI_SmsWriteMsg, primID[%d] resultCode[%d],rspStr[%s]", msg.arg1, msg.arg2, msg.arg3);
        sprintf(rsp_buff, "rspStr[%s]",(char *)msg.arg3);
        sAPI_Debug("[sms]rsp_buff=%s",rsp_buff);
        sAPI_Free(msg.arg3);

        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = ARGSMS_WRITESMSINFO;
        out_arg[0].value = pl_demo_make_value(0, rsp_buff, strlen(rsp_buff));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg,1,NULL,0);
        if (0 != ret.errcode)
        {
            ret.msg = "post data fail";
            goto EXIT;
        }
    }
    else
    {
        LOG_ERROR("sAPI_SmsWriteMsg write sms fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_SmsWriteMsg write sms fail!!!";
        goto EXIT;
    }

    if(g_sms_demo_msgQ != NULL)
        sAPI_MsgQDelete(g_sms_demo_msgQ);

EXIT:
    pl_demo_release_value(out_arg[0].value);
    return ret;
}

static ret_msg_t read_msg_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    arg_t out_arg[1] = {0};
    SC_SMSReturnCode retval = SC_SMS_FAIL;
    const char *formatmodestr = NULL;
    char fmatmode = 0;
    int index = 0;
    SIM_MSG_T msg;
    int resp_buff_len = 500;
    char rsp_buff[resp_buff_len];
    SC_STATUS status;
    status = sAPI_MsgQCreate(&g_sms_demo_msgQ, "g_sms_demo_msgQ", sizeof(SIM_MSG_T), 4, SC_FIFO);
    if (status != SC_SUCCESS)
    {
        LOG_ERROR("ERROR: message queue creat err!\n");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "ERROR: message queue creat err!";
        goto EXIT;
    }

    pl_demo_get_data(&formatmodestr, argv, argn, ARGSMS_FORMATMODE);
    pl_demo_get_value(&index, argv, argn, ARGSMS_SMSINDEX);
    LOG_INFO("%s,fmatmode=%d,index=%d",__func__,fmatmode,index);
    fmatmode = smsFormatjudge(formatmodestr);

    retval = sAPI_SmsReadMsg(fmatmode, index, g_sms_demo_msgQ);
    if (retval == SC_SMS_SUCESS)
    {
        memset(&msg, 0, sizeof(msg));
        sAPI_MsgQRecv(g_sms_demo_msgQ, &msg, SMS_URC_RECIVE_TIME_OUT);
        sAPI_Debug("[sms], sAPI_SmsReadMsg, primID[%d] resultCode[%d],rspStr[%s]", msg.arg1, msg.arg2, msg.arg3);
        sprintf(rsp_buff, "rspStr[%s]",(char *)msg.arg3);
        sAPI_Free(msg.arg3);

        out_arg[0].type = TYPE_RAW;
        out_arg[0].arg_name = ARGSMS_READSMSINFO;
        out_arg[0].value = pl_demo_make_value(0, rsp_buff, strlen(rsp_buff));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
        pl_demo_post_raw(rsp_buff,strlen(rsp_buff));
        if (0 != ret.errcode)
        {
            ret.msg = "post data fail";
            goto EXIT;
        }
    }
    else
    {
        LOG_ERROR("sAPI_SmsReadMsg read sms fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_SmsReadMsg read sms fail!!!";
        goto EXIT;
    }
    if(g_sms_demo_msgQ != NULL)
        sAPI_MsgQDelete(g_sms_demo_msgQ);
EXIT:
    pl_demo_release_value(out_arg[0].value);
    return ret;
}

static ret_msg_t send_msg_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    arg_t out_arg[1] = {0};
    SC_SMSReturnCode retval = SC_SMS_FAIL;
    const char *formatmodestr = NULL;
    char fmatmode = 0;
    const char *sms_data = NULL;
    const char *srcAddr = NULL;
    int smslen = 0;
    SIM_MSG_T msg;
    int resp_buff_len = 100;
    char rsp_buff[resp_buff_len];
    SC_STATUS status;
    status = sAPI_MsgQCreate(&g_sms_demo_msgQ, "g_sms_demo_msgQ", sizeof(SIM_MSG_T), 4, SC_FIFO);
    if (status != SC_SUCCESS)
    {
        LOG_ERROR("ERROR: message queue creat err!\n");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "ERROR: message queue creat err!";
        goto EXIT;
    }

    pl_demo_get_data(&formatmodestr, argv, argn, ARGSMS_FORMATMODE);
    pl_demo_get_value(&smslen, argv, argn, ARGSMS_SMSLEN);
    pl_demo_get_data(&sms_data,argv,argn,ARGSMS_SMSDATA);
    pl_demo_get_data(&srcAddr,argv,argn,ARGSMS_SRCADDR);
    fmatmode = smsFormatjudge(formatmodestr);
    LOG_INFO("%s,fmatmode=%d,smslen=%d,smsdata=%s,addr=%s",__func__,fmatmode,smslen,sms_data,srcAddr);

    if (1 == fmatmode)
        retval = sAPI_SmsSendMsg(fmatmode, (UINT8 *)sms_data, smslen, (UINT8 *)srcAddr, g_sms_demo_msgQ);
    else
        retval = sAPI_SmsSendMsg(fmatmode, (UINT8 *)sms_data, smslen, NULL, g_sms_demo_msgQ);

    LOG_INFO("%s, %d, msg write, ret=%d!!", __func__, __LINE__, ret);
    if (retval == SC_SMS_SUCESS)
    {
        memset(&msg, 0, sizeof(msg));
        sAPI_MsgQRecv(g_sms_demo_msgQ, &msg, SMS_URC_RECIVE_TIME_OUT);
        sAPI_Debug("[sms], sAPI_SmsSendMsg, primID[%d] resultCode[%d],rspStr[%s]", msg.arg1, msg.arg2, msg.arg3);
        sprintf(rsp_buff, "rspStr[%s]",(char *)msg.arg3);
        sAPI_Debug("[sms]rsp_buff=%s",rsp_buff);
        sAPI_Free(msg.arg3);

        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = ARGSMS_SENDSMSINFO;
        out_arg[0].value = pl_demo_make_value(0, rsp_buff, strlen(rsp_buff));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg,1,NULL,0);
        if (0 != ret.errcode)
        {
            ret.msg = "post data fail";
            goto EXIT;
        }
    }
    else
    {
        LOG_ERROR("sAPI_SmsSendMsg send sms fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_SmsSendMsg send sms fail!!!";
        goto EXIT;
    }

    if(g_sms_demo_msgQ != NULL)
        sAPI_MsgQDelete(g_sms_demo_msgQ);
EXIT:
    pl_demo_release_value(out_arg[0].value);
    return ret;
}

static ret_msg_t send_storemsg_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    arg_t out_arg[1] = {0};
    SC_SMSReturnCode retval = SC_SMS_FAIL;
    SIM_MSG_T msg;
    int index = 0;
    const char *srcAddr = NULL;
    int resp_buff_len = 100;
    char rsp_buff[resp_buff_len];
    SC_STATUS status;
    status = sAPI_MsgQCreate(&g_sms_demo_msgQ, "g_sms_demo_msgQ", sizeof(SIM_MSG_T), 4, SC_FIFO);
    if (status != SC_SUCCESS)
    {
        LOG_ERROR("ERROR: message queue creat err!\n");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "ERROR: message queue creat err!";
        goto EXIT;
    }

    pl_demo_get_value(&index, argv, argn, ARGSMS_SMSINDEX);
    pl_demo_get_data(&srcAddr, argv, argn, ARGSMS_SRCADDR);
    LOG_INFO("%s,index=%d,srcAddr=%s",__func__,index,srcAddr);

    retval = sAPI_SmsSendStorageMsg(index, (UINT8 *)srcAddr, g_sms_demo_msgQ);
    if (retval == SC_SMS_SUCESS)
    {
        memset(&msg, 0, sizeof(msg));
        sAPI_MsgQRecv(g_sms_demo_msgQ, &msg, SMS_URC_RECIVE_TIME_OUT);
        sAPI_Debug("[sms], sAPI_SmsSendStorageMsg, primID[%d] resultCode[%d],rspStr[%s]", msg.arg1, msg.arg2, msg.arg3);
        sprintf(rsp_buff, "rspStr[%s]",(char *)msg.arg3);
        sAPI_Debug("[sms]rsp_buff=%s",rsp_buff);
        sAPI_Free(msg.arg3);

        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = ARGSMS_SENDSTORESMSINFO;
        out_arg[0].value = pl_demo_make_value(0, rsp_buff, strlen(rsp_buff));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg,1,NULL,0);
        if (0 != ret.errcode)
        {
            ret.msg = "post data fail";
            goto EXIT;
        }
    }
    else
    {
        LOG_ERROR("sAPI_SmsSendStorageMsg send sms fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_SmsSendStorageMsg send sms fail!!!";
        goto EXIT;
    }
    if(g_sms_demo_msgQ != NULL)
        sAPI_MsgQDelete(g_sms_demo_msgQ);
EXIT:
    pl_demo_release_value(out_arg[0].value);
    return ret;
}

static ret_msg_t send_longmsg_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    arg_t out_arg[1] = {0};
    SC_SMSReturnCode retval = SC_SMS_FAIL;
    SIM_MSG_T msg;
    int smslen = 0;
    const char *srcAddr = NULL;
    const char *sms_data = NULL;
    int resp_buff_len = 100;
    char rsp_buff[resp_buff_len];
    SC_STATUS status;
    status = sAPI_MsgQCreate(&g_sms_demo_msgQ, "g_sms_demo_msgQ", sizeof(SIM_MSG_T), 4, SC_FIFO);
    if (status != SC_SUCCESS)
    {
        LOG_ERROR("ERROR: message queue creat err!\n");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "ERROR: message queue creat err!";
        goto EXIT;
    }

    pl_demo_get_data(&sms_data, argv, argn, ARGSMS_SMSDATA);
    pl_demo_get_value(&smslen, argv, argn, ARGSMS_SMSLEN);
    pl_demo_get_data(&srcAddr, argv, argn, ARGSMS_SRCADDR);
    LOG_INFO("%s,sms_data=%s,smslen=%d,srcAddr=%s",__func__,sms_data,smslen,srcAddr);
    retval = sAPI_SmsSetFormat(1); //only support text mode

    retval = sAPI_SmsSendLongMsg((UINT8 *)sms_data, smslen, (UINT8 *)srcAddr, g_sms_demo_msgQ);
    if (retval == SC_SMS_SUCESS)
    {
        memset(&msg, 0, sizeof(msg));
        sAPI_MsgQRecv(g_sms_demo_msgQ, &msg, SMS_URC_RECIVE_TIME_OUT);
        sAPI_Debug("[sms], sAPI_SmsSendLongMsg, primID[%d] resultCode[%d],rspStr[%s]", msg.arg1, msg.arg2, msg.arg3);
        sprintf(rsp_buff, "rspStr[%s]",(char *)msg.arg3);
        sAPI_Debug("[sms]rsp_buff=%s",rsp_buff);
        sAPI_Free(msg.arg3);

        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = ARGSMS_SENDSMSINFO;
        out_arg[0].value = pl_demo_make_value(0, rsp_buff, strlen(rsp_buff));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg,1,NULL,0);
        if (0 != ret.errcode)
        {
            ret.msg = "post data fail";
            goto EXIT;
        }
    }
    else
    {
        LOG_ERROR("sAPI_SmsSendLongMsg send long sms fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_SmsSendLongMsg send long sms fail!!!";
        goto EXIT;
    }
    if(g_sms_demo_msgQ != NULL)
        sAPI_MsgQDelete(g_sms_demo_msgQ);

EXIT:
    pl_demo_release_value(out_arg[0].value);
    return ret;
}

static ret_msg_t delete_allmsg_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    SC_SMSReturnCode retval = SC_SMS_FAIL;

    SC_STATUS status;
    SIM_MSG_T msg;
    arg_t out_arg[1] = {0};
    int resp_buff_len = 100;
    char rsp_buff[resp_buff_len];

    status = sAPI_MsgQCreate(&g_sms_demo_msgQ, "g_sms_demo_msgQ", sizeof(SIM_MSG_T), 4, SC_FIFO);
    if (status != SC_SUCCESS)
    {
        LOG_ERROR("ERROR: message queue creat err!\n");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "ERROR: message queue creat err!";
        return ret;
    }

    retval = sAPI_SmsDelAllMsg(g_sms_demo_msgQ);
    sAPI_Debug("%s, %d, msg write, ret=%d!!", __func__, __LINE__, retval);
    if(retval != SC_SMS_SUCESS)
    {
        LOG_ERROR("sAPI_SmsDelAllMsg delete sms fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_SmsDelAllMsg delete sms fail!!!";
    }
    else
    {
		memset(&msg, 0, sizeof(msg));
		sAPI_MsgQRecv(g_sms_demo_msgQ, &msg, SMS_URC_RECIVE_TIME_OUT);
		LOG_INFO("[sms], sAPI_SmsSendLongMsg, primID[%d] resultCode[%d],rspStr[%s]", msg.arg1, msg.arg2, msg.arg3);
		strcpy(rsp_buff, "rspStr[delete all msg sucess]");
		sAPI_Free(msg.arg3);

		out_arg[0].type = TYPE_STR;
		out_arg[0].arg_name = ARGSMS_DELALLSMSINFO;
		out_arg[0].value = pl_demo_make_value(0, rsp_buff, strlen(rsp_buff));
		ret.errcode = pl_demo_post_data(OP_POST, out_arg,1,NULL,0);
		if (0 != ret.errcode)
		{
			ret.msg = "post data fail";
			goto EXIT;
		}
    }
    if(g_sms_demo_msgQ != NULL)
        sAPI_MsgQDelete(g_sms_demo_msgQ);
EXIT:
	pl_demo_release_value(out_arg[0].value);
	return ret;
}

static ret_msg_t delete_onemsg_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    SC_SMSReturnCode retval = SC_SMS_FAIL;
    int index = 0;
    SC_STATUS status;
    SIM_MSG_T msg;
    arg_t out_arg[1] = {0};
    int resp_buff_len = 100;
    char rsp_buff[resp_buff_len];

    status = sAPI_MsgQCreate(&g_sms_demo_msgQ, "g_sms_demo_msgQ", sizeof(SIM_MSG_T), 4, SC_FIFO);
    if (status != SC_SUCCESS)
    {
        LOG_ERROR("ERROR: message queue creat err!\n");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "ERROR: message queue creat err!";
        return ret;
    }

    pl_demo_get_value(&index, argv, argn, ARGSMS_SMSINDEX);
    LOG_INFO("%s,index=%d",__func__,index);

    retval = sAPI_SmsDelOneMsg(index, g_sms_demo_msgQ);
    sAPI_Debug("%s, %d, msg del, ret=%d!!", __func__, __LINE__, retval);
    if(retval != SC_SMS_SUCESS)
    {
        LOG_ERROR("sAPI_SmsDelOneMsg delete sms fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_SmsDelOneMsg delete sms fail!!!";
    }
    else
    {
		memset(&msg, 0, sizeof(msg));
		sAPI_MsgQRecv(g_sms_demo_msgQ, &msg, SMS_URC_RECIVE_TIME_OUT);
		LOG_INFO("[sms], sAPI_SmsSendLongMsg, primID[%d] resultCode[%d],rspStr[%s]", msg.arg1, msg.arg2, msg.arg3);
		sprintf(rsp_buff, "rspStr[%s delete %d sucess]",(char *)msg.arg3,index);

		sAPI_Debug("[sms]rsp_buff=%s",rsp_buff);
		sAPI_Free(msg.arg3);

		out_arg[0].type = TYPE_STR;
		out_arg[0].arg_name = ARGSMS_DELONESMSINFO;
		out_arg[0].value = pl_demo_make_value(0, rsp_buff, strlen(rsp_buff));
		ret.errcode = pl_demo_post_data(OP_POST, out_arg,1,NULL,0);
		if (0 != ret.errcode)
		{
			ret.msg = "post data fail";
			goto EXIT;
		}
    }
    if(g_sms_demo_msgQ != NULL)
        sAPI_MsgQDelete(g_sms_demo_msgQ);
EXIT:
	pl_demo_release_value(out_arg[0].value);
	return ret;

}

static ret_msg_t set_newmsgind_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    SC_SMSReturnCode retval = SC_SMS_FAIL;
    const char *formatmodestr = NULL;
    const char *modstr = NULL;
    const char *mtstr = NULL;
    const char *bmstr = NULL;
    const char *dsstr = NULL;
    const char *bfrstr = NULL;
    int fmatmode,mode,mt,bm,ds,bfr = 0;

    pl_demo_get_data(&formatmodestr, argv, argn, ARGSMS_FORMATMODE);
    pl_demo_get_data(&modstr, argv, argn, ARGSMS_SMSMODE);
    pl_demo_get_data(&mtstr, argv, argn, ARGSMS_SMSMT);
    pl_demo_get_data(&bmstr, argv, argn, ARGSMS_SMSBM);
    pl_demo_get_data(&dsstr, argv, argn, ARGSMS_SMSDS);
    pl_demo_get_data(&bfrstr, argv, argn, ARGSMS_SMSBFR);

    fmatmode = smsFormatjudge(formatmodestr);
    if(strncmp(modstr,"mode:2",6) == 0)
        mode = 2;
    else if(strncmp(modstr,"mode:1",6) == 0)
        mode = 1;
    else
        mode = 0;

    if(strncmp(mtstr,"mt:1",4) == 0)
        mt = 1;
    else if(strncmp(mtstr,"mt:0",4) == 0)
        mt = 0;
    else if(strncmp(mtstr,"mt:2",4) == 0)
        mt = 2;
    else
        mt =3;

    if(strncmp(bmstr,"bm:0",4) == 0)
        bm = 0;
    else
        bm = 2;

    if(strncmp(dsstr,"ds:0",4) == 0)
        ds = 0;
    else if(strncmp(dsstr,"ds:1",4) == 0)
        ds = 1;
    else
        ds = 2;

    if(strncmp(bfrstr,"bfr:0",5) == 0)
        bfr = 0;
    else
        bfr = 1;

    LOG_INFO("%s,fmatmode=%d,mode=%d,mt=%d,bm=%d,ds=%d,bfr=%d",__func__,fmatmode,mode,mt,bm,ds,bfr);

    retval = sAPI_SmsSetNewMsgInd((INT32)fmatmode, (INT32)mode, (INT32)mt, (INT32)bm, (INT32)ds, (INT32)bfr);
    if(retval != SC_SMS_SUCESS)
    {
        LOG_ERROR("sAPI_SmsSetNewMsgInd set new sms ind fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_SmsSetNewMsgInd set new sms ind fail!!!";
    }
    return ret;
}

static ret_msg_t get_newmsgind_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    SC_SMSReturnCode retval = SC_SMS_FAIL;
    UINT32 mode,mt,bm,ds,bfr = 0;
    char rsp_buf[100] = {0};
    arg_t out_arg[1] = {0};

    retval = sAPI_SmsGetNewMsgInd(&mode,&mt,&bm,&ds,&bfr);
    if(retval == SC_SMS_SUCESS)
    {
        sprintf(rsp_buf,"mode:%ld mt:%ld bm:%ld ds:%ld bfr:%ld",mode,mt,bm,ds,bfr);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = ARGSMS_GETNEWMSGIND;
        out_arg[0].value = pl_demo_make_value(0, rsp_buf, strlen(rsp_buf));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg,1,NULL,0);
        if (0 != ret.errcode)
        {
            ret.msg = "post data fail";
            goto EXIT;
        }
    }
    else
    {
        LOG_ERROR("sAPI_SmsGetNewMsgInd get new sms ind fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_SmsGetNewMsgInd get new sms ind fail!!!";
        goto EXIT;
    }

EXIT:
    pl_demo_release_value(out_arg[0].value);
    return ret;
}

static ret_msg_t set_format_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_SMSReturnCode retval = SC_SMS_FAIL;
    const char *formatmodestr = NULL;
    char format = 0;

    pl_demo_get_data(&formatmodestr, argv, argn, ARGSMS_FORMATMODE);
    format = smsFormatjudge(formatmodestr);

    LOG_INFO("%s,format=%d",__func__,format);

    retval = sAPI_SmsSetFormat((UINT8)format);
    if(retval != SC_SMS_SUCESS)
    {
        LOG_ERROR("sAPI_SmsSetFormat set sms format fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_SmsSetFormat set sms format fail!!!";
    }
    return ret;
}

static ret_msg_t get_format_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_SMSReturnCode retval = SC_SMS_FAIL;
    UINT8 format = 0;
    char rsp_buf[100] = {0};
    arg_t out_arg[1] = {0};

    retval = sAPI_SmsgetFormat(&format);
    if(retval == SC_SMS_SUCESS)
    {
        sprintf(rsp_buf,"format=%d",format);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = ARGSMS_GETFORMAT;
        out_arg[0].value = pl_demo_make_value(0, rsp_buf, strlen(rsp_buf));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg,1,NULL,0);
        if (0 != ret.errcode)
        {
            ret.msg = "post data fail";
            goto EXIT;
        }
    }
    else
    {
        LOG_ERROR("sAPI_SmsgetFormat get sms format fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_SmsgetFormat get sms format fail!!!";
        goto EXIT;
    }

EXIT:
    pl_demo_release_value(out_arg[0].value);
    return ret;
}

static ret_msg_t decode_pdu_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_SMSReturnCode retval = SC_SMS_FAIL;
    char rsp_buf[1000] = {0};
    arg_t out_arg[1] = {0};
    const char *pdu_data = NULL;
    char addrbuf[100] = {0};
    int len = 0;
    char txtbuf[500] = {0};

    pl_demo_get_data(&pdu_data, argv, argn, ARGSMS_SMSPDU);
    LOG_INFO("%s,pdu_data=%s",__func__,pdu_data);

    retval = sAPI_SmsDecodePdu((char *)pdu_data,addrbuf,&len,txtbuf);
    if(retval == SC_SMS_SUCESS)
    {
        LOG_INFO("pdu to txt sucess!Addr[%s]len[%d]Txtbuf[%s]", addrbuf,len, txtbuf);
        sprintf(rsp_buf, "Addr[%s] len[%d] outbuf[%s]", addrbuf,len, txtbuf);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = ARGSMS_DECODEPDU;
        out_arg[0].value = pl_demo_make_value(0, rsp_buf, strlen(rsp_buf));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg,1,NULL,0);
        if (0 != ret.errcode)
        {
            ret.msg = "post data fail";
            goto EXIT;
        }
    }
    else
    {
        LOG_ERROR("sAPI_SmsDecodePdu pdu sms fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_SmsDecodePdu pdu sms fail!!!";
        goto EXIT;
    }

EXIT:
    pl_demo_release_value(out_arg[0].value);
    return ret;
}

static ret_msg_t encode_pdu_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_SMSReturnCode retval = SC_SMS_FAIL;
    char rsp_buf[1000] = {0};
    arg_t out_arg[1] = {0};
    const char *sms_data = NULL;
    const char *addr_data = NULL;
    int sms_len = 0;
    int pduLen = 0;
    char pduText[500] = {0};

    pl_demo_get_data(&addr_data, argv, argn, ARGSMS_SRCADDR);
    pl_demo_get_data(&sms_data, argv, argn, ARGSMS_SMSDATA);
    pl_demo_get_value(&sms_len, argv, argn, ARGSMS_SMSLEN);
    LOG_INFO("%s,addr_data=%s,sms_data=%s,sms_len=%d",__func__,addr_data,sms_data,sms_len);

    retval = sAPI_SmsEncodePdu((char *)addr_data, (char *)sms_data,sms_len, pduText, &pduLen);
    if(retval == SC_SMS_SUCESS)
    {
        LOG_INFO("pdu to txt sucess! pduText[%s] pdulen[%d]", pduText,pduLen);
        sprintf(rsp_buf, "pduText[%s] pdulen[%d]", pduText,pduLen);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = ARGSMS_ENCODEPDU;
        out_arg[0].value = pl_demo_make_value(0, rsp_buf, strlen(rsp_buf));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg,1,NULL,0);
        if (0 != ret.errcode)
        {
            ret.msg = "post data fail";
            goto EXIT;
        }
    }
    else
    {
        LOG_ERROR("sAPI_SmsEncodePdu pdu sms fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_SmsEncodePdu pdu sms fail!!!";
        goto EXIT;
    }

EXIT:
    pl_demo_release_value(out_arg[0].value);
    return ret;
}

static ret_msg_t set_chest_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_SMSReturnCode retval = SC_SMS_FAIL;
    const char *cheststr = NULL;
    char chest = 0;

    pl_demo_get_data(&cheststr, argv, argn, ARGSMS_SMSCHEST);
    if(strncmp(cheststr,"GSM:0",5) == 0)
        chest = 0;
    else if(strncmp(cheststr,"HEX:1",5) == 0)
        chest = 1;
    else if(strncmp(cheststr,"IRA:2",5) == 0)
        chest = 2;
    else
        chest = 4;
    LOG_INFO("%s,chest=%d",__func__,chest);

    retval = sAPI_SmssetChset((UINT8)chest);
    if(retval != SC_SMS_SUCESS)
    {
        LOG_ERROR("sAPI_SmssetChset set sms chest fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_SmssetChset set sms chest fail!!!";
    }
    return ret;
}

static ret_msg_t get_chest_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_SMSReturnCode retval = SC_SMS_FAIL;
    UINT8 chest = 0;
    arg_t out_arg[1] = {0};
    char rsp_buf[100] = {0};
    char tmpbuf[50] = {0};

    retval = sAPI_SmsgetChset(&chest);
    if(retval == SC_SMS_SUCESS)
    {
        if(chest == 0)
            strcpy(tmpbuf,"GSM");
        else if(chest == 1)
            strcpy(tmpbuf,"HEX");
        else if(chest == 2)
            strcpy(tmpbuf,"IRA");
        else if(chest == 4)
            strcpy(tmpbuf,"UCS2");

        LOG_INFO("get cscs sucess format=%d,buf=%s",chest,tmpbuf);

        sprintf(rsp_buf,"chest=%s",tmpbuf);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = ARGSMS_GETCHEST;
        out_arg[0].value = pl_demo_make_value(0, rsp_buf, strlen(rsp_buf));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg,1,NULL,0);
        if (0 != ret.errcode)
        {
            ret.msg = "post data fail";
            goto EXIT;
        }
    }
    else
    {
        LOG_ERROR("sAPI_SmsgetChset get sms chest fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_SmsgetChset get sms chest fail!!!";
        goto EXIT;
    }

EXIT:
    pl_demo_release_value(out_arg[0].value);
    return ret;
}

