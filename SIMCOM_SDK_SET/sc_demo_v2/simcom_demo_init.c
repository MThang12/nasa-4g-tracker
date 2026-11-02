/*
 * @Author: Louis_Qiu
 * @Date: 2023-11-07 10:39:51
 * @Last Modified by: Louis_Qiu
 * @Last Modified time: 2023-11-21 15:47:56
 */

#include "userspaceConfig.h"
#include "simcom_uart.h"
#include "uart_api.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_cache.h"
#include "demo_protocol_parser.h"
#include "simcom_demo_init.h"


#define LOG(...)       sal_log(__VA_ARGS__)
#define LOG_ERROR(...) sal_log_error("[DEMO-INIT] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-INIT] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-INIT] " __VA_ARGS__)


#define DEMO_STACK_SIZE (1024 * 60)
char stack_demo[DEMO_STACK_SIZE];
int demoTaskStackSize = DEMO_STACK_SIZE;
char *demoTaskStack = stack_demo;

const menu_list_t *root_menu_sub[] =
{
#ifdef SIMCOM_DEBUG
    &debug_menu,
#endif
#ifdef HAS_DEMO_FS
    &fs_menu,
#endif
#ifdef HAS_DEMO_TCPIP
    &tcpip_menu,
#endif
#ifdef HAS_CJSON
    &menu_cjson,
#endif
#ifdef HAS_DEMO_UART
    &uart_menu,
#endif
#ifdef HAS_DEMO_OTA
    &fota_menu,
#endif
#ifdef HAS_DEMO_APP_UPDATE
    &app_updater_menu,
#endif
#ifdef HAS_DEMO_WTD
    &WTD_menu,
#endif
#ifdef HAS_DEMO_RTC
    &rtc_menu,
#endif
#ifdef HAS_DEMO_NTP
    &ntp_menu,
#endif
#ifdef HAS_DEMO_SSL
    &ssl_menu,
#endif

#ifdef HAS_DEMO_SMS
    &sms_menu,
#endif
#ifdef HAS_DEMO_SJDR
    &sjdr_menu,
#endif
#ifdef HAS_DEMO_LBS
    &lbs_menu,
#endif

#ifdef HAS_DEMO_HTP
    &htp_menu,
#endif

#ifdef HAS_DEMO_PING
    &ping_menu,
#endif

#ifdef HAS_DEMO_TTS
    &tts_menu,
#endif
#ifdef HAS_DEMO_WIFI
    &wifi_menu,
#endif

#ifdef HAS_DEMO_HTTPS
    &https_menu,
#endif

#ifdef HAS_DEMO_FTPS
    &ftps_menu,
#endif

#ifdef HAS_DEMO_SIMCARD
    &Simcard_menu,
#endif

#ifdef HAS_DEMO_CALL
    &call_menu,
#endif
#ifdef HAS_DEMO_NETWORK
    &network_menu,
#endif

#ifdef HAS_DEMO_SPI
    &spi_menu,
    &extnorflash_menu,
    &extnandflash_menu,
#endif

#ifdef HAS_DEMO_SYS
    &system_menu,
#endif

#ifdef HAS_DEMO_I2C
    &i2c_menu,
#endif

#ifdef HAS_ZLIB
    &zlib_menu,
#endif

#ifdef HAS_DEMO_APP_DOWNLOAD
    &app_dl_menu,
#endif

#ifdef HAS_DEMO_GPIO
    &gpio_menu,
#endif

#ifdef HAS_DEMO_FLASH
    &flash_menu,
#endif

#ifdef HAS_DEMO_MQTTS
    &mqtts_menu,
#endif

#ifdef HAS_DEMO_PWM
    &pwm_menu,
#endif

#ifdef HAS_DEMO_PMU
    &pmu_menu,
#endif

#ifdef HAS_DEMO_BT
    &bt_menu,
#endif

#ifdef HAS_DEMO_BLE
    &ble_menu,
#endif

#ifdef HAS_DEMO_BT_STACK
    &bt_stack_menu,
#endif

#ifdef HAS_DEMO_GNSS
    &gnss_menu,
#endif

#ifdef HAS_DEMO_AUDIO
    &audio_menu,
#endif

#ifdef HAS_DEMO_POC
    &poc_menu,
#endif
    NULL  // MUST end by NULL
};

const menu_list_t root_menu =
{
    .menu_name = "ROOT",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = root_menu_sub,
};

void simcom_demo_init(void)
{
    demo_uart_init();
    pl_demo_protocol_init();
    pl_demo_register_root_menu(&root_menu);
}

