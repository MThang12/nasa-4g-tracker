#ifndef TRACKER_COMMON_H
#define TRACKER_COMMON_H

/**
 * @file    tracker_common.h
 * @brief   Include SDK, cấu hình và kiểu dữ liệu dùng chung cho mọi module tracker.
 *          (Tách nguyên văn từ sc_application.c, không đổi logic.)
 */

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
#include "simcom_file.h"   /* sAPI_fopen/fwrite/fread/fclose - lưu backup xuống flash */
#include "sc_power.h"      /* sAPI_ReadAdc() */
#include "simcom_wtd.h"    /* watchdog phần cứng */

#include "tracker_config.h"

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
    double   heading;       /* hướng chạy GNSS gần nhất (độ); giữ giá trị cũ khi mất fix / chạy chậm */
} tracker_ctx_t;

/* Chế độ đèn LED */
typedef enum {
    LED_MODE_OFF = 0,       /* tắt */
    LED_MODE_ON,            /* sáng liên tục */
    LED_MODE_BLINK_FAST,    /* nháy nhanh (5 Hz) */
    LED_MODE_BLINK_SLOW     /* nháy chậm (1 Hz) */
} tracker_led_mode_t;

/* Trạng thái đăng nhập (định nghĩa trong sc_application.c, dùng chung với tracker_backup.c) */
extern BOOL is_logged_in;       /* đang có phiên hợp lệ */
extern BOOL has_logged_in_once; /* đã từng đăng nhập (điều kiện để bắt đầu backup) */

#endif /* TRACKER_COMMON_H */
