/**
  ******************************************************************************
  * @file    demo_simcard.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of SIM card demo operation.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include "simcom_simcard.h"
#include "simcom_os.h"
#include "simcom_debug.h"
#include "simcom_uart.h"
#include "simcom_common.h"
#include "stdio.h"
#include "uart_api.h"


sMsgQRef g_simcard_demo_msgQ;

extern void PrintfResp(char *format);
extern void PrintfOptionMenu(char *options_list[], int array_size);

#define SIMCARD_URC_RECIVE_TIME_OUT 3000

typedef enum
{
    SC_SIMCARD_DEMO_GETPIN                = 1,
    SC_SIMCARD_DEMO_HOTPLUG               = 2,
    SC_SIMCARD_DEMO_SETPIN                = 3,
    SC_SIMCARD_DEMO_GETICCID              = 4,
    SC_SIMCARD_DEMO_GETIMSI               = 5,
    SC_SIMCARD_DEMO_GETHPLMN              = 6,
    SC_SIMCARD_DEMO_SIMRESTRICTEDACCESS   = 7,
    SC_SIMCARD_DEMO_SETCLCK               = 8,
    SC_SIMCARD_DEMO_SIMGENERICACCESS      = 9,
    SC_SIMCARD_DEMO_GETNUM                = 10,
    SC_SIMCARD_DEMO_STK                   = 11,
#ifdef FEATURE_SIMCOM_DUALSIM
    SC_SIMCARD_DEMO_SWITCHCARD            = 12,
    SC_SIMCARD_DEMO_GETBINDSIM            = 13,
    SC_SIMCARD_DEMO_SETBINDSIM            = 14,
    SC_SIMCARD_DEMO_GETDUALSIMTYPE        = 15,
    SC_SIMCARD_DEMO_SETDUALSIMTYPE        = 16,
#if 0
    SC_SIMCARD_DEMO_GETDUALSIMURC         = 17,
    SC_SIMCARD_DEMO_SETDUALSIMURC         = 18,
#endif
    SC_SIMCARD_DEMO_Extend_API            = 92,
#endif
    SC_SIMCARD_DEMO_MAX                   = 99
} SC_SIMCARD_DEMO_TYPE;

#if (defined SIMCOM_A7680C_V5_01) || (defined SIMCOM_A7670C_V701) || (defined SIMCOM_A7670C_V702) || defined (SIMCOM_A7680C_V506) || defined (SIMCOM_A7680C_V603)
#define SC_UART    SC_UART4
#else
#define SC_UART    SC_UART
#endif

/**
  * @brief  SIM card hot swap
  * @param  void
  * @note   Please select 8 pin SIM card holder and connect SIM_DET to card holder related pin.
  * @retval void
  */
SC_simcard_err_e SimcardHotSwapDemo()
{
    sAPI_Debug("[HotSwapDemo] ENTER %s...\r\n", __func__);
    SC_simcard_err_e ret = SC_SIM_RETURN_FAIL;
    __attribute__((__unused__)) SC_STATUS status;
    char rsp_buff[100] = {0};
    SIM_MSG_T msg;
    UINT32 opt = 0;
    int param = 0;


    char *note = "\r\nPlease select an option to test from the items listed below:\r\n";
    char *options_list[] =
    {
        "1. Query function state",
        "2. Query detection level",
        "3. Set function switch",
        "4. Set detection level",
        "5. Query plugout func state",
        "6. Set plugout func switch",
        "99. Back"
    };

    while (1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        ret = SC_SIM_RETURN_FAIL;
        opt = UartReadValue();

        switch (opt)
        {
            case SC_HOTSWAP_QUERY_STATE:
            {
                sAPI_Debug("[HotSwapDemo] Query function state\r\n");
                ret = sAPI_SimcardHotSwapMsg((SC_HotSwapCmdType_e)opt, 0, g_simcard_demo_msgQ);
                break;
            }
            case SC_HOTSWAP_QUERY_LEVEL:
            {
                sAPI_Debug("[HotSwapDemo] Query detection level");
                ret = sAPI_SimcardHotSwapMsg((SC_HotSwapCmdType_e)opt, 0, g_simcard_demo_msgQ);
                break;
            }
            case SC_HOTSWAP_SET_SWITCH:
            {
                sAPI_Debug("[HotSwapDemo] Set function switch");
                char *func_list[] =
                {
                    "0. Off",
                    "1. On",
                };
                PrintfResp(note);
                PrintfOptionMenu(func_list, sizeof(func_list) / sizeof(func_list[0]));
                param = UartReadValue();
                if ((param == 0) || (param == 1))
                {
                    ret = sAPI_SimcardHotSwapMsg((SC_HotSwapCmdType_e)opt, param, g_simcard_demo_msgQ);
                }
                else
                {
                    sAPI_UartWriteString(SC_UART, (UINT8 *)"Invalid Parameter");
                }
                break;
            }
            case SC_HOTSWAP_SET_LEVEL:
            {
                sAPI_Debug("[HotSwapDemo] Set detection level");
                char *level_list[] =
                {
                    "0. Low",
                    "1. High",
                };
                PrintfResp(note);
                PrintfOptionMenu(level_list, sizeof(level_list) / sizeof(level_list[0]));
                param = UartReadValue();
                if ((param == 0) || (param == 1))
                {
                    ret = sAPI_SimcardHotSwapMsg((SC_HotSwapCmdType_e)opt, param, g_simcard_demo_msgQ);
                }
                else
                {
                    sAPI_UartWriteString(SC_UART, (UINT8 *)"Invalid Parameter");
                }
                break;
            }
            case SC_HOTSWAP_QUERY_OUTSTATE:
            {
                sAPI_Debug("[HotSwapDemo] Query plugout func state\r\n");
                ret = sAPI_SimcardHotSwapMsg((SC_HotSwapCmdType_e)opt, 0, g_simcard_demo_msgQ);
                break;
            }
            case SC_HOTSWAP_SET_OUTSWITCH:
            {
                sAPI_Debug("[HotSwapDemo] Set plugout function switch");
                char *func_list[] =
                {
                    "0. Off",
                    "1. On",
                };
                PrintfResp(note);
                PrintfOptionMenu(func_list, sizeof(func_list) / sizeof(func_list[0]));
                param = UartReadValue();
                if ((param == 0) || (param == 1))
                {
                    ret = sAPI_SimcardHotSwapMsg((SC_HotSwapCmdType_e)opt, param, g_simcard_demo_msgQ);
                }
                else
                {
                    sAPI_UartWriteString(SC_UART, (UINT8 *)"Invalid Parameter");
                }
                break;
            }
            case 99:
            {
                sAPI_Debug("[HotSwapDemo] back");
                return SC_SIM_RTEURN_UNKNOW;
            }
            default:
            {
                break;
            }
        }

        if (SC_SIM_RETURN_SUCCESS == ret)
        {
            sAPI_Debug("[HotSwapDemo] ret :SC_SIM_RETURN_SUCCESS\r\n");
            memset(&msg, 0, sizeof(msg));
            status = sAPI_MsgQRecv(g_simcard_demo_msgQ, &msg, SIMCARD_URC_RECIVE_TIME_OUT);
            sAPI_Debug("[HotSwapDemo] msg.arg2:%d\r\n", msg.arg2);
            switch (opt)
            {
                case SC_HOTSWAP_QUERY_STATE:
                {
                    sprintf(rsp_buff, "\r\tHot Swap State:%d\t(0:off  1:on)\r\n", msg.arg2);
                    break;
                }
                case SC_HOTSWAP_QUERY_LEVEL:
                {
                    sprintf(rsp_buff, "\r\tHot Swap Detection Level:%d\t(0:low  1:high)\r\n", msg.arg2);
                    break;
                }
                case SC_HOTSWAP_SET_SWITCH:
                {
                    sprintf(rsp_buff, "\r\tHot Swap Set Function Switch Succeed\r\n");
                    break;
                }
                case SC_HOTSWAP_SET_LEVEL:
                {
                    sprintf(rsp_buff, "\r\tHot Swap Set Detection Level Succeed\r\n");
                    break;
                }
                case SC_HOTSWAP_QUERY_OUTSTATE:
                {
                    sprintf(rsp_buff, "\r\tHot Swap Plugout state:%d\t(0:off  1:on)\r\n", msg.arg2);
                    break;
                }
                case SC_HOTSWAP_SET_OUTSWITCH:
                {
                    sprintf(rsp_buff, "\r\tHot Swap Set Plugout Succeed\r\n");
                    break;
                }
                default:
                {
                    break;
                }
            }
            sAPI_UartWriteString(SC_UART, (UINT8 *)rsp_buff);
        }
    }

    return ret;
}

/**
  * @brief  Set CPIN
  * @param  void
  * @note
  * @retval void
  */
SC_simcard_err_e SimcardSetPinDemo()
{
    SC_simcard_err_e ret = SC_SIM_RETURN_SUCCESS;
    __attribute__((__unused__)) SC_STATUS status;
    UINT8 opt;
    char oldPasswd[16] = {0};
    char newPasswd[16] = {0};
    char oldPasswdTemp[16] = {0};

    char *note = "\r\nPlease select:\r\n 0:No Need New PIN\r\n 1:Need New PIN\r\n";
    PrintfResp(note);

    opt = UartReadValue();

    switch (opt)
    {
        case 0:
        {
            char pinbuf[50] = {0};
            int pinlen = 0;
            sAPI_Debug("[PinsetDemo] No Need New PIN");
            char *scan_pin = "\r\nPlease enter the PIN:\r\n";
            PrintfResp(scan_pin);
            pinlen = UartReadLine(pinbuf, 50);
            sAPI_Debug("[PinsetDemo] pin buf = [%d, %s]", pinlen, pinbuf);
            if (pinlen >= 4)
            {
                memcpy(oldPasswdTemp, pinbuf, pinlen);
            }
            else
            {
                sAPI_Debug("[PinsetDemo] old pin length too short!!\r\n");
                sAPI_UartWriteString(SC_UART, (UINT8 *)"\r\n PIN too short\r\n");
                ret = SC_SIM_RETURN_FAIL;
                break;
            }
            if ((oldPasswdTemp[pinlen - 1] == '\n') && (oldPasswdTemp[pinlen - 2] == '\r')) /*delet CR LF*/
            {
                sAPI_Debug("[PinsetDemo] delet CR LF\r\n");
                memcpy(oldPasswd, oldPasswdTemp, pinlen - 2);
            }
            else
                memcpy(oldPasswd, oldPasswdTemp, pinlen);

            sAPI_Debug("[PinsetDemo] oldPasswd:%s,len:%d\r\n", oldPasswd, strlen(oldPasswd));

            sAPI_UartWriteString(SC_UART, (UINT8 *)oldPasswd);
            ret = sAPI_SimcardPinSet(oldPasswd, newPasswd, opt);
            if (ret != SC_SIM_RETURN_SUCCESS)
            {
                sAPI_UartWriteString(SC_UART, (UINT8 *)"\r\n PIN Set Faild!\r\n");
            }
            else
            {
                sAPI_UartWriteString(SC_UART, (UINT8 *)"\r\n PIN Set Succeed!\r\n");
            }
            break;
        }
        case 1:
        {
            sAPI_Debug("[PinsetDemo] Need New PIN");
            char *scan_oldpin = "\r\nPlease enter the old PIN:\r\n";
            PrintfResp(scan_oldpin);
            UartReadLine(oldPasswd, sizeof(oldPasswd));
            sAPI_UartWriteString(SC_UART, (UINT8 *)oldPasswd);

            char *scan_newpin = "\r\nPlease enter the new PIN:\r\n";
            PrintfResp(scan_newpin);
            UartReadLine(newPasswd, sizeof(newPasswd));
            sAPI_UartWriteString(SC_UART, (UINT8 *)newPasswd);

            ret = sAPI_SimcardPinSet(oldPasswd, newPasswd, opt);
            if (ret != SC_SIM_RETURN_SUCCESS)
            {
                sAPI_UartWriteString(SC_UART, (UINT8 *)"\r\n PIN Set Faild!\r\n");
            }
            else
            {
                sAPI_UartWriteString(SC_UART, (UINT8 *)"\r\n PIN Set Succeed!\r\n");
            }
            break;
        }
        default:
            break;
    }
    return ret;
}

/**
  * @brief  Get PIN state
  * @param  void
  * @note
  * @retval void
  */
SC_simcard_err_e SimcardGetPinDemo()
{
    SC_simcard_err_e ret = SC_SIM_RTEURN_UNKNOW;
    char rsp_buff[100] = {0};
    int resp_buff_len = 100;
    UINT8 cpin = 0;

    ret = sAPI_SimcardPinGet(&cpin);
    if (ret == SC_SIM_RETURN_SUCCESS)
    {
        snprintf(rsp_buff, resp_buff_len,
                 "\r\nGet pin state:%d\t(0:READY 1:PIN 2:PUK 3:BLK 4:REMV 5:CRASH 6:NOINSRT 7:UNKN)\r\n", cpin);
    }
    else
    {
        snprintf(rsp_buff, resp_buff_len, "\r\nGet Pin State Falied!\r\n");
    }
    PrintfResp(rsp_buff);
    return ret;
}

/**
  * @brief  Get IMSI
  * @param  void
  * @note
  * @retval void
  */
SC_simcard_err_e SimcardGetImsiDemo()
{
    SC_simcard_err_e ret = SC_SIM_RTEURN_UNKNOW;
    char rsp_buff[100] = {0};
    int resp_buff_len = 100;
    char imsi[32] = {0};

    ret = sAPI_SysGetImsi(&imsi[0]);
    if (ret == SC_SIM_RETURN_SUCCESS)
    {
        snprintf(rsp_buff, resp_buff_len, "\r\nGet IMSI:%s\r\n", &imsi[0]);
    }
    else
    {
        snprintf(rsp_buff, resp_buff_len, "\r\nGet IMSI Faild!!\r\n");
    }
    PrintfResp(rsp_buff);
    return ret;
}

/**
  * @brief  Get ICCID
  * @param  void
  * @note
  * @retval void
  */
SC_simcard_err_e SimcardGetIccidDemo()
{
    SC_simcard_err_e ret = SC_SIM_RTEURN_UNKNOW;
    char rsp_buff[100] = {0};
    int resp_buff_len = 100;
    char iccid[32] = {0};

    ret = sAPI_SysGetIccid(&iccid[0]);
    if (ret == SC_SIM_RETURN_SUCCESS)
    {
        snprintf(rsp_buff, resp_buff_len, "\r\nGet ICCID:%s\r\n", &iccid[0]);
    }
    else
    {
        snprintf(rsp_buff, resp_buff_len, "\r\nGet ICCID Faild!!\r\n");
    }
    PrintfResp(rsp_buff);
    return ret;
}

/**
  * @brief  Get HPLMN
  * @param  void
  * @note
  * @retval void
  */
SC_simcard_err_e SimcardGetHplmnDemo()
{
    SC_simcard_err_e ret = SC_SIM_RTEURN_UNKNOW;
    char rsp_buff[100] = {0};
    int resp_buff_len = 100;
    Hplmn_st hplmn = {0};

    ret = sAPI_SysGetHplmn(&hplmn);
    if (ret == SC_SIM_RETURN_SUCCESS)
    {
        snprintf(rsp_buff, resp_buff_len, "\r\nSPN:%s\r\nHPLMN:%s-%s", hplmn.spn, hplmn.mcc, hplmn.mnc);
    }
    else
    {
        snprintf(rsp_buff, resp_buff_len, "\r\nGet HPLMN Faild!!\r\n");
    }
    PrintfResp(rsp_buff);
    return ret;
}

/**
  * @brief  Sim Restricted Access
  * @param  void
  * @note   
  * @retval void
  */
SC_simcard_err_e SimcardSimRestrictedAccessDemo(void)
{
    SC_simcard_err_e ret = SC_SIM_RETURN_SUCCESS;
    __attribute__((__unused__)) SC_STATUS status;
    char rsp_buff[100] = {0};
    int resp_buff_len = 100;
    int cmd=0, fileId=0, p1=0, p2=0, p3=0;
    char data[255 * 2 + 1], pathid[100 * 2 + 1];
    CrsmResponse_st response;

    char *note = "\r\nPlease select an cmd from the items listed below.\r\n";
    char *cmd_list[] = {
        "176  READ BINARY",
        "178  READ RECORD",
        "192  GET RESPONSE",
        "214  UPDATE BINARY",
        "220  UPDATE RECORD",
        /*
        "242  STATUS",
        "203  RETRIEVE DATA",
        "219  SET DATA"
        */
    };
    PrintfResp(note);
    PrintfOptionMenu(cmd_list,sizeof(cmd_list)/sizeof(cmd_list[0]));

    cmd = UartReadValue();
    switch(cmd)
    {
        case 176:
        case 178:
        case 192:
            PrintfResp("\r\nPlease enter the file ID(dec):\r\n");
            fileId = UartReadValue();
            sAPI_Debug("[SimcardSimRestrictedAccessDemo] fileId = %d",fileId);

            PrintfResp("\r\nPlease enter the p1(0-255):\r\n");
            p1 = UartReadValue();
            sAPI_Debug("[SimcardSimRestrictedAccessDemo] p1 = %d",p1);

            PrintfResp("\r\nPlease enter the p2(0-255):\r\n");
            p2 = UartReadValue();
            sAPI_Debug("[SimcardSimRestrictedAccessDemo] p2 = %d",p2);

            PrintfResp("\r\nPlease enter the p3(Length 0-255):\r\n");
            p3 = UartReadValue();
            sAPI_Debug("[SimcardSimRestrictedAccessDemo] p3 = %d",p3);

            memset(data,0,sizeof(data));
            memset(pathid,0,sizeof(pathid));

            ret = sAPI_SimRestrictedAccess(cmd,fileId,p1,p2,p3,data,pathid,&response);
            if(ret != SC_SIM_RETURN_SUCCESS)
            {
                sAPI_Debug("[SimcardSimRestrictedAccessDemo] sAPI_SimRestrictedAccess is error!!\r\n");
                snprintf(rsp_buff,resp_buff_len,"\r\nOperate %d error!!\r\n", fileId);
                PrintfResp(rsp_buff);
            }
            else
            {
                snprintf(rsp_buff,resp_buff_len,"\r\nfileId=%d, sw1=%d,sw2=%d\r\n", fileId, response.sw1, response.sw2);
                PrintfResp(rsp_buff);
                PrintfResp(response.data);
                snprintf(rsp_buff,resp_buff_len,"\r\nsuccess!!\r\n");
                PrintfResp(rsp_buff);
            }
            break;
        case 214:
        case 220:
            PrintfResp("\r\nPlease enter the file ID(dec):\r\n");
            fileId = UartReadValue();
            sAPI_Debug("[SimcardSimRestrictedAccessDemo] fileId = %d",fileId);

            PrintfResp("\r\nPlease enter the p1(0-255):\r\n");
            p1 = UartReadValue();
            sAPI_Debug("[SimcardSimRestrictedAccessDemo] p1 = %d",p1);

            PrintfResp("\r\nPlease enter the p2(0-255):\r\n");
            p2 = UartReadValue();
            sAPI_Debug("[SimcardSimRestrictedAccessDemo] p2 = %d",p2);

            PrintfResp("\r\nPlease enter the p3(Length 0-255):\r\n");
            p3 = UartReadValue();
            sAPI_Debug("[SimcardSimRestrictedAccessDemo] p3 = %d",p3);

            PrintfResp("\r\nPlease enter the data(hex):\r\n");
            UartReadLine(data, sizeof(data));
#if 0
            if (strstr(data,"\r\n"))
            {
                memcpy(data,data,strlen(data)-2);/*cut out CR LF*/
            }
            else
            {
                memcpy(data,data,strlen(data));
            }
#endif
            sAPI_UartWriteString(SC_UART, (UINT8 *)data);
            sAPI_Debug("[SimcardSimRestrictedAccessDemo] len data = [%d, %s]", strlen(data), data);
#if 0
            if(strlen(data) != p3)//if the legth of data is error, Data should be spliced
            {
                sAPI_MsgQRecv(simcomUI_msgq,&optionMsg,SC_SUSPEND);
                if(SRV_UART != optionMsg.msg_id)
                {
                    sAPI_Debug("[SimcardSimRestrictedAccessDemo] msg_id is error!!\r\n");
                }
                sAPI_Debug("[SimcardSimRestrictedAccessDemo] data arg3 = [%d, %s]",optionMsg.arg2, optionMsg.arg3);
                if (strstr(optionMsg.arg3,"\r\n"))
                {
                    strncat(data,optionMsg.arg3,strlen(optionMsg.arg3)-2);/*cut out CR LF*/
                }
                else
                {
                    strcat(data,optionMsg.arg3);
                }
            }
#endif
            PrintfResp("\r\nPlease enter the pathid(hex):\r\n");
            UartReadLine(pathid, sizeof(pathid));
            sAPI_Debug("[SimcardSimRestrictedAccessDemo] pathid = %s",pathid);
            sAPI_UartWriteString(SC_UART, (UINT8 *)pathid);

            ret = sAPI_SimRestrictedAccess(cmd,fileId,p1,p2,p3,data,pathid,&response);
            if(ret != SC_SIM_RETURN_SUCCESS)
            {
                sAPI_Debug("[SimcardSimRestrictedAccessDemo] sAPI_SimRestrictedAccess is error!!\r\n");
                snprintf(rsp_buff,resp_buff_len,"\r\nOperate %d error!!\r\n", fileId);
                PrintfResp(rsp_buff);
            }
            else
            {
                snprintf(rsp_buff,resp_buff_len,"\r\nfileId=%d, sw1=%d,sw2=%d\r\n", fileId, response.sw1, response.sw2);
                PrintfResp(rsp_buff);
                PrintfResp(response.data);
                snprintf(rsp_buff,resp_buff_len,"\r\nsuccess!!\r\n");
                PrintfResp(rsp_buff);
            }
            break;
        /*
        case 242:
            break;
        case 203:
            break;
        case 219:
            break;
        */
        default:
            break;
    }
    return ret;

}

/**
  * @brief  Sim Generic Access
  * @param  void
  * @note   
  * @retval void
  */
SC_simcard_err_e SimcardGenericAccessDemo(void)
{
    SC_simcard_err_e ret = SC_SIM_RETURN_SUCCESS;
    __attribute__((__unused__)) SC_STATUS status;
    char rsp_buff[600] = {0};
    int resp_buff_len = 600;
    int cmdLen=0;
    char cmd[255 * 2 + 1];
    CsimResponse_st response;

    PrintfResp("\r\nPlease enter the cmdLen(0-255):\r\n");
    cmdLen = UartReadValue();
    sAPI_Debug("[SimcardGenericAccessDemo] cmdLen = %d", cmdLen);

    PrintfResp("\r\nPlease enter the cmd:\r\n");
    UartReadLine(cmd, sizeof(cmd));
    sAPI_UartWriteString(SC_UART, (UINT8 *)cmd);
    sAPI_Debug("[SimcardGenericAccessDemo] strlen(cmd) cmd = [%d, \"%s\"]", strlen(cmd), cmd);

    ret = sAPI_SimGenericAccess(cmdLen, cmd, &response);
    if(ret != SC_SIM_RETURN_SUCCESS)
    {
        sAPI_Debug("[SimcardGenericAccessDemo] sAPI_SimRestrictedAccess is error!!\r\n");
        snprintf(rsp_buff,resp_buff_len,"\r\nsAPI_SimGenericAccess error!!\r\n");
        PrintfResp(rsp_buff);
    }
    else
    {
        snprintf(rsp_buff,resp_buff_len,"\r\nlength=%d, data=\"%s\" \r\n", response.length, response.data);
        PrintfResp(rsp_buff);
        snprintf(rsp_buff,resp_buff_len,"\r\nsuccess!!\r\n");
        PrintfResp(rsp_buff);
    }

    return ret;

}

#ifdef FEATURE_SIMCOM_DUALSIM
/**
  * @brief  SIM card switch card
  * @param  void
  * @note
  * @retval void
  */
SC_simcard_err_e simcardSwitchdemo()
{
    sAPI_Debug("[SIMSwitchDemo] ENTER %s...\r\n", __func__);
    SC_simcard_err_e ret = SC_SIM_RETURN_SUCCESS;
    char rsp_buff[100] = {0};
    char buf[10] = {0};
    UINT8 simcardSwitch;

    char *note = "\r\nPlease select:\r\n 0:SIM1\r\n 1:SIM2\r\n";
    PrintfResp(note);
    UartReadLine(buf, 10);
    sAPI_Debug("[SIMSwitchDemo] arg3 = [%s]", buf);
    sAPI_UartWriteString(SC_UART, (UINT8 *)buf);
    simcardSwitch = atoi(buf);
    ret = sAPI_SimcardSwitchMsg(simcardSwitch, NULL);
    sprintf(rsp_buff, "\rsAPI_SimcardSwitchMsg:\r\n\tresultCode:%d     (0:success  1:fail)\r\n", ret);
    sAPI_UartWriteString(SC_UART, (UINT8 *)rsp_buff);

    return ret;
}

/**
  * @brief  Get BINDSIM
  * @param  void
  * @note
  * @retval void
  */
SC_simcard_err_e SimcardGetBindSimDemo()
{
    SC_simcard_err_e ret = SC_SIM_RTEURN_UNKNOW;
    char rsp_buff[100] = {0};
    int resp_buff_len = 100;
    UINT8 bindsim = {0};

    ret = sAPI_SysGetBindSim(&bindsim);
    if (ret == SC_SIM_RETURN_SUCCESS)
    {
        snprintf(rsp_buff, resp_buff_len, "\r\nGet BINDSIM:%d\t(0:SIM1 1:SIM2)\r\n", bindsim);
    }
    else
    {
        snprintf(rsp_buff, resp_buff_len, "\r\nGet BINDSIM Faild!!\r\n");
    }

    PrintfResp(rsp_buff);
    return ret;
}

/**
  * @brief  Set BINDSIM
  * @param  void
  * @note
  * @retval void
  */
SC_simcard_err_e SimcardSetBindSimDemo()
{
    SC_simcard_err_e ret = SC_SIM_RETURN_SUCCESS;
    __attribute__((__unused__)) SC_STATUS status;
    UINT8 opt;
    char rsp_buff[100] = {0};
    int resp_buff_len = 120;

    char *note = "\r\nPlease select:\r\n 0:SIM1\r\n 1:SIM2\r\n";
    PrintfResp(note);
    opt = UartReadValue();

    ret = sAPI_SysSetBindSim(opt);
    if (ret == SC_SIM_RETURN_SUCCESS)
    {
        snprintf(rsp_buff, resp_buff_len, "\r\nSet BINDSIM:%d\r\n", opt);
    }
    else
    {
        snprintf(rsp_buff, resp_buff_len, "\r\nSet BINDSIM Faild!!\r\n");
    }
    PrintfResp(rsp_buff);
    return ret;
}

/**
  * @brief  Get DUALSIM Type
  * @param  void
  * @note   
  * @retval void
  */
SC_simcard_err_e SimcardGetDualSimTypeDemo()
{
  SC_simcard_err_e ret = SC_SIM_RTEURN_UNKNOW;
  char rsp_buff[121] = {0};
  int resp_buff_len = 120;
  UINT8 type = {0};

  ret = sAPI_SysGetDualSimType(&type);
  if(ret == SC_SIM_RETURN_SUCCESS)
  {
    snprintf(rsp_buff,resp_buff_len,"\r\nGet DualSim Type: %d\t(0: DUAL SIM DUAL STANDBY, 1: DUAL SIM SINGLE STANDBY, 3: DUAL SIM DUAL STANDBY FOR FP)\r\n",type);
  }
  else
  {
    snprintf(rsp_buff,resp_buff_len,"\r\nGet DualSim Type Faild!!\r\n");
  }
  PrintfResp(rsp_buff);
  return ret;
}

/**
  * @brief  Set DUALSIM Type
  * @param  void
  * @note   
  * @retval void
  */
SC_simcard_err_e SimcardSetDualSimTypeDemo()
{
  SC_simcard_err_e ret = SC_SIM_RETURN_SUCCESS;
  __attribute__((__unused__)) SC_STATUS status;
  UINT8 opt;
  char rsp_buff[100] = {0};
  int resp_buff_len = 100;

  char *note = "\r\nPlease select:\r\n 0: DUAL SIM DUAL STANDBY \
                                  \r\n 1: DUAL SIM SINGLE STANDBY \
                                  \r\n 3: DUAL SIM DUAL STANDBY FOR FP\r\n";
  PrintfResp(note);
  opt = UartReadValue();

  ret = sAPI_SysSetDualSimType(opt);
  if(ret == SC_SIM_RETURN_SUCCESS)
  {
    snprintf(rsp_buff,resp_buff_len,"\r\nSet DUALSIM Type: %d\r\n",opt);
  }
  else
  {
    snprintf(rsp_buff,resp_buff_len,"\r\nSet DUALSIM Type Faild!!\r\n");
  }
  PrintfResp(rsp_buff);
  return ret;

}

#if 0
SC_simcard_err_e SimcardGetDualSimURCDemo()
{
    SC_simcard_err_e ret = SC_SIM_RTEURN_UNKNOW;
    char rsp_buff[121] = {0};
    int resp_buff_len = 120;
    UINT8 type = {0};
    
    ret = sAPI_SysGetDualSimURC(&type);
    if(ret == SC_SIM_RETURN_SUCCESS)
    {
      snprintf(rsp_buff,resp_buff_len,"\r\nGet DualSimURC: %d\t(0: ONLY SIM1 report, 1: ONLY SIM2 report, 3: SIM1&SIM2 report)\r\n",type);
    }
    else
    {
      snprintf(rsp_buff,resp_buff_len,"\r\nGet DualSimURC Faild!!\r\n");
    }
    PrintfResp(rsp_buff);
    return ret;
}

SC_simcard_err_e SimcardSetDualSimURCDemo()
{
    SC_simcard_err_e ret = SC_SIM_RETURN_SUCCESS;
    __attribute__((__unused__)) SC_STATUS status;
    UINT8 opt;
    char rsp_buff[100] = {0};
    int resp_buff_len = 100;
    
    char *note = "\r\nPlease select:\r\n 0: ONLY SIM1 report \
                                    \r\n 1: ONLY SIM2 report \
                                    \r\n 2: SIM1&SIM2 report\r\n";
    PrintfResp(note);
    opt = UartReadValue();
    
    ret = sAPI_SysSetDualSimURC(opt);
    if(ret == SC_SIM_RETURN_SUCCESS)
    {
      snprintf(rsp_buff,resp_buff_len,"\r\nSet DUALSIMURC: %d\r\n",opt);
    }
    else
    {
      snprintf(rsp_buff,resp_buff_len,"\r\nSet DUALSIMURC Faild!!\r\n");
    }
    PrintfResp(rsp_buff);
    return ret;
}
#endif

SC_simcard_err_e ex_SimcardHotSwapDemo(BOOL simId)
{
    sAPI_Debug("[HotSwapDemo] ENTER %s...\r\n",__func__);
    SC_simcard_err_e ret = SC_SIM_RETURN_FAIL;
    char rsp_buff[100] = {0};
    __attribute__((__unused__)) SC_STATUS status;
    UINT32 opt = 0;
    UINT8 param = 0;


    char *note = "\r\nPlease select an option to test from the items listed below:\r\n";
    char *options_list[] = {
        "1. Query function state",
        "2. Query detection level",
        "3. Set function switch",
        "4. Set detection level",
        "5. Query plugout func state",
        "6. Set plugout func switch",
        "99. Back"
    };

    while(1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
        opt = UartReadValue();
        ret = SC_SIM_RETURN_FAIL;

        switch(opt)
        {
            case SC_HOTSWAP_QUERY_STATE:
            {
                sAPI_Debug("[ex_SimcardHotSwapDemo] Query function state\r\n");
                ret = sAPI_SimcardHotSwap_Ex(simId,(SC_HotSwapCmdType_e)opt,&param);
                break;
            }
            case SC_HOTSWAP_QUERY_LEVEL:
            {
                sAPI_Debug("[ex_SimcardHotSwapDemo] Query detection level");
                ret = sAPI_SimcardHotSwap_Ex(simId,(SC_HotSwapCmdType_e)opt,&param);
                break;
            }
            case SC_HOTSWAP_SET_SWITCH:
            {
                sAPI_Debug("[ex_SimcardHotSwapDemo] Set function switch");
                char *func_list[] = {
                    "0. Off",
                    "1. On",
                };
                PrintfResp(note);
                PrintfOptionMenu(func_list,sizeof(func_list)/sizeof(func_list[0]));
                param = UartReadValue();
                if((param == 0) ||(param ==1))
                {
                    ret = sAPI_SimcardHotSwap_Ex(simId,(SC_HotSwapCmdType_e)opt,&param);
                }
                else
                {
                    sAPI_UartWriteString(SC_UART, (UINT8 *)"Invalid Parameter");
                }
                break;
            }
            case SC_HOTSWAP_SET_LEVEL:
            {
                sAPI_Debug("[ex_SimcardHotSwapDemo] Set detection level");
                char *level_list[] = {
                    "0. Low",
                    "1. High",
                };
                PrintfResp(note);
                PrintfOptionMenu(level_list,sizeof(level_list)/sizeof(level_list[0]));
                param = UartReadValue();
                if((param == 0) ||(param == 1))
                {
                    ret = sAPI_SimcardHotSwap_Ex(simId,(SC_HotSwapCmdType_e)opt,&param);
                }
                else
                {
                    sAPI_UartWriteString(SC_UART, (UINT8 *)"Invalid Parameter");
                }
                break;
            }
            case SC_HOTSWAP_QUERY_OUTSTATE:
            {
                sAPI_Debug("[ex_SimcardHotSwapDemo] Query plugout func state\r\n");
                ret = sAPI_SimcardHotSwap_Ex(simId,(SC_HotSwapCmdType_e)opt,&param);
                break;
            }
            case SC_HOTSWAP_SET_OUTSWITCH:
            {
                sAPI_Debug("[ex_SimcardHotSwapDemo] Set plugout function switch");
                char *func_list[] = {
                    "0. Off",
                    "1. On",
                };
                PrintfResp(note);
                PrintfOptionMenu(func_list,sizeof(func_list)/sizeof(func_list[0]));
                param = UartReadValue();
                if((param == 0) || (param ==1))
                {
                    ret = sAPI_SimcardHotSwap_Ex(simId,(SC_HotSwapCmdType_e)opt,&param);
                }
                else
                {
                    sAPI_UartWriteString(SC_UART, (UINT8 *)"Invalid Parameter");
                }
                break;
            }
            case 99:
            {
                sAPI_Debug("[ex_SimcardHotSwapDemo] back");
                return SC_SIM_RETURN_FAIL;
            }
            default:
            {
                break;
            }
        }

        if(SC_SIM_RETURN_SUCCESS == ret)
        {
            sAPI_Debug("[HotSwapDemo] ret :SC_SIM_RETURN_SUCCESS\r\n");
            switch(opt)
            {
                case SC_HOTSWAP_QUERY_STATE:
                {
                    sprintf(rsp_buff,"\r\tHot Swap State:%d\t(0:off  1:on)\r\n",param);
                    break;
                }
                case SC_HOTSWAP_QUERY_LEVEL:
                {
                    sprintf(rsp_buff,"\r\tHot Swap Detection Level:%d\t(0:low  1:high)\r\n",param);
                    break;
                }
                case SC_HOTSWAP_SET_SWITCH:
                {
                    sprintf(rsp_buff,"\r\tHot Swap Set Function Switch Succeed\r\n");
                    break;
                }
                case SC_HOTSWAP_SET_LEVEL:
                {
                    sprintf(rsp_buff,"\r\tHot Swap Set Detection Level Succeed\r\n");
                    break;
                }
                case SC_HOTSWAP_QUERY_OUTSTATE:
                {
                    sprintf(rsp_buff,"\r\tHot Swap Plugout state:%d\t(0:off  1:on)\r\n",param);
                    break;
                }
                case SC_HOTSWAP_SET_OUTSWITCH:
                {
                    sprintf(rsp_buff,"\r\tHot Swap Set Plugout Succeed\r\n");
                    break;
                }
                default:
                {
                    break;
                }
            }
            PrintfResp(rsp_buff);
        }

    }

    return ret;
}


SC_simcard_err_e Extend_API_Demo()
{
    sAPI_Debug("[Extend_API_Demo] ENTER %s...\r\n",__func__);
    SC_simcard_err_e ret = SC_SIM_RETURN_FAIL;
    char rsp_buff[100] = {0};
    int resp_buff_len = 100;
    __attribute__((__unused__)) SC_STATUS status;
    UINT32 opt = 0;
    BOOL simId=0;
    UINT8 simStatus=0;

    char *note = "\r\nPlease select an option to test from the items listed below:\r\n";
    char *options_list[] = {
    "1.  Ex_PinGet",
    "2.  Ex_HotPlug",
    "99. back"
    };
    char *note1 = "\r\nwhich sim you wanna query (0:SIM1 1:SIM2)\r\n";
    char *note2 = "\r\nwhich sim you wanna operate (0:SIM1 1:SIM2)\r\n";

    while(1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));
        opt = UartReadValue();
        ret = SC_SIM_RETURN_FAIL;

        switch(opt)
        {
            case 1://Ex_PinGet
            {
                PrintfResp(note1);
                simId = UartReadValue();
                ret = sAPI_SimcardPinGet_Ex(simId,  &simStatus);
                if(ret == SC_SIM_RETURN_SUCCESS)
                {
                    snprintf(rsp_buff,resp_buff_len,"\r\nGet SIM%d pin state:%d\t(0:READY 1:PIN 2:PUK 3:BLK 4:REMV 5:CRASH 6:NOINSRT 7:UNKN)\r\n",simId+1,simStatus);
                }
                else
                {
                    snprintf(rsp_buff,resp_buff_len,"\r\nGet SIM%d Pin State Falied!\r\n",simId+1);
                }
                PrintfResp(rsp_buff);
                break;
            }
            case 2://Ex_HotPlug
            {
                PrintfResp(note2);
                simId = UartReadValue();

                ex_SimcardHotSwapDemo(simId);
                break;
            }
            case 99:
            {
                sAPI_Debug("[Extend_API_Demo] back");
                return SC_SIM_RETURN_FAIL;
            }
            default:
            {
                break;
            }
        }
    }

    return ret;
}
#endif

/**
  * @brief  Set DUALSIM Type
  * @param  void
  * @note   
  * @retval void
  */
SC_simcard_err_e SimcardSetClckDemo()
{
  SC_simcard_err_e ret = SC_SIM_RETURN_FAIL;
  __attribute__((__unused__)) SC_STATUS status;
  UINT8 opt;
  char Passwd[16] = {0};
  int pinlen = 0;
  char rsp_buff[100] = {0};
  int resp_buff_len = 100;

  char *note = "\r\nPlease select:\r\n 1:set simcard lock \
                                  \r\n 2:set simcard unlock\r\n";
  PrintfResp(note);
  opt = UartReadValue();

  switch(opt)
  {
    case 1:
    {
      sAPI_Debug("[SimcardSetClckDemo] need password");
      char *scan_pin = "\r\nPlease enter the password:\r\n";
      PrintfResp(scan_pin);
      pinlen = UartReadLine(Passwd, 16);
      sAPI_Debug("[SimcardSetClckDemo] Passwd = [%d, %s]", pinlen, Passwd);
      if (pinlen < 4)
      {
        sAPI_Debug("[SimcardSetClckDemo] Passwd length too short!!\r\n");
        snprintf(rsp_buff,resp_buff_len,"\r\n Passwd too short\r\n");
        break;
      }

      ret = sAPI_SimLockSet(1,Passwd);
      if(ret == SC_SIM_RETURN_SUCCESS)
      {
        snprintf(rsp_buff,resp_buff_len,"\r\nset  simcard lock sucess \r\n");
      }
      else
      {
        snprintf(rsp_buff,resp_buff_len,"\r\nSet failed\r\n");
      }
      break;
    }
    case 2:
    {
      sAPI_Debug("[SimcardSetClckDemo] need password");
      char *scan_pin = "\r\nPlease enter the password:\r\n";
      PrintfResp(scan_pin);
      pinlen = UartReadLine(Passwd, 16);
      sAPI_Debug("[SimcardSetClckDemo] Passwd = [%d, %s]", pinlen, Passwd);
      if (pinlen < 4)
      {
        sAPI_Debug("[SimcardSetClckDemo] Passwd length too short!!\r\n");
        snprintf(rsp_buff,resp_buff_len,"\r\n Passwd too short\r\n");
        break;
      }

      ret = sAPI_SimLockSet(0, Passwd);
      if(ret == SC_SIM_RETURN_SUCCESS)
      {
        snprintf(rsp_buff,resp_buff_len,"\r\nset  simcard unlock sucess \r\n");
      }
      else
      {
        snprintf(rsp_buff,resp_buff_len,"\r\nSet failed\r\n");
      }
      break;
    }
    default:
        snprintf(rsp_buff,resp_buff_len,"\r\nopt error, Set failed,\r\n");
        break;
  }
  PrintfResp(rsp_buff);
  return ret;
}


SC_simcard_err_e SimcardGetNumDemo()
{
  SC_simcard_err_e ret = SC_SIM_RTEURN_UNKNOW;
  char rsp_buff[100] = {0};
  int resp_buff_len = 100;
  char num[50] = {0};

  ret = sAPI_SysGetSubscriber(&num[0]);
  if(ret == SC_SIM_RETURN_SUCCESS)
  {
    snprintf(rsp_buff,resp_buff_len,"\r\nGet Num:%s\r\n",&num[0]);
  }
  else
  {
    snprintf(rsp_buff,resp_buff_len,"\r\nGet Num Faild!!\r\n");
  }
  PrintfResp(rsp_buff);
  return ret;
}

SC_simcard_err_e SimcardStkDemo()
{
    SC_simcard_err_e ret = SC_SIM_RTEURN_UNKNOW;
    char rsp_buff[64 + 2 * SC_SIM_MAX_CMD_DATA_SIZE + 50] = {0};
    scStkOpt opt;
    char data[SC_SIM_MAX_CMD_DATA_SIZE] = {0};
    StkResult stkRst;

    memset(data,0,sizeof(char)*SC_SIM_MAX_CMD_DATA_SIZE);
    memset(&stkRst,0,sizeof(StkResult));
    char *note = "\r\nPlease select:\r\n\
    0:enable/disable proactive command Indication\r\n\
    1:download ME capability profile\r\n\
    2:requests SIMAT notification capability info\r\n\
    3:get SIM card profile\r\n\
    4:send envelope command\r\n\
    11:respond to proactive command\r\n\
    15:enable/disable auto response proactive command Indication\r\n";  

    PrintfResp(note);

    opt = UartReadValue();
    switch(opt)
    {
        case SC_STK_CMD_ENABLE_SIMAT:
        {
            char *en_list[] = {
                         "0. disable proactive",
                         "1. enable proactive",
            };
            PrintfOptionMenu(en_list,sizeof(en_list)/sizeof(en_list[0]));
            UartReadLine(data, SC_SIM_MAX_CMD_DATA_SIZE);
            if ((strncmp(data, "1",1) != 0) && (strncmp(data, "0",1) != 0))
            {
                sAPI_Debug("%s ,Invalid Parameter",__func__);
                sAPI_UartWriteString(SC_UART, (UINT8*)"Invalid Parameter");
                goto exit;
            }
            break;
        }
        case SC_STK_CMD_DOWNLOAD_PROFILE:
        {
            char *download_list = "\r\nPlease enter ME capability profile:\r\n";
            PrintfResp(download_list);
            UartReadLine(data, SC_SIM_MAX_CMD_DATA_SIZE);
            if ((0 == strlen(data)) || (strlen(data)%2 != 0))
            {
                sAPI_Debug("%s ,Invalid Parameter,%d",__func__,strlen(data));
                sAPI_UartWriteString(SC_UART, (UINT8*)"Invalid Parameter");
                goto exit;
            }

            break;
        }
        case SC_STK_CMD_GET_CAP_INFO:
        {
            /*No param,direct call API*/
            break;
        }
        case SC_STK_CMD_GET_PROFILE:
        {
            /*No param,direct call API*/
            break;
        }
        case SC_STK_CMD_SEND_ENVELOPE:
        {
            char *envelope_list = "\r\nPlease enter the envelope command:\r\n";
            PrintfResp(envelope_list);
            UartReadLine(data, SC_SIM_MAX_CMD_DATA_SIZE);
            if ((0 == strlen(data)) || (strlen(data)%2 != 0))
            {
                sAPI_Debug("%s ,Invalid Parameter,%d",__func__,strlen(data));
                sAPI_UartWriteString(SC_UART, (UINT8*)"Invalid Parameter");
                goto exit;
            }
            break;
        }
        case SC_STK_CMD_PROACTIVE:
        {
            char *respond_list = "\r\nPlease enter the respond command:\r\n";
            PrintfResp(respond_list);
            UartReadLine(data, SC_SIM_MAX_CMD_DATA_SIZE);
            if ((0 == strlen(data)) || (strlen(data)%2 != 0))
            {
                sAPI_Debug("%s ,Invalid Parameter,%d",__func__,strlen(data));
                sAPI_UartWriteString(SC_UART, (UINT8*)"Invalid Parameter");
                goto exit;
            }
            break;
        }
        case SC_STK_CMD_ENABLE_AUTORESP_PROACTIVE:
        {
            char *en_list[] = {
                         "0. disable auto response",
                         "1. enable auto response",
            };
            PrintfOptionMenu(en_list,sizeof(en_list)/sizeof(en_list[0]));
            UartReadLine(data, SC_SIM_MAX_CMD_DATA_SIZE);
            if ((strncmp(data, "1",1) != 0) && (strncmp(data, "0",1) != 0))
            {
                sAPI_Debug("%s ,Invalid Parameter",__func__);
                sAPI_UartWriteString(SC_UART, (UINT8*)"Invalid Parameter");
                goto exit;
            }
            break;
        }
        default:
        {
            goto exit;
        }
    }

    /*Call API*/
    ret = sAPI_StkService(opt,data,&stkRst);
    sAPI_Debug("%s,ret:%d,state:%d,buf:%s\r\n",__func__,ret,stkRst.v_state,stkRst.a_buf);
    if(SC_SIM_RETURN_SUCCESS == ret)
    {
        if(0 == stkRst.v_state)
        {
            sprintf(rsp_buff,"\r\nsucceed\r\nSTATE:%d    RSP:%s\r\n",stkRst.v_state,stkRst.a_buf);
            PrintfResp(rsp_buff);
        }
        else
        {
            sprintf(rsp_buff,"\r\nfaild\r\n");
            PrintfResp(rsp_buff);
        }
    }

exit:
    return ret;

}

/**
  * @brief  SIM card operation demo
  * @param  void
  * @note
  * @retval void
  */
void SimcardDemo(void)
{
    int resp_buff_len = 120;
    __attribute__((__unused__)) SC_STATUS status;
    char rsp_buff[resp_buff_len];
    UINT32 opt = 0;

    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] =
    {
        "1. PinGet",
        "2. HotPlug",
        "3. PinSet",
        "4. GetICCID",
        "5. GetIMSI",
        "6. GetHPLMN",
        "7. Sim Restricted Access",
        "8. SimLockSet",
        "9. Sim Generic Access",
        "10. GetNum",
        "11. STK",
#ifdef FEATURE_SIMCOM_DUALSIM
        "12. SwitchCard",
        "13. GetBindSim",
        "14. SetBindSim",
        "15. GetDualSimType",
        "16. SetDualSimType",
#if 0
        "17. Get DUALSIMURC type",
        "18. Set DUALSIMURC type",
#endif
        "92. Extend_API",
#endif //FEATURE_SIMCOM_DUALSIM
        "99. Back"
    };

    status = sAPI_MsgQCreate(&g_simcard_demo_msgQ, "g_simcard_demo_msgQ", sizeof(SIM_MSG_T), 4, SC_FIFO);
    if (status != SC_SUCCESS)
    {
        sAPI_Debug("ERROR: message queue creat err!\n");
    }

    while (1)
    {
        memset(rsp_buff, 0, resp_buff_len);
        PrintfResp(note);
        PrintfOptionMenu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        opt = UartReadValue();

        switch (opt)
        {
            case SC_SIMCARD_DEMO_GETPIN:
            {
                SimcardGetPinDemo();
                break;
            }
            case SC_SIMCARD_DEMO_HOTPLUG:
            {
                SimcardHotSwapDemo();
                break;
            }
            case SC_SIMCARD_DEMO_SETPIN:
            {
                SimcardSetPinDemo();
                break;
            }
            case SC_SIMCARD_DEMO_GETICCID:
            {
                SimcardGetIccidDemo();
                break;
            }
            case SC_SIMCARD_DEMO_GETIMSI:
            {
                SimcardGetImsiDemo();
                break;
            }
            case SC_SIMCARD_DEMO_GETHPLMN:
            {
                SimcardGetHplmnDemo();
                break;
            }
            case SC_SIMCARD_DEMO_SIMRESTRICTEDACCESS:
            {
                SimcardSimRestrictedAccessDemo();
                break;
            }
            case SC_SIMCARD_DEMO_SETCLCK:
            {
                SimcardSetClckDemo();
                break;
            }
            case SC_SIMCARD_DEMO_SIMGENERICACCESS:
            {
                SimcardGenericAccessDemo();
            }
            case SC_SIMCARD_DEMO_GETNUM:
            {
                SimcardGetNumDemo();
                break;
            }
            case SC_SIMCARD_DEMO_STK:
            {
                SimcardStkDemo();
                break;
            }
#ifdef FEATURE_SIMCOM_DUALSIM
            case SC_SIMCARD_DEMO_SWITCHCARD:
            {
                simcardSwitchdemo();
                break;
            }
            case SC_SIMCARD_DEMO_GETBINDSIM:
            {
                SimcardGetBindSimDemo();
                break;
            }
            case SC_SIMCARD_DEMO_SETBINDSIM:
            {
                SimcardSetBindSimDemo();
                break;
            }
            case SC_SIMCARD_DEMO_GETDUALSIMTYPE:
            {
                SimcardGetDualSimTypeDemo();
                break;
            }
            case SC_SIMCARD_DEMO_SETDUALSIMTYPE:
            {
                SimcardSetDualSimTypeDemo();
                break;
            }
#if 0
            case SC_SIMCARD_DEMO_GETDUALSIMURC:
            {
                SimcardGetDualSimURCDemo();
                break;
            }
            case SC_SIMCARD_DEMO_SETDUALSIMURC:
            {
                SimcardSetDualSimURCDemo();
                break;
            }
#endif
            case SC_SIMCARD_DEMO_Extend_API:
            {
                Extend_API_Demo();
                break;
            }
#endif //FEATURE_SIMCOM_DUALSIM
            case SC_SIMCARD_DEMO_MAX:
            {
                sAPI_Debug("[SIMCARD] Return to the previous menu!");
                PrintfResp("\r\nReturn to the previous menu!\r\n");
                sAPI_MsgQDelete(g_simcard_demo_msgQ);
                return;
            }

            default :
                break;
        }
    }
}
