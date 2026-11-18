/**
 * @file    tracker_sensor.c
 * @brief   Cảm biến: ACC, GNSS (vị trí, hướng, theo dõi fix), điện áp nguồn xe (ADC).
 *          (Tách nguyên văn từ sc_application.c, không đổi logic.)
 */
#include "tracker_sensor.h"
#include "tracker_util.h"
#include "tracker_led.h"
#include <math.h>

/* ===========================================================================
 * 9. ACC VÀ GNSS
 * =========================================================================== */

/* Trạng thái ACC đã lọc nhiễu: task LED ghi (tracker_acc_poll), task tracker đọc */
static volatile BOOL acc_state = FALSE;
static int acc_diff_count = 0;                       /* số lần đọc liên tiếp khác acc_state (chỉ task LED) */

/** Cấu hình chân ACC là ngõ vào pull-up, nhận ngay mức đang đọc được làm trạng thái ban đầu. */
void tracker_acc_init(void)
{
    SC_GPIOReturnCode ret = tracker_gpio_setup(TRACKER_ACC_GPIO_PIN, SC_GPIO_IN_PIN);
    int raw = (int)sAPI_GpioGetValue(TRACKER_ACC_GPIO_PIN);

    if (raw == 0 || raw == 1) acc_state = (raw == 0) ? TRUE : FALSE;
    sAPI_Debug("[NASA] ACC GPIO Initialized on PIN %d (Pull-Up) - ret=%d, level=%d -> ACC=%d",
               TRACKER_ACC_GPIO_PIN, ret, raw, acc_state);
}

/**
 * Lọc nhiễu ACC, gọi mỗi 100 ms từ task LED: mức mới phải giữ liên tục TRACKER_ACC_DEBOUNCE_MS
 * mới được nhận. Giá trị đọc khác 0/1 (lỗi đọc) bị bỏ qua, giữ trạng thái cũ.
 */
void tracker_acc_poll(void)
{
    int raw = (int)sAPI_GpioGetValue(TRACKER_ACC_GPIO_PIN);
    BOOL on;

    if (raw != 0 && raw != 1) return;
    on = (raw == 0) ? TRUE : FALSE;
    if (on == acc_state) { acc_diff_count = 0; return; }
    if (++acc_diff_count >= TRACKER_ACC_DEBOUNCE_MS / 100) {
        acc_diff_count = 0;
        acc_state = on;
        sAPI_Debug("[NASA] ACC: %d -> %d (giu %d ms)", !on, on, TRACKER_ACC_DEBOUNCE_MS);
    }
}

/** @return TRUE nếu ACC bật (đã lọc nhiễu). */
BOOL tracker_get_acc_status(void)
{
    return acc_state;
}

/** Callback trạng thái GNSS (SDK yêu cầu đăng ký, chưa xử lý gì). */
static void tracker_gnss_state_cb(SC_Gnss_State_Type event) { (void)event; }

/** Bật nguồn GNSS, chọn hệ GPS+BDS+QZSS, cập nhật 1 Hz, khởi động nóng. LED GNSS nháy nhanh cho đến khi có fix. */
void tracker_gnss_init(void)
{
    led_gnss_mode = LED_MODE_BLINK_FAST;

    sAPI_GnssStateBindCB(tracker_gnss_state_cb);
    tracker_at("AT+CVAUX=1\r", TRACKER_AT_TMO_SHORT, NULL, 0);   /* cấp nguồn cho anten/GNSS */
    sAPI_GnssPowerStatusSet(SC_GNSS_POWER_ON);
    sAPI_TaskSleep(SEC_TO_TICKS(2));
    sAPI_GnssModeSet(SC_GNSS_MODE_GPS_BDS_QZSS);
    sAPI_GnssNmeaRateSet(SC_GNSS_NMEA_UPDATE_RATE_1HZ);
    sAPI_GnssStartMode(SC_GNSS_START_HOT);
}

/*
 * Dữ liệu GNSS cũ sau khi tắt nguồn GPS (lệnh 38, chế độ ngủ lệnh 58):
 * sAPI_GnssNavdataGet() vẫn giữ nguyên giá trị lúc tắt (fixValid = TRUE, giờ/tọa độ cũ),
 * xác minh qua log 17/11/2026 (NASA-1 sau khi thức mang giờ lúc vào ngủ).
 * Khi tắt, lưu chuỗi giờ/ngày GPS; dữ liệu chỉ được coi là mới khi GPS báo fix với giờ/ngày khác.
 * Chỉ task tracker đọc/ghi.
 */
static BOOL gnss_stale = FALSE;
static char gnss_stale_time[sizeof(((SC_GNSSINFO *)0)->time)];
static char gnss_stale_date[sizeof(((SC_GNSSINFO *)0)->date)];

BOOL tracker_gnss_fix_valid(const SC_GNSSINFO *info)
{
    if (info == NULL || !info->fixValid) return FALSE;
    if (!gnss_stale) return TRUE;
    if (memcmp(info->time, gnss_stale_time, sizeof(gnss_stale_time)) == 0 &&
        memcmp(info->date, gnss_stale_date, sizeof(gnss_stale_date)) == 0) return FALSE;   /* vẫn là dữ liệu lúc tắt */
    gnss_stale = FALSE;
    sAPI_Debug("[NASA] GNSS: co du lieu moi sau khi bat lai");
    return TRUE;
}

/** Tắt nguồn GNSS (lệnh 38, lệnh 58). AT+CVAUX giữ nguyên. Bật lại bằng tracker_gnss_init(). */
void tracker_gnss_power_off(void)
{
    SC_GNSSINFO *info = sAPI_GnssNavdataGet();
    SC_Gnss_Return_Code ret;

    memset(gnss_stale_time, 0, sizeof(gnss_stale_time));
    memset(gnss_stale_date, 0, sizeof(gnss_stale_date));
    if (info != NULL) {
        memcpy(gnss_stale_time, info->time, sizeof(gnss_stale_time));
        memcpy(gnss_stale_date, info->date, sizeof(gnss_stale_date));
    }
    gnss_stale = TRUE;
    ret = sAPI_GnssPowerStatusSet(SC_GNSS_POWER_OFF);
    led_gnss_mode = LED_MODE_OFF;
    sAPI_Debug("[NASA] GNSS power off ret=%d", (int)ret);
}

/* Vị trí có fix gần nhất (lệnh 27 dùng khi hiện tại chưa fix). Chỉ task tracker đọc/ghi. */
static BOOL   last_fix_valid = FALSE;
static double last_fix_lat = 0.0, last_fix_lon = 0.0;

BOOL tracker_get_last_fix(double *lat, double *lon)
{
    if (!last_fix_valid || lat == NULL || lon == NULL) return FALSE;
    *lat = last_fix_lat;
    *lon = last_fix_lon;
    return TRUE;
}

/** Đổi chuỗi số trong struct GNSS (driver cập nhật đồng thời) sang double qua bộ đệm riêng. */
static double tracker_gnss_field_to_double(const char *field, size_t field_size)
{
    char buf[32];
    size_t n = (field_size < sizeof(buf) - 1) ? field_size : sizeof(buf) - 1;
    memcpy(buf, field, n);
    buf[n] = '\0';
    return atof(buf);
}

/**
 * Đọc mẫu GNSS thô (chưa lọc chạy/dừng). Tọa độ/tốc độ/độ cao chỉ lấy khi đã fix.
 * Tốc độ đổi sang km/h bằng TRACKER_GNSS_SPEED_TO_KMH. @return FALSE nếu SDK không trả dữ liệu.
 */
static BOOL tracker_read_raw_location(tracker_location_t *out, double *hdop)
{
    SC_GNSSINFO *info;

    memset(out, 0, sizeof(*out));
    if (hdop != NULL) *hdop = 0.0;
    info = sAPI_GnssNavdataGet();
    if (info == NULL) return FALSE;

    out->fixValid   = tracker_gnss_fix_valid(info);
    out->satellites = atoi(info->valid_sate_num);
    if (out->fixValid) {
        out->lat      = tracker_nmea_to_decimal(info->lat, info->lat_ns);
        out->lon      = tracker_nmea_to_decimal(info->lon, info->lon_we);
        out->speed    = tracker_gnss_field_to_double(info->speed, sizeof(info->speed)) * TRACKER_GNSS_SPEED_TO_KMH;
        out->altitude = tracker_gnss_field_to_double(info->alt, sizeof(info->alt));
        if (hdop != NULL) *hdop = tracker_gnss_field_to_double(info->hdop, sizeof(info->hdop));
    }
    return TRUE;
}

/* ===========================================================================
 * 9a. CHẠY / DỪNG VÀ ĐIỂM NEO (chống trôi GPS khi đứng yên, theo cách của firmware RV4G)
 *   - ACC tắt: dừng ngay. ACC bật: dừng -> chạy khi tốc độ > TRACKER_MOVING_SPEED_KMH liên tục
 *     TRACKER_MOVING_CONFIRM_SEC và đã dời điểm neo >= TRACKER_MOVING_MIN_DISPLACEMENT_M;
 *     chạy -> dừng khi tốc độ <= ngưỡng liên tục TRACKER_STOPPED_CONFIRM_SEC.
 *   - Khi dừng: tốc độ báo 0, hướng giữ nguyên, tọa độ = điểm neo (trung bình các mẫu tốt).
 *   - ACC tắt mà vị trí cách điểm neo >= TRACKER_PARKED_ACC_OFF_MAX_M (xe bị kéo/chở): làm lại điểm neo.
 * Mỗi mẫu GNSS (1 Hz) chỉ xét 1 lần, nhận biết qua chuỗi giờ GNSS. Chỉ task tracker đọc/ghi.
 * =========================================================================== */
static BOOL   motion_moving = FALSE;
static BOOL   motion_above_on = FALSE, motion_below_on = FALSE;
static UINT32 motion_above_tick = 0, motion_below_tick = 0;
static char   motion_last_time[sizeof(((SC_GNSSINFO *)0)->time)];
static double anchor_sum_lat = 0.0, anchor_sum_lon = 0.0;
static double anchor_lat = 0.0, anchor_lon = 0.0;
static int    anchor_n = 0;
static BOOL   anchor_good = FALSE;
static int    anchor_reject_run = 0;

/** Khoảng cách gần đúng (m) giữa 2 điểm, đủ chính xác cho vài km. */
static double tracker_distance_m(double lat1, double lon1, double lat2, double lon2)
{
    const double deg_to_m = 111320.0;                      /* 1 độ vĩ ≈ 111,32 km */
    double dy = (lat2 - lat1) * deg_to_m;
    double dx = (lon2 - lon1) * deg_to_m * cos((lat1 + lat2) * 0.5 * 3.14159265358979 / 180.0);
    return sqrt(dx * dx + dy * dy);
}

static void tracker_anchor_release(const char *why)
{
    if (anchor_n > 0) sAPI_Debug("[NASA] GNSS anchor: bo (%s, n=%d)", why, anchor_n);
    anchor_n = 0;
    anchor_reject_run = 0;
    anchor_good = FALSE;
}

/** Thêm 1 mẫu đứng yên vào điểm neo (bỏ mẫu HDOP cao / nhảy xa). */
static void tracker_anchor_add(double lat, double lon, double hdop)
{
    BOOL good = (hdop <= 0.0 || hdop <= TRACKER_ANCHOR_MAX_HDOP) ? TRUE : FALSE;   /* HDOP 0 = không biết */

    if (anchor_n == 0 || (!anchor_good && good)) {
        anchor_sum_lat = lat;
        anchor_sum_lon = lon;
        anchor_n = 1;
        anchor_good = good;
        anchor_reject_run = 0;
        sAPI_Debug("[NASA] GNSS anchor: bat dau %ld,%ld (x1e-6) hdop x10=%d",
                   (long)(lat * 1e6), (long)(lon * 1e6), (int)(hdop * 10.0));
    } else if (good && anchor_n < TRACKER_ANCHOR_MAX_SAMPLES) {
        double mlat = anchor_sum_lat / anchor_n, mlon = anchor_sum_lon / anchor_n;
        if (anchor_n >= TRACKER_ANCHOR_MIN_SAMPLES_FOR_REJECT &&
            tracker_distance_m(mlat, mlon, lat, lon) > TRACKER_ANCHOR_OUTLIER_M) {
            if (++anchor_reject_run >= TRACKER_ANCHOR_RESTART_REJECTS) {
                sAPI_Debug("[NASA] GNSS anchor: %d mau lech lien tiep -> lam lai (n=%d)", anchor_reject_run, anchor_n);
                anchor_sum_lat = lat;
                anchor_sum_lon = lon;
                anchor_n = 1;
                anchor_reject_run = 0;
            }
        } else {
            anchor_reject_run = 0;
            anchor_sum_lat += lat;
            anchor_sum_lon += lon;
            anchor_n++;
        }
    }
    anchor_lat = anchor_sum_lat / anchor_n;
    anchor_lon = anchor_sum_lon / anchor_n;
}

void tracker_motion_poll(void)
{
    SC_GNSSINFO *info = sAPI_GnssNavdataGet();
    tracker_location_t raw;
    double hdop;
    UINT32 now;
    BOOL acc;

    if (info == NULL) return;
    if (memcmp(motion_last_time, info->time, sizeof(motion_last_time)) == 0) return;   /* chưa có mẫu mới */
    memcpy(motion_last_time, info->time, sizeof(motion_last_time));
    if (!tracker_read_raw_location(&raw, &hdop) || !raw.fixValid) return;            /* chưa fix: giữ nguyên */

    now = sAPI_GetTicks();
    acc = tracker_get_acc_status();

    if (!acc) {
        if (motion_moving) sAPI_Debug("[NASA] GNSS: dung (ACC tat)");
        motion_moving = FALSE;
        motion_above_on = motion_below_on = FALSE;
        if (anchor_n > 0 &&
            tracker_distance_m(anchor_lat, anchor_lon, raw.lat, raw.lon) >= TRACKER_PARKED_ACC_OFF_MAX_M) {
            tracker_anchor_release("ACC tat nhung xe bi doi xa");
        }
    } else if (motion_moving) {
        if (raw.speed > TRACKER_MOVING_SPEED_KMH) {
            motion_below_on = FALSE;
        } else if (!motion_below_on) {
            motion_below_on = TRUE;
            motion_below_tick = now;
        } else if ((UINT32)(now - motion_below_tick) >= (UINT32)SEC_TO_TICKS(TRACKER_STOPPED_CONFIRM_SEC)) {
            motion_moving = FALSE;
            motion_below_on = FALSE;
            sAPI_Debug("[NASA] GNSS: dung (toc do <= %d km/h %ds)", (int)TRACKER_MOVING_SPEED_KMH, TRACKER_STOPPED_CONFIRM_SEC);
        }
    } else {
        if (raw.speed <= TRACKER_MOVING_SPEED_KMH) {
            motion_above_on = FALSE;
        } else if (!motion_above_on) {
            motion_above_on = TRUE;
            motion_above_tick = now;
        } else if ((UINT32)(now - motion_above_tick) >= (UINT32)SEC_TO_TICKS(TRACKER_MOVING_CONFIRM_SEC)) {
            double d = (anchor_n > 0) ? tracker_distance_m(anchor_lat, anchor_lon, raw.lat, raw.lon) : 0.0;
            if (anchor_n == 0 || d >= TRACKER_MOVING_MIN_DISPLACEMENT_M) {
                motion_moving = TRUE;
                motion_above_on = FALSE;
                sAPI_Debug("[NASA] GNSS: chay (%d km/h x10, cach diem neo %dm)", (int)(raw.speed * 10.0), (int)d);
            }
        }
    }

    if (motion_moving) tracker_anchor_release("xe chay");
    else               tracker_anchor_add(raw.lat, raw.lon, hdop);
}

BOOL tracker_motion_is_moving(void)
{
    return motion_moving;
}

void tracker_gnss_log_speed(void)
{
    SC_GNSSINFO *info = sAPI_GnssNavdataGet();
    char buf[sizeof(info->speed) + 1];

    if (info == NULL || !tracker_gnss_fix_valid(info)) return;
    memcpy(buf, info->speed, sizeof(info->speed));
    buf[sizeof(info->speed)] = '\0';
    sAPI_Debug("[NASA] GNSS speed raw=\"%s\" -> %d km/h x10 (he so x1000=%d), %s, anchor n=%d",
               buf, (int)(atof(buf) * TRACKER_GNSS_SPEED_TO_KMH * 10.0), (int)(TRACKER_GNSS_SPEED_TO_KMH * 1000.0),
               motion_moving ? "chay" : "dung", anchor_n);
}

/**
 * Đọc vị trí hiện tại đã lọc chạy/dừng: khi dừng thì tốc độ 0 và tọa độ là điểm neo.
 * Số vệ tinh luôn được cập nhật; tọa độ/tốc độ/độ cao chỉ có khi đã fix (không thì 0).
 * @return FALSE nếu SDK không trả dữ liệu.
 */
BOOL tracker_get_location(tracker_location_t *out)
{
    if (out == NULL) return FALSE;
    if (!tracker_read_raw_location(out, NULL)) return FALSE;

    if (out->fixValid) {
        if (!motion_moving) {
            out->speed = 0.0;
            if (anchor_n > 0) {
                out->lat = anchor_lat;
                out->lon = anchor_lon;
            }
        }
        last_fix_lat   = out->lat;                         /* nhớ vị trí cho lệnh 27 */
        last_fix_lon   = out->lon;
        last_fix_valid = TRUE;
    }
    return TRUE;
}

/**
 * Đọc hướng chạy (độ, [0, 360)) từ trường course của SC_GNSSINFO.
 * Copy chuỗi ra bộ đệm riêng trước khi parse vì driver cập nhật struct đồng thời.
 * Đơn vị của course: CHƯA XÁC MINH trong tài liệu SDK (giả định là độ, như NMEA RMC).
 * @return FALSE nếu chưa fix, chuỗi rỗng/không phải số hoặc giá trị ngoài [0, 360).
 */
BOOL tracker_get_course(double *deg)
{
    SC_GNSSINFO *info = sAPI_GnssNavdataGet();
    char buf[sizeof(info->course) + 1];
    char *end = NULL;
    double v;

    if (deg == NULL || !tracker_gnss_fix_valid(info)) return FALSE;
    memcpy(buf, info->course, sizeof(info->course));
    buf[sizeof(info->course)] = '\0';

    v = strtod(buf, &end);
    if (end == buf || v < 0.0 || v >= 360.0) return FALSE;
    *deg = v;
    return TRUE;
}

/**
 * Theo dõi trạng thái GPS theo thời gian thực, độc lập với chu kỳ gửi bản tin.
 * Chỉ đọc cờ fixValid (không parse tọa độ nên rất nhẹ), được gọi mỗi vòng lặp ~100 ms:
 *   - Cập nhật ngay LED GNSS (sáng = có fix, nháy nhanh = chưa fix).
 *   - Ghi log khi trạng thái đổi (mất fix / có fix lại).
 * Nhờ vậy độ trễ ghi nhận GPS không còn phụ thuộc chu kỳ 10 s (ACC bật) hay 300 s (ACC tắt).
 * Lưu ý: bản tin NASA-2 vẫn chỉ gửi theo chu kỳ, bit GPS trong bản tin lấy mẫu lúc gửi.
 * @param last_fix [in/out] trạng thái fix ở lần gọi trước.
 */
void tracker_monitor_gnss(BOOL *last_fix)
{
    SC_GNSSINFO *info = sAPI_GnssNavdataGet();
    BOOL fix = tracker_gnss_fix_valid(info);   /* không có dữ liệu / dữ liệu cũ = coi như chưa fix */

    if (fix != *last_fix) {
        sAPI_Debug("[NASA] GPS fix %s", fix ? "acquired" : "lost");
        *last_fix = fix;
    }
    led_gnss_mode = fix ? LED_MODE_ON : LED_MODE_BLINK_FAST;
}

/* ===========================================================================
 * 9b. ĐIỆN ÁP NGUỒN XE (ADC)
 * =========================================================================== */

/**
 * Đọc chân ADC một lần bằng sAPI_ReadAdc() (trả về mV, kiểu unsigned).
 * @param adc_mv [out] điện áp tại chân ADC (mV) = giá trị hàm trả về * TRACKER_ADC_TO_MV.
 * @return FALSE nếu giá trị vượt dải chân ADC (đọc lỗi/giá trị bất thường như 0xFFFFFFFF).
 */
static BOOL tracker_read_adc_mv(double *adc_mv)
{
    unsigned int raw;
    double mv;

    if (adc_mv == NULL) return FALSE;
    raw = (unsigned int)TRACKER_ADC_READ(TRACKER_ADC_CHANNEL);
    mv  = (double)raw * TRACKER_ADC_TO_MV;

    if (mv > TRACKER_ADC_MAX_MV) {
        sAPI_Debug("[NASA] ADC invalid raw=%u", raw);
        return FALSE;
    }
    *adc_mv = mv;
    return TRUE;
}

/**
 * Đo điện áp nguồn xe (V): đọc TRACKER_VOLT_SAMPLES lần (cách nhau 10 ms), lấy trung bình,
 * quy đổi theo cầu chia áp. Nếu mọi lần đọc đều lỗi thì trả về giá trị hợp lệ gần nhất
 * (0.0 nếu chưa từng đo được, khi đó bit BATTERY_OK cũng bị xóa để server biết dữ liệu chưa tin cậy).
 */
double tracker_get_voltage(void)
{
    static double last_good_v = 0.0;
    double mv, sum_mv = 0.0;
    int i, n = 0;

    for (i = 0; i < TRACKER_VOLT_SAMPLES; i++) {
        if (tracker_read_adc_mv(&mv)) { sum_mv += mv; n++; }
        if (i + 1 < TRACKER_VOLT_SAMPLES) sAPI_TaskSleep(TRACKER_VOLT_SAMPLE_GAP_TICKS);
    }
    if (n == 0) {
        sAPI_Debug("[NASA] ADC read failed, keep last VIN=%d mV", (int)(last_good_v * 1000.0));
        return last_good_v;
    }

    last_good_v = (sum_mv / n / 1000.0) * TRACKER_VOLT_RATIO + TRACKER_VOLT_OFFSET_V;
    sAPI_Debug("[NASA] ADC avg=%d mV (n=%d) -> VIN=%d mV", (int)(sum_mv / n), n, (int)(last_good_v * 1000.0));
    return last_good_v;
}
