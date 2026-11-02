/**
  ******************************************************************************
  * @file    demo_bt.c
  * @author  SIMCom OpenSDK Team
  * @brief
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "simcom_os.h"
#include "simcom_bt_stack.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "simcom_uart.h"
#include "uart_api.h"
#include <stdarg.h>
#include <stdint.h>
#include "userspaceConfig.h"
#include "ble_gatt_demo.h"

//spp can transmit 200bytes data everytime.
//spp transmit reliability is not guaranteed if out of range.
#define BT_SPP_MAX_FRAME_SIZE 200

//#define SC_EXT_ADV

#define qDebug(fmt, ...) sAPI_Debug("[%s,%d] "fmt, __func__, __LINE__, ##__VA_ARGS__)
#define DEMO_BT(fmt, ...) sAPI_Debug("[%s,%d] "fmt, __func__, __LINE__, ##__VA_ARGS__)

extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(const char *format, ...);

#ifdef HAS_DEMO_BT_STACK

typedef enum
{
    SC_BT_DEMO_OPEN                        = 1,
    SC_BT_DEMO_CLOSE                       = 2,
    SC_BT_DEMO_SET_ADDRESS                 = 3,
    SC_BT_DEMO_GET_ADDRESS                 = 4,
    SC_BT_DEMO_SET_NAME                    = 5,
    SC_BT_DEMO_GET_NAME                    = 6,
    SC_BT_DEMO_SCAN_START                  = 7,
    SC_BT_DEMO_SCAN_STOP                   = 8,
    SC_BT_DEMO_PAIR_DEVICE                 = 9,
    SC_BT_DEMO_UNPAIR_DEVICE               = 10,
    SC_BT_DEMO_GET_PARIED_DEVICE           = 11,
    SC_BT_DEMO_SPP_CONNECT                 = 12,
    SC_BT_DEMO_SPP_DISCONNECT              = 13,
    SC_BT_DEMO_SPP_SEND                    = 14,
    SC_BT_DEMO_RSSI_GET                    = 15,
    SC_BT_DEMO_PAIR_MODE_PASSKEY_ENTRY     = 16,
    SC_BT_DEMO_PAIR_MODE                   = 17,
    SC_BT_DEMO_PIN_REPLY                   = 18,

    SC_BT_LE_DEMO_SET_RAMDOM_ADDR          = 30,
    SC_BT_LE_DEMO_SET_ADV_PARAM            = 31,
    SC_BT_LE_DEMO_SET_ADV_DATA             = 32,
    SC_BT_LE_DEMO_REGISTER_SERVICE         = 33,
    SC_BT_LE_DEMO_ENABLE_ADV               = 34,
    SC_BT_LE_DEMO_DISABLE_ADV              = 35,
    SC_BT_LE_DEMO_ATT_SCAN                 = 36,
    SC_BT_LE_DEMO_INDICATE                 = 37,
    SC_BT_LE_DEMO_NOTIFY                   = 38,

    SC_BT_LE_DEMO_CONNECT                  = 39,
    SC_BT_LE_DEMO_DISCONNECT               = 40,
    SC_BT_LE_DEMO_MTU_REQUEST              = 41,
    SC_BT_LE_DEMO_SCAN_START               = 42,
    SC_BT_LE_DEMO_SCAN_STOP                = 43,
    SC_BT_LE_DEMO_READ_REQUEST             = 44,
    SC_BT_LE_DEMO_WRITE_REQUEST            = 45,
    SC_BT_LE_DEMO_WRITE_COMMAND            = 46,

    SC_BT_LE_DEMO_PAIRING_REQUSET          = 47,
    SC_BT_LE_DEMO_PAIRING_ENABLE           = 48,
    SC_BT_LE_DEMO_GET_PAIRING_INFO         = 49,
    SC_BT_LE_DEMO_REMOVE_PAIRING_RECORD    = 50,
    SC_BT_LE_DEMO_CLEAR_PAIRING_RECORD     = 51,
    SC_BT_LE_DEMO_SET_WHITE_LSIT           = 52,
    SC_BT_LE_DEMO_CLEAR_WHITE_LSIT         = 53,
    SC_BT_DEMO_MAX                         = 99,
}SC_BT_DEMO_TYPE;

typedef enum
{
    BT_DEMO_TYPE_SPP_DEVICE_A,
    BT_DEMO_TYPE_SPP_DEVICE_B,
} SC_BT_DEMO_TYPE_T;

char *bt_options_list[] =
{
    "1. open",
    "2. close",
    "3. set address",
    "4. get address",
    "5. set name",
    "6. get name",
    "7. scan start",
    "8. scan stop",
    "9. pair device",
    "10. unpair device",
    "11. get paired info",
    "12. SPP connect",
    "13. SPP disconnect",
    "14. SPP send",
    "15. Get RSSI",
    "16. Pair with passkey ",
    "17. pair mode",
    "18. PIN reply",

    "30. le set random address",
    "31. le set adv parameters",
    "32. le set adv data",
    "33. le register service",
    "34. le enable adv",
    "35. le disable adv",
    "36. le att scan",
    "37. le indicate",
    "38. le notify",
    "39. le connect",
    "40. le disconnect",
    "41. le MTU request",
    "42. le client scan",
    "43. le client scan stop",
    "44. le read request",
    "45. le write request",
    "46. le write request comamnd",
    "47. le pairing request",
    "48. le pairing enable",
    "49. le get pairing info",
    "50. le remove pairing info",
    "51. le clear pairing info",
    "52. le set white list",
    "53. le clear white list",
    "99. back",
};

static unsigned char bt_spp_port            = 0;
static unsigned short bt_spp_max_frame_size = 0;
static unsigned char scan_num = 0;

static const SC_BT_PAIR_MODE_T g_bt_pairing_mode[5][5] =
{
    {SC_BT_PAIR_MODE_JUST_WORK_MODE,    SC_BT_PAIR_MODE_JUST_WORK_MODE,          SC_BT_PAIR_MODE_PASSKEY_ENTRY_MODE, SC_BT_PAIR_MODE_JUST_WORK_MODE, SC_BT_PAIR_MODE_PASSKEY_ENTRY_MODE},
    {SC_BT_PAIR_MODE_JUST_WORK_MODE,    SC_BT_PAIR_MODE_NUMERIC_COMPARISON_MODE, SC_BT_PAIR_MODE_PASSKEY_ENTRY_MODE, SC_BT_PAIR_MODE_JUST_WORK_MODE, SC_BT_PAIR_MODE_NUMERIC_COMPARISON_MODE},
    {SC_BT_PAIR_MODE_PASSKEY_ENTRY_MODE,SC_BT_PAIR_MODE_PASSKEY_ENTRY_MODE,      SC_BT_PAIR_MODE_PASSKEY_ENTRY_MODE, SC_BT_PAIR_MODE_JUST_WORK_MODE, SC_BT_PAIR_MODE_PASSKEY_ENTRY_MODE},
    {SC_BT_PAIR_MODE_JUST_WORK_MODE,    SC_BT_PAIR_MODE_JUST_WORK_MODE,          SC_BT_PAIR_MODE_JUST_WORK_MODE,     SC_BT_PAIR_MODE_JUST_WORK_MODE, SC_BT_PAIR_MODE_JUST_WORK_MODE},
    {SC_BT_PAIR_MODE_PASSKEY_ENTRY_MODE,SC_BT_PAIR_MODE_NUMERIC_COMPARISON_MODE, SC_BT_PAIR_MODE_PASSKEY_ENTRY_MODE, SC_BT_PAIR_MODE_JUST_WORK_MODE, SC_BT_PAIR_MODE_NUMERIC_COMPARISON_MODE},
};

uint32_t ble_demo_min(uint32_t a, uint32_t b)
{
    return (a < b) ? a : b;
}

uint16_t ble_demo_little_endian_read_16(const uint8_t *buffer, int position)
{
    return (uint16_t)(((uint16_t) buffer[position]) | (((uint16_t)buffer[position + 1]) << 8));
}

uint32_t ble_demo_little_endian_read_32(const uint8_t * buffer, int position){
    return ((uint32_t) buffer[position]) | (((uint32_t)buffer[position+1]) << 8) | (((uint32_t)buffer[position+2]) << 16) | (((uint32_t) buffer[position+3]) << 24);
}

// att_read_callback helpers
uint16_t ble_demo_att_read_callback_handle_blob(const uint8_t *blob, uint16_t blob_size, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    if (buffer != NULL) {
        uint16_t bytes_to_copy = 0;
        if (blob_size >= offset) {
            bytes_to_copy = ble_demo_min(blob_size - offset, buffer_size);
            (void)memcpy(buffer, &blob[offset], bytes_to_copy);
        }
        return bytes_to_copy;
    } else {
        return blob_size;
    }
}

char *ble_demo_uuid128_to_str(const uint8_t *uuid)
{
    static char appbt_uuid128_to_str_buffer[32 + 4 + 1];
    int i;
    int j = 0;
    // after 4, 6, 8, and 10 bytes = XYXYXYXY-XYXY-XYXY-XYXY-XYXYXYXYXYXY, there's a dash
    const int dash_locations = (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9);
    for (i = 0; i < 16; i++)
    {
        snprintf(appbt_uuid128_to_str_buffer + i * 2 + j, 4, "%02x", uuid[i]);
        if (dash_locations & (1 << i))
        {
            snprintf(appbt_uuid128_to_str_buffer + i * 2 + 2 + j, 4, "-");
            j++;
        }
    }
    return appbt_uuid128_to_str_buffer;
}

char *ble_demo_print_properties(uint16_t properties)
{
    static char properties_log[100] = {0};
    int offset = 0;
    if ((properties & SC_BLE_ATT_CHARC_PROP_READ) == SC_BLE_ATT_CHARC_PROP_READ)
    {
        snprintf(properties_log + offset, strlen("read | ") + 1, "read | ");
        offset += strlen("read | ");
    }
    if ((properties & SC_BLE_ATT_CHARC_PROP_WWP) == SC_BLE_ATT_CHARC_PROP_WWP)
    {
        snprintf(properties_log + offset, strlen("write_without_response | ") + 1, "write_without_response | ");
        offset += strlen("write_without_response | ");
    }
    if ((properties & SC_BLE_ATT_CHARC_PROP_WRITE) == SC_BLE_ATT_CHARC_PROP_WRITE)
    {
        snprintf(properties_log + offset, strlen("write | ") + 1, "write | ");
        offset += strlen("write | ");
    }
    if ((properties & SC_BLE_ATT_CHARC_PROP_NOTIFY) == SC_BLE_ATT_CHARC_PROP_NOTIFY)
    {
        snprintf(properties_log + offset, strlen("notify | ") + 1, "notify | ");
        offset += strlen("notify | ");
    }
    if ((properties & SC_BLE_ATT_CHARC_PROP_INDICATE) == SC_BLE_ATT_CHARC_PROP_INDICATE)
    {
        snprintf(properties_log + offset, strlen("indicate | ") + 1, "indicate | ");
        offset += strlen("indicate | ");
    }
    snprintf(properties_log + offset - 3, 2, "~");
    return properties_log;
}

static unsigned short sc_demo_att_read_callback(
                                            unsigned short connection_handle,
                                            unsigned short att_handle,
                                            unsigned short offset,
                                            unsigned char *buffer,
                                            unsigned short buffer_size)
{

    qDebug("[DEMO]connection_handle: %d, att_handle: 0x%04x, offset: %d, buffer: 0x%x, buffer_size: %d", connection_handle, att_handle, offset, buffer, buffer_size);

    if (buffer == NULL)
    {
        qDebug("[DEMO][%s] used for check value len", __func__);
    }
    char buff[10] = "SIMCOM";
    return ble_demo_att_read_callback_handle_blob((const uint8_t *)buff, sizeof(buff), offset, buffer, buffer_size);
    //return 0;
}

unsigned short custom_value_client_configuration1 = 0;
static int sc_demo_att_write_callback(
                                   unsigned short connection_handle,
                                   unsigned short att_handle,
                                   unsigned short transaction_mode,
                                   unsigned short offset,
                                   unsigned char *buffer,
                                   unsigned short buffer_size)
{

    qDebug("[DEMO]connection_handle: %d, att_handle: 0x%04x, offset: %d, buffer_size: %d", connection_handle, att_handle, offset, buffer_size);
    custom_value_client_configuration1 = ble_demo_little_endian_read_16(buffer, 0);
    qDebug("[DEMO]custom_value_client_configuration1 %d", custom_value_client_configuration1);
    return 0;
}

static void bt_event_handle_cb(void *argv)
{
    char buffer[512] = {0};
    static SC_BT_ADDR_T temp_bt_address = {0};
    SC_BT_ADDR_T temp_bt_address_full_zero = {0};
    unsigned short spp_allow_receive_max = 0;
    SC_BT_TASK_EVENT_T *msg = (SC_BT_TASK_EVENT_T *)argv;

    qDebug("event_type:%d event_id:%d", msg->event_type, msg->event_id);
    switch(msg->event_type)
    {
        case SC_BT_EVENT_TYPE_COMMON:
        {
            switch(msg->event_id)
            {
                case SC_COMMON_INQUIRY_RESULT:
                {
                    SC_BT_EVENT_INQUIRY_T *inquiry = (SC_BT_EVENT_INQUIRY_T *)msg->payload;
                    sprintf(buffer,"\r\n[COMMON_INQUIRY_RESULT] name[%s] address(%02X:%02X:%02X:%02X:%02X:%02X) RSSI:%d\r\n",
                            inquiry->name,
                            inquiry->addr[5],inquiry->addr[4],inquiry->addr[3],
                            inquiry->addr[2],inquiry->addr[1],inquiry->addr[0],
                            inquiry->rssi);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                    temp_bt_address.bytes[0] = inquiry->addr[5];
                    temp_bt_address.bytes[1] = inquiry->addr[4];
                    temp_bt_address.bytes[2] = inquiry->addr[3];
                    temp_bt_address.bytes[3] = inquiry->addr[2];
                    temp_bt_address.bytes[4] = inquiry->addr[1];
                    temp_bt_address.bytes[5] = inquiry->addr[0];
                }
                break;
                case SC_COMMON_INQUIRY_COMPLETE:
                {
                    sprintf(buffer,"\r\n[COMMON_INQUIRY_COMPLETE]\r\n");
                    qDebug("%s",buffer);
                    if(memcmp(temp_bt_address.bytes, temp_bt_address_full_zero.bytes, 6) != 0)
                    {
                        memset(temp_bt_address.bytes, 0, 6);
                    }
                    PrintfResp(buffer);
                }
                break;
                case SC_COMMON_PAIRING_REQUEST:
                {
                    SC_BT_EVENT_PAIRING_REQUEST_T *pairing_request = (SC_BT_EVENT_PAIRING_REQUEST_T *)msg->payload;
                    SC_BT_IOCAP_T local_io_capability = 0;
                    SC_BT_IOCAP_T peer_io_capability = 0;
                    SC_BT_PAIR_MODE_T pair_mode;

                    sAPI_BtGetIoCap(&local_io_capability);
                    peer_io_capability = pairing_request->io_capability;
                    pair_mode = g_bt_pairing_mode[local_io_capability][peer_io_capability];
                    sprintf(buffer,"\r\n[COMMON_PAIRING_REQUEST] pairing request mode(%d)\r\n",pair_mode);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);

                    if(pair_mode == SC_BT_PAIR_MODE_JUST_WORK_MODE)
                    {
                        sAPI_TaskSleep(500 / 5);//avoid to fast
                        sAPI_BtPairAccept(SC_BT_PAIR_MODE_JUST_WORK_MODE, pairing_request->addr, 0, NULL);
                    }
                    else if(pair_mode == SC_BT_PAIR_MODE_NUMERIC_COMPARISON_MODE)
                    {
                        sAPI_TaskSleep(500 / 5);//avoid to fast
                        sAPI_BtPairAccept(SC_BT_PAIR_MODE_NUMERIC_COMPARISON_MODE, pairing_request->addr, 0, NULL);
                    }
                }
                break;
                case SC_COMMON_PIN_REQUEST:
                {
                    sprintf(buffer,"\r\n[COMMON_PIN_REQUEST]\r\n");
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_COMMON_USER_PASSKEY_NOTIFICATION:
                {
                    SC_BT_EVENT_USER_PASSKEY_NOTIFICATION_T *event = (SC_BT_EVENT_USER_PASSKEY_NOTIFICATION_T *)msg->payload;
                    sprintf(buffer,"\r\n[COMMON_USER_PASSKEY_NOTIFICATION] address(%02X:%02X:%02X:%02X:%02X:%02X) passkey:%d\r\n",
                            event->addr.bytes[5],event->addr.bytes[4],event->addr.bytes[3],
                            event->addr.bytes[2],event->addr.bytes[1],event->addr.bytes[0],
                            event->passkey);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                    sAPI_TaskSleep(500 / 5);//avoid to fast
                    sAPI_BtPairAccept(SC_BT_PAIR_MODE_JUST_WORK_MODE, event->addr, 1, NULL);
                }
                break;
                case SC_COMMON_USER_PASSKEY_REQUEST:
                {
                    SC_BT_EVENT_USER_PASSKEY_REQUEST_T *event = (SC_BT_EVENT_USER_PASSKEY_REQUEST_T *)msg->payload;
                    sprintf(buffer,"\r\n[COMMON_USER_PASSKEY_REQUEST] request passky(%02X:%02X:%02X:%02X:%02X:%02X)\r\n",
                            event->addr.bytes[5],event->addr.bytes[4],event->addr.bytes[3],
                            event->addr.bytes[2],event->addr.bytes[1],event->addr.bytes[0]);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_COMMON_POWERUP_COMPLETE:
                {
                    sprintf(buffer,"\r\n[COMMON_POWERUP_COMPLETE]\r\n");
                    qDebug("%s",buffer);
                    PrintfResp(buffer);

                }
                break;
                case SC_COMMON_POWERUP_FAILED:
                {
                    sprintf(buffer,"\r\n[COMMON_POWERUP_FAILED]\r\n");
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_COMMON_PAIRED:
                {
                    SC_BT_EVENT_PAIRED_T *paired = (SC_BT_EVENT_PAIRED_T *)msg->payload;
                    sprintf(buffer,"\r\n[COMMON_PAIRED] paired with name[%s] address(%02X:%02X:%02X:%02X:%02X:%02X)\r\n",
                        paired->name,
                        paired->addr[5],paired->addr[4],paired->addr[3],
                        paired->addr[2],paired->addr[1],paired->addr[0]);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_COMMON_SHUTDOWN_COMPLETE:
                {
                    sprintf(buffer,"\r\n[COMMON_SHUTDOWN_COMPLETE]\r\n");
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                    bt_spp_port = 0;
                    bt_spp_max_frame_size = 0;
                }
                break;
                case SC_COMMON_RSSI:
                {
                    signed char *value_rssi = (signed char *)msg->payload;
                    sprintf(buffer,"\r\n[COMMON_RSSI] Read RSSI(%d)\r\n",*value_rssi);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                default: break;
            }
        }
        break;

        case SC_BT_EVENT_TYPE_SPP:
        {
            switch(msg->event_id)
            {
                case SC_SPP_CONNECT_IND:
                {
                    SC_BT_EVENT_SPP_EVENT_T *connect = (SC_BT_EVENT_SPP_EVENT_T *)msg->payload;
                    if(connect->result == 0)
                    {
                        bt_spp_port = connect->port;
                        bt_spp_max_frame_size = connect->max_frame_size;
                        sprintf(buffer,"\r\n[SPP_CONNECT_IND] SPP connect success address(%02X:%02X:%02X:%02X:%02X:%02X),spp_port(%d) spp_max_frame_size(%d)\r\n",
                                    connect->addr[5],connect->addr[4],connect->addr[3],
                                    connect->addr[2],connect->addr[1],connect->addr[0],
                                    connect->port, connect->max_frame_size);
                        qDebug("%s",buffer);
                        PrintfResp(buffer);
                    }
                    else
                    {
                        sprintf(buffer,"\r\n[SPP_CONNECT_IND] SPP connect CNF result= %d,port=%d,spp_max_frame_size = %d\r\n",
                                    connect->result,connect->port, connect->max_frame_size);
                        qDebug("%s",buffer);
                        PrintfResp(buffer);
                    }
                }
                break;
                case SC_SPP_CONNECT_CNF:
                {
                    SC_BT_EVENT_SPP_EVENT_T *connect = (SC_BT_EVENT_SPP_EVENT_T *)msg->payload;
                    if(connect->result == 0)
                    {
                        sprintf(buffer,"\r\n[SPP_CONNECT_CNF] SPP connect success address(%02X:%02X:%02X:%02X:%02X:%02X),spp_port(%d) spp_max_frame_size(%d)\r\n",
                                    connect->addr[5],connect->addr[4],connect->addr[3],
                                    connect->addr[2],connect->addr[1],connect->addr[0],
                                    connect->port, connect->max_frame_size);
                        qDebug("%s",buffer);
                        PrintfResp(buffer);
                        bt_spp_port = connect->port;
                        bt_spp_max_frame_size = connect->max_frame_size;
                    }
                    else
                    {
                        sprintf(buffer,"\r\n[SPP_CONNECT_CNF] SPP connect CNF result= %d,port=%d,spp_max_frame_size = %d\r\n",connect->result,connect->port, connect->max_frame_size);
                        qDebug("%s",buffer);
                        PrintfResp(buffer);
                    }
                }
                break;
                case SC_SPP_DISCONNECT_IND:
                {
                    SC_BT_EVENT_SPP_EVENT_T *connect = (SC_BT_EVENT_SPP_EVENT_T *)msg->payload;
                    if(connect->result == 0)
                    {
                        bt_spp_port = 0;
                        bt_spp_max_frame_size = 0;
                    }
                    sprintf(buffer,"\r\n[SPP_DISCONNECT_IND] SPP disconnect IND result= %d\r\n",connect->result);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_SPP_DISCONNECT_CNF:
                {
                    SC_BT_EVENT_SPP_EVENT_T *connect = (SC_BT_EVENT_SPP_EVENT_T *)msg->payload;
                    if(connect->result == 0)
                    {
                        bt_spp_port = 0;
                        bt_spp_max_frame_size = 0;
                    }
                    sprintf(buffer,"\r\n[SPP_DISCONNECT_CNF] SPP disconnect CNF result= %d\r\n",connect->result);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_SPP_DATA_IND:
                {
                    SC_BT_EVENT_SPP_EVENT_T *connect = (SC_BT_EVENT_SPP_EVENT_T *)msg->payload;
                    spp_allow_receive_max = (bt_spp_max_frame_size <= BT_SPP_MAX_FRAME_SIZE) ? bt_spp_max_frame_size : BT_SPP_MAX_FRAME_SIZE;
                    if(connect->data_len <= spp_allow_receive_max)
                    {
                        char urc[600] = {0};
                        snprintf(buffer, connect->data_len + 1, "%s", connect->data);
                        sprintf(urc,"[SPP_DATA_IND] SPP recieve data result= %d,length[%d] data[%s]",connect->result,connect->data_len,buffer);
                        qDebug("%s",buffer);
                        PrintfResp(urc);
                    }
                    else
                    {
                        qDebug("[SPP_DATA_IND] received data too much, has exceeded the cache");
                        PrintfResp("\r\n[SPP_DATA_IND] received data too much, has exceeded the cache\r\n");
                    }
                }
                break;
                case SC_SPP_DATA_CNF:
                {
                    SC_BT_EVENT_SPP_EVENT_T *connect = (SC_BT_EVENT_SPP_EVENT_T *)msg->payload;
                    sprintf(buffer,"\r\n[SPP_DATA_CNF] SPP send data result= %d\r\n",connect->result);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                default:break;
            }
        }
        break;
        case SC_BT_EVENT_TYPE_LE:
        {
            switch(msg->event_id)
            {
                case SC_BLE_SCAN_EVENT:
                {
                    SC_BLE_EVENT_SCAN_EVENT_T *scan = (SC_BLE_EVENT_SCAN_EVENT_T *)(msg->payload);
                    int i = 0,j = 0;
                    char advData[63] = {0};

                    for(i = 0; i < scan->length; i ++)
                    {
                        j += sprintf(advData + j, "%02x",scan->data[i]);
                    }
                    if(scan->event_type != 0x04)
                    {
                        scan_num ++;
                        sprintf(buffer,"\r\n[LE_SCAN_EVENT] remote device addr(%02x:%02x:%02x:%02x:%02x:%02x),addrType(%d),advData[%s],rssi(%d)\r\n",
                                        scan->address.bytes[5],scan->address.bytes[4],scan->address.bytes[3],
                                        scan->address.bytes[2],scan->address.bytes[1],scan->address.bytes[0],
                                        scan->address_type, advData, scan->rssi);
                        qDebug("%s",buffer);
                        PrintfResp(buffer);
                        if(scan_num >= 60)
                        {
                            sAPI_BleScanStop();
                            scan_num = 0;
                        }
                    }
                    else
                    {
                        sprintf(buffer,"\r\n[LE_SCAN_EVENT] remote device addr(%02x:%02x:%02x:%02x:%02x:%02x),ScanRespData[%s],rssi(%d)\r\n",
                                        scan->address.bytes[5],scan->address.bytes[4],scan->address.bytes[3],
                                        scan->address.bytes[2],scan->address.bytes[1],scan->address.bytes[0],
                                        advData, scan->rssi);
                        qDebug("%s",buffer);
                        PrintfResp(buffer);
                    }
                }
                break;
                case SC_BLE_GATT_CONNECTED:
                {
                    SC_BLE_EVENT_ATT_CONNECTED_T *info = (SC_BLE_EVENT_ATT_CONNECTED_T *)msg->payload;
                    sprintf(buffer,"\r\n[LE_GATT_CONNECTED] connect with (%02x:%02x:%02x:%02x:%02x:%02x) acl_handle:0x%x\r\n",
                            info->addr.bytes[5],info->addr.bytes[4],info->addr.bytes[3],
                            info->addr.bytes[2],info->addr.bytes[1],info->addr.bytes[0],
                            info->acl_handle);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                    sAPI_BleGattScanStart(info->acl_handle);
                }
                break;
                case SC_BLE_GATT_DISCONNECTED:
                {
                    int *conn_handle = (int *)msg->payload;
                    sprintf(buffer,"\r\n[LE_GATT_DISCONNECTED] disconnect with device handler 0x%x\r\n",*conn_handle);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_BLE_MTU_EXCHANGED:
                {
                    SC_BLE_EVENT_MTU_EXCHANGE_EVENT_T *mtu = (SC_BLE_EVENT_MTU_EXCHANGE_EVENT_T *)msg->payload;
                    sprintf(buffer,"\r\n[LE_MTU_EXCHANGED] handle 0x%x,mtu %d\r\n",mtu->acl_handle,mtu->mut);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_BLE_CLIENT_HANDLE_NOTIFY:
                {
                    SC_BLE_EVENT_CLIENT_HANDLE_NOTIFY_T *notify = (SC_BLE_EVENT_CLIENT_HANDLE_NOTIFY_T *)msg->payload;
                    char data[256] = {0};
                    memcpy(data, notify->value, 255);
                    sprintf(buffer,"\r\n[LE_CLIENT_HANDLE_NOTIFY] connection_handle 0x%x,value_handle %d,data[%d][%s]\r\n",
                        notify->connection_handle,notify->value_handle,notify->value_length,data);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_BLE_CLIENT_HANDLE_INDIATION:
                {
                    SC_BLE_EVENT_CLIENT_HANDLE_INDICATION_T *event = (SC_BLE_EVENT_CLIENT_HANDLE_INDICATION_T *)msg->payload;
                    char data[256] = {0};
                    memcpy(data, event->value, 255);
                    sprintf(buffer,"\r\n[LE_CLIENT_HANDLE_INDIATION] connection_handle 0x%x,value_handle %d,data[%d][%s]\r\n",
                        event->connection_handle,event->value_handle,event->value_length,data);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_BLE_WHITE_LIST_SIZE:
                {
                    qDebug("[LE_WHITE_LIST_SIZE]");
                    PrintfResp("\r\n[LE_WHITE_LIST_SIZE]\r\n");
                }
                break;
                case SC_BLE_SMP_PASSKEY:
                {
                    SC_BLE_EVENT_SMP_PASSKEY_T *passkey = (SC_BLE_EVENT_SMP_PASSKEY_T *)msg->payload;
                    sprintf(buffer,"\r\n[LE_SMP_PASSKEY] passkey_value:%d %02X%02X%02X%02X%02X%02X\r\n",
                                passkey->passkey_value,
                                passkey->addr.bytes[5], passkey->addr.bytes[4],
                                passkey->addr.bytes[3], passkey->addr.bytes[2],
                                passkey->addr.bytes[1], passkey->addr.bytes[0]);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_BLE_ADV_PHY_TXPOWER:
                {
                    signed char *tx_power = (signed char *)msg->payload;
                    sprintf(buffer,"\r\n[LE_ADV_PHY_TXPOWER] handle TX power %d\r\n",*tx_power);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_BLE_SLAVE_LE_BOND_COMPLETE:
                {
                    SC_BLE_EVENT_BOND_COMPLETE_T *event = (SC_BLE_EVENT_BOND_COMPLETE_T *)msg->payload;
                    sprintf(buffer,"\r\n[LE_BOND_COMPLETE] type: %d, address: %02x%02x%02x%02x%02x%02x\r\n",
                                event->address_type,
                                event->address.bytes[5], event->address.bytes[4], event->address.bytes[3],
                                event->address.bytes[2], event->address.bytes[1], event->address.bytes[0]);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_BLE_ATT_EVENT_CAN_SEND_NOW:
                {
                    qDebug("[LE_ATT_EVENT_CAN_SEND_NOW]");
                    PrintfResp("\r\n[LE_ATT_EVENT_CAN_SEND_NOW]\r\n");
                }
                break;
                case SC_BLE_ATT_EVENT_CAN_WRITE_NOW:
                {
                    SC_BLE_EVENT_ATT_CAN_WRITE_NOW_T *event = (SC_BLE_EVENT_ATT_CAN_WRITE_NOW_T *)msg->payload;
                    sprintf(buffer,"\r\n[LE_ATT_EVENT_CAN_WRITE_NOW] conn: 0x%x, write_type: %d\r\n", event->connection_handle, event->write_type);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_BLE_IDENTITY_INFO:
                {
                    SC_BLE_EVENT_IDENTITY_INFO_EVENT_T *info = (SC_BLE_EVENT_IDENTITY_INFO_EVENT_T *)msg->payload;
                    if (info->success_flag == 1)
                    {
                        sprintf(buffer,"\r\n[LE_IDENTITY_INFO] identity success, index: %d, connection_handle: 0x%x addr: %02x%02x%02x%02x%02x%02x, addr_type: %d identity_addr: %02x%02x%02x%02x%02x%02x, identity_addr_type: %d\r\n",
                            info->index, info->connection_handle,
                            info->address.bytes[5], info->address.bytes[4],
                            info->address.bytes[3], info->address.bytes[2],
                            info->address.bytes[1], info->address.bytes[0],
                            info->address_type,
                            info->identity_address.bytes[5], info->identity_address.bytes[4],
                            info->identity_address.bytes[3], info->identity_address.bytes[2],
                            info->identity_address.bytes[1], info->identity_address.bytes[0],
                            info->identity_address_type);

                    }
                    else
                    {
                        sprintf(buffer,"\r\n[LE_IDENTITY_INFO] identity fail, index: %d, connection_handle: 0x%x addr: %02x%02x%02x%02x%02x%02x, addr_type: %d\r\n",
                            info->index, info->connection_handle,
                            info->address.bytes[5], info->address.bytes[4],
                            info->address.bytes[3], info->address.bytes[2],
                            info->address.bytes[1], info->address.bytes[0],
                            info->address_type);
                    }
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_BLE_DB_CONTROL_GET_INFO:
                {
                    SC_BLE_EVENT_GET_PAIR_INFO_T *info = (SC_BLE_EVENT_GET_PAIR_INFO_T *)msg->payload;
                    sprintf(buffer,"\r\n[LE_DB_CONTROL_GET_INFO] index: %d, seq: %d, valid count: %d, max count: %d,addr: %02x%02x%02x%02x%02x%02x, addr_type: %d\r\n",
                        info->index, info->seq, info->valid_count, info->max_count,
                        info->identity_address.bytes[5], info->identity_address.bytes[4],
                        info->identity_address.bytes[3], info->identity_address.bytes[2],
                        info->identity_address.bytes[1], info->identity_address.bytes[0],
                        info->identity_address_type);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_BLE_ATT_EVENT_READ_DATA_RESULT:
                {
                    SC_BLE_EVENT_ATT_READ_DATA_RESULT_T *event = (SC_BLE_EVENT_ATT_READ_DATA_RESULT_T *)msg->payload;
                    char *buf = NULL;
                    if(event->value_len > 0)
                    {
                        buf = sAPI_Malloc(event->value_len + 1);
                        memset(buf, 0, event->value_len + 1);
                        memcpy(buf, event->value, event->value_len);
                    }
                    sprintf(buffer,"\r\n[LE_ATT_EVENT_READ_DATA_RESULT] connection_handle: 0x%x, value_handle: 0x%x, value_offset: %d, value_length: %d data[%s]\r\n",
                                event->connection_handle, event->value_handle, event->value_offset, event->value_len, buf);
                    sAPI_Free(buf);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_BLE_ATT_EVENT_READ_OVER_RESULT:
                {
                    SC_BLE_EVENT_ATT_READ_OVER_RESULT_T *event = (SC_BLE_EVENT_ATT_READ_OVER_RESULT_T *)msg->payload;
                    sprintf(buffer,"\r\n[LE_ATT_EVENT_READ_OVER_RESULT] connection_handle: 0x%x, att_status: %d\r\n",event->connection_handle, event->att_status);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_BLE_GATT_SCAN_RESULT:
                {
                    SC_BLE_EVENT_GATT_SCAN_RESULT_T *result = (SC_BLE_EVENT_GATT_SCAN_RESULT_T *)msg->payload;
                    sprintf(buffer,"\r\n[LE_GATT_SCAN_RESULT] connection_handle: 0x%x, status: %d\r\n", result->connection_handle, result->status);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_BLE_GATT_SCAN_DUMP_SERVICE:
                {
                    SC_BLE_EVENT_GATT_SCAN_DUMP_SERVICE_T *result = (SC_BLE_EVENT_GATT_SCAN_DUMP_SERVICE_T *)msg->payload;
                    sprintf(buffer,"\r\n[LE_GATT_SCAN_RESULT] connection_handle: 0x%x\r\n", result->connection_handle);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                    SC_GATT_CLIENT_SERVICE_T *service = &result->service;
                    memset(buffer, 0 ,sizeof(buffer));
                    if (service->uuid16)
                    {
                        sprintf(buffer,"\r\nservice: [0x%x-0x%x], uuid 0x%x\r\n",
                                    service->start_group_handle, service->end_group_handle, service->uuid16);
                    }
                    else
                    {
                        sprintf(buffer,"\r\nservice: [0x%x-0x%x], uuid  %s\r\n", service->start_group_handle, service->end_group_handle, ble_demo_uuid128_to_str(service->uuid128));
                    }
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_BLE_GATT_SCAN_DUMP_CHARACTERISTIC:
                {
                    SC_BLE_EVENT_GATT_SCAN_DUMP_CHARACTERISTIC_T *result = (SC_BLE_EVENT_GATT_SCAN_DUMP_CHARACTERISTIC_T *)msg->payload;
                    sprintf(buffer,"\r\n[LE_GATT_SCAN_DUMP_CHARACTERISTIC] connection_handle: 0x%x\r\n", result->connection_handle);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                    memset(buffer, 0 ,sizeof(buffer));
                    SC_GATT_CLIENT_CHARACTERISTIC_T *characteristic = &result->characteristic;
                    if (characteristic->uuid16)
                    {
                        sprintf(buffer,"\r\ncharacteristic: [0x%x-0x%x-0x%x], properties 0x%x(%s), uuid 0x%x value_handle(0x%x)\r\n",
                                    characteristic->start_handle, characteristic->value_handle, characteristic->end_handle, characteristic->properties,
                                    ble_demo_print_properties(characteristic->properties), characteristic->uuid16, characteristic->value_handle);
                    }
                    else
                    {
                        sprintf(buffer,"\r\ncharacteristic: [0x%x-0x%x-0x%x], properties 0x%x(%s), uuid %s value_handle(0x%x)\r\n",
                                    characteristic->start_handle, characteristic->value_handle, characteristic->end_handle, characteristic->properties,
                                    ble_demo_print_properties(characteristic->properties),
                                    ble_demo_uuid128_to_str(characteristic->uuid128), characteristic->value_handle);
                    }
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_BLE_GATT_SCAN_DUMP_DESCRIPTOR:
                {
                    SC_BLE_EVENT_GATT_SCAN_DUMP_DESCRIPTOR_T *result = (SC_BLE_EVENT_GATT_SCAN_DUMP_DESCRIPTOR_T *)msg->payload;
                    sprintf(buffer,"\r\n[LE_GATT_SCAN_DUMP_DESCRIPTOR] connection_handle: 0x%x\r\n", result->connection_handle);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                    memset(buffer, 0 ,sizeof(buffer));
                    SC_GATT_CLIENT_CHARACTERISTIC_DESCRIPTOR_T *descriptor = &result->descriptor;
                    if (descriptor->uuid16)
                    {
                        sprintf(buffer,"\r\ndescriptor, handle: 0x%x, uuid 0x%x\r\n", descriptor->handle,
                        descriptor->uuid16);
                    }
                    else
                    {
                       sprintf(buffer,"\r\ndescriptor, handle: 0x%x, uuid %s\r\n", descriptor->handle, ble_demo_uuid128_to_str(descriptor->uuid128));
                    }
                    qDebug("%s",buffer);
                    PrintfResp(buffer);

                }
                break;
                case SC_BLE_CONNECTION_UPDATE_COMPLETE:
                {
                    SC_BLE_EVENT_CONNECTION_PARAMETER_UPDATE_T *param = (SC_BLE_EVENT_CONNECTION_PARAMETER_UPDATE_T *)msg->payload;
                    sprintf(buffer,"\r\n[LE_CONNECTION_UPDATE_COMPLETE] status: %d, conn_handle: 0x%x, interval: %d, latency: %d, timeout: %d\r\n",
                                 param->status, param->conn_handle, param->conn_interval, param->slave_latency, param->supervision_timeout);
                    qDebug("%s",buffer);
                    PrintfResp(buffer);
                }
                break;
                case SC_BLE_ATT_INDICATION_COMPLETE:
                {
                    qDebug("[LE_ATT_INDICATION_COMPLETE]\n");
                    PrintfResp("\r\n[LE_ATT_INDICATION_COMPLETE]\r\n");
                }
                break;
                case SC_BLE_ENCRYPTION_CHANGE:
                {
                    qDebug("[LE_ENCRYPTION_CHANGE]\n");
                    PrintfResp("\r\n[LE_ENCRYPTION_CHANGE]\r\n");
                }
                break;
                default: break;
            }
            break;
        }
        default:break;
    }
}

void BTDemo(void)
{
    int result = 0;
    SC_BT_ADDR_T address = {0};
    char urc[500] = {0};
    char name[256] = {0};
    UINT8 nameLen = 0;
    UINT32 opt = 0;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";

    while(1)
    {
        PrintfResp(note);
        PrintfOptionMenu(bt_options_list,sizeof(bt_options_list)/sizeof(bt_options_list[0]));
        opt = UartReadValue();
        result = 0;
        memset(urc, 0, 500);
        if(opt == 99)
            return;
        switch(opt)
        {
            case SC_BT_DEMO_OPEN:
            {
                int mode = 0;
                int io_cap = 0;
                PrintfResp("\r\nPlease input io_capability(0-3):\r\n");
                io_cap = UartReadValue();
                PrintfResp("\r\nPlease input mode:\r\n");
                mode = UartReadValue();
                sAPI_BtRegisterEventHandle(bt_event_handle_cb);
                sAPI_BtSetIoCap(io_cap);
                result = sAPI_BtOpen(mode);
                if(result == 0)
                {
                    sAPI_Debug("open bt device success.");
                    PrintfResp("\r\nopen bt device success.\r\n");
                }
                else
                {
                    sAPI_Debug("open bt device fail. error code = %d", result);
                    sprintf(urc, "\r\nopen bt device fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_DEMO_CLOSE:
            {
                sAPI_BtClose();
                sAPI_Debug("close bt device finish.");
                PrintfResp("\r\nclose bt device finish.\r\n");
            }
            break;
            case SC_BT_DEMO_SET_ADDRESS:
            {
                char addrStr[14] = {0};
                int j = 0;
                PrintfResp("\r\nPlease input mac address, like XXXXXXXXXXXX\r\n");
                UartReadLine(addrStr, 14);
                for(int i = 0;i < 12;i += 2)
                {
                    char tmp[4] = {0};
                    memcpy(tmp,&addrStr[i],2);
                    address.bytes[j++] = strtoul(tmp,NULL,16);
                }
                result = sAPI_BtSetAddress(address);
                if(result == 0)
                {
                    sAPI_Debug("set address success.");
                    PrintfResp("\r\nset address success.\r\n");
                }
                else
                {
                    sAPI_Debug("set address fail. error code = %d", result);
                    sprintf(urc, "\r\nset address fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_DEMO_GET_ADDRESS:
            {
                result = sAPI_BtGetAddress(&address);
                if(result == 0)
                {
                    char buffer0[128] = {0};
                    sprintf(buffer0,"\r\nGet addr: \"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n",
                            address.bytes[0], address.bytes[1], address.bytes[2],
                            address.bytes[3], address.bytes[4], address.bytes[5]);
                    PrintfResp(buffer0);
                }
                else
                {
                    sAPI_Debug("get address fail. error code = %d", result);
                    sprintf(urc, "\r\nget address fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_DEMO_SET_NAME:
            {
                char deviceName[255] = {0};
                PrintfResp("\r\nPlease input device name\r\n");
                UartReadLine(deviceName, sizeof(deviceName));
                sAPI_BtSetName(deviceName, strlen(deviceName));
                sAPI_Debug("set name success %s.",deviceName);
                PrintfResp("\r\nset name success.\r\n");
            }
            break;
            case SC_BT_DEMO_GET_NAME:
            {
                result = sAPI_BtGetName(name, &nameLen);
                if(result == 0)
                {
                    sprintf(urc, "\r\nGet BT name success: %s\r\n", name);
                    PrintfResp(urc);
                    sAPI_Debug("Get BT name success.");
                }
                else
                {
                    sAPI_Debug("Get BT name fail. error code = %d", result);
                    sprintf(urc, "\r\nGet BT name fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_DEMO_SCAN_START:
            {
                int scan_duration = 0;
                int scan_response_num = 0;
                PrintfResp("\r\nPlease input scan_duration, (1-48)\r\n");
                scan_duration = UartReadValue();
                PrintfResp("\r\nPlease input scan_response_num (scan_response_num >= 0)\r\n");
                scan_response_num = UartReadValue();
                result = sAPI_BtScanStart(scan_duration,scan_response_num);
                if(result == 0)
                {
                    sAPI_Debug("BT scan start success.");
                    PrintfResp("\r\nBT scan start success.\r\n");
                }
                else
                {
                    sAPI_Debug("BT scan start fail. error code = %d", result);
                    sprintf(urc, "\r\nBT scan start fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_DEMO_SCAN_STOP:
            {
                result = sAPI_BtScanStop();
                if(result == 0)
                {
                    sAPI_Debug("BT scan stop success.");
                    PrintfResp("\r\nBT scan stop success.\r\n");
                }
                else
                {
                    sAPI_Debug("BT scan stop fail. error code = %d", result);
                    sprintf(urc, "\r\nBT scan stop fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_DEMO_PAIR_DEVICE:
            {
                char addrStr[14] = {0};
                int j = 0;
                PrintfResp("\r\nPlease input mac address, like XXXXXXXXXXXX\r\n");
                UartReadLine(addrStr, 14);
                for(int i = 0;i < 12;i += 2)
                {
                    char tmp[4] = {0};
                    memcpy(tmp,&addrStr[i],2);
                    address.bytes[j++] = strtoul(tmp,NULL,16);
                }
                result = sAPI_BtPairRequest(address);
                if(result == 0)
                {
                    sAPI_Debug("BT pair success.");
                    PrintfResp("\r\nBT pair success.\r\n");
                }
                else
                {
                    sAPI_Debug("BT pair failed. error code = %d", result);
                    sprintf(urc, "\r\nBT pair failed. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_DEMO_UNPAIR_DEVICE:
            {
                char addrStr[14] = {0};
                 int j = 0;
                 PrintfResp("\r\nPlease input address to unpair, like XXXXXXXXXXXX\r\n");
                 UartReadLine(addrStr, 14);
                 for(int i = 0;i < 12;i += 2)
                 {
                     char tmp[4] = {0};
                     memcpy(tmp,&addrStr[i],2);
                     address.bytes[j++] = strtoul(tmp,NULL,16);
                 }
                 result = sAPI_BtUnpair(address);
                 if(result == 0)
                 {
                     sAPI_Debug("BT unpair success.");
                     PrintfResp("\r\nBT unpair success.\r\n");
                 }
                 else
                 {
                     sAPI_Debug("BT unpair fail. error code = %d", result);
                     sprintf(urc, "\r\nBT unpair fail. error code = %d\r\n", result);
                     PrintfResp(urc);
                 }
             }
            break;
            case SC_BT_DEMO_GET_PARIED_DEVICE:
            {
                SC_BT_DEVICE_RECORD_T record = {0};
                for(int i = 0; i < 10; i++)
                {
                    result = sAPI_BtPairedQueryByIndex(i, &record);
                    if(result != 0)
                        break;
                    qDebug("bt device record index(%d) peer name[%s] address(%02X:%02X:%02X:%02X:%02X:%02X)",
                            i, record.name,
                            record.addr[5],record.addr[4],
                            record.addr[3],record.addr[2],
                            record.addr[1],record.addr[0]);
                    sprintf(urc, "\r\nbt device record index(%d) peer name[%s] address(%02X:%02X:%02X:%02X:%02X:%02X)\r\n",
                            i, record.name,
                            record.addr[5],record.addr[4],
                            record.addr[3],record.addr[2],
                            record.addr[1],record.addr[0]);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_DEMO_SPP_CONNECT:
            {
                char addrStr[14] = {0};
                 int j = 0;
                 PrintfResp("\r\nPlease input address to connect spp, like XXXXXXXXXXXX\r\n");
                 UartReadLine(addrStr, 14);
                 for(int i = 0;i < 12;i += 2)
                 {
                     char tmp[4] = {0};
                     memcpy(tmp,&addrStr[i],2);
                     address.bytes[j++] = strtoul(tmp,NULL,16);
                 }
                result = sAPI_BtSppConnect(address);
                if(result == 0)
                {
                    sAPI_Debug("connect bt link success.");
                    PrintfResp("\r\nconnect bt link success.\r\n");
                }
                else
                {
                    sAPI_Debug("connect bt link fail. error code = %d", result);
                    sprintf(urc, "\r\nconnect bt link fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_DEMO_SPP_DISCONNECT:
            {
                result = sAPI_BtSppDisconnect(bt_spp_port);
                if(result == 0)
                {
                    sAPI_Debug("disconnect bt link success.");
                    PrintfResp("\r\ndisconnect bt link success.\r\n");
                }
                else
                {
                    sAPI_Debug("disconnect bt link fail. error code = %d", result);
                    sprintf(urc, "\r\ndisconnect bt link fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_DEMO_SPP_SEND:
            {
                char buffer[500 + 2] = {0};
                int len = 0;
                int port = 0;
                PrintfResp("\r\nPlease input data length:\r\n");
                len = UartReadValue();
                PrintfResp("\r\nPlease input data to send:\r\n");
                UartReadLine(buffer, len + 2);
                PrintfResp("\r\nPlease input spp_port:\r\n");
                port = UartReadValue();
                sAPI_BtSppSend((unsigned char *)buffer, len, port);
            }
            break;
            case SC_BT_DEMO_RSSI_GET:
            {
                unsigned short handle = 0;
                PrintfResp("\r\nPlease input connection handle:\r\n");
                handle = UartReadValue();
                sAPI_BtReadRssi(handle);
            }
            break;
            case SC_BT_DEMO_PAIR_MODE_PASSKEY_ENTRY:
            {
                int passkey = 0;
                char addrStr[14] = {0};
                int j = 0;
                PrintfResp("\r\nPlease input address, like XXXXXXXXXXXX\r\n");
                UartReadLine(addrStr, 14);
                for(int i = 0;i < 12;i += 2)
                {
                    char tmp[4] = {0};
                    memcpy(tmp,&addrStr[i],2);
                    address.bytes[j++] = strtoul(tmp,NULL,16);
                }
                PrintfResp("\r\nPlease input passkey\r\n");
                passkey = UartReadValue();
                result = sAPI_BtPairAccept(SC_BT_PAIR_MODE_PASSKEY_ENTRY_MODE, address, passkey, NULL);
                if(result == 0)
                {
                    sAPI_Debug("PASSKEY_ENTRY_MODE pair success.");
                    PrintfResp("\r\nPASSKEY_ENTRY_MODE pair success.\r\n");
                }
                else
                {
                    sAPI_Debug("PASSKEY_ENTRY_MODE pair fail. error code = %d", result);
                    sprintf(urc, "\r\nPASSKEY_ENTRY_MODE pair fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_DEMO_PAIR_MODE:
            {
                unsigned short mode = 0;
                int result = 0;
                PrintfResp("\r\nPlease input pairing mode(0 PIN CODE/1 SSP):\r\n");
                mode = UartReadValue();
                sAPI_BtPairingMode(mode);
                if(result == 0)
                {
                    sAPI_Debug("set pairing mode success.");
                    PrintfResp("\r\nset pairing mode success.\r\n");
                }
                else
                {
                    sAPI_Debug("set pairing mode fail. error code = %d", result);
                    sprintf(urc, "\r\nset pairing mode fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_DEMO_PIN_REPLY:
            {
                unsigned short mode = 0;
                int result = 0;
                char addrStr[14] = {0};
                int j = 0;
                char *pin = NULL;
                int len = 0;
                UartReadLine(addrStr, 14);
                PrintfResp("\r\nPlease input reply mode(0 negative repaly/1 pin repaly):\r\n");
                mode = UartReadValue();
                PrintfResp("\r\nPlease input address, like XXXXXXXXXXXX\r\n");
                UartReadLine(addrStr, 14);
                for(int i = 0;i < 12;i += 2)
                {
                    char tmp[4] = {0};
                    memcpy(tmp,&addrStr[i],2);
                    address.bytes[j++] = strtoul(tmp,NULL,16);
                }
                if(mode == 1)
                {
                    PrintfResp("\r\nPlease input pin_code len\r\n");
                    len = UartReadValue();
                    if(len > 0)
                    {
                        pin = sAPI_Malloc(len + 2);
                        memset(pin, 0, len + 2);
                    }
                    PrintfResp("\r\nPlease input pin_code\r\n");
                    UartReadLine(pin, len + 2);
                    memset(pin + len, 0, 2);
                }
                result = sAPI_BtPairAccept(SC_BT_PAIR_MODE_PIN_CODE, address, 0, pin);
                if(result == 0)
                {
                    sAPI_Debug("pin reply success.");
                    PrintfResp("\r\npin reply success.\r\n");
                }
                else
                {
                    sAPI_Debug("pin reply fail. error code = %d", result);
                    sprintf(urc, "\r\npin reply fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_LE_DEMO_SET_RAMDOM_ADDR:
            {
                char addrStr[14] = {0};
                int j = 0;
                PrintfResp("\r\nPlease input random address, like XXXXXXXXXXXX\r\n");
                UartReadLine(addrStr, 14);
                for(int i = 0;i < 12;i += 2)
                {
                    char tmp[4] = {0};
                    memcpy(tmp,&addrStr[i],2);
                    address.bytes[j++] = strtoul(tmp,NULL,16);
                }
                result = sAPI_BleSetRandomAddress(&address);
                if(result == 0)
                {
                    sAPI_Debug("set random address success.");
                    PrintfResp("\r\nset random address success.\r\n");
                }
                else
                {
                    sAPI_Debug("set random address fail. error code = %d", result);
                    sprintf(urc, "\r\nset random address fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_LE_DEMO_SET_ADV_PARAM:
            {
#ifndef SC_EXT_ADV
                SC_BLE_TASK_ADV_PARAMETERS_T param = {0};
                memset(&param, 0, sizeof(SC_BLE_TASK_ADV_PARAMETERS_T));
                param.interval_min = 400;
                param.interval_max = 400;
                param.advertising_type = SC_BLE_ADV_TYPE_IND;
                param.own_address_type = SC_BLE_ADDRESS_TYPE_RANDOM;
                result = sAPI_BleSetAdvParam(&param);
                if(result == 0)
                {
                    sAPI_Debug("set broadcast parameter success.");
                    PrintfResp("\r\nset broadcast parameter success.\r\n");
                }
                else
                {
                    sAPI_Debug("set broadcast parameter fail. error code = %d", result);
                    sprintf(urc, "\r\nset broadcast parameter fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
#else
                SC_BLE_TASK_EXT_ADV_PARAMETERS_T ext_param = {0};
                memset(&ext_param, 0, sizeof(SC_BLE_TASK_EXT_ADV_PARAMETERS_T));
                ext_param.properties = 0x41;
                ext_param.interval_max = 400;
                ext_param.interval_min = 300;
                ext_param.channel = 0x07;//37 & 38 & 39
                ext_param.own_address_type = SC_BLE_ADDRESS_TYPE_RANDOM;
                ext_param.tx_power = 8;
                ext_param.primary_phy = 0x3;
                ext_param.secondary_phy = 0x3;
                ext_param.adv_random_addr.bytes[0] = 0XA0;
                ext_param.adv_random_addr.bytes[1] = 0X11;
                ext_param.adv_random_addr.bytes[2] = 0X22;
                ext_param.adv_random_addr.bytes[3] = 0X33;
                ext_param.adv_random_addr.bytes[4] = 0X44;
                ext_param.adv_random_addr.bytes[5] = 0XB5;
                ext_param.adv_mode = SC_BLE_EXTENDED_ADV_AUX_MODE;
                result = sAPI_BleSetExtAdvParam(&ext_param);
                if(result == 0)
                {
                    sAPI_Debug("set ext_adv parameter success.");
                    PrintfResp("\r\nset ext_adv parameter success.\r\n");
                }
                else
                {
                    sAPI_Debug("set ext_adv parameter fail. error code = %d", result);
                    sprintf(urc, "\r\nset ext_adv parameter fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
#endif
            }
            break;
            case SC_BT_LE_DEMO_SET_ADV_DATA:
            {
                int advDataSize = 0;
                int advRspDataSize = 0;
#ifdef SC_EXT_ADV
                unsigned char advData[255] = {0};
                unsigned char advRspData[255] = {0};
#else
                unsigned char advData[31] = {0};
                unsigned char advRspData[31] = {0};
#endif
                unsigned char advData_type_Flags = 0;

                advData_type_Flags = 0x18;
                advDataSize = 0;
                memset(advData, 0, sizeof(advData));
                result = sAPI_BleCreateAdvData(SC_BLE_ADV_DATA_TYPE_FLAG, advData, sizeof(advData), &advData_type_Flags, 1);
                if(result == -1)
                {
                    advDataSize = -1;
                    sAPI_Debug("create adv data fail. error code = %d", result);
                    sprintf(urc, "\r\ncreate adv data fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                    break;
                }
                advDataSize += result;

                char *buff = "SIMCOM BLE DEMO";
                result = sAPI_BleCreateAdvData(SC_BLE_ADV_DATA_TYPE_COMPLETE_NAME, &advData[advDataSize], sizeof(advData)-advDataSize, (unsigned char *)buff, strlen(buff));
                if(result == -1)
                {
                    advDataSize = -1;
                    sAPI_Debug("create adv data fail. error code = %d", result);
                    sprintf(urc, "\r\ncreate adv data fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                    break;
                }
                advDataSize += result;
#ifndef SC_EXT_ADV
                result = sAPI_BleSetAdvData(advData, advDataSize);
                if(result == 0)
                {
                    sAPI_Debug("set adv data success.");
                    PrintfResp("\r\nset adv data success.\r\n");
                }
                else
                {
                    sAPI_Debug("set adv data fail. error code = %d", result);
                    sprintf(urc, "\r\nset adv data fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
                char *resp = "test response";

                result = sAPI_BleCreateAdvData(SC_BLE_ADV_DATA_TYPE_SPECIFIC_DATA, &advRspData[advRspDataSize],
                sizeof(advRspData)-advRspDataSize,(unsigned char *)resp, strlen(resp));
                result = sAPI_BleSetScanResponseData((unsigned char *)advRspData,result);
#else
                char *data = "WEIYUANTESTEXTADV0123456789";
                result = sAPI_BleCreateAdvData(SC_BLE_ADV_DATA_TYPE_SPECIFIC_DATA, &advData[advDataSize], sizeof(advData)-advDataSize, (unsigned char *)data, strlen(data));
                if(result == -1)
                {
                    advDataSize = -1;
                    sAPI_Debug("create adv data fail. error code = %d", result);
                    sprintf(urc, "\r\ncreate adv data fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                    break;
                }
                advDataSize += result;
                result = sAPI_BleSetExtAdvData(advData, advDataSize);
                if(result == 0)
                {
                    sAPI_Debug("set ext adv data success.");
                    PrintfResp("\r\nset ext adv data success.\r\n");
                }
                else
                {
                    sAPI_Debug("set ext adv data fail. error code = %d", result);
                    sprintf(urc, "\r\nset ext adv data fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
                unsigned char resp[50] = {0};
                memset(resp, 0xA5, 50);
                result = sAPI_BleCreateAdvData(SC_BLE_ADV_DATA_TYPE_SPECIFIC_DATA, &advRspData[advRspDataSize], sizeof(advRspData)-advRspDataSize, resp,50);
                result = sAPI_BleSetExtScanResponseData((unsigned char *)advRspData,result);
#endif
                if(result == 0)
                {
                    sAPI_Debug("set adv resp data success.");
                    PrintfResp("\r\nset adv resp data success.\r\n");
                }
                else
                {
                    sAPI_Debug("set adv resp data fail. error code = %d", result);
                    sprintf(urc, "\r\nset adv resp data fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_LE_DEMO_REGISTER_SERVICE:
                result = sAPI_BleRegisterAttServer(profile_data,sizeof(profile_data),sc_demo_att_read_callback,sc_demo_att_write_callback);
                if(result == 0)
                {
                    sAPI_Debug("service register success.");
                    PrintfResp("\r\nservice register success.\r\n");
                }
                else
                {
                    sAPI_Debug("service register fail. error code = %d", result);
                    sprintf(urc, "\r\nservice register fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
                break;
            case SC_BT_LE_DEMO_ENABLE_ADV:
            {
#ifndef SC_EXT_ADV
                result = sAPI_BleEnableAdv();
#else
                result = sAPI_BleEnableExtAdv();
#endif
                if(result == 0)
                {
                    sAPI_Debug("enable adv success.");
                    PrintfResp("\r\nenable adv success.\r\n");
                }
                else
                {
                    sAPI_Debug("enable adv fail. error code = %d", result);
                    sprintf(urc, "\r\nenable adv fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_LE_DEMO_DISABLE_ADV:
            {
#ifndef SC_EXT_ADV
                result = sAPI_BleDisableAdv();
#else
                result = sAPI_BleDisableExtAdv();
#endif
                if(result == 0)
                {
                    sAPI_Debug("disable adv success.");
                    PrintfResp("\r\ndisable adv success.\r\n");
                }
                else
                {
                    sAPI_Debug("disable adv fail. error code = %d", result);
                    sprintf(urc, "\r\ndisable adv fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_LE_DEMO_ATT_SCAN:
            {
                unsigned short handle = 0;
                PrintfResp("\r\nPlease input connect handle\r\n");
                handle = UartReadValue();
                result = sAPI_BleGattScanStart(handle);
                if(result == 0)
                {
                    sAPI_Debug("scan ATT success.");
                    PrintfResp("\r\nscan ATT success.\r\n");
                }
                else
                {
                    sAPI_Debug("scan ATT fail. error code = %d", result);
                    sprintf(urc, "\r\nscan ATT fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_LE_DEMO_INDICATE:
            {
                char *data = NULL;
                int dataLen = 0;
                unsigned short handle = 0, att_handle = 0;
                PrintfResp("\r\nPlease input connect handle\r\n");
                handle = UartReadValue();
                PrintfResp("\r\nPlease input att handle\r\n");
                att_handle = UartReadValue();
                PrintfResp("\r\nPlease input data length\r\n");
                dataLen = UartReadValue();
                if(dataLen > 0)
                    data = sAPI_Malloc(dataLen + 2);
                PrintfResp("\r\nPlease input indicate data\r\n");
                UartReadLine(data, dataLen + 2);
                result = sAPI_BleIndicate(handle, att_handle ,(unsigned char *)data, dataLen);
                if(result == 0)
                {
                    sAPI_Debug("indicate send success.");
                    PrintfResp("\r\nindicate send success.\r\n");
                }
                else
                {
                    sAPI_Debug("indicate send fail. error code = %d", result);
                    sprintf(urc, "\r\nindicate send fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_LE_DEMO_NOTIFY:
            {
                char *data = NULL;
                int length = 0;
                unsigned short handle = 0, att_handle = 0;
                PrintfResp("\r\nPlease input connect handle\r\n");
                handle = UartReadValue();
                PrintfResp("\r\nPlease input att handle\r\n");
                att_handle = UartReadValue();
                PrintfResp("\r\nPlease input data length\r\n");
                length = UartReadValue();
                if(length > 0)
                    data = sAPI_Malloc(length + 2);
                PrintfResp("\r\nPlease input notify data\r\n");
                UartReadLine(data, length + 2);

                result = sAPI_BleNotify(handle, att_handle ,(unsigned char *)data,length);
                if(result == 0)
                {
                    sAPI_Debug("notify send success.");
                    PrintfResp("\r\nnotify send success.\r\n");
                }
                else
                {
                    sAPI_Debug("notify send fail. error code = %d", result);
                    sprintf(urc, "\r\nnotify send fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_LE_DEMO_CONNECT:
            {
                char addrStr[14] = {0};
                int j = 0;
                int type = 0;
                PrintfResp("\r\nPlease input address, like XXXXXXXXXXXX\r\n");
                UartReadLine(addrStr, 14);
                PrintfResp("\r\nPlease input addr type\r\n");
                type = UartReadValue();
                for(int i = 0;i < 12;i += 2)
                {
                    char tmp[4] = {0};
                    memcpy(tmp,&addrStr[i],2);
                    address.bytes[j++] = strtoul(tmp,NULL,16);
                }
                result = sAPI_BleConnect(&address, type);
                if(result == 0)
                {
                    sAPI_Debug("request to connect with device.");
                    PrintfResp("\r\nrequest to connect with device.\r\n");
                }
                else
                {
                    sAPI_Debug("fail to connect with device. error code = %d", result);
                    sprintf(urc, "\r\nfail to connect with device . error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_LE_DEMO_DISCONNECT:
            {
                unsigned short handle = 0;
                PrintfResp("\r\nPlease input connect handle\r\n");
                handle = UartReadValue();
                result = sAPI_BleDisconnect(handle);
                if(result == 0)
                {
                    sAPI_Debug("disconnect ble link success.");
                    PrintfResp("\r\ndisconnect ble link success.\r\n");
                }
                else
                {
                    sAPI_Debug("disconnect ble link fail. error code = %d", result);
                    sprintf(urc, "\r\ndisconnect ble link fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_LE_DEMO_MTU_REQUEST:
            {
                unsigned short mtu = 0;
                unsigned short handle = 0;
                PrintfResp("\r\nPlease input connect handle\r\n");
                handle = UartReadValue();
                PrintfResp("\r\nPlease input mtu_size\r\n");
                mtu = UartReadValue();
                result = sAPI_BleMtuRequest(handle, mtu);
                if(result == 0)
                {
                    sAPI_Debug("mtu request success.");
                    PrintfResp("\r\nmtu request success.\r\n");
                }
                else
                {
                    sAPI_Debug("mtu request fail. error code = %d", result);
                    sprintf(urc, "\r\nmtu request fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;

            case SC_BT_LE_DEMO_SCAN_START:
            {
                unsigned char type = 0, own_addr_type = 0;;
                unsigned short interval = 0,window = 0;
                PrintfResp("\r\nPlease input scan type\r\n");
                type = UartReadValue();
                PrintfResp("\r\nPlease input interval\r\n");
                interval = UartReadValue();
                PrintfResp("\r\nPlease input window\r\n");
                window = UartReadValue();
                PrintfResp("\r\nPlease input own_addr_type\r\n");
                own_addr_type = UartReadValue();
                result = sAPI_BleScan(type,interval,window,own_addr_type);
                if(result == 0)
                {
                    sAPI_Debug("scan start success.");
                    PrintfResp("\r\nscan start success.\r\n");
                }
                else
                {
                    sAPI_Debug("scan start fail. error code = %d", result);
                    sprintf(urc, "\r\nscan start fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_LE_DEMO_SCAN_STOP:
            {
                result = sAPI_BleScanStop();
                if(result == 0)
                {
                    sAPI_Debug("client scan success.");
                    PrintfResp("\r\nclient scan success.\r\n");
                }
                else
                {
                    sAPI_Debug("client scan fail. error code = %d", result);
                    sprintf(urc, "\r\nclient scan fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_LE_DEMO_READ_REQUEST:
            {
                unsigned short handle = 0, att_handle = 0;
                PrintfResp("\r\nPlease input connect handle\r\n");
                handle = UartReadValue();
                PrintfResp("\r\nPlease input att handle\r\n");
                att_handle = UartReadValue();
                result = sAPI_BleReadRequest(handle, att_handle);
                if(result == 0)
                {
                    sAPI_Debug("read request success.");
                    PrintfResp("\r\nread request success.\r\n");
                }
                else
                {
                    sAPI_Debug("read request fail. error code = %d", result);
                    sprintf(urc, "\r\nread request fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_LE_DEMO_WRITE_REQUEST:
            {
                unsigned short handle = 0, att_handle = 0;
                unsigned char *data = NULL;
                int length = 0;
                PrintfResp("\r\nPlease input connect handle\r\n");
                handle = UartReadValue();
                PrintfResp("\r\nPlease input att handle\r\n");
                att_handle = UartReadValue();
                PrintfResp("\r\nPlease input data length\r\n");
                length = UartReadValue();
                if(length > 0)
                {
                    data =sAPI_Malloc(length);
                    PrintfResp("\r\nPlease input written data\r\n");
                    UartReadLine((char *)data, length);
                    result = sAPI_BleWriteRequest(handle, att_handle,data, length);
                    if(result == 0)
                    {
                        sAPI_Debug("write request success.");
                        PrintfResp("\r\nwrite request success.\r\n");
                    }
                    else
                    {
                        sAPI_Debug("write request fail. error code = %d", result);
                        sprintf(urc, "\r\nwrite request fail. error code = %d\r\n", result);
                        PrintfResp(urc);
                    }
                }
            }
            break;
            case SC_BT_LE_DEMO_WRITE_COMMAND:
            {
                unsigned short handle = 0, att_handle = 0;
                unsigned char *data = NULL;
                int length = 0;
                PrintfResp("\r\nPlease input connect handle\r\n");
                handle = UartReadValue();
                PrintfResp("\r\nPlease input att handle\r\n");
                att_handle = UartReadValue();
                PrintfResp("\r\nPlease input data length\r\n");
                length = UartReadValue();
                if(length > 0)
                {
                    data =sAPI_Malloc(length);
                    PrintfResp("\r\nPlease input written data\r\n");
                    UartReadLine((char *)data, length);
                    result = sAPI_BleWriteCommand(handle, att_handle,data, length);
                    if(result == 0)
                    {
                        sAPI_Debug("write request success.");
                        PrintfResp("\r\nwrite request success.\r\n");
                    }
                    else
                    {
                        sAPI_Debug("write request fail. error code = %d", result);
                        sprintf(urc, "\r\nwrite request fail. error code = %d\r\n", result);
                        PrintfResp(urc);
                    }
                }
            }
            break;
            case SC_BT_LE_DEMO_PAIRING_REQUSET:
            {
                unsigned short handle = 0;
                PrintfResp("\r\nPlease input connect handle\r\n");
                handle = UartReadValue();

                result = sAPI_BlePairRequest(handle);
                if(result == 0)
                {
                    sAPI_Debug("le pairing request success.");
                    PrintfResp("\r\nle pairing request success.\r\n");
                }
                else
                {
                    sAPI_Debug("le pairing request fail. error code = %d", result);
                    sprintf(urc, "\r\nle pairing request fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_LE_DEMO_PAIRING_ENABLE:
            {
                int enable = 0;
                PrintfResp("\r\nPlease input disable(0)/enable(1)\r\n");
                enable = UartReadValue();
                result = sAPI_BleSetPairEnable(enable);
                if(result == 0)
                {
                    sAPI_Debug("le enalbe to accpet pairing request success.");
                    PrintfResp("\r\nle enalbe to accpet pairing request success.\r\n");
                }
                else
                {
                    sAPI_Debug("le enalbe to accpet pairing request fail. error code = %d", result);
                    sprintf(urc, "\r\nle enalbe to accpet pairing request fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_LE_DEMO_GET_PAIRING_INFO:
            {
                int index = 0;
                PrintfResp("\r\nPlease input index ,max is 9\r\n");
                index = UartReadValue();
                result = sAPI_BleGetPairInfo(index);
                if(result == 0)
                {
                    sAPI_Debug("le get pairing information success.");
                    PrintfResp("\r\nle get pairing information success.\r\n");
                }
                else
                {
                    sAPI_Debug("le get pairing information fail. error code = %d", result);
                    sprintf(urc, "\r\nle get pairing information fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_LE_DEMO_REMOVE_PAIRING_RECORD:
            {
                int index = 0;
                PrintfResp("\r\nPlease input index ,max is 9\r\n");
                index = UartReadValue();
                sAPI_BleRemovePairInfo(index);
                sAPI_Debug("le remove pairing information success.");
                PrintfResp("\r\nle remove pairing information success.\r\n");
            }
            break;
            case SC_BT_LE_DEMO_CLEAR_PAIRING_RECORD:
            {
                #if 0
                result = sAPI_BleClearPairInfo();
                if(result == 0)
                {
                    sAPI_Debug("clear pairing information success.");
                    PrintfResp("\r\nclear pairing information success.\r\n");
                }
                else
                {
                    sAPI_Debug("clear pairing information fail. error code = %d", result);
                    sprintf(urc, "\r\nclear pairing information fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
                #endif
            }
            break;
            case SC_BT_LE_DEMO_SET_WHITE_LSIT:
            {
                char addrStr[14] = {0};
                int j = 0;
                int type = 0;
                PrintfResp("\r\nPlease input address, like XXXXXXXXXXXX\r\n");
                UartReadLine(addrStr, 14);
                PrintfResp("\r\nPlease input addr type\r\n");
                type = UartReadValue();
                for(int i = 0;i < 12;i += 2)
                {
                    char tmp[4] = {0};
                    memcpy(tmp,&addrStr[i],2);
                    address.bytes[j++] = strtoul(tmp,NULL,16);
                }
                result = sAPI_BleSetWhiteList(address, type);
                if(result == 0)
                {
                    sAPI_Debug("set device information to white list.");
                    PrintfResp("\r\nset device information to white list.\r\n");
                }
                else
                {
                    sAPI_Debug("set device information to white list failed. error code = %d", result);
                    sprintf(urc, "\r\nset device information to white list failed. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            case SC_BT_LE_DEMO_CLEAR_WHITE_LSIT:
            {
                result = sAPI_BleClearWhiteList();
                if(result == 0)
                {
                    sAPI_Debug("clear white list success.");
                    PrintfResp("\r\nclear white list success.\r\n");
                }
                else
                {
                    sAPI_Debug("clear white list fail. error code = %d", result);
                    sprintf(urc, "\r\nclear white list fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
            }
            break;
            default:break;
        }
    }
}

#endif

