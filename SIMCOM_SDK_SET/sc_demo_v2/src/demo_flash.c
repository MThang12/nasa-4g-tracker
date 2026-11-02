/*
 * @Author: yangkui.zhang
 * @Date: 2023-12-23 15:27:38
 * @LastEditTime: 2024-01-09 17:20:12
 * @LastEditors: yangkui.zhang
 * @Description: 
 * @FilePath: \ASR160X\simcom\userspace\sc_demo_v2\src\demo_flash.c
 * no error,no warning
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "simcom_demo_init.h"
#include "demo_flash.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "simcom_os.h"
#include "sc_flash.h"

#define LOG_ERROR(...) sal_log_error("[DEMO-FLASH] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-FLASH] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-FLASH] " __VA_ARGS__)

ret_msg_t g_flashwrite_result;
pub_cache_p g_flashwrite_cache;
sTaskRef g_flashwrite_task_ref;
int flashwrite_size;
int g_flashwrite_flag = 0;

static void set_flash_write_task(void *args);
static int set_flash_write_need_size(void);
static ret_msg_t set_flash_write_init(void);
static ret_msg_t set_flash_write_uninit(void);
static ret_msg_t set_flash_erase_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_flash_write_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_flash_write_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms));
static ret_msg_t set_flash_read_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_flash_read_app_cmd_proc(op_t op, arg_t *argv, int argn);

arg_t flashwrite_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGFLASHWRITE_SIZE,
        .msg = "Integer type, indicates the length of sending flash data"
    }
};

const menu_list_t func_setflasherase =
{
    .menu_name = "Set flash erase",
    .menu_type = TYPE_FUNC,
    .parent = &flash_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_flash_erase_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_setflashwrite =
{
    .menu_name = "Set flash write",
    .menu_type = TYPE_FUNC,
    .parent = &flash_menu,
    .function = {
        .argv = flashwrite_args,
        .argn = 1,
        .RAWSizeLink = ARGFLASHWRITE_SIZE,
        .methods.__init = set_flash_write_init,
        .methods.__uninit = set_flash_write_uninit,
        .methods.cmd_handler = set_flash_write_cmd_proc,
        .methods.raw_handler = set_flash_write_raw_proc,
        .methods.needRAWData = set_flash_write_need_size,
    },
};


const menu_list_t func_setflashread =
{
    .menu_name = "Set flash read",
    .menu_type = TYPE_FUNC,
    .parent = &flash_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_flash_read_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_setflashreadapp =
{
    .menu_name = "Set flash read app",
    .menu_type = TYPE_FUNC,
    .parent = &flash_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = set_flash_read_app_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t *flash_menu_sub[] =
{
    &func_setflasherase,
    &func_setflashwrite,
    &func_setflashread,
    &func_setflashreadapp,

    NULL  // MUST end by NULL
};

const menu_list_t flash_menu =
{
    .menu_name = "FLASH",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = flash_menu_sub,
    .parent = &root_menu,
};

static ret_msg_t set_flash_write_uninit(void)
{
    ret_msg_t ret = {0};

    pub_bzero(&g_flashwrite_result, sizeof(ret_msg_t));
    flashwrite_size = 0;
    //pub_cache_set_status(g_flashwrite_cache, CACHE_STOPED);

    sAPI_TaskSleep(5);  // wait task exit.

    if (g_flashwrite_task_ref)
    {
        ret.errcode = sAPI_TaskDelete(g_flashwrite_task_ref);
        if (ret.errcode != 0)
        {
            LOG_ERROR("task delete fail, errcode=%d", ret.errcode);
            goto DONE;
        }
        else
        {
            g_flashwrite_task_ref = NULL;
        }
    }

    if(g_flashwrite_flag)
    {
        g_flashwrite_flag = 0;
    }


    // if (g_flashwrite_cache)
    // {
    //     ret.errcode = pub_cache_delete(&g_flashwrite_cache);
    //     if (ret.errcode != 0)
    //     {
    //         LOG_ERROR("cache delete fail");
    //         goto DONE;
    //     }
    // }

DONE:
    return ret;
}

static ret_msg_t set_flash_write_init(void)
{
    ret_msg_t ret = {0};

    ret = set_flash_write_uninit();
    if (0 != ret.errcode)
    {
        goto DONE;
    }

    g_flashwrite_cache = pub_cache_create(SAL_4K, CACHE_TYPE_BUFFER, CACHE_SYNC_R);
    if (NULL == g_flashwrite_cache)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("cache create fail");
        goto DONE;
    }

DONE:
    return ret;
}

static int set_flash_write_need_size(void)
{
    return flashwrite_size;
}

static ret_msg_t set_flash_write_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int sendsize = 0;

    pl_demo_get_value(&sendsize, argv, argn, ARGFLASHWRITE_SIZE);
    if (sendsize <= 0)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    flashwrite_size = sendsize;

    if (sendsize > 0)
    {
        ret.errcode = sAPI_TaskCreate(&g_flashwrite_task_ref,
                                      NULL,
                                      SAL_2K,
                                      150,
                                      "FLASHWRITE",
                                      set_flash_write_task,
                                      NULL);
        if (ret.errcode != SC_SUCCESS)
        {
            ret.msg = "task creat fail";
        }
    }

    return ret;
}

static ret_msg_t set_flash_write_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms))
{
    ret_msg_t ret = {0};
    int read_size = 0;
    int process_size = 0;
    int remain_size = size;
    char buf[SAL_256] = {0};

    if (flashwrite_size != size)
    {
        LOG_ERROR("raw data size invalid");
        LOG_ERROR("size:%d, need:%d", size, flashwrite_size);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "raw data size invalid";
        return ret;
    }

    while (remain_size > 0)
    {
        LOG_INFO("remain_size[%d]",remain_size);
        read_size = remain_size <= SAL_256 ? remain_size : SAL_256;
        read_size = raw_read(buf, read_size, 10000);
        LOG_INFO("readsize[%d]",read_size);
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

        process_size = pub_cache_write(g_flashwrite_cache, buf, read_size);
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
    /* wait send data to server. */
    pub_cache_set_status(g_flashwrite_cache, CACHE_STOPED);
    while (g_flashwrite_flag != 1) sAPI_TaskSleep(20);

    if (ret.errcode != 0) return ret;
    else return g_flashwrite_result;
}

static void set_flash_write_task(void *args)
{
    ret_msg_t ret = {0};
    char *buf = NULL;
    int read_size = 0;


    buf = sal_malloc(SAL_4K);
    if (!buf)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "malloc fail";
        goto EXIT;
    }

    while (flashwrite_size > 0 && g_flashwrite_cache)
    {
        read_size = flashwrite_size <= SAL_4K ? flashwrite_size : SAL_4K;

        read_size = pub_cache_read(g_flashwrite_cache, buf, read_size);
        if (read_size <= 0)
        {
            LOG_INFO("cache read fail");
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "cache read fail";
            goto EXIT;
        }


        if (sAPI_WriteFlash(SECTOR_1 , (char *)buf,flashwrite_size) == 1)
        {
            LOG_ERROR("sAPI_WriteFlash fail!!!");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "sAPI_WriteFlash fail!!!";
            goto EXIT;
        }

        pub_cache_set_status(g_flashwrite_cache, CACHE_STOPED);

        sAPI_TaskSleep(5);  // wait task exit.
        if (g_flashwrite_cache)
        {
            ret.errcode = pub_cache_delete(&g_flashwrite_cache);
            if (ret.errcode != 0)
            {
                LOG_ERROR("cache delete fail");
                goto EXIT;
            }
        }

    }

EXIT:
    sal_free(buf);
    g_flashwrite_flag = 1;
    g_flashwrite_result = ret;
}

static ret_msg_t set_flash_erase_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    if (sAPI_EraseFlashSector(SECTOR_1, ERASE_SIZE) != 0)
    {
        LOG_ERROR("erase fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_EraseFlashSector fail!!!";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t set_flash_read_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    arg_t out_arg[1] = {0};
    char readBuff[8] = {0};
    char Buff[128] = {0};
    char temp[32] = {0};

    memset(Buff, 0, sizeof(Buff));
    memset(temp, 0, sizeof(temp));
    snprintf(Buff, sizeof(Buff),"Buff is:");

    if (sAPI_ReadFlash(SECTOR_1 , (char *)readBuff,sizeof(readBuff)) == 1)
    {
        LOG_ERROR("sAPI_ReadFlash fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_ReadFlash fail!!!";
        goto EXIT;
    }

    for(int i=0;i<8;i++)
    {
        snprintf(temp, sizeof(temp),"[%d]=%02X ",i,readBuff[i]);
        strcat(Buff, temp);
    }

    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARGFLASH_READ;
    out_arg[0].value = pl_demo_make_value(0, Buff, strlen(Buff));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

    pl_demo_release_value(out_arg[0].value);

    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
        goto EXIT;
    }
EXIT:
    return ret;
}

static ret_msg_t set_flash_read_app_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    arg_t out_arg[1] = {0};
    char readBuff[8] = {0};
    char appBuff[128] = {0};
    char temp[32] = {0};

    memset(appBuff, 0, sizeof(appBuff));
    memset(temp, 0, sizeof(temp));
    snprintf(appBuff, sizeof(appBuff),"appBuff is:");

    if (sAPI_ReadFlashAppPartition(SECTOR_1,(char *)readBuff,sizeof(readBuff)) == 1)
    {
        LOG_ERROR("API_ReadFlashAppPartitionse fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_ReadFlashAppPartition fail!!!";
        goto EXIT;
    }

    for(int i=0;i<8;i++)
    {
        snprintf(temp, sizeof(temp),"[%d]=%02X ",i,readBuff[i]);
        strcat(appBuff, temp);
    }

    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARGFLASH_READAPP;
    out_arg[0].value = pl_demo_make_value(0, appBuff, strlen(appBuff));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

    pl_demo_release_value(out_arg[0].value);

    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
        goto EXIT;
    }
EXIT:
    return ret;
}

