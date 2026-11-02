#ifndef __SIMCOM_DEMO_H__
#define __SIMCOM_DEMO_H__

#include "userspaceConfig.h"

typedef enum
{
    SC_DEMO_FOR_NETWORK = 0,
    SC_DEMO_FOR_SIMCARD,
    SC_DEMO_FOR_SMS,
    SC_DEMO_FOR_UART,
    SC_DEMO_FOR_USB,
    SC_DEMO_FOR_GPIO,
    SC_DEMO_FOR_PMU,
    SC_DEMO_FOR_I2C,
    SC_DEMO_FOR_AUDIO,
    SC_DEMO_FOR_FILE_SYSTEM,
    SC_DEMO_FOR_TCP_IP,
    SC_DEMO_FOR_HTTP_HTTPS,
    SC_DEMO_FOR_FTP_FTPS,
    SC_DEMO_FOR_MQTT_MQTTS,
    SC_DEMO_FOR_SSL,
    SC_DEMO_FOR_OTA,
    SC_DEMO_FOR_LBS,
    SC_DEMO_FOR_SJDR,
    SC_DEMO_FOR_NTP,
    SC_DEMO_FOR_HTP,
    SC_DEMO_FOR_INTERNET_SERVICE,
    SC_DEMO_FOR_TTS,
    SC_DEMO_FOR_CALL,
    SC_DEMO_FOR_WIFI,
    SC_DEMO_FOR_GNSS,
    SC_DEMO_FOR_LCD,
    SC_DEMO_FOR_RTC,
    SC_DEMO_FOR_FLASH,
    SC_DEMO_FOR_FILE_SYSTEM_OLD,
    SC_DEMO_FOR_SPI,
    SC_DEMO_FOR_SPI_NOR,
    SC_DEMO_FOR_SPI_NAND,
    SC_DEMO_FOR_CAM,
    SC_DEMO_FOR_BLE,
    SC_DEMO_FOR_BT,
    SC_DEMO_FOR_BTSTACK,
    SC_DEMO_FOR_APP_DOWNLOAD,
    SC_DEMO_FOR_APP_UPDATE_FOR_NVM,
    SC_DEMO_FOR_PWM,
    SC_DEMO_FOR_POC,
    SC_DEMO_FOR_WTD,
    SC_DEMO_FOR_PING,
    SC_DEMO_FOR_SM2,
    SC_DEMO_FOR_ZLIB,
    SC_DEMO_FOR_CJSON,
    SC_DEMO_FOR_SYS,
    SC_DEMO_FOR_MBEDTLS,
    SC_DEMO_MAX
} SC_DEMO_TYPE;

typedef struct
{
    char *description;
    void (*cb)(void);
} SC_DEMO_T;
extern void MqttDemo(void);
extern void NtpDemo(void);
extern void HtpDemo(void);
extern void NetWorkDemo(void);
extern void FsDemo(void);
extern void TcpipDemo(void);
extern void LcdDemo(void);
extern void CamDemo(void);
extern void AudioDemo(void);
extern void TTSDemo(void);
extern void FlashRWdemo(void);
extern void SpiDemo(void);
extern void SpiNorDemo(void);
extern void SpiNandDemo(void);
extern void GNSSDemo(void);
extern void UartDemo(void);
extern void I2cDemo(void);
extern void GpioDemo(void);
extern void CALLDemo(void);
extern void SMSDemo(void);
extern void SimcardDemo(void);
extern void PMUDemo(void);
extern void FotaDemo(void);
extern void SslDemo(void);
extern void AppDownloadDemo(void);
extern void FtpsDemo(void);
extern void HttpsDemo(void);
extern void AppUpdateDemo(void);
extern void PwmDemo(void);
extern void WIFIDemo(void);
extern void BLEDemo(void);
extern void BTDemo(void);
extern void POCDemo(void);
extern void LbsDemo(void);
extern void JamDectDemo(void);
extern void RTCDemo(void);
extern void WTDDemo(void);
extern void SM2Demo(void);
extern void ZLIBDemo(void);
extern void CjsonDemo(void);
extern void SysDemo(void);
extern void MbedTLSDemo(void);
extern void PingDemo(void);
extern void exflashmountfsDemo(void);

#endif /* __SIMCOM_DEMO_H__ */
