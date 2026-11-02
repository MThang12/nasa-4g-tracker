#ifndef __SC_POWER_H__
#define __SC_POWER_H__

#include <stdint.h>

typedef enum {
    SC_ONKEY_PRESSED,
    SC_ONKEY_RELEASED
} POWER_KEY_STATUS;

typedef enum
{
    SC_RESET_PRESSED,
    SC_RESET_RELEASED
} RESET_STATUS;

typedef enum {
    POWER_UP_RESET_KEY = 1,
    POWER_UP_POWER_KEY,
    POWER_UP_SOFTWARE_RESET,
    POWER_UP_RTC_ALARM,
    POWER_UP_ERR,
} POWER_UP_REASON;

typedef enum {
    POWER_DOWN_SW_PDOWN_LONGPRESS_POWER_KEY = 1,
    POWER_DOWN_VRTC_DOWN_VRTC_MIN_TH,
    POWER_DOWN_OVERTEMPERATURE,
    POWER_DOWN_VIN_LDO_DOWN_UV_VSYS1_DETECT,
    POWER_DOWN_PMIC_WATCH_DOG_EXPIRY_EVENT,
    POWER_DOWN_LONGPRESS_RESET_KEY,
    POWER_DOWN_VIN_LDO_UP_VSYS_OVER_TH,
    POWER_DOWN_ERR,

} POWER_DOWN_REASON;

typedef void (*PowerKeyIntCallback)(void);
typedef void (*LongPressPowerKeyIntCallback)(void);
/*begin modified by wenjun.cai for XC  jira-A76801606-2051   20221026*/
/*****************************************************************************
 * FUNCTION
 *  sAPI_SetLdo8
 *
 * DESCRIPTION
 *  Set the switch and value of LDO8.
 *
 * PARAMETERS
 *  [in]onoff: Set the switch, 1:open LDO8; 0: close LDO8.
 *  [in]voltage: Voltage range that can be set:1200mV, 1250mV, 1600mV, 1700mV, 1800mV, 1900mV,
 *               2000mV, 2500mV, 2600mV, 2700mV, 2800mV, 2850mV, 2900mV, 3000mV, 3100mV, 3300mV
 *
 * RETURNS
 *  0: set done        -1: fail
 *
 * NOTE
 *  LDO8 must be turned on before setting the value of LDO8.
 *****************************************************************************/
int sAPI_SetLdo8(unsigned int onoff, unsigned int voltage);
/*end modified by wenjun.cai for XC  jira-A76801606-2051   20221026*/

/*****************************************************************************
 * FUNCTION
 *  sAPI_SetLdo7
 *
 * DESCRIPTION
 *  Set the switch and value of LDO7.
 *
 * PARAMETERS
 *  [in]onoff: Set the switch, 1:open LDO7; 0: close LDO7.
 *  [in]voltage: Voltage range that can be set:1200mV, 1250mV, 1600mV, 1700mV, 1800mV, 1900mV,
 *               2000mV, 2500mV, 2600mV, 2700mV, 2800mV, 2850mV, 2900mV, 3000mV, 3100mV, 3300mV
 *
 * RETURNS
 *  0: set done        -1: fail
 *
 * NOTE
 *  LDO7 must be turned on before setting the value of LDO7.
 *****************************************************************************/
int sAPI_SetLdo7(unsigned int onoff, unsigned int voltage);
/*****************************************************************************
 * FUNCTION
 *  sAPI_SetLongPressPowerKeyOffDisable
 *
 * DESCRIPTION
 * Set whether to disable the long press power button shutdown function.
 *
 * PARAMETERS
 *  [in]onoff: Set the switch
 * 1: Set to disable the powerkey button shutdown function.
 * 0: Set the powerkey button to shutdown the function.
 * The default onoff is 0.
 *
 * RETURNS
 *  0: set done        -1: fail
 *****************************************************************************/
int sAPI_SetLongPressPowerKeyOffDisable(unsigned int onoff);
unsigned int sAPI_ReadAdc(int channel);
unsigned int sAPI_ReadVbat(void);
void sAPI_SysPowerOff(void);
void sAPI_SysReset(void);
int sAPI_SetVddAux(unsigned int voltage);
POWER_UP_REASON sAPI_GetPowerUpEvent(void);
POWER_DOWN_REASON sAPI_GetPowerDownEvent(void);
POWER_KEY_STATUS sAPI_GetPowerKeyStatus(void);
uint8_t sAPI_SetPowerKeyOffFunc(uint8_t onoff, uint32_t powerKeyOffTime); //powerKeyOffTime: unit about 100ms
void sAPI_PowerKeyRegisterCallback(PowerKeyIntCallback cb);
void sAPI_LongPressPowerKeyRegisterCallback(LongPressPowerKeyIntCallback cb);
void sAPI_LDO7OnOff(char onoff);
void sAPI_SysGetBuildTime(char* SysBuildTime);
RESET_STATUS sAPI_GetResetKeyStatus(void);

#endif