/**
 * ******************************************************************************
 * @file    sc_application.c
 * @author  SIMCom OpenSDK Team / NASA-4G Protocol v1.3 Implementation
 * @brief   Ứng dụng giám sát hành trình NASA-4G chạy trên SIMCom OpenSDK
 * ******************************************************************************
 *
 * CHỨC NĂNG
 *   1. Đọc vị trí GNSS (GPS/BDS/QZSS), trạng thái ACC (khóa xe), cường độ sóng,
 *      điện áp nguồn xe (đo thật qua ADC, quy đổi theo cầu chia áp).
 *   2. Đăng nhập server bằng bản tin NASA-1, sau đó gửi hành trình NASA-2:
 *        - ACC bật : 10 giây/lần      - ACC tắt : 300 giây/lần
 *        - Gửi ngay khi ACC đổi trạng thái.
 *   3. Mất kết nối: lưu tạm tối đa 100 bản ghi vào RAM; khi có mạng lại thì
 *      gửi bù bằng bản tin NASA-7.
 *   4. Tự phục hồi: reset RF khi mất SIM/mạng, tự kết nối lại TCP.
 *   5. Đồng bộ giờ theo thứ tự ưu tiên: GNSS -> RTC module -> mạng di động -> NTP.
 *   6. Ba đèn LED báo trạng thái: nguồn (PWR), mạng (NET), GNSS.
 *
 * KIẾN TRÚC (2 task chạy song song)
 *   userSpace_Main
 *     |-- sTask_LedControl   : nháy LED theo biến led_net_mode / led_gnss_mode
 *     `-- sTask_NasaTracker  : logic chính, chạy vòng lặp vô hạn:
 *            tracker_wait_network()   chờ SIM + mạng (reset RF nếu treo)
 *            tracker_activate_pdp()   kích hoạt kết nối dữ liệu
 *            tracker_tcp_connect()    mở socket tới server
 *            tracker_login()          gửi NASA-1, chờ ACK
 *            tracker_run_session()    gửi NASA-2 / xả backup NASA-7
 *
 * ĐỊNH DẠNG BẢN TIN
 *   !NASA,<loại>,<msg_id>,<dữ liệu...>*<checksum>\r\n
 *   checksum = tổng giá trị các byte của phần đứng trước dấu '*'.
 *     NASA-1 : đăng nhập (giờ, IMEI, ICCID, biển số, phiên bản, mã, lý do reset)
 *     NASA-2 : hành trình realtime (giờ, tọa độ, tốc độ, trạng thái, ...)
 *     NASA-7 : hành trình gửi bù từ backup (cùng cấu trúc NASA-2)
 *
 * QUY ƯỚC THỜI GIAN CỦA SDK
 *   sAPI_TaskSleep(200) = 1 giây, sAPI_TaskSleep(20) = 100 ms.
 * ******************************************************************************
 */

/* ===========================================================================
 * 1. INCLUDE VÀ KHUNG SDK
 * =========================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "userspaceConfig.h"
#include "api_map.h"
#include "simcom_debug.h"
#include "simcom_os.h"

/* Thư viện cho tracker: GPS, TCP, SIM, AT, GPIO */
#include "simcom_gps.h"
#include "scfw_inet.h"
#include "scfw_netdb.h"
#include "scfw_socket.h"
#include "simcom_common.h"
#include "simcom_tcpip.h"
#include "simcom_tcpip_old.h"
#include "simcom_network.h"
#include "simcom_simcard.h"
#include "simcom_uart.h"
#include "simcom_gpio.h"
#include "sc_power.h"      /* sAPI_ReadAdc() */

#define PROJECT_TRACKER

/* Header/khai báo cho các cấu hình build khác của SDK (không dùng bởi tracker) */
#ifndef FALCON_1803_PLATFORM
  #ifdef HAS_DEMO
    #include "app_version.h"
  #endif
  #ifdef PROJECT_TBDTU
    #include "dtu_version.h"
  #endif
#endif

#ifdef HAS_DEMO
  #ifdef HAS_UART
    #include "uart_api.h"
  #endif
  extern void sAPP_SimcomUIDemo(void);
  extern void sAPP_HelloWorldDemo(void);
  extern void sAPP_UrcTask(void);
  extern void sc_Set_URCtask_flag(UINT8 value);
  extern void sAPP_UsbVcomTask(void);
#endif

#ifdef PROJECT_YSZNBG
  #include "ezhal.h"
  extern void sAPP_UrcTask(void);
#endif

#define TICKS_PER_SEC    200                       /* 1 giây = 200 tick */
#define SEC_TO_TICKS(s)  ((s) * TICKS_PER_SEC)
#define sleep(x)         sAPI_TaskSleep(SEC_TO_TICKS(x))

#ifdef PROJECT_TRACKER

/* ===========================================================================
 * 2. CẤU HÌNH (server, chu kỳ, thiết bị, phần cứng, bit trạng thái)
 * =========================================================================== */

/* --- Server và mạng --- */
#define TRACKER_SERVER_HOST              "103.57.209.16"
#define TRACKER_SERVER_PORT              2590
#define TRACKER_PDP_ID                   1

/* --- Chu kỳ gửi hành trình (giây) --- */
#define TRACKER_RUN_INTERVAL_SEC         10       /* ACC bật  */
#define TRACKER_STOP_INTERVAL_SEC        300      /* ACC tắt  */

/* --- Thời gian chờ / thử lại --- */
#define TRACKER_PDP_RETRY_SEC            5        /* thử lại PDP */
#define TRACKER_TCP_RETRY_SEC            5        /* thử lại TCP / phiên */
#define TRACKER_LOGIN_ACK_TIMEOUT_SEC    15       /* chờ server trả ACK login */
#define TRACKER_RX_POLL_MS               200      /* bước select khi chờ ACK */
#define TRACKER_RF_RESET_RETRIES         15       /* số lần kiểm tra mạng lỗi trước khi reset RF */
#define TRACKER_BACKUP_INTERVAL_SEC      20       /* chu kỳ lưu backup khi mất kết nối */

/* --- Kích thước bộ đệm --- */
#define TRACKER_LOGIN_BUF_SIZE           256
#define TRACKER_JOURNEY_BUF_SIZE         384
#define TRACKER_RX_BUF_SIZE              512

/* --- Thời gian hệ thống --- */
#define TRACKER_DATETIME_WAIT_TIMEOUT_MS     90000  /* tối đa chờ có giờ hợp lệ để login */
#define TRACKER_DATETIME_NETWORK_FALLBACK_MS 20000  /* sau ngần này mà chưa có giờ -> đồng bộ NTP */
#define TRACKER_RTC_SYNC_INTERVAL_SEC        300    /* chu kỳ ghi giờ GNSS vào RTC */
#define TRACKER_MIN_VALID_YEAR               2020   /* năm nhỏ hơn -> coi là giờ chưa hợp lệ */
#define TRACKER_UTC_OFFSET_HOURS             7      /* giờ Việt Nam = UTC+7 */
#define TRACKER_NTP_TZ_HOURS                 0      /* múi giờ áp cho kết quả NTP (0 = UTC) */
#define NTP_UNIX_OFFSET                      2208988800UL /* giây từ 1900 (NTP) đến 1970 (Unix) */

/* --- Nhận diện thiết bị (gửi trong NASA-1) --- */
#define TRACKER_DEVICE_IMEI_PLACEHOLDER  "000000000000000"  /* dùng khi đọc IMEI/ICCID lỗi */
#define TRACKER_PLATE_NUMBER             "51A-12345"
#define TRACKER_HW_VERSION               "v1.0"
#define TRACKER_FW_VERSION               "v1.0.260731"
#define TRACKER_LOGIN_CODE               "007507"
#define TRACKER_RESET_REASON             "R-0.0"

/* --- Điện áp nguồn xe: đo thật qua ADC của module ---
 * API của SDK A7677S: `unsigned int sAPI_ReadAdc(int channel)` (khai báo trong sc_power.h),
 * trả về điện áp chân ADC theo mV (demo_pm.c in ra "ADC%d VOL = %dmv"). SDK có 2 kênh ADC:
 * kênh 1 và kênh 2 (demo gọi sAPI_ReadAdc(1) và sAPI_ReadAdc(2), không có kênh 0).
 * >>> Chọn TRACKER_ADC_CHANNEL theo chân module mà net ADC.PWR đang nối vào (ADC1 -> 1, ADC2 -> 2) <<<
 * Chân ADC đa dụng: dải 0~1.8 V, 9 bit (~3.5 mV/bước).
 *
 * Mạch (theo schematic ADC.PWR): POWER -> R306 220K -> nút ADC -> R307 10K -> GND
 * (C306 100nF lọc nhiễu song song R307; R308 0R nối nút vào ADC.PWR; R309 NC nên
 * nhánh INPUT.SPEED không ảnh hưởng). Hệ số chia = (220K + 10K) / 10K = 23:
 *     V_xe = V_adc * (R_TOP + R_BOTTOM) / R_BOTTOM + OFFSET
 * Nên hiệu chỉnh OFFSET bằng cách so sánh với đồng hồ đo điện áp. */
#define TRACKER_ADC_CHANNEL              1              /* 1 = ADC1, 2 = ADC2 */
#ifndef TRACKER_ADC_READ
#define TRACKER_ADC_READ(ch)             sAPI_ReadAdc(ch)
#endif
#define TRACKER_ADC_TO_MV                1.0            /* hệ số đổi giá trị hàm trả về sang mV (giả định hàm trả mV) */
#define TRACKER_ADC_MAX_MV               1900.0         /* trên ngưỡng này (chân ADC tối đa 1.8 V) coi là đọc sai */
#define TRACKER_VOLT_R_TOP_OHM           220000.0       /* R306 */
#define TRACKER_VOLT_R_BOTTOM_OHM        10000.0        /* R307 */
#define TRACKER_VOLT_RATIO               ((TRACKER_VOLT_R_TOP_OHM + TRACKER_VOLT_R_BOTTOM_OHM) / TRACKER_VOLT_R_BOTTOM_OHM)
#define TRACKER_VOLT_OFFSET_V            0.0            /* hiệu chỉnh cộng thêm (V) */
#define TRACKER_VOLT_SAMPLES             8              /* số lần đọc lấy trung bình mỗi lần đo */
#define TRACKER_VOLT_SAMPLE_GAP_TICKS    2              /* nghỉ giữa các lần đọc (2 tick = 10 ms) */
#define TRACKER_BATTERY_OK_MIN_V         11.0           /* dưới ngưỡng này: xóa bit BATTERY_OK */

/* --- Giá trị cảm biến cố định (chưa có phần cứng đo thật) --- */
#define TRACKER_FUEL_RS232               0.0
#define TRACKER_FUEL_AF                  0.0
#define TRACKER_TEMPERATURE              0.0

/* --- Phần cứng GPIO theo sơ đồ RV26 --- */
#define TRACKER_LED_NET_PIN              32
#define TRACKER_LED_PWR_PIN              80
#define TRACKER_LED_GNSS_PIN             74
#define TRACKER_DOUT_PIN                 73       /* ngõ ra số (chưa sử dụng) */
#define TRACKER_ACC_GPIO_PIN             37       /* ACC: mức thấp = bật */
#define LED_LEVEL_ON                     1
#define LED_LEVEL_OFF                    0

/* --- Lệnh AT: kênh và thời gian chờ (đơn vị theo SDK, giữ nguyên như bản gốc) --- */
#define TRACKER_AT_CHANNEL               10
#define TRACKER_AT_TMO_SHORT             150      /* lệnh thông thường */
#define TRACKER_AT_TMO_REG               300      /* CPIN / CEREG / CGREG */
#define TRACKER_AT_TMO_CFUN              500      /* CFUN reset RF */
#define TRACKER_AT_TMO_CCLK_SET          900      /* ghi RTC */

/* --- Loại bản tin NASA --- */
#define NASA_MSG_LOGIN                   1
#define NASA_MSG_JOURNEY                 2
#define NASA_MSG_BACKUP                  7

/* --- Bit trạng thái NASA (trường "status" của NASA-2/7) ---
 * Hiện chỉ dùng GPS, ACC, BATTERY_OK; các bit còn lại để dành cho mở rộng. */
#define TRACKER_STATUS_BIT_GPS           (1UL << 0)
#define TRACKER_STATUS_BIT_FLASH_OK      (1UL << 1)
#define TRACKER_STATUS_BIT_ACC           (1UL << 2)
#define TRACKER_STATUS_BIT_DOOR          (1UL << 3)
#define TRACKER_STATUS_BIT_AC            (1UL << 4)
#define TRACKER_STATUS_BIT_DRIVER_LOGIN  (1UL << 5)
#define TRACKER_STATUS_BIT_OVERSPEED     (1UL << 6)
#define TRACKER_STATUS_BIT_LXLT          (1UL << 7)
#define TRACKER_STATUS_BIT_BATTERY_OK    (1UL << 8)
#define TRACKER_STATUS_BIT_FUEL_RS232    (1UL << 9)
#define TRACKER_STATUS_BIT_FUEL_PULSE    (1UL << 10)
#define TRACKER_STATUS_BIT_TEMP_SENSOR   (1UL << 11)
#define TRACKER_STATUS_BIT_SOS           (1UL << 12)
#define TRACKER_STATUS_BIT_BEN           (1UL << 13)

/* ===========================================================================
 * 3. KIỂU DỮ LIỆU VÀ BIẾN TOÀN CỤC
 * =========================================================================== */

/* Vị trí đọc từ GNSS */
typedef struct {
    double lat, lon;        /* độ thập phân (Nam/Tây là số âm) */
    double speed;           /* tốc độ (km/h) */
    double altitude;        /* độ cao (m) */
    int    satellites;      /* số vệ tinh nhìn thấy */
    BOOL   fixValid;        /* TRUE khi đã bắt được sóng */
} tracker_location_t;

/* Thời gian dạng tách trường */
typedef struct { int year, mon, day, hour, min, sec; } tracker_time_t;

/* Một bản ghi backup lưu trong RAM */
typedef struct {
    char               datetime[32];  /* "YYYY-MM-DD hh:mm:ss" tại thời điểm ghi */
    tracker_location_t loc;
    BOOL               acc_on;
    double             voltage;       /* điện áp xe (V) tại thời điểm ghi */
} backup_record_t;

/* Trạng thái phiên làm việc, truyền giữa các hàm của task chính */
typedef struct {
    int      sockfd;        /* socket TCP, -1 khi chưa kết nối */
    uint32_t msg_id;        /* số thứ tự bản tin */
    int      odometer_km;   /* quãng đường (hiện luôn = 0, chưa tính) */
    double   heading;       /* hướng chạy (hiện luôn = 0, chưa tính) */
} tracker_ctx_t;

/* Chế độ đèn LED */
typedef enum {
    LED_MODE_OFF = 0,       /* tắt */
    LED_MODE_ON,            /* sáng liên tục */
    LED_MODE_BLINK_FAST,    /* nháy nhanh (5 Hz) */
    LED_MODE_BLINK_SLOW     /* nháy chậm (1 Hz) */
} tracker_led_mode_t;

/* Hàng đợi backup: bộ đệm vòng, đầy thì ghi đè bản cũ nhất */
#define BACKUP_QUEUE_MAX 100
static backup_record_t backup_queue[BACKUP_QUEUE_MAX];
static int backup_head = 0, backup_tail = 0, backup_count = 0;
static uint32_t last_backup_time = 0;   /* giây trong ngày của lần backup gần nhất */

/* Trạng thái đăng nhập */
static BOOL is_logged_in       = FALSE; /* đang có phiên hợp lệ */
static BOOL has_logged_in_once = FALSE; /* đã từng đăng nhập (điều kiện để bắt đầu backup) */

/* Chế độ LED do task chính ghi, task LED đọc (volatile vì dùng chung giữa 2 task) */
static volatile tracker_led_mode_t led_net_mode  = LED_MODE_OFF;
static volatile tracker_led_mode_t led_gnss_mode = LED_MODE_OFF;

/* ===========================================================================
 * 4. TIỆN ÍCH CHUNG (GPIO, AT, checksum, tọa độ)
 * =========================================================================== */

/** Cấu hình một chân GPIO (pull-up, không ngắt). @param dir SC_GPIO_IN_PIN / SC_GPIO_OUT_PIN */
static SC_GPIOReturnCode tracker_gpio_setup(int pin, int dir)
{
    SC_GPIOConfiguration cfg;
    cfg.pinDir  = dir;
    cfg.pinPull = SC_GPIO_PULLUP_ENABLE;
    cfg.pinEd   = SC_GPIO_NO_EDGE;
    cfg.isr     = NULL;
    return sAPI_GpioConfig(pin, cfg);
}

/**
 * Gửi lệnh AT và chờ phản hồi.
 * @param resp, size  Bộ đệm nhận phản hồi; resp = NULL nếu không cần đọc.
 * @return 0 nếu thành công.
 */
static int tracker_at(const char *cmd, int timeout, char *resp, size_t size)
{
    char tmp[128];
    if (resp == NULL) { resp = tmp; size = sizeof(tmp); }
    memset(resp, 0, size);
    return (int)sAPI_SendATCMDWaitResp(TRACKER_AT_CHANNEL, (char *)cmd, timeout,
                                       NULL, 1, NULL, resp, size);
}

/** Checksum NASA: tổng giá trị các byte. */
static uint32_t tracker_checksum(const uint8_t *buf, uint32_t len)
{
    uint32_t sum = 0;
    if (buf == NULL) return 0;
    while (len--) sum += *buf++;
    return sum;
}

/** Đổi tọa độ NMEA (ddmm.mmmm) sang độ thập phân; hướng S/W cho giá trị âm. */
static double tracker_nmea_to_decimal(const char *coord, const char *direction)
{
    double raw, decimal;
    int degrees;
    if (coord == NULL || coord[0] == '\0') return 0.0;
    raw     = atof(coord);
    degrees = (int)(raw / 100.0);
    decimal = degrees + (raw - degrees * 100.0) / 60.0;
    if (direction != NULL && (direction[0] == 'S' || direction[0] == 'W')) decimal = -decimal;
    return decimal;
}

/* ===========================================================================
 * 5. ĐÈN LED (task chạy độc lập, chu kỳ 100 ms)
 * =========================================================================== */

/** Xuất mức ra chân LED theo chế độ và bộ đếm tick (mỗi tick = 100 ms). */
static void tracker_led_apply(int pin, tracker_led_mode_t mode, int tick)
{
    int on;
    switch (mode) {
    case LED_MODE_ON:         on = 1;                break;
    case LED_MODE_BLINK_FAST: on = (tick % 2 == 0);  break;  /* đổi trạng thái mỗi 100 ms */
    case LED_MODE_BLINK_SLOW: on = (tick % 20 < 10); break;  /* sáng 1 s, tắt 1 s */
    default:                  on = 0;                break;
    }
    sAPI_GpioSetValue(pin, on ? LED_LEVEL_ON : LED_LEVEL_OFF);
}

/** Task điều khiển LED: PWR sáng cố định, NET/GNSS theo led_net_mode/led_gnss_mode. */
static void sTask_LedControl(void *arg)
{
    int tick_100ms = 0;
    (void)arg;

    tracker_gpio_setup(TRACKER_LED_PWR_PIN,  SC_GPIO_OUT_PIN);
    tracker_gpio_setup(TRACKER_LED_NET_PIN,  SC_GPIO_OUT_PIN);
    tracker_gpio_setup(TRACKER_LED_GNSS_PIN, SC_GPIO_OUT_PIN);
    sAPI_GpioSetValue(TRACKER_LED_PWR_PIN, LED_LEVEL_ON);

    while (1) {
        tick_100ms++;
        tracker_led_apply(TRACKER_LED_NET_PIN,  led_net_mode,  tick_100ms);
        tracker_led_apply(TRACKER_LED_GNSS_PIN, led_gnss_mode, tick_100ms);
        sAPI_TaskSleep(20);                               /* 100 ms */
    }
}

/* ===========================================================================
 * 6. HÀNG ĐỢI BACKUP (bộ đệm vòng trong RAM, mất khi reset thiết bị)
 * =========================================================================== */

/** Thêm một bản ghi vào hàng đợi; nếu đầy thì bỏ bản cũ nhất. */
static void tracker_backup_push(const char *dt, const tracker_location_t *loc, BOOL acc_on, double voltage)
{
    if (backup_count >= BACKUP_QUEUE_MAX) {
        backup_tail = (backup_tail + 1) % BACKUP_QUEUE_MAX;
        backup_count--;
    }
    snprintf(backup_queue[backup_head].datetime, sizeof(backup_queue[backup_head].datetime), "%s", dt);
    backup_queue[backup_head].loc    = *loc;
    backup_queue[backup_head].acc_on = acc_on;
    backup_queue[backup_head].voltage = voltage;
    backup_head = (backup_head + 1) % BACKUP_QUEUE_MAX;
    backup_count++;
}

/** Lấy bản ghi cũ nhất ra khỏi hàng đợi. @return FALSE nếu hàng đợi rỗng. */
static BOOL tracker_backup_pop(backup_record_t *out)
{
    if (backup_count == 0) return FALSE;
    *out = backup_queue[backup_tail];
    backup_tail = (backup_tail + 1) % BACKUP_QUEUE_MAX;
    backup_count--;
    return TRUE;
}

/* ===========================================================================
 * 7. NHẬN DIỆN THIẾT BỊ (ICCID của SIM, IMEI của module)
 * =========================================================================== */

/** Đọc ICCID (bỏ ký tự 'F' cuối nếu có); lỗi thì dùng giá trị placeholder. */
static void tracker_get_sim_id(char *out, size_t out_size)
{
    char iccid[32] = {0};
    size_t len;
    if (out == NULL || out_size == 0) return;

    if (sAPI_SysGetIccid(iccid) == 0 && iccid[0] != '\0') {
        len = strlen(iccid);
        if (iccid[len - 1] == 'F' || iccid[len - 1] == 'f') iccid[len - 1] = '\0';
        snprintf(out, out_size, "%s", iccid);
    } else {
        snprintf(out, out_size, "%s", TRACKER_DEVICE_IMEI_PLACEHOLDER);
    }
}

/** Đọc IMEI bằng AT+CGSN: lấy chuỗi số đầu tiên dài 14-16 ký tự; lỗi thì dùng placeholder. */
static void tracker_get_imei(char *out, size_t out_size)
{
    char resp[64] = {0}, digits[32];
    const char *p = resp;
    int n;
    if (out == NULL || out_size == 0) return;

    if (tracker_at("AT+CGSN\r", TRACKER_AT_TMO_SHORT, resp, sizeof(resp)) == 0) {
        while (*p != '\0') {
            if (*p < '0' || *p > '9') { p++; continue; }
            for (n = 0; *p >= '0' && *p <= '9' && n < (int)sizeof(digits) - 1; n++) digits[n] = *p++;
            digits[n] = '\0';
            if (n >= 14 && n <= 16) { snprintf(out, out_size, "%s", digits); return; }
        }
    }
    snprintf(out, out_size, "%s", TRACKER_DEVICE_IMEI_PLACEHOLDER);
}

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

/** Ghi giờ vào RTC của module (AT+CCLK). "+28" = múi giờ +7 giờ (đơn vị 15 phút). */
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
    if (info == NULL || !info->fixValid) return FALSE;
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

/** Đổi số ngày kể từ 1970-01-01 sang (năm, tháng, ngày) - thuật toán "civil from days" của H. Hinnant. */
static void tracker_civil_from_days(int64_t days_since_epoch, int *out_year, int *out_month, int *out_day)
{
    int64_t z   = days_since_epoch + 719468;
    int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    int64_t doe = z - era * 146097;
    int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    int64_t mp  = (5 * doy + 2) / 153;
    int64_t m   = mp + (mp < 10 ? 3 : -9);
    *out_year  = (int)(yoe + era * 400 + (m <= 2));
    *out_month = (int)m;
    *out_day   = (int)(doy - (153 * mp + 2) / 5 + 1);
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
 * LƯU Ý: TRACKER_NTP_TZ_HOURS = 0 nên kết quả là giờ UTC, nhưng tracker_set_rtc() ghi kèm
 * hậu tố +28 (UTC+7) -> có thể lệch 7 giờ. Giữ nguyên như bản gốc, cần kiểm tra thực tế.
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
static BOOL tracker_sync_network_time(void)
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
static void tracker_sync_rtc_from_gnss_if_due(int elapsed_sec_since_last_call)
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
static BOOL tracker_get_datetime(char *out, size_t out_size)
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
static uint32_t tracker_get_seconds_today(void)
{
    char dt[32];
    int year, month, day, hour, minute, second;
    if (!tracker_get_datetime(dt, sizeof(dt))) return 0;
    if (sscanf(dt, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) != 6) return 0;
    return (uint32_t)(second + minute * 60 + hour * 3600);
}

/* ===========================================================================
 * 9. ACC VÀ GNSS
 * =========================================================================== */

/** Cấu hình chân ACC là ngõ vào pull-up. */
static void tracker_acc_init(void)
{
    SC_GPIOReturnCode ret = tracker_gpio_setup(TRACKER_ACC_GPIO_PIN, SC_GPIO_IN_PIN);
    sAPI_Debug("[NASA] ACC GPIO Initialized on PIN %d (Pull-Up) - ret=%d", TRACKER_ACC_GPIO_PIN, ret);
}

/** @return TRUE nếu ACC bật (chân xuống mức thấp). */
static BOOL tracker_get_acc_status(void)
{
    return (sAPI_GpioGetValue(TRACKER_ACC_GPIO_PIN) == 0) ? TRUE : FALSE;
}

/** Callback trạng thái GNSS (SDK yêu cầu đăng ký, chưa xử lý gì). */
static void tracker_gnss_state_cb(SC_Gnss_State_Type event) { (void)event; }

/** Bật nguồn GNSS, chọn hệ GPS+BDS+QZSS, cập nhật 1 Hz, khởi động nóng. LED GNSS nháy nhanh cho đến khi có fix. */
static void tracker_gnss_init(void)
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

/**
 * Đọc vị trí hiện tại. Số vệ tinh luôn được cập nhật; tọa độ/tốc độ/độ cao chỉ lấy
 * khi đã fix, còn không thì để 0 (tránh dính tọa độ cũ trong bộ đệm).
 * @return FALSE nếu SDK không trả dữ liệu.
 */
static BOOL tracker_get_location(tracker_location_t *out)
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
 * Theo dõi trạng thái GPS theo thời gian thực, độc lập với chu kỳ gửi bản tin.
 * Chỉ đọc cờ fixValid (không parse tọa độ nên rất nhẹ), được gọi mỗi vòng lặp ~100 ms:
 *   - Cập nhật ngay LED GNSS (sáng = có fix, nháy nhanh = chưa fix).
 *   - Ghi log khi trạng thái đổi (mất fix / có fix lại).
 * Nhờ vậy độ trễ ghi nhận GPS không còn phụ thuộc chu kỳ 10 s (ACC bật) hay 300 s (ACC tắt).
 * Lưu ý: bản tin NASA-2 vẫn chỉ gửi theo chu kỳ, bit GPS trong bản tin lấy mẫu lúc gửi.
 * @param last_fix [in/out] trạng thái fix ở lần gọi trước.
 */
static void tracker_monitor_gnss(BOOL *last_fix)
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
static double tracker_get_voltage(void)
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

/* ===========================================================================
 * 10. SIM, MẠNG DI ĐỘNG VÀ TCP
 * =========================================================================== */

/** Đọc cường độ sóng CSQ (0-31, 99 = không xác định). */
static int tracker_get_csq(void)
{
    UINT8 csq = 99;
    return (sAPI_NetworkGetCsq(&csq) == SC_NET_SUCCESS) ? (int)csq : 99;
}

/** Kiểm tra đã đăng ký mạng: gửi lệnh query (CEREG/CGREG), stat = 1 (home) hoặc 5 (roaming) là OK. */
static BOOL tracker_is_registered(const char *query, const char *tag)
{
    char resp[128];
    int n, stat;
    const char *p;

    if (tracker_at(query, TRACKER_AT_TMO_REG, resp, sizeof(resp)) != 0) return FALSE;
    p = strstr(resp, tag);
    return (p != NULL && sscanf(p + strlen(tag), "%d,%d", &n, &stat) == 2 && (stat == 1 || stat == 5));
}

/** SIM đã READY và đã đăng ký mạng LTE (CEREG) hoặc 2G/3G (CGREG)? */
static BOOL tracker_check_sim_and_network(void)
{
    char resp[128];
    if (tracker_at("AT+CPIN?\r", TRACKER_AT_TMO_REG, resp, sizeof(resp)) != 0 ||
        strstr(resp, "READY") == NULL) return FALSE;

    return tracker_is_registered("AT+CEREG?\r", "+CEREG:") ||
           tracker_is_registered("AT+CGREG?\r", "+CGREG:");
}

/** Kích hoạt PDP context (kết nối dữ liệu). */
static BOOL tracker_pdp_active(void)
{
    return sAPI_TcpipPdpActive(TRACKER_PDP_ID, 1) == SC_TCPIP_SUCCESS;
}

/** Mở kết nối TCP tới server, thử lần lượt các địa chỉ phân giải được. @return fd hoặc -1. */
static int tracker_tcp_connect(void)
{
    struct addrinfo hints, *list = NULL, *rp;
    char portstr[8];
    int fd = -1;
    int pdp_id = TRACKER_PDP_ID;

    snprintf(portstr, sizeof(portstr), "%d", TRACKER_SERVER_PORT);
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family   = AF_INET;
    hints.ai_protocol = IPPROTO_TCP;
    if (getaddrinfo(TRACKER_SERVER_HOST, portstr, &hints, &list) != 0 || list == NULL) return -1;

    for (rp = list; rp != NULL; rp = rp->ai_next) {
        fd = socket(rp->ai_family, rp->ai_socktype, 0);
        if (fd < 0) continue;
#ifdef SO_BINDTODEVICE
        setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &pdp_id, sizeof(pdp_id));  /* ép đi qua PDP đã kích hoạt */
#else
        (void)pdp_id;
#endif
        if (connect(fd, rp->ai_addr, rp->ai_addrlen) == 0) break;
        close(fd);
        fd = -1;
    }
    freeaddrinfo(list);
    return fd;
}

/** Gửi đủ len byte (lặp nếu send() chỉ gửi một phần). @return len nếu thành công, -1 nếu lỗi. */
static int tracker_tcp_send_all(int fd, const char *data, int len)
{
    int sent = 0, ret;
    if (fd < 0 || data == NULL || len <= 0) return -1;

    while (sent < len) {
        ret = send(fd, data + sent, len - sent, 0);
        if (ret <= 0) return -1;
        sent += ret;
    }
    return sent;
}

/** Đóng socket và đặt *fd = -1. */
static void tracker_tcp_close(int *fd)
{
    if (fd != NULL && *fd >= 0) { close(*fd); *fd = -1; }
}

/* ===========================================================================
 * 11. ĐÓNG GÓI BẢN TIN NASA
 * =========================================================================== */

/** Thêm "*<checksum>\r\n" vào payload để có bản tin hoàn chỉnh. @return độ dài bản tin, <0 nếu lỗi. */
static int tracker_finish_msg(const char *payload, char *out, size_t out_size)
{
    return snprintf(out, out_size, "%s*%lu\r\n", payload,
                    (unsigned long)tracker_checksum((const uint8_t *)payload, (uint32_t)strlen(payload)));
}

/** Tạo bản tin đăng nhập NASA-1. @return -1 nếu chưa có giờ hợp lệ. */
static int tracker_build_login(char *out, size_t out_size, uint32_t msg_id)
{
    char datetime[32], imei[32], sim_id[32], payload[224];
    if (out == NULL || out_size == 0 || !tracker_get_datetime(datetime, sizeof(datetime))) return -1;

    tracker_get_imei(imei, sizeof(imei));
    tracker_get_sim_id(sim_id, sizeof(sim_id));

    snprintf(payload, sizeof(payload), "!NASA,%d,%lu,%s,%s,%s,%s,%s,%s,%s,%s",
             NASA_MSG_LOGIN, (unsigned long)msg_id, datetime, imei, sim_id,
             TRACKER_PLATE_NUMBER, TRACKER_HW_VERSION, TRACKER_FW_VERSION,
             TRACKER_LOGIN_CODE, TRACKER_RESET_REASON);
    return tracker_finish_msg(payload, out, out_size);
}

/** Tạo trường "status": bit GPS, ACC và BATTERY_OK (điện áp đo được >= TRACKER_BATTERY_OK_MIN_V). */
static int tracker_get_status(BOOL acc_on, BOOL gps_valid, double voltage)
{
    uint32_t status = 0;
    if (gps_valid) status |= TRACKER_STATUS_BIT_GPS;
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

/** NASA-2: hành trình realtime, dùng giờ hiện tại. @return -1 nếu chưa có giờ hợp lệ. */
static int tracker_build_journey(char *out, size_t out_size, uint32_t msg_id,
                                 const tracker_location_t *loc, BOOL acc_on, double voltage,
                                 int odometer_km, double heading)
{
    char datetime[32];
    if (out == NULL || loc == NULL || out_size == 0 || !tracker_get_datetime(datetime, sizeof(datetime))) return -1;
    return tracker_build_position(out, out_size, NASA_MSG_JOURNEY, msg_id, datetime, loc, acc_on, voltage, odometer_km, heading);
}

/** NASA-7: gửi bù một bản ghi backup, giữ nguyên giờ lúc ghi. */
static int tracker_build_backup(char *out, size_t out_size, uint32_t msg_id,
                                const backup_record_t *rec, int odometer_km, double heading)
{
    return tracker_build_position(out, out_size, NASA_MSG_BACKUP, msg_id, rec->datetime,
                                  &rec->loc, rec->acc_on, rec->voltage, odometer_km, heading);
}

/* ===========================================================================
 * 12. BACKUP KHI MẤT KẾT NỐI VÀ CHỜ CÓ GIỜ
 * =========================================================================== */

/**
 * Lưu vị trí vào hàng đợi backup mỗi 20 giây khi đang mất kết nối.
 * Chỉ chạy khi: đã từng đăng nhập, hiện không có phiên, và đã có giờ hợp lệ.
 * Được gọi trong mọi vòng chờ (tracker_delay, chờ giờ, chờ ACK) để không bỏ sót dữ liệu.
 */
static void tracker_process_backup(void)
{
    tracker_location_t loc;
    char dt[32];
    uint32_t now, delta;

    if (is_logged_in || !has_logged_in_once) return;
    now = tracker_get_seconds_today();
    if (now == 0) return;

    /* Khoảng cách với lần backup trước (xử lý qua nửa đêm) */
    delta = (now >= last_backup_time) ? (now - last_backup_time) : (now + 86400 - last_backup_time);
    if (delta < TRACKER_BACKUP_INTERVAL_SEC && last_backup_time != 0) return;

    if (tracker_get_location(&loc) && tracker_get_datetime(dt, sizeof(dt))) {
        tracker_backup_push(dt, &loc, tracker_get_acc_status(), tracker_get_voltage());
        last_backup_time = now;
        sAPI_Debug("[NASA] Backup saved, queue=%d/%d", backup_count, BACKUP_QUEUE_MAX);
    }
}

/** Ngủ `seconds` giây, mỗi giây kiểm tra backup một lần (thay cho sleep thường). */
static void tracker_delay(int seconds)
{
    while (seconds-- > 0) {
        sAPI_TaskSleep(SEC_TO_TICKS(1));
        tracker_process_backup();
    }
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
static BOOL tracker_wait_login_ack(int fd)
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
static BOOL tracker_poll_server(int fd, int timeout_ms)
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

/* ===========================================================================
 * 14. TASK CHÍNH - CÁC BƯỚC CỦA MỘT CHU KỲ KẾT NỐI
 * =========================================================================== */

/** Ngắt phiên hiện tại: đóng socket, xóa cờ đăng nhập, LED NET nháy chậm. */
static void tracker_drop_session(tracker_ctx_t *c)
{
    tracker_tcp_close(&c->sockfd);
    is_logged_in = FALSE;
    led_net_mode = LED_MODE_BLINK_SLOW;
}

/** Bước 1: chờ SIM + mạng. Sau 15 lần kiểm tra lỗi thì reset RF (CFUN=0/1) để khôi phục SIM/mạng. */
static void tracker_wait_network(void)
{
    int retry = 0;
    if (tracker_check_sim_and_network()) return;

    sAPI_Debug("[NASA] Waiting for SIM & Cellular Network...");
    while (!tracker_check_sim_and_network()) {
        tracker_delay(2);
        if (++retry >= TRACKER_RF_RESET_RETRIES) {
            sAPI_Debug("[NASA] Resetting RF (CFUN=0/1) for Hot-Swap recovery...");
            tracker_at("AT+CFUN=0\r", TRACKER_AT_TMO_CFUN, NULL, 0);
            sAPI_TaskSleep(SEC_TO_TICKS(3));
            tracker_at("AT+CFUN=1\r", TRACKER_AT_TMO_CFUN, NULL, 0);
            sAPI_TaskSleep(SEC_TO_TICKS(5));
            retry = 0;
        }
    }
    sAPI_Debug("[NASA] Cellular network registered successfully!");
}

/** Bước 2: kích hoạt PDP, lặp cho đến khi được. @return FALSE nếu giữa chừng mất SIM/mạng. */
static BOOL tracker_activate_pdp(void)
{
    while (!tracker_pdp_active()) {
        sAPI_Debug("[NASA] PDP activation failed, retry...");
        tracker_delay(TRACKER_PDP_RETRY_SEC);
        if (!tracker_check_sim_and_network()) return FALSE;
    }
    return tracker_check_sim_and_network();
}

/**
 * Bước 4: đăng nhập server.
 *   - Chờ có giờ hợp lệ (tối đa 90 s; sau 20 s thử đồng bộ giờ mạng/NTP), rồi gửi NASA-1.
 *   - Chờ ACK; thành công thì bật cờ đăng nhập và LED NET sáng cố định.
 * @return TRUE nếu đăng nhập thành công; FALSE thì socket đã được đóng.
 */
static BOOL tracker_login(tracker_ctx_t *c)
{
    char msg[TRACKER_LOGIN_BUF_SIZE];
    int len = -1, wait_ms = 0;
    BOOL tried_network_sync = FALSE;

    is_logged_in = FALSE;

    while (wait_ms < TRACKER_DATETIME_WAIT_TIMEOUT_MS) {
        len = tracker_build_login(msg, sizeof(msg), c->msg_id);
        if (len > 0) break;                                  /* đã có giờ, gói login sẵn sàng */

        if (!tried_network_sync && wait_ms >= TRACKER_DATETIME_NETWORK_FALLBACK_MS) {
            tracker_sync_network_time();
            tried_network_sync = TRUE;
        }
        sAPI_TaskSleep(SEC_TO_TICKS(1));
        wait_ms += 1000;
        tracker_process_backup();
    }

    if (len > 0) {
        sAPI_Debug("[NASA] TX LOGIN: %s", msg);
        if (tracker_tcp_send_all(c->sockfd, msg, len) == len && tracker_wait_login_ack(c->sockfd)) {
            is_logged_in       = TRUE;
            has_logged_in_once = TRUE;
            led_net_mode       = LED_MODE_ON;
            sAPI_Debug("[NASA] LOGIN OK - Session Established");
            return TRUE;
        }
    }
    tracker_tcp_close(&c->sockfd);
    return FALSE;
}

/**
 * Bước 5: vòng lặp phiên làm việc (mỗi vòng 100 ms), chạy đến khi mất kết nối:
 *   1) Kiểm tra socket còn sống.
 *   2) Còn backup thì xả từng bản NASA-7 (ưu tiên trước dữ liệu realtime).
 *   1b) Mỗi vòng ~100 ms theo dõi GPS: cập nhật LED GNSS, log khi mất/có fix.
 *   3) Đến hạn (ACC bật 10 s / tắt 300 s) hoặc ACC đổi trạng thái: đọc GNSS,
 *      đồng bộ RTC, gửi NASA-2. Gửi lỗi thì lưu bản đó vào backup.
 */
static void tracker_run_session(tracker_ctx_t *c)
{
    char msg[TRACKER_JOURNEY_BUF_SIZE];
    tracker_location_t loc;
    backup_record_t rec;
    char dt[32];
    double voltage;                                        /* điện áp xe đo được ở lần gửi này */
    int len, interval_sec, tick_100ms = 0;
    BOOL send_now = TRUE;                                  /* gửi ngay bản đầu tiên sau login */
    BOOL last_acc = tracker_get_acc_status();
    SC_GNSSINFO *gi = sAPI_GnssNavdataGet();
    BOOL last_fix = (gi != NULL && gi->fixValid) ? TRUE : FALSE;   /* trạng thái GPS lúc bắt đầu phiên */

    while (c->sockfd >= 0 && is_logged_in) {

        /* (1) Phiên còn sống? */
        if (!tracker_poll_server(c->sockfd, 0)) {
            sAPI_Debug("[NASA] TCP session lost -> reconnect");
            tracker_drop_session(c);
            return;
        }

        /* (1b) Theo dõi GPS mỗi vòng (~100 ms): LED + log, không phụ thuộc chu kỳ gửi */
        tracker_monitor_gnss(&last_fix);

        /* (2) Xả kho backup: mỗi vòng gửi 1 bản NASA-7 */
        if (backup_count > 0) {
            if (tracker_backup_pop(&rec)) {
                len = tracker_build_backup(msg, sizeof(msg), c->msg_id, &rec, c->odometer_km, c->heading);
                if (len > 0) {
                    sAPI_Debug("[NASA] TX NASA-7: %s", msg);
                    if (tracker_tcp_send_all(c->sockfd, msg, len) != len) {
                        tracker_drop_session(c);
                        tracker_backup_push(rec.datetime, &rec.loc, rec.acc_on, rec.voltage);   /* trả lại hàng đợi */
                        return;
                    }
                    c->msg_id++;
                    sAPI_TaskSleep(20);                    /* nghỉ 100 ms giữa các bản */
                }
            }
            continue;
        }

        /* (3a) Đếm thời gian: đến hạn hoặc ACC đổi thì đặt cờ gửi.
         * Ghi log riêng khi ACC đổi để phân biệt với bản tin gửi do đến chu kỳ -
         * nếu chỉ nhìn bản tin NASA-2 thì hai trường hợp trông giống hệt nhau. */
        if (!send_now) {
            BOOL acc = tracker_get_acc_status();
            sAPI_TaskSleep(20);                            /* 100 ms */
            tick_100ms++;
            interval_sec = acc ? TRACKER_RUN_INTERVAL_SEC : TRACKER_STOP_INTERVAL_SEC;
            if (acc != last_acc) {
                sAPI_Debug("[NASA] ACC changed %d -> %d, sending immediately", last_acc, acc);
                send_now = TRUE;
            } else if (tick_100ms >= interval_sec * 10) {
                send_now = TRUE;
            }
        }
        if (!send_now) continue;

        /* (3b) Đọc GNSS một lần rồi gửi NASA-2 (LED GNSS đã được bước 1b cập nhật) */
        if (!tracker_get_location(&loc)) memset(&loc, 0, sizeof(loc));  /* lỗi đọc: gửi 0 để báo thiết bị còn sống */

        last_acc     = tracker_get_acc_status();
        interval_sec = last_acc ? TRACKER_RUN_INTERVAL_SEC : TRACKER_STOP_INTERVAL_SEC;
        if (loc.fixValid) tracker_sync_rtc_from_gnss_if_due(interval_sec);

        voltage = tracker_get_voltage();                   /* đo điện áp thật mỗi lần gửi */
        len = tracker_build_journey(msg, sizeof(msg), c->msg_id, &loc, last_acc, voltage, c->odometer_km, c->heading);
        if (len <= 0) {                                    /* chưa có giờ hợp lệ: thử lại ở vòng sau */
            sAPI_TaskSleep(20);
            continue;
        }

        sAPI_Debug("[NASA] TX NASA-2: %s", msg);
        if (tracker_tcp_send_all(c->sockfd, msg, len) != len) {
            if (tracker_get_datetime(dt, sizeof(dt))) tracker_backup_push(dt, &loc, last_acc, voltage);
            tracker_drop_session(c);
            return;
        }
        c->msg_id++;
        send_now   = FALSE;
        tick_100ms = 0;
    }
}

/**
 * Task chính: khởi tạo mạng + GNSS rồi lặp vô hạn
 *   chờ mạng -> PDP -> TCP -> login -> phiên làm việc -> (mất kết nối) đợi 5 s -> lặp lại.
 * LED NET: nháy nhanh = chưa có mạng, nháy chậm = có mạng nhưng chưa login, sáng = đã login.
 */
static void sTask_NasaTracker(void *arg)
{
    tracker_ctx_t ctx = { -1, 1, 0, 0.0 };                 /* sockfd, msg_id, odometer, heading */
    (void)arg;

    sAPI_TaskSleep(SEC_TO_TICKS(5));                       /* chờ hệ thống ổn định */
    sAPI_Debug("================================================");
    sAPI_Debug(" NASA-4G Tracker (Protocol v1.3) Started        ");
    sAPI_Debug("================================================");

    sAPI_NetworkInit();
    tracker_gnss_init();

    while (1) {
        led_net_mode = LED_MODE_BLINK_FAST;
        tracker_wait_network();
        led_net_mode = LED_MODE_BLINK_SLOW;

        if (!tracker_activate_pdp()) continue;             /* mất mạng giữa chừng: kiểm tra lại từ đầu */

        ctx.sockfd = tracker_tcp_connect();
        if (ctx.sockfd < 0) {
            is_logged_in = FALSE;
            tracker_delay(TRACKER_TCP_RETRY_SEC);
            continue;
        }

        if (tracker_login(&ctx)) tracker_run_session(&ctx);
        tracker_delay(TRACKER_TCP_RETRY_SEC);              /* nghỉ rồi kết nối lại (backup vẫn chạy) */
    }
}

#endif /* PROJECT_TRACKER */

/* ===========================================================================
 * 15. ĐIỂM VÀO CHƯƠNG TRÌNH
 * =========================================================================== */

/** Tạo task cho các dự án khác của SDK (stack tính bằng byte). */
#define SDK_CREATE_TASK(fn, stack, prio, name)                                   \
    do { extern void fn(void *arg); sTaskRef ref_ = NULL;                        \
         sAPI_TaskCreate(&ref_, NULL, (stack), (prio), (name), fn, NULL); } while (0)

void userSpace_Main(void *arg)
{
    ApiMapInit(arg);

#ifdef PROJECT_TRACKER
    tracker_acc_init();

    sTaskRef led_task_ref = NULL;
    sAPI_TaskCreate(&led_task_ref, NULL, 1024 * 4, 105, "NASA-LED Control", sTask_LedControl, NULL);

    sTaskRef tracker_task_ref = NULL;
    sAPI_TaskCreate(&tracker_task_ref, NULL, 1024 * 12, 100, "NASA-4G tracker", sTask_NasaTracker, NULL);
#endif

#ifdef HAS_DEMO
    sleep(5);
  #ifdef HAS_URC
    sAPP_UrcTask();
    sc_Set_URCtask_flag(1);
  #endif
  #ifdef HAS_USB
    sAPP_UsbVcomTask();
  #endif
    sAPP_HelloWorldDemo();
  #ifdef HAS_UART
    sDemo_UartInit();
    sAPP_SimcomUIDemo();
  #endif
#endif

#ifdef PROJECT_YSZNBG
    SDK_CREATE_TASK(ez_main, 1024 * 20, 120, "eziot main task");
    sAPP_UrcTask();
#endif
#ifdef PROJECT_TBDTU
    SDK_CREATE_TASK(dtu_Main, 1024 * 20, 120, "dtu main task");
#endif
#ifdef PROJECT_LYTBOX
    SDK_CREATE_TASK(lytbox_Main, 1024 * 20, 120, "lytbox main task");
#endif
#ifdef PROJECT_TOPFLY
    SDK_CREATE_TASK(topfly_Main, 1024 * 20, 120, "topfly main task");
#endif

    printf("user app is running...");
}

void abort(void)
{
    printf("abort!!!");
    while (1);
}

/* Đăng ký ứng dụng với SDK: đặt vào section .userSpaceRegTable */
#define _appRegTable_attr_ __attribute__((unused, section(".userSpaceRegTable")))
#define appMainStackSize   (1024 * 10)

#ifndef APP_VERSION
#define APP_VERSION ""
#endif
userSpaceEntry_t userSpaceEntry _appRegTable_attr_ =
    {NULL, NULL, appMainStackSize, 30, "userSpaceMain", userSpace_Main, APP_VERSION};