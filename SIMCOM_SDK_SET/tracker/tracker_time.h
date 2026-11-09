#ifndef TRACKER_TIME_H
#define TRACKER_TIME_H

/**
 * @file    tracker_time.h
 * @brief   Thời gian: GNSS -> RTC (AT+CCLK) -> giờ mạng -> NTP.
 *          RTC phải được ghi giờ UTC kèm "+28" (đã xác minh trên thiết bị, xem tracker_time.c).
 */
#include "tracker_common.h"

/** Đồng bộ giờ mạng/NTP khi chưa có giờ hợp lệ. */
BOOL tracker_sync_network_time(void);

/** Định kỳ (mỗi TRACKER_RTC_SYNC_INTERVAL_SEC) ghi giờ GNSS vào RTC. */
void tracker_sync_rtc_from_gnss_if_due(int elapsed_sec_since_last_call);

/** Lấy giờ hiện tại "YYYY-MM-DD hh:mm:ss" (GNSS -> RTC -> mạng). @return FALSE nếu chưa có giờ hợp lệ. */
BOOL tracker_get_datetime(char *out, size_t out_size);

/** Số giây kể từ 00:00 của ngày hiện tại (0 nếu chưa có giờ). */
uint32_t tracker_get_seconds_today(void);

/** out = base_dt + add_sec giây, định dạng "YYYY-MM-DD hh:mm:ss". */
BOOL tracker_datetime_add_seconds(const char *base_dt, int add_sec, char *out, size_t out_size);

#endif /* TRACKER_TIME_H */
