/**
 * @file    tracker_util.c
 * @brief   Tiện ích chung: GPIO, lệnh AT, checksum NASA, tọa độ NMEA.
 *          (Tách nguyên văn từ sc_application.c, không đổi logic.)
 */
#include "tracker_util.h"

/* ===========================================================================
 * 4. TIỆN ÍCH CHUNG (GPIO, AT, checksum, tọa độ)
 * =========================================================================== */

/** Cấu hình một chân GPIO (pull-up, không ngắt). @param dir SC_GPIO_IN_PIN / SC_GPIO_OUT_PIN */
SC_GPIOReturnCode tracker_gpio_setup(int pin, int dir)
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
int tracker_at(const char *cmd, int timeout, char *resp, size_t size)
{
    char tmp[128];
    if (resp == NULL) { resp = tmp; size = sizeof(tmp); }
    memset(resp, 0, size);
    return (int)sAPI_SendATCMDWaitResp(TRACKER_AT_CHANNEL, (char *)cmd, timeout,
                                       NULL, 1, NULL, resp, size);
}

/** Checksum NASA: tổng giá trị các byte. */
uint32_t tracker_checksum(const uint8_t *buf, uint32_t len)
{
    uint32_t sum = 0;
    if (buf == NULL) return 0;
    while (len--) sum += *buf++;
    return sum;
}

/** Đổi tọa độ NMEA (ddmm.mmmm) sang độ thập phân; hướng S/W cho giá trị âm. */
double tracker_nmea_to_decimal(const char *coord, const char *direction)
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
