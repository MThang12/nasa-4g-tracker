/*** 
 * @Author: yangkui.zhang
 * @Date: 2023-12-20 15:32:10
 * @LastEditTime: 2024-01-10 14:33:47
 * @LastEditors: yangkui.zhang
 * @Description: 
 * @FilePath: \ASR160X\simcom\userspace\sc_demo_v2\simcom_demo_init.h
 * @no error,no warning
 */
#ifndef __SIMCOM_DEMO_INIT_H__
#define __SIMCOM_DEMO_INIT_H__

#include "userspaceConfig.h"
#include "demo_protocol_parser.h"


extern const menu_list_t root_menu;

#ifdef SIMCOM_DEBUG
extern const menu_list_t debug_menu;
#endif

#ifdef HAS_DEMO_FS
extern const menu_list_t fs_menu;
#endif

#ifdef HAS_DEMO_TCPIP
extern const menu_list_t tcpip_menu;
#endif

#ifdef HAS_CJSON
extern const menu_list_t menu_cjson;
#endif

#ifdef HAS_DEMO_UART
extern const menu_list_t uart_menu;
#endif

#ifdef HAS_DEMO_OTA
extern const menu_list_t fota_menu;
#endif

#ifdef HAS_DEMO_APP_UPDATE
extern const menu_list_t app_updater_menu;
#endif

#ifdef HAS_DEMO_WTD
extern const menu_list_t WTD_menu;
#endif

#ifdef HAS_DEMO_RTC
extern const menu_list_t rtc_menu;
#endif

#ifdef HAS_DEMO_NTP
extern const menu_list_t ntp_menu;
#endif

#ifdef HAS_DEMO_SSL
extern const menu_list_t ssl_menu;
#endif

#ifdef HAS_DEMO_SJDR
extern const menu_list_t sjdr_menu;
#endif
#ifdef HAS_DEMO_LBS
extern const menu_list_t lbs_menu;
#endif

#ifdef HAS_DEMO_HTP
extern const menu_list_t htp_menu;
#endif

#ifdef HAS_DEMO_PING
extern const menu_list_t ping_menu;
#endif

#ifdef HAS_DEMO_TTS
extern const menu_list_t tts_menu;
#endif

#ifdef HAS_DEMO_WIFI
extern const menu_list_t wifi_menu;
#endif

#ifdef HAS_DEMO_HTTPS
extern const menu_list_t https_menu;
#endif

#ifdef HAS_DEMO_SIMCARD
extern const menu_list_t Simcard_menu;
#endif

#ifdef HAS_DEMO_CALL
extern const menu_list_t call_menu;
#endif

#ifdef HAS_DEMO_NETWORK
extern const menu_list_t network_menu;
#endif

#ifdef HAS_DEMO_SMS
extern const menu_list_t sms_menu;
#endif

#ifdef HAS_DEMO_SPI
extern const menu_list_t spi_menu;
extern const menu_list_t extnorflash_menu;
extern const menu_list_t extnandflash_menu;
#endif

#ifdef HAS_DEMO_I2C
    extern const menu_list_t i2c_menu;
#endif

#ifdef HAS_DEMO_SYS
extern const menu_list_t system_menu;
#endif

#ifdef HAS_ZLIB
extern const menu_list_t zlib_menu;
#endif

#ifdef HAS_DEMO_APP_DOWNLOAD
extern const menu_list_t app_dl_menu;
#endif

#ifdef HAS_DEMO_GPIO
extern const menu_list_t gpio_menu;
#endif

#ifdef HAS_DEMO_FLASH
extern const menu_list_t flash_menu;
#endif

#ifdef HAS_DEMO_FTPS
extern const menu_list_t ftps_menu;
#endif

#ifdef HAS_DEMO_MQTTS
extern const menu_list_t mqtts_menu;
#endif

#ifdef HAS_DEMO_PWM
extern const menu_list_t pwm_menu;
#endif

#ifdef HAS_DEMO_PMU
extern const menu_list_t pmu_menu;
#endif

#ifdef HAS_DEMO_BT
extern const menu_list_t bt_menu;
#endif

#ifdef HAS_DEMO_BLE
extern const menu_list_t ble_menu;
#endif

#ifdef HAS_DEMO_BT_STACK
extern const menu_list_t bt_stack_menu;
#endif

#ifdef HAS_DEMO_GNSS
extern const menu_list_t gnss_menu;
#endif

#ifdef HAS_DEMO_AUDIO
extern const menu_list_t audio_menu;
#endif

#ifdef HAS_DEMO_POC
extern const menu_list_t poc_menu;
#endif

void simcom_demo_init(void);


#endif /* __SIMCOM_DEMO_INIT_H__ */
