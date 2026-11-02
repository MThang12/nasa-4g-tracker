/**
 * \file ssl.h
 *
 * \brief SSL/TLS functions.
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */
//#include "ci_mm.h"
#ifndef _SIMCOM_NETWORK_H_
#define _SIMCOM_NETWORK_H_

#include "simcom_os.h"
#define APIHandle 0XFF
#define PLATFORM_160X

#define SC_GET_CSQ_MASK                 (0x01<<SC_GET_CSQ)
#define SC_GET_CREG_MASK                (0x01<<SC_GET_CREG)
#define SC_GET_CGREG_MASK               (0x01<<SC_GET_CGREG)
#define SC_GET_CPSI_MASK                (0x01<<SC_GET_CPSI)
#define SC_GET_CNMP_MASK                (0x01<<SC_GET_CNMP)
#define SC_GET_CNBP_MASK                (0x01<<SC_GET_CNBP)
#define SC_GET_MODE_MASK                (0x01<<SC_GET_MODE)
#define SC_GET_COPS_MASK                (0x01<<SC_GET_COPS)
#define SC_GET_CGDCONT_MASK             (0x01<<SC_GET_CGDCONT)
#define SC_GET_CGACT_MASK               (0x01<<SC_GET_CGACT)
#define SC_GET_CGATT_MASK               (0x01<<SC_GET_CGATT)
#define SC_GET_CGPADDR_MASK             (0x01<<SC_GET_CGPADDR)
#define SC_GET_CPIN_MASK                (0x01<<SC_GET_CPIN)
#define SC_GET_CFUN_MASK                (0x01<<SC_GET_CFUN)
#define SC_SET_CFUN_MASK                (0x01<<SC_SET_CFUN)
#define SC_GET_IMSI_MASK                (0x01<<SC_GET_IMSI)
#define SC_GET_HPLMN_MASK               (0x01<<SC_GET_HPLMN)
#define SC_GET_CNETCI_MASK              (0x01<<SC_GET_CNETCI)
#define SC_SET_CPIN_MASK                (0x01<<SC_SET_CPIN)
#define SC_GET_DNSADDR_MASK             (0x01<<SC_GET_DNSADDR)

#define SC_MAX_NEICELL_NUM  12
#define SC_MAX_LTE_NEIGHBORING_CELLS 6
#define SC_MAX_GSM_NEIGHBORING_CELLS 6

/**  \brief Encoded bit error rate (BER) indication */
/** \remarks Common Data Section */
typedef UINT8 SCciMmEncodedBER; /**< Reported as an index between 0-7, inclusive  */

/** \brief  Signal quality indications: information structure */
typedef struct SCciMmSigNormalQualityInfo_struct {
    UINT8       Rssi; 
    UINT8       Ber;
} SCciMmSigNormalQualityInfo;

typedef struct{
    char networkmode[40];
    char Mnc_Mcc[20];
    char GSMBandStr[20];
    char LTEBandStr[20];
    int LAC;
    int CellID;
    int TAC;
    int Rsrp;
    int Rsrq;
    int RXLEV;
    int TA;
    int SINR;
    int Rssi;
    int dlEuArfcn;
    int subframeAssignment;
    int systemFrameNumber;
    int pCellID;
}SCcpsiParm;

typedef struct
{
	char user[64];
	char pswd[64];
	char apn[64];
	char ip_type[64];
	INT32 auth;
}SCdialapnparm;

typedef struct{
    int mcc;
    int mnc;
    int cell_id;
    int tac;
    int rsrp;
    int rsrq;
    int earfcn; 
    int rssi;
    int rrc;
    int pcid;
}SCLteNeiCellParm;

typedef struct{
    int mcc;
    int mnc;
    int cell_id;
    int lac;
    int arfcn; 
    int bsic;
    int rssi;
    int pcid;
    int rxsiglevel;
}SCGsmNeiCellParm;

typedef struct{
    UINT8  scLteIntraCellNum;
    SCLteNeiCellParm scLteIntraCell[SC_MAX_LTE_NEIGHBORING_CELLS];
    UINT8   scLteInterCellNum;
    SCLteNeiCellParm scLteInterCell[SC_MAX_LTE_NEIGHBORING_CELLS];
    UINT8   scLteNeiGsmCellNum;
    SCGsmNeiCellParm scLteNeiGsmCell[SC_MAX_GSM_NEIGHBORING_CELLS];
    UINT8   scGsmNeiGsmCellNum;
    SCGsmNeiCellParm scGsmNeiGsmCell[SC_MAX_GSM_NEIGHBORING_CELLS];
    UINT8   scGsmNeiLteCellNum;
    SCLteNeiCellParm scGsmNeiLteCell[SC_MAX_GSM_NEIGHBORING_CELLS];
}SCcnetciParm;

typedef struct{
    UINT8 cid;
    UINT8 iptype;
    char ipv4addr[16];
    char ipv6addr[64];
}SCcgpaddrParm;

typedef struct{
    UINT8 cid;
    UINT8 iptype;
    char ApnStr[40];
}SCApnParm;

typedef struct{
    UINT8 cid;
    UINT8 authtype;
    char user[50];
    char passwd[50];
}SCCGAUTHParm;

typedef struct{
    UINT8 cid;
    UINT8 isActived;
    UINT8 isdefine;
}SCAPNact;

typedef struct{
    UINT8 cid;
    char *dnsIpv4PrimAddr;
    char *dnsIpv4SecAddr;
    char *dnsIpv6PrimAddr;
    char *dnsIpv6SecAddr;
}SCDNSAddr;

typedef unsigned long long uint64_t;

enum SCnetworkid
{
    SC_GET_CSQ = 0,
    SC_GET_CREG,
    SC_GET_CGREG,
    SC_GET_CPSI,
    SC_GET_CNMP,
    SC_GET_CNBP,
    SC_GET_MODE,
    SC_GET_COPS,
    SC_GET_CGDCONT,
    SC_GET_CGACT,
    SC_GET_CGATT,
    SC_GET_CGPADDR,
    SC_GET_CFUN,
    SC_SET_CFUN,
    SC_GET_CNETCI,
    SC_GET_DNSADDR
};
enum SCnetworkSF
{
    SC_NET_SUCCESS = 0,
    SC_NET_FAIL
};

enum SCcfunvalue
{
    SC_CFUN_0 = 0,
    SC_CFUN_1 = 1,
    SC_CFUN_4 = 4,
    SC_CFUN_RESET = 0XFF,
};

enum SCsysMode
{
    SC_NO_SERVICE = 0,
    SC_GSM = 2,
    SC_LTE = 4,
    SC_MODE = 0XFF,
};

typedef struct{
	unsigned long long Rx_Bytes;
	unsigned long long Tx_Bytes;
	unsigned long TX_Packets;
    unsigned long RX_Packets;
	unsigned long Rx_Dropped_Packets;
    unsigned long Tx_Dropped_Packets;
}SCstatisticsData;

enum SCPsNwRegStatus
{
  SC_PS_NW_REG_STA_NOT_REGED = 0, 				/**< Not registered and not searching */
  SC_PS_NW_REG_STA_REG_HPLMN,     				/**< Registered on home PLMN */
  SC_PS_NW_REG_STA_TRYING,        				/**< Not registered, but cellular subsystem is searching for a PLMN to register to */
  SC_PS_NW_REG_STA_REG_DENIED,    				/**< Registration denied */
  SC_PS_NW_REG_STA_UNKNOWN,       				/**< Unknown */
  SC_PS_NW_REG_STA_REG_ROAMING,    				/**< Registered on visited PLMN */
  SC_PS_NW_REG_STA_SMS_ONLY_HOME,				/**< registered for "SMS only", home network (applicable only when <AcT> indicates E-UTRAN) */
  SC_PS_NW_REG_STA_SMS_ONLY_ROAMING,			/**< registered for "SMS only", roaming (applicable only when <AcT> indicates E-UTRAN) */
  SC_PS_NW_REG_STA_EMERGENCY_ONLY_NOT_USED,		/**< attached for emergency bearer services only (see NOTE 2) (not applicable) */
  SC_PS_NW_REG_STA_CSFB_NOT_PREFERRED_HOME,		/**<registered for "CSFB not preferred", home network (applicable only when <AcT> indicates E-UTRAN) */
  SC_PS_NW_REG_STA_CSFB_NOT_PREFERRED_ROAMING,	/**<registered for "CSFB not preferred", roaming (applicable only when <AcT> indicates E-UTRAN) */
  SC_PS_NW_REG_STA_REG_EMERGENCY,    			/**< attached for emergency bearer services only*/
  SC_PS_NW_REG_STA_REG_DENIED_IN_ROAMING,       /**< registeration denied in roaming, only used for SSG project by now*/
  SC_PS_NW_REG_STA_SYNC_DONE_IN_LTE_ROAMING,    /**< sync done in LTE roaming network, only used for SSG project by now*/
  SC_PS_NW_REG_STA_ECALL_INACTIVE,              /**< eCall only when camp on LTE for eCall over IMS */
  SC_PS_NUM_REGSTATUS                			/**< Number of status values defined */
};


typedef struct{
       int sec;   //seconds [0,59]
       int min;   //minutes [0,59]
       int hour;  //hour [0,23]
       int day;  //day of month [1,31]
       int mon;   //month of year [1,12]
       int year; // since 1970
       int timezone;//[-96,96]
       char sign; //+ or -
}scRealTime;

void sAPI_NetworkInit(void);
unsigned int sAPI_NetworkGetCsq(UINT8 *pCsq);
unsigned int sAPI_NetworkGetCreg(int* pReg);
unsigned int sAPI_NetworkGetCpsi(SCcpsiParm *pStr);
unsigned int sAPI_NetworkGetCgreg(int* pGreg);
unsigned int sAPI_NetworkNwStatus(int* pReg);
unsigned int sAPI_NetworkGetSysMode(int* pMode);
unsigned int sAPI_NetworkGetCnmp(int *pCnmp);
unsigned int sAPI_NetworkSetCnmp(int CnmpValue);
unsigned int sAPI_NetworkGetCnbp(UINT32 *pCnbpH,UINT32 *pCnbpL);
unsigned int sAPI_NetworkSetCnbp(uint64_t lteband);
unsigned int sAPI_NetworkGetPreferMode(int *pMode);
unsigned int sAPI_NetworkSetPreferMode(int mode);
unsigned int sAPI_NetworkTestCops(char* pCops);
unsigned int sAPI_NetworkGetCops(char* pCops);
unsigned int sAPI_NetworkSetCops(int modeVal,int formatVal,char *networkOperator,int accTchVal);
unsigned int sAPI_NetworkGetCgdcont(SCApnParm* pCgdcont);
unsigned int sAPI_NetworkSetCgdcont(int primCid,char *type,char *APNstr);
unsigned int sAPI_NetworkGetDNSAddr(int cid, SCDNSAddr *pDnsAddr);
unsigned int sAPI_NetworkGetCgact(SCAPNact* pCgact);
unsigned int sAPI_NetworkSetCgact(int pdpState,int cid);
unsigned int sAPI_NetworkGetCgatt(int* pCgatt);
unsigned int sAPI_NetworkSetCgatt(int CgattValue);
unsigned int sAPI_NetworkSetCfun(int CfunValue);
unsigned int sAPI_NetworkGetCfun(UINT8 *pCfun);
unsigned int sAPI_NetworkSetCtzu(int CtzuValue);
unsigned int sAPI_NetworkGetCtzu(int* CtzuValue);
unsigned int sAPI_NetworkGetRealTime(scRealTime *pRealTime);
unsigned int sAPI_NetworkGetCgpaddr(int Cid,SCcgpaddrParm *Pstr);
unsigned int sAPI_NetworkGetCnetci(SCcnetciParm *pStr);
unsigned int sAPI_NetworkGetCgauth(SCCGAUTHParm *pCgauth,int cid);
unsigned int sAPI_NetworkSetCgauth(SCCGAUTHParm *pCgauth,BOOL delflag);
int sAPI_GetStatisticsData(SCstatisticsData *pStatisticsData);
unsigned int sAPI_NetworkGetCnetcon(SCdialapnparm *pCnetcon);
unsigned int sAPI_NetworkSetCnetcon(int chanel,SCdialapnparm *pCnetcon);
unsigned int sAPI_NetworkGetSrvInfo(SCcpsiParm *psrv);
unsigned int sAPI_EnableOneBis(void);
unsigned int sAPI_EnableHardWarePSM(void);
unsigned int sAPI_PSMSetT3412Timer(UINT8 t3412);
unsigned int sAPI_PSMSetT3324Timer(UINT8 mode, char *t3324);
#endif
