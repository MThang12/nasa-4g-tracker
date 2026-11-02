#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "demo_gps.h"
#include "simcom_gps.h"


#define LOG_ERROR(...) sal_log_error("[DEMO-GNSS] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-GNSS] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-GNSS] " __VA_ARGS__)

static ret_msg_t gnss_apflash_status_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_pwr_status_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_nmea_data_get_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_start_mode_set_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_baud_rate_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_mode_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_nmea_rate_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_nmea_sentence_set_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_nmea_sentence_get_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_and_gps_info_get_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_cmd_send_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t gnss_agps_cmd_proc(op_t op, arg_t *argv, int argn);
const value_t apflash_status[3] =
{
    {
        .bufp = AP_FLASH_STATUS_ON,
        .size = sizeof(AP_FLASH_STATUS_ON)-1,
    },

    {
        .bufp = AP_FLASH_STATUS_OFF,
        .size = sizeof(AP_FLASH_STATUS_OFF)-1,
    },

    {
        .bufp = GET_AP_FLASH_STATUS,
        .size = sizeof(GET_AP_FLASH_STATUS)-1,
    },

};

const value_list_t apflash_range =
{
    .isrange = false,
    .list_head = apflash_status,
    .count = 3,
};

arg_t apflash_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGGNSS_APFLASH_STATUS,
        .value_range = &apflash_range,
    }
};

const value_t gnss_power_status[3] =
{
    {
        .bufp = GNSS_PWR_STATUS_ON,
        .size = sizeof(GNSS_PWR_STATUS_ON)-1,
    },

    {
        .bufp = GNSS_PWR_STATUS_OFF,
        .size = sizeof(GNSS_PWR_STATUS_OFF)-1,
    },

    {
        .bufp = GET_GNSS_PWR_STATUS,
        .size = sizeof(GET_GNSS_PWR_STATUS)-1,
    },

};

const value_list_t gnss_power_range =
{
    .isrange = false,
    .list_head = gnss_power_status,
    .count = 3,
};

arg_t power_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGGNSS_POWER_STATUS,
        .value_range = &gnss_power_range,
    }
};

const value_t gnss_nmea_data_get_status[4] =
{
    {
        .bufp = GNSS_NMEA_DATA_GET_START_BY_PORT,
        .size = sizeof(GNSS_NMEA_DATA_GET_START_BY_PORT)-1,
    },

    {
        .bufp = GNSS_NMEA_DATA_GET_STOP_BY_PORT,
        .size = sizeof(GNSS_NMEA_DATA_GET_STOP_BY_PORT)-1,
    },

    {
        .bufp = GNSS_NMEA_DATA_GET_START_BY_URC,
        .size = sizeof(GNSS_NMEA_DATA_GET_START_BY_URC)-1,
    },

    {
        .bufp = GNSS_NMEA_DATA_GET_STOP_BY_URC,
        .size = sizeof(GNSS_NMEA_DATA_GET_STOP_BY_URC)-1,
    },

};

const value_list_t gnss_nmea_data_get_range =
{
    .isrange = false,
    .list_head = gnss_nmea_data_get_status,
    .count = 4,
};

arg_t nmea_data_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGGNSS_NMEA_DATA_GET,
        .value_range = &gnss_nmea_data_get_range,
    }
};

const value_t gnss_start_mode_set_status[3] =
{
    {
        .bufp = GNSS_HOT_START,
        .size = sizeof(GNSS_HOT_START)-1,
    },

    {
        .bufp = GNSS_WARM_START,
        .size = sizeof(GNSS_WARM_START)-1,
    },

    {
        .bufp = GNSS_COLD_START,
        .size = sizeof(GNSS_COLD_START)-1,
    },
};

const value_list_t gnss_start_mode_set_range =
{
    .isrange = false,
    .list_head = gnss_start_mode_set_status,
    .count = 3,
};

arg_t gnss_start_mode_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGGNSS_START_MODE_SET,
        .value_range = &gnss_start_mode_set_range,
    }
};

#ifdef JACANA_GPS_SUPPORT
const value_t gnss_baud_rate_status[2] =
{
    {
        .bufp = GNSS_DEMO_BAUD_RATE_115200,
        .size = sizeof(GNSS_DEMO_BAUD_RATE_115200)-1,
    },

    {
        .bufp = GET_GNSS_BAUD_RATE,
        .size = sizeof(GET_GNSS_BAUD_RATE)-1,
    },
};
#elif SC_DRIVER_GNSS_ZKW
const value_t gnss_baud_rate_status[7] =
{
    {
        .bufp = GNSS_DEMO_BAUD_RATE_4800,
        .size = sizeof(GNSS_DEMO_BAUD_RATE_4800)-1,
    },

    {
        .bufp = GNSS_DEMO_BAUD_RATE_9600,
        .size = sizeof(GNSS_DEMO_BAUD_RATE_9600)-1,
    },

    {
        .bufp = GNSS_DEMO_BAUD_RATE_19200,
        .size = sizeof(GNSS_DEMO_BAUD_RATE_19200)-1,
    },
    {
        .bufp = GNSS_DEMO_BAUD_RATE_38400,
        .size = sizeof(GNSS_DEMO_BAUD_RATE_38400)-1,
    },

    {
        .bufp = GNSS_DEMO_BAUD_RATE_57600,
        .size = sizeof(GNSS_DEMO_BAUD_RATE_57600)-1,
    },

    {
        .bufp = GNSS_DEMO_BAUD_RATE_115200,
        .size = sizeof(GNSS_DEMO_BAUD_RATE_115200)-1,
    },

    {
        .bufp = GET_GNSS_BAUD_RATE,
        .size = sizeof(GET_GNSS_BAUD_RATE)-1,
    },
};
#else//SC_DRIVER_GNSS_UNIC
const value_t gnss_baud_rate_status[4] =
{
    {
        .bufp = GNSS_DEMO_BAUD_RATE_9600,
        .size = sizeof(GNSS_DEMO_BAUD_RATE_9600)-1,
    },

    {
        .bufp = GNSS_DEMO_BAUD_RATE_115200,
        .size = sizeof(GNSS_DEMO_BAUD_RATE_115200)-1,
    },

    {
        .bufp = GNSS_DEMO_BAUD_RATE_230400,
        .size = sizeof(GNSS_DEMO_BAUD_RATE_230400)-1,
    },

    {
        .bufp = GET_GNSS_BAUD_RATE,
        .size = sizeof(GET_GNSS_BAUD_RATE)-1,
    },
};
#endif

const value_list_t gnss_baud_rate_range =
{
    .isrange = false,
    .list_head = gnss_baud_rate_status,
#ifdef JACANA_GPS_SUPPORT
    .count = 2,
#elif SC_DRIVER_GNSS_ZKW
    .count = 7,
#else//SC_DRIVER_GNSS_UNIC
    .count = 4,
#endif
};

arg_t gnss_baud_rate_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGGNSS_BAUD_RATE,
        .value_range = &gnss_baud_rate_range,
    }
};

#if (defined JACANA_GPS_SUPPORT) || (defined SC_DRIVER_GNSS_ZKW)
const value_t gnss_mode_status[8] =
{
    {
        .bufp = GNSS_DEMO_MODE_GPS,
        .size = sizeof(GNSS_DEMO_MODE_GPS)-1,
    },

    {
        .bufp = GNSS_DEMO_MODE_BDS,
        .size = sizeof(GNSS_DEMO_MODE_BDS)-1,
    },

    {
        .bufp = GNSS_DEMO_MODE_GPS_BDS,
        .size = sizeof(GNSS_DEMO_MODE_GPS_BDS)-1,
    },
    {
        .bufp = GNSS_DEMO_MODE_GLONASS,
        .size = sizeof(GNSS_DEMO_MODE_GLONASS)-1,
    },

    {
        .bufp = GNSS_DEMO_MODE_GPS_GLONASS,
        .size = sizeof(GNSS_DEMO_MODE_GPS_GLONASS)-1,
    },

    {
        .bufp = GNSS_DEMO_MODE_BDS_GLONASS,
        .size = sizeof(GNSS_DEMO_MODE_BDS_GLONASS)-1,
    },

    {
        .bufp = GNSS_DEMO_MODE_GPS_BDS_GLONASS,
        .size = sizeof(GNSS_DEMO_MODE_GPS_BDS_GLONASS)-1,
    },

    {
        .bufp = GET_GNSS_MODE,
        .size = sizeof(GET_GNSS_MODE)-1,
    },

};
#else//SC_DRIVER_GNSS_UNIC
#ifdef FEATURE_SIMCOM_GPS_OVERSEAS
const value_t gnss_mode_status[5] =
{
    {
        .bufp = GNSS_DEMO_MODE_GPS,
        .size = sizeof(GNSS_DEMO_MODE_GPS)-1,
    },

    {
        .bufp = GNSS_DEMO_MODE_BDS,
        .size = sizeof(GNSS_DEMO_MODE_BDS)-1,
    },

    {
        .bufp = GNSS_DEMO_MODE_GPS_GLONASS_GALILEO,
        .size = sizeof(GNSS_DEMO_MODE_GPS_GLONASS_GALILEO)-1,
    },
    {
        .bufp = GNSS_DEMO_MODE_GPS_BDS_GALILEO,
        .size = sizeof(GNSS_DEMO_MODE_GPS_BDS_GALILEO)-1,
    },

    {
        .bufp = GET_GNSS_MODE,
        .size = sizeof(GET_GNSS_MODE)-1,
    },
};
#else
const value_t gnss_mode_status[4] =
{
    {
        .bufp = GNSS_DEMO_MODE_GPS_BDS,
        .size = sizeof(GNSS_DEMO_MODE_GPS_BDS)-1,
    },

    {
        .bufp = GNSS_DEMO_MODE_BDS,
        .size = sizeof(GNSS_DEMO_MODE_BDS)-1,
    },

    {
        .bufp = GNSS_DEMO_MODE_GPS,
        .size = sizeof(GNSS_DEMO_MODE_GPS)-1,
    },

    {
        .bufp = GET_GNSS_MODE,
        .size = sizeof(GET_GNSS_MODE)-1,
    },
};
#endif
#endif

const value_list_t gnss_mode_range =
{
    .isrange = false,
    .list_head = gnss_mode_status,
#if (defined JACANA_GPS_SUPPORT) || (defined SC_DRIVER_GNSS_ZKW)
    .count = 8,
#else//SC_DRIVER_GNSS_UNIC
#ifdef FEATURE_SIMCOM_GPS_OVERSEAS
    .count = 5,
#else
    .count = 4,
#endif
#endif
};

arg_t gnss_mode_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGGNSS_MODE,
        .value_range = &gnss_mode_range,
    }
};

#ifdef JACANA_GPS_SUPPORT
const value_t gnss_nmea_rate_status[2] =
{
    {
        .bufp = GNSS_DEMO_NMEA_RATE_1HZ,
        .size = sizeof(GNSS_DEMO_NMEA_RATE_1HZ)-1,
    },

    {
        .bufp = GET_GNSS_NMEA_RATE,
        .size = sizeof(GET_GNSS_NMEA_RATE)-1,
    },
};
#elif SC_DRIVER_GNSS_ZKW
const value_t gnss_nmea_rate_status[6] =
{
    {
        .bufp = GNSS_DEMO_NMEA_RATE_1HZ,
        .size = sizeof(GNSS_DEMO_NMEA_RATE_1HZ)-1,
    },

    {
        .bufp = GNSS_DEMO_NMEA_RATE_2HZ,
        .size = sizeof(GNSS_DEMO_NMEA_RATE_2HZ)-1,
    },

    {
        .bufp = GNSS_DEMO_NMEA_RATE_4HZ,
        .size = sizeof(GNSS_DEMO_NMEA_RATE_4HZ)-1,
    },

    {
        .bufp = GNSS_DEMO_NMEA_RATE_5HZ,
        .size = sizeof(GNSS_DEMO_NMEA_RATE_5HZ)-1,
    },

    {
        .bufp = GNSS_DEMO_NMEA_RATE_10HZ,
        .size = sizeof(GNSS_DEMO_NMEA_RATE_10HZ)-1,
    },

    {
        .bufp = GET_GNSS_NMEA_RATE,
        .size = sizeof(GET_GNSS_NMEA_RATE)-1,
    },
};
#else
const value_t gnss_nmea_rate_status[4] =
{
    {
        .bufp = GNSS_DEMO_NMEA_RATE_1HZ,
        .size = sizeof(GNSS_DEMO_NMEA_RATE_1HZ)-1,
    },

    {
        .bufp = GNSS_DEMO_NMEA_RATE_2HZ,
        .size = sizeof(GNSS_DEMO_NMEA_RATE_2HZ)-1,
    },

    {
        .bufp = GNSS_DEMO_NMEA_RATE_5HZ,
        .size = sizeof(GNSS_DEMO_NMEA_RATE_5HZ)-1,
    },

    {
        .bufp = GET_GNSS_NMEA_RATE,
        .size = sizeof(GET_GNSS_NMEA_RATE)-1,
    },
};
#endif

const value_list_t gnss_nmea_rate_range =
{
    .isrange = false,
    .list_head = gnss_nmea_rate_status,
#ifdef JACANA_GPS_SUPPORT
    .count = 2,
#elif SC_DRIVER_GNSS_ZKW
    .count = 6,
#else
    .count = 4,
#endif
};

arg_t gnss_nmea_rate_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGGNSS_NMEA_RATE,
        .value_range = &gnss_nmea_rate_range,
    }
};

const value_t gnss_nmea_sentence_get_status[1] =
{

    {
        .bufp = GET_NMEA_SENTENCE,
        .size = sizeof(GET_NMEA_SENTENCE)-1,
    },
};

const value_list_t gnss_nmea_sentence_get_range =
{
    .isrange = false,
    .list_head = gnss_nmea_sentence_get_status,
    .count = 1,
};

arg_t gnss_nmea_sentence_get_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGGNSS_NMEA_SENTENCE_GET,
        .value_range = &gnss_nmea_sentence_get_range,
    },
};

const value_list_t nmea_sentence_mask_range =
{
    .isrange = true,
    .min = 0,
    #ifdef JACANA_GPS_SUPPORT
    .max = 511,
    #elif SC_DRIVER_GNSS_ZKW
    .max = 13311,
    #else
    .max = 255,
    #endif
};

arg_t gnss_nmea_sentence_set_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGGNSS_NMEA_SENTENCE_SET,
        #ifdef JACANA_GPS_SUPPORT
        .msg = "Input mask(0-511),bit0-RMC,bit1-VTG,bit2-GGA,bit3-GSA,bit4-GSV,bit5-GLL,bit6-ZDA,bit7-GST,bit8-TXT,default mask is 511(111111111),open is 1, close is 0.",
        #elif SC_DRIVER_GNSS_ZKW
        .msg = "Input mask(0-13311),bit0-GGA,bit1-GLL,bit2-GSA,bit3-GSV,bit4-RMC,bit5-VTG,bit6-ZDA,bit7-ANT,bit8-DHV,bit9-LPS,bit10-res1,bit11-res2,bit12-UTC,bit13-GST,default mask is 255(11111111),open is 1, close is 0.",
        #else
        .msg = "Input mask(0-255),bit0-GGA,bit1-GLL,bit2-GSA,bit3-GSV,bit4-RMC,bit5-VTG,bit6-ZDA,bit7-GST,default mask is 63(00111111),open is 1, close is 0.",
        #endif
        .value_range = &nmea_sentence_mask_range,
    },
};

const value_t gnss_and_gps_info_get_status[2] =
{
    {
        .bufp = GET_GPSINFO,
        .size = sizeof(GET_GPSINFO)-1,
    },

    {
        .bufp = GET_GNSSINFO,
        .size = sizeof(GET_GNSSINFO)-1,
    },
};

const value_list_t gnss_and_gps_info_get_range =
{
    .isrange = false,
    .list_head = gnss_and_gps_info_get_status,
    .count = 2,
};

const value_list_t time_range =
{
    .isrange = true,
    .min = 0,
    .max = 255,
};

arg_t gnss_and_gps_info_get_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGGNSS_AND_GPS_INFO_GET,
        .value_range = &gnss_and_gps_info_get_range,
    },

    {
        .type = TYPE_INT,
        .arg_name = SET_INFO_TIMER,
        .msg = "please input the reporting cycle of GPS/GNSS information, the range is 0-255s, 0 means stop reporting!",
        .value_range = &time_range,
    },
};

arg_t gnss_cmd_send_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGGNSS_CMD_SEND,
        .msg = "please input the string of the NMEA format directly, like this: $PDTINFO"
    },
};

const value_t gnss_agps_status[1] =
{
    {
        .bufp = AGPS_OPEN,
        .size = sizeof(AGPS_OPEN)-1,
    },
};

const value_list_t gnss_agps_range =
{
    .isrange = false,
    .list_head = gnss_agps_status,
    .count = 1,
};

arg_t gnss_agps_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGGNSS_AGPS,
        .value_range = &gnss_agps_range,
    },
};

const menu_list_t func_gnssApflashStatus =
{
    .menu_name = "gnss ap_flash status",
    .menu_type = TYPE_FUNC,
    .parent = &gnss_menu,
    .function = {
        .argv = apflash_args,
        .argn = sizeof(apflash_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_apflash_status_cmd_proc,
    },
};

const menu_list_t func_gnssPowerStatus =
{
    .menu_name = "gnss power status",
    .menu_type = TYPE_FUNC,
    .parent = &gnss_menu,
    .function = {
        .argv = power_args,
        .argn = sizeof(power_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_pwr_status_cmd_proc,
    },
};

const menu_list_t func_gnssNmeaGet =
{
    .menu_name = "gnss nmea data get",
    .menu_type = TYPE_FUNC,
    .parent = &gnss_menu,
    .function = {
        .argv = nmea_data_args,
        .argn = sizeof(nmea_data_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_nmea_data_get_cmd_proc,
    },
};

const menu_list_t func_gnssStartMode =
{
    .menu_name = "gnss start mode",
    .menu_type = TYPE_FUNC,
    .parent = &gnss_menu,
    .function = {
        .argv = gnss_start_mode_args,
        .argn = sizeof(gnss_start_mode_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_start_mode_set_cmd_proc,
    },
};

const menu_list_t func_gnssBaudrate =
{
    .menu_name = "gnss baud rate",
    .menu_type = TYPE_FUNC,
    .parent = &gnss_menu,
    .function = {
        .argv = gnss_baud_rate_args,
        .argn = sizeof(gnss_baud_rate_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_baud_rate_cmd_proc,
    },
};

const menu_list_t func_gnssMode =
{
    .menu_name = "gnss mode",
    .menu_type = TYPE_FUNC,
    .parent = &gnss_menu,
    .function = {
        .argv = gnss_mode_args,
        .argn = sizeof(gnss_mode_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_mode_cmd_proc,
    },
};

const menu_list_t func_gnssNmeaRate =
{
    .menu_name = "gnss nmea rate",
    .menu_type = TYPE_FUNC,
    .parent = &gnss_menu,
    .function = {
        .argv = gnss_nmea_rate_args,
        .argn = sizeof(gnss_nmea_rate_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_nmea_rate_cmd_proc,
    },
};

const menu_list_t func_gnssNmeaSentenceSet =
{
    .menu_name = "gnss nmea sentence set",
    .menu_type = TYPE_FUNC,
    .parent = &gnss_menu,
    .function = {
        .argv = gnss_nmea_sentence_set_args,
        .argn = sizeof(gnss_nmea_sentence_set_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_nmea_sentence_set_cmd_proc,
    },
};

const menu_list_t func_gnssNmeaSentenceGet =
{
    .menu_name = "gnss nmea sentence get",
    .menu_type = TYPE_FUNC,
    .parent = &gnss_menu,
    .function = {
        .argv = gnss_nmea_sentence_get_args,
        .argn = sizeof(gnss_nmea_sentence_get_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_nmea_sentence_get_cmd_proc,
    },
};

const menu_list_t func_gnssAndGpsInfoGet =
{
    .menu_name = "gpsinfo and gnssinfo",
    .menu_type = TYPE_FUNC,
    .parent = &gnss_menu,
    .function = {
        .argv = gnss_and_gps_info_get_args,
        .argn = sizeof(gnss_and_gps_info_get_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_and_gps_info_get_cmd_proc,
    },
};

const menu_list_t func_gnssCmdSend =
{
    .menu_name = "send command to gnss",
    .menu_type = TYPE_FUNC,
    .parent = &gnss_menu,
    .function = {
        .argv = gnss_cmd_send_args,
        .argn = sizeof(gnss_cmd_send_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_cmd_send_proc,
    },
};

const menu_list_t func_gnssOpenAgps =
{
    .menu_name = "open agps",
    .menu_type = TYPE_FUNC,
    .parent = &gnss_menu,
    .function = {
        .argv = gnss_agps_args,
        .argn = sizeof(gnss_agps_args)/sizeof(arg_t),
        .methods.cmd_handler = gnss_agps_cmd_proc,
    },
};

const menu_list_t *gnss_menu_sub[] =
{
    &func_gnssApflashStatus,
    &func_gnssPowerStatus,
    &func_gnssNmeaGet,
    &func_gnssStartMode,
    &func_gnssBaudrate,
    &func_gnssMode,
    &func_gnssNmeaRate,
    &func_gnssNmeaSentenceSet,
    &func_gnssNmeaSentenceGet,
    &func_gnssAndGpsInfoGet,
    &func_gnssCmdSend,
    &func_gnssOpenAgps,
    NULL  // MUST end by NULL
};

const menu_list_t gnss_menu =
{
    .menu_name = "GNSS",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = gnss_menu_sub,
    .parent = &root_menu,
};

static ret_msg_t gnss_apflash_status_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_Gnss_Ap_Flash_Status apflash_switch = SC_GNSS_AP_FLASH_OFF;
    int isGetApflashStatus = 0;
    const char *ap_flash_status = NULL;
    char statusbuf[64] = {0};
    arg_t out_arg[1] = {0};

    pl_demo_get_data(&ap_flash_status, argv, argn, ARGGNSS_APFLASH_STATUS);
    if (!ap_flash_status)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    if(strncmp(ap_flash_status,AP_FLASH_STATUS_ON,sizeof(AP_FLASH_STATUS_ON)-1) == 0)
    {
        apflash_switch = SC_GNSS_AP_FLASH_ON;
    }
    if(strncmp(ap_flash_status,AP_FLASH_STATUS_OFF,sizeof(AP_FLASH_STATUS_OFF)-1) == 0)
    {
        apflash_switch = SC_GNSS_AP_FLASH_OFF;
    }
    if(strncmp(ap_flash_status,GET_AP_FLASH_STATUS,sizeof(GET_AP_FLASH_STATUS)-1) == 0)
    {
        isGetApflashStatus = 1;
    }
    LOG_INFO("%s,apflash_switch:[%d],isGetApflashStatus:%d", __func__,apflash_switch,isGetApflashStatus);

    if(isGetApflashStatus == 1)
    {
        memset(statusbuf, 0, sizeof(statusbuf));
        apflash_switch = sAPI_GnssApFlashGet();
        if(apflash_switch == SC_GNSS_AP_FLASH_ON)
            snprintf(statusbuf, sizeof(statusbuf),"ap_flash status is on.\r\n");
        else
            snprintf(statusbuf, sizeof(statusbuf),"ap_flash status is off.\r\n");

        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = ARGGNSS_APFLASH_STATUS;
        out_arg[0].value = pl_demo_make_value(0, statusbuf, strlen(statusbuf));

        ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

        pl_demo_release_value(out_arg[0].value);

        if (0 != ret.errcode)
        {
            ret.msg = "sAPI_GnssApFlashGet post data fail";
            goto EXIT;
        }
    }
    else
    {
        if (sAPI_GnssApFlashSet(apflash_switch) != SC_GNSS_RETURN_CODE_OK)
        {
            LOG_ERROR("sAPI_GnssApFlashSet fail!!!");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "sAPI_GnssApFlashSet fail!!!";
            goto EXIT;
        }
    }

EXIT:
    return ret;
}

static ret_msg_t gnss_pwr_status_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_Gnss_Power_Status power_switch = SC_GNSS_POWER_OFF;
    int isGetPowerStatus = 0;
    const char *gnss_pwr_status = NULL;
    char statusbuf[64] = {0};
    arg_t out_arg[1] = {0};

    pl_demo_get_data(&gnss_pwr_status, argv, argn, ARGGNSS_POWER_STATUS);
    if (!gnss_pwr_status)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    if(strncmp(gnss_pwr_status,GNSS_PWR_STATUS_ON,sizeof(GNSS_PWR_STATUS_ON)-1) == 0)
    {
        power_switch = SC_GNSS_POWER_ON;
    }
    if(strncmp(gnss_pwr_status,GNSS_PWR_STATUS_OFF,sizeof(GNSS_PWR_STATUS_OFF)-1) == 0)
    {
        power_switch = SC_GNSS_POWER_OFF;
    }
    if(strncmp(gnss_pwr_status,GET_GNSS_PWR_STATUS,sizeof(GET_GNSS_PWR_STATUS)-1) == 0)
    {
        isGetPowerStatus = 1;
    }
    LOG_INFO("%s,power_switch:[%d],isGetPowerStatus:%d", __func__,power_switch,isGetPowerStatus);

    if(isGetPowerStatus == 1)
    {
        memset(statusbuf, 0, sizeof(statusbuf));
        power_switch = sAPI_GnssPowerStatusGet();
        if(power_switch == SC_GNSS_POWER_ON)
            snprintf(statusbuf, sizeof(statusbuf),"gnss power status is on.\r\n");
        else
            snprintf(statusbuf, sizeof(statusbuf),"gnss power status is off.\r\n");

        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = ARGGNSS_POWER_STATUS;
        out_arg[0].value = pl_demo_make_value(0, statusbuf, strlen(statusbuf));

        ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

        pl_demo_release_value(out_arg[0].value);

        if (0 != ret.errcode)
        {
            ret.msg = "sAPI_GnssPowerStatusGet post data fail";
            goto EXIT;
        }
    }
    else
    {
        if (sAPI_GnssPowerStatusSet(power_switch) != SC_GNSS_RETURN_CODE_OK)
        {
            LOG_ERROR("sAPI_GnssPowerStatusSet fail!!!");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "sAPI_GnssPowerStatusSet fail!!!";
            goto EXIT;
        }
    }

EXIT:
    return ret;
}

static ret_msg_t gnss_nmea_data_get_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_Gnss_Output_Control is_get_nmea_data = SC_GNSS_STOP_OUTPUT_NMEA_DATA;
    SC_Gnss_Nmea_Data_Get get_nmea_data_way = SC_GNSS_NMEA_DATA_GET_BY_PORT;
    const char *nmea_data_get_status = NULL;


    pl_demo_get_data(&nmea_data_get_status, argv, argn, ARGGNSS_NMEA_DATA_GET);
    if (!nmea_data_get_status)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    if(strncmp(nmea_data_get_status,GNSS_NMEA_DATA_GET_START_BY_PORT,sizeof(GNSS_NMEA_DATA_GET_START_BY_PORT)-1) == 0)
    {
        is_get_nmea_data = SC_GNSS_START_OUTPUT_NMEA_DATA;
        get_nmea_data_way = SC_GNSS_NMEA_DATA_GET_BY_PORT;
    }
    if(strncmp(nmea_data_get_status,GNSS_NMEA_DATA_GET_STOP_BY_PORT,sizeof(GNSS_NMEA_DATA_GET_STOP_BY_PORT)-1) == 0)
    {
        is_get_nmea_data = SC_GNSS_STOP_OUTPUT_NMEA_DATA;
        get_nmea_data_way = SC_GNSS_NMEA_DATA_GET_BY_PORT;
    }
    if(strncmp(nmea_data_get_status,GNSS_NMEA_DATA_GET_START_BY_URC,sizeof(GNSS_NMEA_DATA_GET_START_BY_URC)-1) == 0)
    {
        is_get_nmea_data = SC_GNSS_START_OUTPUT_NMEA_DATA;
        get_nmea_data_way = SC_GNSS_NMEA_DATA_GET_BY_URC;
    }
    if(strncmp(nmea_data_get_status,GNSS_NMEA_DATA_GET_STOP_BY_URC,sizeof(GNSS_NMEA_DATA_GET_STOP_BY_URC)-1) == 0)
    {
        is_get_nmea_data = SC_GNSS_STOP_OUTPUT_NMEA_DATA;
        get_nmea_data_way = SC_GNSS_NMEA_DATA_GET_BY_URC;
    }
    LOG_INFO("%s,is_get_nmea_data:[%d],get_nmea_data_way:%d", __func__,is_get_nmea_data,get_nmea_data_way);

    if (sAPI_GnssNmeaDataGet(is_get_nmea_data,get_nmea_data_way) != SC_GNSS_RETURN_CODE_OK)
    {
        LOG_ERROR("sAPI_GnssNmeaDataGet fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_GnssNmeaDataGet fail!!!";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t gnss_start_mode_set_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_Gnss_Start_Mode start_mode = SC_GNSS_START_COLD;
    const char *start_mode_set_opt = NULL;


    pl_demo_get_data(&start_mode_set_opt, argv, argn, ARGGNSS_START_MODE_SET);
    if (!start_mode_set_opt)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    if(strncmp(start_mode_set_opt,GNSS_HOT_START,sizeof(GNSS_HOT_START)-1) == 0)
    {
        start_mode = SC_GNSS_START_HOT;
    }
    if(strncmp(start_mode_set_opt,GNSS_WARM_START,sizeof(GNSS_WARM_START)-1) == 0)
    {
        start_mode = SC_GNSS_START_WARM;
    }
    if(strncmp(start_mode_set_opt,GNSS_COLD_START,sizeof(GNSS_COLD_START)-1) == 0)
    {
        start_mode = SC_GNSS_START_COLD;
    }

    LOG_INFO("%s,start_mode:[%d]", __func__,start_mode);

    if (sAPI_GnssStartMode(start_mode) != SC_GNSS_RETURN_CODE_OK)
    {
        LOG_ERROR("sAPI_GnssStartMode fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_GnssStartMode fail!!!";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t gnss_baud_rate_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_Gnss_Baud_Rate gnss_baud_rate = SC_GNSS_BAUD_RATE_115200;
    int isGetBaudRate = 0;
    const char *gnss_baud_rate_status = NULL;
    char statusbuf[64] = {0};
    arg_t out_arg[1] = {0};

    pl_demo_get_data(&gnss_baud_rate_status, argv, argn, ARGGNSS_BAUD_RATE);
    if (!gnss_baud_rate_status)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
#ifdef SC_DRIVER_GNSS_ZKW
    if(strncmp(gnss_baud_rate_status,GNSS_DEMO_BAUD_RATE_4800,sizeof(GNSS_DEMO_BAUD_RATE_4800)-1) == 0)
    {
        gnss_baud_rate = SC_GNSS_BAUD_RATE_4800;
    }
    if(strncmp(gnss_baud_rate_status,GNSS_DEMO_BAUD_RATE_19200,sizeof(GNSS_DEMO_BAUD_RATE_19200)-1) == 0)
    {
        gnss_baud_rate = SC_GNSS_BAUD_RATE_19200;
    }
    if(strncmp(gnss_baud_rate_status,GNSS_DEMO_BAUD_RATE_38400,sizeof(GNSS_DEMO_BAUD_RATE_38400)-1) == 0)
    {
        gnss_baud_rate = SC_GNSS_BAUD_RATE_38400;
    }
    if(strncmp(gnss_baud_rate_status,GNSS_DEMO_BAUD_RATE_57600,sizeof(GNSS_DEMO_BAUD_RATE_57600)-1) == 0)
    {
        gnss_baud_rate = SC_GNSS_BAUD_RATE_57600;
    }
#endif
#if (defined SC_DRIVER_GNSS_ZKW) || (defined SC_DRIVER_GNSS_UNIC)
    if(strncmp(gnss_baud_rate_status,GNSS_DEMO_BAUD_RATE_9600,sizeof(GNSS_DEMO_BAUD_RATE_9600)-1) == 0)
    {
        gnss_baud_rate = SC_GNSS_BAUD_RATE_9600;
    }
#endif

#ifdef SC_DRIVER_GNSS_UNIC
    if(strncmp(gnss_baud_rate_status,GNSS_DEMO_BAUD_RATE_230400,sizeof(GNSS_DEMO_BAUD_RATE_230400)-1) == 0)
    {
        gnss_baud_rate = SC_GNSS_BAUD_RATE_230400;
    }
#endif

    if(strncmp(gnss_baud_rate_status,GNSS_DEMO_BAUD_RATE_115200,sizeof(GNSS_DEMO_BAUD_RATE_115200)-1) == 0)
    {
        gnss_baud_rate = SC_GNSS_BAUD_RATE_115200;
    }
    if(strncmp(gnss_baud_rate_status,GET_GNSS_BAUD_RATE,sizeof(GET_GNSS_BAUD_RATE)-1) == 0)
    {
        isGetBaudRate = 1;
    }
    LOG_INFO("%s,gnss_baud_rate:[%d],isGetBaudRate:%d", __func__,gnss_baud_rate,isGetBaudRate);

    if(isGetBaudRate == 1)
    {
        memset(statusbuf, 0, sizeof(statusbuf));
        gnss_baud_rate = sAPI_GnssBaudRateGet();
        snprintf(statusbuf, sizeof(statusbuf),"gnss baud rate is %d.\r\n",gnss_baud_rate);

        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = ARGGNSS_BAUD_RATE;
        out_arg[0].value = pl_demo_make_value(0, statusbuf, strlen(statusbuf));

        ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

        pl_demo_release_value(out_arg[0].value);

        if (0 != ret.errcode)
        {
            ret.msg = "sAPI_GnssBaudRateGet post data fail";
            goto EXIT;
        }
    }
    else
    {
        if (sAPI_GnssBaudRateSet(gnss_baud_rate) != SC_GNSS_RETURN_CODE_OK)
        {
            LOG_ERROR("sAPI_GnssBaudRateSet fail!!!");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "sAPI_GnssBaudRateSet fail!!!";
            goto EXIT;
        }
    }

EXIT:
    return ret;
}

static ret_msg_t gnss_mode_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_Gnss_Mode gnss_mode = SC_GNSS_MODE_MAX;
    int isGetGnssMode = 0;
    const char *gnss_mode_status = NULL;
    char statusbuf[64] = {0};
    arg_t out_arg[1] = {0};

    pl_demo_get_data(&gnss_mode_status, argv, argn, ARGGNSS_MODE);
    if (!gnss_mode_status)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
#if defined(JACANA_GPS_SUPPORT) || defined(SC_DRIVER_GNSS_ZKW)
    if(strncmp(gnss_mode_status,GNSS_DEMO_MODE_GPS,sizeof(GNSS_DEMO_MODE_GPS)-1) == 0)
    {
        gnss_mode = SC_GNSS_MODE_GPS;
    }
    if(strncmp(gnss_mode_status,GNSS_DEMO_MODE_BDS,sizeof(GNSS_DEMO_MODE_BDS)-1) == 0)
    {
        gnss_mode = SC_GNSS_MODE_BDS;
    }
    if(strncmp(gnss_mode_status,GNSS_DEMO_MODE_GPS_BDS,sizeof(GNSS_DEMO_MODE_GPS_BDS)-1) == 0)
    {
        gnss_mode = SC_GNSS_MODE_GPS_BDS;
    }
    if(strncmp(gnss_mode_status,GNSS_DEMO_MODE_GLONASS,sizeof(GNSS_DEMO_MODE_GLONASS)-1) == 0)
    {
        gnss_mode = SC_GNSS_MODE_GLONASS;
    }
    if(strncmp(gnss_mode_status,GNSS_DEMO_MODE_GPS_GLONASS,sizeof(GNSS_DEMO_MODE_GPS_GLONASS)-1) == 0)
    {
        gnss_mode = SC_GNSS_MODE_GPS_GLONASS;
    }
    if(strncmp(gnss_mode_status,GNSS_DEMO_MODE_BDS_GLONASS,sizeof(GNSS_DEMO_MODE_BDS_GLONASS)-1) == 0)
    {
        gnss_mode = SC_GNSS_MODE_BDS_GLONASS;
    }
    if(strncmp(gnss_mode_status,GNSS_DEMO_MODE_GPS_BDS_GLONASS,sizeof(GNSS_DEMO_MODE_GPS_BDS_GLONASS)-1) == 0)
    {
        gnss_mode = SC_GNSS_MODE_GPS_BDS_GLONASS;
    }
#else
#ifdef FEATURE_SIMCOM_GPS_OVERSEAS
    if(strncmp(gnss_mode_status,GNSS_DEMO_MODE_GPS,sizeof(GNSS_DEMO_MODE_GPS)-1) == 0)
    {
        gnss_mode = SC_GNSS_MODE_GPS_L1_SBAS_QZSS;
    }
        if(strncmp(gnss_mode_status,GNSS_DEMO_MODE_BDS,sizeof(GNSS_DEMO_MODE_BDS)-1) == 0)
    {
        gnss_mode = SC_GNSS_MODE_BDS;
    }
    if(strncmp(gnss_mode_status,GNSS_DEMO_MODE_GPS_GLONASS_GALILEO,sizeof(GNSS_DEMO_MODE_GPS_GLONASS_GALILEO)-1) == 0)
    {
        gnss_mode = SC_GNSS_MODE_GPS_GLONASS_GALILEO_SBAS_QZSS;
    }
    if(strncmp(gnss_mode_status,GNSS_DEMO_MODE_GPS_BDS_GALILEO,sizeof(GNSS_DEMO_MODE_GPS_BDS_GALILEO)-1) == 0)
    {
        gnss_mode = SC_GNSS_MODE_GPS_BDS_GALILEO_SBAS_QZSS;
    }
#else
    if(strncmp(gnss_mode_status,GNSS_DEMO_MODE_GPS_BDS,sizeof(GNSS_DEMO_MODE_GPS_BDS)-1) == 0)
    {
        gnss_mode = SC_GNSS_MODE_GPS_BDS_QZSS;
    }
    if(strncmp(gnss_mode_status,GNSS_DEMO_MODE_BDS,sizeof(GNSS_DEMO_MODE_BDS)-1) == 0)
    {
        gnss_mode = SC_GNSS_MODE_BDS;
    }
    if(strncmp(gnss_mode_status,GNSS_DEMO_MODE_GPS,sizeof(GNSS_DEMO_MODE_GPS)-1) == 0)
    {
        gnss_mode = SC_GNSS_MODE_GPS_QZSS;
    }
#endif
#endif
    if(strncmp(gnss_mode_status,GET_GNSS_MODE,sizeof(GET_GNSS_MODE)-1) == 0)
    {
        isGetGnssMode = 1;
    }
    LOG_INFO("%s,gnss_mode:[%d],isGetGnssMode:%d", __func__,gnss_mode,isGetGnssMode);

    if(isGetGnssMode == 1)
    {
        memset(statusbuf, 0, sizeof(statusbuf));
        gnss_mode = sAPI_GnssModeGet();
        snprintf(statusbuf, sizeof(statusbuf),"gnss mode is %d.\r\n",gnss_mode);

        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = ARGGNSS_MODE;
        out_arg[0].value = pl_demo_make_value(0, statusbuf, strlen(statusbuf));

        ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

        pl_demo_release_value(out_arg[0].value);

        if (0 != ret.errcode)
        {
            ret.msg = "sAPI_GnssModeGet post data fail";
            goto EXIT;
        }
    }
    else
    {
        if (sAPI_GnssModeSet(gnss_mode) != SC_GNSS_RETURN_CODE_OK)
        {
            LOG_ERROR("sAPI_GnssModeSet fail!!!");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "sAPI_GnssModeSet fail!!!";
            goto EXIT;
        }
    }

EXIT:
    return ret;
}

static ret_msg_t gnss_nmea_rate_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_Gnss_Nmea_Rate gnss_nmea_rate = SC_GNSS_NMEA_UPDATE_RATE_1HZ;
    int isGetGnssNmeaRate = 0;
    const char *gnss_nmea_rate_status = NULL;
    char statusbuf[64] = {0};
    arg_t out_arg[1] = {0};

    pl_demo_get_data(&gnss_nmea_rate_status, argv, argn, ARGGNSS_NMEA_RATE);
    if (!gnss_nmea_rate_status)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    if(strncmp(gnss_nmea_rate_status,GNSS_DEMO_NMEA_RATE_1HZ,sizeof(GNSS_DEMO_NMEA_RATE_1HZ)-1) == 0)
    {
        gnss_nmea_rate = SC_GNSS_NMEA_UPDATE_RATE_1HZ;
    }
#if (defined SC_DRIVER_GNSS_ZKW) || (defined SC_DRIVER_GNSS_UNIC)
    if(strncmp(gnss_nmea_rate_status,GNSS_DEMO_NMEA_RATE_2HZ,sizeof(GNSS_DEMO_NMEA_RATE_2HZ)-1) == 0)
    {
        gnss_nmea_rate = SC_GNSS_NMEA_UPDATE_RATE_2HZ;
    }
    if(strncmp(gnss_nmea_rate_status,GNSS_DEMO_NMEA_RATE_5HZ,sizeof(GNSS_DEMO_NMEA_RATE_5HZ)-1) == 0)
    {
        gnss_nmea_rate = SC_GNSS_NMEA_UPDATE_RATE_5HZ;
    }
#endif
#ifdef SC_DRIVER_GNSS_ZKW
    if(strncmp(gnss_nmea_rate_status,GNSS_DEMO_NMEA_RATE_4HZ,sizeof(GNSS_DEMO_NMEA_RATE_4HZ)-1) == 0)
    {
        gnss_nmea_rate = SC_GNSS_NMEA_UPDATE_RATE_4HZ;
    }
        if(strncmp(gnss_nmea_rate_status,GNSS_DEMO_NMEA_RATE_10HZ,sizeof(GNSS_DEMO_NMEA_RATE_10HZ)-1) == 0)
    {
        gnss_nmea_rate = SC_GNSS_NMEA_UPDATE_RATE_10HZ;
    }
#endif
    if(strncmp(gnss_nmea_rate_status,GET_GNSS_NMEA_RATE,sizeof(GET_GNSS_NMEA_RATE)-1) == 0)
    {
        isGetGnssNmeaRate = 1;
    }
    LOG_INFO("%s,gnss_nmea_rate:[%d],isGetGnssNmeaRate:%d", __func__,gnss_nmea_rate,isGetGnssNmeaRate);

    if(isGetGnssNmeaRate == 1)
    {
        memset(statusbuf, 0, sizeof(statusbuf));
        gnss_nmea_rate = sAPI_GnssNmeaRateGet();
        snprintf(statusbuf, sizeof(statusbuf),"gnss nmea rate is %d.\r\n",gnss_nmea_rate);

        out_arg[0].type = TYPE_STR;
        out_arg[0].arg_name = ARGGNSS_NMEA_RATE;
        out_arg[0].value = pl_demo_make_value(0, statusbuf, strlen(statusbuf));

        ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

        pl_demo_release_value(out_arg[0].value);

        if (0 != ret.errcode)
        {
            ret.msg = "sAPI_GnssNmeaRateGet post data fail";
            goto EXIT;
        }
    }
    else
    {
        if (sAPI_GnssNmeaRateSet(gnss_nmea_rate) != SC_GNSS_RETURN_CODE_OK)
        {
            LOG_ERROR("sAPI_GnssNmeaRateSet fail!!!");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "sAPI_GnssNmeaRateSet fail!!!";
            goto EXIT;
        }
    }

EXIT:
    return ret;
}

static ret_msg_t gnss_nmea_sentence_set_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int gnss_nmea_sentence = 0;

    pl_demo_get_value(&gnss_nmea_sentence, argv, argn, ARGGNSS_NMEA_SENTENCE_SET);

    LOG_INFO("%s,gnss_nmea_sentence:[%d]", __func__,gnss_nmea_sentence);

#ifdef JACANA_GPS_SUPPORT
    if ((gnss_nmea_sentence < 0) || (gnss_nmea_sentence > 511))
#elif SC_DRIVER_GNSS_ZKW
    if ((gnss_nmea_sentence < 0) || (gnss_nmea_sentence > 13311))
#else //SIMCOM_UNICORECOMM_GNSS_SUPPORT
    if ((gnss_nmea_sentence < 0) || (gnss_nmea_sentence > 255))
#endif
    {
        ret.msg = "mask is error!";
        ret.errcode = ERRCODE_MASK;
        goto EXIT;
    }

    if (sAPI_GnssNmeaSentenceSet(gnss_nmea_sentence) != SC_GNSS_RETURN_CODE_OK)
    {
        LOG_ERROR("sAPI_GnssNmeaSentenceSet fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_GnssNmeaSentenceSet fail!!!";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t gnss_nmea_sentence_get_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    unsigned char *gnss_nmea_sentence = NULL;
    const char *gnss_nmea_sentence_status = NULL;
    char statusbuf[256] = {0};
    char buf[16] = {0};
    arg_t out_arg[1] = {0};
    int i = 0;
    pl_demo_get_data(&gnss_nmea_sentence_status, argv, argn, ARGGNSS_NMEA_SENTENCE_GET);
    if (!gnss_nmea_sentence_status)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    LOG_INFO("%s,gnss_nmea_rate:[%s]", __func__,gnss_nmea_sentence_status);


    memset(statusbuf, 0, sizeof(statusbuf));
    gnss_nmea_sentence = sAPI_GnssNmeaSentenceGet();
    #ifdef JACANA_GPS_SUPPORT
    for (i = 0; i < 9; i++)
    #elif SC_DRIVER_GNSS_ZKW
    for (i = 0; i < 14; i++)
    #else
    for (i = 0; i < 8; i++)
    #endif
    {
        memset(buf, 0, 16);
        snprintf(buf, 16, "bit%d:%d ", i, gnss_nmea_sentence[i]);
        strcat(statusbuf, buf);
    }

    out_arg[0].type = TYPE_STR;
    out_arg[0].arg_name = ARGGNSS_NMEA_SENTENCE_GET;
    out_arg[0].value = pl_demo_make_value(0, statusbuf, strlen(statusbuf));

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

    pl_demo_release_value(out_arg[0].value);

    if (0 != ret.errcode)
    {
        ret.msg = "sAPI_GnssNmeaSentenceGet post data fail";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t gnss_and_gps_info_get_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *gnss_and_info_get_status = NULL;
    int gnss_or_info_get_period = 0;

    pl_demo_get_value(&gnss_or_info_get_period, argv, argn, SET_INFO_TIMER);
    LOG_INFO("%s,gnss_or_info_get_period:[%d]", __func__,gnss_or_info_get_period);

    if ((gnss_or_info_get_period < 0) || (gnss_or_info_get_period > 255))
    {
        ret.msg = "time is error!";
        ret.errcode = ERRCODE_MASK;
        goto EXIT;
    }
    pl_demo_get_data(&gnss_and_info_get_status, argv, argn, ARGGNSS_AND_GPS_INFO_GET);
    if (!gnss_and_info_get_status)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    if(strncmp(gnss_and_info_get_status,GET_GPSINFO,sizeof(GET_GPSINFO)-1) == 0)
    {
        LOG_INFO("%s,gpsinfo get", __func__);
        if (sAPI_GpsInfoGet(gnss_or_info_get_period) != SC_GNSS_RETURN_CODE_OK)
        {
            LOG_ERROR("sAPI_GpsInfoGet fail!!!");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "sAPI_GpsInfoGet fail!!!";
            goto EXIT;
        }
    }

    if(strncmp(gnss_and_info_get_status,GET_GNSSINFO,sizeof(GET_GNSSINFO)-1) == 0)
    {
        LOG_INFO("%s,gnssinfo get", __func__);
        if (sAPI_GnssInfoGet(gnss_or_info_get_period) != SC_GNSS_RETURN_CODE_OK)
        {
            LOG_ERROR("sAPI_GnssInfoGet fail!!!");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "sAPI_GnssInfoGet fail!!!";
            goto EXIT;
        }
    }

EXIT:
    return ret;
}

static ret_msg_t gnss_cmd_send_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *gnssCmd = NULL;

    pl_demo_get_data(&gnssCmd, argv, argn, ARGGNSS_CMD_SEND);
    if (!gnssCmd)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    LOG_INFO("%s,gnssCmd:%s", __func__,gnssCmd);
    if (sAPI_SendCmd2Gnss((char *)gnssCmd) != SC_GNSS_RETURN_CODE_OK)
    {
        LOG_ERROR("sAPI_SendCmd2Gnss fail!!!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "sAPI_SendCmd2Gnss fail!!!";
        goto EXIT;
    }

EXIT:
    return ret;
}

static ret_msg_t gnss_agps_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int agpsReturn= 0;
    const char *gnss_agps_status = NULL;

    pl_demo_get_data(&gnss_agps_status, argv, argn, ARGGNSS_AGPS);
    if (!gnss_agps_status)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    if(strncmp(gnss_agps_status,AGPS_OPEN,sizeof(AGPS_OPEN)-1) == 0)
    {
        agpsReturn = sAPI_GnssAgpsSeviceOpen();
        if (agpsReturn != SC_GNSS_RETURN_CODE_OK)
        {
            LOG_ERROR("sAPI_GpsInfoGet fail!!!");
            ret.errcode = agpsReturn;
            ret.msg = "sAPI_GpsInfoGet fail!!!";
            goto EXIT;
        }
    }

EXIT:
    return ret;
}