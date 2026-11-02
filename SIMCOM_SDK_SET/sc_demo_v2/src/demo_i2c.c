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
#include "sc_i2c.h"

#define I2C_port "I2C port"
#define I2C_clk "I2C clk"
#define I2C_open "I2C open"
#define I2C_close "I2C close"
#define I2C_write "I2C write"
#define I2C_writeEx "I2C writeEx"
#define I2C_read "I2C read"
#define I2C_readEx "I2C readEx"
#define SLAVE_ADDR "slave addr"
#define REG_ADDR "slave reg addr"
#define REG_DATA "slave reg data"
#define DATA_LEN "slave_data_len"
#define LOG_ERROR(...) sal_log_error("[DEMO-I2C] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-I2C] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-I2C] " __VA_ARGS__)


static ret_msg_t i2c_init_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t i2c_deinit_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t i2c_write_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t i2c_read_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t i2c_writeex_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t i2c_readex_cmd_proc(op_t op, arg_t *argv, int argn);

//such as :input "0x23"  return hex 0x23
static int trans(char* str){
    unsigned int hex = 0;
    sscanf(str, "%x", &hex);
    return hex;
}

const value_t i2c_port[3] =
{
    {
        .bufp = "i2c1",
        .size = 4,
    },

    {
        .bufp = "i2c2",
        .size = 4,
    },

    {
        .bufp = "i2c3",
        .size = 4,
    },

};

const value_t i2c_clk[3] =
{
    {
        .bufp = "100KHz",
        .size = 6,
    },

    {
        .bufp = "400KHz",
        .size = 6,
    },

    {
        .bufp = "3.4MHz",
        .size = 6,
    },
};

const value_list_t i2cport_range =
{
    .isrange = false,
    .list_head = i2c_port,
    .count = 3,
};

const value_list_t i2cclk_range =
{
    .isrange = false,
    .list_head = i2c_clk,
    .count = 3,
};

arg_t i2cport_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = I2C_port,
        .value_range = &i2cport_range,
    },
    {
        .type = TYPE_RAW,
        .arg_name = SLAVE_ADDR,
        .msg = "slave write addr (0x00-0xFF)",
    },
    {
        .type = TYPE_RAW,
        .arg_name = REG_ADDR,
        .msg = "0x00-0xFF",
    },
    {
        .type = TYPE_RAW,
        .arg_name = REG_DATA,
        .msg = "0x00-0xFF",
    },
    {
        .type = TYPE_INT,
        .arg_name = DATA_LEN,
        .msg =  "1 or 2",
    },

};

arg_t i2cport_r_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = I2C_port,
        .value_range = &i2cport_range,
    },
    {
        .type = TYPE_RAW,
        .arg_name = SLAVE_ADDR,
        .msg = "slave write addr (0x00-0xFF)",
    },
    {
        .type = TYPE_RAW,
        .arg_name = REG_ADDR,
        .msg = "0x00-0xFF",
    },
    {
        .type = TYPE_INT,
        .arg_name = DATA_LEN,
        .msg =  "1 or 2",
    },

};

#if 0
arg_t i2cclk_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = "I2C clk",
        .value_range = &i2cclk_range,
    }
};
#endif

arg_t i2copen_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = I2C_port,
        .value_range = &i2cport_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = I2C_clk,
        .value_range = &i2cclk_range,
    },
};


const menu_list_t func_i2cinit =
{
    .menu_name = I2C_open,
    .menu_type = TYPE_FUNC,
    .parent = &i2c_menu,
    .function = {
        .argv = i2copen_args,
        .argn = sizeof(i2copen_args)/sizeof(arg_t),
        .methods.cmd_handler = i2c_init_cmd_proc,
    },
};

const menu_list_t func_i2cdeinit =
{
    .menu_name = I2C_close,
    .menu_type = TYPE_FUNC,
    .parent = &i2c_menu,
    .function = {
        .argv = i2copen_args,
        .argn = sizeof(i2copen_args)/sizeof(arg_t),
        .methods.cmd_handler = i2c_deinit_cmd_proc,
    },
};

const menu_list_t func_i2cwrite =
{
    .menu_name = "I2C write",
    .menu_type = TYPE_FUNC,
    .parent = &i2c_menu,
    .function = {
        .argv = i2cport_args,
        .argn = sizeof(i2cport_args)/sizeof(arg_t),
        .methods.cmd_handler = i2c_write_cmd_proc,
    },
};

const menu_list_t func_i2cread =
{
    .menu_name = I2C_read,
    .menu_type = TYPE_FUNC,
    .parent = &i2c_menu,
    .function = {
        .argv = i2cport_r_args,
        .argn = sizeof(i2cport_r_args)/sizeof(arg_t),
        .methods.cmd_handler = i2c_read_cmd_proc,
    },
};

const menu_list_t func_i2cwriteEx =
{
    .menu_name = I2C_writeEx,
    .menu_type = TYPE_FUNC,
    .parent = &i2c_menu,
    .function = {
        .argv = i2cport_args,
        .argn = sizeof(i2cport_args)/sizeof(arg_t),
        .methods.cmd_handler = i2c_writeex_cmd_proc,
    },
};

const menu_list_t func_i2creadEx =
{
    .menu_name = I2C_readEx,
    .menu_type = TYPE_FUNC,
    .parent = &i2c_menu,
    .function = {
        .argv = i2cport_r_args,
        .argn = sizeof(i2cport_r_args)/sizeof(arg_t),
        .methods.cmd_handler = i2c_readex_cmd_proc,
    },
};

const menu_list_t *i2c_menu_sub[] =
{
    &func_i2cinit,
    &func_i2cdeinit,
    &func_i2cwrite,
    &func_i2cread,
    &func_i2cwriteEx,
    &func_i2creadEx,
    NULL  // MUST end by NULL
};

const menu_list_t i2c_menu =
{
    .menu_name = "I2C",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = i2c_menu_sub,
    .parent = &root_menu,
};


UINT8 regAddr;
UINT16 regData;
UINT8 i2cdata[2] = {0x01, 0x56};
UINT8 r_i2cdata[2] = {0};
#define NAU8810_SLAVE_ADDR 0x34
static ret_msg_t i2c_init_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    arg_t out_arg[1] = {0};
    char buf[64] = {0};
    SC_I2C_ReturnCode i2c_ret = SC_I2C_RC_NOT_OK;
    SC_I2C_DEV i2c_dev = {0};
    int i2c_port =0;
    int i2c_clk =0;
    const char *i2c_port_input = NULL;
    const char *i2c_clk_input = NULL;
    pl_demo_get_data(&i2c_port_input, argv, argn, I2C_port);
    pl_demo_get_data(&i2c_clk_input, argv, argn, I2C_clk);

    if ((!i2c_port_input)||(!i2c_clk_input))
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    if(!strncmp(i2c_port_input,"i2c1",4))
    {
        i2c_port = 1;
    }
    else if(!strncmp(i2c_port_input,"i2c2",4))
    {
        i2c_port = 2;
    }
    else if(!strncmp(i2c_port_input,"i2c3",4))
    {
        i2c_port = 3;
    }

    if(!strncmp(i2c_clk_input,"100KHz",4))
    {
        i2c_clk = 1;
    }
    else if(!strncmp(i2c_clk_input,"400KHz",4))
    {
        i2c_clk = 2;
    }
    else if(!strncmp(i2c_clk_input,"3.4MHz",4))
    {
        i2c_clk = 3;
    }

    switch(i2c_port)
    {
        case 1:
            i2c_dev.i2c_channel = SC_I2C_CHANNEL0;
            break;
        case 2:
            i2c_dev.i2c_channel = SC_I2C_CHANNEL1;
            break;
        case 3:
            i2c_dev.i2c_channel = SC_I2C_CHANNEL2;
            break;
        default:
            break;
    }

    switch(i2c_clk)
    {
        case 1:
            i2c_dev.i2c_clock = SC_I2C_STANDARD_MODE;
            break;
        case 2:
            i2c_dev.i2c_clock = SC_I2C_FAST_MODE;
            break;
        case 3:
            i2c_dev.i2c_clock = SC_I2C_HS_MODE;
            break;
        default:
            break;
    }

    i2c_ret =  sAPI_I2CConfigInit(&i2c_dev);
    if(i2c_ret == SC_I2C_RC_OK)
    {
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf),"port %d init ok",i2c_dev.i2c_channel+1);
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "IIC init";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }else{
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf),"port %d init fail",i2c_dev.i2c_channel+1);
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "IIC init";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }
    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);
    if (0 != ret.errcode)
    {
        ret.msg = "i2c_init: fail";
    }
    return ret;
}

static ret_msg_t i2c_deinit_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    arg_t out_arg[1] = {0};
    char buf[64] = {0};
    SC_I2C_ReturnCode i2c_ret = SC_I2C_RC_NOT_OK;
    SC_I2C_DEV i2c_dev = {0};
    int i2c_port = 0;
    const char *i2c_port_input = NULL;

    pl_demo_get_data(&i2c_port_input, argv, argn, I2C_port);

    if (!i2c_port_input)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    if(!strncmp(i2c_port_input,"i2c1",4))
    {
        i2c_port = 1;
    }
    else if(!strncmp(i2c_port_input,"i2c2",4))
    {
        i2c_port = 2;
    }
    else if(!strncmp(i2c_port_input,"i2c3",4))
    {
        i2c_port = 3;
    }

    switch(i2c_port)
    {
        case 1:
            i2c_dev.i2c_channel = SC_I2C_CHANNEL0;
            break;
        case 2:
            i2c_dev.i2c_channel = SC_I2C_CHANNEL1;
            break;
        case 3:
            i2c_dev.i2c_channel = SC_I2C_CHANNEL2;
            break;
        default:
            break;
    }
    i2c_ret = sAPI_I2CDeInit(&i2c_dev);
    if(i2c_ret == SC_I2C_RC_OK)
    {
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf),"port %d deinit ok",i2c_dev.i2c_channel+1);
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "IIC deinit";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }else{
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf),"port %d deinit fail",i2c_dev.i2c_channel+1);
    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = "IIC deinit";
    out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }
    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);
    if (0 != ret.errcode)
    {
        ret.msg = "i2c_deinit: pl_demo_post_data fail";
    }
    return ret;
}

static ret_msg_t i2c_write_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_I2C_ReturnCode i2c_ret = SC_I2C_RC_NOT_OK;
    int i2c_port = 0;
    arg_t out_arg[1] = {0};
    char buf[64] = {0};

    const char *i2c_port_input = NULL;

    pl_demo_get_data(&i2c_port_input, argv, argn, I2C_port);

    if (!i2c_port_input)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    if(!strncmp(i2c_port_input,"i2c1",4))
    {
        i2c_port = 1;
    }
    else if(!strncmp(i2c_port_input,"i2c2",4))
    {
        i2c_port = 2;
    }
    else if(!strncmp(i2c_port_input,"i2c3",4))
    {
        i2c_port = 3;
    }

    i2c_port = i2c_port - 1;

    int h_slaveaddr = 0;
    int h_slaveregaddr = 0;
    int h_slaveregdata = 0;
    int datalen = 0;
    UINT8 i2c_data=0;

    const char *r_slaveaddr = NULL;
    char slaveaddr[4] = {0};
    pl_demo_get_data(&r_slaveaddr, argv, argn, SLAVE_ADDR);
    if (!r_slaveaddr)
    {
        LOG_ERROR("r_slaveaddr Param invalid!");
        ret.errcode = ERRCODE_DEFAULT;
        return ret;
    }
    memcpy(slaveaddr, r_slaveaddr, strlen(r_slaveaddr));
    const char *r_slaveregaddr = NULL;
    char slaveregaddr[4] = { 0 };
    pl_demo_get_data(&r_slaveregaddr, argv, argn, REG_ADDR);
    if (!r_slaveregaddr)
    {
        LOG_ERROR("r_slaveregaddr Param invalid!");
        ret.errcode = ERRCODE_DEFAULT;
        return ret;
    }
    memcpy(slaveregaddr, r_slaveregaddr, strlen(r_slaveregaddr));

    const char *r_slaveregdata = NULL;
    char slaveregdata[4] = { 0 };
    pl_demo_get_data(&r_slaveregdata, argv, argn, REG_DATA);
    if (!r_slaveregdata)
    {
        LOG_ERROR("r_slaveregdata Param invalid!");
        ret.errcode = ERRCODE_DEFAULT;
        return ret;
    }
    memcpy(slaveregdata, r_slaveregdata, strlen(r_slaveregdata));


    pl_demo_get_value(&datalen, argv, argn, DATA_LEN);

    h_slaveaddr = trans(slaveaddr);
    h_slaveregaddr = trans(slaveregaddr);
    h_slaveregdata = trans(slaveregdata);
    i2c_data = h_slaveregdata;

    /* *****************only for nau8810 *********************/
    // regAddr = 0x41;
    // regData = 0x01a5;
    // i2cdata[0] = ((regAddr << 1) & 0xFE) | ((regData >> 8) & 0x01);
    // i2cdata[1] = (UINT8)(regData & 0xFF);
    /******************************************************/
    i2c_ret = sAPI_I2CWrite(i2c_port, h_slaveaddr, h_slaveregaddr, &(i2c_data), datalen);
    if (SC_I2C_RC_OK == i2c_ret){
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf)," port%d reg[0x%x]:0x%x\r\n", i2c_port+1,h_slaveregaddr, h_slaveregdata);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "IIC Write";
        out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }
    else{
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf)," port%d fail", i2c_port);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "IIC Write";
        out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);
    if (0 != ret.errcode)
    {
        ret.msg = "i2c_write: pl_demo_post_data fail";
    }
    return ret;
}

static ret_msg_t i2c_read_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_I2C_ReturnCode i2c_ret = SC_I2C_RC_NOT_OK;
    int i2c_port = 0;
    arg_t out_arg[1] = {0};
    char buf[64] = {0};
    const char *i2c_port_input = NULL;

    pl_demo_get_data(&i2c_port_input, argv, argn, I2C_port);

    if (!i2c_port_input)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    if(!strncmp(i2c_port_input,"i2c1",4))
    {
        i2c_port = 1;
    }
    else if(!strncmp(i2c_port_input,"i2c2",4))
    {
        i2c_port = 2;
    }
    else if(!strncmp(i2c_port_input,"i2c3",4))
    {
        i2c_port = 3;
    }

    i2c_port = i2c_port - 1;

    int h_slaveaddr = 0;
    int h_slaveregaddr = 0;
    int datalen = 0;

    const char *r_slaveaddr = NULL;
    char slaveaddr[4] = {0};
    pl_demo_get_data(&r_slaveaddr, argv, argn, SLAVE_ADDR);
    if (!r_slaveaddr)
    {
        LOG_ERROR("r_slaveaddr Param invalid!");
        ret.errcode = ERRCODE_DEFAULT;
        return ret;
    }
    memcpy(slaveaddr, r_slaveaddr, strlen(r_slaveaddr));
    const char *r_slaveregaddr = NULL;
    char slaveregaddr[4] = { 0 };
    pl_demo_get_data(&r_slaveregaddr, argv, argn, REG_ADDR);
    if (!r_slaveregaddr)
    {
        LOG_ERROR("r_slaveregaddr Param invalid!");
        ret.errcode = ERRCODE_DEFAULT;
        return ret;
    }
    memcpy(slaveregaddr, r_slaveregaddr, strlen(r_slaveregaddr));

    pl_demo_get_value(&datalen, argv, argn, DATA_LEN);

    h_slaveaddr = trans(slaveaddr);
    h_slaveregaddr = trans(slaveregaddr);
    /* *****************only for nau8810 *********************/
    //  regAddr = 0x41;
    //  regData = 0x00;
    /******************************************************/
    i2c_ret = sAPI_I2CRead(i2c_port, h_slaveaddr, h_slaveregaddr, r_i2cdata, datalen);
    if(datalen == 2){
    regData = (((UINT16)r_i2cdata[0]) << 8) | r_i2cdata[1];
    }else if(datalen == 1)
    {
        regData = r_i2cdata[0];
    }
    if (SC_I2C_RC_OK == i2c_ret){
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf)," port%d reg[0x%x]:0x%x\r\n", i2c_port+1,h_slaveregaddr, regData);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "IIC Read";
        out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }
    else{
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf)," port%d fail", i2c_port);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "IIC Read";
        out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);
    if (0 != ret.errcode)
    {
        ret.msg = "i2c_read: pl_demo_post_data fail";
    }
    return ret;
}

static ret_msg_t i2c_writeex_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_I2C_ReturnCode i2c_ret = SC_I2C_RC_NOT_OK;
    int i2c_port = 0;
    arg_t out_arg[1] = {0};
    char buf[64] = {0};
    const char *i2c_port_input = NULL;

    pl_demo_get_data(&i2c_port_input, argv, argn, I2C_port);

    if (!i2c_port_input)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    if(!strncmp(i2c_port_input,"i2c1",4))
    {
        i2c_port = 1;
    }
    else if(!strncmp(i2c_port_input,"i2c2",4))
    {
        i2c_port = 2;
    }
    else if(!strncmp(i2c_port_input,"i2c3",4))
    {
        i2c_port = 3;
    }

    i2c_port = i2c_port - 1;

    int h_slaveaddr = 0;
    int h_slaveregaddr = 0;
    int h_slaveregdata = 0;
    int datalen = 0;
    UINT8 i2c_data=0;

    const char *r_slaveaddr = NULL;
    char slaveaddr[4] = {0};
    pl_demo_get_data(&r_slaveaddr, argv, argn, SLAVE_ADDR);
    if (!r_slaveaddr)
    {
        LOG_ERROR("filename Param invalid!");
        ret.errcode = ERRCODE_DEFAULT;
        return ret;
    }
    memcpy(slaveaddr, r_slaveaddr, strlen(r_slaveaddr));
    const char *r_slaveregaddr = NULL;
    char slaveregaddr[4] = { 0 };
    pl_demo_get_data(&r_slaveregaddr, argv, argn, REG_ADDR);
    if (!r_slaveregaddr)
    {
        LOG_ERROR("filename Param invalid!");
        ret.errcode = ERRCODE_DEFAULT;
        return ret;
    }
    memcpy(slaveregaddr, r_slaveregaddr, strlen(r_slaveregaddr));

    const char *r_slaveregdata = NULL;
    char slaveregdata[4] = { 0 };
    pl_demo_get_data(&r_slaveregdata, argv, argn, REG_DATA);
    if (!r_slaveregdata)
    {
        LOG_ERROR("filename Param invalid!");
        ret.errcode = ERRCODE_DEFAULT;
        return ret;
    }
    memcpy(slaveregdata, r_slaveregdata, strlen(r_slaveregdata));


    pl_demo_get_value(&datalen, argv, argn, DATA_LEN);

    h_slaveaddr = trans(slaveaddr);
    h_slaveregaddr = trans(slaveregaddr);
    h_slaveregdata = trans(slaveregdata);
    i2c_data = h_slaveregdata;
    /* *****************only for nau8810 *********************/
    // regAddr = 0x41;
    // regData = 0x01a5;
    // i2cdata[0] = ((regAddr << 1) & 0xFE) | ((regData >> 8) & 0x01);
    // i2cdata[1] = (UINT8)(regData & 0xFF);
    /******************************************************/
    i2c_ret = sAPI_I2CWriteEx(i2c_port, h_slaveaddr, h_slaveregaddr, &(i2c_data), 1);
    if (SC_I2C_RC_OK == i2c_ret){
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf)," port%d reg[0x%x]:0x%x\r\n", i2c_port+1,h_slaveregaddr, i2c_data);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "IIC WriteEx";
        out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }
    else{
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf)," port%d fail", i2c_port);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "IIC WriteEx";
        out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);
    if (0 != ret.errcode)
    {
        ret.msg = "i2c_writeEx: pl_demo_post_data fail";
    }
    return ret;
}
static ret_msg_t i2c_readex_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_I2C_ReturnCode i2c_ret = SC_I2C_RC_NOT_OK;
    int i2c_port = 0;
    arg_t out_arg[1] = {0};
    char buf[64] = {0};
    const char *i2c_port_input = NULL;

    pl_demo_get_data(&i2c_port_input, argv, argn, I2C_port);

    if (!i2c_port_input)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    if(!strncmp(i2c_port_input,"i2c1",4))
    {
        i2c_port = 1;
    }
    else if(!strncmp(i2c_port_input,"i2c2",4))
    {
        i2c_port = 2;
    }
    else if(!strncmp(i2c_port_input,"i2c3",4))
    {
        i2c_port = 3;
    }

    i2c_port = i2c_port - 1;

    int h_slaveaddr = 0;
    int h_slaveregaddr = 0;
    int datalen = 0;

    const char *r_slaveaddr = NULL;
    char slaveaddr[4] = {0};
    pl_demo_get_data(&r_slaveaddr, argv, argn, SLAVE_ADDR);
    if (!r_slaveaddr)
    {
        LOG_ERROR("filename Param invalid!");
        ret.errcode = ERRCODE_DEFAULT;
        return ret;
    }
    memcpy(slaveaddr, r_slaveaddr, strlen(r_slaveaddr));
    const char *r_slaveregaddr = NULL;
    char slaveregaddr[4] = { 0 };
    pl_demo_get_data(&r_slaveregaddr, argv, argn, REG_ADDR);
    if (!r_slaveregaddr)
    {
        LOG_ERROR("filename Param invalid!");
        ret.errcode = ERRCODE_DEFAULT;
        return ret;
    }
    memcpy(slaveregaddr, r_slaveregaddr, strlen(r_slaveregaddr));

    pl_demo_get_value(&datalen, argv, argn, DATA_LEN);

    h_slaveaddr = trans(slaveaddr);
    h_slaveregaddr = trans(slaveregaddr);
    /* *****************only for nau8810 *********************/
    // regAddr = 0x41;
    // regData = 0x00;
    /******************************************************/
    i2c_ret = sAPI_I2CReadEx(i2c_port, h_slaveaddr, h_slaveregaddr, r_i2cdata, datalen);
    if(datalen == 2){
    regData = (((UINT16)r_i2cdata[0]) << 8) | r_i2cdata[1];
    }else if(datalen == 1)
    {
        regData = r_i2cdata[0];
    }
    if (SC_I2C_RC_OK == i2c_ret){
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf)," port%d reg[0x%x]:0x%x\r\n", i2c_port+1,h_slaveregaddr, regData);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "IIC ReadEx";
        out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }
    else{
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf)," port%d fail", i2c_port);
        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = "IIC ReadEx";
        out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));
    }

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    pl_demo_release_value(out_arg[0].value);
    if (0 != ret.errcode)
    {
        ret.msg = "i2c_readEx: pl_demo_post_data fail";
    }
    return ret;
}
