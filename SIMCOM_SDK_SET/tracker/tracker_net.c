/**
 * @file    tracker_net.c
 * @brief   SIM, mạng di động, PDP và TCP.
 *          (Tách nguyên văn từ sc_application.c, không đổi logic.)
 */
#include "tracker_net.h"
#include "tracker_util.h"
#include "tracker_backup.h"
#include "tracker_led.h"        /* tracker_alive, tracker_reset_mark, led_net_mode (lệnh 33) */
#include "sc_app_download.h"    /* sAPI_AppDownload */
#include "sc_app_update.h"      /* sAPI_AppPackageCrc */
#include "sc_flash.h"           /* sAPI_FBF_Disable */

/* ===========================================================================
 * 10. SIM, MẠNG DI ĐỘNG VÀ TCP
 * =========================================================================== */

/* ---------------------------------------------------------------------------
 * Server (lệnh 1). Chỉ task tracker đọc/ghi.
 * ------------------------------------------------------------------------- */
static char server_cfg_host[TRACKER_SERVER_HOST_MAX + 1] = TRACKER_SERVER_HOST;
static int  server_cfg_port     = TRACKER_SERVER_PORT;
static BOOL server_use_default  = FALSE;   /* TRUE: server cấu hình lỗi liên tục -> dùng mặc định đến hết lần chạy */
static int  server_fail_count   = 0;

BOOL tracker_server_valid(const char *host, long port)
{
    size_t i, n;
    if (host == NULL || port < 1 || port > 65535) return FALSE;
    n = strlen(host);
    if (n == 0 || n > TRACKER_SERVER_HOST_MAX) return FALSE;
    for (i = 0; i < n; i++) {
        char ch = host[i];
        if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') ||
              ch == '.' || ch == '-')) return FALSE;
    }
    return TRUE;
}

/** Server cấu hình có khác server mặc định không. */
static BOOL tracker_server_is_custom(void)
{
    return (strcmp(server_cfg_host, TRACKER_SERVER_HOST) != 0 || server_cfg_port != TRACKER_SERVER_PORT) ? TRUE : FALSE;
}

void tracker_server_load(void)
{
    char buf[TRACKER_SERVER_HOST_MAX + 16];
    char host[TRACKER_SERVER_HOST_MAX + 1];
    int n, port = 0;
    SCFILE *fp = sAPI_fopen(TRACKER_SERVER_CFG_PATH, "rb");

    if (fp == NULL) {
        sAPI_Debug("[NASA] Server: mac dinh %s:%d", server_cfg_host, server_cfg_port);
        return;
    }
    n = sAPI_fread(buf, 1, sizeof(buf) - 1, fp);           /* trả về số BYTE đã đọc */
    sAPI_fclose(fp);
    if (n < 0) n = 0;
    buf[n] = '\0';
    if (sscanf(buf, "%63[^,],%d", host, &port) == 2 && tracker_server_valid(host, port)) {
        snprintf(server_cfg_host, sizeof(server_cfg_host), "%s", host);
        server_cfg_port = port;
        sAPI_Debug("[NASA] Server (lenh 1): %s:%d", server_cfg_host, server_cfg_port);
    } else {
        sAPI_Debug("[NASA] Server: file cau hinh loi, dung mac dinh %s:%d", server_cfg_host, server_cfg_port);
    }
}

BOOL tracker_server_set(const char *host, int port)
{
    char buf[TRACKER_SERVER_HOST_MAX + 16];
    int len;
    BOOL ok;
    SCFILE *fp;

    snprintf(server_cfg_host, sizeof(server_cfg_host), "%s", host);
    server_cfg_port    = port;
    server_use_default = FALSE;
    server_fail_count  = 0;

    len = snprintf(buf, sizeof(buf), "%s,%d", server_cfg_host, server_cfg_port);
    fp = sAPI_fopen(TRACKER_SERVER_CFG_PATH, "wb");
    if (fp == NULL) {
        sAPI_Debug("[NASA] Server: khong mo duoc file de ghi");
        return FALSE;
    }
    ok = (sAPI_fwrite(buf, 1, len, fp) == len) ? TRUE : FALSE;   /* trả về số BYTE đã ghi */
    sAPI_fsync(fp);
    sAPI_fclose(fp);
    sAPI_Debug("[NASA] Server -> %s:%d, luu %s", server_cfg_host, server_cfg_port, ok ? "OK" : "FAIL");
    return ok;
}

const char *tracker_server_cfg_host(void) { return server_cfg_host; }
int tracker_server_cfg_port(void)         { return server_cfg_port; }
const char *tracker_server_host(void)     { return server_use_default ? TRACKER_SERVER_HOST : server_cfg_host; }
int tracker_server_port(void)             { return server_use_default ? TRACKER_SERVER_PORT : server_cfg_port; }

void tracker_server_result(BOOL ok)
{
    if (ok) { server_fail_count = 0; return; }
    if (server_use_default || !tracker_server_is_custom()) return;
    if (++server_fail_count >= TRACKER_SERVER_FALLBACK_FAILS) {
        server_use_default = TRUE;
        sAPI_Debug("[NASA] Server %s:%d loi %d lan lien tiep -> quay ve mac dinh %s:%d",
                   server_cfg_host, server_cfg_port, server_fail_count, TRACKER_SERVER_HOST, TRACKER_SERVER_PORT);
    }
}

/** Đọc cường độ sóng CSQ (0-31, 99 = không xác định). */
int tracker_get_csq(void)
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
int tracker_tcp_connect(void)
{
    struct addrinfo hints, *list = NULL, *rp;
    char portstr[8];
    int fd = -1;
    int pdp_id = TRACKER_PDP_ID;

    snprintf(portstr, sizeof(portstr), "%d", tracker_server_port());
    sAPI_Debug("[NASA] TCP connect %s:%s", tracker_server_host(), portstr);
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family   = AF_INET;
    hints.ai_protocol = IPPROTO_TCP;
    if (getaddrinfo(tracker_server_host(), portstr, &hints, &list) != 0 || list == NULL) return -1;

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
int tracker_tcp_send_all(int fd, const char *data, int len)
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
void tracker_tcp_close(int *fd)
{
    if (fd != NULL && *fd >= 0) { close(*fd); *fd = -1; }
}

/** Bước 1: chờ SIM + mạng. Sau 15 lần kiểm tra lỗi thì reset RF (CFUN=0/1) để khôi phục SIM/mạng. */
void tracker_wait_network(void)
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
BOOL tracker_activate_pdp(void)
{
    while (!tracker_pdp_active()) {
        sAPI_Debug("[NASA] PDP activation failed, retry...");
        tracker_delay(TRACKER_PDP_RETRY_SEC);
        if (!tracker_check_sim_and_network()) return FALSE;
    }
    return tracker_check_sim_and_network();
}

/* ===========================================================================
 * LỆNH 33: CẬP NHẬT FIRMWARE TỪ XA (theo sc_demo/src/demo_app_download.c)
 * =========================================================================== */
static char          fota_url[TRACKER_FOTA_URL_MAX + 1];
static volatile BOOL fota_done   = FALSE;
static volatile int  fota_dl_ret = -1;      /* SCAppDwonLoadReturnCode */
static volatile int  fota_crc_ret = -1;     /* sAPI_AppPackageCrc: 0 = gói hợp lệ */

/** Task tải: sAPI_AppDownload chặn lâu nên không chạy trong task tracker (watchdog 300 s). */
static void sTask_NasaFota(void *arg)
{
    SCAppDownloadPram pram;
    SCAppPackageInfo info;
    (void)arg;

    memset(&pram, 0, sizeof(pram));
    pram.url = fota_url;
    pram.mod = (strncmp(fota_url, "ftp", 3) == 0) ? SC_APP_DOWNLOAD_FTP_MOD : SC_APP_DOWNLOAD_HTTP_MOD;
    pram.recvtimeout = TRACKER_FOTA_RECV_TIMEOUT_MS;

    sAPI_FBF_Disable();                                    /* demo: "must unlock fbf before download" */
    fota_dl_ret = (int)sAPI_AppDownload(&pram);
    sAPI_Debug("[NASA] FOTA: AppDownload ret=%d", fota_dl_ret);

    if (fota_dl_ret == SC_APP_DOWNLOAD_SUCESSED) {
        memset(&info, 0, sizeof(info));
        fota_crc_ret = sAPI_AppPackageCrc(&info);
        sAPI_Debug("[NASA] FOTA: AppPackageCrc ret=%d hasPackage=%d size=%u crc=0x%08X",
                   fota_crc_ret, info.hasPackage, info.binSize, info.crcValue);
    }
    fota_done = TRUE;                                      /* task tracker xóa task này sau khi hàm trả về */
}

void tracker_fota_run(const char *url)
{
    sTaskRef task = NULL;
    int waited;

    if (url == NULL || url[0] == '\0') return;
    snprintf(fota_url, sizeof(fota_url), "%s", url);
    fota_done = FALSE;
    fota_dl_ret = -1;
    fota_crc_ret = -1;

    sAPI_Debug("[NASA] FOTA: bat dau tai %s", fota_url);
    tracker_backup_flush_ram();                            /* không mất backup RAM nếu reset */
    led_net_mode = LED_MODE_BLINK_FAST;

    if (sAPI_TaskCreate(&task, NULL, TRACKER_FOTA_TASK_STACK, TRACKER_FOTA_TASK_PRIO,
                        "NASA-FOTA", sTask_NasaFota, NULL) != SC_SUCCESS) {
        sAPI_Debug("[NASA] FOTA: tao task loi");
        return;
    }

    for (waited = 0; !fota_done && waited < TRACKER_FOTA_TIMEOUT_SEC; waited++) {
        sAPI_TaskSleep(SEC_TO_TICKS(1));
        tracker_alive();
    }

    if (!fota_done) {
        sAPI_Debug("[NASA] FOTA: qua %ds chua xong -> reset mem", TRACKER_FOTA_TIMEOUT_SEC);
    } else {
        sAPI_TaskSleep(SEC_TO_TICKS(1));                   /* chắc chắn hàm task đã trả về rồi mới xóa */
        sAPI_TaskDelete(task);
        if (fota_dl_ret != SC_APP_DOWNLOAD_SUCESSED || fota_crc_ret != 0) {
            sAPI_Debug("[NASA] FOTA: that bai (download=%d crc=%d) -> chay tiep firmware cu", fota_dl_ret, fota_crc_ret);
            return;
        }
        sAPI_Debug("[NASA] FOTA: tai xong, CRC OK -> reset de chay firmware moi");
    }
    tracker_reset_mark(TRACKER_RESET_REASON_SOFT);         /* lần login đầu sau khởi động gửi R-3.0 */
    sAPI_TaskSleep(SEC_TO_TICKS(1));                       /* cho log kịp xuất */
    sAPI_SysReset();
}
