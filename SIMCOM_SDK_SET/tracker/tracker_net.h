#ifndef TRACKER_NET_H
#define TRACKER_NET_H

/**
 * @file    tracker_net.h
 * @brief   SIM, mạng di động, PDP và TCP.
 */
#include "tracker_common.h"

/** Đọc cường độ sóng CSQ (0-31, 99 = không xác định). */
int tracker_get_csq(void);

/** Mở kết nối TCP tới server. @return fd hoặc -1. */
int tracker_tcp_connect(void);

/** Gửi đủ len byte. @return len nếu thành công, -1 nếu lỗi. */
int tracker_tcp_send_all(int fd, const char *data, int len);

/** Đóng socket và đặt *fd = -1. */
void tracker_tcp_close(int *fd);

/** Chờ SIM + mạng; sau TRACKER_RF_RESET_RETRIES lần lỗi thì reset RF (CFUN=0/1). */
void tracker_wait_network(void);

/** Kích hoạt PDP, lặp đến khi được. @return FALSE nếu giữa chừng mất SIM/mạng. */
BOOL tracker_activate_pdp(void);

#endif /* TRACKER_NET_H */
