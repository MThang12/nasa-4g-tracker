/**
 * @file    tracker_led.c
 * @brief   Đèn LED trạng thái và watchdog (task LED cho WDT ăn khi task tracker còn báo sống).
 *          (Tách nguyên văn từ sc_application.c, không đổi logic.)
 */
#include "tracker_led.h"
#include "tracker_util.h"
#include "tracker_sensor.h"   /* tracker_acc_poll() */

/* Chế độ LED do task chính ghi, task LED đọc (volatile vì dùng chung giữa 2 task) */
volatile tracker_led_mode_t led_net_mode  = LED_MODE_OFF;
volatile tracker_led_mode_t led_gnss_mode = LED_MODE_OFF;
volatile tracker_led_mode_t led_pwr_mode  = LED_MODE_ON;

/* Bộ đếm "còn sống": chỉ task tracker ghi (tracker_alive), task LED đọc để quyết định cho WDT ăn */
static volatile uint32_t tracker_alive_counter = 0;

/** Task tracker báo còn sống. Gọi trong mọi vòng lặp/vòng chờ của task tracker. */
void tracker_alive(void)
{
    tracker_alive_counter++;
}

/* ===========================================================================
 * NGUYÊN NHÂN RESET (trường 11 của NASA-1)
 *   R-2.0: bật nguồn (không có file dấu)     R-1.0: lần trước do watchdog (có file dấu)
 *   R-0.0: kết nối lại TCP trong cùng lần chạy
 * Hạn chế: chỉ ghi được dấu khi task LED chủ động ngừng cho ăn (task tracker treo). Nếu cả hệ thống
 * treo thì không ai ghi dấu -> lần khởi động sau báo R-2.0 (SDK không có API đọc lý do reset).
 * =========================================================================== */

/* Chỉ task tracker đọc/ghi biến này (init / get / consumed) */
static const char *reset_reason = TRACKER_RESET_REASON_POWER_ON;

void tracker_reset_reason_init(void)
{
    if (sAPI_access(TRACKER_RESET_FLAG_PATH, 0) == 0) {    /* 0 = file tồn tại */
        char buf[8] = {0};
        SCFILE *fp = sAPI_fopen(TRACKER_RESET_FLAG_PATH, "rb");
        if (fp != NULL) {
            sAPI_fread(buf, 1, sizeof(buf) - 1, fp);
            sAPI_fclose(fp);
        }
        /* "R-3.0" = reset mềm bằng lệnh 9; nội dung khác (kể cả đọc lỗi) = watchdog như trước */
        reset_reason = (strncmp(buf, TRACKER_RESET_REASON_SOFT, sizeof(TRACKER_RESET_REASON_SOFT) - 1) == 0)
                       ? TRACKER_RESET_REASON_SOFT : TRACKER_RESET_REASON_WATCHDOG;
        sAPI_remove(TRACKER_RESET_FLAG_PATH);
    } else {
        reset_reason = TRACKER_RESET_REASON_POWER_ON;
    }
    sAPI_Debug("[NASA] Reset reason: %s", reset_reason);
}

const char *tracker_reset_reason_get(void)
{
    return reset_reason;
}

void tracker_reset_reason_consumed(void)
{
    reset_reason = TRACKER_RESET_REASON_RECONNECT;
}

/**
 * Ghi file dấu lý do cho lần reset sắp tới ("R-1.0" watchdog / "R-3.0" lệnh 9).
 * Watchdog: task LED gọi ngay trước khi ngừng cho ăn. Lệnh 9: task tracker gọi trước sAPI_SysReset().
 */
void tracker_reset_mark(const char *reason)
{
    SCFILE *fp = sAPI_fopen(TRACKER_RESET_FLAG_PATH, "wb");
    int len = (int)strlen(reason);
    BOOL ok;

    if (fp == NULL) {
        sAPI_Debug("[NASA] Reset mark: khong mo duoc file");
        return;
    }
    /* sAPI_fwrite trả về số BYTE đã ghi */
    ok = (sAPI_fwrite(reason, 1, len, fp) == len) ? TRUE : FALSE;
    sAPI_fsync(fp);
    sAPI_fclose(fp);
    sAPI_Debug("[NASA] Reset mark (%s): %s", reason, ok ? "OK" : "FAIL");
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

/**
 * Bật watchdog phần cứng theo đúng trình tự sc_demo/src/demo_wtd.c:
 * đặt timeout -> FalutWakeEnable(1) -> SoftWtdEnable(1) -> cho ăn 1 lần để nạp timeout.
 * @return TRUE nếu mọi bước trả 0.
 */
static BOOL tracker_wdt_start(void)
{
    int ret;
    ret  = sAPI_SetWtdTimeOutPeriod(TRACKER_WDT_TIMEOUT_REG_VALUE);
    ret |= sAPI_FalutWakeEnable(1);
    ret |= sAPI_SoftWtdEnable(1);
    ret |= sAPI_FeedWtd();
    sAPI_Debug("[NASA] WDT start %s (timeout reg=0x%02X, stall=%ds)",
               (ret == 0) ? "OK" : "FAIL", TRACKER_WDT_TIMEOUT_REG_VALUE, TRACKER_WDT_STALL_SEC);
    return (ret == 0) ? TRUE : FALSE;
}

/**
 * Task điều khiển LED: PWR/NET/GNSS theo led_pwr_mode/led_net_mode/led_gnss_mode.
 * Kiêm cho WDT ăn mỗi 1 s khi task tracker còn báo sống (xem TRACKER_WDT_STALL_SEC).
 */
void sTask_LedControl(void *arg)
{
    int      tick_100ms   = 0;
    BOOL     wdt_on;
    uint32_t last_alive;
    int      stall_ticks  = 0;                            /* số tick 100 ms tracker không báo sống */
    BOOL     wdt_give_up  = FALSE;                        /* đã quyết định để WDT reset: không cho ăn nữa */
    (void)arg;

    tracker_gpio_setup(TRACKER_LED_PWR_PIN,  SC_GPIO_OUT_PIN);
    tracker_gpio_setup(TRACKER_LED_NET_PIN,  SC_GPIO_OUT_PIN);
    tracker_gpio_setup(TRACKER_LED_GNSS_PIN, SC_GPIO_OUT_PIN);
    sAPI_GpioSetValue(TRACKER_LED_PWR_PIN, LED_LEVEL_ON);

    wdt_on     = tracker_wdt_start();
    last_alive = tracker_alive_counter;

    while (1) {
        tick_100ms++;
        tracker_acc_poll();                               /* lọc nhiễu ACC mỗi 100 ms */
        tracker_led_apply(TRACKER_LED_PWR_PIN,  led_pwr_mode,  tick_100ms);
        tracker_led_apply(TRACKER_LED_NET_PIN,  led_net_mode,  tick_100ms);
        tracker_led_apply(TRACKER_LED_GNSS_PIN, led_gnss_mode, tick_100ms);

        if (wdt_on && !wdt_give_up) {
            uint32_t now_alive = tracker_alive_counter;
            if (now_alive != last_alive) {
                last_alive = now_alive;
                stall_ticks = 0;
            } else {
                stall_ticks++;
            }

            if (stall_ticks < TRACKER_WDT_STALL_SEC * 10) {
                if (tick_100ms % TRACKER_WDT_FEED_PERIOD_TICKS == 0) sAPI_FeedWtd();
            } else {
                sAPI_Debug("[NASA] WDT: tracker task khong bao song %ds -> ngung cho an, cho reset", TRACKER_WDT_STALL_SEC);
                tracker_reset_mark(TRACKER_RESET_REASON_WATCHDOG);
                wdt_give_up = TRUE;   /* chốt: không cho ăn lại dù tracker hồi phục, để dấu R-1.0 luôn khớp lần reset */
            }
        }
        sAPI_TaskSleep(20);                               /* 100 ms */
    }
}
