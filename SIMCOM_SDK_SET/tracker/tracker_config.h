#ifndef TRACKER_CONFIG_H
#define TRACKER_CONFIG_H

/**
 * @file    tracker_config.h
 * @brief   Toàn bộ cấu hình của tracker (server, chu kỳ, thiết bị, phần cứng, bit trạng thái).
 *          (Tách nguyên văn từ sc_application.c, không đổi logic.)
 */

#define TICKS_PER_SEC    200                       /* 1 giây = 200 tick */
#define SEC_TO_TICKS(s)  ((s) * TICKS_PER_SEC)

/* ===========================================================================
 * 2. CẤU HÌNH (server, chu kỳ, thiết bị, phần cứng, bit trạng thái)
 * =========================================================================== */

/* --- Server và mạng --- */
#define TRACKER_SERVER_HOST              "tcp.rovigps.vn"   /* tên miền: phân giải bằng getaddrinfo() trong tracker_tcp_connect */
#define TRACKER_SERVER_PORT              2598
/* Lệnh 1 (na,1,<host>,<port>): server cấu hình lưu ở file (text "host,port"); không có file = mặc định ở trên.
 * Server cấu hình (khác mặc định) lỗi kết nối/đăng nhập liên tiếp TRACKER_SERVER_FALLBACK_FAILS lần
 * -> quay về server mặc định đến hết lần chạy này (file giữ nguyên, khởi động lại sẽ thử lại). */
#define TRACKER_SERVER_CFG_PATH          "C:/nasa_server.dat"
#define TRACKER_SERVER_HOST_MAX          63       /* độ dài tối đa tên miền/IP */
#define TRACKER_SERVER_FALLBACK_FAILS    10
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
/* Chu kỳ lưu backup khi mất kết nối (theo ACC; ACC đổi trạng thái thì lưu ngay) */
#define TRACKER_BACKUP_ACC_ON_SEC        20       /* ACC bật */
#define TRACKER_BACKUP_ACC_OFF_SEC       300      /* ACC tắt */

/* --- Lưu backup xuống flash khi RAM đầy ---
 * Dùng hệ thống file sẵn có của SDK (sAPI_fopen/fwrite/fread/fclose/remove), ghi theo LÔ
 * (toàn bộ hàng đợi RAM trong 1 lần mở file) chứ không ghi từng bản một, để giảm hao mòn
 * flash và không làm chậm vòng lặp 100ms. "C:/" là vùng flash nội bộ của module.
 * CẦN KIỂM TRA THỰC TẾ: nếu sAPI_fopen() trả NULL dù đường dẫn đúng, module có thể cần
 * mount file system trước (xem sAPI_NorFlashMountLfs trong demo_exfs.c của SDK). */
#define TRACKER_BACKUP_FLASH_PATH        "C:/nasa_backup.dat"
#define TRACKER_BACKUP_FLUSH_EVERY       10       /* RAM đủ số bản này thì ghi cả lô xuống flash: mất nguồn chỉ mất < 10 bản */

/* --- Trạng thái lưu bền qua khởi động lại (tracker_backup.c) ---
 * msg_id: dành trước một khối ID, chỉ ghi flash khi dùng hết khối -> không trùng ID sau reboot trong ngày.
 * Vị trí đọc file backup: ghi sau mỗi TRACKER_STATE_OFFSET_SAVE_EVERY bản NASA-7 lấy từ flash. */
#define TRACKER_STATE_PATH               "C:/nasa_state.dat"
#define TRACKER_MSG_ID_RESERVE_BLOCK     100      /* ~17 phút ở chu kỳ 10 s -> ~86 lần ghi/ngày */
#define TRACKER_STATE_OFFSET_SAVE_EVERY  10       /* sau reboot gửi lặp tối đa 9 bản NASA-7 */

/* --- Lệnh 38 tắt/mở thiết bị: lưu trạng thái qua khởi động lại (1 byte '0'/'1'; không có file = mở) --- */
#define TRACKER_DEVICE_STATE_PATH        "C:/nasa_power.dat"

/* --- Lệnh 58 chế độ ngủ (giai đoạn 1: ngủ ở mức ứng dụng, chưa dùng sAPI_SystemSleepSet) ---
 * na,58,xx,yy: ACC tắt liên tục xx giây -> ngủ; ngủ yy giây (hoặc ACC bật) -> thức. 0,0 = tắt chế độ ngủ.
 * Lưu "xx,yy"; không có file = tắt. */
#define TRACKER_SLEEP_CFG_PATH           "C:/nasa_sleep.dat"
#define TRACKER_SLEEP_AFTER_MIN_SEC      10
#define TRACKER_SLEEP_AFTER_MAX_SEC      65535
#define TRACKER_SLEEP_WAKE_MIN_SEC       60
#define TRACKER_SLEEP_WAKE_MAX_SEC       86400

/* --- Lệnh cấu hình qua SMS (tracker_sms.c) --- */
#define TRACKER_SMS_QUEUE_DEPTH          8        /* số URC/kết quả SMS chờ xử lý */
#define TRACKER_SMS_SEND_TIMEOUT_SEC     30       /* chờ kết quả gửi SMS phản hồi */
#define TRACKER_SMS_REPLY_MIN_SEC        60       /* cùng 1 số: tối đa 1 phản hồi / 60 s (chống tốn tiền) */
#define TRACKER_SMS_BODY_SIZE            200      /* độ dài tối đa nội dung lệnh nhận qua SMS */

/* --- Kích thước bộ đệm --- */
#define TRACKER_LOGIN_BUF_SIZE           256
#define TRACKER_JOURNEY_BUF_SIZE         384
#define TRACKER_RX_BUF_SIZE              512
#define TRACKER_CMD_LINE_IDLE_MS         300      /* lệnh TCP không có \r/\n: chạy sau 300 ms không nhận thêm byte */

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
#define TRACKER_FW_VERSION               "v1.0.261117c"
#define TRACKER_LOGIN_CODE               "007507"

/* --- Nguyên nhân reset (trường 11 của NASA-1, theo tài liệu giao thức v1.3) ---
 * Lần login đầu sau khởi động: R-1.0 nếu có file dấu watchdog, ngược lại R-2.0 (bật nguồn).
 * Các lần login sau trong cùng lần chạy (kết nối lại TCP): R-0.0.
 * File dấu chứa chuỗi lý do: "R-3.0" = reset mềm bằng lệnh 9, nội dung khác = watchdog. */
#define TRACKER_RESET_REASON_RECONNECT   "R-0.0"
#define TRACKER_RESET_REASON_WATCHDOG    "R-1.0"
#define TRACKER_RESET_REASON_POWER_ON    "R-2.0"
#define TRACKER_RESET_REASON_SOFT        "R-3.0"
#define TRACKER_RESET_FLAG_PATH          "C:/nasa_reset.dat"  /* có file = lần reset trước do watchdog / lệnh 9 */
#define TRACKER_CMD_RESET_DELAY_SEC      5        /* lệnh 9: reset sau khi phản hồi ngần này giây (bảng lệnh: 5-10 s) */

/* --- Lệnh 33: cập nhật firmware (customer_app.bin) từ xa bằng sAPI_AppDownload (sc_app_download.h) --- */
#define TRACKER_FOTA_URL_MAX             255      /* SC_FOTA_URL_MAX_SIZE = 256 gồm '\0' */
#define TRACKER_FOTA_TIMEOUT_SEC         600      /* quá thời gian này chưa xong -> reset mềm (chạy lại firmware cũ) */
#define TRACKER_FOTA_RECV_TIMEOUT_MS     20000    /* SCAppDownloadPram.recvtimeout (giá trị theo demo_app_download.c) */
#define TRACKER_FOTA_TASK_STACK          (1024 * 10)  /* như demo_app_download.c */
#define TRACKER_FOTA_TASK_PRIO           100

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

/* --- Hướng chạy (trường 12 của NASA-2) ---
 * Chỉ cập nhật khi xe chạy từ ngưỡng này trở lên: lúc đứng yên/chạy chậm hướng GNSS dao động mạnh,
 * khi đó giữ nguyên hướng gần nhất. */
#define TRACKER_HEADING_MIN_SPEED_KMH    3.0

/* --- Watchdog ---
 * WDT phần cứng (simcom_wtd.h) do task LED cho ăn mỗi 1 s, NHƯNG chỉ khi task tracker còn báo sống
 * (tracker_alive) trong TRACKER_WDT_STALL_SEC. Tracker treo quá ngưỡng -> ngừng cho ăn -> module tự reset.
 * Giá trị timeout 0x04 = 16 s theo comment trong sc_demo/src/demo_wtd.c; các giá trị khác CHƯA XÁC MINH.
 * Ngưỡng 300 s lớn hơn các lệnh chặn lâu nhất (connect() blocking, AT+CFUN, ...) để tránh reset nhầm. */
#define TRACKER_WDT_TIMEOUT_REG_VALUE    0x04     /* 16 s */
#define TRACKER_WDT_FEED_PERIOD_TICKS    10       /* 10 x 100 ms = cho ăn mỗi 1 s */
#define TRACKER_WDT_STALL_SEC            300      /* tracker không báo sống quá ngưỡng này -> để WDT reset */

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
#define NASA_MSG_SMS_REPORT              13       /* báo lên server lệnh nhận qua SMS + phản hồi (theo mẫu server) */

/* --- Bit trạng thái NASA (trường "status" của NASA-2/7) ---
 * Hiện chỉ dùng GPS, FLASH_OK, ACC, BATTERY_OK; các bit còn lại để dành cho mở rộng. */
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

#endif /* TRACKER_CONFIG_H */
