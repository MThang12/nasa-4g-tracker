#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "demo_network.h"
#include "simcom_network.h"
#include "string.h"
#include "simcom_os.h"
#include "simcom_simcard.h"
#include "simcom_tcpip.h"




#define LOG_ERROR(...) sal_log_error("[DEMO-NETWORK] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-NETWORK] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-NETWORK] " __VA_ARGS__)

typedef struct
{
    int mode;
    int format;
    char oper[20];
    int act;
} SCcopsParm;

typedef struct
{
    int cid;
    char pdptype[20];
    char apn[20];
} SCcgdcontParm;


static ret_msg_t network_init_func();
static ret_msg_t get_csq_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_creg_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_cgreg_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_cpsi_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_cfun_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_cfun_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_cgatt_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_cgatt_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_cgact_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_cgact_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_cgdcont_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_cgdcont_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_cgauth_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_cgauth_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_cops_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_cops_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_ipaddr_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_cnmp_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_cnmp_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_cnetci_cmd_proc(op_t op, arg_t *argv, int argn);
#if 0
static ret_msg_t test_gprs_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t test_network_mode_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t test_multiple_dialing_cmd_proc(op_t op, arg_t *argv, int argn);
#endif
#ifdef PLATFORM_160X
static ret_msg_t get_cclk_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_ctzu_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_ctzu_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t get_dns_addr_proc(op_t op, arg_t *argv, int argn);
#if 0
static ret_msg_t get_realtime_cmd_proc(op_t op, arg_t *argv, int argn);
#endif
static ret_msg_t enable_1bis_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_t3324_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t enable_HardWare_psm_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t set_t3412_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t close_psm_cmd_proc(op_t op, arg_t *argv, int argn);
#endif


const menu_list_t cfun_menu;
const menu_list_t cgatt_menu;
const menu_list_t cgact_menu;
const menu_list_t cgdcont_menu;
const menu_list_t cops_menu;
const menu_list_t cgpaddr_menu;
const menu_list_t cnmp_menu;
const menu_list_t cclk_menu;
const menu_list_t psm_menu;


const menu_list_t func_get_cfun =
{
    .menu_name = "GET CFUN",
    .menu_type = TYPE_FUNC,
    .parent = &cfun_menu,
    .function = {
        .methods.cmd_handler = get_cfun_cmd_proc,
    },
};

const value_t set_cfun_value[3] =
{
    {
        .val = 0,
    },

    {
        .val = 1,
    },

    {
        .val = 4,
    },
};

const value_list_t set_cfun_range =
{
    .isrange = false,
    .list_head = set_cfun_value,
    .count = 3,
};

arg_t set_cfun_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_SETCFUN,
        .value_range = &set_cfun_range,
    }
};

const menu_list_t func_set_cfun =
{
    .menu_name = "SET CFUN",
    .menu_type = TYPE_FUNC,
    .parent = &cfun_menu,
    .function = {
        .argv = set_cfun_args,
        .argn = sizeof(set_cfun_args) / sizeof(arg_t),
        .methods.cmd_handler = set_cfun_cmd_proc,
    },
};

const menu_list_t *cfun_sub_menu[]=
{
    &func_get_cfun,
    &func_set_cfun,
    NULL
};

const menu_list_t cfun_menu =
{
    .menu_name = "phone functionality(CFUN)",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = cfun_sub_menu,
    .parent = &network_menu,
};

const value_t set_cgatt_state_value[2] =
{
    {
        .bufp = ARG_CGATT_DETACH,
        .size = sizeof(ARG_CGATT_DETACH) - 1,
    },

    {
        .bufp = ARG_CGATT_ATTACH,
        .size = sizeof(ARG_CGATT_ATTACH) - 1,
    },
};

const value_list_t cgatt_state_range =
{
    .isrange = false,
    .list_head = set_cgatt_state_value,
    .count = 2,
};


arg_t set_cgatt_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_CGATT_STATE,
        .value_range = &cgatt_state_range,
    }
};

const menu_list_t func_get_cgatt =
{
    .menu_name = "GET CGATT",
    .menu_type = TYPE_FUNC,
    .parent = &cgatt_menu,
    .function = {
        .methods.cmd_handler = get_cgatt_cmd_proc,
    },
};

const menu_list_t func_set_cgatt =
{
    .menu_name = "SET CGATT",
    .menu_type = TYPE_FUNC,
    .parent = &cgatt_menu,
    .function = {
        .argv = set_cgatt_args,
        .argn = sizeof(set_cgatt_args) / sizeof(arg_t),
        .methods.cmd_handler = set_cgatt_cmd_proc,
    },
};

const menu_list_t *cgatt_sub_menu[] =
{
    &func_get_cgatt,
    &func_set_cgatt,
    NULL
};

const menu_list_t cgatt_menu =
{
    .menu_name = "attach or detach(CGATT)",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = cgatt_sub_menu,
    .parent = &network_menu,
};

const value_list_t cid_range =
{
    .isrange = true,
    .min = 1,
    .max = 16,
};

const value_t set_cgact_state_value[2] =
{
    {
        .bufp = ARG_CGACT_DEACT,
        .size = sizeof(ARG_CGACT_DEACT) - 1,
    },

    {
        .bufp = ARG_CGACT_ACT,
        .size = sizeof(ARG_CGACT_ACT) - 1,
    },
};

const value_list_t cgact_state_range =
{
    .isrange = false,
    .list_head = set_cgact_state_value,
    .count = 2,
};


arg_t set_cgact_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_CID,
        .value_range = &cid_range,
        .msg = "please input CID(1-16)"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_CGACTSTATE,
        .value_range = &cgact_state_range,
        .msg = "please input state(0,1)"
    }
};

const menu_list_t func_get_cgact =
{
    .menu_name = "GET CGACT",
    .menu_type = TYPE_FUNC,
    .parent = &cgact_menu,
    .function = {
        .methods.cmd_handler = get_cgact_cmd_proc,
    },
};

const menu_list_t func_set_cgact =
{
    .menu_name = "SET CGACT",
    .menu_type = TYPE_FUNC,
    .parent = &cgact_menu,
    .function = {
        .argv = set_cgact_args,
        .argn = sizeof(set_cgact_args)/sizeof(arg_t),
        .methods.cmd_handler = set_cgact_cmd_proc,
    },
};

const menu_list_t *cgact_sub_menu[]=
{
    &func_get_cgact,
    &func_set_cgact,
    NULL
};

const menu_list_t cgact_menu =
{
    .menu_name = "act or deact(CGACT)",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = cgact_sub_menu,
    .parent = &network_menu,
};

const value_t set_pdptype_value[3] =
{
    {
        .bufp = ARG_IP,
        .size = sizeof(ARG_IP) - 1,
    },

    {
        .bufp = ARG_IPV6,
        .size = sizeof(ARG_IPV6) - 1,
    },

    {
        .bufp = ARG_IPV4V6,
        .size = sizeof(ARG_IPV4V6) - 1,
    },
};

const value_list_t set_pdptype_range =
{
    .isrange = false,
    .list_head = set_pdptype_value,
    .count = 3,
};

arg_t set_cgdcont_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_CID,
        .value_range = &cid_range,
        .msg = "please input CID(1-16)"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_PDPTYPE,
        .value_range = &set_pdptype_range,

    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_APN,
    }
};

const value_t set_auth_type_value[3] =
{
    {
        .bufp = ARG_AUTH_TYPE_PAP,
        .size = sizeof(ARG_AUTH_TYPE_PAP) - 1,
    },

    {
        .bufp = ARG_AUTH_TYPE_CHAP,
        .size = sizeof(ARG_AUTH_TYPE_CHAP) - 1,
    },
};

const value_list_t authtype_range =
{
    .isrange = false,
    .list_head = set_auth_type_value,
    .count = 2,
};

arg_t get_cgauth_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_CID,
        .value_range = &cid_range,
        .msg = "please input CID(1-16)"
    }
};

arg_t set_cgauth_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_CID,
        .value_range = &cid_range,
        .msg = "please input CID(1-16)"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_AUTHTYPE,
        .value_range = &authtype_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_AUTHUSER,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_AUTHPASSWD,
    }
};



const menu_list_t func_get_cgdcont =
{
    .menu_name = "GET CGDCONT",
    .menu_type = TYPE_FUNC,
    .parent = &cgdcont_menu,
    .function = {
        .methods.cmd_handler = get_cgdcont_cmd_proc,
    },
};

const menu_list_t func_set_cgdcont =
{
    .menu_name = "SET CGDCONT",
    .menu_type = TYPE_FUNC,
    .parent = &cgdcont_menu,
    .function = {
        .argv = set_cgdcont_args,
        .argn = sizeof(set_cgdcont_args)/sizeof(arg_t),
        .methods.cmd_handler = set_cgdcont_cmd_proc,
    },
};

const menu_list_t func_get_cgauth =
{
    .menu_name = "GET CGAUTH",
    .menu_type = TYPE_FUNC,
    .parent = &cgdcont_menu,
    .function = {
        .argv = get_cgauth_args,
        .argn = sizeof(get_cgauth_args)/sizeof(arg_t),
        .methods.cmd_handler = get_cgauth_cmd_proc,
    },
};

const menu_list_t func_set_cgauth =
{
    .menu_name = "SET CGAUTH",
    .menu_type = TYPE_FUNC,
    .parent = &cgdcont_menu,
    .function = {
        .argv = set_cgauth_args,
        .argn = sizeof(set_cgauth_args)/sizeof(arg_t),
        .methods.cmd_handler = set_cgauth_cmd_proc,
    },
};

const menu_list_t *cgdcont_sub_menu[]=
{
    &func_get_cgdcont,
    &func_set_cgdcont,
    &func_get_cgauth,
    &func_set_cgauth,
    NULL
};

const menu_list_t cgdcont_menu =
{
    .menu_name = "PDP context(CGDCONT/CGAUTH)",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = cgdcont_sub_menu,
    .parent = &network_menu,
};

const value_list_t cops_mode_range =
{
    .isrange = true,
    .min = 0,
    .max = 4,
};

const value_list_t cops_format_range =
{
    .isrange = true,
    .min = 0,
    .max = 2,
};

const value_list_t cops_act_range =
{
    .isrange = true,
    .min = 0,
    .max = 8,
};

arg_t set_cops_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_COPSMODE,
        .value_range = &cops_mode_range,
        .msg = "mode range is 0~4"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_COPSFORMAT,
        .value_range = &cops_format_range,
        .msg = "format range is 0~2"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARG_COPSOPER,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARG_COPSACT,
        .value_range = &cops_act_range,
        .msg = "act range is 0~8"
    }
};

const menu_list_t func_get_cops =
{
    .menu_name = "GET COPS",
    .menu_type = TYPE_FUNC,
    .parent = &cops_menu,
    .function = {
        .methods.cmd_handler = get_cops_cmd_proc,
    },
};

const menu_list_t func_set_cops =
{
    .menu_name = "SET COPS",
    .menu_type = TYPE_FUNC,
    .parent = &cops_menu,
    .function = {
        .argv = set_cops_args,
        .argn = sizeof(set_cops_args)/sizeof(arg_t),
        .methods.cmd_handler = set_cops_cmd_proc,
    },
};

const menu_list_t *cops_sub_menu[]=
{
    &func_get_cops,
    &func_set_cops,
    NULL
};

const menu_list_t cops_menu =
{
    .menu_name = "Operator selection(COPS)",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = cops_sub_menu,
    .parent = &network_menu,
};

const value_list_t cgpaddr_cid_range =
{
    .isrange = true,
    .min = 1,
    .max = 15,
};

arg_t get_ipaddr_args[1] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_CID,
        .value_range = &cgpaddr_cid_range,
        .msg = "please input CID(1-15)"
    }
};

const menu_list_t func_get_IPaddr =
{
    .menu_name = "GET IPaddr",
    .menu_type = TYPE_FUNC,
    .parent = &cgpaddr_menu,
    .function = {
        .argv = get_ipaddr_args,
        .argn = sizeof(get_ipaddr_args)/sizeof(arg_t),
        .methods.cmd_handler = get_ipaddr_cmd_proc,
    },
};

const menu_list_t *cgpaddr_sub_menu[]=
{
    &func_get_IPaddr,
    NULL
};

const menu_list_t cgpaddr_menu =
{
    .menu_name = "IP Address(CGPADDR)",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = cgpaddr_sub_menu,
    .parent = &network_menu,
};

const menu_list_t func_get_cnmp =
{
    .menu_name = "GET CNMP",
    .menu_type = TYPE_FUNC,
    .parent = &cnmp_menu,
    .function = {
        .methods.cmd_handler = get_cnmp_cmd_proc,
    },
};

const value_t set_cnmp_value[3] =
{
    {
        .val = 2,
    },

    {
        .val = 13,
    },

    {
        .val = 38,
    },
};

const value_list_t set_cnmp_range =
{
    .isrange = false,
    .list_head = set_cnmp_value,
    .count = 3,
};

arg_t set_cnmp_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_SETCNMP,
        .value_range = &set_cnmp_range,
    }
};

const menu_list_t func_set_cnmp =
{
    .menu_name = "SET CNMP",
    .menu_type = TYPE_FUNC,
    .parent = &cnmp_menu,
    .function = {
        .argv = set_cnmp_args,
        .argn = sizeof(set_cnmp_args) / sizeof(arg_t),
        .methods.cmd_handler = set_cnmp_cmd_proc,
    },
};

const menu_list_t *cnmp_sub_menu[]=
{
    &func_get_cnmp,
    &func_set_cnmp,
    NULL
};

const menu_list_t cnmp_menu =
{
    .menu_name = "mode selection(CNMP)",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = cnmp_sub_menu,
    .parent = &network_menu,
};

const menu_list_t func_get_csq =
{
    .menu_name = "Query signal quality(CSQ)",
    .menu_type = TYPE_FUNC,
    .parent = &network_menu,
    .function = {
        .methods.cmd_handler = get_csq_cmd_proc,
    },
};

const menu_list_t func_get_creg =
{
    .menu_name = "CS domain(CREG)",
    .menu_type = TYPE_FUNC,
    .parent = &network_menu,
    .function = {
        .methods.cmd_handler = get_creg_cmd_proc,
    },
};

const menu_list_t func_get_cgreg =
{
    .menu_name = "PS domain(CGREG)",
    .menu_type = TYPE_FUNC,
    .parent = &network_menu,
    .function = {
        .methods.cmd_handler = get_cgreg_cmd_proc,
    },
};

const menu_list_t func_get_cpsi =
{
    .menu_name = "system information(CPSI)",
    .menu_type = TYPE_FUNC,
    .parent = &network_menu,
    .function = {
        .methods.cmd_handler = get_cpsi_cmd_proc,
    },
};

const menu_list_t func_get_cnetci =
{
    .menu_name = "Adjacent NB information(CNETCI)",
    .menu_type = TYPE_FUNC,
    .parent = &network_menu,
    .function = {
        .methods.cmd_handler = get_cnetci_cmd_proc,
    },
};

#if 0
const menu_list_t func_test_gprs =
{
    .menu_name = "Test GPRS",
    .menu_type = TYPE_FUNC,
    .parent = &network_menu,
    .function = {
        .methods.cmd_handler = test_gprs_cmd_proc,
    },
};

const menu_list_t func_test_network_mode =
{
    .menu_name = "Test NETWORK Mode",
    .menu_type = TYPE_FUNC,
    .parent = &network_menu,
    .function = {
        .methods.cmd_handler = test_network_mode_cmd_proc,
    },
};


const menu_list_t func_test_multiple_dialing =
{
    .menu_name = "Test Multiple dialing",
    .menu_type = TYPE_FUNC,
    .parent = &network_menu,
    .function = {
        .methods.cmd_handler = test_multiple_dialing_cmd_proc,
    },
};
#endif

#ifdef PLATFORM_160X

const value_t set_ctzu_value[2] =
{
    {
        .bufp = ARG_CTZU_OFF,
        .size = sizeof(ARG_CTZU_OFF) - 1,
    },

    {
        .bufp = ARG_CTZU_ON,
        .size = sizeof(ARG_CTZU_ON) - 1,
    },
};

const value_list_t set_ctzu_range =
{
    .isrange = false,
    .list_head = set_ctzu_value,
    .count = 2,
};

arg_t set_ctzu_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_SETCTZU,
        .value_range = &set_ctzu_range,
    }
};

const menu_list_t func_get_cclk =
{
    .menu_name = "GET CCLK",
    .menu_type = TYPE_FUNC,
    .parent = &cclk_menu,
    .function = {
        .methods.cmd_handler = get_cclk_cmd_proc,
    },
};

const menu_list_t func_get_ctzu =
{
    .menu_name = "GET CTZU",
    .menu_type = TYPE_FUNC,
    .parent = &cclk_menu,
    .function = {
        .methods.cmd_handler = get_ctzu_cmd_proc,
    },
};

const menu_list_t func_set_ctzu =
{
    .menu_name = "SET CTZU",
    .menu_type = TYPE_FUNC,
    .parent = &cclk_menu,
    .function = {
        .argv = set_ctzu_args,
        .argn = sizeof(set_ctzu_args) / sizeof(arg_t),
        .methods.cmd_handler = set_ctzu_cmd_proc,
    },
};

#if 0
const menu_list_t func_get_realtime =
{
    .menu_name = "GET RealTime",
    .menu_type = TYPE_FUNC,
    .parent = &cclk_menu,
    .function = {
        .methods.cmd_handler = get_realtime_cmd_proc,
    },
};
#endif

const menu_list_t *cclk_sub_menu[]=
{
    &func_get_cclk,
    &func_get_ctzu,
    &func_set_ctzu,
//    &func_get_realtime,
    NULL
};

const menu_list_t cclk_menu =
{
    .menu_name = "Test RealTime(CCLK)",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = cclk_sub_menu,
    .parent = &network_menu,
};


arg_t get_dnsaddr_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_CID,
        .value_range = &cid_range,
    }
};


const value_t set_t3324_value[2] =
{
    {
        .bufp = ARG_T3324_MODE_DISABLE,
        .size = sizeof(ARG_T3324_MODE_DISABLE) - 1,
    },

    {
        .bufp = ARG_T3324_MODE_ENABLE,
        .size = sizeof(ARG_T3324_MODE_ENABLE) - 1,
    },
};

const value_list_t set_t3324_range =
{
    .isrange = false,
    .list_head = set_t3324_value,
    .count = 2,
};

arg_t set_t3324_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARG_T3324MODE,
        .value_range = &set_t3324_range,
    },

    {
        .type = TYPE_STR,
        .arg_name = ARG_T3324TIME,
        .msg = "please enter binary number",
    }
};

const value_list_t set_t3412_range =
{
    .isrange = true,
    .min = 1,
    .max = 255,
};

arg_t set_t3412_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARG_T3412TIME,
        .value_range = &set_t3412_range,
    }
};


const menu_list_t func_enable_1bis =
{
    .menu_name = "Enable 1Bis",
    .menu_type = TYPE_FUNC,
    .parent = &psm_menu,
    .function = {
        .methods.cmd_handler = enable_1bis_cmd_proc,
    },
};

const menu_list_t func_set_T3324 =
{
    .menu_name = "Set T3324",
    .menu_type = TYPE_FUNC,
    .parent = &psm_menu,
    .function = {
        .argv = set_t3324_args,
        .argn = sizeof(set_t3324_args) / sizeof(arg_t),
        .methods.cmd_handler = set_t3324_cmd_proc,
    },
};

const menu_list_t func_enable_hardware_psm =
{
    .menu_name = "Enable HardWare PSM",
    .menu_type = TYPE_FUNC,
    .parent = &psm_menu,
    .function = {
        .methods.cmd_handler = enable_HardWare_psm_cmd_proc,
    },
};

const menu_list_t func_set_T3412 =
{
    .menu_name = "Set T3412",
    .menu_type = TYPE_FUNC,
    .parent = &psm_menu,
    .function = {
        .argv = set_t3412_args,
        .argn = sizeof(set_t3412_args) / sizeof(arg_t),
        .methods.cmd_handler = set_t3412_cmd_proc,
    },
};

const menu_list_t func_close_psm =
{
    .menu_name = "Close PSM",
    .menu_type = TYPE_FUNC,
    .parent = &psm_menu,
    .function = {
        .methods.cmd_handler = close_psm_cmd_proc,
    },
};

const menu_list_t *psm_sub_menu[]=
{
    &func_enable_1bis,
    &func_set_T3324,
    &func_enable_hardware_psm,
    &func_set_T3412,
    &func_close_psm,
    NULL
};

const menu_list_t psm_menu =
{
    .menu_name = "Test PSM",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = psm_sub_menu,
    .parent = &network_menu,
};

const menu_list_t func_get_dns_addr =
{
    .menu_name = "Get DNS Addr",
    .menu_type = TYPE_FUNC,
    .parent = &network_menu,
    .function = {
        .argv = get_dnsaddr_args,
        .argn = sizeof(get_dnsaddr_args) / sizeof(arg_t),
        .methods.cmd_handler = get_dns_addr_proc,
    },
};
#endif

const menu_list_t *network_sub_menu[]=
{
    &func_get_csq,
    &func_get_creg,
    &func_get_cgreg,
    &func_get_cpsi,
    &cnmp_menu,
    &cops_menu,
    &cgdcont_menu,
    &cgact_menu,
    &cgatt_menu,
    &cfun_menu,
    &cgpaddr_menu,
    &func_get_cnetci,
    &func_get_dns_addr,
/*
    &func_test_gprs,
    &func_test_network_mode,
    &func_test_multiple_dialing,
*/
    &cclk_menu,
    &psm_menu,
    NULL
};

const menu_list_t network_menu =
{
    .menu_name = "network",
    .menu_type = TYPE_MENU,
    .__init = network_init_func,
    .__uninit = NULL,
    .submenu_list_head = network_sub_menu,
    .parent = &root_menu,
};

static ret_msg_t post_network_result(char* name, type_t type, char *buf)
{
	ret_msg_t ret = {0};
	arg_t out_arg[1] = {0};

	out_arg[0].arg_name = name;
	out_arg[0].type = type;
	out_arg[0].value = pl_demo_make_value(0, buf, strlen(buf));

	ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

	pl_demo_release_value(out_arg[0].value);

	return ret;
}


static ret_msg_t network_init_func()
{
	ret_msg_t ret = {0};
	sAPI_NetworkInit();
	return ret;
}


static ret_msg_t get_csq_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    UINT8 csq;
    char NetResp[1000] = {0};
    ret.errcode = sAPI_NetworkGetCsq(&csq);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Get csq success. csq=%d!", csq);
        sprintf(NetResp, "Get csq success. csq:%d!", csq);
    }
    else
    {
        LOG_ERROR("Get csq falied!");
        sprintf(NetResp, "Get csq falied!");
    }
    post_network_result(name, type, NetResp);

    return ret;
}

static ret_msg_t get_creg_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    int creg;
    char NetResp[1000] = {0};
    ret.errcode = sAPI_NetworkGetCreg(&creg);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Get creg success. creg=%d!", creg);
        sprintf(NetResp, "Get creg success. creg=%d!", creg);
    }
    else
    {
        LOG_ERROR("Get creg falied!");
        sprintf(NetResp, "Get creg falied!");
    }
    post_network_result(name, type, NetResp);

    return ret;
}

static ret_msg_t get_cgreg_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    int cgreg;
    char NetResp[1000] = {0};
    ret.errcode = sAPI_NetworkGetCgreg(&cgreg);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Get cgreg success. cgreg=%d!", cgreg);
        sprintf(NetResp, "Get cgreg success. cgreg=%d!", cgreg);
    }
    else
    {
        LOG_ERROR("Get cgreg falied!");
        sprintf(NetResp, "Get cgreg falied!");
    }
    post_network_result(name, type, NetResp);

    return ret;
}

static ret_msg_t get_cpsi_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_RAW;
    SCcpsiParm Scpsi = { 0 };
    char NetResp[1000] = {0};
    ret.errcode = sAPI_NetworkGetCpsi(&Scpsi);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Get cpsi success. NEmode=%s,MM=%s,LAC=%d,CELL=%d,Gband=%s,Lband=%s,TAC=%d,RSRP=%d,RXLEV=%d,TA=%d!",
                   Scpsi.networkmode, Scpsi.Mnc_Mcc, Scpsi.LAC, Scpsi.CellID, Scpsi.GSMBandStr, Scpsi.LTEBandStr, Scpsi.TAC, Scpsi.Rsrp,
                   Scpsi.RXLEV, Scpsi.TA);
        sprintf(NetResp,"Get cpsi success. NEmode=%s,MM=%s,LAC=%d,CELL=%d,Gband=%s,Lband=%s,TAC=%d,RSRP=%d,RXLEV=%d,TA=%d,SINR=%d!",
                Scpsi.networkmode, Scpsi.Mnc_Mcc, Scpsi.LAC, Scpsi.CellID, Scpsi.GSMBandStr, Scpsi.LTEBandStr, Scpsi.TAC, Scpsi.Rsrp,
                Scpsi.RXLEV, Scpsi.TA, Scpsi.SINR);
    }
    else
    {
        LOG_ERROR("Get cpsi falied!");
        sprintf(NetResp, "Get cpsi falied!");
    }
    post_network_result(name, type, NetResp);

    return ret;
}


static ret_msg_t get_cfun_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    UINT8 cfun;
    char getCfunResp[50]= {0};
    ret.errcode = sAPI_NetworkGetCfun(&cfun);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Get cfun success. cfun=%d!", cfun);
        sprintf(getCfunResp, "Get Cfun success. Cfun=%d!", cfun);
    }
    else
    {
        LOG_ERROR("Get cfun falied!");
        sprintf(getCfunResp, "Get cfun falied!");
    }
    post_network_result(name, type, getCfunResp);

    return ret;
}

static ret_msg_t set_cfun_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    char setCfunResp[50] = {0};
    int cfun;
    pl_demo_get_value(&cfun, argv, argn, ARG_SETCFUN);
    ret.errcode = sAPI_NetworkSetCfun(cfun);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Set cfun %d success.!",cfun);
        sprintf(setCfunResp, "Set cfun %d success.!",cfun);
    }
    else
    {
        LOG_ERROR("Set cfun falied!");
        sprintf(setCfunResp, "Set cfun falied!");
    }

    post_network_result(name, type, setCfunResp);

    return ret;
}

static ret_msg_t get_cgatt_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_RAW;
    char cgattResp[50] = {0};
    int cgatt;
    ret.errcode = sAPI_NetworkGetCgatt(&cgatt);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Get Cgatt success. Cgatt=%d!", cgatt);
        sprintf(cgattResp, "Get cgatt success. Cgatt=%d!", cgatt);
    }
    else
    {
        LOG_ERROR("Get cgatt falied!");
        sprintf(cgattResp, "Get cgatt falied!");
    }
    post_network_result(name, type, cgattResp);

    return ret;
}

static ret_msg_t set_cgatt_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    char setCgattResp[50] = {0};
    const char* cgattStateStr = NULL;
    int cgattState;
    pl_demo_get_data(&cgattStateStr, argv, argn, ARG_CGATT_STATE);
    if(strncmp(cgattStateStr,ARG_CGATT_DETACH,sizeof(ARG_CGATT_DETACH)-1) == 0)
        cgattState = 0;
    else
        cgattState = 1;
    ret.errcode = sAPI_NetworkSetCgatt(cgattState);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Set cgatt success.!");
        sprintf(setCgattResp, "Set cgatt success!");
    }
    else
    {
        LOG_ERROR("Set cgatt falied!");
        sprintf(setCgattResp, "Set cgatt falied!");
    }
    post_network_result(name, type, setCgattResp);

    return ret;
}

static ret_msg_t get_cgact_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_RAW;
    int i;
    SCAPNact SCact[8] = {0};
    char cgactresp[100] = {0};
    ret.errcode = sAPI_NetworkGetCgact(SCact);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        for (i = 0; i < 8; i++) /*Read it eight times*/
        {
            LOG_INFO("Get cgact success. cid=%d,actstate=%d!", SCact[i].cid, SCact[i].isActived);
            if (SCact[i].isdefine)
            {
                sprintf(cgactresp, "cid=%d, actstate=%d", SCact[i].cid, SCact[i].isActived);
                post_network_result(name, type, cgactresp);
            }
        }
    }
    else
    {
        LOG_ERROR("Get cgact falied!");
        sprintf(cgactresp, "Get cgact falied!");
        post_network_result(name, type, cgactresp);
    }


    return ret;
}

static ret_msg_t set_cgact_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    int cgactcid = 0XFF;
    int cgactstate = 0xFF;
    const char* statestr = NULL;
    char cgactresp[100] = {0};
    pl_demo_get_value(&cgactcid, argv, argn, ARG_CID);
    pl_demo_get_data(&statestr, argv, argn, ARG_CGACTSTATE);
    if(strncmp(statestr,ARG_CGACT_ACT,sizeof(ARG_CGACT_ACT)-1) == 0)
        cgactstate = 1;
    else
        cgactstate = 0;
    ret.errcode = sAPI_NetworkSetCgact(cgactstate, cgactcid);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Set cgact success.!");
        sprintf(cgactresp, "Set cgact success.!");
    }
    else
    {
        LOG_ERROR("Set cgact falied!");
        sprintf(cgactresp, "Set cgact falied!");
    }

    post_network_result(name, type, cgactresp);

    return ret;
}

static ret_msg_t get_cgdcont_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_RAW;
    SCApnParm SCapn[8] = {0};
    char cgdcontresp[100] = {0};
    int i;
    ret.errcode = sAPI_NetworkGetCgdcont(SCapn);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        for (i = 0; i < 8; i++) /*Read it eight times*/
        {
            LOG_INFO("Get Cgdcont success. cid=%d,iptype=%d,apnstr=%s!", SCapn[i].cid, SCapn[i].iptype, SCapn[i].ApnStr);
            if (SCapn[i].iptype) /*Iptype 0 means undefined*/
            {
                sprintf(cgdcontresp, "cid=%d,iptype=%d,apnstr=%s", SCapn[i].cid, SCapn[i].iptype, SCapn[i].ApnStr);
                post_network_result(name, type, cgdcontresp);
            }
        }
    }
    else
    {
        LOG_ERROR("Get cgdcont falied!");
        sprintf(cgdcontresp, "Get cgdcont falied!");
        post_network_result(name, type, cgdcontresp);
    }

    return ret;
}

static ret_msg_t set_cgdcont_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    int cgdcontcid = 0XFF;
    const char *pdptype = NULL;
    const char *APN = NULL;
    char cgdcontresp[100] = {0};
    SCcgdcontParm cgdcontParm = {0, {0}, {0}};
    pl_demo_get_value(&cgdcontcid, argv, argn, ARG_CID);
    pl_demo_get_data(&pdptype, argv, argn, ARG_PDPTYPE);
    pl_demo_get_data(&APN, argv, argn, ARG_APN);
    cgdcontParm.cid = cgdcontcid;
    if(strstr(pdptype, "\r\n") != NULL)
        memcpy(cgdcontParm.pdptype, pdptype, strlen(pdptype) -2);
    else
        memcpy(cgdcontParm.pdptype, pdptype, strlen(pdptype));
    if(strstr(APN, "\r\n") != NULL)
        memcpy(cgdcontParm.apn, APN, strlen(APN) -2);
    else
        memcpy(cgdcontParm.apn, APN, strlen(APN));
    LOG_INFO("Set cgdcont:%d   %s   %s", cgdcontParm.cid, cgdcontParm.pdptype, cgdcontParm.apn);
    ret.errcode = sAPI_NetworkSetCgdcont(cgdcontParm.cid, (char *)cgdcontParm.pdptype, (char *)cgdcontParm.apn);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Set cgdcont success.!");
        sprintf(cgdcontresp, "Set cgdcont success.!");
    }
    else
    {
        LOG_ERROR("Set cgdcont falied!");
        sprintf(cgdcontresp, "Set cgdcont falied!");
    }

    post_network_result(name, type, cgdcontresp);

    return ret;
}

static ret_msg_t get_cgauth_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_RAW;
    int cgauthcid = 0;
    char cgauthresp[150] = {0};
    SCCGAUTHParm cgauthParm = {0, 0, {0}, {0}};
    pl_demo_get_value(&cgauthcid, argv, argn, ARG_CID);
    ret.errcode = sAPI_NetworkGetCgauth(&cgauthParm, cgauthcid);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        sprintf(cgauthresp, "cid=%d,type=%d,usr=%s,passwd=%s", cgauthParm.cid, cgauthParm.authtype, cgauthParm.user,
                cgauthParm.passwd);
    }
    else
    {
        sprintf(cgauthresp, "GET FAIL");
    }

    post_network_result(name, type, cgauthresp);

    return ret;
}

static ret_msg_t set_cgauth_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    SCCGAUTHParm auth;
    int cid;
    int authtype;
    const char *authuser = NULL;
    const char *authpasswd = NULL;
    const char *authtypestr = NULL;
    char cgauthresp[100] = {0};
    pl_demo_get_value(&cid, argv, argn, ARG_CID);
    pl_demo_get_data(&authtypestr, argv, argn, ARG_AUTHTYPE);
    pl_demo_get_data(&authuser, argv, argn, ARG_AUTHUSER);
    pl_demo_get_data(&authpasswd, argv, argn, ARG_AUTHPASSWD);
    if(strncmp(authtypestr,ARG_AUTH_TYPE_PAP,sizeof(ARG_AUTH_TYPE_PAP)-1) == 0)
        authtype = 1;
    else
        authtype = 2;
    auth.cid = cid;
    auth.authtype = authtype;
    strcpy(auth.user, authuser);
    strcpy(auth.passwd, authpasswd);
    ret.errcode = sAPI_NetworkSetCgauth(&auth,0);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Set cgauth success!");
        sprintf(cgauthresp, "Set cgauth success!");

    }
    else
    {
        LOG_ERROR("Set cgauth falied!");
        sprintf(cgauthresp, "Set cgauth falied!");
    }

    post_network_result(name, type, cgauthresp);

    return ret;
}

static ret_msg_t get_cops_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_RAW;
    char cops[200] = {0};
    char copsresp[250] = {0};
    ret.errcode = sAPI_NetworkGetCops(cops);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Get cops success. cops=%s!", cops);
        sprintf(copsresp, "Get cops success. cops=%s!", cops);

    }
    else
    {
        LOG_ERROR("Get cops falied!");
        sprintf(copsresp, "Get cops falied!");
    }

    post_network_result(name, type, copsresp);

    return ret;
}

static ret_msg_t set_cops_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    int mode;
    int format;
    const char *oper = NULL;
    int act;
    char copsresp[100] = {0};
    pl_demo_get_value(&mode, argv, argn, ARG_COPSMODE);
    pl_demo_get_value(&format, argv, argn, ARG_COPSFORMAT);
    pl_demo_get_data(&oper, argv, argn, ARG_COPSOPER);
    pl_demo_get_value(&act, argv, argn, ARG_COPSACT);
    ret.errcode = sAPI_NetworkSetCops(mode, format, (char *)oper, act);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Set cops success.!");
        sprintf(copsresp, "Set cops success!");

    }
    else
    {
        LOG_ERROR("Set cops falied!");
        sprintf(copsresp, "Set cops falied!");
    }

    post_network_result(name, type, copsresp);

    return ret;
}

static ret_msg_t get_ipaddr_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_RAW;
    SCcgpaddrParm cgpaddrParm;
    int cid = 0;
    char NetResp[200];
    pl_demo_get_value(&cid, argv, argn, ARG_CID);
    memset(&cgpaddrParm, 0, sizeof(cgpaddrParm));
    ret.errcode = sAPI_NetworkGetCgpaddr(cid, &cgpaddrParm);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Get Ipaddr success. cid=%d,type=%d,ipv4=%s,ipv6=%s!", cgpaddrParm.cid, cgpaddrParm.iptype,
                    cgpaddrParm.ipv4addr, cgpaddrParm.ipv6addr);
        sprintf(NetResp, "Get Ipaddr success. cid=%d,type=%d,ipv4=%s,ipv6=%s!", cgpaddrParm.cid, cgpaddrParm.iptype,
                cgpaddrParm.ipv4addr, cgpaddrParm.ipv6addr);
    }
    else
    {
        LOG_ERROR("Get Ipaddr falied!");
        sprintf(NetResp, "Get Ipaddr falied!");
    }

    post_network_result(name, type, NetResp);

    return ret;
}

static ret_msg_t get_cnmp_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    int cnmp;
    char cnmpresp[50] = {0};
    ret.errcode = sAPI_NetworkGetCnmp(&cnmp);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Get cnmp success. cnmp=%d!", cnmp);
        sprintf(cnmpresp, "Get cnmp success. cnmp:%d!", cnmp);
    }
    else
    {
        LOG_ERROR("Get cnmp falied!");
        sprintf(cnmpresp, "Get cnmp falied!");
    }

    post_network_result(name, type, cnmpresp);

    return ret;
}

static ret_msg_t set_cnmp_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    char cnmpresp[50] = {0};
    int cnmp;
    pl_demo_get_value(&cnmp, argv, argn, ARG_SETCNMP);
    ret.errcode = sAPI_NetworkSetCnmp(cnmp);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Set cnmp %d success!", cnmp);
        sprintf(cnmpresp, "Set cnmp %d success!", cnmp);
    }
    else
    {
        LOG_ERROR("Set cnmp %d falied!", cnmp);
        sprintf(cnmpresp, "Set cnmp %d falied!", cnmp);
    }

    post_network_result(name, type, cnmpresp);

    return ret;
}

static ret_msg_t get_cnetci_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_RAW;
#ifdef PLATFORM_160X  //104 baseline 110 baseline ...
    SCcnetciParm Snetci;
#else
    SCcnetciParm Snetci[12];
#endif
    char NetResp[1000] = {0};
#ifdef PLATFORM_160X  //104 baseline 110 baseline ...
    int i;
    ret.errcode = sAPI_NetworkGetCnetci(&Snetci);
    if (ret.errcode == SC_NET_SUCCESS)
    {
        for(i = 0; i < Snetci.scLteInterCellNum; i++)
        {
            LOG_INFO("CNETCI-INTER-CELL: %d--0X%3X-%02X cellId: %#x tac: %#X pci: %d rsrp: %d rsrq: %d earfcn: %d rssi %d", i,
                Snetci.scLteInterCell[i].mcc,Snetci.scLteInterCell[i].mnc,Snetci.scLteInterCell[i].cell_id,Snetci.scLteInterCell[i].tac,
                Snetci.scLteInterCell[i].pcid,Snetci.scLteInterCell[i].rsrp,Snetci.scLteInterCell[i].rsrq,
                Snetci.scLteInterCell[i].earfcn,Snetci.scLteInterCell[i].rssi);
            sprintf(NetResp, "Inter-Cell: %d--0X%3X-%02X cellId: %#x tac: %#X pci: %d rsrp: %d rsrq: %d earfcn: %d rssi %d", i, 
                Snetci.scLteInterCell[i].mcc,Snetci.scLteInterCell[i].mnc,Snetci.scLteInterCell[i].cell_id,Snetci.scLteInterCell[i].tac,
                Snetci.scLteInterCell[i].pcid,Snetci.scLteInterCell[i].rsrp,Snetci.scLteInterCell[i].rsrq,
                Snetci.scLteInterCell[i].earfcn,Snetci.scLteInterCell[i].rssi);
            post_network_result(name, type, NetResp);
        }
        for(i = 0; i < Snetci.scLteIntraCellNum; i++)
        {
            LOG_INFO("CNETCI-INTRA-CELL: %d--0X%3X-%02X cellId: %#x tac: %#X pci: %d rsrp: %d rsrq: %d earfcn: %d rssi %d", i,
                Snetci.scLteIntraCell[i].mcc,Snetci.scLteIntraCell[i].mnc,Snetci.scLteIntraCell[i].cell_id,Snetci.scLteIntraCell[i].tac,
                Snetci.scLteIntraCell[i].pcid,Snetci.scLteIntraCell[i].rsrp,Snetci.scLteIntraCell[i].rsrq,
                Snetci.scLteIntraCell[i].earfcn,Snetci.scLteIntraCell[i].rssi);
            sprintf(NetResp, "Intra-Cell: %d--0X%3X-%02X cellId: %#x tac: %#X pci: %d rsrp: %d rsrq: %d earfcn: %d rssi %d", i, 
                Snetci.scLteIntraCell[i].mcc,Snetci.scLteIntraCell[i].mnc,Snetci.scLteIntraCell[i].cell_id,Snetci.scLteIntraCell[i].tac,
                Snetci.scLteIntraCell[i].pcid,Snetci.scLteIntraCell[i].rsrp,Snetci.scLteIntraCell[i].rsrq,
                Snetci.scLteIntraCell[i].earfcn,Snetci.scLteIntraCell[i].rssi);
            post_network_result(name, type, NetResp);
        }
    }
    else
    {
        LOG_ERROR("Get cnetci falied!");
        sprintf(NetResp,"Get cnetci falied!");
        post_network_result(name, type, NetResp);
    }
#else // other version
    ret.errcode = sAPI_NetworkGetCnetci(Snetci);
    if (ret.errcode == SC_NET_SUCCESS)
    {
        int i = 0;
        for (i = 0; i < 12; i++) /*Read it ten times*/
        {
            LOG_INFO("Get cnetci success.i=%d:MM=%s,TAC=%d,CELL=%d,RSRP=%d,RSRQ=%d,RXSIGLEVEL=%d", i, Snetci[i].Mnc_Mcc,
                       Snetci[i].TAC, Snetci[i].CellID, Snetci[i].Rsrp, Snetci[i].Rsrq, Snetci[i].RXSIGLEVEL);
            if (Snetci[i].Rsrp || Snetci[i].RXSIGLEVEL) /*Iptype 0 means undefined*/
            {
                sprintf(NetResp, "i=%d,MM=%s,TAC=%d,CELL=%d,RSRP=%d,RSRQ=%d,RXSIGLEVEL=%d", i, Snetci[i].Mnc_Mcc, Snetci[i].TAC,
                        Snetci[i].CellID, Snetci[i].Rsrp, Snetci[i].Rsrq, Snetci[i].RXSIGLEVEL);
                post_network_result(name, type, NetResp);
            }
        }
    }
    else
    {
        LOG_INFO("Get cnetci falied!");
        sprintf(NetResp,"Get cnetci falied!");
        post_network_result(NetResp);
    }
#endif

    return ret;
}

ret_msg_t test_gprs_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    int APInum = 0;
    UINT8 cpin;
    SCcpsiParm Scpsi = { 0 };
    /*This is an example of testing the state of the network ready to use a data service,
    This is a generic example,Users can also design according to their actual needs*/
    //post_network_result(name, type, "This is a generic example,Users can also design according to their actual needs");
    while (1)
    {
        if (APInum > 20) /*You can define the number of retries you want*/
        {
            /*check card*/
            break;
        }
        APInum++;
        ret.errcode = sAPI_SimcardPinGet(&cpin);/*Power on to inquire the SIM card status*/
        if(ret.errcode == SC_NET_SUCCESS)
        {
            if(cpin == SC_SIM_READY)
            {
                post_network_result(name, type, "Get cpin success and cpin ready");
                break;
            }
            else if((cpin == SC_SIM_PIN)||(cpin == SC_SIM_PUK))
            {
                post_network_result(name, type, "Get cpin success and cpin PIN");
                sAPI_TaskSleep(200);

                /*<Call the SimCard interface>*/

                sAPI_TaskSleep(200);
                continue;/*retry*/
            }
            else
            {
                sAPI_TaskSleep(200);
                continue;/*retry*/
            }
        }
        else
        {
            sAPI_TaskSleep(200);/*All API query times are recommended once 1S*/
            post_network_result(name, type, "Get cpin falied!");
            //continue;/*If this fails, you can wait for 1S to continue the query*/
            break;
        }
    }
    while (1)
    {
        if (APInum > 60) /*You can define the number of retries you want*/
        {
            /*It is recommended to switch to cFUN or CNMP mode*/
            break;
        }
        APInum++;
        ret.errcode = sAPI_NetworkGetCpsi(&Scpsi);/*Power on to inquire the SIM card status*/
        if (ret.errcode == SC_NET_SUCCESS)
        {
            if (strncmp(Scpsi.networkmode, "No", sizeof(char) * 2) == 0)
            {
                sAPI_TaskSleep(200);
                continue;/*retry*/
            }
            if (strncmp(Scpsi.networkmode, "LTE", sizeof(char) * 3) == 0)
            {
                post_network_result(name, type, "Get cpsi success and LTE");
                break;/*You can use the TCPIP API*/
            }
            if (strncmp(Scpsi.networkmode, "GSM", sizeof(char) * 3) == 0)
            {
                post_network_result(name, type, "Get cpsi success and GSM");
                break;/*Maybe some other API has to be called*/
            }
        }
        else
        {
            sAPI_TaskSleep(200);/*All API query times are recommended once 1S*/
            post_network_result(name, type, "Get networkmode falied!");
            //continue;/*If this fails, you can wait for 1S to continue the query*/
            break;
        }
    }
    /*If you are already registered for a GSM network,You can try switching CNMP =38 and register LTE*/
    /*If you accept GSM,Please refer to the CgdcontDemo:case 2,After setting it up successfully,You can use the TCPIP API*/

    return ret;
}

ret_msg_t test_network_mode_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    int APInum = 0;
    int creg;
    SCcpsiParm Scpsi = { 0 };
    /*This is a generic example,Users can also design according to their actual needs*/
    post_network_result(name, type, "This is a generic example,Users can also design according to their actual needs");
    while (1)
    {
        if (APInum > 60) /*You can define the number of retries you want*/
        {
            /*It is recommended to switch to cFUN or CNMP mode*/
            break;
        }
        APInum++;
        ret.errcode = sAPI_NetworkGetCpsi(&Scpsi);/*Power on to inquire the SIM card status*/
        if (ret.errcode == SC_NET_SUCCESS)
        {
            if (strncmp(Scpsi.networkmode, "No", sizeof(char) * 2) == 0)
            {
                sAPI_TaskSleep(200);
                continue;/*retry*/
            }
            if (strncmp(Scpsi.networkmode, "LTE", sizeof(char) * 3) == 0)
            {
                post_network_result(name, type, "Get cpsi success and LTE");
                break;/*You can use the TCPIP API*/
            }
            if (strncmp(Scpsi.networkmode, "GSM", sizeof(char) * 3) == 0)
            {
                post_network_result(name, type, "Get cpsi success and GSM");
                break;/*Maybe some other API has to be called*/
            }
        }
        else
        {
            sAPI_TaskSleep(200);/*All API query times are recommended once 1S*/
            post_network_result(name, type, "Get cpsi falied!");
            //continue;/*If this fails, you can wait for 1S to continue the query*/
            break;
        }
    }
    while (1)
    {
        if (APInum > 30) /*You can define the number of retries you want*/
        {
            /*It is recommended to switch to cFUN or CNMP mode*/
            break;
        }
        APInum++;
        ret.errcode = sAPI_NetworkGetCreg(&creg);/*creg judgment cs      cgreg judgment ps*/
        if (ret.errcode == SC_NET_SUCCESS)
        {
            if ((creg == 1) || (creg == 5))
            {
                /*you can voice call*/
                post_network_result(name, type, "Network has registed!");
                break;
            }
            else
            {
                sAPI_TaskSleep(200);
                continue;/*retry*/
            }
        }
        else
        {
            sAPI_TaskSleep(200);/*All API query times are recommended once 1S*/
            post_network_result(name, type, "Network registed falied!");
            //continue;/*If this fails, you can wait for 1S to continue the query*/
            break;
        }
    }

    return ret;
}

ret_msg_t test_multiple_dialing_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_RAW;
    char NetResp[1000] = {0};
    SCdialapnparm apnparm = {0};
    SCdialapnparm dialapnparm[6];
    int i;
    sprintf(apnparm.ip_type, "IP");
    sprintf(apnparm.apn, "CMNET_0");
    sAPI_NetworkSetCnetcon(0, &apnparm);
    memset(&apnparm, 0, sizeof(SCdialapnparm));

    sprintf(apnparm.ip_type, "IP");
    sprintf(apnparm.apn, "CMNET_1");
    sAPI_NetworkSetCnetcon(1, &apnparm);

    memset(dialapnparm, 0, sizeof(SCdialapnparm));
    sAPI_NetworkGetCnetcon(dialapnparm);
    for (i = 0; i < 6; i++)
    {
        LOG_INFO("%s,%d,%s,%s,%s,%s,%d", __func__, i, dialapnparm[i].apn, dialapnparm[i].user, dialapnparm[i].pswd,
                   dialapnparm[i].ip_type, dialapnparm[i].auth);
        if(strlen (dialapnparm[i].apn) != 0)
        {
            memset(NetResp, 0, sizeof(NetResp));
            sprintf(NetResp, "%d,%s,%s", i, dialapnparm[i].apn,dialapnparm[i].ip_type);
            post_network_result(name, type, NetResp);
        }
    }
    ret.errcode = sAPI_TcpipPdpActive(1, 1);
    if(ret.errcode == 0){
        sprintf(NetResp, "cid 1 actived!");
        post_network_result(name, type, NetResp);
    }
    sAPI_TaskSleep(400);
    ret.errcode = sAPI_TcpipPdpActive(2, 1);
    if(ret.errcode == 0){
        sprintf(NetResp, "cid 2 actived!");
        post_network_result(name, type, NetResp);
    }

    return ret;
}

#ifdef PLATFORM_160X
static ret_msg_t get_cclk_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_RAW;
    scRealTime Srealtime;
    char RTresp[50] = {0};
    ret.errcode = sAPI_NetworkGetRealTime(&Srealtime);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Get realtime success!");
        post_network_result("info",TYPE_RAW,"(If you want get real time, pls set ctzu, then set cfun0, cfun1 to update firstly!)");
        sprintf(RTresp,"RT: 20%02d-%02d-%02d %02d:%02d:%02d:%c%02d",Srealtime.year,Srealtime.mon,\
                Srealtime.day,Srealtime.hour,Srealtime.min,Srealtime.sec,Srealtime.sign,Srealtime.timezone);
        post_network_result(name, type, RTresp);
    }
    else
    {
        LOG_ERROR("Get realtime falied!");
        sprintf(RTresp, "Get realtime falied!");
        post_network_result(name, type, RTresp);
    }

    return ret;
}

static ret_msg_t get_ctzu_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_RAW;
    int ctzuvalue = 0;
    char RTresp[50] = {0};
    ret.errcode = sAPI_NetworkGetCtzu(&ctzuvalue);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Get CTZU success. CTZU=%d!",ctzuvalue);
        sprintf(RTresp,"CTZU=%d!",ctzuvalue);
    }
    else
    {
        LOG_ERROR("Get CTZU falied!");
        sprintf(RTresp, "Get CTZU falied!");
    }
    post_network_result(name, type, RTresp);

    return ret;
}

static ret_msg_t set_ctzu_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    const char* ctzuStr = NULL;
    int ctzuVal;
    char RTresp[50] = {0};
    pl_demo_get_data(&ctzuStr, argv, argn, ARG_SETCTZU);
    if(strncmp(ctzuStr, ARG_CTZU_OFF, sizeof(ARG_CTZU_OFF) - 1) == 0)
        ctzuVal = 0;
    else
        ctzuVal = 1;
    ret.errcode = sAPI_NetworkSetCtzu(ctzuVal);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Set CTZU success.");
        sprintf(RTresp,"Set CTZU success");
    }
    else
    {
        LOG_ERROR("Set CTZU falied!");
        sprintf(RTresp, "Set CTZU falied!");
    }
    post_network_result(name, type, RTresp);

    return ret;
}

ret_msg_t get_realtime_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_RAW;
    char RTresp[50];
    scRealTime Srealtime;
    sAPI_NetworkSetCtzu(1);
    sAPI_TaskSleep(100);
    sAPI_NetworkSetCfun(0);
    sAPI_TaskSleep(200*3);
    sAPI_NetworkSetCfun(1);
    sAPI_TaskSleep(200*3);
    ret.errcode = sAPI_NetworkGetRealTime(&Srealtime);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Get realtime success!");
        sprintf(RTresp,"RT: 20%02d-%02d-%02d %02d:%02d:%02d:%c%02d",Srealtime.year,Srealtime.mon,\
                Srealtime.day,Srealtime.hour,Srealtime.min,Srealtime.sec,Srealtime.sign,Srealtime.timezone);
    }
    else
    {
        LOG_ERROR("Get realtime falied!");
        sprintf(RTresp, "Get realtime falied!");
    }
    post_network_result(name, type, RTresp);

    return ret;
}

static ret_msg_t enable_1bis_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    char PSMresp[50] = {0};
    ret.errcode = sAPI_EnableOneBis();
    sAPI_TaskSleep(400);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Enable 1 bis!");
        sprintf(PSMresp,"Enable 1 bis!");
    }
    else
    {
        LOG_ERROR("Enable 1 bis falied!");
        sprintf(PSMresp, "Enable 1 bis falied!");
    }

    post_network_result(name, type, PSMresp);
    return ret;
}

static ret_msg_t set_t3324_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    char PSMresp[50] = {0};
    int mode;
    const char *modeStr = NULL;
    const char *t3324Str = NULL;
    char t3324[20] = {0};
    pl_demo_get_data(&modeStr, argv, argn, ARG_T3324MODE);
    pl_demo_get_data(&t3324Str, argv, argn, ARG_T3324TIME);
    if(t3324Str != NULL)
        strcpy(t3324, t3324Str);
    if(strncmp(modeStr, ARG_T3324_MODE_DISABLE, sizeof(ARG_T3324_MODE_DISABLE) - 1) == 0)
        mode = 0;
    else
        mode = 1;
    LOG_INFO("mode = %d, t3324 = %s", mode, t3324);
    ret.errcode = sAPI_PSMSetT3324Timer(mode, t3324);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Set T3324 Timer success!");
        sprintf(PSMresp,"Set T3324 Timer success!");
    }
    else
    {
        LOG_ERROR("Set T3324 Timer falied!");
        sprintf(PSMresp, "Set T3324 Timer falied!");
    }

    post_network_result(name, type, PSMresp);
    return ret;
}

static ret_msg_t enable_HardWare_psm_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    char PSMresp[50] = {0};
    ret.errcode = sAPI_EnableHardWarePSM();
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Enable Hardware PSM success!");
        sprintf(PSMresp,"Enable Hardware PSM success!");
    }
    else
    {
        LOG_ERROR("Enable Hardware PSM falied!");
        sprintf(PSMresp, "Enable Hardware PSM falied!");
    }

    post_network_result(name, type, PSMresp);
    return ret;
}

static ret_msg_t set_t3412_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    int t3412;
    char PSMresp[50] = {0};
    pl_demo_get_value(&t3412, argv, argn, ARG_T3412TIME);
    ret.errcode = sAPI_PSMSetT3412Timer(t3412);//t3412(min)
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Set T3412 Timer success!");
        sprintf(PSMresp,"Set T3412 Timer success!");
    }
    else
    {
        LOG_ERROR("Set T3412 Timer falied!");
        sprintf(PSMresp, "Set T3412 Timer falied!");
    }

    post_network_result(name, type, PSMresp);
    return ret;
}

static ret_msg_t close_psm_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_STR;
    char PSMresp[50] = {0};
    ret.errcode = sAPI_PSMSetT3324Timer(0,"0");//Close PSM
    if(ret.errcode == SC_NET_SUCCESS)
    {
        LOG_INFO("Close PSM success!");
        sprintf(PSMresp,"Close PSM success!");
    }
    else
    {
        LOG_ERROR("Close PSM falied!");
        sprintf(PSMresp, "Close PSM falied!");
    }

    post_network_result(name, type, PSMresp);
    return ret;
}

ret_msg_t get_dns_addr_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int cid;
    SCDNSAddr dnsAddr = {0};
    char* name = ARG_NETWORKRESULT;
    type_t type = TYPE_RAW;
    char dnsaddrResp[200] = {0};
    pl_demo_get_value(&cid, argv, argn, ARG_CID);
    ret.errcode = sAPI_NetworkGetDNSAddr(cid, &dnsAddr);
    if(ret.errcode == SC_NET_SUCCESS)
    {
        sprintf(dnsaddrResp, "cid: %d,dnsIpv4PrimAddr: %s,dnsIpv4SecAddr: %s,dnsIpv6PrimAddr: %s,dnsIpv6SecAddr: %s",
               dnsAddr.cid + 1, dnsAddr.dnsIpv4PrimAddr,dnsAddr.dnsIpv4SecAddr,dnsAddr.dnsIpv6PrimAddr,dnsAddr.dnsIpv6SecAddr);
        post_network_result(name, type, dnsaddrResp);
    }
    else
    {
        sAPI_TaskSleep(200);/*All API query times are recommended once 1S*/
        post_network_result(name, TYPE_STR, "Get dnsAddr falied!");
        //continue;/*If this fails, you can wait for 1S to continue the query*/
    }
    return ret;
}

#endif



