#ifndef TRACKER_SMS_H
#define TRACKER_SMS_H

/**
 * @file    tracker_sms.h
 * @brief   Nhận lệnh cấu hình "na,..."/"sa,..." qua SMS và gửi phản hồi về số đã nhắn.
 *          Chạy trong task tracker (không tạo task riêng): URC SMS đổ vào hàng đợi,
 *          tracker_sms_poll() đọc hàng đợi không chặn ở các vòng lặp/vòng chờ.
 */
#include "tracker_common.h"

/** Gọi 1 lần khi task tracker khởi động: tạo hàng đợi, đăng ký URC SMS. */
void tracker_sms_init(void);

/** Xử lý SMS mới (nếu có). Không chặn khi không có SMS; khi gửi phản hồi thì chờ tối đa TRACKER_SMS_SEND_TIMEOUT_SEC. */
void tracker_sms_poll(void);

/**
 * Phản hồi của lệnh SMS gần nhất cần báo lên server bằng NASA-13 (chỉ lưu khi đang có phiên).
 * @return chuỗi phản hồi (hợp lệ tới lần tracker_sms_poll() kế tiếp) và xóa cờ; NULL nếu không có.
 */
const char *tracker_sms_report_take(void);

#endif /* TRACKER_SMS_H */
