/**
 * @file    tracker_sensor.c
 * @brief   Cảm biến: ACC, GNSS (vị trí, hướng, theo dõi fix), điện áp nguồn xe (ADC).
 *          (Tách nguyên văn từ sc_application.c, không đổi logic.)
 */
#include "tracker_sensor.h"
#include "tracker_util.h"
#include "tracker_led.h"

/* ===========================================================================
 * 9. ACC VÀ GNSS
 * =========================================================================== */

/** Cấu hình chân ACC là ngõ vào pull-up. */
void tracker_acc_init(void)
{
    SC_GPIOReturnCode ret = tracker_gpio_setup(TRACKER_ACC_GPIO_PIN, SC_GPIO_IN_PIN);
    sAPI_Debug("[NASA] ACC GPIO Initialized on PIN %d (Pull-Up) - ret=%d", TRACKER_ACC_GPIO_PIN, ret);
}

/** @return TRUE nếu ACC bật (chân xuống mức thấp). */
BOOL tracker_get_acc_status(void)
{
    return (sAPI_GpioGetValue(TRACKER_ACC_GPIO_PIN) == 0) ? TRUE : FALSE;
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

/** Tắt nguồn GNSS (lệnh 38). AT+CVAUX giữ nguyên. Bật lại bằng tracker_gnss_init(). */
void tracker_gnss_power_off(void)
{
    SC_Gnss_Return_Code ret = sAPI_GnssPowerStatusSet(SC_GNSS_POWER_OFF);
    led_gnss_mode = LED_MODE_OFF;
    sAPI_Debug("[NASA] GNSS power off ret=%d", (int)ret);
}

/**
 * Đọc vị trí hiện tại. Số vệ tinh luôn được cập nhật; tọa độ/tốc độ/độ cao chỉ lấy
 * khi đã fix, còn không thì để 0 (tránh dính tọa độ cũ trong bộ đệm).
 * @return FALSE nếu SDK không trả dữ liệu.
 */
BOOL tracker_get_location(tracker_location_t *out)
{
    SC_GNSSINFO *info;
    if (out == NULL) return FALSE;

    memset(out, 0, sizeof(*out));
    info = sAPI_GnssNavdataGet();
    if (info == NULL) return FALSE;

    out->fixValid   = info->fixValid;
    out->satellites = atoi(info->valid_sate_num);
    if (out->fixValid) {
        out->lat      = tracker_nmea_to_decimal(info->lat, info->lat_ns);
        out->lon      = tracker_nmea_to_decimal(info->lon, info->lon_we);
        out->speed    = atof(info->speed);
        out->altitude = atof(info->alt);
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

    if (deg == NULL || info == NULL || !info->fixValid) return FALSE;
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
    BOOL fix = (info != NULL && info->fixValid) ? TRUE : FALSE;   /* không có dữ liệu = coi như chưa fix */

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
