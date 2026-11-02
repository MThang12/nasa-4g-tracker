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
#include "simcom_bt.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "simcom_uart.h"
#include "uart_api.h"



#ifdef BT_SUPPORT

typedef enum{
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
    SC_BT_DEMO_MAX                         = 99,
}SC_BT_DEMO_TYPE;

extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(const char *format, ...);

#define qDebug(fmt, ...) sAPI_Debug("[%s,%d] "fmt, __func__, __LINE__, ##__VA_ARGS__)

//spp can transmit 200bytes data everytime.
//spp transmit reliability is not guaranteed if out of range.
#define BT_SPP_MAX_FRAME_SIZE 200

typedef enum
{
    BT_DEMO_TYPE_SPP_DEVICE_A,
    BT_DEMO_TYPE_SPP_DEVICE_B,
} SC_BT_DEMO_TYPE_T;

static unsigned char bt_spp_port            = 0;
static unsigned short bt_spp_max_frame_size = 0;

static const bt_pair_mode_enum g_bt_pairing_mode[5][5] =
{
    {JUST_WORK_MODE, JUST_WORK_MODE, PASSKEY_ENTRY_MODE, JUST_WORK_MODE, PASSKEY_ENTRY_MODE},
    {JUST_WORK_MODE, NUMERIC_COMPARISON_MODE, PASSKEY_ENTRY_MODE, JUST_WORK_MODE, NUMERIC_COMPARISON_MODE},
    {PASSKEY_ENTRY_MODE, PASSKEY_ENTRY_MODE, PASSKEY_ENTRY_MODE, JUST_WORK_MODE, PASSKEY_ENTRY_MODE},
    {JUST_WORK_MODE, JUST_WORK_MODE, JUST_WORK_MODE, JUST_WORK_MODE, JUST_WORK_MODE},
    {PASSKEY_ENTRY_MODE, NUMERIC_COMPARISON_MODE, PASSKEY_ENTRY_MODE, JUST_WORK_MODE, NUMERIC_COMPARISON_MODE},
};

static void bt_event_handle_cb(void *argv)
{
    char buffer[300] = {0};
    static struct sc_bt_addr temp_bt_address = {0};
    struct sc_bt_addr temp_bt_address_full_zero = {0};
    struct sc_bt_event_inquiry *inquiry = NULL;
    struct sc_bt_event_spp_event *connect = NULL;
    struct sc_bt_event_pairing_request *pairing_request = NULL;
    bt_pair_mode_enum pair_mode = JUST_WORK_MODE;
    unsigned char local_io_capability = 0;
    unsigned char peer_io_capability = 0;
    unsigned short spp_allow_receive_max = 0;
    signed char *value_rssi;
    struct sc_bt_task_event *msg = (struct sc_bt_task_event *)argv;

    qDebug("event_type:%d event_id:%d", msg->event_type, msg->event_id);
    switch(msg->event_type)
    {
        case SC_BTTASK_IND_TYPE_COMMON:
            switch(msg->event_id)
            {
                case SC_BTTASK_IND_INQUIRY_RESULT:
                    inquiry = (struct sc_bt_event_inquiry *)msg->payload;
                    qDebug("inquiry result:");
                    qDebug("peer bt address(%02X:%02X:%02X:%02X:%02X:%02X) RSSI:%d",
                            inquiry->addr[5],inquiry->addr[4],inquiry->addr[3],
                            inquiry->addr[2],inquiry->addr[1],inquiry->addr[0],inquiry->rssi);
                    
                    temp_bt_address.bytes[0] = inquiry->addr[5];
                    temp_bt_address.bytes[1] = inquiry->addr[4];
                    temp_bt_address.bytes[2] = inquiry->addr[3];
                    temp_bt_address.bytes[3] = inquiry->addr[2];
                    temp_bt_address.bytes[4] = inquiry->addr[1];
                    temp_bt_address.bytes[5] = inquiry->addr[0];
                    sprintf(buffer,"\r\ninquiry result: %s \"%02X:%02X:%02X:%02X:%02X:%02X\" RSSI:%d\r\n",
                            inquiry->name,
                            inquiry->addr[5],inquiry->addr[4],inquiry->addr[3],
                            inquiry->addr[2],inquiry->addr[1],inquiry->addr[0],
                            inquiry->rssi);
                    PrintfResp(buffer);
                    //get RSSI in inquriy
                    break;
                case SC_BTTASK_IND_INQUIRY_COMPLETE:
                    qDebug("inquiry complete");
                    if(memcmp(temp_bt_address.bytes, temp_bt_address_full_zero.bytes, 6) != 0)
                    {
                        memset(temp_bt_address.bytes, 0, 6);
                    }
                    PrintfResp("\r\ninquiry complete\r\n");
                    break;
                case SC_BTTASK_IND_PAIRING_REQUEST:
                    pairing_request = (struct sc_bt_event_pairing_request *)msg->payload;
                    
                    sAPI_BTGetIOCap(&local_io_capability);
                    peer_io_capability = pairing_request->io_capability;
                    pair_mode = g_bt_pairing_mode[local_io_capability][peer_io_capability];

                    qDebug("pair request");
                    qDebug("pair mode(%d)", pair_mode);

                    if(pair_mode == JUST_WORK_MODE)
                    {
                        sAPI_TaskSleep(1000 / 5);//avoid to fast
                        sAPI_BTPairAccept(JUST_WORK_MODE, pairing_request->addr, 0);
                    }

                    if(pair_mode == NUMERIC_COMPARISON_MODE)
                    {
                        sAPI_TaskSleep(1000 / 5);//avoid to fast
                        sAPI_BTPairAccept(NUMERIC_COMPARISON_MODE, pairing_request->addr, 0);
                    }
                    break;
                case SC_BTTASK_IND_PAIRED:
                {
                    struct sc_bt_event_paired *paired = (struct sc_bt_event_paired *)msg->payload;
                    sprintf(buffer,"\r\npaired with %s(%02X:%02X:%02X:%02X:%02X:%02X)\r\n",paired->name,
                        paired->addr[5],paired->addr[4],paired->addr[3],
                        paired->addr[2],paired->addr[1],paired->addr[0]);
                    PrintfResp(buffer);
                    break;
                }
                case SC_BTTASK_IND_SHUTDOWN_COMPLETE:
                    qDebug("bluetooth close success.");
                    bt_spp_port = 0;
                    bt_spp_max_frame_size = 0;
                    break;
                case SC_BTTASK_IND_RSSI:
                    value_rssi = (signed char *)msg->payload;
                    qDebug("demo_bt:handle Read RSSI message %d\r\n", *value_rssi );
                    sprintf(buffer,"\r\nGet RSSI %d\r\n",*value_rssi);
                    PrintfResp(buffer);
                    break;
                default:break;
            }
            break;

        case SC_BTTASK_IND_TYPE_SPP:
            switch(msg->event_id)
            {
                case SC_BTTASK_IND_SPP_CONNECT_IND:
                    connect = (struct sc_bt_event_spp_event *)msg->payload;
                    if(connect->result == 0)
                    {
                        qDebug("spp connect success");
                        qDebug("peer bt address(%02X:%02X:%02X:%02X:%02X:%02X)",
                                    connect->addr[5],connect->addr[4],connect->addr[3],
                                    connect->addr[2],connect->addr[1],connect->addr[0]);
                        qDebug("spp_port(%d) spp_max_frame_size(%d)", connect->port, connect->max_frame_size);
                        bt_spp_port = connect->port;
                        bt_spp_max_frame_size = connect->max_frame_size;
                        //sAPI_BTSPPSend(bt_spp_device_b_send_data, strlen((const char *)bt_spp_device_b_send_data), bt_spp_port);
                        sprintf(buffer,"\r\nSPP connect IND result= %d,port=%d,spp_max_frame_size = %d\r\n",connect->result,connect->port, connect->max_frame_size);
                        PrintfResp(buffer);
                    }
                    break;
                case SC_BTTASK_IND_SPP_CONNECT_CNF:
                    connect = (struct sc_bt_event_spp_event *)msg->payload;
                    if(connect->result == 0)
                    {
                        qDebug("spp connect success");
                        qDebug("peer bt address(%02X:%02X:%02X:%02X:%02X:%02X)",
                                    connect->addr[5],connect->addr[4],connect->addr[3],
                                    connect->addr[2],connect->addr[1],connect->addr[0]);
                        qDebug("spp_port(%d) spp_max_frame_size(%d)", connect->port, connect->max_frame_size);
                        bt_spp_port = connect->port;
                        bt_spp_max_frame_size = connect->max_frame_size;
                        //sAPI_BTSPPSend(bt_spp_device_a_send_data, strlen((const char *)bt_spp_device_a_send_data), bt_spp_port);
                    }
                    sprintf(buffer,"\r\nSPP connect CNF result= %d,port=%d,spp_max_frame_size = %d\r\n",connect->result,connect->port, connect->max_frame_size);
                    PrintfResp(buffer);
                    break;
                case SC_BTTASK_IND_SPP_DISCONNECT_IND:
                    connect = (struct sc_bt_event_spp_event *)msg->payload;
                    if(connect->result == 0)
                    {
                        qDebug("spp disconnect success");
                        bt_spp_port = 0;
                        bt_spp_max_frame_size = 0;
                    }
                    sprintf(buffer,"\r\nSPP disconnect IND result= %d\r\n",connect->result);
                    PrintfResp(buffer);
                    break;
                case SC_BTTASK_IND_SPP_DISCONNECT_CNF:
                    connect = (struct sc_bt_event_spp_event *)msg->payload;
                    if(connect->result == 0)
                    {
                        qDebug("spp disconnect success");
                        bt_spp_port = 0;
                        bt_spp_max_frame_size = 0;
                    }
                    sprintf(buffer,"\r\nSPP disconnect CNF result= %d\r\n",connect->result);
                    PrintfResp(buffer);
                    break;
                case SC_BTTASK_IND_SPP_DATA_IND:
                    connect = (struct sc_bt_event_spp_event *)msg->payload;
                    spp_allow_receive_max = (bt_spp_max_frame_size <= BT_SPP_MAX_FRAME_SIZE) ? bt_spp_max_frame_size : BT_SPP_MAX_FRAME_SIZE;
                    qDebug("spp receive data");
                    qDebug("the length of received data is (%d)", connect->data_len);
                    if(connect->data_len <= spp_allow_receive_max)
                    {
                        char urc[500] = {0};
                        snprintf(buffer, connect->data_len+1, "%s", connect->data);
                        qDebug("%s", buffer);
                        sprintf(urc,"SPP recieve data result= %d,data=%s",connect->result,buffer);
                        PrintfResp(urc);
                    }
                    else
                    {
                        qDebug("received data too much, has exceeded the cache");
                       PrintfResp("\r\nreceived data too much, has exceeded the cache\r\n");
                    }
                    break;
                case SC_BTTASK_IND_SPP_DATA_CNF:
                    connect = (struct sc_bt_event_spp_event *)msg->payload;
                    sprintf(buffer,"\r\nSPP send data result= %d\r\n",connect->result);
                    PrintfResp(buffer);
                    if(connect->result == 0)
                        qDebug("spp send data success");
                    else
                        qDebug("spp send data failure");
                    break;
                default:break;
            }
            break;

        default:break;
    }
}

void BTDemo(void)
{
    int result = 0;
    struct sc_bt_addr address = {0};
    char urc[500] = {0};
    char name[256] = {0};
    UINT8 nameLen = 0;
    UINT32 opt = 0;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] =
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
        "99. back",
    };

    while(1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
        opt = UartReadValue();
        result = 0;
        memset(urc, 0, 500);
        if(opt == 99)
            return;
        switch(opt)
        {
            case SC_BT_DEMO_OPEN:
                sAPI_BTRegisterEventHandle(bt_event_handle_cb);
                sAPI_BTSetIOCap(NO_DISPALY_AND_NO_KEYBOARD);
                result = sAPI_BTOpen();
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
                break;
            case SC_BT_DEMO_CLOSE:
                sAPI_BTClose();
                sAPI_Debug("close bt device finish.");
                PrintfResp("\r\nclose bt device finish.\r\n");
                break;
            case SC_BT_DEMO_SET_ADDRESS:
            {
                char addrStr[13] = {0};
                unsigned char addr[6] = {0};
                int j = 6;
                PrintfResp("\r\nPlease input mac address, like XXXXXXXXXXXX\r\n");
                UartReadLine(addrStr, sizeof(addrStr));
                for(int i = 0;i < 12;i += 2)
                {
                    char tmp[4] = {0};
                    memcpy(tmp,&addrStr[i],2);
                    addr[--j] = strtoul(tmp,NULL,16);
                }
                result = sAPI_BTSetAddress(addr);
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
                break;
            }
            case SC_BT_DEMO_GET_ADDRESS:
                result = sAPI_BTGetAddress(address.bytes);
                if(result == 0)
                {
                    char buffer0[128] = {0};
                    sprintf(buffer0,"\r\n+Get addr: \"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n",
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
                break;
            case SC_BT_DEMO_SET_NAME:
            {
                char deviceName[255] = {0};
                PrintfResp("\r\nPlease input device name\r\n");
                UartReadLine(deviceName, sizeof(deviceName));
                sAPI_BTSetName(deviceName, strlen(deviceName));
                sAPI_Debug("set name success %s.",deviceName);
                PrintfResp("\r\nset name success.\r\n");
                break;
            }
            case SC_BT_DEMO_GET_NAME:
                result = sAPI_BTGetName(name, &nameLen);
                if(result == 0)
                {
                    sprintf(urc, "\r\ngGet BT name success. %s\r\n", name);
                    PrintfResp(urc);
                    sAPI_Debug("Get BT name success.");
                }
                else
                {
                    sAPI_Debug("Get BT name fail. error code = %d", result);
                    sprintf(urc, "\r\nGet BT name fail. error code = %d\r\n", result);
                    PrintfResp(urc);
                }
                break;
            case SC_BT_DEMO_SCAN_START:
            {
                int scan_duration = 0;
                int scan_response_num = 0;
                PrintfResp("\r\nPlease input scan_duration, (1-48)\r\n");
                scan_duration = UartReadValue();
                PrintfResp("\r\nPlease input scan_response_num (scan_response_num > 0)\r\n");
                scan_response_num = UartReadValue();
                result = sAPI_BTScanStart(scan_duration,scan_response_num);
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
                break;
            }
            case SC_BT_DEMO_SCAN_STOP:
                result = sAPI_BTScanStop();
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
                break;
            case SC_BT_DEMO_PAIR_DEVICE:
            {
                char addrStr[13] = {0};
                struct sc_bt_addr addr = {0};
                int j = 6;
                PrintfResp("\r\nPlease input address to pair, like XXXXXXXXXXXX\r\n");
                UartReadLine(addrStr, sizeof(addrStr));
                for(int i = 0;i < 12;i += 2)
                {
                    char tmp[4] = {0};
                    memcpy(tmp,&addrStr[i],2);
                    addr.bytes[--j] = strtoul(tmp,NULL,16);
                }
                sAPI_BTPairStart(addr);
                break;
            }
            case SC_BT_DEMO_UNPAIR_DEVICE:
            {
                 char addrStr[13] = {0};
                 struct sc_bt_addr addr = {0};
                 int j = 6;
                 PrintfResp("\r\nPlease input address to unpair, like XXXXXXXXXXXX\r\n");
                 UartReadLine(addrStr, sizeof(addrStr));
                 for(int i = 0;i < 12;i += 2)
                 {
                     char tmp[4] = {0};
                     memcpy(tmp,&addrStr[i],2);
                     addr.bytes[--j] = strtoul(tmp,NULL,16);
                 }
                 result = sAPI_BTUnpair(addr);
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
                 break;
             }
            case SC_BT_DEMO_GET_PARIED_DEVICE:
            {
                unsigned char bt_device_record_num = 0;
                struct sc_bt_device_record bt_device_record_array[10] = {0};
                memset(bt_device_record_array, 0, sizeof(struct sc_bt_device_record) * 10);
                sAPI_BTPaired(bt_device_record_array, &bt_device_record_num);
                qDebug("pair success");
                qDebug("bt device record num(%d)", bt_device_record_num);
                for(int i = 0; i < bt_device_record_num; i++)
                {
                    qDebug("bt device record index(%d) peer bt address(%02X:%02X:%02X:%02X:%02X:%02X)", i,
                            bt_device_record_array[i].addr[5],bt_device_record_array[i].addr[4],
                            bt_device_record_array[i].addr[3],bt_device_record_array[i].addr[2],
                            bt_device_record_array[i].addr[1],bt_device_record_array[i].addr[0]);
                    sprintf(urc, "\r\nbt device record index(%d) peer bt address(%02X:%02X:%02X:%02X:%02X:%02X)\r\n", i,
                            bt_device_record_array[i].addr[5],bt_device_record_array[i].addr[4],
                            bt_device_record_array[i].addr[3],bt_device_record_array[i].addr[2],
                            bt_device_record_array[i].addr[1],bt_device_record_array[i].addr[0]);
                    PrintfResp(urc);
                }
                break;
            }
            case SC_BT_DEMO_SPP_CONNECT:
            {
                 char addrStr[13] = {0};
                 struct sc_bt_addr addr = {0};
                 int j = 6;
                 PrintfResp("\r\nPlease input address to connect spp, like XXXXXXXXXXXX\r\n");
                 UartReadLine(addrStr, sizeof(addrStr));
                 for(int i = 0;i < 12;i += 2)
                 {
                     char tmp[4] = {0};
                     memcpy(tmp,&addrStr[i],2);
                     addr.bytes[--j] = strtoul(tmp,NULL,16);
                 }
                result = sAPI_BTSPPConnect(addr);
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
                break;
            }
            case SC_BT_DEMO_SPP_DISCONNECT:
                result = sAPI_BTSPPDisconnect(bt_spp_port);
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
                break;
            case SC_BT_DEMO_SPP_SEND:
            {
                char buffer[500] = {0};
                PrintfResp("\r\nPlease input data to send:\r\n");
                UartReadLine(buffer, sizeof(buffer));
                sAPI_BTSPPSend((unsigned char *)buffer, 10,bt_spp_port);
                break;
            }
            case SC_BT_DEMO_RSSI_GET:
                sAPI_BTReadRssi();
                break;
            default:break;
        }
    }
}

#endif

