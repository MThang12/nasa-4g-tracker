#ifndef TRACKER_UTIL_H
#define TRACKER_UTIL_H

/**
 * @file    tracker_util.h
 * @brief   Tiện ích chung: GPIO, lệnh AT, checksum NASA, tọa độ NMEA.
 */
#include "tracker_common.h"

/** Cấu hình một chân GPIO (pull-up, không ngắt). @param dir SC_GPIO_IN_PIN / SC_GPIO_OUT_PIN */
SC_GPIOReturnCode tracker_gpio_setup(int pin, int dir);

/** Gửi lệnh AT trên kênh TRACKER_AT_CHANNEL và chờ phản hồi. resp = NULL nếu không cần đọc. @return 0 nếu thành công. */
int tracker_at(const char *cmd, int timeout, char *resp, size_t size);

/** Checksum NASA: tổng giá trị các byte. */
uint32_t tracker_checksum(const uint8_t *buf, uint32_t len);

/** Đổi tọa độ NMEA (ddmm.mmmm) sang độ thập phân; hướng S/W cho giá trị âm. */
double tracker_nmea_to_decimal(const char *coord, const char *direction);

#endif /* TRACKER_UTIL_H */
