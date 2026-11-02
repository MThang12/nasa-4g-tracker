#ifndef __A7680C_MNNV_V801_GPIO_H__
#define __A7680C_MNNV_V801_GPIO_H__

/* A7680C_MNNV_LNNV GPIO Resource Sheet  */
typedef enum
{
    SC_MODULE_GPIO_00 = 53,  /*  pin 3, pad name: GPIO_00/MAIN_RTS */
    SC_MODULE_GPIO_01 = 54,  /*  pin 4, pad name: GPIO_01/MAIN_CTS */
    SC_MODULE_GPIO_02 = 5,   /*  pin 5, pad name: GPIO_02 */
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_03 = 6,   /*  pin 6, pad name: DTR  */
#endif
    SC_MODULE_GPIO_04 = 4,   /*  pin 7, pad name: GPIO_04 */

    SC_MODULE_GPIO_05 = 9,   /*  pin 20, pad name: GPIO_05 */
    SC_MODULE_GPIO_06 = 11,  /*  pin 22, pad name: GPIO_06/UART3_TX */
    SC_MODULE_GPIO_07 = 10,  /*  pin 23, pad name: GPIO_07/UART3_RX */
    SC_MODULE_GPIO_08 = 37,  /*  pin 41, pad name: GPIO_08 */
    SC_MODULE_GPIO_09 = 69,  /*  pin 42, pad name: GPIO_09/PWM1 */

    SC_MODULE_GPIO_10 = 2,   /*  pin 43, pad name: GPIO_10 */
    SC_MODULE_GPIO_11 = 3,   /*  pin 44, pad name: GPIO_11 */
    SC_MODULE_GPIO_12 = 86,  /*  pin 57, pad name: GPIO_12 */
    SC_MODULE_GPIO_13 = 13,  /*  pin 62, pad name: GPIO_13/SPI2_CS */
    SC_MODULE_GPIO_14 = 12,  /*  pin 63, pad name: GPIO_14/SPI2_CLK */
    SC_MODULE_GPIO_15 = 14,  /*  pin 64, pad name: GPIO_15/SPI2_MISO */

    SC_MODULE_GPIO_16 = 15,  /*  pin 65, pad name: GPIO_16/SPI2_MOSI */
    SC_MODULE_GPIO_17 = 50,  /*  pin 68, pad name: GPIO_17/CI2C_SDA */
    SC_MODULE_GPIO_18 = 49,  /*  pin 69, pad name: GPIO_18/CI2C_SCL */
    SC_MODULE_GPIO_25 = 33,  /*  pin 80, pad name: GPIO_25/LCD_SPI_CLK */
    SC_MODULE_GPIO_26 = 34,  /*  pin 81, pad name: GPIO_26/LCD_SPI_CS */

    SC_MODULE_GPIO_27 = 55,  /*  pin 82, pad name: GPIO_27/LCD_RST */
    SC_MODULE_GPIO_28 = 35,  /*  pin 83, pad name: GPIO_28/LCD_SPI_RXD */

    SC_MODULE_GPIO_29 = 36,  /*  pin 84, pad name: GPIO_29/LCD_SPI_TXD */
    SC_MODULE_GPIO_30 = 70,  /*  pin 86, pad name: GPIO_30/PWM2 */


    SC_MODULE_GPIO_MAX = 128,
} SC_Module_GPIONumbers;

#endif