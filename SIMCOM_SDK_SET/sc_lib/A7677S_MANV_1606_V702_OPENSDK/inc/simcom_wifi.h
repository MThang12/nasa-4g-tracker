/*
 * Created on 2021-08-16 14:52:01
 *
 * @Author: yonghang.qin
 * @Date: 2021-08-16 14:52:01
 * @Last Modified by: yonghang.qin
 * @Last Modified time: 2021-08-16 15:02:04
 *
 * Copyright (c) 2021 simcom
 */

#ifndef __SIMCOM_WIFI_H__
#define __SIMCOM_WIFI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define SC_WIFI_INFO_NUMBERS_MAX (30)

enum
{
    SC_WIFI_EVENT_SCAN_STOP,
    SC_WIFI_EVENT_SCAN_INFO,
};

typedef struct
{
    unsigned char mac_addr[6];
    unsigned char padding[2];
    int rssi;
    unsigned int channel_number;
} SC_WIFI_INFO_T;

typedef struct
{
    unsigned char type;
    union
    {
        SC_WIFI_INFO_T info;
    };
} SC_WIFI_EVENT_T;

typedef struct SC_WIFI_SCANPARAM_S
{
    unsigned char round_number;     //from 1 to 3. default is 3.
    unsigned char max_bssid_number; //from 4 to 10。 default is 5
    unsigned char timeout;         // uint:second。 default is 25.
    unsigned char priority;         //the prioprity of PS and wifi. 0 is PS, 1 is wifi. default is 0.
} SC_WIFI_SCANPARAM_T;

/*
 * @name: sAPI_WifiSetHandler
 * @brief: set callback function: it will notise application after WIFI did all operations
 * @param: (*handler)(const SC_WIFI_EVENT_T *param)
 * @retval void
 */
void sAPI_WifiSetHandler(void (*handler)(const SC_WIFI_EVENT_T *param));

/*
 * @name: sAPI_WifiSetScanParam()
 * @brief: set parameters of scanning WIFI
 * @param: SC_WIFI_SCANPARAM_T
 * @retval void
 */
void sAPI_WifiSetScanParam(const SC_WIFI_SCANPARAM_T *param);

/*
 * @name: sAPI_WifiScanStart
 * @brief: start scanning WIFI network
 * @param: void
 * @retval void
 */
void sAPI_WifiScanStart(void);

/*
 * @name: sAPI_WifiScanStop
 * @brief: stop scanning WIFI network
 * @param: void
 * @retval void
 */
void sAPI_WifiScanStop(void);

#ifdef __cplusplus
}
#endif

#endif
