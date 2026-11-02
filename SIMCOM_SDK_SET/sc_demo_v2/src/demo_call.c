
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "demo_call.h"
#include "simcom_os.h"
#include "simcom_call.h"

sMsgQRef g_call_demo_msgQ;

#define CALL_URC_RECIVE_TIME_OUT 20000

#define LOG_ERROR(...) sal_log_error("[DEMO-CALL] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-CALL] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-CALL] " __VA_ARGS__)

static ret_msg_t call_result_raw_proc(char *buf);
static ret_msg_t call_dial_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t call_answer_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t call_end_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t call_autoanswer_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t call_callstate_cmd_proc(op_t op, arg_t *argv, int argn);

arg_t call_dial_args[1]=
{
	{
		.type = TYPE_STR,
		.arg_name = ARG_DIALNUMBER,
		.msg = "Please input dial number"
	}
};

const menu_list_t func_call_dial =
{
	.menu_name = "dial",
	.menu_type = TYPE_FUNC,
	.parent    = &call_menu,
	.function  = {
		.argv = call_dial_args,
		.argn = sizeof(call_dial_args)/sizeof(arg_t),
		.methods.cmd_handler = call_dial_cmd_proc,
	},
};

const menu_list_t func_call_answer =
{
	.menu_name = "answer",
	.menu_type = TYPE_FUNC,
	.parent    = &call_menu,
	.function  = {
		.methods.cmd_handler = call_answer_cmd_proc,
	},
};

const menu_list_t func_call_end =
{
	.menu_name = "end",
	.menu_type = TYPE_FUNC,
	.parent    = &call_menu,
	.function  = {
		.methods.cmd_handler = call_end_cmd_proc,
	},
};

arg_t call_auotoanswer_args[1]=
{
	{
		.type = TYPE_INT,
		.arg_name = ARG_ATUOANSWERTIME,
		.msg = "auto answer after x seconds"
	}
};

const menu_list_t func_call_autoanswer =
{
	.menu_name = "autoanswer",
	.menu_type = TYPE_FUNC,
	.parent    = &call_menu,
	.function  = {
		.argv = call_auotoanswer_args,
		.argn = sizeof(call_auotoanswer_args)/sizeof(arg_t),
		.methods.cmd_handler = call_autoanswer_cmd_proc,
	},
};

const menu_list_t func_call_callstate =
{
	.menu_name = "callstate",
	.menu_type = TYPE_FUNC,
	.parent    = &call_menu,
	.function  = {
		.methods.cmd_handler = call_callstate_cmd_proc,
	},
};

const menu_list_t *call_sub_menu[] =
{
	&func_call_dial,
	&func_call_answer,
	&func_call_end,
	&func_call_autoanswer,
	&func_call_callstate,
	NULL
};


const menu_list_t call_menu =
{
	.menu_name = "call",
	.menu_type = TYPE_MENU,
	.__init = NULL,
	.__uninit = NULL,
	.submenu_list_head = call_sub_menu,
	.parent = &root_menu,
};


static ret_msg_t call_result_raw_proc(char *buf)
{
	ret_msg_t ret = {0};
	arg_t out_arg[1] = {0};

	out_arg[0].arg_name = pre_arg_name[ARG_ID_DATA];
	out_arg[0].type = TYPE_RAW;
	out_arg[0].value = pl_demo_make_value(0, NULL, strlen(buf));

	ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
	pl_demo_post_raw(buf, strlen(buf));

	return ret;
}


static ret_msg_t call_dial_cmd_proc(op_t op, arg_t *argv, int argn)
{
	ret_msg_t ret = {0};
	const char *dialnumber = NULL;
	SC_STATUS status;
	char rsp_buff[100];
	SIM_MSG_T msg;

	pl_demo_get_data(&dialnumber, argv, argn, ARG_DIALNUMBER);

	status = sAPI_MsgQCreate(&g_call_demo_msgQ, "g_call_demo_msgQ", sizeof(SIM_MSG_T), 4, SC_FIFO);
	if (status != SC_SUCCESS)
	{
		ret.errcode = status;
		ret.msg = "ERROR: message queue creat err!\n";
	}
	else
	{
		ret.errcode = sAPI_CallDialMsg((unsigned char *)dialnumber, g_call_demo_msgQ);

		if(ret.errcode == SC_CALL_SUCESS)
		{
			memset(&msg, 0, sizeof(msg));
			status = sAPI_MsgQRecv(g_call_demo_msgQ, &msg, CALL_URC_RECIVE_TIME_OUT);
			sprintf(rsp_buff, "sAPI_CallDialMsg: resultCode[%d]\r\n", msg.arg2);
			call_result_raw_proc(rsp_buff);
		}
		sAPI_MsgQDelete(g_call_demo_msgQ);
	}
	return ret;
}

static ret_msg_t call_answer_cmd_proc(op_t op, arg_t *argv, int argn)
{
	ret_msg_t ret = {0};
	char rsp_buff[100];
	SIM_MSG_T msg;
	SC_STATUS status;
	status = sAPI_MsgQCreate(&g_call_demo_msgQ, "g_call_demo_msgQ", sizeof(SIM_MSG_T), 4, SC_FIFO);
	if(status != SC_SUCCESS)
	{
		ret.errcode = status;
		ret.msg = "ERROR: message queue creat err!\n";
	}
	else
	{
		ret.errcode = sAPI_CallAnswerMsg(g_call_demo_msgQ);
		if(ret.errcode == SC_CALL_SUCESS)
		{
			memset(&msg, 0, sizeof(msg));
			status = sAPI_MsgQRecv(g_call_demo_msgQ, &msg, CALL_URC_RECIVE_TIME_OUT);
			sprintf(rsp_buff, "sAPI_CallAnswerMsg: resultCode[%d]\r\n", msg.arg2);
			call_result_raw_proc(rsp_buff);
		}
		sAPI_MsgQDelete(g_call_demo_msgQ);
	}
	return ret;
}

static ret_msg_t call_end_cmd_proc(op_t op, arg_t *argv, int argn)
{
	ret_msg_t ret = {0};
	char rsp_buff[100];
	SIM_MSG_T msg;
	SC_STATUS status;

	status = sAPI_MsgQCreate(&g_call_demo_msgQ, "g_call_demo_msgQ", sizeof(SIM_MSG_T), 4, SC_FIFO);
	if(status != SC_SUCCESS)
	{
		ret.errcode = status;
		ret.msg = "ERROR: message queue creat err!\n";
	}
	else
	{
		ret.errcode = sAPI_CallEndMsg(g_call_demo_msgQ);
		if(ret.errcode == SC_CALL_SUCESS)
		{
			memset(&msg, 0, sizeof(msg));
			status = sAPI_MsgQRecv(g_call_demo_msgQ, &msg, CALL_URC_RECIVE_TIME_OUT);
			sprintf(rsp_buff, "sAPI_CallEndMsg: resultCode[%d]\r\n", msg.arg2);
			call_result_raw_proc(rsp_buff);
		}
		sAPI_MsgQDelete(g_call_demo_msgQ);
	}

	return ret;
}

static ret_msg_t call_autoanswer_cmd_proc(op_t op, arg_t *argv, int argn)
{
	ret_msg_t ret = {0};
	int autoanswertime = 0;
	SC_STATUS status;
	char rsp_buff[100];
	SIM_MSG_T msg;

	pl_demo_get_value(&autoanswertime, argv, argn, ARG_ATUOANSWERTIME);

	status = sAPI_MsgQCreate(&g_call_demo_msgQ, "g_call_demo_msgQ", sizeof(SIM_MSG_T), 4, SC_FIFO);
	if(status != SC_SUCCESS)
	{
		ret.errcode = status;
		ret.msg = "ERROR: message queue creat err!\n";
	}
	else
	{
		ret.errcode = sAPI_CallAutoAnswer(autoanswertime,g_call_demo_msgQ);
		if(ret.errcode == SC_CALL_SUCESS)
		{
			memset(&msg, 0, sizeof(msg));
			status = sAPI_MsgQRecv(g_call_demo_msgQ, &msg, CALL_URC_RECIVE_TIME_OUT);
			sprintf(rsp_buff, "sAPI_CallAutoAnswer: resultCode[%d]\r\n", msg.arg2);
			call_result_raw_proc(rsp_buff);
		}
		sAPI_MsgQDelete(g_call_demo_msgQ);
	}
	return ret;
}

static ret_msg_t call_callstate_cmd_proc(op_t op, arg_t *argv, int argn)
{
	ret_msg_t ret = {0};
	char rsp_buff[100];
	SIM_MSG_T msg;
	unsigned char callstate = 9;

	callstate = sAPI_CallStateMsg();

	memset(&msg, 0, sizeof(msg));
	sprintf(rsp_buff, "sAPI_CallStateMsg: callstate[%d]\r\n", callstate);
	call_result_raw_proc(rsp_buff);

	return ret;
}

