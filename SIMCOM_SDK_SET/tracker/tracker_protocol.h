#ifndef TRACKER_PROTOCOL_H
#define TRACKER_PROTOCOL_H

/**
 * @file    tracker_protocol.h
 * @brief   Giao thức NASA v1.3: đóng gói NASA-1/2/7, msg_id theo ngày, ACK đăng nhập, giám sát phiên.
 */
#include "tracker_common.h"

/** Sang ngày mới (theo phần "YYYY-MM-DD" của datetime) thì đặt lại c->msg_id = 1. */
void tracker_msg_id_check_new_day(tracker_ctx_t *c, const char *datetime);

/** Tạo bản tin đăng nhập NASA-1 với ID hiện tại của phiên. @return -1 nếu chưa có giờ hợp lệ. */
int tracker_build_login(char *out, size_t out_size, tracker_ctx_t *c);

/** NASA-2: hành trình realtime, dùng datetime do caller đưa vào. */
int tracker_build_journey(char *out, size_t out_size, uint32_t msg_id,
                          const tracker_location_t *loc, BOOL acc_on, double voltage,
                          int odometer_km, double heading, const char *datetime);

/** NASA-7: gửi bù một bản ghi backup, giữ nguyên giờ lúc ghi. */
int tracker_build_backup(char *out, size_t out_size, uint32_t msg_id,
                         const backup_record_t *rec, int odometer_km, double heading);

/** Chờ server trả ACK login tối đa TRACKER_LOGIN_ACK_TIMEOUT_SEC. */
BOOL tracker_wait_login_ack(int fd);

/** Kiểm tra phiên TCP còn sống (dữ liệu server gửi xuống bị bỏ). @return FALSE nếu server đóng/socket lỗi. */
BOOL tracker_poll_server(int fd, int timeout_ms);

#endif /* TRACKER_PROTOCOL_H */
