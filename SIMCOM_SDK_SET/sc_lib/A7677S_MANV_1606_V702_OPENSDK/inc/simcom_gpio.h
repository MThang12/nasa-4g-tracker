#ifndef __SIMCOM_GPIO_H__
#define __SIMCOM_GPIO_H__

#include "stdio.h"
#include <stdint.h>


typedef void (*GPIOCallback)(void);
typedef unsigned long UINT32;

typedef enum
{
	SC_GPIORC_FALSE = 0,
	SC_GPIORC_TRUE = 1,
	SC_GPIORC_LOW = 0,
	SC_GPIORC_HIGH = 1,

	SC_GPIORC_OK = 0,
    SC_GPIORC_INVALID_PORT_HANDLE = -100,
    SC_GPIORC_NOT_OUTPUT_PORT,
    SC_GPIORC_NO_TIMER,
    SC_GPIORC_NO_FREE_HANDLE,
    SC_GPIORC_AMOUNT_OUT_OF_RANGE,
    SC_GPIORC_INCORRECT_PORT_SIZE,
    SC_GPIORC_PORT_NOT_ON_ONE_REG,
    SC_GPIORC_INVALID_PIN_NUM,
    SC_GPIORC_PIN_USED_IN_PORT,
    SC_GPIORC_PIN_NOT_FREE,
    SC_GPIORC_PIN_NOT_LOCKED,
    SC_GPIORC_NULL_POINTER,
    SC_GPIORC_PULLED_AND_OUTPUT,
	SC_GPIORC_INCORRECT_PORT_TYPE,
	SC_GPIORC_INCORRECT_DEBOUNCE,
    SC_GPIORC_INCORRECT_TRANSITION_TYPE,
	SC_GPIORC_INCORRECT_DIRECTION,
	SC_GPIORC_INCORRECT_PULL,
	SC_GPIORC_INCORRECT_INIT_VALUE,
	SC_GPIORC_WRITE_TO_INPUT,
	SC_GPIORC_INVALID_MULTI_FUNCTION,
	SC_GPIORC_INVALID_PARAMETER
}SC_GPIOReturnCode;

typedef enum
{
    SC_GPIO_IN_PIN = 0,
    SC_GPIO_OUT_PIN = 1
}SC_GPIOPinDirection;

typedef enum
{
    SC_GPIO_PULL_DISABLE = 0,
    SC_GPIO_PULLUP_ENABLE,
    SC_GPIO_PULLDN_ENABLE
}SC_GPIOPullUpDown;

typedef enum
{
    SC_GPIO_NO_EDGE = 0,
    SC_GPIO_RISE_EDGE,
    SC_GPIO_FALL_EDGE,
    SC_GPIO_TWO_EDGE,
}SC_GPIOTransitionType;


typedef struct
{
	SC_GPIOPinDirection pinDir;
	UINT32			initLv;
	SC_GPIOPullUpDown 	pinPull;
	SC_GPIOTransitionType pinEd;
	GPIOCallback isr;
	GPIOCallback wu;
} SC_GPIOConfiguration;

typedef enum
{
    SC_SIMDET_FUNCTION_DISABLE = 0,
    SC_SIMDET_FUNCTION_ENABLE,
}SC_SimDetPinConfig;

/*****************************************************************************
 * FUNCTION
 *  sAPI_GpioSetValue
 *
 * DESCRIPTION
 *  set a specified gpio's level
 *
 * PARAMETERS
 *
 *
 * RETURNS
 *  @SC_GPIOReturnCode
 *
 * NOTE:
 *      please confirm that specified gpio is output mode, before using this API.
 *
 *****************************************************************************/
SC_GPIOReturnCode sAPI_GpioSetValue(unsigned int gpio,unsigned int value);

/*****************************************************************************
 * FUNCTION
 *  sAPI_GpioGetValue
 *
 * DESCRIPTION
 *  get a specified gpio's level
 *
 * PARAMETERS
 *
 *
 * RETURNS
 *  @SC_GPIOReturnCode
 *
 * NOTE:
 *      please confirm that specified gpio is input mode, before using this API.
 *
 *****************************************************************************/
SC_GPIOReturnCode sAPI_GpioGetValue(unsigned int gpio);


/*****************************************************************************
 * FUNCTION
 *  sAPI_GpioConfig
 *
 * DESCRIPTION
 *  reconfigure all gpio's contribution.
 *
 * PARAMETERS
 *  a array constructed by SC_GPIOConfiguration struct.
 *
 * RETURNS
 *  @SC_GPIOReturnCode
 *
 * NOTE:
 *      GPIO will auto initial by the table when module start, you can use this API to configure GPIO again.
 *
 *****************************************************************************/
 SC_GPIOReturnCode sAPI_GpioConfig(unsigned int gpio, SC_GPIOConfiguration GpioConfig);
 SC_GPIOReturnCode sAPI_GpioConfigInterrupt(unsigned int gpio,SC_GPIOTransitionType type,GPIOCallback handler);
 SC_GPIOReturnCode sAPI_PinmuxFunc(unsigned int gpio, unsigned int mf);
 SC_GPIOReturnCode sAPI_GpioSetDirection(unsigned int gpio, unsigned int direction);
 SC_GPIOReturnCode sAPI_GpioGetDirection(unsigned int gpio);
 SC_GPIOReturnCode sAPI_GpioWakeupEnable(unsigned int gpio, SC_GPIOTransitionType type);
 SC_GPIOReturnCode sAPI_SimDetectPinConfig(SC_SimDetPinConfig isSimDet);
#ifdef CUS_TOPFLY
typedef void (* CusGPIOCallback) (uint32_t gpio_num, uint32_t state);
typedef void (* CusGPIOWakeupCallback) (uint32_t gpio_num, uint32_t state);
 SC_GPIOReturnCode sAPI_GpioSetInterrupt(unsigned int gpio,SC_GPIOPullUpDown pull_config,SC_GPIOTransitionType type,unsigned int isIsr);
SC_GPIOReturnCode sAPI_CusGpioConfigInterrupt(CusGPIOCallback isrHandler,CusGPIOWakeupCallback wakeupHandler);
#endif
typedef void (*DTRPullUpCallback)(void);
typedef void (*DTRPullDownCallback)(void);
SC_GPIOReturnCode sAPI_DTRConfigInterrupt(DTRPullUpCallback handler1,DTRPullDownCallback handler2);
#endif


