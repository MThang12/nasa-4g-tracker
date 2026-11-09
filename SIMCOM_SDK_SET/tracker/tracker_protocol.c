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
 */
void tracker_msg_id_check_new_day(tracker_ctx_t *c, const char *datetime)
{
    static char last_day[11] = {0};                        /* "YYYY-MM-DD" của bản tin gần nhất */

    if (c == NULL || datetime == NULL || strlen(datetime) < 10) return;
    if (last_day[0] != '\0' && strncmp(last_day, datetime, 10) != 0) {
        sAPI_Debug("[NASA] New day %.10s -> reset msg_id", datetime);
        c->msg_id = 1;
    }
    memcpy(last_day, datetime, 10);
    last_day[10] = '\0';
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
        if (backup_tick_ms >= 1000) { tracker_process_backup(); backup_tick_ms = 0; }

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

/**
 * Kiểm tra phiên TCP còn sống bằng select() + recv(). Dữ liệu server gửi xuống bị bỏ
 * (chưa xử lý lệnh từ server). @return FALSE nếu server đã đóng hoặc socket lỗi.
 */
BOOL tracker_poll_server(int fd, int timeout_ms)
{
    fd_set rfds;
    struct timeval tv = { timeout_ms / 1000, (timeout_ms % 1000) * 1000 };
    char rx[TRACKER_RX_BUF_SIZE];
    int ret;

    if (fd < 0) return FALSE;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    ret = select(fd + 1, &rfds, NULL, NULL, &tv);
    if (ret < 0) return FALSE;
    if (ret == 0 || !FD_ISSET(fd, &rfds)) return TRUE;     /* không có dữ liệu = vẫn ổn */

    return recv(fd, rx, sizeof(rx) - 1, 0) > 0;
}
