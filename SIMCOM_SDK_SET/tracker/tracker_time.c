/**
 * @file    tracker_time.c
 * @brief   Thời gian: GNSS -> RTC (AT+CCLK) -> giờ mạng -> NTP.
 *          (Tách nguyên văn từ sc_application.c, không đổi logic.)
 */
#include "tracker_time.h"
#include "tracker_util.h"
#include "tracker_sensor.h"   /* tracker_gnss_fix_valid() */

/* ===========================================================================
 * 8. THỜI GIAN (GNSS -> RTC module -> mạng di động -> NTP)
 * =========================================================================== */

/** Đổi năm 2 chữ số sang 4 chữ số (<70 -> 20xx, còn lại -> 19xx). */
static int tracker_full_year(int yy) { return (yy < 70) ? (2000 + yy) : (1900 + yy); }

/** Số ngày của tháng `mon` (1-12) trong năm `year`, có xử lý năm nhuận. */
static int tracker_days_in_month(int year, int mon)
{
    static const int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    BOOL leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (mon == 2 && leap) return 29;
    if (mon < 1 || mon > 12) return 31;         /* phòng vệ: giá trị mon lỗi */
    return days[mon - 1];
}

/** Cộng thêm `add_days` ngày vào (year, mon, day), lăn qua tháng/năm nếu vượt quá số ngày trong tháng. */
static void tracker_add_days(int *year, int *mon, int *day, int add_days)
{
    *day += add_days;
    while (*day > tracker_days_in_month(*year, *mon)) {
        *day -= tracker_days_in_month(*year, *mon);
        if (++(*mon) > 12) { *mon = 1; (*year)++; }
    }
}

/** Định dạng "YYYY-MM-DD hh:mm:ss". */
static void tracker_time_to_str(const tracker_time_t *t, char *out, size_t out_size)
{
    snprintf(out, out_size, "%04d-%02d-%02d %02d:%02d:%02d",
             t->year, t->mon, t->day, t->hour, t->min, t->sec);
}

/**
 * Ghi giờ vào RTC của module (AT+CCLK): ghi GIỜ UTC kèm "+28" (28 x 15 phút = +7 giờ).
 * ĐÃ XÁC MINH bằng log thực tế ngày 05/11/2026 trên module A7677S:
 *   - Ghi UTC kèm "+28"  -> AT+CCLK? đọc ra đúng giờ Việt Nam.
 *   - Ghi giờ VN kèm "+28" (bản thử v1.0.261105b) -> đọc ra dư 7 giờ (11:19 thành 18:19).
 * Tức là module tự cộng múi giờ khi đọc; KHÔNG đổi sang giờ địa phương trước khi ghi.
 */
static void tracker_set_rtc(const tracker_time_t *t)
{
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "AT+CCLK=\"%02d/%02d/%02d,%02d:%02d:%02d+28\"\r",
             t->year % 100, t->mon, t->day, t->hour, t->min, t->sec);
    tracker_at(cmd, TRACKER_AT_TMO_CCLK_SET, NULL, 0);
}

/** Đọc giờ UTC thô từ GNSS (chỉ khi đã fix). @return FALSE nếu chưa có dữ liệu hợp lệ. */
static BOOL tracker_get_raw_utc_from_gnss(tracker_time_t *t)
{
    SC_GNSSINFO *info;
    int hh, mm, ss, dd, mo, yy;
    if (t == NULL) return FALSE;

    info = sAPI_GnssNavdataGet();
    if (!tracker_gnss_fix_valid(info)) return FALSE;    /* gồm cả dữ liệu cũ sau khi tắt GPS */
    if (info->time == NULL || info->date == NULL ||
        info->time[0] == '\0' || info->date[0] == '\0') return FALSE;
    /* Định dạng NMEA: time = hhmmss, date = ddmmyy */
    if (sscanf(info->time, "%2d%2d%2d", &hh, &mm, &ss) != 3 ||
        sscanf(info->date, "%2d%2d%2d", &dd, &mo, &yy) != 3) return FALSE;

    t->year = tracker_full_year(yy);
    t->mon  = mo;  t->day = dd;
    t->hour = hh;  t->min = mm;  t->sec = ss;
    return TRUE;
}

/** Nguồn giờ 1: GNSS, cộng 7 giờ sang giờ Việt Nam. */
static BOOL tracker_get_datetime_from_gnss(char *out, size_t out_size)
{
    tracker_time_t t;
    if (!tracker_get_raw_utc_from_gnss(&t)) return FALSE;

    t.hour += TRACKER_UTC_OFFSET_HOURS;
    if (t.hour >= 24) {
        t.hour -= 24;
        tracker_add_days(&t.year, &t.mon, &t.day, 1);   /* lăn đúng qua tháng/năm, kể cả năm nhuận */
    }

    tracker_time_to_str(&t, out, out_size);
    return TRUE;
}

/** Phân tích phản hồi "+CCLK: "yy/mm/dd,hh:mm:ss+zz"". @return FALSE nếu sai định dạng hoặc năm < 2020. */
static BOOL tracker_parse_cclk_datetime(const char *resp, char *out, size_t out_size)
{
    tracker_time_t t;
    int yy;
    const char *p = (resp != NULL) ? strstr(resp, "+CCLK:") : NULL;

    if (p == NULL || out == NULL || out_size == 0) return FALSE;
    if (sscanf(p, "+CCLK: \"%d/%d/%d,%d:%d:%d", &yy, &t.mon, &t.day, &t.hour, &t.min, &t.sec) != 6) return FALSE;

    t.year = tracker_full_year(yy);
    if (t.year < TRACKER_MIN_VALID_YEAR) return FALSE;
    tracker_time_to_str(&t, out, out_size);
    return TRUE;
}

/** Nguồn giờ 2: đọc RTC của module (AT+CCLK?). */
static BOOL tracker_read_cclk(char *out, size_t out_size)
{
    char resp[128];
    return tracker_at("AT+CCLK?\r", TRACKER_AT_TMO_SHORT, resp, sizeof(resp)) == 0 &&
           tracker_parse_cclk_datetime(resp, out, out_size);
}

/** Kiểm tra năm nhuận theo lịch Gregorian. */
#define IS_LEAP(y) (((y) % 4 == 0 && (y) % 100 != 0) || (y) % 400 == 0)

/** Đổi số ngày kể từ 1970-01-01 sang (năm, tháng, ngày). */
static void tracker_civil_from_days(int64_t days_since_epoch, int *out_year, int *out_month, int *out_day)
{
    static const int days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int64_t rem = days_since_epoch;
    int year = 1970;
    int m;

    /* Xác định năm: trừ dần số ngày trong từng năm */
    for (;;) {
        int diy = IS_LEAP(year) ? 366 : 365;
        if (rem < diy) break;
        rem -= diy;
        year++;
    }

    /* Xác định tháng: trừ dần số ngày trong từng tháng */
    for (m = 0; m < 12; m++) {
        int dim = (m == 1 && IS_LEAP(year)) ? 29 : days_in_month[m];
        if (rem < dim) break;
        rem -= dim;
    }

    *out_year  = year;
    *out_month = m + 1;
    *out_day   = (int)rem + 1;
}

/** Gửi một gói NTP tới địa chỉ rp, chờ tối đa 3 giây. @param unix_sec [out] giây Unix. */
static BOOL tracker_ntp_query(const struct addrinfo *rp, uint32_t *unix_sec)
{
    unsigned char pkt[48] = {0};
    fd_set rfds;
    struct timeval tv = { 3, 0 };
    struct sockaddr addr;
    socklen_t addr_len = sizeof(addr);
    uint32_t ntp_sec;
    BOOL ok = FALSE;
    int fd = socket(rp->ai_family, rp->ai_socktype, 0);

    if (fd < 0) return FALSE;
    pkt[0] = 0x23;                                        /* LI=0, VN=4, Mode=3 (client) */
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    if (sendto(fd, pkt, sizeof(pkt), 0, rp->ai_addr, rp->ai_addrlen) == (int)sizeof(pkt) &&
        select(fd + 1, &rfds, NULL, NULL, &tv) > 0 && FD_ISSET(fd, &rfds) &&
        recvfrom(fd, pkt, sizeof(pkt), 0, &addr, &addr_len) >= 48) {
        /* Transmit timestamp nằm ở byte 40-43 (big-endian) */
        ntp_sec = ((uint32_t)pkt[40] << 24) | ((uint32_t)pkt[41] << 16) |
                  ((uint32_t)pkt[42] << 8)  |  (uint32_t)pkt[43];
        if (ntp_sec >= NTP_UNIX_OFFSET) { *unix_sec = ntp_sec - NTP_UNIX_OFFSET; ok = TRUE; }
    }
    close(fd);
    return ok;
}

/**
 * Lấy giờ từ máy chủ NTP (pool.ntp.org).
 * TRACKER_NTP_TZ_HOURS = 0 nên kết quả là giờ UTC, đúng với tracker_set_rtc() (ghi UTC kèm "+28").
 */
static BOOL tracker_ntp_get_vietnam_time(tracker_time_t *t)
{
    struct addrinfo hints, *list = NULL, *rp;
    uint32_t unix_sec;
    int64_t local, days, rem;
    BOOL ok = FALSE;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    if (getaddrinfo("pool.ntp.org", "123", &hints, &list) != 0 || list == NULL) return FALSE;

    for (rp = list; rp != NULL && !ok; rp = rp->ai_next) {   /* thử lần lượt từng địa chỉ */
        if (!tracker_ntp_query(rp, &unix_sec)) continue;

        local = (int64_t)unix_sec + (int64_t)TRACKER_NTP_TZ_HOURS * 3600;
        days  = local / 86400;
        rem   = local % 86400;
        if (rem < 0) { rem += 86400; days--; }

        tracker_civil_from_days(days, &t->year, &t->mon, &t->day);
        if (t->year < 1970 || t->year > 2037) continue;       /* kết quả vô lý -> thử địa chỉ khác */
        t->hour = (int)(rem / 3600);
        t->min  = (int)(rem % 3600 / 60);
        t->sec  = (int)(rem % 60);
        ok = TRUE;
    }
    freeaddrinfo(list);
    return ok;
}

/**
 * Đồng bộ giờ mạng khi chưa có giờ hợp lệ: bật tự cập nhật múi giờ (CTZU),
 * nếu RTC đã đúng thì xong, ngược lại lấy giờ NTP rồi ghi vào RTC.
 */
BOOL tracker_sync_network_time(void)
{
    char dt[32];
    tracker_time_t t;

    tracker_at("AT+CTZU=1\r", TRACKER_AT_TMO_SHORT, NULL, 0);
    if (tracker_read_cclk(dt, sizeof(dt))) return TRUE;       /* RTC đã hợp lệ (năm >= 2020) */
    if (!tracker_ntp_get_vietnam_time(&t)) return FALSE;

    tracker_set_rtc(&t);
    return tracker_read_cclk(dt, sizeof(dt));                 /* đọc lại để xác nhận */
}

/** Định kỳ (mỗi 300 s) ghi giờ GNSS vào RTC. @param elapsed_sec_since_last_call giây kể từ lần gọi trước. */
void tracker_sync_rtc_from_gnss_if_due(int elapsed_sec_since_last_call)
{
    static int seconds_since_last_sync = TRACKER_RTC_SYNC_INTERVAL_SEC;  /* để lần đầu đồng bộ ngay */
    tracker_time_t t;

    seconds_since_last_sync += elapsed_sec_since_last_call;
    if (seconds_since_last_sync < TRACKER_RTC_SYNC_INTERVAL_SEC) return;
    if (!tracker_get_raw_utc_from_gnss(&t)) return;

    tracker_set_rtc(&t);
    seconds_since_last_sync = 0;
}

/**
 * Lấy giờ hiện tại "YYYY-MM-DD hh:mm:ss" theo thứ tự ưu tiên:
 *   1) GNSS  2) RTC module (AT+CCLK?)  3) giờ mạng di động.
 * @return FALSE nếu cả ba nguồn đều chưa hợp lệ.
 */
BOOL tracker_get_datetime(char *out, size_t out_size)
{
    scRealTime rt;
    if (out == NULL || out_size == 0) return FALSE;

    if (tracker_get_datetime_from_gnss(out, out_size)) return TRUE;
    if (tracker_read_cclk(out, out_size)) return TRUE;

    memset(&rt, 0, sizeof(rt));
    if (sAPI_NetworkGetRealTime(&rt) == SC_NET_SUCCESS && rt.year >= TRACKER_MIN_VALID_YEAR) {
        tracker_time_t t = { rt.year, rt.mon, rt.day, rt.hour, rt.min, rt.sec };
        tracker_time_to_str(&t, out, out_size);
        return TRUE;
    }
    return FALSE;
}

/** Số giây kể từ 00:00 của ngày hiện tại (0 nếu chưa có giờ). Dùng để đo chu kỳ backup. */
uint32_t tracker_get_seconds_today(void)
{
    char dt[32];
    int year, month, day, hour, minute, second;
    if (!tracker_get_datetime(dt, sizeof(dt))) return 0;
    if (sscanf(dt, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) != 6) return 0;
    return (uint32_t)(second + minute * 60 + hour * 3600);
}

/**
 * Tính timestamp mới = base_dt + add_sec giây, ghi vào out[out_size].
 * Format: "YYYY-MM-DD hh:mm:ss". Xử lý đúng qua nửa đêm (ngày ++).
 * Dùng để tạo timestamp chính xác 10s/bản tin mà không phụ thuộc GPS granularity 1Hz.
 * @return FALSE nếu base_dt không parse được.
 */
BOOL tracker_datetime_add_seconds(const char *base_dt, int add_sec, char *out, size_t out_size)
{
    static const int days_in_month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int y, mo, d, h, mi, s, total_sec;

    if (base_dt == NULL || out == NULL || out_size == 0) return FALSE;
    if (sscanf(base_dt, "%d-%d-%d %d:%d:%d", &y, &mo, &d, &h, &mi, &s) != 6) return FALSE;

    total_sec = s + mi * 60 + h * 3600 + add_sec;

    /* Xử lý tràn sang ngày kế tiếp (hoặc rút về ngày trước) */
    while (total_sec >= 86400) {
        total_sec -= 86400;
        d++;
        int dim = ((mo == 2) && ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0))) ? 29 : days_in_month[mo - 1];
        if (d > dim) { d = 1; mo++; if (mo > 12) { mo = 1; y++; } }
    }
    while (total_sec < 0) {
        total_sec += 86400;
        d--;
        if (d < 1) { mo--; if (mo < 1) { mo = 12; y--; }
            int dim = ((mo == 2) && ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0))) ? 29 : days_in_month[mo - 1];
            d = dim;
        }
    }

    h  = total_sec / 3600;
    mi = (total_sec % 3600) / 60;
    s  = total_sec % 60;
    snprintf(out, out_size, "%04d-%02d-%02d %02d:%02d:%02d", y, mo, d, h, mi, s);
    return TRUE;
}
