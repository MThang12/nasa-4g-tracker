#ifndef __A7676E_GPIO_H__
#define __A7676E_GPIO_H__

/* A7676E GPIO Resource Sheet  */
typedef enum
{
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_00 = 31,  /*  pin4, pad name: RI  */
#endif
    SC_MODULE_GPIO_01 = 32,  /*  pin5, pad name: DCD  */
    SC_MODULE_GPIO_02 = 54,  /*  pin7, pad name: CTS  */
    SC_MODULE_GPIO_03 = 53,  /*  pin8, pad name: RTS  */
    SC_MODULE_GPIO_04 = 120,  /*  pin19, pad name: GPIO_01 */
    SC_MODULE_GPIO_05 = 6,  /*  pin20, pad name: MK_IN_3 */
    SC_MODULE_GPIO_06 = 7,  /*  pin21, pad name: MK_OUT_3 */
    SC_MODULE_GPIO_07 = 124,  /*  pin26, pad name: GPIO_02 */
    SC_MODULE_GPIO_08 = 13,  /*  pin35, pad name: MK_OUT_6 */
    SC_MODULE_GPIO_09 = 12,  /*  pin36, pad name: MK_IN_6 */
    SC_MODULE_GPIO_10 = 5,  /*  pin44, pad name: MK_OUT_2 */
    SC_MODULE_GPIO_11 = 4,  /*  pin47, pad name: MK_IN_2 */
    SC_MODULE_GPIO_12 = 77,  /*  pin48, pad name: GPIO_03 */
    SC_MODULE_GPIO_13 = 0,  /*  pin49, pad name: GPIO_04 */
    SC_MODULE_GPIO_14 = 1,  /*  pin50, pad name: GPIO_05 */
    SC_MODULE_GPIO_15 = 28,  /*  pin52, pad name: Netlight */
    SC_MODULE_GPIO_16 = 8,  /*  pin56, pad name: Status */
    SC_MODULE_GPIO_17 = 11,  /*  pin67, pad name: MK_OUT_5 */
    SC_MODULE_GPIO_18 = 10,  /*  pin68, pad name: MK_IN_5 */

#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_19 = 27,  /*  pin3, pad name: DTR  */
#endif
    SC_MODULE_GPIO_MAX = 128,
} SC_Module_GPIONumbers;

#endif