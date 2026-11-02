/*
 * @Author: xiaobing.fang
 * @Date: 2023-12-17 14.17.42
 * @Last Modified by: xiaobing.fang
 * @Last Modified time: 2023-12-18 12.17.42
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

#define LOG_ERROR(...) sal_log_error("[DEMO-SPI] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-SPI] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-SPI] " __VA_ARGS__)


#define SPI_port "SPI port"
#define SPI_clk "SPI clk"
#define SPI_mode "SPI work mode"
#define SPI_cs "SPI csmode"
#define SPI_init "Spi interface init"
#define SPI_readid "Read flash id"
#define SPI_rwstatus "flash R/W status reg"
#define SPI_readiddma "Read flash id for DMA"

static ret_msg_t spiinit_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t readid_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t rwstatus_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t readid_dma_cmd_proc(op_t op, arg_t *argv, int argn);


const value_t spi_port[] =
{
    {
        .bufp = "spi0",
        .size = 4,
    },

    {
        .bufp = "spi1",
        .size = 4,
    },

    {
        .bufp = "spi2",
        .size = 4,
    },

};

const value_t spi_clk[] =
{
    {
        .bufp = "6MHz",
        .size = 4,
    },

    {
        .bufp = "13MHz",
        .size = 5,
    },

    {
        .bufp = "26MHz",
        .size = 5,
    },

    {
        .bufp = "812KHz",
        .size = 6,
    },

};

const value_t spi_mode[] =
{
    {
        .bufp = "PH0_PO0",
        .size = 7,
    },

    {
        .bufp = "PH0_PO1",
        .size = 7,
    },

    {
        .bufp = "PH1_PO0",
        .size = 7,
    },

    {
        .bufp = "PH1_PO1",
        .size = 7,
    },

};

const value_t spi_csmode[] =
{
    {
        .bufp = "GPIO_control",
        .size = 12,
    },

    {
        .bufp = "SPI_SYNC",
        .size = 8,
    },

    {
        .bufp = "Cust_GPIO_control",
        .size = 17,
    },
};



const value_list_t spiport_range =
{
    .isrange = false,
    .list_head = spi_port,
    .count = 3,
};
const value_list_t spiclk_range =
{
    .isrange = false,
    .list_head = spi_clk,
    .count = 4,
};
const value_list_t spimode_range =
{
    .isrange = false,
    .list_head = spi_mode,
    .count = 4,
};
const value_list_t spics_range =
{
    .isrange = false,
    .list_head = spi_csmode,
    .count = 3,
};

arg_t spiinit_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = SPI_port,
        .value_range = &spiport_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = SPI_clk,
        .value_range = &spiclk_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = SPI_mode,
        .value_range = &spimode_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = SPI_cs,
        .value_range = &spics_range,
    },
};


const menu_list_t func_spiinit =
{
    .menu_name = SPI_init,
    .menu_type = TYPE_FUNC,
    .parent = &spi_menu,
    .function = {
        .argv = spiinit_args,
        .argn = sizeof(spiinit_args)/sizeof(arg_t),
        .methods.cmd_handler = spiinit_cmd_proc,
    },
};

const menu_list_t func_readid =
{
    .menu_name = SPI_readid,
    .menu_type = TYPE_FUNC,
    .parent = &spi_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = readid_cmd_proc,
    },
};

const menu_list_t func_rwstatus =
{
    .menu_name = SPI_rwstatus,
    .menu_type = TYPE_FUNC,
    .parent = &spi_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = rwstatus_cmd_proc,
    },
};

const menu_list_t func_readiddma =
{
    .menu_name = SPI_readiddma,
    .menu_type = TYPE_FUNC,
    .parent = &spi_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = readid_dma_cmd_proc,
    },
};

const menu_list_t *spi_menu_sub[] =
{
    &func_spiinit,
    &func_readid,
    &func_rwstatus,
    &func_readiddma,
    NULL  // MUST end by NULL
};

const menu_list_t spi_menu =
{
    .menu_name = "SPI",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = spi_menu_sub,
    .parent = &root_menu,
};

static SC_SPI_DEV spiDev = {0};

static ret_msg_t spiinit_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char buf[64] = {0};
    arg_t out_arg[1] = {0};

    int ssp_port = 0;
    int ssp_clk = 0;
    int ssp_mode = 0;
    int ssp_csmode = 0;

    const char *ssp_port_input = NULL;
    const char *ssp_clk_input = NULL;
    const char *ssp_mode_input = NULL;
    const char *ssp_cs_input = NULL;

    pl_demo_get_data(&ssp_port_input, argv, argn, SPI_port);
    pl_demo_get_data(&ssp_clk_input, argv, argn, SPI_clk);
    pl_demo_get_data(&ssp_mode_input, argv, argn, SPI_mode);
    pl_demo_get_data(&ssp_cs_input, argv, argn, SPI_cs);


    if ((!ssp_port_input)||(!ssp_clk_input)||(!ssp_mode_input)||(!ssp_cs_input))
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    if(!strncmp(ssp_port_input,"spi0",4))
    {
        ssp_port = 0;
    }
    else if(!strncmp(ssp_port_input,"spi1",4))
    {
        ssp_port = 1;
    }
    else if(!strncmp(ssp_port_input,"spi2",4))
    {
        ssp_port = 2;
    }

    if(!strncmp(ssp_clk_input,"6MHz",4))
    {
        ssp_clk = 0;
    }
    else if(!strncmp(ssp_clk_input,"13MHz",5))
    {
        ssp_clk = 1;
    }
    else if(!strncmp(ssp_clk_input,"26MHz",5))
    {
        ssp_clk = 2;
    }
    else if(!strncmp(ssp_clk_input,"812KHz",6))
    {
        ssp_clk = 6;
    }

    if(!strncmp(ssp_mode_input,"PH0_PO0",7))
    {
        ssp_mode = 0;
    }
    else if(!strncmp(ssp_mode_input,"PH0_PO1",7))
    {
        ssp_mode = 1;
    }
    else if(!strncmp(ssp_mode_input,"PH1_PO0",7))
    {
        ssp_mode = 2;
    }
    else if(!strncmp(ssp_mode_input,"PH1_PO1",7))
    {
        ssp_mode = 3;
    }


    if(!strncmp(ssp_cs_input,"GPIO_control",12))
    {
        ssp_csmode = 1;
    }
    else if(!strncmp(ssp_cs_input,"SPI_SYNC",8))
    {
        ssp_csmode = 0;
    }
    else if(!strncmp(ssp_cs_input,"Cust_GPIO_control",17))
    {
        ssp_csmode = 2;
    }

    spiDev.clock = ssp_clk;
    spiDev.index = (unsigned char)ssp_port;
    spiDev.mode = ssp_mode;
    spiDev.csMode = ssp_csmode;
    sAPI_SpiConfigInitEx(&spiDev);

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf),"spi port%d init ok",spiDev.index);
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = NULL;
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);
    if (0 != ret.errcode)
    {
        ret.msg = "spi_init: pl_demo_post_data fail";
    }

    return ret;
}

static ret_msg_t readid_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    arg_t out_arg[1] = {0};
    char buf[64] = {0};
    SC_SPI_ReturnCode spi_ret = SC_SPI_RC_ERROR;
    unsigned char SendData[4] = {0x9F, 0, 0, 0};
    unsigned char RevData[4] = {0};
    spi_ret = sAPI_SpiReadBytesEx(&spiDev, SendData, 1, RevData, 3);
    memset(buf, 0, sizeof(buf));
    if(SC_SPI_RC_OK == spi_ret)
    {
        snprintf(buf, sizeof(buf)," %x", (RevData[0] << 16) | (RevData[1] << 8) | RevData[2]);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "READ FLASH ID:";
        out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    }
    else{
        snprintf(buf, sizeof(buf),"fail");
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "READ FLASH ID:";
        out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    }
    pl_demo_release_value(out_arg[0].value);
    if (0 != ret.errcode)
    {
        ret.msg = "spi_readid: pl_demo_post_data fail";
    }
    return ret;
}

static ret_msg_t rwstatus_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_SPI_ReturnCode spi_ret = SC_SPI_RC_ERROR;
    arg_t out_arg[2] = {0};
    char buf[64] = {0};
    char buf2[64] = {0};

    unsigned char writeData[2] = {0x01};
    unsigned char readCmd = 0x05;
    unsigned char status = 0;

    spi_ret = sAPI_SpiReadBytesEx(&spiDev, &readCmd, 1, &status, 1);
    if(SC_SPI_RC_OK == spi_ret){
        snprintf(buf, sizeof(buf)," %x", status);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "Read Flash status:";
        out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }
    else{
        snprintf(buf, sizeof(buf)," fail");
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "Read Flash status:";
        out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }



    writeData[1] = status | 0x02;
    spi_ret = sAPI_SpiWriteBytesEx(&spiDev, writeData, 2);
    if(SC_SPI_RC_OK == spi_ret)
    {
        snprintf(buf2, sizeof(buf2)," %x", writeData[1]);
        out_arg[1].type = TYPE_STR;
        out_arg[1].arg_name = "Write Flash status:";
        out_arg[1].value = pl_demo_make_value(0, buf, strlen(buf));
    }
    else{
        snprintf(buf2, sizeof(buf2)," fail");
        out_arg[1].type = TYPE_STR;
        out_arg[1].arg_name = "Write Flash status:";
        out_arg[1].value = pl_demo_make_value(0, buf, strlen(buf));
    }

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 2, NULL, 0);
    if (0 != ret.errcode)
    {
        ret.msg = "spi_r/w status: pl_demo_post_data fail";
    }
    pl_demo_release_value(out_arg[0].value);
    pl_demo_release_value(out_arg[1].value);

    return ret;
}

static ret_msg_t readid_dma_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    arg_t out_arg[1] = {0};
    char buf[64] = {0};
    memset(buf, 0, sizeof(buf));
#ifdef SPIDRV_SUPPORT
    SC_SPI_ReturnCode spi_ret = SC_SPI_RC_ERROR;
    unsigned char SendData1[4] = {0x9F, 0, 0, 0};
    unsigned char RevData1[4] = {0};
    spi_ret = sAPI_SpiReadBytesEx(&spiDev, SendData1, 1, RevData1, 3);

    if(SC_SPI_RC_OK == spi_ret)
    {
        snprintf(buf, sizeof(buf)," %x", (RevData1[1] << 16) | (RevData1[2] << 8) | RevData1[3]);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "USE DMA READ FLASH ID:";
        out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    }
    else{
        snprintf(buf, sizeof(buf),"fail");
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "USE DMA READ FLASH ID:";
        out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
        ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    }
#else
    snprintf(buf, sizeof(buf)," Function Not yet opened");
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "USE DMA READ FLASH ID:";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
#endif
    pl_demo_release_value(out_arg[0].value);
    if (0 != ret.errcode)
    {
        ret.msg = "spi_readid: pl_demo_post_data fail";
    }
    return ret;
}
