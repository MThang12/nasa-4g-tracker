/**
 * @file    tracker_net.c
 * @brief   SIM, mạng di động, PDP và TCP.
 *          (Tách nguyên văn từ sc_application.c, không đổi logic.)
 */
#include "tracker_net.h"
#include "tracker_util.h"
#include "tracker_backup.h"

/* ===========================================================================
 * 10. SIM, MẠNG DI ĐỘNG VÀ TCP
 * =========================================================================== */

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
