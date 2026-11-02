/**
  ******************************************************************************
  * @file    demo_gpio.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of gpio operation.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "simcom_system.h"
#include "simcom_os.h"
#include "simcom_common.h"
#include "simcom_gpio.h"
#ifdef SIMCOM_A7630C_V702
#include "A7630C_LANS_GPIO.h" //A7630C_LANY(1606 Module)/A7630C_LANS(1606 Module)/A7630C_LANV(1606 Module)
#elif SIMCOM_A7670C_V6_02
#include "A7670C_FANS_GPIO.h" //A7670C_FANS(1606 Module)
#elif SIMCOM_A7680C_V5_01
#include "A7680C_LANS_GPIO.h" //A7680C_LANS(1606 Module)/A7680C_LANV(1606 Module)/A7680C_MANV(1606 Module)/A7680C_MANS(1606 Module)
#elif SIMCOM_A7680C_V506
#include "A7680C_LANS_GPIO.h" //A7680C_LANY(1606 Module)/A7680C_LANS(1606 Module)/A7680C_LANV(1606 Module)
#elif SIMCOM_A7676E
#include "A7676E_GPIO.h" //A7676E_LASE(1603 Module)A7676E_LBSE(1603 Module)
#elif SIMCOM_A7670C_V6_03
#include "A7670C_BANS_GPIO.h" //A7670C_FANV(1606 Module)/A7670C_FANS(1606 Module)/A7670C_BANS(1606 Module)/A7670C_BANV(1606 Module)
#elif SIMCOM_A7690C_V102
#include "A7690C_LANV_V102_GPIO.h" //A7690C_LANV(1606 Module)/A7690C_LANS(1606 Module)
#elif SIMCOM_A7680C_V801
#include "A7680C_V801_GPIO.h" //A7680C_MNNY(1602 Module)/A7680C_MNNV(1602 Module)/A7680C_LNNY(1602 Module)/A7680C_LNNV(1602 Module)/A7682S_LNXV(1602 Module)/A7682S_MNXV(1602 Module)
#elif SIMCOM_A7680C_V101
#include "A7680C_V801_GPIO.h" //A7680C_MNNV_V7(1602 Module)
#elif SIMCOM_A7670C_V801
#include "A7670C_V8_GPIO.h" //A7670C_MNNY(1602 Module)/A7670C_MNNV(1602 Module)/A7670C_LNNY(1602 Module)/A7670C_LNNV(1602 Module)/A7677S_LNXV(1602 Module)/A7677S_MNXV(1602 Module)
#elif SIMCOM_A7680C_V603
#include "A7680C_LANS_GPIO.h" //A7680C_MANV(1606 Module)/A7680C_MANS(1606 Module)
#elif SIMCOM_A7605C1_V201
#include "A7605C1_MAVX_V201_GPIO.h" //A7605C1_MAXV(1606 Module)
#elif SIMCOM_A7678_V102
#include "A7678_V102_GPIO.h" //A7670G(1603 Module)/A7670E(1603 Module)/A7672S(1603 Module)/A7672E(1603 Module)/A7670SA(1603 Module)/A7672SA(1603 Module)/A7670C-MASL(1603 Module)/A7670C-fASL(1603 Module)
#elif SIMCOM_A7683E_V102
#include "A7683E_V102_GPIO.h" //A7660E_LAXS(1606 Module)/A7663E_MAXS(1606 Module)/A7683E_LAXS(1606 Module)
#elif SIMCOM_A7673G_V201
#include "A7673G_V201_GPIO.h" //A7670U_MAMS(1606 Module)/A7670U_LAMS(1606 Module)/A7670NA_MAMS(1606 Module)/A7670E_MAMS(1606 Module)/A7670E_LAMS(1606 Module)/A7670G_MAMS(1606 Module)/A7670SA_MAMS(1606 Module)/A7670SA_LAMS(1606 Module)/A7670SA_MNMV(1606 Module)/A7673G_LAMS(1606 Module)/A7673G_MAMS(1606 Module)/A7673E_LAMS(1606 Module)/A7673E_MAMS(1606 Module)/A7673SA_LAMS(1606 Module)/A7673SA_MAMS(1606 Module)/A7670E_MAMV(1606 Module)/A7670SA_MANV(1606 Module)
#elif SIMCOM_A7670G_UB_V101
#include "A7670SA_V101_GPIO.h" //A7670SA_MNMV_V101(1602 Module)/A7670E_LNXY(1602 Module)
#elif SIMCOM_A7690C_V201
#include "A7690C_V201_GPIO.h" //A7690C-LNXV(1602 Module)
#else
#include "A7670C_MANS_LANS_GPIO.h"//A7670C_LANS(1606 Module)/A7670C_LANV(1606 Module)/A7670C_MANS(1606 Module)/A7670C_MANV(1606 Module)/A7677S_MANV(1606 Module)/A7677S_LANV(1606 Module)/A7677S_LANS(1606 Module)/A7677S_MANS(1606 Module)
#endif
#include "uart_api.h"





extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);
//#define GPIO_INT_WAKEUP_TEST

#ifdef SIMCOM_A7630C_V702
static int all_gpio[] =
{
    SC_MODULE_GPIO_00,
    SC_MODULE_GPIO_01,
    SC_MODULE_GPIO_02,
    SC_MODULE_GPIO_03,
    SC_MODULE_GPIO_04,

    SC_MODULE_GPIO_05,
    SC_MODULE_GPIO_06,
    SC_MODULE_GPIO_07,
    SC_MODULE_GPIO_08,
    SC_MODULE_GPIO_09,

    SC_MODULE_GPIO_10,
    SC_MODULE_GPIO_12,
    SC_MODULE_GPIO_13,
    SC_MODULE_GPIO_14,
    SC_MODULE_GPIO_15,

    SC_MODULE_GPIO_16,
    SC_MODULE_GPIO_17,
    SC_MODULE_GPIO_19,
    SC_MODULE_GPIO_20,
    SC_MODULE_GPIO_21,

    SC_MODULE_GPIO_22,
    SC_MODULE_GPIO_25,
    SC_MODULE_GPIO_26,
    SC_MODULE_GPIO_27,
    SC_MODULE_GPIO_28,

    SC_MODULE_GPIO_29,
    SC_MODULE_GPIO_30,
    SC_MODULE_GPIO_31,
    SC_MODULE_GPIO_32,
    SC_MODULE_GPIO_33,

    SC_MODULE_GPIO_34,
    SC_MODULE_GPIO_35,
    SC_MODULE_GPIO_36,
    SC_MODULE_GPIO_37,
#ifdef SIMCOM_A7630C_ST
    SC_MODULE_GPIO_38,
    SC_MODULE_GPIO_39,
#endif
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_40,
#endif
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_41,
#endif
    -1,
};
#elif SIMCOM_A7670C_V6_02
static int all_gpio[] =
{
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_00,
#endif
    SC_MODULE_GPIO_01,
    SC_MODULE_GPIO_02,
    SC_MODULE_GPIO_03,
    SC_MODULE_GPIO_04,
    SC_MODULE_GPIO_05,
    SC_MODULE_GPIO_07,
    SC_MODULE_GPIO_08,
    SC_MODULE_GPIO_09,
    SC_MODULE_GPIO_10,

    SC_MODULE_GPIO_11,
    SC_MODULE_GPIO_12,
    SC_MODULE_GPIO_13,
    SC_MODULE_GPIO_14,
    SC_MODULE_GPIO_15,
    SC_MODULE_GPIO_16,
    SC_MODULE_GPIO_17,
    SC_MODULE_GPIO_18,
    SC_MODULE_GPIO_19,
    SC_MODULE_GPIO_20,

    SC_MODULE_GPIO_21,
    SC_MODULE_GPIO_22,
    SC_MODULE_GPIO_23,
    SC_MODULE_GPIO_24,
    SC_MODULE_GPIO_25,
    SC_MODULE_GPIO_26,
    SC_MODULE_GPIO_27,
    SC_MODULE_GPIO_28,
    SC_MODULE_GPIO_29,
    SC_MODULE_GPIO_30,

    SC_MODULE_GPIO_31,
    SC_MODULE_GPIO_32,
    SC_MODULE_GPIO_33,
    SC_MODULE_GPIO_34,
    SC_MODULE_GPIO_35,
    SC_MODULE_GPIO_36,
    SC_MODULE_GPIO_37,
    SC_MODULE_GPIO_38,
    SC_MODULE_GPIO_39,
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_40 ,
#endif
    -1,
};
#elif SIMCOM_A7670C_V6_03
static int all_gpio[] =
{
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_00,
#endif
    SC_MODULE_GPIO_01,
    SC_MODULE_GPIO_02,
    SC_MODULE_GPIO_03,
    SC_MODULE_GPIO_04,
    SC_MODULE_GPIO_05,
    SC_MODULE_GPIO_07,
    SC_MODULE_GPIO_08,
    SC_MODULE_GPIO_09,
    SC_MODULE_GPIO_10,

    SC_MODULE_GPIO_11,
    SC_MODULE_GPIO_12,
    SC_MODULE_GPIO_13,
    SC_MODULE_GPIO_14,
    SC_MODULE_GPIO_15,
    SC_MODULE_GPIO_16,
    SC_MODULE_GPIO_17,
    SC_MODULE_GPIO_18,
    SC_MODULE_GPIO_19,
    SC_MODULE_GPIO_20,

    SC_MODULE_GPIO_21,
    SC_MODULE_GPIO_22,
    SC_MODULE_GPIO_23,
    SC_MODULE_GPIO_24,
    SC_MODULE_GPIO_25,
    SC_MODULE_GPIO_26,
    SC_MODULE_GPIO_27,
    SC_MODULE_GPIO_28,
    SC_MODULE_GPIO_29,
    SC_MODULE_GPIO_30,

    SC_MODULE_GPIO_31,
    SC_MODULE_GPIO_32,
    SC_MODULE_GPIO_33,
    SC_MODULE_GPIO_34,
    SC_MODULE_GPIO_35,
    SC_MODULE_GPIO_36,
    SC_MODULE_GPIO_37,
    SC_MODULE_GPIO_38,
    SC_MODULE_GPIO_39,
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_40,
#endif
    -1,
};
#elif SIMCOM_A7680C_V5_01
static int all_gpio[] =
{
    SC_MODULE_GPIO_00,
    SC_MODULE_GPIO_01,
    SC_MODULE_GPIO_02,
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_03,
#endif
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_04,
#endif
    SC_MODULE_GPIO_05,
    SC_MODULE_GPIO_06,
    SC_MODULE_GPIO_07,
    SC_MODULE_GPIO_08,
    SC_MODULE_GPIO_09,
    SC_MODULE_GPIO_10,
    SC_MODULE_GPIO_11,

    SC_MODULE_GPIO_12,
    SC_MODULE_GPIO_13,
    SC_MODULE_GPIO_14,
    SC_MODULE_GPIO_15,
    SC_MODULE_GPIO_16,
    SC_MODULE_GPIO_17,
    SC_MODULE_GPIO_18,
#ifdef CUS_LXT
    SC_MODULE_GPIO_19,
    SC_MODULE_GPIO_20,
    SC_MODULE_GPIO_21,
    SC_MODULE_GPIO_22,
#else
    SC_MODULE_GPIO_19,
    SC_MODULE_GPIO_20,
    SC_MODULE_GPIO_21,
    SC_MODULE_GPIO_22,
    SC_MODULE_GPIO_23,
    SC_MODULE_GPIO_24,
#endif
    -1,
};
#elif SIMCOM_A7680C_V506
static int all_gpio[] =
{
    SC_MODULE_GPIO_00,
    SC_MODULE_GPIO_01,
    SC_MODULE_GPIO_02,
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_03,
#endif
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_04,
#endif
    SC_MODULE_GPIO_05,
    SC_MODULE_GPIO_06,
    SC_MODULE_GPIO_07,
    SC_MODULE_GPIO_08,
    SC_MODULE_GPIO_09,
    SC_MODULE_GPIO_10,
    SC_MODULE_GPIO_11,

    SC_MODULE_GPIO_12,
    SC_MODULE_GPIO_13,
    SC_MODULE_GPIO_14,
    SC_MODULE_GPIO_15,
    SC_MODULE_GPIO_16,
    SC_MODULE_GPIO_17,
    SC_MODULE_GPIO_18,
#ifdef CUS_LXT
    SC_MODULE_GPIO_19,
    SC_MODULE_GPIO_20,
    SC_MODULE_GPIO_21,
    SC_MODULE_GPIO_22,
#else
    SC_MODULE_GPIO_19,
    SC_MODULE_GPIO_20,
    SC_MODULE_GPIO_21,
    SC_MODULE_GPIO_22,
    SC_MODULE_GPIO_23,
    SC_MODULE_GPIO_24,
#ifdef CUS_QGWL
    SC_MODULE_GPIO_25,
    SC_MODULE_GPIO_26,
#endif
#endif
    -1,
};
#elif SIMCOM_A7676E
static int all_gpio[] =
{
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_00,
#endif
    SC_MODULE_GPIO_01,
    SC_MODULE_GPIO_02,
    SC_MODULE_GPIO_03,
    SC_MODULE_GPIO_04,
    SC_MODULE_GPIO_05,
    SC_MODULE_GPIO_06,
    SC_MODULE_GPIO_07,
    SC_MODULE_GPIO_08,
    SC_MODULE_GPIO_09,
    SC_MODULE_GPIO_10,
    SC_MODULE_GPIO_11,
    SC_MODULE_GPIO_12,
    SC_MODULE_GPIO_13,
    SC_MODULE_GPIO_14,
    SC_MODULE_GPIO_15,
    SC_MODULE_GPIO_16,
    SC_MODULE_GPIO_17,
    SC_MODULE_GPIO_18,
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_19,
#endif
    -1,
};
#elif SIMCOM_A7690C_V102
static int all_gpio[] =
{
    SC_MODULE_GPIO_01,
    SC_MODULE_GPIO_02,
    SC_MODULE_GPIO_03,
    SC_MODULE_GPIO_04,
    SC_MODULE_GPIO_05,
    SC_MODULE_GPIO_06,
    SC_MODULE_GPIO_07,
    SC_MODULE_GPIO_08,
    SC_MODULE_GPIO_09,
    SC_MODULE_GPIO_10,

    SC_MODULE_GPIO_11,
    SC_MODULE_GPIO_12,
    SC_MODULE_GPIO_13,
    SC_MODULE_GPIO_14,
    SC_MODULE_GPIO_15,
    SC_MODULE_GPIO_16,
    SC_MODULE_GPIO_17,
    SC_MODULE_GPIO_18,
    SC_MODULE_GPIO_19,
    SC_MODULE_GPIO_20,

    SC_MODULE_GPIO_21,
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_22,
#endif
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_23,
#endif

    -1,
};
#elif SIMCOM_A7690C_V201
static int all_gpio[] =
{
    SC_MODULE_GPIO_00,
    SC_MODULE_GPIO_01,
    SC_MODULE_GPIO_02,
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_03,
#endif
    SC_MODULE_GPIO_04,
    SC_MODULE_GPIO_05,
    SC_MODULE_GPIO_06,
    SC_MODULE_GPIO_07,
    SC_MODULE_GPIO_09,
    SC_MODULE_GPIO_15,
    SC_MODULE_GPIO_16,
    SC_MODULE_GPIO_17,
    SC_MODULE_GPIO_18,
    SC_MODULE_GPIO_25,
    SC_MODULE_GPIO_26,
    SC_MODULE_GPIO_28,
    SC_MODULE_GPIO_29,
    SC_MODULE_GPIO_30,

    -1,
};
#elif SIMCOM_A7680C_V801
static int all_gpio[] =
{
    SC_MODULE_GPIO_00,
    SC_MODULE_GPIO_01,
    SC_MODULE_GPIO_02,
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_03,
#endif
    SC_MODULE_GPIO_04,

    SC_MODULE_GPIO_05,
    SC_MODULE_GPIO_08,
    SC_MODULE_GPIO_09,

    SC_MODULE_GPIO_10,
    SC_MODULE_GPIO_11,
    SC_MODULE_GPIO_12,
    SC_MODULE_GPIO_13,
    SC_MODULE_GPIO_14,
    SC_MODULE_GPIO_15,

    SC_MODULE_GPIO_16,
    SC_MODULE_GPIO_17,
    SC_MODULE_GPIO_18,
    SC_MODULE_GPIO_25,
    SC_MODULE_GPIO_26,

    SC_MODULE_GPIO_27,
    SC_MODULE_GPIO_28,

    SC_MODULE_GPIO_29,
    SC_MODULE_GPIO_30,

    -1,
};
#elif SIMCOM_A7680C_V101
static int all_gpio[] =
{
    SC_MODULE_GPIO_00,
    SC_MODULE_GPIO_01,
    SC_MODULE_GPIO_02,
    SC_MODULE_GPIO_04,

    SC_MODULE_GPIO_05,
    SC_MODULE_GPIO_08,
    SC_MODULE_GPIO_09,

    SC_MODULE_GPIO_10,
    SC_MODULE_GPIO_11,
    SC_MODULE_GPIO_12,
    SC_MODULE_GPIO_13,
    SC_MODULE_GPIO_14,
    SC_MODULE_GPIO_15,

    SC_MODULE_GPIO_16,
    SC_MODULE_GPIO_17,
    SC_MODULE_GPIO_18,
    SC_MODULE_GPIO_25,
    SC_MODULE_GPIO_26,

    SC_MODULE_GPIO_27,
    SC_MODULE_GPIO_28,

    SC_MODULE_GPIO_29,
    SC_MODULE_GPIO_30,

    -1,
};
#elif SIMCOM_A7670C_V801
static int all_gpio[] =
{
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_00,
#endif
    SC_MODULE_GPIO_01,
    SC_MODULE_GPIO_02,
    SC_MODULE_GPIO_03,
    SC_MODULE_GPIO_04,
    SC_MODULE_GPIO_05,
    SC_MODULE_GPIO_06,
    SC_MODULE_GPIO_07,
    SC_MODULE_GPIO_08,
    SC_MODULE_GPIO_09,

    SC_MODULE_GPIO_10,
    SC_MODULE_GPIO_11,
    SC_MODULE_GPIO_12,
    SC_MODULE_GPIO_13,
    SC_MODULE_GPIO_14,
    SC_MODULE_GPIO_15,
    SC_MODULE_GPIO_16,
    SC_MODULE_GPIO_17,
    SC_MODULE_GPIO_18,
    SC_MODULE_GPIO_19,

    SC_MODULE_GPIO_20,
    SC_MODULE_GPIO_21,
    SC_MODULE_GPIO_22,
    SC_MODULE_GPIO_23,
    SC_MODULE_GPIO_24,
    SC_MODULE_GPIO_25,

    SC_MODULE_GPIO_34,
    SC_MODULE_GPIO_35,
    SC_MODULE_GPIO_36,
    SC_MODULE_GPIO_37,
    SC_MODULE_GPIO_38,
    SC_MODULE_GPIO_39,
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_40,
#endif

    -1,
};
#elif SIMCOM_A7680C_V603
static int all_gpio[] =
{
    SC_MODULE_GPIO_00,
    SC_MODULE_GPIO_01,
    SC_MODULE_GPIO_02,
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_03,
#endif
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_04,
#endif
    SC_MODULE_GPIO_05,
    SC_MODULE_GPIO_06,
    SC_MODULE_GPIO_07,
    SC_MODULE_GPIO_08,
    SC_MODULE_GPIO_09,
    SC_MODULE_GPIO_10,
    SC_MODULE_GPIO_11,

    SC_MODULE_GPIO_12,
    SC_MODULE_GPIO_13,
    SC_MODULE_GPIO_14,
    SC_MODULE_GPIO_15,
    SC_MODULE_GPIO_16,
    SC_MODULE_GPIO_17,
    SC_MODULE_GPIO_18,
#ifdef CUS_LXT
    SC_MODULE_GPIO_19,
    SC_MODULE_GPIO_20,
    SC_MODULE_GPIO_21,
    SC_MODULE_GPIO_22,
#else
    SC_MODULE_GPIO_19,
    SC_MODULE_GPIO_20,
    SC_MODULE_GPIO_21,
    SC_MODULE_GPIO_22,
    SC_MODULE_GPIO_23,
    SC_MODULE_GPIO_24,
#endif
    -1,
};
#elif SIMCOM_A7605C1_V201
static int all_gpio[] =
{
    SC_MODULE_GPIO_01,
    SC_MODULE_GPIO_02,
    SC_MODULE_GPIO_03,
    SC_MODULE_GPIO_04,

    SC_MODULE_GPIO_05,
    SC_MODULE_GPIO_06,
    SC_MODULE_GPIO_07,
    SC_MODULE_GPIO_08,

    SC_MODULE_GPIO_09,
    SC_MODULE_GPIO_10,
    SC_MODULE_GPIO_11,
    SC_MODULE_GPIO_12,

    //SC_MODULE_GPIO_13,
    SC_MODULE_GPIO_14,
    SC_MODULE_GPIO_15,
    SC_MODULE_GPIO_16,

    SC_MODULE_GPIO_17,
    SC_MODULE_GPIO_18,
    SC_MODULE_GPIO_19,
    SC_MODULE_GPIO_20,

    SC_MODULE_GPIO_21,
    SC_MODULE_GPIO_22,
    SC_MODULE_GPIO_23,
    SC_MODULE_GPIO_24,

    SC_MODULE_GPIO_25,
    SC_MODULE_GPIO_26,
    SC_MODULE_GPIO_27,
    SC_MODULE_GPIO_28,

    SC_MODULE_GPIO_29,
    SC_MODULE_GPIO_30,
    SC_MODULE_GPIO_31,
    SC_MODULE_GPIO_32,

    SC_MODULE_GPIO_33,
    SC_MODULE_GPIO_34,
    SC_MODULE_GPIO_35,
    SC_MODULE_GPIO_36,

    SC_MODULE_GPIO_37,
    SC_MODULE_GPIO_38,

    -1,
};
#elif SIMCOM_A7678_V102
static int all_gpio[] =
{
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_00,
#endif
    SC_MODULE_GPIO_01,
    SC_MODULE_GPIO_02,
    SC_MODULE_GPIO_03,
    SC_MODULE_GPIO_04,
    SC_MODULE_GPIO_05,
#if (defined CUS_TOPFLY) || (defined A7672E_V201)
    SC_MODULE_GPIO_06,
#endif
    SC_MODULE_GPIO_07,
    SC_MODULE_GPIO_08,
    SC_MODULE_GPIO_09,
    SC_MODULE_GPIO_10,
    SC_MODULE_GPIO_11,
    SC_MODULE_GPIO_12,

    SC_MODULE_GPIO_13,
    SC_MODULE_GPIO_14,
    SC_MODULE_GPIO_15,
    SC_MODULE_GPIO_16,

    SC_MODULE_GPIO_17,
    SC_MODULE_GPIO_18,
    SC_MODULE_GPIO_19,
    SC_MODULE_GPIO_20,

    SC_MODULE_GPIO_21,
    SC_MODULE_GPIO_22,
    SC_MODULE_GPIO_23,
    SC_MODULE_GPIO_24,

    SC_MODULE_GPIO_25,
    SC_MODULE_GPIO_26,
    SC_MODULE_GPIO_27,
    SC_MODULE_GPIO_28,

    SC_MODULE_GPIO_29,
    SC_MODULE_GPIO_30,
    SC_MODULE_GPIO_31,
    SC_MODULE_GPIO_32,
    SC_MODULE_GPIO_33,
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_34,
#endif
#ifdef CUS_TOPFLY
    SC_MODULE_GPIO_35,
    SC_MODULE_GPIO_36,
    SC_MODULE_GPIO_37,
    SC_MODULE_GPIO_38,
    SC_MODULE_GPIO_39,
#endif
    -1,
};
#elif SIMCOM_A7683E_V102
static int all_gpio[] =
{
    SC_MODULE_GPIO_00,
    SC_MODULE_GPIO_01,
    SC_MODULE_GPIO_02,
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_03,
#endif
    SC_MODULE_GPIO_04,
    SC_MODULE_GPIO_05,
    SC_MODULE_GPIO_06,
    SC_MODULE_GPIO_07,
    SC_MODULE_GPIO_08,
    SC_MODULE_GPIO_09,
    SC_MODULE_GPIO_10,
    SC_MODULE_GPIO_11,
    SC_MODULE_GPIO_12,
    SC_MODULE_GPIO_13,
    SC_MODULE_GPIO_14,
    SC_MODULE_GPIO_15,
    SC_MODULE_GPIO_16,
    SC_MODULE_GPIO_17,
    SC_MODULE_GPIO_18,
    SC_MODULE_GPIO_19,
    SC_MODULE_GPIO_20,
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_21,
#endif
    -1,
};
#elif SIMCOM_A7673G_V201
static int all_gpio[] =
{
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_00,
#endif
    SC_MODULE_GPIO_01,
    SC_MODULE_GPIO_02,
    SC_MODULE_GPIO_04,
    SC_MODULE_GPIO_05,
    SC_MODULE_GPIO_07,
    SC_MODULE_GPIO_08,
    SC_MODULE_GPIO_09,
    SC_MODULE_GPIO_10,
    SC_MODULE_GPIO_11,
    SC_MODULE_GPIO_12,
    SC_MODULE_GPIO_13,
    SC_MODULE_GPIO_14,
    SC_MODULE_GPIO_15,
    SC_MODULE_GPIO_16,
    SC_MODULE_GPIO_17,
    SC_MODULE_GPIO_18,
    SC_MODULE_GPIO_19,
    SC_MODULE_GPIO_20,
    SC_MODULE_GPIO_21,
    SC_MODULE_GPIO_22,
    SC_MODULE_GPIO_23,
    SC_MODULE_GPIO_24,
    SC_MODULE_GPIO_25,
    SC_MODULE_GPIO_26,
    SC_MODULE_GPIO_27,
    SC_MODULE_GPIO_28,
    SC_MODULE_GPIO_29,
    SC_MODULE_GPIO_30,
    SC_MODULE_GPIO_31,
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_32,
#endif
    -1,
};
#elif SIMCOM_A7670G_UB_V101
static int all_gpio[] =
{
    SC_MODULE_GPIO_00,
    SC_MODULE_GPIO_01,
    SC_MODULE_GPIO_02,
    SC_MODULE_GPIO_03,
    SC_MODULE_GPIO_04,
    SC_MODULE_GPIO_05,
    SC_MODULE_GPIO_06,
    SC_MODULE_GPIO_07,
    SC_MODULE_GPIO_08,
    SC_MODULE_GPIO_09,
    SC_MODULE_GPIO_10,
    SC_MODULE_GPIO_11,
    SC_MODULE_GPIO_12,
    SC_MODULE_GPIO_13,
    SC_MODULE_GPIO_14,
    SC_MODULE_GPIO_15,
    SC_MODULE_GPIO_16,
    SC_MODULE_GPIO_17,
    SC_MODULE_GPIO_18,
    SC_MODULE_GPIO_19,
    SC_MODULE_GPIO_20,
    SC_MODULE_GPIO_21,
    SC_MODULE_GPIO_22,
    SC_MODULE_GPIO_23,
    SC_MODULE_GPIO_24,
#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_25,
#endif

    -1,
};
#else
static int all_gpio[] =
{
#ifdef FEATURE_SIMCOM_NORI
    SC_MODULE_GPIO_00,
#endif
    SC_MODULE_GPIO_01,
    SC_MODULE_GPIO_02,
    SC_MODULE_GPIO_04,
    SC_MODULE_GPIO_05,
    SC_MODULE_GPIO_07,
    SC_MODULE_GPIO_08,
    SC_MODULE_GPIO_09,
    SC_MODULE_GPIO_10,

    SC_MODULE_GPIO_11,
    SC_MODULE_GPIO_12,
    SC_MODULE_GPIO_13,
    SC_MODULE_GPIO_14,
    SC_MODULE_GPIO_15,
    SC_MODULE_GPIO_16,
    SC_MODULE_GPIO_17,
    SC_MODULE_GPIO_18,

    SC_MODULE_GPIO_34,
    SC_MODULE_GPIO_35,
    SC_MODULE_GPIO_36,
    SC_MODULE_GPIO_37,
    SC_MODULE_GPIO_37,
    SC_MODULE_GPIO_36,
    SC_MODULE_GPIO_38,
    SC_MODULE_GPIO_39,

#ifdef FEATURE_SIMCOM_NODTR
    SC_MODULE_GPIO_40 ,
#endif

#ifdef CUS_A7670C_LANS_HWQS
    SC_MODULE_GPIO_41 ,
    SC_MODULE_GPIO_42 ,
    SC_MODULE_GPIO_43 ,
    SC_MODULE_GPIO_44 ,
#endif

    -1,
};
#endif


enum SC_GPIO_SWITCH
{
    SC_GPIO_SET_DIRECTION = 1,
    SC_GPIO_GET_DIRECTION,
    SC_GPIO_SET_LEVEL,
    SC_GPIO_GET_LEVEL,
    SC_GPIO_SET_INTERRUPT = 5,
    SC_GPIO_WAKEUP_ENABLE,
    SC_GPIO_CONFIG,
    SC_INIT_NET_LIGHT,
    SC_GPIO_AUTO_INPUT_TEST,
    SC_GPIO_TURN_ON_TEST = 10,
    SC_GPIO_TURN_OFF_TEST,
    SC_SIM_DET_FUNCTION_SWITCH,
    SC_GPIO_BACK = 99
};

/**
  * @brief  GPIO interrupt handle
  * @param  void
  * @note
  * @retval void
  */
void GPIO_IntHandler(void)
{
#ifdef GPIO_INT_WAKEUP_TEST
    static unsigned int i = 0;

    if (i % 2 == 0)
        sAPI_GpioSetValue(SC_MODULE_GPIO_10, 1);
    else
        sAPI_GpioSetValue(SC_MODULE_GPIO_10, 0);

    i++;
#endif
}

/**
  * @brief  GPIO to wake up system,interrupt handle
  * @param  void
  * @note
  * @retval void
  */
void GPIO_WakeupHandler(void)
{
#ifdef GPIO_INT_WAKEUP_TEST
    static unsigned int i = 0;

    if (i % 2 == 0)
        sAPI_GpioSetValue(SC_MODULE_GPIO_09, 1);
    else
        sAPI_GpioSetValue(SC_MODULE_GPIO_09, 0);

    i++;
#endif
    /* If you want to leave sleep after wake up, please disable system sleep ! */
    sAPI_SystemSleepSet(SC_SYSTEM_SLEEP_DISABLE);
}



/**
  * @brief  GPIO demo code.
  * @param  This demo will show how to set direction\level\read\write\interrupt on GPIOs.
  * @note
  * @retval void
  */
void GpioDemo(void)
{
    UINT32 opt = 0;
    SC_GPIOReturnCode ret;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] =
    {
        "1. GPIO set direction",
        "2. GPIO get direction",
        "3. GPIO set level",
        "4. GPIO get level",
        "5. GPIO set interrupt",
        "6. GPIO wakeup",
        "7. GPIO parameters config",
        "8. Init NET Light",
        "10. high level check",
        "11. low level check",
        "12. SIM DET function config",
        "99. back",

    };
    unsigned int gpio_num;
    unsigned int direction;
    unsigned int gpio_level = 0;
#ifdef GPIO_INT_WAKEUP_TEST
    SC_GPIOConfiguration pinConfig;
    pinConfig.initLv = 0;
    pinConfig.isr =  NULL;
    pinConfig.pinDir = SC_GPIO_IN_PIN;
    pinConfig.pinEd = SC_GPIO_TWO_EDGE;
    pinConfig.wu = GPIO_WakeupHandler;
    pinConfig.pinPull = SC_GPIO_PULLUP_ENABLE;//pull_up

    ret = sAPI_GpioConfig(SC_MODULE_GPIO_10, pinConfig);
    if (ret == SC_GPIORC_OK)
    {
        PrintfResp("\r\nConfig GPIO successed !\r\n");
    }
    else
    {
        printf("\r\nConfig GPIO failed ret =%d!\r\n", ret);
    }

    ret = sAPI_GpioConfig(SC_MODULE_GPIO_9, pinConfig);
    if (ret == SC_GPIORC_OK)
    {
        PrintfResp("\r\nConfig GPIO successed !\r\n");
    }
    else
    {
        printf("\r\nConfig GPIO failed ret= %d!\r\n", ret);
    }
#endif

    while (1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list, sizeof(options_list) / sizeof(options_list[0]));

        opt = UartReadValue();

        switch (opt)
        {
            case SC_GPIO_SET_DIRECTION:
            {
                PrintfResp("\r\nPlease input gpio number.\r\n");
                gpio_num = UartReadValue();

                if (gpio_num >= SC_MODULE_GPIO_MAX)
                {
                    PrintfResp("\r\nincorrect SC MODULE GPIO NUMBER.\r\n");
                    break;
                }

                PrintfResp("\r\nPlease input direction.     0:input    1:output.\r\n");
                direction = UartReadValue();

                ret = sAPI_GpioSetDirection(gpio_num, direction);
                if (ret != SC_GPIORC_OK)
                    printf("sAPI_setGpioDirection:    failed.");
                else
                    printf("sAPI_setGpioDirection:    success.");

                PrintfResp("\r\noperation successful!\r\n");

                break;
            }
            case SC_GPIO_GET_DIRECTION:
            {
                PrintfResp("\r\nPlease input gpio number.\r\n");
                gpio_num = UartReadValue();

                if (gpio_num >= SC_MODULE_GPIO_MAX)
                {
                    PrintfResp("\r\nincorrect SC MODULE GPIO NUMBER.\r\n");
                    break;
                }

                direction = sAPI_GpioGetDirection(gpio_num);

                char tmp[40];

                sprintf(tmp, "\r\nthe direction of gpio_%d is %d \r\n", gpio_num, direction);

                PrintfResp(tmp);
                PrintfResp("\r\noperation successful!\r\n");

                break;
            }
            case SC_GPIO_SET_LEVEL:
            {
                PrintfResp("\r\nPlease input gpio number.\r\n");
                gpio_num = UartReadValue();

                if (gpio_num >= SC_MODULE_GPIO_MAX)
                {
                    PrintfResp("\r\nincorrect SC MODULE GPIO NUMBER.\r\n");
                    break;
                }

                PrintfResp("\r\nPlease input gpio level.    0:low level    1:high level.\r\n");
                gpio_level = UartReadValue();

                ret = sAPI_GpioSetValue(gpio_num, gpio_level);
                if (ret != SC_GPIORC_OK)
                    printf("sAPI_GpioSetValue:    failed.");
                else
                    printf("sAPI_GpioSetValue:    success.");

                PrintfResp("\r\noperation successful!\r\n");

                break;
            }
            case SC_GPIO_GET_LEVEL:
            {
                PrintfResp("\r\nPlease input gpio number.\r\n");
                gpio_num = UartReadValue();

                if (gpio_num >= SC_MODULE_GPIO_MAX)
                {
                    PrintfResp("\r\nincorrect SC MODULE GPIO NUMBER.\r\n");
                    break;
                }

                ret = sAPI_GpioGetValue(gpio_num);

                char tmp[40];

                sprintf(tmp, "\r\nthe level of gpio_%d is %d \r\n", gpio_num, ret);

                PrintfResp(tmp);
                PrintfResp("\r\noperation successful!\r\n");

                break;
            }
            case SC_GPIO_SET_INTERRUPT:
            {
                PrintfResp("\r\nPlease input gpio number.\r\n");
                gpio_num = UartReadValue();

                if (gpio_num >= SC_MODULE_GPIO_MAX)
                {
                    PrintfResp("\r\nincorrect SC MODULE GPIO NUMBER.\r\n");
                    break;
                }
                ret = sAPI_GpioSetDirection(gpio_num, 0);
                if (ret != SC_GPIORC_OK)
                {
                    printf("sAPI_setGpioDirection:    failed.");
                    break;
                }
                ret = sAPI_GpioConfigInterrupt(gpio_num, SC_GPIO_TWO_EDGE, GPIO_IntHandler);
                if (ret != SC_GPIORC_OK)
                {
                    printf("sAPI_GpioConfigInterrupt:    failed.");
                    break;
                }

                PrintfResp("\r\noperation successful!\r\n");
                break;
            }
            case SC_GPIO_WAKEUP_ENABLE:
            {
                PrintfResp("\r\nPlease input wake up gpio number.\r\n");
                gpio_num = UartReadValue();

                if (gpio_num >= SC_MODULE_GPIO_MAX)
                {
                    PrintfResp("\r\nincorrect SC MODULE GPIO NUMBER.\r\n");
                    break;
                }

                ret = sAPI_GpioWakeupEnable(gpio_num, SC_GPIO_FALL_EDGE);
                if (ret != SC_GPIORC_OK)
                {
                    PrintfResp("\r\nGpio set wake up failed !\r\n");
                    break;
                }

                PrintfResp("\r\nGpio set wake up successful !\r\n");
                break;
            }
            case SC_GPIO_CONFIG:
            {
                PrintfResp("\r\nPlease input all parameters, delimited by comma.\r\n");
                PrintfResp("\r\nSyntax: GPIO number,direction,init level,pull type,edge type\r\n");
                PrintfResp("\r\nExample: 0,0,1,1,3\r\n");
                char input_paras[40] = {0};
                UartReadLine(input_paras, 40);

                SC_GPIOConfiguration pinconfig;
                char *p_delim;
                p_delim = strtok(input_paras, ",");
                if (p_delim)
                    gpio_num = atoi(p_delim);
                else
                {
                    PrintfResp("\r\nFomat Error.\r\n");
                    break;
                }

                p_delim = strtok(NULL, ",");
                if (p_delim)
                    pinconfig.pinDir = atoi(p_delim);
                else
                {
                    PrintfResp("\r\nFomat Error.\r\n");
                    break;
                }

                p_delim = strtok(NULL, ",");
                if (p_delim)
                    pinconfig.initLv = atoi(p_delim);
                else
                {
                    PrintfResp("\r\nFomat Error.\r\n");
                    break;
                }

                p_delim = strtok(NULL, ",");
                if (p_delim)
                    pinconfig.pinPull = atoi(p_delim);
                else
                {
                    PrintfResp("\r\nFomat Error.\r\n");
                    break;
                }

                p_delim = strtok(NULL, ",");
                if (p_delim)
                    pinconfig.pinEd = atoi(p_delim);
                else
                {
                    PrintfResp("\r\nFomat Error.\r\n");
                    break;
                }

                if (pinconfig.pinEd == SC_GPIO_RISE_EDGE || pinconfig.pinEd == SC_GPIO_FALL_EDGE || pinconfig.pinEd == SC_GPIO_TWO_EDGE)
                    pinconfig.isr = GPIO_IntHandler;
                else
                    pinconfig.isr = NULL;

                pinconfig.wu = GPIO_WakeupHandler;

                ret = sAPI_GpioConfig(gpio_num, pinconfig);
                if (ret == SC_GPIORC_OK)
                    PrintfResp("\r\nConfig OK!\r\n");
                else
                {
                    char tmp[40];
                    sprintf(tmp, "\r\nConfig Error, Error code is %d\r\n", ret);
                    PrintfResp(tmp);
                }
                break;
            }
            case SC_GPIO_TURN_ON_TEST:
            {
                SC_GPIOConfiguration gpio_cfg =
                {
                    .initLv = 1,
                    .isr = NULL,
                    .pinDir = 1,
                    .pinEd = 0,
                    .wu = NULL,
                    .pinPull = 0,
                };
                int *p = all_gpio;
                char tmp1[80];
                do
                {
                    ret = sAPI_GpioConfig(*p, gpio_cfg);
                    if (ret){
                        sprintf(tmp1,"\r\ngpio [%d] cfg fail\r\n", *p);
                        PrintfResp(tmp1);}

                    if (sAPI_GpioGetValue(*p) != 1){
                        sprintf(tmp1,"\r\ngpio [%d] is not high level\r\n", *p);
                        PrintfResp(tmp1);}
                } while (*p ++ != -1);
                break;
            }
            case SC_GPIO_TURN_OFF_TEST:
            {
                SC_GPIOConfiguration gpio_cfg =
                {
                    .initLv = 0,
                    .isr = NULL,
                    .pinDir = 1,
                    .pinEd = 0,
                    .wu = NULL,
                    .pinPull = 0,
                };
                int *p = all_gpio;
                char tmp2[80];
                do
                {
                    ret = sAPI_GpioConfig(*p, gpio_cfg);
                    if (ret){
                        sprintf(tmp2,"\r\ngpio [%d] cfg fail\r\n", *p);
                        PrintfResp(tmp2);}

                    if (sAPI_GpioGetValue(*p) != 0){
                        sprintf(tmp2,"\r\ngpio [%d] is not low level\r\n", *p);
                        PrintfResp(tmp2);}
                } while (*p ++ != -1);
                break;
            }
            case SC_SIM_DET_FUNCTION_SWITCH:
            {
                #if 0
                PrintfResp("\r\nPlease input sim det function.\r\n");
                PrintfResp("\r\n0: disable sim det function\r\n");
                PrintfResp("\r\n1: enable sim det function\r\n");
                int sim_det_func = UartReadValue();

                if (sim_det_func > 1 && sim_det_func < 0)
                {
                    PrintfResp("\r\nincorrect sim_det_func NUMBER.\r\n");
                    break;
                }
                if(sim_det_func == 0)
                {
                    ret = sAPI_SimDetectPinConfig(SC_SIMDET_FUNCTION_DISABLE);
                    if (ret != SC_GPIORC_OK)
                    {
                        PrintfResp("\r\nSIM DET function disable failed !\r\n");
                        break;
                    }
                    else
                        PrintfResp("\r\nSIM DET function disable successful !\r\n");
                    SC_GPIOConfiguration pinConfig;
                    pinConfig.initLv = 0;
                    pinConfig.isr =  GPIO_IntHandler;
                    pinConfig.pinDir = SC_GPIO_IN_PIN;
                    pinConfig.pinEd = SC_GPIO_TWO_EDGE;
                    pinConfig.wu = NULL;
                    pinConfig.pinPull = SC_GPIO_PULLUP_ENABLE;//pull_up
                    PrintfResp("\r\nPlease input sim det pin.\r\n");
                    int inc_gpio = UartReadValue();
                    if (inc_gpio >= SC_MODULE_GPIO_MAX)
                    {
                        PrintfResp("\r\nincorrect SC MODULE GPIO NUMBER.\r\n");
                        break;
                    }
                    ret = sAPI_GpioConfig(inc_gpio, pinConfig);
                    if (ret == SC_GPIORC_OK)
                    {
                        PrintfResp("\r\nConfig GPIO successed !\r\n");
                    }
                    else
                    {
                        printf("\r\nConfig GPIO failed ret =%d!\r\n", ret);
                    }
                }
                else
                {
                    ret = sAPI_SimDetectPinConfig(SC_SIMDET_FUNCTION_ENABLE);
                    if (ret != SC_GPIORC_OK)
                    {
                        PrintfResp("\r\nSIM DET function set failed !\r\n");
                        break;
                    }
                    else
                        PrintfResp("\r\nSIM DET function set successful !\r\n");
                }
                #endif
                break;
            }
            case SC_GPIO_BACK:
                return;
        }
    }
}
