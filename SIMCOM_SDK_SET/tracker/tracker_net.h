#ifndef TRACKER_NET_H
#define TRACKER_NET_H

/**
 * @file    tracker_net.h
 * @brief   SIM, mạng di động, PDP và TCP.
 */
#include "tracker_common.h"

/** Đọc cường độ sóng CSQ (0-31, 99 = không xác định). */
int tracker_get_csq(void);

/** Đọc server đã cấu hình (lệnh 1) từ TRACKER_SERVER_CFG_PATH. Gọi 1 lần khi khởi động. */
void tracker_server_load(void);

/** host 1..TRACKER_SERVER_HOST_MAX ký tự [A-Za-z0-9.-], port 1..65535. */
BOOL tracker_server_valid(const char *host, long port);

/** Lệnh 1: đặt server mới (đã kiểm tra hợp lệ), lưu file, bỏ chế độ quay về mặc định. @return FALSE nếu ghi file lỗi. */
BOOL tracker_server_set(const char *host, int port);

/** Server đã cấu hình (trả lời sa,1). */
const char *tracker_server_cfg_host(void);
int tracker_server_cfg_port(void);

/** Server đang dùng để kết nối (= đã cấu hình, hoặc mặc định nếu đang quay về). */
const char *tracker_server_host(void);
int tracker_server_port(void);

/** Báo kết quả 1 lần kết nối + đăng nhập để quyết định quay về server mặc định. */
void tracker_server_result(BOOL ok);

/** Mở kết nối TCP tới server đang dùng. @return fd hoặc -1. */
int tracker_tcp_connect(void);

/** Gửi đủ len byte. @return len nếu thành công, -1 nếu lỗi. */
int tracker_tcp_send_all(int fd, const char *data, int len);

/** Đóng socket và đặt *fd = -1. */
void tracker_tcp_close(int *fd);

/** Chờ SIM + mạng; sau TRACKER_RF_RESET_RETRIES lần lỗi thì reset RF (CFUN=0/1). */
void tracker_wait_network(void);

/** Kích hoạt PDP, lặp đến khi được. @return FALSE nếu giữa chừng mất SIM/mạng. */
BOOL tracker_activate_pdp(void);

/**
 * Lệnh 33: tải customer_app.bin từ url (sAPI_AppDownload, task riêng) rồi kiểm CRC (sAPI_AppPackageCrc).
 * Thành công -> ghi dấu R-3.0, reset để chạy firmware mới (không trả về).
 * Lỗi -> trả về, chạy tiếp firmware cũ. Quá TRACKER_FOTA_TIMEOUT_SEC -> reset mềm.
 * Gọi khi đã có PDP và không còn phiên TCP.
 */
void tracker_fota_run(const char *url);

#endif /* TRACKER_NET_H */
