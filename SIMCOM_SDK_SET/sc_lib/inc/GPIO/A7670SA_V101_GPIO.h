#ifndef __A7670SA_V101_GPIO_H__
#define __A7670SA_V101_GPIO_H__

/* A7670SA_V101 GPIO Resource Sheet  */
typedef enum
{
    SC_MODULE_GPIO_00 = 86,  /*  pin4, pad name: GPIO_00  */
    SC_MODULE_GPIO_01 = 89,  /*  pin5, pad name: GPIO_01   */
    SC_MODULE_GPIO_02 = 9,   /*  pin6, pad name: GPIO_02   */
    SC_MODULE_GPIO_03 = 54,  /*  pin7, pad name: GPIO_03   */
    SC_MODULE_GPIO_04 = 53,  /*  pin8, pad name: GPIO_04   */
    SC_MODULE_GPIO_05 = 87, /*  pin19, pad name: GPIO_05  */
    SC_MODULE_GPIO_06 = 14,   /*  pin20, pad name: GPIO_06  */
    SC_MODULE_GPIO_07 = 15,   /*  pin21, pad name: GPIO_07  */
    SC_MODULE_GPIO_08 = 88,  /*  pin48, pad name: GPIO_08  */
    SC_MODULE_GPIO_09 = 69,  /*  pin52, pad name: GPIO_09  */
    SC_MODULE_GPIO_10 = 70,  /*  pin66, pad name: GPIO_10  */
    SC_MODULE_GPIO_11 = 5,  /*  pin67, pad name: GPIO_11  */
    SC_MODULE_GPIO_12 = 4,  /*  pin68, pad name: GPIO_12  */
    SC_MODULE_GPIO_13 = 1,  /*  pin35, pad name: GPIO_13  */
    SC_MODULE_GPIO_14 = 0,  /*  pin36, pad name: GPIO_14  */
    SC_MODULE_GPIO_15 = 55,   /*  pin44, pad name: GPIO_15  */
    SC_MODULE_GPIO_16 = 8,   /*  pin47, pad name: GPIO_16  */
    SC_MODULE_GPIO_17 = 37, /*  pin26, pad name: GPIO_17  */
    SC_MODULE_GPIO_18 = 90, /*  pin53, pad name: GPIO_18  */
    SC_MODULE_GPIO_19 = 33, /*  pin11, pad name: GPIO_34 */
    SC_MODULE_GPIO_20 = 34,  /*  pin12, pad name: GPIO_35 */
    SC_MODULE_GPIO_21 = 36,  /*  pin13, pad name: GPIO_36 */
    SC_MODULE_GPIO_22 = 35,  /*  pin14, pad name: GPIO_37 */
    SC_MODULE_GPIO_23 = 50,  /*  pin37, pad name: GPIO_38 */
    SC_MODULE_GPIO_24 = 49,  /*  pin38, pad name: GPIO_39 */
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_25 = 74,  /*  pin3, pad name: DTR */
#endif
    SC_MODULE_GPIO_MAX = 128,
} SC_Module_GPIONumbers;

#endif


