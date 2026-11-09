#ifndef TRACKER_DEVICE_H
#define TRACKER_DEVICE_H

/**
 * @file    tracker_device.h
 * @brief   Nhận diện thiết bị: ICCID của SIM, IMEI của module.
 */
#include "tracker_common.h"

/** Đọc ICCID (bỏ ký tự 'F' cuối nếu có); lỗi thì dùng giá trị placeholder. */
void tracker_get_sim_id(char *out, size_t out_size);

/** Đọc IMEI bằng AT+CGSN; lỗi thì dùng placeholder. */
void tracker_get_imei(char *out, size_t out_size);

#endif /* TRACKER_DEVICE_H */
