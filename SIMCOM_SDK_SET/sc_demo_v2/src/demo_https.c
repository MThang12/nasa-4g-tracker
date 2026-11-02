#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "simcom_os.h"
#include "simcom_https.h"
#include "simcom_common.h"
#include "demo_https.h"

#define LOG_ERROR(...) sal_log_error("[DEMO-HTTPS] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-HTTPS] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-HTTPS] " __VA_ARGS__)


static ret_msg_t httpsdata_init(void);
static ret_msg_t httpsdata_uninit(void);
static ret_msg_t httpsdata_raw(int size , int (*raw_read)(char *buf , unsigned int buf_size, int timeout_ms));
static int httpsdata_need_size(void);

static ret_msg_t httpsinit_cmd(op_t op,arg_t *argv, int argn);
static ret_msg_t httpspara_cmd(op_t op,arg_t *argv, int argn);
static ret_msg_t httpsdata_cmd(op_t op,arg_t *argv, int argn);
static ret_msg_t httpsaction_cmd(op_t op,arg_t *argv, int argn);
static ret_msg_t httpshead_cmd(op_t op,arg_t *argv, int argn);
static ret_msg_t httpsread_cmd(op_t op,arg_t *argv, int argn);
static ret_msg_t httpspostfile_cmd(op_t op,arg_t *argv, int argn);
static ret_msg_t httpsterm_cmd(op_t op,arg_t *argv, int argn);


const menu_list_t func_httpsinit=
{
    .menu_name = "Https session initialize",
    .menu_type = TYPE_FUNC,
    .parent = &https_menu,
    .function = {
        .argv = NULL ,
        .argn = 0 ,
        .methods.cmd_handler = httpsinit_cmd
    }

};

const value_t para_type_head[]=
{
    {
        .bufp = "URL",
        .size =3,
    },
    {
        .bufp = "CONNECTTO",
        .size = 9,
    },
    {
        .bufp = "CONTENT",
        .size =7,
    },
    {
        .bufp = "USERDATA",
        .size = 8,
    },
    {
        .bufp = "ACCEPT",
        .size = 6,
    },
    {
        .bufp = "SSLCFG",
        .size = 6,
    },
    {
        .bufp = "RECVTO",
        .size = 6,
    },
    {
        .bufp = "READMODE",
        .size = 8,
    },
};

const value_list_t para_type_range=
{
    .isrange =false,
    .list_head = para_type_head,
    .count = 8,
};

arg_t httpspara_args[2]=
{
    {
        .type = TYPE_STR,
        .arg_name = ARGHTTPS_PARATYPE,
        .value_range =&para_type_range,
        
        //.msg = "input type:(URL,CONNECTTO,CONTENT,USERDATA,ACCEPT,SSLCFG,RECVTO,READMODE)"

    },
    {
        .type = TYPE_STR,
        .arg_name = ARGHTTPS_PARAVALUE
    }
};


const menu_list_t func_httpspara=
{
    .menu_name = "Set parameters",
    .menu_type = TYPE_FUNC,
    .parent = &https_menu,
    .function = {
        .argv = httpspara_args ,
        .argn = sizeof(httpspara_args)/sizeof(arg_t),
        .methods.cmd_handler = httpspara_cmd
    }

};


arg_t httpsdata_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGHTTPS_DATALEN
    }
};

const menu_list_t func_httpsdata=
{
    .menu_name = "Set post data",
    .menu_type = TYPE_FUNC,
    .parent = &https_menu,
    .function = {
        .argv = httpsdata_args,
        .argn = sizeof(httpsdata_args)/sizeof(arg_t),
        .RAWSizeLink = ARGHTTPS_DATALEN,
        .methods.__init = httpsdata_init,
        .methods.__uninit = httpsdata_uninit,
        .methods.cmd_handler = httpsdata_cmd,
        .methods.raw_handler = httpsdata_raw,
        .methods.needRAWData = httpsdata_need_size
    }

};


const value_t action_method_head[]=
{
    {
        .bufp = "0:GET",
        .size = 5,
    },
    {
        .bufp ="1:POST",
        .size = 6,
    },
    {
        .bufp = "2:HEAD",
        .size = 6,
    },
    {
        .bufp = "3:DELETE",
        .size = 8,
    },
    {
        .bufp ="4:PUT",
        .size = 4,
    },
};
const value_list_t method_range =
{
    .isrange = false,
    .list_head = action_method_head,
    .count = 5,
};

arg_t httpsaction_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGHTTPS_METHOD,
        .value_range = &method_range
    }
};

const menu_list_t func_httpsaction=
{
    .menu_name = "Send http request",
    .menu_type = TYPE_FUNC,
    .parent = &https_menu,
    .function = {
        .argv = httpsaction_args ,
        .argn = sizeof(httpsaction_args)/sizeof(arg_t),
        .methods.cmd_handler = httpsaction_cmd
    }

};
const menu_list_t func_httpshead=
{
    .menu_name = "read response header",
    .menu_type = TYPE_FUNC,
    .parent = &https_menu,
    .function = {
        .argv = NULL ,
        .argn = 0 ,
        .methods.cmd_handler = httpshead_cmd
    }

};
const value_t readtype_range_head[]=
{
   {
        .bufp = "0 get data len",
        .size = 14,
   },
   {
        .bufp = "1 read data",
        .size =11,
   },
};

const value_list_t readtype_range =
{
    .isrange = false,
    .list_head =readtype_range_head,
    .count = 2, 
};

arg_t read_args[3] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGHTTPS_READTYPE,
        .value_range = &readtype_range
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGHTTPS_READOFFSET,
        .msg = "read data offset"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGHTTPS_READSIZE,
        .msg = "noce read size"
    }
};


const menu_list_t func_httpsread=
{
    .menu_name = "read response body",
    .menu_type = TYPE_FUNC,
    .parent = &https_menu,
    .function = {
        .argv = read_args ,
        .argn = sizeof(read_args)/sizeof(arg_t),
        .methods.cmd_handler = httpsread_cmd
    }

};
const value_t dir_range_head[]=
{
    {
        .bufp = "1 C:",
        .size = 4,
    },
    {
        .bufp = "2 D:",
        .size = 4,
    },
};
const value_list_t dir_range =
{
    .isrange = false,
    .list_head = dir_range_head,
    .count =2
};

arg_t postfile_args[2]=
{
    {
        .type = TYPE_STR,
        .arg_name = ARGHTTPS_PATHNAME,
        .msg = "file path,e.g: C:/test.txt"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGHTTPS_DIR,
        .value_range = &dir_range
    }
};

const menu_list_t func_httpspostfile=
{
    .menu_name = "httpspostfile",
    .menu_type = TYPE_FUNC,
    .parent = &https_menu,
    .function = {
        .argv = postfile_args ,
        .argn = sizeof(postfile_args)/sizeof(arg_t) ,
        .methods.cmd_handler = httpspostfile_cmd
    }

};


const menu_list_t func_httpsterm=
{
    .menu_name = "httpsterm",
    .menu_type = TYPE_FUNC,
    .parent = &https_menu,
    .function = {
        .argv = NULL ,
        .argn = 0 ,
        .methods.cmd_handler = httpsterm_cmd
    }

};

const menu_list_t *https_menu_sub[] =
{
    &func_httpsinit,
    &func_httpspara,
    &func_httpsdata,
    &func_httpsaction,
    &func_httpshead,
    &func_httpsread,
    &func_httpspostfile,
    &func_httpsterm,
    NULL  // MUST end by NULL
};

const menu_list_t https_menu =
{
    .menu_name = "HTTPS ",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = https_menu_sub,
    .parent = &root_menu,
};

static sMsgQRef httpsUIResp_msgq = NULL;


static ret_msg_t httpsinit_cmd(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_HTTP_RETURNCODE api_ret = SC_HTTPS_SUCCESS;
    if (NULL == httpsUIResp_msgq)
    {
        SC_STATUS status;
        status = sAPI_MsgQCreate(&httpsUIResp_msgq, "httpsUIResp_msgq", sizeof(SIM_MSG_T), 8, SC_FIFO);
        if (SC_SUCCESS != status)
        {
            LOG_ERROR("msgQ create fail");
            ret.errcode = -1;
            ret.msg ="https init fail";
            return ret;

        }
    }

    api_ret = sAPI_HttpInit(1, httpsUIResp_msgq);
    if (api_ret == SC_HTTPS_SUCCESS)
    {
        LOG_INFO("\r\nhttps init success\r\n");
        ret.errcode = 0;
        ret.msg ="https init success";
    }
    else
    {
        LOG_ERROR("\r\nhttps init fail\r\n");
        ret.errcode = -1;
        ret.msg ="https init fail";
    }

    return ret;
}


static ret_msg_t httpspara_cmd(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret={0};
    SC_HTTP_RETURNCODE api_ret = SC_HTTPS_SUCCESS;
    const char * paraType=NULL;
    const char * paraValue=NULL;
    pl_demo_get_data(&paraType, argv, argn, ARGHTTPS_PARATYPE);
    pl_demo_get_data(&paraValue, argv, argn, ARGHTTPS_PARAVALUE);

    if(!paraType || !paraValue)
    {
        LOG_ERROR("para invalid");
        ret.errcode = -1;
        ret.msg = "para invalid";
        return ret;
    }
    char *ptr1 = sal_malloc(strlen(paraType)+1);
    if(ptr1 == NULL )
    {
        LOG_ERROR("malloc error");
        ret.errcode = -1;
        ret.msg = "malloc error";
        return ret;
    }
    char *ptr2 = sal_malloc(strlen(paraValue)+1);
     if(ptr2 == NULL )
    {
        sal_free(ptr1);
        LOG_ERROR("malloc error");
        ret.errcode = -1;
        ret.msg = "malloc error";
        return ret;
    }
    strncpy(ptr1 , paraType, strlen(paraType));
    strncpy(ptr2, paraValue, strlen(paraValue));
    api_ret = sAPI_HttpPara(ptr1, ptr2);
    if ( api_ret == SC_HTTPS_SUCCESS)
    {
        LOG_INFO("https set parameters success");
        ret.msg = "https set parameters success";
    }
    else
    {
        ret.errcode = -1;
        ret.msg = "https set parameters fail";
        LOG_ERROR("https set parameters fail");
    }
    sal_free(ptr1);
    sal_free(ptr2);
    return ret;
}

int g_httpsdata_size;
sTaskRef g_httpsdata_task_ref;
pub_cache_p g_httpsdata_cache;
ret_msg_t g_httpsdata_result;
char *data_cache_buf=NULL;

static int httpsdata_need_size(void)
{
    return g_httpsdata_size;
}
static void httpsdata_task(void *args);

static ret_msg_t httpsdata_uninit(void)
{
    ret_msg_t ret = {0};
    pub_bzero(&g_httpsdata_result, sizeof(ret_msg_t));
    g_httpsdata_size = 0;
    pub_cache_set_status(g_httpsdata_cache, CACHE_STOPED);

    sAPI_TaskSleep(5);  // wait task exit.

    if (g_httpsdata_task_ref)
    {
        ret.errcode = sAPI_TaskDelete(g_httpsdata_task_ref);
        if (ret.errcode != 0)
        {
            LOG_ERROR("task delete fail, errcode=%d", ret.errcode);
            goto DONE;
        }
        else
        {
            g_httpsdata_task_ref = NULL;
        }
    }
    if (g_httpsdata_cache)
    {
        ret.errcode = pub_cache_delete(&g_httpsdata_cache);
        if (ret.errcode != 0)
        {
            LOG_ERROR("cache delete fail");
            goto DONE;
        }
    }

DONE:
    return ret;

}

static ret_msg_t httpsdata_init(void)
{
    ret_msg_t ret = { 0 };
    ret = httpsdata_uninit();
    if (0 != ret.errcode)
    {
        goto DONE;
    }
    g_httpsdata_cache = pub_cache_create(SAL_20K, CACHE_TYPE_BUFFER, CACHE_SYNC_R);
    if (NULL == g_httpsdata_cache)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("cache create fail");
        goto DONE;
    }

DONE:
    return ret;
}

static ret_msg_t httpsdata_cmd(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret={0};
    int dataLen  = 0;

    pl_demo_get_value(&dataLen,argv,argn,ARGHTTPS_DATALEN);
    if(dataLen < 0)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    data_cache_buf = sal_malloc( dataLen);
    if(data_cache_buf == NULL)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "malloc error";
        return ret;
    }
    g_httpsdata_size = dataLen;
    if( dataLen > 0)
    {
        ret.errcode = sAPI_TaskCreate(&g_httpsdata_task_ref,
                                      NULL,
                                      SAL_2K,
                                      150,
                                      "HTTPSDATA",
                                      httpsdata_task,
                                      NULL);
        if (ret.errcode != SC_SUCCESS)
        {
            ret.msg = "task creat fail";
        }
    }
    return ret;
}

static ret_msg_t httpsdata_raw(int size , int (*raw_read)(char *buf , unsigned int buf_size, int timeout_ms))
{
    ret_msg_t ret = {0};
    int read_size = 0;
    int process_size = 0;
    int remain_size = size;
    char buf[SAL_256] = {0};

    if(g_httpsdata_size != size)
    {
        LOG_ERROR("raw data size invalid");
        LOG_ERROR("size:%d, need:%d", size, g_httpsdata_size);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "raw data size invalid";
        return ret;
    }

    while (remain_size > 0 && data_cache_buf)
    {
        read_size = remain_size <= SAL_256 ? remain_size : SAL_256;
        read_size = raw_read(buf, read_size, 10000);
        if (read_size < 0)
        {
            LOG_ERROR("raw data read fail");
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "raw data read fail";
            goto EXIT;
        }
        else if (0 == read_size)
        {
            LOG_ERROR("raw data read timeout");
            ret.errcode = ERRCODE_RAW_RECV_TIMEOUT;
            ret.msg = "raw data read timeout";
            goto EXIT;
        }

        process_size = pub_cache_write(g_httpsdata_cache, buf, read_size);
        if (process_size < read_size)
        {
            LOG_ERROR("raw data cache fail");
            LOG_ERROR("read size:%d, cache size:%d", read_size, process_size);
            ret.errcode = process_size < 0 ? process_size : ERRCODE_DEFAULT;
            ret.msg = "raw data cache fail";
            goto EXIT;
        }

        remain_size -= read_size;
    }
EXIT:
    /* wait file write to fs. */
    pub_cache_set_status(g_httpsdata_cache, CACHE_STOPED);
    while (NULL != data_cache_buf) sAPI_TaskSleep(20);

    if (ret.errcode != 0) return ret;
    else return g_httpsdata_result;
}

static void httpsdata_task(void *args)
{
     SC_HTTP_RETURNCODE api_ret = SC_HTTPS_SUCCESS;
     ret_msg_t ret = {0};
    char *buf = NULL;
    int read_size = 0 ;
    int cache_size = 0;

    buf = sal_malloc(SAL_4K);
    if (!buf)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "malloc fail";
        goto EXIT;
    }
    while ((g_httpsdata_size > cache_size)  && data_cache_buf && g_httpsdata_cache)
    {
        read_size = g_httpsdata_size <= SAL_4K ? g_httpsdata_size : SAL_4K;

        read_size = pub_cache_read(g_httpsdata_cache, buf, read_size);
        if (read_size <= 0)
        {
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "cache read fail";
            goto EXIT;
        }

        strncpy(data_cache_buf+cache_size , buf , read_size);

        cache_size += read_size;
    }

    api_ret = sAPI_HttpData(data_cache_buf,g_httpsdata_size);
    if(api_ret != SC_HTTPS_SUCCESS)
    {
        ret.errcode = -1;
        ret.msg = "set post data error";
    }
    else
    {
        ret.msg = "set post data success";
    }
EXIT:
    sal_free(buf);
    sal_free(data_cache_buf);
    data_cache_buf = NULL;
    g_httpsdata_result = ret;

}

static void show_httpaction_urc(ret_msg_t *ret)
{
    char buf[100]={0};
    SIM_MSG_T msgQ_data_recv = {SIM_MSG_INIT, 0, -1, NULL};
    sAPI_MsgQRecv(httpsUIResp_msgq, &msgQ_data_recv, SC_SUSPEND);
    arg_t out_arg[1] = {0};

    SChttpApiTrans *sub_data = (SChttpApiTrans *)(msgQ_data_recv.arg3);

    int bufsize=snprintf(buf,sizeof(buf)-1,"status-code=%ld content-length=%ld",sub_data->status_code,sub_data->action_content_len);
    out_arg[0].arg_name = pre_arg_name[ARG_ID_DATA];
    out_arg[0].type = TYPE_RAW;
    out_arg[0].value = pl_demo_make_value(0, buf, bufsize);
    ret->errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_post_raw(buf, bufsize);
    LOG_INFO("response info [%s]",buf);
    pl_demo_release_value(out_arg[0].value);
    if(sub_data)
    {
        sAPI_Free(sub_data->data);
    }
    sAPI_Free(sub_data);
}
static ret_msg_t httpsaction_cmd(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret={0};
    int method = -1;
    SC_HTTP_RETURNCODE api_ret = SC_HTTPS_SUCCESS;
    const char *str_action_method=NULL;
    pl_demo_get_data(&str_action_method, argv, argn, ARGHTTPS_METHOD);
    if(NULL==str_action_method)
    {
        ret.errcode = -1;
        ret.msg="para invalid";
        LOG_ERROR("para invalid");
        return ret;
    }
     method = str_action_method[0]-'0';
    api_ret = sAPI_HttpAction(method);
    if(api_ret == SC_HTTPS_SUCCESS )
    {
        ret.errcode=0;
        show_httpaction_urc(&ret);
        LOG_INFO("https action success");
        ret.msg= "https action success";
    }
    else
    {
        ret.errcode = -1;
        ret.msg="https action fail";
        LOG_ERROR("https action fail");
    }
    return ret;
}
static void show_httphead_urc( ret_msg_t *ret,int type)
{
    SIM_MSG_T msgQ_data_recv = {SIM_MSG_INIT, 0, -1, NULL};
    SChttpApiTrans *sub_data = NULL;
    arg_t out_arg[1] = {0};

    sAPI_MsgQRecv(httpsUIResp_msgq, &msgQ_data_recv, SC_SUSPEND);

    sub_data = (SChttpApiTrans *)(msgQ_data_recv.arg3);

    if (1 == type)
    {
        if (sub_data->data != NULL && sub_data->dataLen > 0)
        {

            out_arg[0].arg_name = pre_arg_name[ARG_ID_DATA];
            out_arg[0].type = TYPE_RAW;
            out_arg[0].value = pl_demo_make_value(0, (char *)sub_data->data, sub_data->dataLen);
            ret->errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
            LOG_INFO("\rsub_data->dataLen[%d]\r\n", sub_data->dataLen);
            pl_demo_post_raw((char *)sub_data->data, sub_data->dataLen);
            pl_demo_release_value(out_arg[0].value);
        }
    }
    else if (0 == type)
    {
        char rspBuf[50] = { 0 };
        snprintf(rspBuf, sizeof(rspBuf) - 1, "left data length=%ld\r\n", sub_data->action_content_len);
        out_arg[0].arg_name = pre_arg_name[ARG_ID_DATA];
        out_arg[0].type = TYPE_RAW;
        out_arg[0].value = pl_demo_make_value(0, rspBuf, strlen(rspBuf));
        ret->errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
        pl_demo_post_raw(rspBuf, strlen(rspBuf));
        pl_demo_release_value(out_arg[0].value);
    }

    if(sub_data)
    {
        sAPI_Free(sub_data->data);
    }
    sAPI_Free(sub_data);
}
static ret_msg_t httpshead_cmd(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret={0};
    SC_HTTP_RETURNCODE api_ret = SC_HTTPS_SUCCESS;
    api_ret = sAPI_HttpHead();
    if(api_ret == SC_HTTPS_SUCCESS)
    {
        show_httphead_urc(&ret,1);
        LOG_INFO("https read head success");
        ret.msg = "https read head success";
    }
    else
    {
        LOG_ERROR("https read head fail");
        ret.errcode = -1;
        ret.msg = "https read head fail";
    }
    return ret;
}
static ret_msg_t httpsread_cmd(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret={0};
    SC_HTTP_RETURNCODE api_ret = SC_HTTPS_SUCCESS;
    int type = -1;
    int offset = -1;
    int size = -1;
    const char *type_str=NULL;
    pl_demo_get_data(&type_str,argv,argn,ARGHTTPS_READTYPE);
    pl_demo_get_value(&offset,argv,argn,ARGHTTPS_READOFFSET);
    pl_demo_get_value(&size,argv,argn,ARGHTTPS_READSIZE);
    if(NULL==type_str)
    {
        LOG_ERROR("para invalid");
        ret.errcode = -1;
        ret.msg = "para invalid";
        return ret;
    }
    type = type_str[0]-'0';
    api_ret = sAPI_HttpRead(type,offset,size);
    if(api_ret == SC_HTTPS_SUCCESS)
    {
        show_httphead_urc(&ret,type);
        LOG_INFO("https read body success");
        ret.msg ="https read body success";
    }
    else
    {
        LOG_ERROR("https read body fail");
        ret.errcode = -1;
        ret.msg = "https read body fail";
    }
    return ret;
}
static ret_msg_t httpspostfile_cmd(op_t op,arg_t *argv, int argn)
{
    ret_msg_t ret={0};
    SC_HTTP_RETURNCODE api_ret = SC_HTTPS_SUCCESS;
    const char *filename = NULL;
    int dir = 0;
    const char *dir_str=NULL;
    pl_demo_get_data(&filename,argv,argn,ARGHTTPS_PATHNAME);
    pl_demo_get_data(&dir_str,argv,argn,ARGHTTPS_DIR);
    if(NULL == dir_str)
    {
        LOG_ERROR("para invalid");
        ret.errcode = -1;
        ret.msg = "para invalid";
        return ret;
    }
    dir = dir_str[0]-'0';

    if(filename == NULL || dir < 0)
    {
        LOG_ERROR("param invalid");
        ret.errcode = -1;
        ret.msg = "param invalid";
        return ret;
    }
    char *ptr =NULL;
    ptr = sal_malloc(strlen(filename)+1);
    if(ptr == NULL)
    {
        LOG_ERROR("malloc error");
        ret.errcode = -1;
        ret.msg = "malloc error";
        return ret;
    }
    strncpy(ptr , filename, strlen(filename));
    api_ret = sAPI_HttpPostfile(ptr,dir);
    if(api_ret != SC_HTTPS_SUCCESS)
    {
        LOG_ERROR("postfile error");
        ret.errcode = -1;
        ret.msg = "httpspostfile error";
    }
    else
    {
        ret.msg = "httpspostfile success";
        show_httpaction_urc(&ret);
    }
    sal_free(ptr);
    return ret;
}
static ret_msg_t httpsterm_cmd(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = { 0 };
    SC_HTTP_RETURNCODE api_ret = SC_HTTPS_SUCCESS;
    api_ret = sAPI_HttpTerm();
    if (api_ret == SC_HTTPS_SUCCESS)
    {
        LOG_ERROR("\r\nhttp term success\r\n");
    }
    else
    {
        LOG_INFO("\r\nhttp term fail\r\n");
    }

    return ret;
}
