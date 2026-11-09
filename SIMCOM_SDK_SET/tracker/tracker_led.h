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

/** Gọi 1 lần khi task tracker khởi động: có file dấu watchdog -> R-1.0 (và xóa file), không có -> R-2.0. */
void tracker_reset_reason_init(void);

/** Nguyên nhân reset để gửi trong NASA-1 ("R-x.x"). */
const char *tracker_reset_reason_get(void);

/** Gọi sau khi login thành công: các lần login sau trong cùng lần chạy là kết nối lại (R-0.0). */
void tracker_reset_reason_consumed(void);

#endif /* TRACKER_LED_H */
