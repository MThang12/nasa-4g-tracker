/**
 * \simcom_rtc.h
 *
 * \brief rtc functions.
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
 */

#ifndef _SIMCOM_RTC_H_
#define _SIMCOM_RTC_H_

typedef signed short INT16;
typedef unsigned char BOOL;
typedef void (*AlCallback)(void);

typedef struct rtc_time {
  int tm_sec;	//seconds [0,59]
  int tm_min;	//minutes [0,59]
  int tm_hour;  //hour [0,23]
  int tm_mday;  //day of month [1,31]
  int tm_mon;   //month of year [1,12]
  int tm_year; // since 1970
  int tm_wday; // sunday = 0
}t_rtc;


INT16 sAPI_SetRealTimeClock(t_rtc *RtcTime);
INT16 sAPI_GetRealTimeClock(t_rtc *RtcTime);
void sAPI_RtcSetAlarm(t_rtc *tmp);
void sAPI_RtcGetAlarm(t_rtc *tmp);
void sAPI_RtcEnableAlarm(BOOL onoff);
void sAPI_RtcRegisterCB(AlCallback callback);
int sAPI_localtime(int time, t_rtc *tm);
INT16 sAPI_GetUtcTime(t_rtc *UtcTime);

#endif
