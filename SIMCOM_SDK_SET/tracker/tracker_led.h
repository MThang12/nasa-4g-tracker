#ifndef TRACKER_LED_H
#define TRACKER_LED_H

/**
 * @file    tracker_led.h
 * @brief   Đèn LED trạng thái và watchdog.
 *          Task LED cho WDT ăn mỗi 1 s, chỉ khi task tracker còn gọi tracker_alive()
 *          trong TRACKER_WDT_STALL_SEC (xem tracker_config.h).
 */
#include "tracker_common.h"

/* Chế độ LED do task tracker ghi, task LED đọc (volatile vì dùng chung giữa 2 task) */
extern volatile tracker_led_mode_t led_net_mode;
extern volatile tracker_led_mode_t led_gnss_mode;

/** Task tracker báo còn sống. Gọi trong mọi vòng lặp/vòng chờ của task tracker. */
void tracker_alive(void);

/** Task LED + giám sát watchdog (tạo trong userSpace_Main). */
void sTask_LedControl(void *arg);

#endif /* TRACKER_LED_H */
