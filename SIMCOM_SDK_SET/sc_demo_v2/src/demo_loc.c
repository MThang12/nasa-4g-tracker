/**
  ******************************************************************************
  * @file    demo_loc.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of location feature from LBS.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "simcom_os.h"
#include "simcom_loc.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "simcom_uart.h"
#include "uart_api.h"
#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "simcom_network.h"

#define LOG_ERROR(...) sal_log_error("[DEMO-LBS] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-LBS] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-LBS] " __VA_ARGS__)
#define ARGLBS_UPDATE_RESULT "updateResult"
sSemaRef lbsDemo2testSema;
sMsgQRef lbsDemo2TestMsgq;
sMsgQRef SC_Lbstest2Resp_msgq;
sTaskRef lbsDemo2TestTask;
static unsigned char lbsDemo2TestTaskStack[1024 * 4];

static ret_msg_t LbsMsgqInit(void);

static ret_msg_t func_lbs_get_lonlat_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t func_lbs_get_detailaddress_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t func_lbs_get_error_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t func_lbs_get_lonlattime_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t func_lbs_test_proc(op_t op, arg_t *argv, int argn);
void sTask_lbsTestProcesser(void * arg);

static int lbsdemoCurrentTime = 1;
static int lbsdemoTestTime = 0;
static sMsgQRef SC_Lbs2Resp_msgq;


const menu_list_t func_lbs_get_lonlat=
{
    .menu_name = "get longitude and latitude",
    .menu_type = TYPE_FUNC,
    .parent = &lbs_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = func_lbs_get_lonlat_proc,
    },
};
		
const menu_list_t func_lbs_get_detailaddress=
{
    .menu_name = "get detail address",
    .menu_type = TYPE_FUNC,
    .parent = &lbs_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = func_lbs_get_detailaddress_proc,
    },
};
const menu_list_t func_lbs_get_error=
{
    .menu_name = "get error number",
    .menu_type = TYPE_FUNC,
    .parent = &lbs_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = func_lbs_get_error_proc,
    },
};		
const menu_list_t func_lbs_get_lonlattime=
{
    .menu_name = "get longitude latitude and date time",
    .menu_type = TYPE_FUNC,
    .parent = &lbs_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = func_lbs_get_lonlattime_proc,
    },
};			
arg_t testvalue_args[1] =
{
	{
		.type = TYPE_INT,
		.arg_name = "Testtime",
		.msg = "Please input test time."
	},


};
	
const menu_list_t func_lbs_test=
{
    .menu_name = "test the API for LBS",
    .menu_type = TYPE_FUNC,
    .parent = &lbs_menu,
    .function = {
        .argv = testvalue_args,
        .argn = 1,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = func_lbs_test_proc,
    },
};			
const menu_list_t *lbs_menu_sub[] =
{
    &func_lbs_get_lonlat,
    &func_lbs_get_detailaddress,
    &func_lbs_get_error,
    &func_lbs_get_lonlattime,
    &func_lbs_test,
    NULL  // MUST end by NULL
};
const menu_list_t lbs_menu =
{
	.menu_name = "LBS",
	.menu_type = TYPE_MENU,
	.__init = LbsMsgqInit,
	.__uninit = NULL,
	.submenu_list_head = lbs_menu_sub,
	.parent = &root_menu,
};


/**
  * @brief  LBS demo
  * @param  void
  * @note   Need to insert SIM card and get registered before LBS operation.
  * @retval void
  */
static ret_msg_t LbsMsgqInit(void)
{
	ret_msg_t ret = {0};
	if (SC_Lbs2Resp_msgq == NULL)
	{
		ret.errcode = sAPI_MsgQCreate(&SC_Lbs2Resp_msgq, "SC_LbsResp_msgq", (sizeof(SIM_MSG_T)), 4, SC_FIFO);
		if (SC_SUCCESS != ret.errcode)
		{
			LOG_ERROR("osaStatus = [%d],create msg error!", ret.errcode);
			ret.errcode = ERRCODE_DEFAULT;
			ret.msg = "create msg error";
		}
	}
	else
	{
		LOG_INFO("msg has been created!");
	}

	return ret;
}
static ret_msg_t func_lbs_get_lonlat_proc(op_t op, arg_t *argv, int argn)
{
	ret_msg_t ret = {0};
	INT16 channel = 0;
	char path[300] = {0};
	int bufsize = 0;
	arg_t out_arg[1] = { 0 };
	out_arg[0].arg_name = ARGLBS_UPDATE_RESULT;
	out_arg[0].type = TYPE_RAW;
	
	ret.errcode = sAPI_LocGet(channel, SC_Lbs2Resp_msgq, 1);
	if (ret.errcode != SC_LBS_SUCCESS)
	{
		LOG_ERROR("get location fail!\r\n");
		ret.errcode = ERRCODE_DEFAULT;
		ret.msg = "LBS get longitude and latitude Fail!";
	}
	else
	{
		LOG_INFO("get location success!\r\n");
		SIM_MSG_T msgQ_data_recv = {0, 0, 0, NULL};
		SC_lbs_info_t *sub_data = NULL;
		ret.errcode = sAPI_MsgQRecv(SC_Lbs2Resp_msgq, &msgQ_data_recv, SC_SUSPEND);
		if (SC_SUCCESS != ret.errcode)
		{
			LOG_ERROR("osaStatus = [%d],recv msg error!", ret.errcode);
			ret.errcode = ERRCODE_DEFAULT;
			ret.msg = "LBS get longitude and latitude Fail!";
		}
		else
		{
			if (msgQ_data_recv.msg_id == SC_SRV_LBS)
			{
				LOG_INFO("get location success 2!\r\n");
				sub_data = (SC_lbs_info_t *)msgQ_data_recv.arg3;
				if (sub_data->u8ErrorCode != 0)
				{
					bufsize = snprintf(path, sizeof(path), "LBS get longitude and latitude Fail!Recv LBS info:errocode:%d",
							sub_data->u8ErrorCode);
					out_arg[0].value = pl_demo_make_value(0,path, bufsize);
					sAPI_Free(sub_data);
				}
				else
				{
					bufsize = snprintf(path, sizeof(path), "Recv LBS info:errocode:%d,Lng:%f,Lat:%f,Acc:%d", sub_data->u8ErrorCode,
						(float)(sub_data->u32Lng) / 1000000, (float)(sub_data->u32Lat) / 1000000, sub_data->u16Acc);
					LOG_INFO("%s", path);
					out_arg[0].value = pl_demo_make_value(0,path, bufsize);
					sAPI_Free(sub_data);
				}
				ret.errcode = pl_demo_post_urc(&func_lbs_get_lonlat,TYPE_URC_NOTICE, out_arg, 1, NULL, 0);

				pl_demo_release_value(out_arg[0].value);
			}
		}
	}
	return ret;
}
static ret_msg_t func_lbs_get_detailaddress_proc(op_t op, arg_t *argv, int argn)
{
	ret_msg_t ret = {0};
	INT16 channel = 0;
	char path[300] = {0};
	int bufsize = 0;
	arg_t out_arg[1] = { 0 };
	out_arg[0].arg_name = ARGLBS_UPDATE_RESULT;
	out_arg[0].type = TYPE_RAW;
	
	ret.errcode = sAPI_LocGet(channel, SC_Lbs2Resp_msgq, 2);
	if (ret.errcode != SC_LBS_SUCCESS)
	{
		LOG_ERROR("get location fail!\r\n");
		ret.errcode = ERRCODE_DEFAULT;
		ret.msg = "LBS get detail address Fail!";
	}
	else
	{
		sAPI_Debug("get location success!\r\n");
		SIM_MSG_T msgQ_data_recv = {0, 0, 0, NULL};
		SC_lbs_info_t *sub_data = NULL;
		ret.errcode = sAPI_MsgQRecv(SC_Lbs2Resp_msgq, &msgQ_data_recv, SC_SUSPEND);
		if (SC_SUCCESS != ret.errcode)
		{
			LOG_ERROR("osaStatus = [%d],recv msg error!", ret.errcode);
			ret.errcode = ERRCODE_DEFAULT;
			ret.msg = "LBS get detail address Fail!";
		}
		else
		{
			if (msgQ_data_recv.msg_id == SC_SRV_LBS)
			{
				sub_data = (SC_lbs_info_t *)msgQ_data_recv.arg3;
				if (sub_data->u8ErrorCode != 0)
				{
					bufsize = snprintf(path, sizeof(path), "LBS get longitude and latitude Fail!Recv LBS info:errocode:%d",
							sub_data->u8ErrorCode);
					out_arg[0].value = pl_demo_make_value(0,path, bufsize);
					sAPI_Free(sub_data);
				}
				else
				{
					char tempBuf[151];
					memset(tempBuf, 0, 151);
					char *p_tmp = tempBuf;
					UINT16 i;
					for (i = 0; i < sub_data->u32AddrLen; i++)
					{
						p_tmp += sprintf(p_tmp, "%02x", sub_data->u8LocAddress[i]);
						sAPI_Debug("%02x", sub_data->u8LocAddress[i]);
					}
					bufsize = snprintf(path, sizeof(path), "Recv LBS info:errocode:%d,detail_addr:%s", sub_data->u8ErrorCode, tempBuf);
					out_arg[0].value = pl_demo_make_value(0,path, bufsize);
					sAPI_Free(sub_data);
				}
				ret.errcode = pl_demo_post_urc(&func_lbs_get_detailaddress,TYPE_URC_NOTICE, out_arg, 1, NULL, 0);

				pl_demo_release_value(out_arg[0].value);
			}
		}
	}
	return ret;
}

static ret_msg_t func_lbs_get_error_proc(op_t op, arg_t *argv, int argn)
{
	ret_msg_t ret = {0};
	INT16 channel = 0;
	char path[300] = {0};
	int bufsize = 0;
	arg_t out_arg[1] = { 0 };
	out_arg[0].arg_name = ARGLBS_UPDATE_RESULT;
	out_arg[0].type = TYPE_RAW;
	
	ret.errcode = sAPI_LocGet(channel, SC_Lbs2Resp_msgq, 3);
	if (ret.errcode != SC_LBS_SUCCESS)
	{
		LOG_ERROR("get location fail!\r\n");
		ret.errcode = ERRCODE_DEFAULT;
		ret.msg = "LBS get detail address Fail!";
	}
	else
	{
		sAPI_Debug("get location success!\r\n");
		SIM_MSG_T msgQ_data_recv = {0, 0, 0, NULL};
		SC_lbs_info_t *sub_data = NULL;
		ret.errcode = sAPI_MsgQRecv(SC_Lbs2Resp_msgq, &msgQ_data_recv, SC_SUSPEND);
		if (SC_SUCCESS != ret.errcode)
		{
			LOG_ERROR("osaStatus = [%d],recv msg error!", ret.errcode);
			ret.errcode = ERRCODE_DEFAULT;
			ret.msg = "LBS get detail address Fail!";
		}
		else
		{
			if (msgQ_data_recv.msg_id == SC_SRV_LBS)
			{
				sub_data = (SC_lbs_info_t *)msgQ_data_recv.arg3;
				if (sub_data->u8ErrorCode != 0)
				{
					bufsize = snprintf(path, sizeof(path), "LBS get longitude and latitude Fail!Recv LBS info:errocode:%d",
							sub_data->u8ErrorCode);
					out_arg[0].value = pl_demo_make_value(0,path, bufsize);
					sAPI_Free(sub_data);
				}
				else
				{
					bufsize = snprintf(path, sizeof(path), "Recv LBS info:errocode:%d", sub_data->u8ErrorCode);
					out_arg[0].value = pl_demo_make_value(0,path, bufsize);
					sAPI_Free(sub_data);
				}
				ret.errcode = pl_demo_post_urc(&func_lbs_get_error,TYPE_URC_NOTICE, out_arg, 1, NULL, 0);

				pl_demo_release_value(out_arg[0].value);
			}
		}
	}
	return ret;
}

static ret_msg_t func_lbs_get_lonlattime_proc(op_t op, arg_t *argv, int argn)
{
	ret_msg_t ret = {0};
	INT16 channel = 0;
	char path[300] = {0};
	int bufsize = 0;
	arg_t out_arg[1] = { 0 };
	out_arg[0].arg_name = ARGLBS_UPDATE_RESULT;
	out_arg[0].type = TYPE_RAW;
	
	ret.errcode = sAPI_LocGet(channel, SC_Lbs2Resp_msgq, 4);
	if (ret.errcode != SC_LBS_SUCCESS)
	{
		LOG_ERROR("get location fail!\r\n");
		ret.errcode = ERRCODE_DEFAULT;
		ret.msg = "LBS get detail address Fail!";
	}
	else
	{
		sAPI_Debug("get location success!\r\n");
		SIM_MSG_T msgQ_data_recv = {0, 0, 0, NULL};
		SC_lbs_info_t *sub_data = NULL;
		ret.errcode = sAPI_MsgQRecv(SC_Lbs2Resp_msgq, &msgQ_data_recv, SC_SUSPEND);
		if (SC_SUCCESS != ret.errcode)
		{
			LOG_ERROR("osaStatus = [%d],recv msg error!", ret.errcode);
			ret.errcode = ERRCODE_DEFAULT;
			ret.msg = "LBS get detail address Fail!";
		}
		else
		{
			if (msgQ_data_recv.msg_id == SC_SRV_LBS)
			{
				sub_data = (SC_lbs_info_t *)msgQ_data_recv.arg3;
				if (sub_data->u8ErrorCode != 0)
				{
					bufsize = snprintf(path, sizeof(path), "LBS get longitude and latitude Fail!Recv LBS info:errocode:%d",
							sub_data->u8ErrorCode);
					out_arg[0].value = pl_demo_make_value(0,path, bufsize);
					sAPI_Free(sub_data);
				}
				else
				{
					bufsize = snprintf(path, sizeof(path), "Recv LBS info:errocode %d,Lng:%ld.%ld,Lat:%ld.%ld,Acc:%d,DateAndTime:%s",
										sub_data->u8ErrorCode, (sub_data->u32Lng) / 1000000, (sub_data->u32Lng) % 1000000, (sub_data->u32Lat) / 1000000,
										(sub_data->u32Lat) % 1000000, sub_data->u16Acc, sub_data->u8DateAndTime);
					out_arg[0].value = pl_demo_make_value(0,path, bufsize);
					sAPI_Free(sub_data);
				}
				ret.errcode = pl_demo_post_urc(&func_lbs_get_lonlattime,TYPE_URC_NOTICE, out_arg, 1, NULL, 0);

				pl_demo_release_value(out_arg[0].value);
			}
		}
	}
	return ret;
}

void set_lbs_testtime(int lbstest)
{

    lbsdemoTestTime = lbstest;

}
int get_lbs_testtime(void)
{
    int lbstest = 0;
    lbstest = lbsdemoTestTime;
    return lbstest;
}

void LbsDemoTest(void)
{
    INT16 channel = 0;
    INT16 type;
    int ret = {0};
    int bufsize = 0;
    arg_t out_arg[1] = { 0 };
    out_arg[0].arg_name = ARGLBS_UPDATE_RESULT;
    out_arg[0].type = TYPE_RAW;

    if(SC_Lbstest2Resp_msgq == NULL)
    {
        ret = sAPI_MsgQCreate(&SC_Lbstest2Resp_msgq, "SC_LbsResp_msgq", (sizeof(SIM_MSG_T)), 4, SC_FIFO);
        if(SC_SUCCESS != ret)
        {
            LOG_ERROR("osaStatus = [%d],create msg error!",ret);
        }
    }
    else
    {
        LOG_INFO("msg has been created!"); 
    }

    type = 1;
    char path[300] = {0};
    ret = sAPI_LocGet(channel, SC_Lbstest2Resp_msgq, type);
    if(ret != SC_LBS_SUCCESS)
    {
        LOG_ERROR("get location fail!\r\n");
    }
    else
    {
        LOG_INFO("get location success!\r\n");
        SIM_MSG_T msgQ_data_recv = {0, 0, 0, NULL};                 
        SC_lbs_info_t *sub_data = NULL;
        ret = sAPI_MsgQRecv(SC_Lbstest2Resp_msgq, &msgQ_data_recv, SC_SUSPEND); 
        if(SC_SUCCESS != ret )
        {
            LOG_ERROR("osaStatus = [%d],recv msg error!",ret);
        }
        else
        {
            if(msgQ_data_recv.msg_id == SC_SRV_LBS)
            {
                sub_data = (SC_lbs_info_t *)msgQ_data_recv.arg3;
                if(sub_data->u8ErrorCode != 0)
                {
                    bufsize = snprintf(path,sizeof(path),"LBS get longitude and latitude Fail!Recv LBS info:errocode:%d\n",sub_data->u8ErrorCode);
                    out_arg[0].value = pl_demo_make_value(0,path, bufsize);
                    sAPI_Free(sub_data);
                }
                else
                {   
                    bufsize = snprintf(path,sizeof(path),"Recv LBS info:errocode %d Lng %ld.%ld Lat %ld.%ld Acc %d",sub_data->u8ErrorCode,(sub_data->u32Lng)/1000000,(sub_data->u32Lng)%1000000,(sub_data->u32Lat)/1000000,(sub_data->u32Lat)%1000000,sub_data->u16Acc);
                    LOG_INFO("%s",path);
                    out_arg[0].value = pl_demo_make_value(0,path, bufsize);
                    sAPI_Free(sub_data);
                    
                }
                pl_demo_post_urc(&func_lbs_test,TYPE_URC_NOTICE, out_arg, 1, NULL, 0);
                pl_demo_release_value(out_arg[0].value);
            }
        }
    }

    type = 2;
    memset(path,0,300);
    ret = sAPI_LocGet(channel, SC_Lbstest2Resp_msgq, type);
    if(ret != SC_LBS_SUCCESS)
    {
        LOG_ERROR("get location fail!\r\n");
    }
    else
    {
        LOG_INFO("get location success!\r\n");
        SIM_MSG_T msgQ_data_recv = {0, 0, 0, NULL};                 
        SC_lbs_info_t *sub_data = NULL;

        
        ret = sAPI_MsgQRecv(SC_Lbstest2Resp_msgq, &msgQ_data_recv, SC_SUSPEND); 
        if(SC_SUCCESS != ret)
        {
            LOG_ERROR("osaStatus = [%d],recv msg error!",ret);
        }
        else
        {
            if(msgQ_data_recv.msg_id == SC_SRV_LBS)
            {
                sub_data = (SC_lbs_info_t *)msgQ_data_recv.arg3;
                if(sub_data->u8ErrorCode != 0)
                {
                    bufsize = snprintf(path,sizeof(path),"LBS get detail address Fail!Recv LBS info:errocode:%d\n",sub_data->u8ErrorCode);
                    sAPI_Free(sub_data);
                    out_arg[0].value = pl_demo_make_value(0,path, bufsize);
                }
                else
                {
                    char tempBuf[151];
                    memset(tempBuf,0,151);
                    char *p_tmp=tempBuf;
                    UINT16 i;	
                    for(i=0; i<sub_data->u32AddrLen; i++)
                    {
                        p_tmp+= sprintf(p_tmp,"%02x", sub_data->u8LocAddress[i]); 
                        LOG_INFO("%02x",sub_data->u8LocAddress[i]);
                    }
                    bufsize = snprintf(path,sizeof(path),"Recv LBS info:errocode:%d detail_addr:%s\n",sub_data->u8ErrorCode,tempBuf);
                    sAPI_Free(sub_data);
                    out_arg[0].value = pl_demo_make_value(0,path, bufsize);
                }
				pl_demo_post_urc(&func_lbs_test,TYPE_URC_NOTICE, out_arg, 1, NULL, 0);
                pl_demo_release_value(out_arg[0].value);
            }
        }
    }

    type = 3;
    //char path[300] = {0};
    memset(path,0,300);
    ret = sAPI_LocGet(channel, SC_Lbstest2Resp_msgq, type);
    if(ret != SC_LBS_SUCCESS)
    {
        LOG_ERROR("get location fail!\r\n");
    }
    else
    {
        LOG_INFO("get location success!\r\n");
        SIM_MSG_T msgQ_data_recv = {0, 0, 0, NULL};                 
        SC_lbs_info_t *sub_data = NULL;
        ret = sAPI_MsgQRecv(SC_Lbstest2Resp_msgq, &msgQ_data_recv, SC_SUSPEND); 
        if(SC_SUCCESS != ret)
        {
            LOG_ERROR("osaStatus = [%d],recv msg error!",ret);
        }
        else
        {
            if(msgQ_data_recv.msg_id == SC_SRV_LBS)
            {
                sub_data = (SC_lbs_info_t *)msgQ_data_recv.arg3;
                if(sub_data->u8ErrorCode != 0)
                {
                    bufsize = snprintf(path,sizeof(path),"LBS get error number Fail!Recv LBS info:errocode:%d\n",sub_data->u8ErrorCode);
                    sAPI_Free(sub_data);
                    out_arg[0].value = pl_demo_make_value(0,path, bufsize);
                }
                else
                {
                    bufsize = snprintf(path,sizeof(path),"Recv LBS info:errocode:%d\n",sub_data->u8ErrorCode);
                    sAPI_Free(sub_data);

                    out_arg[0].value = pl_demo_make_value(0,path, bufsize);
                    
                }
				 pl_demo_post_urc(&func_lbs_test,TYPE_URC_NOTICE, out_arg, 1, NULL, 0);
                pl_demo_release_value(out_arg[0].value);
            }
        }
    }

    type = 4;
    //char path[300] = {0};
    memset(path,0,300);
    ret = sAPI_LocGet(channel, SC_Lbstest2Resp_msgq, type);
    if(ret != SC_LBS_SUCCESS)
    {
        LOG_ERROR("get location fail!\r\n");
    }
    else
    {
        LOG_INFO("get location success!\r\n");
        SIM_MSG_T msgQ_data_recv = {0, 0, 0, NULL};                 
        SC_lbs_info_t *sub_data = NULL;
        ret = sAPI_MsgQRecv(SC_Lbstest2Resp_msgq, &msgQ_data_recv, SC_SUSPEND); 
        if(SC_SUCCESS != ret)
        {
            LOG_ERROR("osaStatus = [%d],recv msg error!",ret);

        }
        else
        {
            if(msgQ_data_recv.msg_id == SC_SRV_LBS)
            {
                sub_data = (SC_lbs_info_t *)msgQ_data_recv.arg3;
                if(sub_data->u8ErrorCode != 0)
                {
                    bufsize = snprintf(path,sizeof(path),"LBS get longitude latitude and date time Fail!Recv LBS info:errocode:%d\n",sub_data->u8ErrorCode);
                    LOG_INFO("%s",path);
					out_arg[0].value = pl_demo_make_value(0,path, bufsize);
                    sAPI_Free(sub_data);
                }
                else
                {   
                    bufsize = snprintf(path,sizeof(path),"Recv LBS info:errocode %d Lng:%ld.%ld Lat:%ld.%ld Acc:%d DateAndTime:%s\n",sub_data->u8ErrorCode,(sub_data->u32Lng)/1000000,(sub_data->u32Lng)%1000000,(sub_data->u32Lat)/1000000,(sub_data->u32Lat)%1000000, sub_data->u16Acc,sub_data->u8DateAndTime);
					out_arg[0].value = pl_demo_make_value(0,path, bufsize);
					sAPI_Free(sub_data);
                }
                 pl_demo_post_urc(&func_lbs_test,TYPE_URC_NOTICE, out_arg, 1, NULL, 0);
                pl_demo_release_value(out_arg[0].value);
            }
        }
    }

    
}



static ret_msg_t func_lbs_test_proc(op_t op, arg_t *argv, int argn)
{
	ret_msg_t ret = {0};
	int testvlue = 0;
	static UINT16 isCreated = 0;
	pl_demo_get_value(&testvlue, argv, argn, "Testtime");
	lbsdemoCurrentTime = 1;
	set_lbs_testtime(testvlue);
	
	
	LOG_INFO("lbsTestTime:%d,lbsCurrentTime:%d", testvlue, lbsdemoCurrentTime);
	
	if(1 == isCreated)
	{
		LOG_INFO("init successful!");
		sAPI_SemaphoreRelease(lbsDemo2testSema);
	}
	else
	{
		ret.errcode = sAPI_TaskCreate(&lbsDemo2TestTask,lbsDemo2TestTaskStack,SAL_8K,150,"lbsDemoTestProcesser",sTask_lbsTestProcesser,NULL);
		if(SC_SUCCESS != ret.errcode)
		{
			ret.errcode = ERRCODE_DEFAULT;
			ret.msg = "task create fail";
			LOG_ERROR("task create fail");
			goto EXIT;
		}
	}
	ret.errcode = sAPI_SemaphoreCreate(&lbsDemo2testSema, 1, SC_FIFO);
	if(SC_SUCCESS != ret.errcode)
	{
		ret.errcode = ERRCODE_DEFAULT;
		ret.msg = "flag create fail";
		LOG_ERROR("flag create fail");
		goto EXIT;
	}

	isCreated = 1;
	ret.msg = "lbs test start";
EXIT:

	return ret;

}
void sTask_lbsTestProcesser(void * arg)
{
    SC_STATUS status = SC_SUCCESS;
    char path[256] = {0};
	int bufsize = 0;
	arg_t out_arg[1] = { 0 };
	out_arg[0].arg_name = ARGLBS_UPDATE_RESULT;
	out_arg[0].type = TYPE_RAW;
	int lbstestvlue = 0;
	LOG_INFO("init sTask_lbsDemoTestProcesser! %d",get_lbs_testtime());
	
    while(1)
		
    {
		lbstestvlue = get_lbs_testtime();
        status = sAPI_SemaphoreAcquire(lbsDemo2testSema, SC_SUSPEND);
        if(SC_SUCCESS != status)
        {
            LOG_ERROR("status = [%d], accquire seamphore error!",status);
            continue;
        }


        memset(path, 0, sizeof(path));
        bufsize = snprintf(path, sizeof(path), "current time = %d, testTime = %d",lbsdemoCurrentTime,lbstestvlue);


        out_arg[0].value = pl_demo_make_value(0,path, bufsize);
        pl_demo_post_urc(&func_lbs_test,TYPE_URC_NOTICE, out_arg, 1, NULL, 0);
        pl_demo_release_value(out_arg[0].value);


        if(lbstestvlue > 0)
        {
        	LbsDemoTest();
            lbstestvlue--;
			set_lbs_testtime(lbstestvlue);
            lbsdemoCurrentTime++;
            if(0 == lbstestvlue)
            {
               LOG_INFO("test complete!");
               continue;
            }
            sAPI_SemaphoreRelease(lbsDemo2testSema);
        }
         
    }

}

