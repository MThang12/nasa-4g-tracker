/**
 * @file    tracker_protocol.c
 * @brief   Giao thức NASA v1.3: đóng gói NASA-1/2/7, msg_id theo ngày, ACK đăng nhập, giám sát phiên.
 *          (Tách nguyên văn từ sc_application.c, không đổi logic.)
 */
#include "tracker_protocol.h"
#include "tracker_util.h"
#include "tracker_time.h"
#include "tracker_device.h"
#include "tracker_net.h"
#include "tracker_backup.h"
#include "tracker_led.h"
#include "tracker_sensor.h"    /* tracker_get_voltage() cho lệnh sa,29 */
#include "tracker_sms.h"       /* tracker_sms_poll() trong lúc chờ ACK */

/* ===========================================================================
 * 11. ĐÓNG GÓI BẢN TIN NASA
 * =========================================================================== */

/** Thêm "*<checksum>\r\n" vào payload để có bản tin hoàn chỉnh. @return độ dài bản tin, <0 nếu lỗi. */
static int tracker_finish_msg(const char *payload, char *out, size_t out_size)
{
    return snprintf(out, out_size, "%s*%lu\r\n", payload,
                    (unsigned long)tracker_checksum((const uint8_t *)payload, (uint32_t)strlen(payload)));
}

/**
 * ID bản tin là số thứ tự tính từ đầu ngày (giao thức NASA v1.3, mục 2).
 * So phần ngày "YYYY-MM-DD" của datetime với ngày của bản tin trước đó:
 * sang ngày mới thì đặt lại msg_id = 1.
 * Lần đầu có giờ sau khởi động: nếu cùng ngày với ID đã dành trên flash thì tiếp tục từ ID đó.
 * Gọi ngay trước khi dùng c->msg_id cho một bản tin (NASA-1/2/7).
 */
void tracker_msg_id_check_new_day(tracker_ctx_t *c, const char *datetime)
{
    static char last_day[11] = {0};                        /* "YYYY-MM-DD" của bản tin gần nhất */

    if (c == NULL || datetime == NULL || strlen(datetime) < 10) return;
    if (last_day[0] == '\0') {
        /* Lần đầu có giờ sau khởi động: tiếp tục từ ID đã dành trên flash nếu cùng ngày (tránh trùng ID) */
        uint32_t resume = tracker_state_msg_id_resume(datetime);
        if (resume > c->msg_id) {
            sAPI_Debug("[NASA] Resume msg_id %lu (cung ngay %.10s)", (unsigned long)resume, datetime);
            c->msg_id = resume;
        }
    } else if (strncmp(last_day, datetime, 10) != 0) {
        sAPI_Debug("[NASA] New day %.10s -> reset msg_id", datetime);
        c->msg_id = 1;
    }
    memcpy(last_day, datetime, 10);
    last_day[10] = '\0';
    tracker_state_msg_id_reserve(datetime, c->msg_id);     /* chỉ ghi flash khi hết khối ID đã dành */
}

/** Tạo bản tin đăng nhập NASA-1 với ID hiện tại của phiên. @return -1 nếu chưa có giờ hợp lệ. */
int tracker_build_login(char *out, size_t out_size, tracker_ctx_t *c)
{
    char datetime[32], imei[32], sim_id[32], payload[224];
    if (out == NULL || out_size == 0 || c == NULL || !tracker_get_datetime(datetime, sizeof(datetime))) return -1;

    tracker_msg_id_check_new_day(c, datetime);
    tracker_get_imei(imei, sizeof(imei));
    tracker_get_sim_id(sim_id, sizeof(sim_id));

    snprintf(payload, sizeof(payload), "!NASA,%d,%lu,%s,%s,%s,%s,%s,%s,%s,%s",
             NASA_MSG_LOGIN, (unsigned long)c->msg_id, datetime, imei, sim_id,
             TRACKER_PLATE_NUMBER, TRACKER_HW_VERSION, TRACKER_FW_VERSION,
             TRACKER_LOGIN_CODE, tracker_reset_reason_get());
    return tracker_finish_msg(payload, out, out_size);
}

/** Tạo trường "status": bit GPS, FLASH_OK, ACC và BATTERY_OK (điện áp đo được >= TRACKER_BATTERY_OK_MIN_V). */
static int tracker_get_status(BOOL acc_on, BOOL gps_valid, double voltage)
{
    uint32_t status = 0;
    if (gps_valid) status |= TRACKER_STATUS_BIT_GPS;
    if (flash_ok)  status |= TRACKER_STATUS_BIT_FLASH_OK;
    if (acc_on)    status |= TRACKER_STATUS_BIT_ACC;
    if (voltage >= TRACKER_BATTERY_OK_MIN_V) status |= TRACKER_STATUS_BIT_BATTERY_OK;
    return (int)status;
}

/**
 * Bản tin vị trí dùng chung cho NASA-2 (realtime) và NASA-7 (backup) - chỉ khác loại và giờ.
 * Thứ tự trường: giờ, vĩ độ, kinh độ, tốc độ, status, CSQ, số vệ tinh,
 *                odometer, hướng, điện áp, nhiên liệu RS232, nhiên liệu AF, nhiệt độ.
 */
static int tracker_build_position(char *out, size_t out_size, int type, uint32_t msg_id,
                                  const char *datetime, const tracker_location_t *loc,
                                  BOOL acc_on, double voltage, int odometer_km, double heading)
{
    char payload[320];
    snprintf(payload, sizeof(payload),
             "!NASA,%d,%lu,%s,%.6f,%.6f,%d,%d,%d,%d,%d,%.1f,%.2f,%.0f,%.0f,%.1f",
             type, (unsigned long)msg_id, datetime, loc->lat, loc->lon,
             (int)(loc->speed + 0.5), tracker_get_status(acc_on, loc->fixValid, voltage),
             tracker_get_csq(), loc->satellites, odometer_km, heading,
             voltage, TRACKER_FUEL_RS232, TRACKER_FUEL_AF, TRACKER_TEMPERATURE);
    return tracker_finish_msg(payload, out, out_size);
}

/** NASA-2: hành trình realtime, dùng giờ đã capture từ bên ngoài (trước ADC delay).
 * Nhận datetime từ caller để đảm bảo timestamp khớp chính xác với GPS fix,
 * tránh lệch giây do delay 80ms của tracker_get_voltage(). */
int tracker_build_journey(char *out, size_t out_size, uint32_t msg_id,
                                 const tracker_location_t *loc, BOOL acc_on, double voltage,
                                 int odometer_km, double heading, const char *datetime)
{
    if (out == NULL || loc == NULL || out_size == 0 || datetime == NULL || datetime[0] == '\0') return -1;
    return tracker_build_position(out, out_size, NASA_MSG_JOURNEY, msg_id, datetime, loc, acc_on, voltage, odometer_km, heading);
}

/** NASA-7: gửi bù một bản ghi backup, giữ nguyên giờ lúc ghi. */
int tracker_build_backup(char *out, size_t out_size, uint32_t msg_id,
                                const backup_record_t *rec, int odometer_km, double heading)
{
    return tracker_build_position(out, out_size, NASA_MSG_BACKUP, msg_id, rec->datetime,
                                  &rec->loc, rec->acc_on, rec->voltage, odometer_km, heading);
}

/* ===========================================================================
 * 13. XÁC THỰC ACK ĐĂNG NHẬP VÀ GIÁM SÁT PHIÊN
 * =========================================================================== */

/**
 * Kiểm tra bản tin ACK có dạng "!NASA,1,...*<checksum>\r" và checksum đúng.
 * @param msg  Chuỗi kết thúc bằng '\0'.
 */
static BOOL tracker_validate_login_ack(const char *msg, int len)
{
    const char *end, *star;
    char *parse_end = NULL;
    uint32_t received;

    if (msg == NULL || len <= 0 || strncmp(msg, "!NASA,1,", 8) != 0) return FALSE;

    end = memchr(msg, '\r', (size_t)len);
    if (end == NULL) end = msg + len;

    star = end;                                            /* tìm dấu '*' cuối cùng trước '\r' */
    while (star > msg && star[-1] != '*') star--;
    if (star <= msg || star >= end) return FALSE;          /* star trỏ ngay sau dấu '*' */

    received = (uint32_t)strtoul(star, &parse_end, 10);
    if (parse_end == star) return FALSE;                   /* không có chữ số nào */

    return tracker_checksum((const uint8_t *)msg, (uint32_t)(star - 1 - msg)) == received;
}

/** Chờ server trả ACK login tối đa 15 giây. Trong lúc chờ vẫn backup mỗi giây. */
BOOL tracker_wait_login_ack(int fd)
{
    char rx[TRACKER_RX_BUF_SIZE] = {0};
    int total = 0, elapsed_ms = 0, backup_tick_ms = 0, ret;

    while (elapsed_ms < TRACKER_LOGIN_ACK_TIMEOUT_SEC * 1000) {
        fd_set rfds;
        struct timeval tv = { 0, TRACKER_RX_POLL_MS * 1000 };
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        ret = select(fd + 1, &rfds, NULL, NULL, &tv);
        if (ret < 0) return FALSE;

        tracker_alive();
        elapsed_ms     += TRACKER_RX_POLL_MS;
        backup_tick_ms += TRACKER_RX_POLL_MS;
        if (backup_tick_ms >= 1000) { tracker_process_backup(); tracker_sms_poll(); backup_tick_ms = 0; }

        if (ret == 0 || !FD_ISSET(fd, &rfds)) continue;    /* chưa có dữ liệu */

        ret = recv(fd, rx + total, (int)sizeof(rx) - total - 1, 0);
        if (ret <= 0) return FALSE;                        /* server đóng kết nối / lỗi */
        total += ret;
        rx[total] = '\0';

        if (memchr(rx, '\r', (size_t)total) != NULL) return tracker_validate_login_ack(rx, total);
        if (total >= (int)sizeof(rx) - 1) return FALSE;    /* đầy bộ đệm mà chưa có '\r' */
    }
    return FALSE;
}

/* ===========================================================================
 * 13b. LỆNH CẤU HÌNH TỪ SERVER (file "Config Command NASA-4G latest.xlsx")
 *   Ghi: "na,<mã lệnh>,<nội dung>"   Đọc: "sa,<mã lệnh>"   Dòng khác: bỏ qua.
 *   Phản hồi: văn bản như cột "Thiết bị phản hồi", các dòng cách nhau "\n"; qua TCP thêm "\r\n" ở cuối,
 *   qua SMS gửi nguyên văn (tracker_sms.c). Kênh TCP và SMS dùng chung tracker_cmd_execute().
 *   Dạng đóng gói lệnh/phản hồi qua TCP (chữ trần hay bọc !NASA): CHƯA XÁC MINH với server.
 * =========================================================================== */

/** "YYYY-MM-DD hh:mm:ss" -> 6 số. @return FALSE nếu không parse được. */
static BOOL tracker_parse_datetime(const char *dt, int *y, int *mo, int *d, int *h, int *mi, int *s)
{
    return dt != NULL && sscanf(dt, "%d-%d-%d %d:%d:%d", y, mo, d, h, mi, s) == 6;
}

/**
 * sa,29 - Kiểm tra thông tin thiết bị:
 *   NASA4G,<imei>,<fw>,<hw>,<khóa 0/1>,<số ngày hoạt động>,<thời điểm khởi động DDMMYYhhmmss>
 *   GSM,<0/1 GPRS>,<CSQ>
 *   GPS,<0/1/2 OK/ERROR/ngủ>,<0/1 fix>,<số vệ tinh>
 *   SV,<server>,<port>,<0/1 đã kết nối>
 *   PW,<0 normal/1 sleeping>,<điện áp nguồn ngoài>
 *   TI,<ddmmyy>-<hh:mm:ss>
 * Thiết bị tắt bằng lệnh 38: GPS báo 2 (đang ngủ), PW báo 1.
 * Chưa có chức năng khóa -> luôn 0. "Số ngày hoạt động" = số ngày kể từ lần khởi động này
 * (tính từ sAPI_GetTicks, chưa lưu tổng qua các lần khởi động).
 */
static int tracker_cmd_device_info(char *out, size_t out_size)
{
    char imei[32], now_dt[32], boot_dt[32], boot_str[16], ti_str[24];
    SC_GNSSINFO *gi = device_enabled ? sAPI_GnssNavdataGet() : NULL;   /* GPS đã tắt: không đọc dữ liệu cũ */
    uint32_t uptime_sec = sAPI_GetTicks() / TICKS_PER_SEC;
    int y, mo, d, h, mi, s;
    int gps_state = !device_enabled ? 2 : (gi != NULL ? 0 : 1);         /* 0 OK / 1 ERROR / 2 đang ngủ */

    tracker_get_imei(imei, sizeof(imei));
    snprintf(boot_str, sizeof(boot_str), "000000000000");
    snprintf(ti_str, sizeof(ti_str), "000000-00:00:00");
    if (tracker_get_datetime(now_dt, sizeof(now_dt)) && tracker_parse_datetime(now_dt, &y, &mo, &d, &h, &mi, &s)) {
        snprintf(ti_str, sizeof(ti_str), "%02d%02d%02d-%02d:%02d:%02d", d, mo, y % 100, h, mi, s);
        if (tracker_datetime_add_seconds(now_dt, -(int)uptime_sec, boot_dt, sizeof(boot_dt)) &&
            tracker_parse_datetime(boot_dt, &y, &mo, &d, &h, &mi, &s)) {
            snprintf(boot_str, sizeof(boot_str), "%02d%02d%02d%02d%02d%02d", d, mo, y % 100, h, mi, s);
        }
    }

    return snprintf(out, out_size,
                    "NASA4G,%s,%s,%s,0,%lu,%s\n"
                    "GSM,%d,%d\n"
                    "GPS,%d,%d,%d\n"
                    "SV,%s,%d,%d\n"
                    "PW,%d,%.2f\n"
                    "TI,%s",
                    imei, TRACKER_FW_VERSION, TRACKER_HW_VERSION,
                    (unsigned long)(uptime_sec / 86400UL), boot_str,
                    is_logged_in ? 1 : 0, tracker_get_csq(),
                    gps_state, (gi != NULL && gi->fixValid) ? 1 : 0,
                    gi != NULL ? atoi(gi->valid_sate_num) : 0,
                    TRACKER_SERVER_HOST, TRACKER_SERVER_PORT, is_logged_in ? 1 : 0,
                    device_enabled ? 0 : 1, tracker_get_voltage(),
                    ti_str);
}

/**
 * Lệnh 38 - Tắt/mở thiết bị. args trỏ ngay sau mã lệnh (",0" / ",1" với na).
 *   sa,38   -> "trang thai thiet bi: <0/1>"
 *   na,38,0 -> tắt: "thiet bi ngung hoat dong" (task tracker đóng phiên, tắt GPS, chỉ nhận SMS)
 *   na,38,1 -> mở : "thiet bi hoat dong tro lai"
 * Hành động thực hiện ở sc_application.c theo cờ device_enabled, sau khi phản hồi đã gửi.
 */
static int tracker_cmd_power(BOOL is_read, const char *args, char *out, size_t out_size)
{
    char *e = NULL;
    long v;

    if (is_read) return snprintf(out, out_size, "trang thai thiet bi: %d", device_enabled ? 1 : 0);

    if (args[0] != ',') return snprintf(out, out_size, "loi tham so");
    v = strtol(args + 1, &e, 10);
    if (e == args + 1 || (v != 0 && v != 1)) return snprintf(out, out_size, "loi tham so");
    while (*e == ' ' || *e == '\r' || *e == '\n') e++;
    if (*e != '\0') return snprintf(out, out_size, "loi tham so");

    tracker_device_set_enabled(v == 1 ? TRUE : FALSE);
    return snprintf(out, out_size, "%s", v == 1 ? "thiet bi hoat dong tro lai" : "thiet bi ngung hoat dong");
}

/**
 * Xử lý 1 dòng nhận từ server. Chỉ nhận dòng bắt đầu bằng "na," hoặc "sa,".
 * @return FALSE nếu gửi phản hồi lỗi (caller ngắt phiên).
 */
int tracker_cmd_execute(const char *line, char *reply, size_t reply_size)
{
    char *end = NULL;
    BOOL is_read;
    long code;
    int len;

    if (line == NULL || reply == NULL || reply_size == 0) return 0;
    if (strncmp(line, "sa,", 3) == 0)      is_read = TRUE;
    else if (strncmp(line, "na,", 3) == 0) is_read = FALSE;
    else return 0;                                         /* không phải lệnh cấu hình: bỏ qua */

    code = strtol(line + 3, &end, 10);
    if (end == line + 3) return 0;                         /* thiếu mã lệnh */

    if (is_read && code == 29) {
        len = tracker_cmd_device_info(reply, reply_size);
    } else if (code == 38) {
        len = tracker_cmd_power(is_read, end, reply, reply_size);
    } else {
        sAPI_Debug("[NASA] CMD %s,%ld: chua ho tro", is_read ? "sa" : "na", code);
        return 0;
    }

    if (len <= 0 || len >= (int)reply_size) return 0;
    sAPI_Debug("[NASA] CMD %s,%ld -> %s", is_read ? "sa" : "na", code, reply);
    return len;
}

/** Xử lý 1 dòng nhận qua TCP: chạy lệnh, gửi phản hồi kèm "\r\n". @return FALSE nếu gửi lỗi (caller ngắt phiên). */
static BOOL tracker_cmd_handle_line(int fd, const char *line)
{
    char reply[TRACKER_RX_BUF_SIZE];
    int len;

    sAPI_Debug("[NASA] RX: %s", line);
    len = tracker_cmd_execute(line, reply, sizeof(reply) - 2);
    if (len <= 0) return TRUE;
    reply[len++] = '\r';
    reply[len++] = '\n';
    reply[len]   = '\0';
    return tracker_tcp_send_all(fd, reply, len) == len;
}

/**
 * Kiểm tra phiên TCP còn sống bằng select() + recv(). Dữ liệu server gửi xuống được gom thành
 * từng dòng (kết thúc '\r' hoặc '\n') và xử lý lệnh cấu hình. @return FALSE nếu server đã đóng hoặc socket lỗi.
 */
BOOL tracker_poll_server(int fd, int timeout_ms)
{
    static char line[TRACKER_RX_BUF_SIZE];
    static int  line_len = 0;
    static int  line_fd = -1;                              /* socket của dữ liệu đang gom; đổi socket thì bỏ phần dở */
    fd_set rfds;
    struct timeval tv = { timeout_ms / 1000, (timeout_ms % 1000) * 1000 };
    char rx[TRACKER_RX_BUF_SIZE];
    int ret, i;

    if (fd < 0) return FALSE;
    if (fd != line_fd) { line_len = 0; line_fd = fd; }
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    ret = select(fd + 1, &rfds, NULL, NULL, &tv);
    if (ret < 0) return FALSE;
    if (ret == 0 || !FD_ISSET(fd, &rfds)) return TRUE;     /* không có dữ liệu = vẫn ổn */

    ret = recv(fd, rx, sizeof(rx) - 1, 0);
    if (ret <= 0) return FALSE;

    for (i = 0; i < ret; i++) {
        char ch = rx[i];
        if (ch == '\r' || ch == '\n') {
            if (line_len > 0) {
                line[line_len] = '\0';
                line_len = 0;
                if (!tracker_cmd_handle_line(fd, line)) return FALSE;
            }
        } else if (line_len < (int)sizeof(line) - 1) {
            line[line_len++] = ch;
        } else {
            line_len = 0;                                  /* dòng quá dài: bỏ */
        }
    }
    return TRUE;
}
