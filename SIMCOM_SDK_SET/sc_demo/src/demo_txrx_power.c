/** 
* @file         demo_txrx_power.c 
* @brief        demo
* @author       ZYL
* @date         2021/4/6
* @version      V1.0.0 
* @par Copyright (c):  
*               SIMCom Co.Ltd 2003-2021
* @par History: 1:Create
*
*/
#ifdef FALCON_1803_PLATFORM

#include "simcom_os.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "simcom_uart.h"
#include "uart_api.h"
#include "sapi_txrx_power.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

extern sMsgQRef simcomUI_msgq;
extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);


typedef enum{
    SC_TXRX_POWER_DEMO_GETRX          = 1,
    SC_TXRX_POWER_DEMO_GETTX          = 2,
    SC_TXRX_POWER_DEMO_MAX              = 99
}SC_TXRX_POWER_DEMO_E;

void txRxPowerDemo(void)
{
    int opt = 0;
    int resp_buff_len = 120;
    char rsp_buff[resp_buff_len];
    SC_TXRX_POWER_RETURN_ERRCODE ret =SC_TXRX_POWER_RTEURN_UNKNOW;

    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] = {
        "1. GetRxPower",
        "2. GetTxPower",
        "99. Back"
    };

    while(1)
    {
        memset(rsp_buff, 0, resp_buff_len);
        PrintfResp(note);
        PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));

        opt = UartReadValue();

        switch(opt)
        {
            case SC_TXRX_POWER_DEMO_GETRX:
            {
                sAPI_Debug("[SC_TXRX_POWER_DEMO_GETRX] Enter...");
                INT16 rxPwrVal = 0;
                ret = sAPI_getRxPower(&rxPwrVal);
                sAPI_Debug("[RXPWR] ret=%d,rxPwrVal:%d",ret,rxPwrVal);

                if(SC_TXRX_POWER_RETURN_SUCCESS == ret)
                {
                    snprintf(rsp_buff,resp_buff_len,"\r\nGet rx power:%d dbm\r\n",rxPwrVal);
                }
                else
                {
                    snprintf(rsp_buff,resp_buff_len,"\r\nGet rx power faild!!\r\n");
                }
                PrintfResp(rsp_buff);
                break;
            }
            case SC_TXRX_POWER_DEMO_GETTX:
            {
                sAPI_Debug("[SC_TXRX_POWER_DEMO_GETRX] Enter...");
                INT16 txPwrVal = 0;
                ret = sAPI_getTxPower(&txPwrVal);
                sAPI_Debug("[RXPWR] ret=%d,txPwrVal:%d",ret,txPwrVal);

                if(SC_TXRX_POWER_RETURN_SUCCESS == ret)
                {
                    snprintf(rsp_buff,resp_buff_len,"\r\nGet tx power:%d dbm\r\n",txPwrVal);
                }
                else
                {
                    snprintf(rsp_buff,resp_buff_len,"\r\nGet tx power faild!!\r\n");
                }
                PrintfResp(rsp_buff);
                break;
            }
            case SC_TXRX_POWER_DEMO_MAX:
            {
                sAPI_Debug("[SC_TXRX_POWER_DEMO_MAX] Enter...");
                PrintfResp("\r\nReturn to the previous menu!\r\n");
                return;
            }
            default:
                break;
        }
    }
}
#endif
