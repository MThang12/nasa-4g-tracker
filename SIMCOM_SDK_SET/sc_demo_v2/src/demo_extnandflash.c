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

#define LOG_ERROR(...) sal_log_error("[DEMO-NandFlash] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-NandFlash] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-NandFlash] " __VA_ARGS__)

#define NandFlash_addr "nand FLash Addr"
#define Input_data_size "Input data size"
#define NandFlash_init "nand Flash init"
#define NandFlash_id "Get nandFlash id"
#define NandFlash_write "nand Flash Write"
#define NandFlash_read "nand Flash Read"
#define NandFlash_earse "nand Flash Earse"

pub_cache_p g_nandflashwrite_cache;
ret_msg_t g_nandflashwrite_result;
sTaskRef g_nandflashwrite_task_ref;
int nandflashwrite_size;
int exnandflash_addr;
int g_nandflashwrite_flag = 0;


static void nand_flash_write_task(void *args);
static int nand_flash_write_need_size(void);
static ret_msg_t nand_flash_write_init(void);
static ret_msg_t nand_flash_write_uninit(void);
static ret_msg_t nand_flash_write_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms));


static ret_msg_t nand_flash_init_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_nand_flash_id_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t nand_flash_read_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t nand_flash_write_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t nand_flash_earse_proc(op_t op, arg_t *argv, int argn);


const value_list_t input_nanddata_range =
{
    .isrange = true,
    .min = 0,
    .max = 128,
};

const value_list_t read_nanddata_range =
{
    .isrange = true,
    .min = 0,
    .max = 128,
};


arg_t nandflash_write_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = NandFlash_addr,
        .msg =  "Integer type",
    },
    {
        .type = TYPE_INT,
        .arg_name = Input_data_size,
        .msg =  "Integer type",
        .value_range = &input_nanddata_range,
    },
};

arg_t nandflash_read_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = NandFlash_addr,
        .msg =  "Integer type",
    },
    {
        .type = TYPE_INT,
        .arg_name = Input_data_size,
        .msg =  "Integer type",
        .value_range = &read_nanddata_range,
    },

};


arg_t readidargs[] =
{
    {
        .type = TYPE_INT,
        .arg_name = "read id mode",
        .msg =  " 0(OPCODE)  1(OPCODE_ADDR)  2(OPCODE_DUMMY)",
    },
};

arg_t nandflash_earse_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = NandFlash_addr,
        .msg =  "start address",
    },
};

const menu_list_t func_nandflashinit =
{
    .menu_name = NandFlash_init,
    .menu_type = TYPE_FUNC,
    .parent = &extnandflash_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = nand_flash_init_proc,
    },
};
const menu_list_t func_getnandflashid =
{
    .menu_name = NandFlash_id,
    .menu_type = TYPE_FUNC,
    .parent = &extnandflash_menu,
    .function = {
        .argv = readidargs,
        .argn = sizeof(readidargs)/sizeof(arg_t),
        .methods.cmd_handler = get_nand_flash_id_proc,
    },
};

const menu_list_t func_nandflashwrite =
{
    .menu_name = NandFlash_write,
    .menu_type = TYPE_FUNC,
    .parent = &extnandflash_menu,
    .function = {
        .argv = nandflash_write_args,
        .argn = sizeof(nandflash_write_args)/sizeof(arg_t),
        .RAWSizeLink = Input_data_size,
        .methods.__init = nand_flash_write_init,
        .methods.__uninit = nand_flash_write_uninit,
        .methods.cmd_handler = nand_flash_write_cmd_proc,
        .methods.raw_handler = nand_flash_write_raw_proc,
        .methods.needRAWData = nand_flash_write_need_size,
    },
};

const menu_list_t func_nandflashread =
{
    .menu_name = NandFlash_read,
    .menu_type = TYPE_FUNC,
    .parent = &extnandflash_menu,
    .function = {
        .argv = nandflash_read_args,
        .argn = sizeof(nandflash_read_args)/sizeof(arg_t),
        .methods.cmd_handler = nand_flash_read_proc,
    },
};

const menu_list_t func_nandflashearse =
{
    .menu_name = NandFlash_earse,
    .menu_type = TYPE_FUNC,
    .parent = &extnandflash_menu,
    .function = {
        .argv = nandflash_earse_args,
        .argn = sizeof(nandflash_earse_args)/sizeof(arg_t),
        .methods.cmd_handler = nand_flash_earse_proc,
    },
};


const menu_list_t *extnandflash_menu_sub[] =
{
    &func_nandflashinit,
    &func_getnandflashid,
    &func_nandflashwrite,
    &func_nandflashread,
    &func_nandflashearse,
    NULL  // MUST end by NULL
};

const menu_list_t extnandflash_menu =
{
    .menu_name = "Extern nandFlash",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = extnandflash_menu_sub,
    .parent = &root_menu,
};

static ret_msg_t nand_flash_write_uninit(void)
{
    ret_msg_t ret = {0};

    pub_bzero(&g_nandflashwrite_result, sizeof(ret_msg_t));
    nandflashwrite_size = 0;
    //pub_cache_set_status(g_flashwrite_cache, CACHE_STOPED);

    sAPI_TaskSleep(5);  // wait task exit.

    if (g_nandflashwrite_task_ref)
    {
        ret.errcode = sAPI_TaskDelete(g_nandflashwrite_task_ref);
        if (ret.errcode != 0)
        {
            LOG_ERROR("task delete fail, errcode=%d", ret.errcode);
            goto DONE;
        }
        else
        {
            g_nandflashwrite_task_ref = NULL;
        }
    }

    if(g_nandflashwrite_flag)
    {
        g_nandflashwrite_flag = 0;
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

static ret_msg_t nand_flash_write_init(void)
{
    ret_msg_t ret = {0};

    ret = nand_flash_write_uninit();
    if (0 != ret.errcode)
    {
        goto DONE;
    }

    g_nandflashwrite_cache = pub_cache_create(SAL_4K, CACHE_TYPE_BUFFER, CACHE_SYNC_R);
    if (NULL == g_nandflashwrite_cache)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("cache create fail");
        goto DONE;
    }

DONE:
    return ret;
}

static void nand_flash_write_task(void *args)
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

    while (nandflashwrite_size > 0 && g_nandflashwrite_cache)
    {
        read_size = nandflashwrite_size <= SAL_4K ? nandflashwrite_size : SAL_4K;

        read_size = pub_cache_read(g_nandflashwrite_cache, buf, read_size);
        if (read_size <= 0)
        {
            LOG_INFO("cache read fail");
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "cache read fail";
            goto EXIT;
        }
#ifdef SIMCOM_SPINAND_FLASH_SUPPORT
         spi_ret = sAPI_SPINAND_Write(exnandflash_addr, (unsigned int)buf, nandflashwrite_size, BOOT_FLASH);
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
            LOG_ERROR("sAPI_SPINAND_Write fail!!!");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "fail!!!";
            goto EXIT;
        }
        else
        {
            ret.errcode = ERRCODE_OK;
            ret.msg = "success!!!";
        }

        pub_cache_set_status(g_nandflashwrite_cache, CACHE_STOPED);

        sAPI_TaskSleep(5);  // wait task exit.
        if (g_nandflashwrite_cache)
        {
            ret.errcode = pub_cache_delete(&g_nandflashwrite_cache);
            if (ret.errcode != 0)
            {
                LOG_ERROR("cache delete fail");
                goto EXIT;
            }
        }

    }

EXIT:
    sal_free(buf);
    g_nandflashwrite_flag = 1;
    g_nandflashwrite_result = ret;
}


static ret_msg_t nand_flash_write_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms))
{
    ret_msg_t ret = {0};
    int read_size = 0;
    int process_size = 0;
    int remain_size = size;
    char buf[SAL_256] = {0};

    if (nandflashwrite_size != size)
    {
        LOG_ERROR("raw data size invalid");
        LOG_ERROR("size:%d, need:%d", size, nandflashwrite_size);
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

        process_size = pub_cache_write(g_nandflashwrite_cache, buf, read_size);
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
    pub_cache_set_status(g_nandflashwrite_cache, CACHE_STOPED);
    while (g_nandflashwrite_flag != 1) sAPI_TaskSleep(20);

    if (ret.errcode != 0) return ret;
    else return g_nandflashwrite_result;
}

static int nand_flash_write_need_size(void)
{
    return nandflashwrite_size;
}

static ret_msg_t nand_flash_write_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int sendsize = 0;
    int addr = 0;
    pl_demo_get_value(&addr, argv, argn, NandFlash_addr);
    pl_demo_get_value(&sendsize, argv, argn, Input_data_size);
    if (sendsize <= 0)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    exnandflash_addr = addr;
    nandflashwrite_size = sendsize;

    if (sendsize > 0)
    {
        ret.errcode = sAPI_TaskCreate(&g_nandflashwrite_task_ref,
                                      NULL,
                                      SAL_2K,
                                      150,
                                      "NandFlashWrite",
                                      nand_flash_write_task,
                                      NULL);
        if (ret.errcode != SC_SUCCESS)
        {
            ret.msg = "task creat fail";
        }
    }

    return ret;
}












static ret_msg_t nand_flash_init_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

#ifdef SIMCOM_SPINAND_FLASH_SUPPORT
    char buf[128] = {0};
    arg_t out_arg[1] = {0};
    int spi_ret;
    spi_ret = extSpiNandInit();
    if(spi_ret == 0){
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf)," ok! defalut SSP0 CLK 26MHz!");
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "nandFlash init";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }else{
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf)," fail");
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "nandFlash init";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }
    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);
    if (0 != ret.errcode)
    {
        ret.msg = "nandFlash_init: pl_demo_post_data fail";
    }
#else
    ret.msg = "nand flash fun yet not open";
#endif


    return ret;
}


static ret_msg_t get_nand_flash_id_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
#ifdef SIMCOM_SPINAND_FLASH_SUPPORT
    arg_t out_arg[1] = {0};
    char buf[64] = {0};
    SC_SPI_ReturnCode spi_ret = SC_SPI_RC_ERROR;
    unsigned char nand_flash_id[3] = {0};
    int mode ;

    pl_demo_get_value(&mode, argv, argn, "read id mode");

    switch(mode){
        case NAND_READID_MODE_OPCODE:
            mode = NAND_READID_MODE_OPCODE;
            break;
        case NAND_READID_MODE_OPCODE_ADDR:
            mode = NAND_READID_MODE_OPCODE_ADDR;
            break;
        case NAND_READID_MODE_OPCODE_DUMMY:
            mode = NAND_READID_MODE_OPCODE_DUMMY;
            break;
        default:
            break;
    }

    spi_ret = sAPI_ExtNnandFlashReadID(mode,nand_flash_id);
    memset(buf, 0, sizeof(buf));
    if(SC_SPI_RC_OK == spi_ret)
    {
        snprintf(buf, sizeof(buf)," Manufacture ID 0x%x  Device ID 0x%x", nand_flash_id[0],nand_flash_id[1]);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "nand Flash ID:";
        out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    }
    else{
        snprintf(buf, sizeof(buf)," fail! Please Check again!!!");
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "nand Flash ID:";
        out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    }
    pl_demo_release_value(out_arg[0].value);
    if (0 != ret.errcode)
    {
        ret.msg = "nandFlash_id: pl_demo_post_data fail";
    }
#else
    ret.msg = "nand flash fun yet not open";
#endif
    return ret;
}






static ret_msg_t nand_flash_read_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
#ifdef SIMCOM_SPINAND_FLASH_SUPPORT
    int spi_ret ;
    ret.errcode = ERRCODE_OK;

    arg_t out_arg[1] = {0};
    char buf[16] = {0};

    char read_buf[128] = {0};

    int nandFlashAddr;
    int input_data_size =0;

    pl_demo_get_value(&nandFlashAddr, argv, argn, NandFlash_addr);
    pl_demo_get_value(&input_data_size, argv, argn, Input_data_size);

    memset(read_buf, 0X00, input_data_size);
    spi_ret = sAPI_SPINAND_Read(nandFlashAddr, (unsigned int)read_buf, input_data_size, BOOT_FLASH);
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

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);

    if (0 != ret.errcode)
    {
        ret.msg = "nandFlash Write: pl_demo_post_data fail";
    }
#else
    ret.msg = "nand flash fun yet not open";
#endif
    return ret;
}



#if 0
static ret_msg_t nand_flash_write_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
#ifdef SIMCOM_SPINAND_FLASH_SUPPORT
    int spi_ret ;
    ret.errcode = ERRCODE_OK;

    arg_t out_arg[2] = {0};
    char buf[16] = {0};
    char write_buf[128] = {0};
    char write1_buf[128] = {0};
    char read_buf[128] = {0};

    int nandFlashAddr;
    int input_data_size =0;

    const char *input_data = NULL;
    pl_demo_get_value(&nandFlashAddr, argv, argn, NandFlash_addr);
    pl_demo_get_value(&input_data_size, argv, argn, Input_data_size);
    pl_demo_get_data(&input_data, argv, argn, "Input data");
    memcpy(write_buf,input_data,input_data_size);
    memcpy(write1_buf,input_data,input_data_size);


    spi_ret = sAPI_SPINAND_Write(nandFlashAddr, (unsigned int)write_buf, input_data_size, BOOT_FLASH);
    if(SC_SPI_RC_OK == spi_ret){
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "Write data";
    out_arg[0].value = pl_demo_make_value(0, write1_buf, strlen(write1_buf));
    }
    else{
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf),"fail");
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "Write data";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }

    memset(read_buf, 0X00, input_data_size);
    spi_ret = sAPI_SPINAND_Read(nandFlashAddr, (unsigned int)read_buf, input_data_size, BOOT_FLASH);
    if(SC_SPI_RC_OK == spi_ret){
    out_arg[1].type = TYPE_STR;
    out_arg[1].arg_name = "Read data";
    out_arg[1].value = pl_demo_make_value(0, read_buf, strlen(read_buf));
    }
    else{
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf),"fail");
    out_arg[1].type = TYPE_STR;
    out_arg[1].arg_name = "Read data";
    out_arg[1].value = pl_demo_make_value(0, buf, strlen(buf));
    }

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 2, NULL, 0);
    pl_demo_release_value(out_arg[0].value);
    pl_demo_release_value(out_arg[1].value);
    if (0 != ret.errcode)
    {
        ret.msg = "nandFlash Write: pl_demo_post_data fail";
    }
#else
    ret.msg = "nand flash fun yet not open";
#endif

    return ret;
}
#endif



static ret_msg_t nand_flash_earse_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
#ifdef SIMCOM_SPINAND_FLASH_SUPPORT
    int spi_ret ;
    ret.errcode = ERRCODE_OK;

    arg_t out_arg[1] = {0};
    char buf[16] = {0};

    int nandFlashAddr;

    pl_demo_get_value(&nandFlashAddr, argv, argn, NandFlash_addr);

    memset(buf, 0, sizeof(buf));
    spi_ret = sAPI_SPINAND_Erase(nandFlashAddr, 0x800, BOOT_FLASH);
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


    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);

    if (0 != ret.errcode)
    {
        ret.msg = "nandFlash Earse: pl_demo_post_data fail";
    }
#else
    ret.msg = "nand flash fun yet not open";
#endif

    return ret;
}
