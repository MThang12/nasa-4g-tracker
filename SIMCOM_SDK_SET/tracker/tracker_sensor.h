#ifndef TRACKER_SENSOR_H
#define TRACKER_SENSOR_H

/**
 * @file    tracker_sensor.h
 * @brief   Cảm biến: ACC, GNSS (vị trí, hướng, theo dõi fix), điện áp nguồn xe (ADC).
 */
#include "tracker_common.h"

/** Cấu hình chân ACC là ngõ vào pull-up. */
void tracker_acc_init(void);

/** @return TRUE nếu ACC bật (chân xuống mức thấp). */
BOOL tracker_get_acc_status(void);

/** Bật nguồn GNSS, chọn GPS+BDS+QZSS, cập nhật 1 Hz, khởi động nóng. */
void tracker_gnss_init(void);

/** Đọc vị trí hiện tại (tọa độ chỉ có khi đã fix). @return FALSE nếu SDK không trả dữ liệu. */
BOOL tracker_get_location(tracker_location_t *out);

/** Đọc hướng chạy (độ, [0, 360)) từ trường course. @return FALSE nếu chưa fix hoặc giá trị không hợp lệ. */
BOOL tracker_get_course(double *deg);

/** Theo dõi trạng thái fix mỗi ~100 ms: cập nhật LED GNSS, log khi đổi trạng thái. */
void tracker_monitor_gnss(BOOL *last_fix);

/** Đo điện áp nguồn xe (V), lấy trung bình nhiều lần đọc ADC. */
double tracker_get_voltage(void);

#endif /* TRACKER_SENSOR_H */
