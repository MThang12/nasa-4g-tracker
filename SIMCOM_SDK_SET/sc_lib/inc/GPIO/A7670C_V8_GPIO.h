#ifndef __A7670C_V8_GPIO_H__
#define __A7670C_V8_GPIO_H__

/* A7670C_V8 GPIO Resource Sheet  */
typedef enum {
    SC_MODULE_GPIO_NOT_ASSIGNED = -1,
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_00 = 4,   /*  pin4, pad name: RI  */
#endif
    SC_MODULE_GPIO_01 = 5,   /*  pin5, pad name: GPIO_01 */
    SC_MODULE_GPIO_02 = 9,   /*  pin6, pad name: GPIO_02 */
    SC_MODULE_GPIO_03 = 54,  /*  pin7, pad name: GPIO_03/UART_CTS */
    SC_MODULE_GPIO_04 = 53,  /*  pin8, pad name: GPIO_04/UART_RTS  */
    SC_MODULE_GPIO_05 = 87,  /*  pin19, pad name: GPIO_05  */
    SC_MODULE_GPIO_06 = 73,  /*  pin20, pad name: GPIO_06/MK_IN_3  */
    SC_MODULE_GPIO_07 = 74,  /*  pin21, pad name: GPIO_07/MK_OUT_3  */
    SC_MODULE_GPIO_08 = 88,  /*  pin48, pad name: GPIO_08  */
    SC_MODULE_GPIO_09 = 69,  /*  pin52, pad name: GPIO_09 /PWM1  */
    SC_MODULE_GPIO_10 = 70,  /*  pin66, pad name: GPIO_10 /PWM2  */

    SC_MODULE_GPIO_11 = 3,   /*  pin67, pad name: GPIO_11/MK_OUT_5  */
    SC_MODULE_GPIO_12 = 2,   /*  pin68, pad name: GPIO_12/MK_IN_5  */
    SC_MODULE_GPIO_13 = 1,   /*  pin35, pad name: GPIO_13/MK_OUT_6  */
    SC_MODULE_GPIO_14 = 0,   /*  pin36, pad name: GPIO_14/MK_IN_6  */
    SC_MODULE_GPIO_15 = 68,  /*  pin44, pad name: GPIO_15/MK_OUT_2  */
    SC_MODULE_GPIO_16 = 8,   /*  pin47, pad name: GPIO_16/MK_IN_2  */
    SC_MODULE_GPIO_17 = 37,  /*  pin26, pad name: GPIO_17  */
    SC_MODULE_GPIO_18 = 90,  /*  pin53, pad name: GPIO_18  */
    SC_MODULE_GPIO_19 = 89,  /*  pin101, pad name: GPIO_19/LCD_BL_PWM/PWM3  */

    SC_MODULE_GPIO_20 = 33,  /*  pin102,pad name: GPIO_20/LCD_SPI_CLK/SPI0_CLK  */
    SC_MODULE_GPIO_21 = 36,  /*  pin103,pad name: GPIO_21/LCD_SPI_TXD/SPI0_MOSI  */
    SC_MODULE_GPIO_22 = 35,  /*  pin104,pad name: GPIO_22/LCD_SPI_RXD/SPI0_MISO  */
    SC_MODULE_GPIO_23 = 34,  /*  pin105,pad name: GPIO_23/LCD_SPI_CS/SPI0_CS  */
    SC_MODULE_GPIO_24 = 55,  /*  pin106,pad name: GPIO_24/LCD_RST  */
    SC_MODULE_GPIO_25 = 85,  /*  pin107,pad name: GPIO_25/LCD_DCX  */

    SC_MODULE_GPIO_34 = 12,  /*  pin11, pad name: GPIO_34/SPI2_CLK  */
    SC_MODULE_GPIO_35 = 13,  /*  pin12, pad name: GPIO_35/SPI2_CS  */
    SC_MODULE_GPIO_36 = 15,  /*  pin13, pad name: GPIO_36/SPI2_MOSI  */
    SC_MODULE_GPIO_37 = 14,  /*  pin14, pad name: GPIO_37/SPI2_MISO  */

    SC_MODULE_GPIO_38 = 50,  /*  pin37, pad name: GPIO_38/CI2C_SDA  */
    SC_MODULE_GPIO_39 = 49,  /*  pin38,pad name:  GPIO_39/CI2C_SCL  */
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_40 = 6,  /*  pin3, pad name: DTR  */
#endif

    SC_MODULE_GPIO_MAX = 128,
} SC_Module_GPIONumbers;

#endif