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
 *   3. Mất kết nối: lưu tạm tối đa 100 bản ghi vào RAM (~8.8 KB); RAM đầy thì tự
 *      động archive xuống file flash (C:/). Khi có mạng lại thì
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
 * TỔ CHỨC MÃ NGUỒN (thư mục tracker/)
 *   tracker_config.h    toàn bộ cấu hình TRACKER_* (server, chu kỳ, GPIO, ADC, watchdog, bit trạng thái)
 *   tracker_common.h    include SDK + kiểu dữ liệu dùng chung
 *   tracker_util.c      GPIO, lệnh AT, checksum, tọa độ NMEA
 *   tracker_led.c       task LED + watchdog (tracker_alive)
 *   tracker_backup.c    hàng đợi backup RAM/flash, tracker_delay
 *   tracker_device.c    IMEI, ICCID
 *   tracker_time.c      giờ GNSS / RTC / mạng / NTP
 *   tracker_sensor.c    ACC, GNSS, hướng, điện áp ADC
 *   tracker_net.c       SIM, mạng, PDP, TCP
 *   tracker_protocol.c  đóng gói NASA-1/2/7, msg_id, ACK, giám sát phiên
 *   sc_application.c    (file này) task tracker, vòng phiên làm việc, điểm vào userSpace_Main
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
#include "tracker_common.h"     /* include SDK + tracker_config.h + kiểu dữ liệu */
#include "tracker_util.h"
#include "tracker_led.h"
#include "tracker_backup.h"
#include "tracker_time.h"
#include "tracker_sensor.h"
#include "tracker_net.h"
#include "tracker_protocol.h"

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

#define sleep(x)         sAPI_TaskSleep(SEC_TO_TICKS(x))

#ifdef PROJECT_TRACKER

/* Trạng thái đăng nhập */
BOOL is_logged_in       = FALSE; /* đang có phiên hợp lệ */
BOOL has_logged_in_once = FALSE; /* đã từng đăng nhập (điều kiện để bắt đầu backup) */

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
        len = tracker_build_login(msg, sizeof(msg), c);
        if (len > 0) break;                                  /* đã có giờ, gói login sẵn sàng */

        if (!tried_network_sync && wait_ms >= TRACKER_DATETIME_NETWORK_FALLBACK_MS) {
            tracker_sync_network_time();
            tried_network_sync = TRUE;
        }
        sAPI_TaskSleep(SEC_TO_TICKS(1));
        tracker_alive();
        wait_ms += 1000;
        tracker_process_backup();
    }

    if (len > 0) {
        sAPI_Debug("[NASA] TX LOGIN: %s", msg);
        if (tracker_tcp_send_all(c->sockfd, msg, len) == len) {
            c->msg_id++;                                    /* login đã gửi: ID này đã dùng, không cấp lại */
            if (tracker_wait_login_ack(c->sockfd)) {
                is_logged_in       = TRUE;
                has_logged_in_once = TRUE;
                led_net_mode       = LED_MODE_ON;
                tracker_reset_reason_consumed();            /* các lần login sau trong lần chạy này: R-0.0 */
                sAPI_Debug("[NASA] LOGIN OK - Session Established");
                return TRUE;
            }
        }
    }
    tracker_tcp_close(&c->sockfd);
    return FALSE;
}

/**
 * Bước 5: vòng lặp phiên làm việc (mỗi vòng 100 ms), chạy đến khi mất kết nối.
 *
 * SONG SONG BACKUP + REALTIME:
 *   Mỗi vòng lặp thực hiện CẢ HAI bước, không bước nào chặn bước kia:
 *     (2) Nếu còn backup → gửi 1 bản NASA-7 rồi RƠI XUỐNG kiểm tra realtime.
 *     (3) Nếu đến chu kỳ (ACC ON 10 s / OFF 300 s) → gửi NASA-2 ngay,
 *         dù còn bao nhiêu bản NASA-7 đang chờ.
 *   Tick 100 ms tăng đều đầu mỗi vòng → chu kỳ realtime không bị trễ bởi backup.
 */
static void tracker_run_session(tracker_ctx_t *c)
{
    char msg[TRACKER_JOURNEY_BUF_SIZE];
    tracker_location_t loc;
    backup_record_t rec;
    char dt[32];
    char dt_next[32];                                      /* timestamp danh nghĩa của bản tin kế tiếp */
    double voltage;
    int len, interval_sec;
    int  elapsed_ticks = 0;                                /* số tick 100ms đã trôi qua kể từ lần gửi NASA-2 gần nhất */
    int  pending_ticks = 0;                                /* tích lũy tick từ blocking I/O để bù vào elapsed_ticks */
    int  send_count = 0;                                   /* đếm số bản tin NASA-2 đã gửi trong phiên */
    BOOL send_now = TRUE;
    BOOL last_acc = tracker_get_acc_status();
    SC_GNSSINFO *gi = sAPI_GnssNavdataGet();
    BOOL last_fix = (gi != NULL && gi->fixValid) ? TRUE : FALSE;

    dt_next[0] = '\0';                                    /* chưa có timestamp tham chiếu */

    while (c->sockfd >= 0 && is_logged_in) {

        /* Tick 100 ms — nghỉ đúng 100ms, sau đó cộng dồn pending_ticks
         * (thời gian blocking I/O từ vòng trước) vào elapsed_ticks.
         * Nhờ vậy chu kỳ realtime được đo bằng thời gian thực,
         * không bị sai lệch bởi tracker_tcp_send_all / tracker_flash_pop_one. */
        sAPI_TaskSleep(20);                                /* 100 ms */
        tracker_alive();
        elapsed_ticks += 1 + pending_ticks;                /* 1 tick (100ms sleep) + bù blocking */
        pending_ticks = 0;

        /* (1) Phiên còn sống? */
        if (!tracker_poll_server(c->sockfd, 0)) {
            sAPI_Debug("[NASA] TCP session lost -> reconnect");
            tracker_drop_session(c);
            return;
        }

        /* (1b) Theo dõi GPS mỗi vòng (~100 ms): LED + log, không phụ thuộc chu kỳ gửi */
        tracker_monitor_gnss(&last_fix);

        /* (2) Gửi 1 bản NASA-7 nếu còn backup (flash trước vì cũ hơn, RAM sau).
         * KHÔNG dùng continue → vòng lặp vẫn rơi xuống kiểm tra NASA-2 cùng vòng này.
         * Nhờ vậy realtime luôn gửi đúng chu kỳ, NASA-7 được xả xen kẽ từng bản 1.
         *
         * tracker_flash_pop_one / tracker_tcp_send_all là blocking I/O (~100-300ms).
         * Sau khi gửi xong, cộng thêm 2 tick (~200ms) vào pending_ticks để bù vào
         * elapsed_ticks ở đầu vòng sau, tránh đánh giá thấp thời gian đã trôi qua. */
        if (flash_has_data || backup_count > 0) {
            BOOL got;
            if (flash_has_data && tracker_flash_has_backup()) {
                got = tracker_flash_pop_one(&rec);         /* ưu tiên flash: cũ hơn → gửi trước */
                pending_ticks += 2;                        /* bù ~200ms mở/đọc file flash */
            } else {
                got = tracker_backup_pop(&rec);
            }
            if (got) {
                char dt_now[32];
                /* ID theo ngày GỬI (giờ hiện tại), không theo giờ lưu trong bản ghi backup */
                if (tracker_get_datetime(dt_now, sizeof(dt_now))) tracker_msg_id_check_new_day(c, dt_now);
                /* Bản ghi backup chưa lưu hướng → gửi 0, không gắn hướng hiện tại vào vị trí cũ */
                len = tracker_build_backup(msg, sizeof(msg), c->msg_id, &rec, c->odometer_km, 0.0);
                if (len > 0) {
                    sAPI_Debug("[NASA] TX NASA-7: %s", msg);
                    if (tracker_tcp_send_all(c->sockfd, msg, len) != len) {
                        tracker_drop_session(c);
                        tracker_backup_push(rec.datetime, &rec.loc, rec.acc_on, rec.voltage);
                        return;
                    }
                    c->msg_id++;
                    pending_ticks += 1;                    /* bù ~100ms gửi TCP */
                }
            }
            /* Rơi xuống bước (3) để kiểm tra ngay xem NASA-2 có đến hạn không */
        }

        /* (3a) Đếm thời gian bằng elapsed_ticks (tính cả thời gian blocking I/O từ pending_ticks).
         * Gửi NASA-2 khi đến hạn hoặc ACC đổi trạng thái.                          */
        if (!send_now) {
            BOOL acc = tracker_get_acc_status();
            interval_sec = acc ? TRACKER_RUN_INTERVAL_SEC : TRACKER_STOP_INTERVAL_SEC;
            if (acc != last_acc) {
                sAPI_Debug("[NASA] ACC changed %d -> %d, sending immediately", last_acc, acc);
                send_now = TRUE;
            } else if (elapsed_ticks >= interval_sec * 10) {
                send_now = TRUE;
            }
        }
        if (!send_now) continue;

        /* (3b) Đọc GNSS → timestamp chính xác → điện áp → gửi NASA-2.
         *
         * Nguyên tắc timestamp CHÍNH XÁC:
         *   - Bản đầu tiên (send_count=0): đọc GPS time thực → lưu làm gốc dt_next
         *   - Các bản tiếp: dùng dt_next (danh nghĩa = dt_prev + interval_sec)
         *     → Timestamp LUÔN cách đều interval_sec, không phụ thuộc GPS granularity 1Hz
         *   - Resync: nếu GPS lệch > 5s so với nominal → cập nhật gốc (time jump) */
        if (!tracker_get_location(&loc)) memset(&loc, 0, sizeof(loc));
        {
            double course;
            /* Hướng chạy: chỉ cập nhật khi có fix và xe chạy đủ nhanh; ngược lại giữ hướng cũ */
            if (loc.fixValid && loc.speed >= TRACKER_HEADING_MIN_SPEED_KMH && tracker_get_course(&course)) {
                c->heading = course;
            }
        }

        {
            char dt_gps[32] = {0};
            tracker_get_datetime(dt_gps, sizeof(dt_gps));     /* đọc GPS ngay, trước ADC */

            if (send_count == 0 || dt_next[0] == '\0') {
                /* Bản đầu tiên: dùng GPS time thực làm gốc */
                if (dt_gps[0] == '\0') { continue; }          /* chưa có giờ → giữ send_now=TRUE, thử lại vòng sau */
                snprintf(dt, sizeof(dt), "%s", dt_gps);
            } else {
                /* Bản kế tiếp: dùng timestamp danh nghĩa */
                snprintf(dt, sizeof(dt), "%s", dt_next);

                /* Resync nếu GPS lệch > 5s (GPS time jump, bật máy lại, v.v.) */
                if (dt_gps[0] != '\0') {
                    int yg,mog,dg,hg,mig,sg, yn,mon,dn,hn,minn,sn;
                    if (sscanf(dt_gps,  "%d-%d-%d %d:%d:%d", &yg,&mog,&dg,&hg,&mig,&sg) == 6 &&
                        sscanf(dt_next, "%d-%d-%d %d:%d:%d", &yn,&mon,&dn,&hn,&minn,&sn) == 6) {
                        int diff = (sg + mig*60 + hg*3600) - (sn + minn*60 + hn*3600);
                        if (diff < -43200) diff += 86400;
                        if (diff >  43200) diff -= 86400;
                        if (diff > 5 || diff < -5) {
                            snprintf(dt, sizeof(dt), "%s", dt_gps);
                            sAPI_Debug("[NASA] Timestamp resync: GPS drift %ds", diff);
                        }
                    }
                }
            }
        }

        last_acc     = tracker_get_acc_status();
        interval_sec = last_acc ? TRACKER_RUN_INTERVAL_SEC : TRACKER_STOP_INTERVAL_SEC;
        if (loc.fixValid) tracker_sync_rtc_from_gnss_if_due(interval_sec);

        /* Tính dt_next = dt hiện tại + interval_sec → dùng cho bản tin kế tiếp */
        tracker_datetime_add_seconds(dt, interval_sec, dt_next, sizeof(dt_next));

        voltage = tracker_get_voltage();           /* ADC 80ms: sau khi đã lưu dt và dt_next */
        tracker_msg_id_check_new_day(c, dt);       /* sang ngày mới → msg_id = 1 */
        len = tracker_build_journey(msg, sizeof(msg), c->msg_id, &loc, last_acc, voltage,
                                    c->odometer_km, c->heading, dt);
        if (len <= 0) { continue; }                /* build lỗi → giữ send_now=TRUE, thử lại vòng sau */

        sAPI_Debug("[NASA] TX NASA-2: %s", msg);
        if (tracker_tcp_send_all(c->sockfd, msg, len) != len) {
            if (tracker_get_datetime(dt, sizeof(dt))) tracker_backup_push(dt, &loc, last_acc, voltage);
            tracker_drop_session(c);
            return;
        }
        c->msg_id++;
        send_count++;
        send_now      = FALSE;
        elapsed_ticks = 0;                         /* reset đồng hồ realtime sau mỗi lần gửi NASA-2 */
        pending_ticks = 0;
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
    sAPI_Debug(" NASA-4G Tracker (Protocol v1.3) Started, FW %s", TRACKER_FW_VERSION);
    sAPI_Debug("================================================");

    tracker_reset_reason_init();                           /* R-1.0 nếu lần trước do watchdog, ngược lại R-2.0 */
    sAPI_NetworkInit();
    tracker_gnss_init();

    /* Startup probe: kiểm tra file backup còn sót từ lần reset trước.
     * sAPI_access() trả 0 nếu file tồn tại. Nếu có → bật cache để gửi lại khi có mạng. */
    if (sAPI_access(TRACKER_BACKUP_FLASH_PATH, 0) == 0) {
        flash_has_data    = TRUE;
        flash_read_offset = 0;
        sAPI_Debug("[NASA] Startup: phat hien file backup cu tren flash, se gui lai khi co mang");
    }

    while (1) {
        tracker_alive();
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
