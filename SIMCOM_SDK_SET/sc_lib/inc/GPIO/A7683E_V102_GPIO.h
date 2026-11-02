#ifndef __A7683E_V102_GPIO_H__
#define __A7683E_V102_GPIO_H__

typedef enum
{
    SC_MODULE_GPIO_00 = 73,  /*  pin3, pad name: GPIO_00/UART4_CTS  */
    SC_MODULE_GPIO_01 = 74,  /*  pin4, pad name: GPIO_01/UART4_RTS  */
    SC_MODULE_GPIO_02 = 124,  /*  pin5, pad name: GPIO_02  */
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_03 = 10,  /*  pin7, pad name: GPIO_03/RI */
#endif
    SC_MODULE_GPIO_04 = 121,  /*  pin41, pad name: GPIO_04  */
    SC_MODULE_GPIO_05 = 6,  /*  pin42, pad name: GPIO_05/PWM3  */
    SC_MODULE_GPIO_06 = 51,   /*  pin46, pad name: GPIO_06/MMC_DAT3/CP_UART_RXD  */
    SC_MODULE_GPIO_07 = 52,   /*  pin47, pad name: GPIO_07/MMC_DAT2/CP_UART_TXD  */
    SC_MODULE_GPIO_08 = 53,  /*  pin48, pad name: GPIO_08/MMC_DAT1/CP_UART_RTS  */
    SC_MODULE_GPIO_09 = 54,  /*  pin49, pad name: GPIO_09/MMC_DAT0/CP_UART_CTS  */
    SC_MODULE_GPIO_10 = 27,  /*  pin50, pad name: GPIO_10/ MMC_CLK  */

    SC_MODULE_GPIO_11 = 31,  /*  pin51, pad name: GPIO_11/MMC_CMD  */
    SC_MODULE_GPIO_12 = 69,  /*  pin57, pad name: GPIO_12  */
    SC_MODULE_GPIO_13 = 70,  /*  pin58, pad name: GPIO_13  */
    SC_MODULE_GPIO_14 = 50,  /*  pin64, pad name: GPIO_14/CI2C_SDA  */
    SC_MODULE_GPIO_15 = 49,  /*  pin65, pad name: GPIO_15/CI2C_SCL  */
    SC_MODULE_GPIO_16 = 4,  /*  pin78 , pad name: GPIO_16/SPI_SCLK/PCM_CLK/GRFC1  */
    SC_MODULE_GPIO_17 = 5, /*  pin79 , pad name: GPIO_17/SPI_CS/PCM_SYNC/GRFC2  */
    SC_MODULE_GPIO_18 = 19,  /*  pin80 , pad name: GPIO_18/SPI_MOSI/PCM_DIN  */
    SC_MODULE_GPIO_19 = 18,  /*  pin81 , pad name: GPIO_19/SPI_MISO/PCM_DOUT  */
    SC_MODULE_GPIO_20 = 9,  /*  pin82 , pad name: GPIO_20  */
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_21 = 00,  /*  pin 6, pad name: DTR  */
#endif

    SC_MODULE_GPIO_MAX = 128,
} SC_Module_GPIONumbers;

#endif

