#ifndef SIMCOM_NTP_CLIENT_H_
#define SIMCOM_NTP_CLIENT_H_

#include "simcom_os.h"



typedef enum {
    SC_NTP_OP_SET,
    SC_NTP_OP_GET,
    SC_NTP_OP_EXC,
}SCntpOperationType;

typedef struct SCsysTime_s {
  int tm_sec;	//seconds [0,59]
  int tm_min;	//minutes [0,59]
  int tm_hour;  //hour [0,23]
  int tm_mday;  //day of month [1,31]
  int tm_mon;   //month of year [1,12]
  int tm_year; // since 1970
  int tm_wday; // sunday = 0
}SCsysTime_t;

typedef enum {
    SC_NTP_OK = 0,
    SC_NTP_ERROR,
    SC_NTP_ERROR_INVALID_PARAM,
    SC_NTP_ERROR_TIME_CALCULATED,
    SC_NTP_ERROR_NETWORK_FAIL,
    SC_NTP_ERROR_TIME_ZONE,
    SC_NTP_ERROR_TIME_OUT,
    SC_NTP_END
}SCntpResultType;

typedef SCntpResultType SCntpReturnCode;



/*****************************************************************************
* FUNCTION
*  sAPI_NtpUpdate
*
* DESCRIPTION
*  sAPI_NtpUpdate is used to updata system time with http server . 
* PARAMETERS
*   commad_type:
*       SC_NTP_OP_SET, set host addr
*       SC_NTP_OP_GET, get current host addr, long enough host_addr para is needed
*       SC_NTP_OP_EXC, update sys time by ntp protocol
*   host_addr: NTP server addr
*   time_zone: the local time zone, (-47 to 48)default is 32
*   magQ_urc: the msg queue for urc
* RETURNS
*  SCntpReturnCode  
*
* NOTE
*  Ignoring part of parameters by set NULL or 0.
*****************************************************************************/

SCntpReturnCode sAPI_NtpUpdate(SCntpOperationType commad_type, char* host_addr, int time_zone, sMsgQRef magQ_urc);

/*****************************************************************************
* FUNCTION
*  sAPI_GetSysLocalTime
*
* DESCRIPTION
*  sAPI_GetSysLocalTime is used to get system local time
* PARAMETERS
*   *currUtcTime
* RETURNS
*  NULL
*
* NOTE
*  you can call this function as following
*  tm_rtc currUtcTime;
*  sAPI_GetSysLocalTime(&currUtcTime);
*  currUtcTime is that we need,you can use currUtcTime.tm_year,currUtcTime.tm_mon....
*****************************************************************************/

void sAPI_GetSysLocalTime(SCsysTime_t *currUtcTime);

/*****************************************************************************
* FUNCTION
*  sAPI_SetSysLocalTime
*
* DESCRIPTION
*  sAPI_GetSysLocalTime is used to set system local time
* PARAMETERS
*   *timeStr
* RETURNS
*  NULL
*
* NOTE
*  String format is “yy/MM/dd,hh:mm:ss"
*  you can call this function as following
*  char *timeStr="14/01/01,02:14:36";
*  sAPI_GetSysLocalTime(timeStr);
*****************************************************************************/

void sAPI_SetSysLocalTime(char* timeStr);

/*****************************************************************************
* FUNCTION
*  sAPI_GetTxnNo
*
* DESCRIPTION
*  sAPI_GetTxnNo is used to Get the current timestamp
* PARAMETERS
*   void
* RETURNS
*  UINT64
*****************************************************************************/

UINT64 sAPI_GetTxnNo(void);

#endif

