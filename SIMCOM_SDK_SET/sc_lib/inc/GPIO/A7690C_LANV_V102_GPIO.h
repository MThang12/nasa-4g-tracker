#ifndef __A7690C_LANV_V102_GPIO_H__
#define __A7690C_LANV_V102_GPIO_H__

/* A7680C_LANS GPIO Resource Sheet  */
typedef enum
{
    SC_MODULE_GPIO_01 = 6,    /*  pin 16, pad name: GPIO_01/PWM3 */
    SC_MODULE_GPIO_02 = 124,  /*  pin 21, pad name: GPIO_02 */
    SC_MODULE_GPIO_03 = 54,   /*  pin 22, pad name: GPIO_03/MAIN_CTS */
    SC_MODULE_GPIO_04 = 53,   /*  pin 23, pad name: GPIO_04/MAIN_RTS */
    SC_MODULE_GPIO_05 = 70,   /*  pin 25, pad name: GPIO_05 */
    SC_MODULE_GPIO_06 = 16,   /*  pin 30, pad name: GPIO_06/PCM_CLK/SSP0_CLK */
    SC_MODULE_GPIO_07 = 17,   /*  pin 31, pad name: GPIO_07/PCM_SYNC/SSP0_FRM */
    SC_MODULE_GPIO_08 = 19,   /*  pin 32, pad name: GPIO_08/PCM_DIN/SSP0_RXD */
    SC_MODULE_GPIO_09 = 18,   /*  pin 33, pad name: GPIO_09/PCM_DOUT/SSP0_TXD */
    SC_MODULE_GPIO_10 = 50,   /*  pin 66, pad name: GPIO_10/I2C2_SDA */
    SC_MODULE_GPIO_11 = 49,   /*  pin 67, pad name: GPIO_11/I2C2_SCL */
    SC_MODULE_GPIO_12 = 12,   /*  pin 68, pad name: GPIO_12/I2C_SCL */
    SC_MODULE_GPIO_13 = 13,    /*  pin 69, pad name: GPIO_13/I2C_SDA */
    SC_MODULE_GPIO_14 = 69,    /*  pin 79, pad name: GPIO_14 */
    SC_MODULE_GPIO_15 = 9,     /*  pin 82, pad name: GPIO_15/PWM2/PWM4 */
    SC_MODULE_GPIO_16 = 4,     /*  pin 84, pad name: GPIO_16/MK_IN_2 */
    SC_MODULE_GPIO_17 = 1,     /*  pin 85, pad name: GPIO_17/MK_OUT_2 */
    SC_MODULE_GPIO_18 = 0,     /*  pin 86, pad name: GPIO_18/MK_IN_0 */
    SC_MODULE_GPIO_19 = 5,     /*  pin 87, pad name: GPIO_19/MK_OUT_0 */
    SC_MODULE_GPIO_20 = 2,     /*  pin 28, pad name: GPIO_20/UART4_RXD/MK_IN_1 */
    SC_MODULE_GPIO_21 = 3,     /*  pin 29, pad name: GPIO_21/UART4_TXD/MK_OUT_1 */
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_22 = 27,     /*  pin 19, pad name: MAIN_DTR */
#endif
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_23 = 31,     /*  pin 20, pad name: MAIN_RI */
#endif

    SC_MODULE_GPIO_MAX = 128,
} SC_Module_GPIONumbers;

#endif
