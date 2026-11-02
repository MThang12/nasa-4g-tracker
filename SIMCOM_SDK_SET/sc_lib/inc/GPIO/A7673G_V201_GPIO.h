#ifndef __A7673G_V201_GPIO_H__
#define __A7673G_V201_GPIO_H__

/* A7673G_V201 GPIO Resource Sheet  */
typedef enum
{
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_00 = 25,  /*  pin4, pad name: GPIO_00  */
#endif
    SC_MODULE_GPIO_01 = 32,  /*  pin5, pad name: GPIO_01   */
    SC_MODULE_GPIO_02 = 9,   /*  pin6, pad name: GPIO_02   */
    SC_MODULE_GPIO_03 = 74,  /*  pin7, pad name: GPIO_03   */
    SC_MODULE_GPIO_04 = 73,  /*  pin8, pad name: GPIO_04   */
    SC_MODULE_GPIO_05 = 120, /*  pin19, pad name: GPIO_05  */
    SC_MODULE_GPIO_06 = 6,   /*  pin20, pad name: GPIO_06  */
    SC_MODULE_GPIO_07 = 7,   /*  pin21, pad name: GPIO_07  */
    SC_MODULE_GPIO_08 = 21,  /*  pin48, pad name: GPIO_08  */
    SC_MODULE_GPIO_09 = 28,  /*  pin52, pad name: GPIO_09  */
    SC_MODULE_GPIO_10 = 37,  /*  pin66, pad name: GPIO_10  */
    SC_MODULE_GPIO_11 = 11,  /*  pin67, pad name: GPIO_11  */
    SC_MODULE_GPIO_12 = 10,  /*  pin68, pad name: GPIO_12  */
    SC_MODULE_GPIO_13 = 13,  /*  pin35, pad name: GPIO_13  */
    SC_MODULE_GPIO_14 = 12,  /*  pin36, pad name: GPIO_14  */
    SC_MODULE_GPIO_15 = 1,   /*  pin44, pad name: GPIO_15  */
    SC_MODULE_GPIO_16 = 0,   /*  pin47, pad name: GPIO_16  */
    SC_MODULE_GPIO_17 = 124, /*  pin26, pad name: GPIO_17  */
    SC_MODULE_GPIO_18 = 118, /*  pin53, pad name: GPIO_18  */
    SC_MODULE_GPIO_19 = 126, /*  pin101, pad name: GPIO_19 */
    SC_MODULE_GPIO_20 = 33,  /*  pin102, pad name: GPIO_20 */
    SC_MODULE_GPIO_21 = 36,  /*  pin103, pad name: GPIO_21 */
    SC_MODULE_GPIO_22 = 35,  /*  pin104, pad name: GPIO_22 */
    SC_MODULE_GPIO_23 = 34,  /*  pin105, pad name: GPIO_23 */
    SC_MODULE_GPIO_24 = 78,  /*  pin106, pad name: GPIO_24 */
    SC_MODULE_GPIO_25 = 121, /*  pin107, pad name: GPIO_25 */
    SC_MODULE_GPIO_26 = 16,  /*  pin11, pad name: GPIO_26  */
    SC_MODULE_GPIO_27 = 17,  /*  pin12, pad name: GPIO_27  */
    SC_MODULE_GPIO_28 = 19,  /*  pin13, pad name: GPIO_28  */
    SC_MODULE_GPIO_29 = 18,  /*  pin14, pad name: GPIO_29  */
    SC_MODULE_GPIO_30 = 50,  /*  pin37, pad name: GPIO_30  */
    SC_MODULE_GPIO_31 = 49, /*  pin38, pad name: GPIO_31 */
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_32 = 26, /*  pin3, pad name: GPIO_32 */
#endif
    SC_MODULE_GPIO_MAX = 128,
} SC_Module_GPIONumbers;

#endif

