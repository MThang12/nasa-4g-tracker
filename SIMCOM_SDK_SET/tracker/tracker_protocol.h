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

/**
 * NASA-13: báo lên server phản hồi của 1 lệnh nhận qua SMS (hoặc thông báo vào chế độ ngủ, lệnh 58),
 * gửi qua socket phiên hiện tại.
 * Dạng (theo mẫu server 13/11/2026): !NASA,13,<id>,<giờ>,info. <dòng 1>. <dòng 2>.#OK,*<checksum>
 * @return FALSE nếu gửi TCP lỗi (caller đóng phiên); TRUE nếu đã gửi hoặc bỏ qua (chưa có giờ).
 */
BOOL tracker_send_sms_report(tracker_ctx_t *c, const char *reply);

/** Chờ server trả ACK login tối đa TRACKER_LOGIN_ACK_TIMEOUT_SEC. */
BOOL tracker_wait_login_ack(int fd);

/**
 * Chạy 1 lệnh cấu hình "sa,<mã>" / "na,<mã>,<nội dung>" (dùng chung cho TCP và SMS).
 * @return độ dài phản hồi (không kèm "\r\n"); 0 nếu không phải lệnh hoặc lệnh chưa hỗ trợ (không phản hồi).
 */
int tracker_cmd_execute(const char *line, char *reply, size_t reply_size);

/** Lệnh 1: TRUE (một lần, rồi xóa cờ) nếu vừa đổi server -> caller đóng phiên để kết nối server mới. */
BOOL tracker_cmd_reconnect_take(void);

/** Lệnh 9: TRUE nếu đang chờ reset (caller đóng phiên TCP). */
BOOL tracker_cmd_reset_pending(void);

/** Lệnh 33: TRUE nếu đang chờ cập nhật firmware (caller đóng phiên TCP). */
BOOL tracker_cmd_fota_pending(void);

/** Lệnh 33: link firmware đang chờ (rồi xóa cờ); NULL nếu không có. */
const char *tracker_cmd_fota_take(void);

/** Gọi định kỳ cùng chỗ với tracker_sms_poll(): lệnh 9 đến hạn thì ghi backup RAM, ghi dấu R-3.0 rồi reset. */
void tracker_cmd_service(void);

/** Kiểm tra phiên TCP còn sống, nhận và xử lý lệnh cấu hình "na,..."/"sa,..." từ server. @return FALSE nếu server đóng/socket lỗi. */
BOOL tracker_poll_server(int fd, int timeout_ms);

/** Phản hồi lệnh cấu hình nhận qua TCP đang chờ báo NASA-13 (rồi xóa cờ); NULL nếu không có. */
const char *tracker_cmd_server_reply_take(void);

#endif /* TRACKER_PROTOCOL_H */
