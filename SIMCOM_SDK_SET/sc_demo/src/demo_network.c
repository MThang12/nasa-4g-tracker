/**
  ******************************************************************************
  * @file    demo_network.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of network management.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "simcom_network.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "simcom_os.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "simcom_uart.h"
#include "simcom_simcard.h"
#include "simcom_tcpip.h"
#include "uart_api.h"


sTimerRef networkTimerRef;
extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);

typedef struct
{
    int cid;
    char pdptype[20];
    char apn[20];
} SCcgdcontParm;
typedef struct
{
    int mode;
    int format;
    char oper[20];
    int act;
} SCcopsParm;

/**
  * @brief  AT+CFUN operation demo
  * @param  void
  * @note   Same effect as AT+CFUN
  * @retval void
  */
void CfunDemo(void)
{
    int opt = 0;
    UINT8 cfun;
    UINT8 ret;
    char cfunResp[50];
    char *note = "\r\n1.GET CFUN   2.SET CFUN 1   3.SET CFUN 0   4.SET CFUN 4  99.back.\r\n";
    while (1)
    {
        PrintfResp(note);

        opt = UartReadValue();

        switch (opt)
        {
            case 1:
            {
                ret = sAPI_NetworkGetCfun(&cfun);
                if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Get cfun success. cfun=%d!", cfun);
                    sprintf(cfunResp, "\r\nGet Cfun success. Cfun=%d!\r\n", cfun);
                    PrintfResp(cfunResp);
                    break;
                }
                else
                {
                    sAPI_Debug("Get cfun falied!");
                    PrintfResp("\r\nGet cfun falied!\r\n");
                    break;
                }
            }
            case 2:
            {
                ret = sAPI_NetworkSetCfun(1);
                if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Set cfun 1 success.!");
                    PrintfResp("\r\nSet cfun 1 success.!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("Set cfun falied!");
                    PrintfResp("\r\nSet cfun falied!\r\n");
                    break;
                }
            }
            case 3:
            {
                ret = sAPI_NetworkSetCfun(0);
                if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Set cfun 0 success.!");
                    PrintfResp("\r\nSet cfun 0 success.!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("Set cfun falied!");
                    PrintfResp("\r\nSet cfun falied!\r\n");
                    break;
                }
            }
            case 4:
            {
                ret = sAPI_NetworkSetCfun(4);
                if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Set cfun 4 success.!");
                    PrintfResp("\r\nSet cfun 4 success.!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("Set cfun falied!");
                    PrintfResp("\r\nSet cfun falied!\r\n");
                    break;
                }
            }
            case 99:
            {
                return;
            }
            default:
                break;
        }
    }
}

/**
  * @brief  AT+CGATT operation demo
  * @param  void
  * @note   Same effect as AT+CGATT
  * @retval void
  */
void CgattDemo(void)
{
    int opt = 0;
    int cgatt;
    UINT8 ret;
    char cgattresp[50] = {0};
    char *note = "\r\n1.GET CGATT   2.Packet domain attach   3.Packet domain detach   99.back.\r\n";
    while (1)
    {
        PrintfResp(note);

        opt = UartReadValue();

        switch (opt)
        {
            case 1:
            {
                ret = sAPI_NetworkGetCgatt(&cgatt);
                if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Get Cgatt success. Cgatt=%d!", cgatt);
                    sprintf(cgattresp, "\r\nGet Cgatt success. Cgatt=%d!\r\n", cgatt);
                    PrintfResp(cgattresp);
                    break;
                }
                else
                {
                    sAPI_Debug("Get cgatt falied!");
                    PrintfResp("\r\nGet cgatt falied!\r\n");
                    break;
                }
            }
            case 2:
            {
                ret = sAPI_NetworkSetCgatt(1);
                if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Set attach success.!");
                    PrintfResp("\r\nSet attach success.!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("Set attach falied!");
                    PrintfResp("\r\nSet attach falied!\r\n");
                    break;
                }
            }
            case 3:
            {
                ret = sAPI_NetworkSetCgatt(0);
                if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Set detach success.!");
                    PrintfResp("\r\nSet detach success.!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("Set detach falied!");
                    PrintfResp("\r\nSet detach falied!\r\n");
                    break;
                }
            }
            case 99:
            {
                return;
            }
            default:
                break;
        }
    }
}

/**
  * @brief  AT+CGACT=xxx operation demo
  * @param  void
  * @note   Same effect as AT+CGACT
  * @retval void
  */
void SetCgact(void)
{
    int listlen = 0;
    UINT8 ret;
    int opt = 0;
    int cgactcid = 0XFF;
    int cgactstate = 0XFF;
    char *cgact_list[] =
    {
        "1. CID:(1-16)",
        "2. state:(0,1)",
        "99.back",
    };

    while (1)
    {
        PrintfResp(cgact_list[listlen]);
        listlen++;

        opt = UartReadValue();

        switch (listlen)
        {
            case 1:
            {
                cgactcid = opt;
                if ((1 > cgactcid) || (cgactcid > 16))
                {
                    PrintfResp("\r\nPlease enter valid cid parameters\r\n");
                    return;
                }
                break;
            }
            case 2:
            {
                cgactstate = opt;
                ret = sAPI_NetworkSetCgact(cgactstate, cgactcid);
                if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Set cgactt success.!");
                    PrintfResp("\r\nSet cgact success.!\r\n");
                    return;
                }
                else
                {
                    sAPI_Debug("Set cgact falied!");
                    PrintfResp("\r\nSet cgact falied!\r\n");
                    return;
                }
            }
            case 99:
            {
                return;
            }
            default:
                break;
        }
    }
}
/**
  * @brief  AT+CGACT operation demo
  * @param  void
  * @note   Same effect as AT+CGACT
  * @retval void
  */
void CgactDemo(void)
{
    int opt = 0;
    int i;
    UINT8 ret;
    SCAPNact SCact[8] = {0};
    char cgactresp[100] = {0};
    char *note = "\r\n1.GET CGACT   2.SET CGACT   99.back.\r\n";
    while (1)
    {
        PrintfResp(note);

        opt = UartReadValue();

        switch (opt)
        {
            case 1:
            {
                ret = sAPI_NetworkGetCgact(SCact);
                if (ret == SC_NET_SUCCESS)
                {
                    for (i = 0; i < 8; i++) /*Read it eight times*/
                    {
                        sAPI_Debug("Get cgact success. cid=%d,actstate=%d!", SCact[i].cid, SCact[i].isActived);
                        if (SCact[i].isdefine)
                        {
                            sprintf(cgactresp, "\r\n cid=%d, actstate=%d\r\n", SCact[i].cid, SCact[i].isActived);
                            PrintfResp(cgactresp);
                        }
                    }
                    break;
                }
                else
                {
                    sAPI_Debug("Get cgact falied!");
                    PrintfResp("\r\nGet cgact falied!\r\n");
                    break;
                }
            }
            case 2:
            {
                SetCgact();
                break;
            }
            case 99:
            {
                return;
            }
            default:
                break;
        }
    }
}
/**
  * @brief  AT+CGDCONT operation demo
  * @param  void
  * @note   Same effect as AT+CGDCONT
  * @retval void
  */
void SetCgdcont(void)
{
    int listlen = 0;
    UINT8 ret;
    char opt[100];
    SCcgdcontParm cgdcontParm = {0, {0}, {0}};
    char *cgact_list[] =
    {
        "1. CID:(1-16)",
        "2. pdptype:(IP,IPV6,IPV4V6)",
        "3. APN:",
        "99.back",
    };

    while (1)
    {
        PrintfResp(cgact_list[listlen]);
        listlen++;

        UartReadLine(opt, sizeof(opt));
        sAPI_Debug("opt = [%s]", opt);

        switch (listlen)
        {
            case 1:
            {
                cgdcontParm.cid = atoi(opt);
                if ((1 > cgdcontParm.cid) || (cgdcontParm.cid > 16))
                {
                    PrintfResp("\r\nPlease enter valid cid parameters\r\n");
                    return;
                }
                break;
            }
            case 2:
            {
                if (strstr(opt, "\r\n") != NULL)
                    memcpy(cgdcontParm.pdptype, opt, strlen(opt) - 2);
                else
                    memcpy(cgdcontParm.pdptype, opt, strlen(opt));
                break;
            }
            case 3:
            {
                if (strstr(opt, "\r\n") != NULL)
                    memcpy(cgdcontParm.apn, opt, strlen(opt) - 2);
                else
                    memcpy(cgdcontParm.apn, opt, strlen(opt));
                sAPI_Debug("Set cgdcont:%d   %s   %s", cgdcontParm.cid, cgdcontParm.pdptype, cgdcontParm.apn);
                ret = sAPI_NetworkSetCgdcont(cgdcontParm.cid, cgdcontParm.pdptype, cgdcontParm.apn);
                if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Set cgdcont success.!");
                    PrintfResp("\r\nSet cgdcont success.!\r\n");
                    return;
                }
                else
                {
                    sAPI_Debug("Set cgdcont falied!");
                    PrintfResp("\r\nSet cgdcont falied!\r\n");
                    return;
                }
            }
            case 99:
            {
                return;
            }
            default:
                break;
        }
    }
}
/**
  * @brief  AT+CGAUTH operation demo
  * @param  void
  * @note   Same effect as AT+CGAUTH
  * @retval void
  */
void GetCGAUTH()
{
    int cid = 0;
    UINT8 ret;
    int opt = 0;
    char cgauthresp[150];
    SCCGAUTHParm cgauthParm = {0, 0, {0}, {0}};
    char cgauth_list[50] = "\r\n1. CID:(1-16),99.back\r\n";
    while (1)
    {
        PrintfResp(cgauth_list);

        opt = UartReadValue();

        switch (opt)
        {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            {
                cid = opt;
                ret = sAPI_NetworkGetCgauth(&cgauthParm, cid);
                if (SC_NET_SUCCESS == ret)
                {
                    sprintf(cgauthresp, "\r\n cid=%d,type=%d,usr=%s,passwd=%s \r\n", cgauthParm.cid, cgauthParm.authtype, cgauthParm.user,
                            cgauthParm.passwd);
                    PrintfResp(cgauthresp);
                }
                else
                {
                    sprintf(cgauthresp, "\r\n GET FAIL \r\n");
                    PrintfResp(cgauthresp);
                }
                break;
            }
            case 99:
            {
                return;
            }
            default:
                break;
        }
    }
}
/**
  * @brief  AT+CGAUTH operation demo
  * @param  void
  * @note   Same effect as AT+CGAUTH
  * @retval void
  */
void SetCGAUTH()
{
    SCCGAUTHParm auth;

    auth.cid = 1;
    auth.authtype = 2;
    UINT8 ret;
    sprintf(auth.user, "simcom");
    sprintf(auth.passwd, "simcom");
    ret = sAPI_NetworkSetCgauth(&auth,0); /*delflag:1(Delete the parameters corresponding to CID)   0(Set the parameters corresponding to CID)*/
    if (ret == SC_NET_SUCCESS)
    {
        sAPI_Debug("Set cgauth success!");
        PrintfResp("\r\nSet cgauth success!\r\n");
    }
    else
    {
        sAPI_Debug("Set cgauth falied!");
        PrintfResp("\r\nSet cgauth falied!\r\n");
    }
}
/**
  * @brief  AT+CGAUTH operation demo
  * @param  void
  * @note   Same effect as AT+CGAUTH
  * @retval void
  */
void CgdcontDemo(void)
{
    int opt = 0;
    int i;
    UINT8 ret;
    SCApnParm SCapn[8] = {0};
    char cgdcontresp[100] = {0};
    char *note = "\r\n1.GET CGDCONT   2.SET CGDCONT   3.GET CGAUTH   4.SET CGAUTH   99.back.\r\n";
    while (1)
    {
        PrintfResp(note);

        opt = UartReadValue();

        switch (opt)
        {
            case 1:
            {
                ret = sAPI_NetworkGetCgdcont(SCapn);
                if (ret == SC_NET_SUCCESS)
                {
                    for (i = 0; i < 8; i++) /*Read it eight times*/
                    {
                        sAPI_Debug("Get Cgdcont success. cid=%d,iptype=%d,apnstr=%s!", SCapn[i].cid, SCapn[i].iptype, SCapn[i].ApnStr);
                        if (SCapn[i].iptype) /*Iptype 0 means undefined*/
                        {
                            sprintf(cgdcontresp, "\r\n cid=%d,iptype=%d,apnstr=%s \r\n", SCapn[i].cid, SCapn[i].iptype, SCapn[i].ApnStr);
                            PrintfResp(cgdcontresp);
                        }
                    }
                    break;
                }
                else
                {
                    sAPI_Debug("Get cgdcont falied!");
                    PrintfResp("\r\nGet cgdcont falied!\r\n");
                    break;
                }
            }
            case 2:
            {
                SetCgdcont();
                break;
            }
            case 3:
            {
                GetCGAUTH();
                break;
            }
            case 4:
            {
                SetCGAUTH();
                break;
            }
            case 99:
            {
                return;
            }
            default:
                break;
        }
    }
}
/**
  * @brief  Set AT+COPS
  * @param  void
  * @note   Same effect as AT+COPS
  * @retval void
  */
void SetCops(void)
{
    int listlen = 0;
    UINT8 ret;
    char opt[50] = {0};
    SCcopsParm copsParm = {0, 0, {0}, 0};
    char *cgact_list[] =
    {
        "1. mode:",
        "2. format:",
        "3. oper:",
        "4. ACT:",
        "99.back",
    };

    while (1)
    {
        PrintfResp(cgact_list[listlen]);
        listlen++;

        UartReadLine(opt, sizeof(opt));
        sAPI_Debug("opt = [%s]", opt);

        switch (listlen)
        {
            case 1:
            {
                copsParm.mode = atoi(opt);
                break;
            }
            case 2:
            {
                copsParm.format = atoi(opt);
                break;
            }
            case 3:
            {
                if (strstr(opt, "\r\n") != NULL)
                    memcpy(copsParm.oper, opt, strlen(opt) - 2);
                else
                    memcpy(copsParm.oper, opt, strlen(opt));
                break;
            }
            case 4:
            {
                copsParm.act = atoi(opt);
                ret = sAPI_NetworkSetCops(copsParm.mode, copsParm.format, copsParm.oper, copsParm.act);
                if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Set cops success.!");
                    PrintfResp("\r\nSet cops success.!\r\n");
                    return;
                }
                else
                {
                    sAPI_Debug("Set cops falied!");
                    PrintfResp("\r\nSet cops falied!\r\n");
                    return;
                }
                break;
            }
            case 99:
            {
                return;
            }
            default:
                break;
        }
    }
}
/**
  * @brief  AT+COPS demo
  * @param  void
  * @note   Same effect as AT+COPS
  * @retval void
  */
void CopsDemo(void)
{
    int opt = 0;
    UINT8 ret;
    char cops[200] = {0};
    char copsresp[250] = {0};
    char *note = "\r\n1.GET COPS   2.SET COPS   3. Search COPS  99.back.\r\n";
    while (1)
    {
        PrintfResp(note);

        opt = UartReadValue();

        switch (opt)
        {
            case 1:
            {
                ret = sAPI_NetworkGetCops(cops);
                if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Get cops success. cops=%s!", cops);
                    sprintf(copsresp, "\r\nGet cops success. cops=%s!\r\n", cops);
                    PrintfResp(copsresp);
                    break;
                }
                else
                {
                    sAPI_Debug("Get cops falied!");
                    PrintfResp("\r\nGet cops falied!\r\n");
                    break;
                }
            }
            case 2:
            {
                SetCops();
                break;
            }
#if 0
			case 3:
			{
				ret = sAPI_NetworkTestCops(cops);
				PrintfResp("Searching waiting...");
				if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Serach cops success!");
                    sprintf(copsresp, "\r\nSearch cops success!\r\n %s", cops);
                    PrintfResp(copsresp);
                    break;
                }
                else
                {
					PrintfResp("Searching...");
					sAPI_TaskSleep(200);
					continue;/*retry*/
                }
			}
#endif
            case 99:
            {
                return;
            }
            default:
                break;
        }
    }
}
/**
  * @brief  AT+CGPADDR demo
  * @param  void
  * @note   Same effect as AT+CGPADDR
  * @retval void
  */
void CgpaddrDemo(void)
{
    int opt = 0;
    UINT8 ret;
    SCcgpaddrParm cgpaddrParm;
    char *note = "\r\n1.GET (1-15) IPaddr  99.back.\r\n";
    char NetResp[200];

    while (1)
    {
        PrintfResp(note);

        opt = UartReadValue();

        switch (opt)
        {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            {
                memset(&cgpaddrParm, 0, sizeof(cgpaddrParm));
                ret = sAPI_NetworkGetCgpaddr(opt, &cgpaddrParm);
                if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Get Ipaddr success. cid=%d,type=%d,ipv4=%s,ipv6=%s!", cgpaddrParm.cid, cgpaddrParm.iptype,
                               cgpaddrParm.ipv4addr, cgpaddrParm.ipv6addr);
                    sprintf(NetResp, "\r\nGet Ipaddr success. cid=%d,type=%d,ipv4=%s,ipv6=%s!", cgpaddrParm.cid, cgpaddrParm.iptype,
                            cgpaddrParm.ipv4addr, cgpaddrParm.ipv6addr);
                    PrintfResp(NetResp);
                    break;
                }
                else
                {
                    sAPI_Debug("Get Ipaddr falied!");
                    PrintfResp("\r\nGet Ipaddr falied!\r\n");
                    break;
                }
            }
            case 99:
            {
                return;
            }
            default:
                break;
        }
    }
}
/**
  * @brief  AT+CNMP demo
  * @param  void
  * @note   Same effect as AT+CNMP
  * @retval void
  */
void CnmpDemo(void)
{
    int opt = 0;
    int cnmp;
    char cnmpresp[50];
    UINT8 ret;
    char *note = "\r\n1.GET CNMP   2.SET CNMP:2   13.SET CNMP:13   38.SET CNMP:38   99.back.\r\n";
    while (1)
    {
        PrintfResp(note);

        opt = UartReadValue();

        switch (opt)
        {
            case 1:
            {
                ret = sAPI_NetworkGetCnmp(&cnmp);
                if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Get cnmp success. cnmp=%d!", cnmp);
                    sprintf(cnmpresp, "\r\nGet cnmp success. cnmp:%d!\r\n", cnmp);
                    PrintfResp(cnmpresp);
                    break;
                }
                else
                {
                    sAPI_Debug("Get cnmp falied!");
                    PrintfResp("\r\nGet cnmp falied!\r\n");
                    break;
                }
            }
            case 2:
            {
                ret = sAPI_NetworkSetCnmp(2);
                if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Set cnmp 2 success.!");
                    PrintfResp("\r\nSet cnmp 2 success.!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("Set cnmp falied!");
                    PrintfResp("\r\nSet cnmp falied!\r\n");
                    break;
                }
            }
            case 13:
            {
                ret = sAPI_NetworkSetCnmp(13);
                if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Set cnmp 13 success.!");
                    PrintfResp("\r\nSet cnmp 13 success.!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("Set cnmp falied!");
                    PrintfResp("\r\nSet cnmp falied!\r\n");
                    break;
                }
            }
            case 38:
            {
                ret = sAPI_NetworkSetCnmp(38);
                if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Set cnmp 38 success.!");
                    PrintfResp("\r\nSet cnmp 38 success.!\r\n");
                    break;
                }
                else
                {
                    sAPI_Debug("Set cnmp falied!");
                    PrintfResp("\r\nSet cnmp falied!\r\n");
                    break;
                }
            }
            case 99:
            {
                return;
            }
            default:
                break;
        }
    }
}
#ifdef PLATFORM_160X  //104 baseline 110 baseline ...
void RealTimeDemo(void)
{
	int opt = 0;
	char RTbuf[50]={0};
	int ctzuvalue = 0;
	scRealTime Srealtime;
    UINT8 ret;
    char *note = "\r\n1. Get CCLK   2. Get CTZU   3. Set CTZU   4. Set CFUN0   5. Set CFUN1   6. Get RealTime    99.back\r\n";
    while(1)
    {
        PrintfResp(note);
        opt = UartReadValue();
        switch(opt)
        {
            case 1:
            {
				ret = sAPI_NetworkGetRealTime(&Srealtime);
                if(ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Get realtime success.!");
					PrintfResp("\r\n(If you want get real time, pls set ctzu, then set cfun0, cfun1 to update firstly!)\r\n");
					sprintf(RTbuf,"\r\nRT: 20%02d-%02d-%02d %02d:%02d:%02d:%c%02d\r\n",Srealtime.year,Srealtime.mon,\
						Srealtime.day,Srealtime.hour,Srealtime.min,Srealtime.sec,Srealtime.sign,Srealtime.timezone);
                    PrintfResp(RTbuf);
                    break;
                }
                else
                {
                    sAPI_Debug("Get realtime falied!");
                    PrintfResp("\r\nGet realtime falied!\r\n");
                    break;
                }
            }
			case 2:
			{
				ret = sAPI_NetworkGetCtzu(&ctzuvalue);
                if(ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Get CTZU success. CTZU=%d!",ctzuvalue);
                    sprintf(RTbuf,"\r\nCTZU=%d!\r\n",ctzuvalue);
                    PrintfResp(RTbuf);
                    break;
                }
                else
                {
                    sAPI_Debug("GetCTZU falied!");
                    PrintfResp("\r\nGet CTZU falied!\r\n");
                    break;
                }

			}
			case 3:
			{
				ret = sAPI_NetworkSetCtzu(1);
                if(ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Set CTZU success.");
                    sprintf(RTbuf,"\r\nSet CTZU success\r\n");
                    PrintfResp(RTbuf);
                    break;
                }
                else
                {
                    sAPI_Debug("SetCTZU falied!");
                    PrintfResp("\r\nSet CTZU falied!\r\n");
                    break;
                }
			}
			case 4:
			{
				ret = sAPI_NetworkSetCfun(0);
				if(ret == SC_NET_SUCCESS)
				{
					sAPI_Debug("Set cfun 0 success.!");
					PrintfResp("\r\nSet cfun 0 success.!\r\n");
					break;
				}
				else
				{
					sAPI_Debug("Set cfun falied!");
					PrintfResp("\r\nSet cfun falied!\r\n");
					break;
				}
			}
			case 5:
			{
				ret = sAPI_NetworkSetCfun(1);
				if(ret == SC_NET_SUCCESS)
				{
					sAPI_Debug("Set cfun 1 success.!");
					PrintfResp("\r\nSet cfun 1 success.!\r\n");
					break;
				}
				else
				{
					sAPI_Debug("Set cfun falied!");
					PrintfResp("\r\nSet cfun falied!\r\n");
					break;
				}
			}
			case 6:
			{
				sAPI_NetworkSetCtzu(1);
				sAPI_TaskSleep(100);
				sAPI_NetworkSetCfun(0);
				sAPI_TaskSleep(200*3);
				sAPI_NetworkSetCfun(1);
				sAPI_TaskSleep(200*3);
				ret = sAPI_NetworkGetRealTime(&Srealtime);
				if(ret == SC_NET_SUCCESS)
				{
					sAPI_Debug("Get realtime success.!");
					sprintf(RTbuf,"\r\nRT: 20%02d-%02d-%02d %02d:%02d:%02d:%c%02d\r\n",Srealtime.year,Srealtime.mon,\
						Srealtime.day,Srealtime.hour,Srealtime.min,Srealtime.sec,Srealtime.sign,Srealtime.timezone);
					PrintfResp(RTbuf);
					break;
				}
				else
				{
					sAPI_Debug("Get realtime falied!");
					sAPI_TaskSleep(200);
					continue;/*retry*/
				}
			}
            case 99:
            {
                return;
            }
            default:
                break;
        }
   }
}

void PowerSaveModeDemo(void)
{
	int opt = 0;
	char RTbuf[50]={0};
    char *t3324 = "00001111";//30s
    UINT8 ret;
    char *note = "\r\n1. Enable 1Bis  2. Set CFUN0  3. Set T3324  4. Enable HardWare PSM   5. Set T3412   6. Set CFUN1    7 Close PSM   99.back\r\n";
    while(1)
    {
        PrintfResp(note);
        opt = UartReadValue();
        switch(opt)
        {
            case 1:
            {
				sAPI_EnableOneBis();
                sAPI_TaskSleep(400);
                PrintfResp("\r\nEnable 1 bis!\r\n");
                break;
            }
			case 2:
			{
				ret = sAPI_NetworkSetCfun(0);
				if(ret == SC_NET_SUCCESS)
				{
					sAPI_Debug("Set cfun 0 success.!");
					PrintfResp("\r\nSet cfun 0 success.!\r\n");
					break;
				}
				else
				{
					sAPI_Debug("Set cfun 0 falied!");
					PrintfResp("\r\nSet cfun 0 falied!\r\n");
					break;
				}
			}
			case 3:
			{
				ret = sAPI_PSMSetT3324Timer(1,t3324);//30s
                if(ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Set T3324 Timer success.");
                    sprintf(RTbuf,"\r\nSet T3324 Timer success\r\n");
                    PrintfResp(RTbuf);
                    break;
                }
                else
                {
                    sAPI_Debug("Set T3324 Timer falied!");
                    PrintfResp("\r\n Set T3324 Timer falied!\r\n");
                    break;
                }
			}
			case 4:
			{
				sAPI_EnableHardWarePSM();
                PrintfResp("\r\nEnable Hardware PSM!\r\n");
                break;
			}
			case 5:
			{
				ret = sAPI_PSMSetT3412Timer(2);//2min
				if(ret == SC_NET_SUCCESS)
				{
					sAPI_Debug("Set T3412 Timer success.!");
					PrintfResp("\r\nSet T3412 Timer success.!\r\n");
					break;
				}
				else
				{
					sAPI_Debug("Set T3412 Timer falied!");
					PrintfResp("\r\nSet T3412 Timer falied!\r\n");
					break;
				}
			}
			case 6:
			{
				ret = sAPI_NetworkSetCfun(1);
				if(ret == SC_NET_SUCCESS)
				{
					sAPI_Debug("Set cfun 1 success.!");
					PrintfResp("\r\nSet cfun 1 success.!\r\n");
					break;
				}
				else
				{
					sAPI_Debug("Set cfun 1 falied!");
					PrintfResp("\r\nSet cfun 1 falied!\r\n");
					break;
				}

			}
			case 7:
			{
				ret = sAPI_PSMSetT3324Timer(0,"0");//Close PSM
                if(ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Close PSM success.");
                    sprintf(RTbuf,"\r\nClose PSM success\r\n");
                    PrintfResp(RTbuf);
                    break;
                }
                else
                {
                    sAPI_Debug("Close PSM falied!");
                    PrintfResp("\r\n Close PSM falied!\r\n");
                    break;
                }
			}

            case 99:
            {
                return;
            }
            default:
                break;
        }
   }
}

#endif
/**
  * @brief  Network management demo
  * @param  void
  * @note   This demo shows how to manage network by API.
  * @retval void
  */
void NetWorkDemo(void)
{
    UINT8 csq;
    UINT8 ret;
    UINT8 cpin;
    int creg, cgreg;
    SCcpsiParm Scpsi = { 0 };
#ifdef PLATFORM_160X  //104 baseline 110 baseline ...
    SCcnetciParm Snetci;
#else
    SCcnetciParm Snetci[12];
#endif
    char NetResp[1000] = {0};
    int opt = 0;
    int APInum = 0;
#ifndef FALCON_1803_PLATFORM  //104 baseline 110 baseline ...
    SCdialapnparm dialapnparm[6];
    SCdialapnparm apnparm = {0};
#endif

    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] =
    {
        "1. Query signal quality(CSQ)",
        "2. CS domain(CREG)",
        "3. PS domain(CGREG)",
        "4. system information(CPSI)",
        "5. mode selection(CNMP)",
        "6. Operator selection(COPS)",
        "7. PDP context(CGDCONT)",
        "8. act or deact(CGACT)",
        "9. attach or detach(CGATT)",
        "10.phone functionality(CFUN)",
        "11.IP Address(CGPADDR)",
        "12.Adjacent NB information(CNETCI)",
        "13.Test GPRS",
        "14.Test NETWORK MODE",
        "15.Test Multiple Dialing ",
        "16.Test RealTime(CCLK) ",
        "17.Test PSM",
        "18.Test GetDnsAddr",
#ifdef FALCON_1803_PLATFORM
        "19.TXRX_POWER demo"
#endif
        "99.back",
    };
    /*Create flag for network,the initialization operation must be used*/
    sAPI_NetworkInit();

    while (1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list, sizeof(options_list) / sizeof(options_list[0]));

        opt = UartReadValue();

        switch (opt)
        {
            case 1:
            {
                ret = sAPI_NetworkGetCsq(&csq);
                if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Get csq success. csq=%d!", csq);
                    sprintf(NetResp, "\r\nGet csq success. csq:%d!\r\n", csq);
                    PrintfResp(NetResp);
                    break;
                }
                else
                {
                    sAPI_Debug("Get csq falied!");
                    PrintfResp("\r\nGet csq falied!\r\n");
                    break;
                }
            }

            case 2:
            {
                ret = sAPI_NetworkGetCreg(&creg);
                if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Get creg success. creg=%d!", creg);
                    sprintf(NetResp, "\r\nGet creg success. creg=%d!\r\n", creg);
                    PrintfResp(NetResp);
                    break;
                }
                else
                {
                    sAPI_Debug("Get creg falied!");
                    PrintfResp("\r\nGet creg falied!\r\n");
                    break;
                }
            }

            case 3:
            {
                ret = sAPI_NetworkGetCgreg(&cgreg);
                if (ret == SC_NET_SUCCESS)
                {
                    sAPI_Debug("Get cgreg success. cgreg=%d!", cgreg);
                    sprintf(NetResp, "\r\nGet cgreg success. cgreg=%d!\r\n", cgreg);
                    PrintfResp(NetResp);
                    break;
                }
                else
                {
                    sAPI_Debug("Get cgreg falied!");
                    PrintfResp("\r\nGet cgreg falied!\r\n");
                    break;
                }
            }

            case 4:
            {
                ret = sAPI_NetworkGetCpsi(&Scpsi);
                if (ret == SC_NET_SUCCESS)
                {
#ifndef FALCON_1803_PLATFORM  //104 baseline 110 baseline ...
                    sAPI_Debug("Get cpsi success. NEmode=%s,MM=%s,LAC=%d,CELL=%d,Gband=%s,Lband=%s,TAC=%d,RSRP=%d,RXLEV=%d,TA=%d!",
                               Scpsi.networkmode, Scpsi.Mnc_Mcc, Scpsi.LAC, Scpsi.CellID, Scpsi.GSMBandStr, Scpsi.LTEBandStr, Scpsi.TAC, Scpsi.Rsrp,
                               Scpsi.RXLEV, Scpsi.TA);
                    sprintf(NetResp,
                            "\r\nGet cpsi success. NEmode=%s,MM=%s,LAC=%d,CELL=%d,Gband=%s,Lband=%s,TAC=%d,RSRP=%d,RXLEV=%d,TA=%d,SINR=%d!\r\n",
                            Scpsi.networkmode, Scpsi.Mnc_Mcc, Scpsi.LAC, Scpsi.CellID, Scpsi.GSMBandStr, Scpsi.LTEBandStr, Scpsi.TAC, Scpsi.Rsrp,
                            Scpsi.RXLEV, Scpsi.TA, Scpsi.SINR);
#else
                    sAPI_Debug("Get cpsi success. NEmode=%s,MM=%s,LAC=%d,CELL=%d,Gband=%s,Lband=%s,TAC=%d,RXLEV=%d,TA=%d!",
                               Scpsi.networkmode, Scpsi.Mnc_Mcc, Scpsi.LAC, Scpsi.CellID, Scpsi.GSMBandStr, Scpsi.LTEBandStr, Scpsi.TAC,
                               Scpsi.RXLEV, Scpsi.TA);
                    sprintf(NetResp,
                            "\r\nGet cpsi success. NEmode=%s,MM=%s,LAC=%d,CELL=%d,Gband=%s,Lband=%s,TAC=%d,RXLEV=%d,TA=%d!\r\n",
                            Scpsi.networkmode, Scpsi.Mnc_Mcc, Scpsi.LAC, Scpsi.CellID, Scpsi.GSMBandStr, Scpsi.LTEBandStr, Scpsi.TAC,
                            Scpsi.RXLEV, Scpsi.TA);
#endif
                    PrintfResp(NetResp);
                    break;
                }
                else
                {
                    sAPI_Debug("Get cpsi falied!");
                    PrintfResp("\r\nGet cpsi falied!\r\n");
                    break;
                }
            }

            case 5:
            {
                CnmpDemo();
                break;
            }

            case 6:
            {
                CopsDemo();
                break;
            }

            case 7:
            {
                CgdcontDemo();
                break;
            }

            case 8:
            {
                CgactDemo();
                break;
            }

            case 9:
            {
                CgattDemo();
                break;
            }

            case 10:
            {
                CfunDemo();
                break;
            }
            case 11:
            {
                CgpaddrDemo();
                break;
            }
            case 12:
            {
#ifdef PLATFORM_160X  //104 baseline 110 baseline ...
                int i;
                ret = sAPI_NetworkGetCnetci(&Snetci);
                if (ret == SC_NET_SUCCESS)
                {
                    for(i = 0; i < Snetci.scLteInterCellNum; i++)
                    {
                        sAPI_Debug("CNETCI-INTER-CELL: %d--0X%3X-%02X cellId: %#x tac: %#X pci: %d rsrp: %d rsrq: %d earfcn: %d rssi %d\n", i,
                            Snetci.scLteInterCell[i].mcc,Snetci.scLteInterCell[i].mnc,Snetci.scLteInterCell[i].cell_id,Snetci.scLteInterCell[i].tac,
                            Snetci.scLteInterCell[i].pcid,Snetci.scLteInterCell[i].rsrp,Snetci.scLteInterCell[i].rsrq,
                            Snetci.scLteInterCell[i].earfcn,Snetci.scLteInterCell[i].rssi);
                            sprintf(NetResp, "Inter-Cell: %d--0X%3X-%02X cellId: %#x tac: %#X pci: %d rsrp: %d rsrq: %d earfcn: %d rssi %d\n", i, 
                                Snetci.scLteInterCell[i].mcc,Snetci.scLteInterCell[i].mnc,Snetci.scLteInterCell[i].cell_id,Snetci.scLteInterCell[i].tac,
                            Snetci.scLteInterCell[i].pcid,Snetci.scLteInterCell[i].rsrp,Snetci.scLteInterCell[i].rsrq,
                            Snetci.scLteInterCell[i].earfcn,Snetci.scLteInterCell[i].rssi);
                            PrintfResp(NetResp);
                    }
                    for(i = 0; i < Snetci.scLteIntraCellNum; i++)
                    {
                        sAPI_Debug("CNETCI-INTRA-CELL: %d--0X%3X-%02X cellId: %#x tac: %#X pci: %d rsrp: %d rsrq: %d earfcn: %d rssi %d\n", i,
                            Snetci.scLteIntraCell[i].mcc,Snetci.scLteIntraCell[i].mnc,Snetci.scLteIntraCell[i].cell_id,Snetci.scLteIntraCell[i].tac,
                            Snetci.scLteIntraCell[i].pcid,Snetci.scLteIntraCell[i].rsrp,Snetci.scLteIntraCell[i].rsrq,
                            Snetci.scLteIntraCell[i].earfcn,Snetci.scLteIntraCell[i].rssi);
                            sprintf(NetResp, "Intra-Cell: %d--0X%3X-%02X cellId: %#x tac: %#X pci: %d rsrp: %d rsrq: %d earfcn: %d rssi %d\n", i, 
                                Snetci.scLteIntraCell[i].mcc,Snetci.scLteIntraCell[i].mnc,Snetci.scLteIntraCell[i].cell_id,Snetci.scLteIntraCell[i].tac,
                            Snetci.scLteIntraCell[i].pcid,Snetci.scLteIntraCell[i].rsrp,Snetci.scLteIntraCell[i].rsrq,
                            Snetci.scLteIntraCell[i].earfcn,Snetci.scLteIntraCell[i].rssi);
                            PrintfResp(NetResp);
                    }
                    break;
                }
                else
                {
                    sAPI_Debug("Get cnetci falied!");
                    PrintfResp("\r\nGet cnetci falied!\r\n");
                    break;
                }
#else // other version
                ret = sAPI_NetworkGetCnetci(Snetci);
                if (ret == SC_NET_SUCCESS)
                {
                    int i = 0;
                    for (i = 0; i < 12; i++) /*Read it ten times*/
                    {
                        sAPI_Debug("Get cnetci success.i=%d:MM=%s,TAC=%d,CELL=%d,RSRP=%d,RSRQ=%d,RXSIGLEVEL=%d", i, Snetci[i].Mnc_Mcc,
                                   Snetci[i].TAC, Snetci[i].CellID, Snetci[i].Rsrp, Snetci[i].Rsrq, Snetci[i].RXSIGLEVEL);
                        if (Snetci[i].Rsrp || Snetci[i].RXSIGLEVEL) /*Iptype 0 means undefined*/
                        {
                            sprintf(NetResp, "\r\n i=%d,MM=%s,TAC=%d,CELL=%d,RSRP=%d,RSRQ=%d,RXSIGLEVEL=%d", i, Snetci[i].Mnc_Mcc, Snetci[i].TAC,
                                    Snetci[i].CellID, Snetci[i].Rsrp, Snetci[i].Rsrq, Snetci[i].RXSIGLEVEL);
                            PrintfResp(NetResp);
                        }
                    }
                    break;
                }
                else
                {
                    sAPI_Debug("Get cnetci falied!");
                    PrintfResp("\r\nGet cnetci falied!\r\n");
                    break;
                }
#endif
            }
            case 13:
            {
                /*This is an example of testing the state of the network ready to use a data service,
                This is a generic example,Users can also design according to their actual needs*/
                PrintfResp("\r\nThis is a generic example,Users can also design according to their actual needs\r\n");
                while (1)
                {
                    if (APInum > 20) /*You can define the number of retries you want*/
                    {
                        /*check card*/
                        break;
                    }
                    APInum++;
                    ret = sAPI_SimcardPinGet(&cpin);/*Power on to inquire the SIM card status*/
                    if(ret == SC_NET_SUCCESS)
                    {
                        if(cpin == SC_SIM_READY)
                        {
                            PrintfResp("\r\nGet cpin success and cpin ready\r\n");
                            break;
                        }
                        else if((cpin == SC_SIM_PIN)||(cpin == SC_SIM_PUK))
                        {
                            PrintfResp("\r\nGet cpin success and cpin PIN\r\n");
                            sAPI_TaskSleep(200);

                            /*<Call the SimCard interface>*/

                            sAPI_TaskSleep(200);
                            continue;/*retry*/
                        }
                        else
                        {
                            sAPI_TaskSleep(200);
                            continue;/*retry*/
                        }
                    }
                    else
                    {
                        sAPI_TaskSleep(200);/*All API query times are recommended once 1S*/
                        PrintfResp("\r\nGet cpin falied!\r\n");
                        //continue;/*If this fails, you can wait for 1S to continue the query*/
                        break;
                    }
                }
                while (1)
                {
                    if (APInum > 60) /*You can define the number of retries you want*/
                    {
                        /*It is recommended to switch to cFUN or CNMP mode*/
                        break;
                    }
                    APInum++;
                    ret = sAPI_NetworkGetCpsi(&Scpsi);/*Power on to inquire the SIM card status*/
                    if (ret == SC_NET_SUCCESS)
                    {
                        if (strncmp(Scpsi.networkmode, "No", sizeof(char) * 2) == 0)
                        {
                            sAPI_TaskSleep(200);
                            continue;/*retry*/
                        }
                        if (strncmp(Scpsi.networkmode, "LTE", sizeof(char) * 3) == 0)
                        {
                            PrintfResp("\r\nGet cpsi success and LTE\r\n");
                            break;/*You can use the TCPIP API*/
                        }
                        if (strncmp(Scpsi.networkmode, "GSM", sizeof(char) * 3) == 0)
                        {
                            PrintfResp("\r\nGet cpsi success and GSM\r\n");
                            break;/*Maybe some other API has to be called*/
                        }
                    }
                    else
                    {
                        sAPI_TaskSleep(200);/*All API query times are recommended once 1S*/
                        PrintfResp("\r\nGet networkmode falied!\r\n");
                        //continue;/*If this fails, you can wait for 1S to continue the query*/
                        break;
                    }
                }
                /*If you are already registered for a GSM network,You can try switching CNMP =38 and register LTE*/
                /*If you accept GSM,Please refer to the CgdcontDemo:case 2,After setting it up successfully,You can use the TCPIP API*/
            }
            case 14:
            {
                /*The detection of the card refers to Case 15*/
               PrintfResp("\r\nThis is a generic example,Users can also design according to their actual needs\r\n");
                while (1)
                {
                    if (APInum > 60) /*You can define the number of retries you want*/
                    {
                        /*It is recommended to switch to cFUN or CNMP mode*/
                        break;
                    }
                    APInum++;
                    ret = sAPI_NetworkGetCpsi(&Scpsi);
                    if (ret == SC_NET_SUCCESS)
                    {
                        if (strncmp(Scpsi.networkmode, "No", sizeof(char) * 2) == 0)
                        {
                            sAPI_TaskSleep(200);
                            continue;/*retry*/
                        }
                        if (strncmp(Scpsi.networkmode, "LTE", sizeof(char) * 3) == 0)
                        {
                            PrintfResp("\r\nGet cpsi success and LTE\r\n");
                            break;
                        }
                        if (strncmp(Scpsi.networkmode, "GSM", sizeof(char) * 3) == 0)
                        {
                            PrintfResp("\r\nGet cpsi success and GSM\r\n");
                            break;
                        }
                    }
                    else
                    {
                        sAPI_TaskSleep(200);/*All API query times are recommended once 1S*/
                        PrintfResp("\r\nGet CPSI falied!\r\n");
                        //continue;/*If this fails, you can wait for 1S to continue the query*/
                        break;
                    }
                }
                while (1)
                {
                    if (APInum > 30) /*You can define the number of retries you want*/
                    {
                        /*It is recommended to switch to cFUN or CNMP mode*/
                        break;
                    }
                    APInum++;
                    ret = sAPI_NetworkGetCreg(&creg);/*creg judgment cs      cgreg judgment ps*/
                    if (ret == SC_NET_SUCCESS)
                    {
                        if ((creg == 1) || (creg == 5))
                        {
                            /*you can voice call*/
                            PrintfResp("\r\nNetwork has registed!\r\n");
                            break;
                        }
                        else
                        {
                            sAPI_TaskSleep(200);
                            continue;/*retry*/
                        }
                    }
                    else
                    {
                        sAPI_TaskSleep(200);/*All API query times are recommended once 1S*/
                        PrintfResp("\r\nNetwork registed falied!\r\n");
                        //continue;/*If this fails, you can wait for 1S to continue the query*/
                        break;
                    }
                }
                break;
            }
            case 15:
            {
#ifndef FALCON_1803_PLATFORM  //104 baseline 110 baseline ...
                int i;
                int ret = -1;
                sprintf(apnparm.ip_type, "IP");
                sprintf(apnparm.apn, "CMNET_0");
                sAPI_NetworkSetCnetcon(0, &apnparm);
                memset(&apnparm, 0, sizeof(SCdialapnparm));

                sprintf(apnparm.ip_type, "IP");
                sprintf(apnparm.apn, "CMNET_1");
                sAPI_NetworkSetCnetcon(1, &apnparm);

                memset(dialapnparm, 0, sizeof(SCdialapnparm));
                sAPI_NetworkGetCnetcon(dialapnparm);
                for (i = 0; i < 6; i++)
                {
                    sAPI_Debug("%s,%d,%s,%s,%s,%s,%d", __func__, i, dialapnparm[i].apn, dialapnparm[i].user, dialapnparm[i].pswd,
                               dialapnparm[i].ip_type, dialapnparm[i].auth);
                    if(strlen (dialapnparm[i].apn) != 0)
                    {
                        memset(NetResp, 0, sizeof(NetResp));
                        sprintf(NetResp, "\r\n %d,%s,%s", i, dialapnparm[i].apn,dialapnparm[i].ip_type);
                        PrintfResp(NetResp);
                    }
                }
                ret = sAPI_TcpipPdpActive(1, 1);
                if(ret == 0){
                sprintf(NetResp, "\r\n cid 1 actived!");
                PrintfResp(NetResp);
                }
                sAPI_TaskSleep(400);
                ret = sAPI_TcpipPdpActive(2, 1);
                if(ret == 0){
                sprintf(NetResp, "\r\n cid 2 actived!");
                PrintfResp(NetResp);
                }
#else
                sprintf(NetResp, "\r\n To Do...!");
                PrintfResp(NetResp);
#endif
                break;
            }
            case 16:
            {
#ifdef PLATFORM_160X  //104 baseline 110 baseline ...
                RealTimeDemo ();
#else
                sprintf(NetResp, "\r\n To Do...!");
                PrintfResp(NetResp);
#endif
                break;
            }
            case 17:
            {
                //NetLedPlatConnectDemo();
                //PrintfResp("To do...");
#ifdef PLATFORM_160X  //104 baseline 110 baseline ...
            PowerSaveModeDemo();
#else
            sprintf(NetResp, "\r\n To Do...!");
            PrintfResp(NetResp);
#endif
                break;
            }
            case 18:
            {
#ifdef PLATFORM_160X
                int cid = 1;
                SCDNSAddr dnsAddr = {0};
                ret = sAPI_NetworkGetDNSAddr(cid, &dnsAddr);
                if(ret == SC_NET_SUCCESS)
                {
                    memset(NetResp, 0, sizeof(NetResp));
                    sprintf(NetResp, "\r\n cid: %d,dnsIpv4PrimAddr: %s,dnsIpv4SecAddr: %s,dnsIpv6PrimAddr: %s,dnsIpv6SecAddr: %s\r\n",
                           dnsAddr.cid + 1, dnsAddr.dnsIpv4PrimAddr,dnsAddr.dnsIpv4SecAddr,dnsAddr.dnsIpv6PrimAddr,dnsAddr.dnsIpv6SecAddr);
                    PrintfResp(NetResp);
                }
                else
                {
                    sAPI_TaskSleep(200);/*All API query times are recommended once 1S*/
                    PrintfResp("\r\nGet dnsAddr falied!\r\n");
                    //continue;/*If this fails, you can wait for 1S to continue the query*/
                    break;
                }
#else
                            sprintf(NetResp, "\r\n To Do...!");
                            PrintfResp(NetResp);
#endif
            }
#ifdef FALCON_1803_PLATFORM
            case 19:
            {
                extern void txRxPowerDemo(void);
                txRxPowerDemo();
            }
#endif
            case 99:
            {
                return;
            }
            default :
                break;
        }
    }
}

