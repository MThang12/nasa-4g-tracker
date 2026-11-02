/*
 * @Author: xiaobing.fang
 * @Date: 2023-12-18 14.17.42
 * @Last Modified by: xiaobing.fang
 * @Last Modified time: 2023-12-18 14.17.42
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "simcom_os.h"
#include "sc_spi.h"

#define LOG_ERROR(...) sal_log_error("[DEMO-NorFlash] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-NorFlash] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-NorFlash] " __VA_ARGS__)


#define NorFlash_addr "Nor FLash Addr"
#define Input_data_size "Input data size"
#define NorFlash_init "Nor Flash init"
#define NorFlash_id "Get NorFlash id"
#define NorFlash_write "Nor Flash Write"
#define NorFlash_read "Nor Flash Read"
#define NorFlash_earse "Nor Flash Earse"

pub_cache_p g_norflashwrite_cache;
ret_msg_t g_norflashwrite_result;
sTaskRef g_norflashwrite_task_ref;
int norflashwrite_size;
int norflash_addr;
int g_norflashwrite_flag = 0; 


static void nor_flash_write_task(void *args);
static int nor_flash_write_need_size(void);
static ret_msg_t nor_flash_write_init(void);
static ret_msg_t nor_flash_write_uninit(void);
static ret_msg_t nor_flash_write_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms));
static ret_msg_t nor_flash_init_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_nor_flash_id_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t nor_flash_read_proc(op_t op, arg_t *argv, int argn);
//static ret_msg_t nor_flash_write_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t nor_flash_write_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t nor_flash_earse_proc(op_t op, arg_t *argv, int argn);




const value_list_t input_data_range =
{
    .isrange = true,
    .min = 0,
    .max = 128,
};

const value_list_t read_data_range =
{
    .isrange = true,
    .min = 0,
    .max = 128,
};


arg_t norflash_write_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = NorFlash_addr,
        .msg = "Integer type, indicates flash addr",
    },
    {
        .type = TYPE_INT,
        .arg_name = Input_data_size,
        .msg = "Integer type",
        .value_range = &input_data_range,
    },
};

arg_t norflash_read_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = NorFlash_addr,
        .msg =  "Integer type, indicates flash addr",
    },
    {
        .type = TYPE_INT,
        .arg_name = Input_data_size,
        .msg =  "Integer type",
        .value_range = &read_data_range,
    },

};

arg_t norflash_earse_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = NorFlash_addr,
        .msg =  "Integer type, indicates flash addr",
    },
};

const menu_list_t func_norflashinit =
{
    .menu_name = NorFlash_init,
    .menu_type = TYPE_FUNC,
    .parent = &extnorflash_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = nor_flash_init_proc,
    },
};
const menu_list_t func_getid =
{
    .menu_name = NorFlash_id,
    .menu_type = TYPE_FUNC,
    .parent = &extnorflash_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = get_nor_flash_id_proc,
    },
};

const menu_list_t func_norflashwrite =
{
    .menu_name = NorFlash_write,
    .menu_type = TYPE_FUNC,
    .parent = &extnorflash_menu,
    .function = {
        .argv = norflash_write_args,
        .argn = sizeof(norflash_write_args)/sizeof(arg_t),
        .RAWSizeLink = Input_data_size,
        .methods.__init = nor_flash_write_init,
        .methods.__uninit = nor_flash_write_uninit,
        .methods.cmd_handler = nor_flash_write_cmd_proc,
        .methods.raw_handler = nor_flash_write_raw_proc,
        .methods.needRAWData = nor_flash_write_need_size,
    },
};

const menu_list_t func_norflashread =
{
    .menu_name = NorFlash_read,
    .menu_type = TYPE_FUNC,
    .parent = &extnorflash_menu,
    .function = {
        .argv = norflash_read_args,
        .argn = sizeof(norflash_read_args)/sizeof(arg_t),
        .methods.cmd_handler = nor_flash_read_proc,
    },
};

const menu_list_t func_norflashearse =
{
    .menu_name = NorFlash_earse,
    .menu_type = TYPE_FUNC,
    .parent = &extnorflash_menu,
    .function = {
        .argv = norflash_earse_args,
        .argn = sizeof(norflash_earse_args)/sizeof(arg_t),
        .methods.cmd_handler = nor_flash_earse_proc,
    },
};


const menu_list_t *extnorflash_menu_sub[] =
{
    &func_norflashinit,
    &func_getid,
    &func_norflashwrite,
    &func_norflashread,
    &func_norflashearse,
    NULL  // MUST end by NULL
};

const menu_list_t extnorflash_menu =
{
    .menu_name = "Extern NorFlash",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = extnorflash_menu_sub,
    .parent = &root_menu,
};

static ret_msg_t nor_flash_write_uninit(void)
{
    ret_msg_t ret = {0};

    pub_bzero(&g_norflashwrite_result, sizeof(ret_msg_t));
    norflashwrite_size = 0;
    //pub_cache_set_status(g_flashwrite_cache, CACHE_STOPED);

    sAPI_TaskSleep(5);  // wait task exit.

    if (g_norflashwrite_task_ref)
    {
        ret.errcode = sAPI_TaskDelete(g_norflashwrite_task_ref);
        if (ret.errcode != 0)
        {
            LOG_ERROR("task delete fail, errcode=%d", ret.errcode);
            goto DONE;
        }
        else
        {
            g_norflashwrite_task_ref = NULL;
        }
    }

    if(g_norflashwrite_flag)
    {
        g_norflashwrite_flag = 0;
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

static ret_msg_t nor_flash_write_init(void)
{
    ret_msg_t ret = {0};

    ret = nor_flash_write_uninit();
    if (0 != ret.errcode)
    {
        goto DONE;
    }

    g_norflashwrite_cache = pub_cache_create(SAL_4K, CACHE_TYPE_BUFFER, CACHE_SYNC_R);
    if (NULL == g_norflashwrite_cache)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("cache create fail");
        goto DONE;
    }

DONE:
    return ret;
}

static void nor_flash_write_task(void *args)
{
    ret_msg_t ret = {0};
    char *buf = NULL;
    int read_size = 0;

    SC_SPI_ReturnCode spi_ret = SC_SPI_RC_ERROR;


    buf = sal_malloc(SAL_4K);
    if (!buf)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "malloc fail";
        goto EXIT;
    }

    while (norflashwrite_size > 0 && g_norflashwrite_cache)
    {
        read_size = norflashwrite_size <= SAL_4K ? norflashwrite_size : SAL_4K;

        read_size = pub_cache_read(g_norflashwrite_cache, buf, read_size);
        if (read_size <= 0)
        {
            LOG_INFO("cache read fail");
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "cache read fail";
            goto EXIT;
        }
#if defined(SPINOR_SUPPORT) || defined (SC_DRIVER_NORFLASH)
        spi_ret = sAPI_ExtNorFlashWrite(norflash_addr, norflashwrite_size, (unsigned int)buf);
#else
        if(SC_SPI_RC_ERROR == spi_ret)
        {
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "fun no open";
            goto EXIT;
        }
#endif


        if(SC_SPI_RC_OK != spi_ret)
        {
            LOG_ERROR("sAPI_ExtNorFlashWrite fail!!!");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = " fail!!!";
            goto EXIT;
        }
        else
        {
            ret.errcode = ERRCODE_OK;
            ret.msg = "success!!!";
        }

        pub_cache_set_status(g_norflashwrite_cache, CACHE_STOPED);

        sAPI_TaskSleep(5);  // wait task exit.
        if (g_norflashwrite_cache)
        {
            ret.errcode = pub_cache_delete(&g_norflashwrite_cache);
            if (ret.errcode != 0)
            {
                LOG_ERROR("cache delete fail");
                goto EXIT;
            }
        }

    }

EXIT:
    sal_free(buf);
    g_norflashwrite_flag = 1;
    g_norflashwrite_result = ret;
}


static ret_msg_t nor_flash_write_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms))
{
    ret_msg_t ret = {0};
    int read_size = 0;
    int process_size = 0;
    int remain_size = size;
    char buf[SAL_256] = {0};

    if (norflashwrite_size != size)
    {
        LOG_ERROR("raw data size invalid");
        LOG_ERROR("size:%d, need:%d", size, norflashwrite_size);
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

        process_size = pub_cache_write(g_norflashwrite_cache, buf, read_size);
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
    pub_cache_set_status(g_norflashwrite_cache, CACHE_STOPED);
    while (g_norflashwrite_flag != 1) sAPI_TaskSleep(20);

    if (ret.errcode != 0) return ret;
    else return g_norflashwrite_result;
}

static int nor_flash_write_need_size(void)
{
    return norflashwrite_size;
}

static ret_msg_t nor_flash_write_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int sendsize = 0;
    int addr = 0;
    pl_demo_get_value(&addr, argv, argn, NorFlash_addr);
    pl_demo_get_value(&sendsize, argv, argn, Input_data_size);
    if (sendsize <= 0)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    norflash_addr = addr;
    norflashwrite_size = sendsize;

    if (sendsize > 0)
    {
        ret.errcode = sAPI_TaskCreate(&g_norflashwrite_task_ref,
                                      NULL,
                                      SAL_2K,
                                      150,
                                      "NorFlashWrite",
                                      nor_flash_write_task,
                                      NULL);
        if (ret.errcode != SC_SUCCESS)
        {
            ret.msg = "task creat fail";
        }
    }

    return ret;
}



static ret_msg_t nor_flash_init_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char buf[128] = {0};
    arg_t out_arg[1] = {0};
#if defined(SPINOR_SUPPORT) || defined (SC_DRIVER_NORFLASH)

    sAPI_ExtNorFlashInit();

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf)," ok! defalut SSP0 CLK 13MHz!");
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "NorFlash init";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
#else
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf)," fun not open");
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "NorFlash";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
#endif
    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);
    if (0 != ret.errcode)
    {
        ret.msg = "NorFlash_init: pl_demo_post_data fail";
    }

    return ret;
}


static ret_msg_t get_nor_flash_id_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    arg_t out_arg[1] = {0};
    char buf[64] = {0};

#if defined(SPINOR_SUPPORT) || defined (SC_DRIVER_NORFLASH)
    SC_SPI_ReturnCode spi_ret = SC_SPI_RC_ERROR;
    unsigned int nor_flash_id = 0;
    spi_ret = sAPI_ExtNorFlashReadID((unsigned char *)&nor_flash_id);
    memset(buf, 0, sizeof(buf));
    if(SC_SPI_RC_OK == spi_ret)
    {
        snprintf(buf, sizeof(buf)," 0x%x", nor_flash_id);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "Nor Flash ID:";
        out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    }
    else{
        snprintf(buf, sizeof(buf)," fail! Please Check again!!!");
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "Nor Flash ID:";
        out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    }
#else
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf)," fun not open");
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "NorFlash";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
#endif
    pl_demo_release_value(out_arg[0].value);
    if (0 != ret.errcode)
    {
        ret.msg = "NorFlash_id: pl_demo_post_data fail";
    }
    return ret;
}






static ret_msg_t nor_flash_read_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    ret.errcode = ERRCODE_OK;
    arg_t out_arg[1] = {0};
    char buf[16] = {0};

    int NorFlashAddr;
    int input_data_size =0;

    pl_demo_get_value(&NorFlashAddr, argv, argn, NorFlash_addr);
    pl_demo_get_value(&input_data_size, argv, argn, Input_data_size);

#if defined(SPINOR_SUPPORT) || defined (SC_DRIVER_NORFLASH)
    SC_SPI_ReturnCode spi_ret = SC_SPI_RC_ERROR;
    char read_buf[128] = {0};
    memset(read_buf, 0X00, input_data_size);
    spi_ret = sAPI_ExtNorFlashRead(NorFlashAddr, input_data_size, (unsigned int)read_buf);
    if(SC_SPI_RC_OK == spi_ret){
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "Read data";
    out_arg[0].value = pl_demo_make_value(0, read_buf, strlen(read_buf));
    }
    else{
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf),"fail");
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "Read data";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }
#else
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf)," fun not open");
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "NorFlash";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
#endif
    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);

    if (0 != ret.errcode)
    {
        ret.msg = "NorFlash Write: pl_demo_post_data fail";
    }

    return ret;
}

static ret_msg_t nor_flash_earse_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    ret.errcode = ERRCODE_OK;

    arg_t out_arg[1] = {0};
    char buf[16] = {0};

    int NorFlashAddr;

    pl_demo_get_value(&NorFlashAddr, argv, argn, NorFlash_addr);

#if defined(SPINOR_SUPPORT) || defined (SC_DRIVER_NORFLASH)
    SC_SPI_ReturnCode spi_ret = SC_SPI_RC_ERROR;
    memset(buf, 0, sizeof(buf));
    spi_ret = sAPI_ExtNorFlashSectorErase(NorFlashAddr, 0x1000);
    if(SC_SPI_RC_OK == spi_ret){

    snprintf(buf, sizeof(buf),"success");
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "Earse data";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }
    else{
    snprintf(buf, sizeof(buf),"fail");
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "Earse data";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }

#else
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf)," fun not open");
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "NorFlash";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
#endif

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);

    if (0 != ret.errcode)
    {
        ret.msg = "NorFlash Earse: pl_demo_post_data fail";
    }

    return ret;
}
