/**
 * @file    tracker_sms.c
 * @brief   Lệnh cấu hình qua SMS (file "Config Command NASA-4G latest.xlsx": cùng cấu trúc với TCP).
 *
 * Luồng xử lý:
 *   - tracker_sms_init(): tạo hàng đợi URC, sAPI_UrcRefRegister(..., SC_URC_SMS_MASK).
 *   - Khi SMS sẵn sàng (getSmsReady()): sAPI_SmsSetNewMsgInd(1,1,2,0,0,0) -> SMS mới được báo thẳng
 *     nội dung qua URC SC_URC_FLASH_MSG, không lưu vào SIM (simcom_sms.h, demo_sms.c).
 *   - tracker_sms_poll(): đọc URC không chặn, tách số gửi + nội dung, chạy tracker_cmd_execute(),
 *     gửi phản hồi bằng sAPI_SmsSendLongMsg() (tự chia nhiều SMS nếu dài).
 *
 * Định dạng SC_URC_FLASH_MSG thấy trên thiết bị (12/11/2026): dạng PDU
 *   +CMT: "",<độ dài TPDU><PDU hex>   (log không hiện xuống dòng giữa độ dài và PDU)
 * -> giải mã bằng sAPI_SmsDecodePdu(). Vẫn hỗ trợ dạng text +CMT: "<số>",...\r\n<nội dung>.
 * Luôn log chuỗi thô để đối chiếu.
 */
#include "tracker_sms.h"
#include "tracker_protocol.h"   /* tracker_cmd_execute */
#include "tracker_led.h"        /* tracker_alive */
#include "simcom_sms.h"

static sMsgQRef sms_urc_q  = NULL;      /* URC SMS (SC_URC_SMS_MASK) */
static sMsgQRef sms_rsp_q  = NULL;      /* kết quả của sAPI_SmsSendLongMsg */
static BOOL     sms_configured = FALSE; /* đã đặt chế độ báo SMS mới */

static char   last_from[SC_SMS_MAX_ADDRESS_LENGTH + 1] = {0};  /* chống trả lời dồn dập */
static UINT32 last_reply_tick = 0;

void tracker_sms_init(void)
{
    int ret;

    if (sms_urc_q != NULL) return;
    if (sAPI_MsgQCreate(&sms_urc_q, "nasa_sms_urc", sizeof(SIM_MSG_T), TRACKER_SMS_QUEUE_DEPTH, SC_FIFO) != SC_SUCCESS ||
        sAPI_MsgQCreate(&sms_rsp_q, "nasa_sms_rsp", sizeof(SIM_MSG_T), TRACKER_SMS_QUEUE_DEPTH, SC_FIFO) != SC_SUCCESS) {
        sAPI_Debug("[NASA] SMS: tao hang doi loi");
        sms_urc_q = NULL;
        return;
    }
    ret = sAPI_UrcRefRegister(sms_urc_q, SC_URC_SMS_MASK);
    sAPI_Debug("[NASA] SMS: dang ky URC ret=%d", ret);
    if (ret != 0) sms_urc_q = NULL;
}

/** Đặt chế độ báo SMS mới khi dịch vụ SMS đã sẵn sàng (chỉ làm 1 lần). */
static void tracker_sms_configure(void)
{
    SC_SMSReturnCode ret;

    if (sms_configured || !getSmsReady()) return;
    ret = sAPI_SmsSetNewMsgInd(1, 1, 2, 0, 0, 0);          /* text mode, báo thẳng nội dung (SC_URC_FLASH_MSG) */
    sAPI_Debug("[NASA] SMS ready, SetNewMsgInd(1,1,2,0,0,0) ret=%d", ret);
    sms_configured = TRUE;
}

/** Copy chuỗi, cắt bớt nếu dài hơn dst_size - 1; luôn kết thúc '\0'. */
static void tracker_copy_str(char *dst, size_t dst_size, const char *src)
{
    size_t n = strlen(src);
    if (dst_size == 0) return;
    if (n >= dst_size) n = dst_size - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

/** Bỏ \r, \n, khoảng trắng ở cuối chuỗi. @return độ dài còn lại. */
static size_t tracker_sms_rtrim(char *s)
{
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\r' || s[n - 1] == '\n' || s[n - 1] == ' ')) s[--n] = '\0';
    return n;
}

static int tracker_hex_val(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

/** Số ký tự hex liên tiếp từ đầu chuỗi. */
static size_t tracker_hex_span(const char *s)
{
    size_t n = 0;
    while (tracker_hex_val(s[n]) >= 0) n++;
    return n;
}

/** PDU (hex) có độ dài khớp "<độ dài TPDU>" không: tổng = 2 * (độ dài SMSC + 1 + tpdu_len). */
static BOOL tracker_pdu_len_ok(const char *hex, size_t hex_len, int tpdu_len)
{
    int smsc;
    if (hex_len < 2 || tpdu_len <= 0) return FALSE;
    smsc = tracker_hex_val(hex[0]) * 16 + tracker_hex_val(hex[1]);
    return hex_len == (size_t)(2 * (smsc + 1 + tpdu_len));
}

/**
 * Lấy PDU hex từ URC dạng PDU: +CMT: "",<độ dài TPDU>\r\n<PDU hex>
 * (log 12/11/2026 cho thấy dạng này; nếu không có xuống dòng giữa độ dài và PDU thì thử tách 1-3 chữ số
 * đầu làm độ dài, chọn cách tách mà độ dài PDU khớp).
 */
static BOOL tracker_sms_extract_pdu(const char *raw, char *pdu, size_t pdu_size)
{
    const char *p = strrchr(raw, '"');
    const char *hex = NULL;
    size_t hex_len = 0;
    int k;

    p = (p != NULL) ? p + 1 : raw;
    if (*p == ',') p++;
    while (*p == ' ') p++;

    if (strpbrk(p, "\r\n") != NULL) {                      /* có xuống dòng: "<độ dài>\r\n<PDU>" */
        int tpdu_len = atoi(p);
        hex = strpbrk(p, "\r\n");
        while (*hex == '\r' || *hex == '\n') hex++;
        hex_len = tracker_hex_span(hex);
        if (!tracker_pdu_len_ok(hex, hex_len, tpdu_len)) hex = NULL;
    } else {                                               /* độ dài và PDU dính liền */
        for (k = 1; k <= 3 && hex == NULL; k++) {
            char num[4] = {0};
            int i;
            for (i = 0; i < k; i++) {
                if (p[i] < '0' || p[i] > '9') break;
                num[i] = p[i];
            }
            if (i < k) break;
            hex_len = tracker_hex_span(p + k);
            if (tracker_pdu_len_ok(p + k, hex_len, atoi(num))) hex = p + k;
        }
    }

    if (hex == NULL || hex_len == 0 || hex_len >= pdu_size) return FALSE;
    memcpy(pdu, hex, hex_len);
    pdu[hex_len] = '\0';
    return TRUE;
}

/**
 * Tách số gửi và nội dung từ chuỗi URC.
 *   - Dạng PDU (đã thấy trên thiết bị): +CMT: "",<len>\r\n<PDU> -> giải mã bằng sAPI_SmsDecodePdu().
 *   - Dạng text: +CMT: "<số>","","<giờ>"\r\n<nội dung>.
 * @return FALSE nếu không tách được số hoặc nội dung.
 */
static BOOL tracker_sms_parse(const char *raw, char *from, size_t from_size, char *body, size_t body_size)
{
    static char pdu[400];                                  /* PDU hex tối đa ~(12 + 164) * 2 */
    static char text[512];                                 /* nội dung sau giải mã (bộ đệm rộng cho an toàn) */
    char addr[64];
    const char *q1, *q2, *nl;
    SC_SMSReturnCode ret;
    int text_len = 0;
    size_t n;

    /* Dạng text: số nằm trong cặp nháy kép đầu tiên và không rỗng */
    q1 = strchr(raw, '"');
    q2 = (q1 != NULL) ? strchr(q1 + 1, '"') : NULL;
    if (q2 != NULL && q2 > q1 + 1) {
        n = (size_t)(q2 - q1 - 1);
        nl = strchr(q2, '\n');
        if (n >= from_size || nl == NULL) return FALSE;
        memcpy(from, q1 + 1, n);
        from[n] = '\0';
        snprintf(body, body_size, "%s", nl + 1);
        return tracker_sms_rtrim(body) > 0;
    }

    /* Dạng PDU */
    if (!tracker_sms_extract_pdu(raw, pdu, sizeof(pdu))) return FALSE;
    memset(addr, 0, sizeof(addr));
    memset(text, 0, sizeof(text));
    ret = sAPI_SmsDecodePdu(pdu, addr, &text_len, text);
    sAPI_Debug("[NASA] SMS PDU decode ret=%d addr=%s len=%d text=%s", ret, addr, text_len, text);
    if (ret != SC_SMS_SUCESS || addr[0] == '\0') return FALSE;

    addr[sizeof(addr) - 1] = '\0';
    text[sizeof(text) - 1] = '\0';
    tracker_copy_str(from, from_size, addr);
    tracker_copy_str(body, body_size, text);
    return tracker_sms_rtrim(body) > 0;
}

/** Gửi phản hồi và chờ kết quả tối đa TRACKER_SMS_SEND_TIMEOUT_SEC (vẫn báo sống cho watchdog). */
static void tracker_sms_send(const char *to, const char *text, int len)
{
    SIM_MSG_T rsp;
    SC_SMSReturnCode ret;
    int waited;

    while (sAPI_MsgQRecv(sms_rsp_q, &rsp, SC_NO_SUSPEND) == SC_SUCCESS) {    /* bỏ kết quả cũ còn sót */
        if (rsp.arg3 != NULL) sAPI_Free(rsp.arg3);
    }

    ret = sAPI_SmsSendLongMsg((UINT8 *)text, (UINT16)len, (UINT8 *)to, sms_rsp_q);
    sAPI_Debug("[NASA] SMS TX to %s (%d B) ret=%d", to, len, ret);
    if (ret != SC_SMS_SUCESS) return;

    for (waited = 0; waited < TRACKER_SMS_SEND_TIMEOUT_SEC; waited++) {
        memset(&rsp, 0, sizeof(rsp));
        if (sAPI_MsgQRecv(sms_rsp_q, &rsp, SEC_TO_TICKS(1)) == SC_SUCCESS) {
            sAPI_Debug("[NASA] SMS TX result: primId=%d result=%d rsp=%s",
                       rsp.arg1, rsp.arg2, rsp.arg3 != NULL ? (char *)rsp.arg3 : "");
            if (rsp.arg3 != NULL) sAPI_Free(rsp.arg3);
            return;
        }
        tracker_alive();
    }
    sAPI_Debug("[NASA] SMS TX: het %ds chua co ket qua", TRACKER_SMS_SEND_TIMEOUT_SEC);
}

/** Xử lý 1 SMS: chạy lệnh, gửi phản hồi (giới hạn 1 phản hồi / TRACKER_SMS_REPLY_MIN_SEC cho cùng 1 số). */
static void tracker_sms_handle(const char *raw)
{
    char from[SC_SMS_MAX_ADDRESS_LENGTH + 1];
    char body[TRACKER_SMS_BODY_SIZE];
    char reply[TRACKER_RX_BUF_SIZE];
    UINT32 now;
    int len;

    sAPI_Debug("[NASA] SMS RX raw: %s", raw);
    if (!tracker_sms_parse(raw, from, sizeof(from), body, sizeof(body))) {
        sAPI_Debug("[NASA] SMS: khong tach duoc so/noi dung");
        return;
    }
    sAPI_Debug("[NASA] SMS from %s: %s", from, body);

    len = tracker_cmd_execute(body, reply, sizeof(reply));
    if (len <= 0) return;                                  /* không phải lệnh hoặc lệnh chưa hỗ trợ */

    now = sAPI_GetTicks();
    if (strcmp(from, last_from) == 0 &&
        (UINT32)(now - last_reply_tick) < (UINT32)TRACKER_SMS_REPLY_MIN_SEC * TICKS_PER_SEC) {
        sAPI_Debug("[NASA] SMS: %s da duoc tra loi trong %ds, bo qua", from, TRACKER_SMS_REPLY_MIN_SEC);
        return;
    }
    snprintf(last_from, sizeof(last_from), "%s", from);
    last_reply_tick = now;

    tracker_sms_send(from, reply, len);
}

void tracker_sms_poll(void)
{
    SIM_MSG_T msg;

    if (sms_urc_q == NULL) return;
    tracker_sms_configure();

    while (sAPI_MsgQRecv(sms_urc_q, &msg, SC_NO_SUSPEND) == SC_SUCCESS) {
        if (msg.msg_id == SRV_URC && msg.arg1 == SC_URC_SMS_MASK) {
            if (msg.arg2 == SC_URC_FLASH_MSG && msg.arg3 != NULL) {
                tracker_sms_handle((const char *)msg.arg3);
            } else {
                sAPI_Debug("[NASA] SMS URC %d: %s", msg.arg2, msg.arg3 != NULL ? (char *)msg.arg3 : "");
            }
        }
        if (msg.arg3 != NULL) free(msg.arg3);              /* như cus_urc.c: bên nhận giải phóng arg3 */
    }
}
