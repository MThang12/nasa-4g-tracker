/**
 * @file    tracker_device.c
 * @brief   Nhận diện thiết bị: ICCID của SIM, IMEI của module.
 *          (Tách nguyên văn từ sc_application.c, không đổi logic.)
 */
#include "tracker_device.h"
#include "tracker_util.h"

/* ===========================================================================
 * 7. NHẬN DIỆN THIẾT BỊ (ICCID của SIM, IMEI của module)
 * =========================================================================== */

/** Đọc ICCID (bỏ ký tự 'F' cuối nếu có); lỗi thì dùng giá trị placeholder. */
void tracker_get_sim_id(char *out, size_t out_size)
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
void tracker_get_imei(char *out, size_t out_size)
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
