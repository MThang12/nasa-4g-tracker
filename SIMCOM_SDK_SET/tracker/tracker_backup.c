/**
 * @file    tracker_backup.c
 * @brief   Hàng đợi backup RAM + archive flash, lưu backup khi mất kết nối, tracker_delay.
 *          (Tách nguyên văn từ sc_application.c, không đổi logic.)
 */
#include "tracker_backup.h"
#include "tracker_led.h"
#include "tracker_time.h"
#include "tracker_sensor.h"

/* Hàng đợi backup (RAM): bộ đệm vòng ~8.8 KB, đầy thì archive toàn bộ xuống flash.
 * Giữ nhỏ (100 bản) để tránh tràn RAM userspace của module (~64 KB tổng). */
#define BACKUP_QUEUE_MAX 100
static backup_record_t backup_queue[BACKUP_QUEUE_MAX];
static int backup_head = 0, backup_tail = 0;
int backup_count = 0;
static uint32_t last_backup_time = 0;   /* giây trong ngày của lần backup gần nhất */
long flash_read_offset = 0;      /* vị trí đang đọc dở trong file backup trên flash */
BOOL flash_has_data    = FALSE;  /* cache: có file flash chưa gửi? Chỉ mở file khi cần thực sự */
BOOL flash_ok          = TRUE;   /* bit 1 trạng thái: FALSE khi lần ghi backup xuống flash gần nhất bị lỗi */

/* ===========================================================================
 * 6. HÀNG ĐỢI BACKUP (bộ đệm vòng RAM + archive xuống flash khi đầy)
 * =========================================================================== */

/** Lấy bản ghi cũ nhất ra khỏi hàng đợi RAM. @return FALSE nếu hàng đợi rỗng. */
BOOL tracker_backup_pop(backup_record_t *out)
{
    if (backup_count == 0) return FALSE;
    *out = backup_queue[backup_tail];
    backup_tail = (backup_tail + 1) % BACKUP_QUEUE_MAX;
    backup_count--;
    return TRUE;
}

/**
 * Ghi TOÀN BỘ hàng đợi RAM hiện tại xuống file flash trong một lần mở file (chế độ "ab" -
 * nối thêm vào cuối, giữ dữ liệu cũ nếu có từ lần tràn trước), rồi làm rỗng RAM để có chỗ
 * cho các bản ghi mới nhất tiếp theo. Chỉ gọi khi RAM đã đầy và vẫn đang mất kết nối.
 *
 * LUẦU Ý API SDK SIMCom: sAPI_fwrite/fread trả về số BYTES đã ghi/đọc (không phải số phần tử
 * như fwrite/fread chuẩn C). Phải so sánh với sizeof(rec), không phải với 1.
 *
 * Xử lý lỗi:
 *   - fopen lỗi: reset RAM để tránh vòng lặp vô hạn.
 *   - fwrite lỗi giữa chừng: trả bản ghi vừa pop về RAM, dừng, đóng file.
 *   - Ghi xong (n > 0): đặt flash_has_data = TRUE để vòng lặp session biết có flash cần gửi.
 */
static void tracker_flash_archive_ram(void)
{
    SCFILE *fp;
    backup_record_t rec;
    int n = 0;
    BOOL write_ok = TRUE;

    fp = sAPI_fopen(TRACKER_BACKUP_FLASH_PATH, "ab");
    if (fp == NULL) {
        sAPI_Debug("[NASA] Flash archive: khong mo duoc file, reset RAM tranh lap vo han");
        backup_head = backup_tail = backup_count = 0;
        flash_ok = FALSE;
        return;
    }

    while (tracker_backup_pop(&rec)) {
        /* sAPI_fwrite tra ve so BYTES ghi duoc, khong phai so phan tu */
        if (sAPI_fwrite(&rec, sizeof(rec), 1, fp) == (int)sizeof(rec)) {
            n++;
        } else {
            sAPI_Debug("[NASA] Flash archive: ghi loi sau %d ban - tra lai RAM", n);
            backup_queue[backup_head] = rec;
            backup_head = (backup_head + 1) % BACKUP_QUEUE_MAX;
            backup_count++;
            write_ok = FALSE;
            break;
        }
    }

    sAPI_fsync(fp);
    sAPI_fclose(fp);
    sAPI_Debug("[NASA] Flash archive: da luu %d ban xuong flash, RAM con lai=%d", n, backup_count);

    if (n > 0) {
        flash_has_data = TRUE;      /* bật cache: vòng lặp session sẽ gửi từ flash */
    } else {
        sAPI_Debug("[NASA] Flash archive: flash loi lien tuc, reset RAM");
        backup_head = backup_tail = backup_count = 0;
    }
    flash_ok = (n > 0 && write_ok) ? TRUE : FALSE;     /* ghi trọn lô thành công mới coi flash tốt */
}

/**
 * Kiểm tra còn bản ghi nào trong file flash chưa gửi không.
 * Dùng cache flash_has_data để tránh mở file mỗi 100 ms.
 * Nếu đã đọc hết thì xóa file và reset cache.
 */
BOOL tracker_flash_has_backup(void)
{
    SCFILE *fp;
    int size;

    if (!flash_has_data) return FALSE;      /* cache: không có file → không mở */

    fp = sAPI_fopen(TRACKER_BACKUP_FLASH_PATH, "rb");
    if (fp == NULL) {
        /* File bị xóa bên ngoài hoặc lỗi flash → reset cache */
        flash_has_data = FALSE;
        flash_read_offset = 0;
        return FALSE;
    }
    size = sAPI_fsize(fp);
    sAPI_fclose(fp);

    if (size <= 0 || (long)size <= flash_read_offset) {
        sAPI_remove(TRACKER_BACKUP_FLASH_PATH);
        flash_read_offset = 0;
        flash_has_data = FALSE;
        sAPI_Debug("[NASA] Flash: đã gửi hết, xóa file");
        return FALSE;
    }
    return TRUE;
}

/** Lấy 1 bản ghi kế tiếp từ file flash, đúng thứ tự đã lưu.
 * LUẦU Ý: sAPI_fread trả về số BYTES, không phải số phần tử! */
BOOL tracker_flash_pop_one(backup_record_t *out)
{
    SCFILE *fp = sAPI_fopen(TRACKER_BACKUP_FLASH_PATH, "rb");

    if (fp == NULL) {
        flash_has_data = FALSE;
        flash_read_offset = 0;
        return FALSE;
    }
    if (sAPI_fseek(fp, flash_read_offset, FS_SEEK_BEGIN) == 0 &&
        sAPI_fread(out, sizeof(*out), 1, fp) == (int)sizeof(*out)) {
        /* Đọc được 1 bản ghi (sizeof bytes) → tiến con trỏ */
        flash_read_offset += (long)sizeof(*out);
        sAPI_fclose(fp);
        return TRUE;
    }
    /* Đọc lỗi hoặc hết file: dọn sạch */
    sAPI_fclose(fp);
    sAPI_remove(TRACKER_BACKUP_FLASH_PATH);
    flash_read_offset = 0;
    flash_has_data = FALSE;
    sAPI_Debug("[NASA] Flash pop: doc loi, xoa file");
    return FALSE;
}

/** Thêm một bản ghi vào hàng đợi RAM; nếu đầy thì archive toàn bộ xuống flash trước. */
void tracker_backup_push(const char *dt, const tracker_location_t *loc, BOOL acc_on, double voltage)
{
    if (backup_count >= BACKUP_QUEUE_MAX) {
        tracker_flash_archive_ram();                     /* RAM đầy: lưu hết xuống flash, làm rỗng RAM */
    }
    snprintf(backup_queue[backup_head].datetime, sizeof(backup_queue[backup_head].datetime), "%s", dt);
    backup_queue[backup_head].loc    = *loc;
    backup_queue[backup_head].acc_on = acc_on;
    backup_queue[backup_head].voltage = voltage;
    backup_head = (backup_head + 1) % BACKUP_QUEUE_MAX;
    backup_count++;
}

/* ===========================================================================
 * 12. BACKUP KHI MẤT KẾT NỐI VÀ CHỜ CÓ GIỜ
 * =========================================================================== */

/**
 * Lưu vị trí vào hàng đợi backup khi đang mất kết nối.
 * Chu kỳ lưu phụ thuộc trạng thái ACC tại thời điểm gọi:
 *   ACC ON  → mỗi TRACKER_RUN_INTERVAL_SEC  (10 s)
 *   ACC OFF → mỗi TRACKER_STOP_INTERVAL_SEC (300 s)
 * Đảm bảo mật độ bản tin backup khớp đúng với chu kỳ NASA-2 thực tế,
 * tránh gửi bù quá dày hoặc quá thưa khi có lại kết nối.
 * Chỉ chạy khi: đã từng đăng nhập, hiện không có phiên, và đã có giờ hợp lệ.
 * Được gọi trong mọi vòng chờ (tracker_delay, chờ giờ, chờ ACK) để không bỏ sót dữ liệu.
 */
void tracker_process_backup(void)
{
    tracker_location_t loc;
    char dt[32];
    uint32_t now, delta;
    BOOL acc_on;
    int  backup_interval;

    if (is_logged_in || !has_logged_in_once) return;
    now = tracker_get_seconds_today();
    if (now == 0) return;

    /* Chu kỳ lưu backup = chu kỳ NASA-2 theo ACC hiện tại */
    acc_on          = tracker_get_acc_status();
    backup_interval = acc_on ? TRACKER_RUN_INTERVAL_SEC : TRACKER_STOP_INTERVAL_SEC;

    /* Khoảng cách với lần backup trước (xử lý qua nửa đêm) */
    delta = (now >= last_backup_time) ? (now - last_backup_time) : (now + 86400 - last_backup_time);
    if (delta < (uint32_t)backup_interval && last_backup_time != 0) return;

    if (tracker_get_location(&loc) && tracker_get_datetime(dt, sizeof(dt))) {
        tracker_backup_push(dt, &loc, acc_on, tracker_get_voltage());
        last_backup_time = now;
        sAPI_Debug("[NASA] Backup saved (ACC=%d, interval=%ds), queue=%d/%d",
                   acc_on, backup_interval, backup_count, BACKUP_QUEUE_MAX);
    }
}

/** Ngủ `seconds` giây, mỗi giây kiểm tra backup một lần (thay cho sleep thường). */
void tracker_delay(int seconds)
{
    while (seconds-- > 0) {
        sAPI_TaskSleep(SEC_TO_TICKS(1));
        tracker_alive();
        tracker_process_backup();
    }
}
