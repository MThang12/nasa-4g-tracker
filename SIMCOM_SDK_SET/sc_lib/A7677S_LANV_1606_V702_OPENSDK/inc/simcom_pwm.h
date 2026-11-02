
#ifndef __SIMCOM_PWM_H__
#define __SIMCOM_PWM_H__

typedef enum
{
    SC_PWM_DEVICE_1 = 0,
    SC_PWM_DEVICE_2,
    SC_PWM_DEVICE_3,
    SC_PWM_DEVICE_4,
    SC_PWM_MAX_DEVICE
}PWM_DEVICE_NUM;

typedef enum
{
    PWM_OFF = 0,
    PWM_ON = 1
} PWM_SWITCH;

typedef enum
{
	PWM_CLK_SRC_13M,
	PWM_CLK_SRC_32K,
}PWM_CLK_SRC;

typedef enum
{
    PWM_RC_OK,
    PWM_RC_ERROR,
    PWM_RC_ILLEGALL_FREQUENCY,
    PWM_RC_ILLEGALL_DUTY_CYCLE,
    PWM_RC_ILLEGALL_DEVICE_NUMBER,
    PMW_RC_ILLEGALL_FRQ_DIV,
    PMW_RC_ILLEGALL_CNT,
    PMW_RC_ILLEGALL_SWITCH,
    PMW_RC_ILLEGALL_CLK
}PWM_ReturnCode;

typedef struct
{
    PWM_DEVICE_NUM      pwm_channel;/* four channels: [0,4] */
    PWM_CLK_SRC         pwm_clock;  /* The clock source is 13M and 32K */
    PWM_SWITCH          pwm_switch;
    unsigned short      frq_div;    /* The range is (0,64] */
    unsigned short      high_level_cnt;
    unsigned short      total_cnt;
}SC_PWM_CONFIG;


PWM_ReturnCode sAPI_PWMConfig(PWM_DEVICE_NUM     pwm_num, PWM_SWITCH on_off, unsigned char frq_div,unsigned short high_level_cnt, unsigned short total_cnt);

PWM_ReturnCode sAPI_PWMConfigEx(SC_PWM_CONFIG *handle);

#endif