/**
 * @file    tracker_led.c
 * @brief   Đèn LED trạng thái và watchdog (task LED cho WDT ăn khi task tracker còn báo sống).
 *          (Tách nguyên văn từ sc_application.c, không đổi logic.)
 */
#include "tracker_led.h"
#include "tracker_util.h"

/* Chế độ LED do task chính ghi, task LED đọc (volatile vì dùng chung giữa 2 task) */
volatile tracker_led_mode_t led_net_mode  = LED_MODE_OFF;
volatile tracker_led_mode_t led_gnss_mode = LED_MODE_OFF;

/* Bộ đếm "còn sống": chỉ task tracker ghi (tracker_alive), task LED đọc để quyết định cho WDT ăn */
static volatile uint32_t tracker_alive_counter = 0;

/** Task tracker báo còn sống. Gọi trong mọi vòng lặp/vòng chờ của task tracker. */
void tracker_alive(void)
{
    tracker_alive_counter++;
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
 * Task điều khiển LED: PWR sáng cố định, NET/GNSS theo led_net_mode/led_gnss_mode.
 * Kiêm cho WDT ăn mỗi 1 s khi task tracker còn báo sống (xem TRACKER_WDT_STALL_SEC).
 */
void sTask_LedControl(void *arg)
{
    int      tick_100ms   = 0;
    BOOL     wdt_on;
    uint32_t last_alive;
    int      stall_ticks  = 0;                            /* số tick 100 ms tracker không báo sống */
    BOOL     stall_logged = FALSE;
    (void)arg;

    tracker_gpio_setup(TRACKER_LED_PWR_PIN,  SC_GPIO_OUT_PIN);
    tracker_gpio_setup(TRACKER_LED_NET_PIN,  SC_GPIO_OUT_PIN);
    tracker_gpio_setup(TRACKER_LED_GNSS_PIN, SC_GPIO_OUT_PIN);
    sAPI_GpioSetValue(TRACKER_LED_PWR_PIN, LED_LEVEL_ON);

    wdt_on     = tracker_wdt_start();
    last_alive = tracker_alive_counter;

    while (1) {
        tick_100ms++;
        tracker_led_apply(TRACKER_LED_NET_PIN,  led_net_mode,  tick_100ms);
        tracker_led_apply(TRACKER_LED_GNSS_PIN, led_gnss_mode, tick_100ms);

        if (wdt_on) {
            uint32_t now_alive = tracker_alive_counter;
            if (now_alive != last_alive) {
                last_alive = now_alive;
                stall_ticks = 0;
                stall_logged = FALSE;
            } else if (stall_ticks < TRACKER_WDT_STALL_SEC * 10) {
                stall_ticks++;
            }

            if (stall_ticks < TRACKER_WDT_STALL_SEC * 10) {
                if (tick_100ms % TRACKER_WDT_FEED_PERIOD_TICKS == 0) sAPI_FeedWtd();
            } else if (!stall_logged) {
                sAPI_Debug("[NASA] WDT: tracker task khong bao song %ds -> ngung cho an, cho reset", TRACKER_WDT_STALL_SEC);
                stall_logged = TRUE;
            }
        }
        sAPI_TaskSleep(20);                               /* 100 ms */
    }
}
