/**
 * \simcom_system.h
 *
 * \brief system/sleep functions.
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

#ifndef _SIMCOM_SYSTEM_H_
#define _SIMCOM_SYSTEM_H_

 
#define CUS_VERSION       "NULL"

enum SCsystemSF
{
	SC_SYS_SUCCESS = 0,
	SC_SYS_FAIL
};


typedef struct
{
    char *manufacture_namestr;
    char *Module_modelstr;
    char *Revision;
    char *GCAP;
    char *cgmr;
    char *internal_verstr;
    char *version_tagstr;
    char *SDK_Version;
}SIMComVersion;

typedef struct
{
    char sdkversion[50];
}SDKVersion;

typedef struct
{
    char rfversion1[50];
    char rfversion2[50];
}RFVersion;

typedef struct
{
    char cusversion[50];
}CUSVersion;

typedef enum
{
    SC_SYSTEM_SLEEP_DISABLE     =   0,
    SC_SYSTEM_SLEEP_ENABLE      =   1
}SC_SYSTEM_SLEEP_FLAG;

typedef struct
{
  SC_SYSTEM_SLEEP_FLAG sleep_flag;
  unsigned char time;
}SC_SleepEx_str;

#ifdef CUS_TBDTU

char *sAPI_GetKernelVersion(void);

char *sAPI_GetManuVersion(void);

char *sAPI_GetProjtVersion(void);

char *sAPI_GetHwMajorVersion(void);

char *sAPI_GetHwChildVersion(void);

char *sAPI_GetHwCorrectionVersion(void);

#endif

void sAPI_SysGetVersion (SIMComVersion *simcominfo);
void sAPI_SysGetRFVersion(RFVersion *RFinfo);
void sAPI_SysGetCusVersion(CUSVersion *CUSinfo);
unsigned int sAPI_SysGetImei (char *ImeiValue);
void sAPI_GetSystemInfo(unsigned int *cpuUsedRate, unsigned int *heapFreeSize);

/*****************************************************************************
* FUNCTION
*  sAPI_GetCpuUid
*
* DESCRIPTION
* To get the unique id of CPU
*
* PARAMETERS
*  None
*
* RETURNS
*  unique id
*
* NOTE
*
*****************************************************************************/
unsigned long long sAPI_GetCpuUid(void);

/*****************************************************************************
* FUNCTION
*  sAPI_SystemSleepSet
*
* DESCRIPTION
*  Module will enter into sleep mode after setting flag to 1, module will exit
*from sleep mode after setting flag to 0
*  
* PARAMETERS
*  flag     the flag of the system sleep mode
*
* RETURNS
*  0 is OK.
*
* NOTE
*  
*****************************************************************************/
int sAPI_SystemSleepSet(SC_SYSTEM_SLEEP_FLAG flag);
/*****************************************************************************
* FUNCTION
*  sAPI_SystemSleepGet
*
* DESCRIPTION
*  Get the flag of the system sleep mode
*  
* PARAMETERS
*  None
*
* RETURNS
*  flag
*
* NOTE
*  
*****************************************************************************/
SC_SYSTEM_SLEEP_FLAG sAPI_SystemSleepGet(void);

int sAPI_SystemSleepExSet(SC_SYSTEM_SLEEP_FLAG flag, unsigned char time);

/*****************************************************************************
* FUNCTION
*  sAPI_SystemAlarmClock2Wakeup
*
* DESCRIPTION
* The module will enter sleep mode ,and exit sleep mode after time
*
* PARAMETERS
*  time(ms)     The amount of sleep
*
* RETURNS
*  0 is OK.
*
* NOTE
*
*****************************************************************************/
int sAPI_SystemAlarmClock2Wakeup(unsigned long time);
int sAPI_GetSocTemprature(void);

#endif
