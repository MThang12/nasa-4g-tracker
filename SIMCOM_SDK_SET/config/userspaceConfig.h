#ifndef __USERSPACECONFIG_H__
#define __USERSPACECONFIG_H__


// general config
#define HAS_UART
#define HAS_URC
#define HAS_USB
#define SIMCOM_UI_DEMO_TO_UART1_PORT
/* #undef SIMCOM_UI_DEMO_TO_USB_AT_PORT */


/* #undef SIMCOM_DEBUG */
#define HAS_DEMO

// driver
#define HAS_DEMO_WTD
#define HAS_DEMO_PMU
#define HAS_DEMO_GPIO
#define HAS_DEMO_PWM
#define HAS_DEMO_UART
#define HAS_DEMO_USB
#define HAS_DEMO_I2C
#define HAS_DEMO_SPI
#define HAS_DEMO_FLASH
#define HAS_DEMO_OTA
#define HAS_DEMO_GNSS
/* #undef HAS_DEMO_LCD */
/* #undef HAS_DEMO_CAM */
#define HAS_DEMO_SYS
/* #undef HAS_DEMO_EXFLASHMOUNTFS */

// modem
#define HAS_DEMO_NETWORK
#define HAS_DEMO_SIMCARD
/* #undef HAS_DEMO_CALL */
#define HAS_DEMO_SMS
#define HAS_DEMO_LBS
#define HAS_DEMO_SJDR
#define HAS_DEMO_INTERNET_SERVICE

// app
#define HAS_DEMO_SSL
#define HAS_TCPIP
#define HAS_DEMO_TCPIP
#define HAS_DEMO_HTTPS
#define HAS_DEMO_FTPS
#define HAS_DEMO_MQTTS
#define HAS_DEMO_NTP
#define HAS_DEMO_HTP
/* #undef HAS_DEMO_AUDIO */
/* #undef HAS_DEMO_TTS */
/* #undef HAS_DEMO_POC */
#define HAS_DEMO_WIFI
#define HAS_DEMO_RTC
/* #undef HAS_DEMO_BLE */
/* #undef HAS_DEMO_BT */
/* #undef HAS_DEMO_BT_STACK */
#define HAS_FS
#define HAS_DEMO_FS
#define HAS_DEMO_FS_OLD
#define HAS_DEMO_APP_DOWNLOAD
#define HAS_DEMO_APP_UPDATE
#define HAS_DEMO_PING

// other libs
/* #undef HAS_ZLIB */
/* #undef HAS_MIRACL */
/* #undef HAS_SM2 */
#define HAS_CJSON
/* #undef HAS_MBEDTLS */


// custom switch
/* #undef PROJECT_YSZNBG */
/* #undef PROJECT_TBDTU */
/* #undef PROJECT_LYTBOX */
/* #undef PROJECT_TOPFLY */

#endif /* __USERSPACECONFIG.H_H__ */
