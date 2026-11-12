/**
 * @file    tracker_backup.c
 * @brief   Hàng đợi backup RAM + archive flash, lưu backup khi mất kết nối, tracker_delay.
 *          (Tách nguyên văn từ sc_application.c, không đổi logic.)
 */
#include "tracker_backup.h"
#include "tracker_led.h"
#include "tracker_time.h"
#include "tracker_sensor.h"
#include "tracker_sms.h"

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
volatile BOOL device_enabled = TRUE;  /* lệnh 38: FALSE = thiết bị tắt */

/* ===========================================================================
 * TRẠNG THÁI LƯU BỀN QUA KHỞI ĐỘNG LẠI (TRACKER_STATE_PATH)
 *   - msg_id: lưu "ID dùng được sau reboot" = ID đang dùng + TRACKER_MSG_ID_RESERVE_BLOCK; chỉ ghi lại
 *     khi đã dùng hết khối -> reboot trong cùng ngày tiếp tục từ ID này, không trùng ID đã gửi
 *     (có thể nhảy số tới TRACKER_MSG_ID_RESERVE_BLOCK).
 *   - flash_read_offset: ghi sau mỗi TRACKER_STATE_OFFSET_SAVE_EVERY bản lấy từ flash, và ngay khi
 *     file backup bị xóa (offset về 0) -> sau reboot gửi lặp tối đa (SAVE_EVERY - 1) bản NASA-7.
 * Chỉ task tracker gọi các hàm này.
 * =========================================================================== */
#define TRACKER_STATE_MAGIC  0x4E415331UL                 /* "NAS1": nhận diện file hợp lệ */

typedef struct {
    uint32_t magic;
    char     day[11];          /* "YYYY-MM-DD" ứng với msg_id_next */
    uint32_t msg_id_next;      /* ID dùng được sau khởi động lại (đã dành trước) */
    long     flash_offset;     /* vị trí đã gửi trong file backup */
} tracker_state_t;

static tracker_state_t state = { TRACKER_STATE_MAGIC, "", 0, 0 };
static int offset_unsaved = 0;                            /* số bản lấy từ flash chưa ghi vị trí */

/** Ghi toàn bộ trạng thái xuống flash. */
static void tracker_state_save(void)
{
    SCFILE *fp = sAPI_fopen(TRACKER_STATE_PATH, "wb");
    if (fp == NULL) {
        sAPI_Debug("[NASA] State: khong mo duoc file de ghi");
        return;
    }
    /* sAPI_fwrite trả về số BYTE đã ghi */
    if (sAPI_fwrite(&state, sizeof(state), 1, fp) != (int)sizeof(state)) {
        sAPI_Debug("[NASA] State: ghi loi");
    }
    sAPI_fsync(fp);
    sAPI_fclose(fp);
}

/** Đọc trạng thái đã lưu (nếu có và hợp lệ). */
static void tracker_state_load(void)
{
    tracker_state_t s;
    SCFILE *fp = sAPI_fopen(TRACKER_STATE_PATH, "rb");

    if (fp == NULL) {
        sAPI_Debug("[NASA] State: chua co file trang thai");
        return;
    }
    if (sAPI_fread(&s, sizeof(s), 1, fp) == (int)sizeof(s) && s.magic == TRACKER_STATE_MAGIC) {
        s.day[sizeof(s.day) - 1] = '\0';
        state = s;
        sAPI_Debug("[NASA] State: day=%s msg_id_next=%lu flash_offset=%ld",
                   state.day, (unsigned long)state.msg_id_next, state.flash_offset);
    } else {
        sAPI_Debug("[NASA] State: file trang thai loi, bo qua");
    }
    sAPI_fclose(fp);
}

/** Vị trí đọc đã lưu, nếu còn hợp lệ với file backup hiện tại (không vượt kích thước, đúng biên bản ghi). */
static long tracker_state_flash_offset_resume(void)
{
    SCFILE *fp;
    int size;
    long off = state.flash_offset;

    if (off <= 0) return 0;
    fp = sAPI_fopen(TRACKER_BACKUP_FLASH_PATH, "rb");
    if (fp == NULL) return 0;
    size = sAPI_fsize(fp);
    sAPI_fclose(fp);

    if (off > (long)size || off % (long)sizeof(backup_record_t) != 0) {
        sAPI_Debug("[NASA] State: offset %ld khong hop le (size=%d) -> doc tu dau", off, size);
        return 0;
    }
    return off;
}

/** Ghi vị trí đọc file backup: ngay (force) hoặc sau mỗi TRACKER_STATE_OFFSET_SAVE_EVERY lần đọc. */
static void tracker_state_offset_note(BOOL force)
{
    if (!force && ++offset_unsaved < TRACKER_STATE_OFFSET_SAVE_EVERY) return;
    offset_unsaved = 0;
    if (state.flash_offset == flash_read_offset) return;
    state.flash_offset = flash_read_offset;
    tracker_state_save();
}

uint32_t tracker_state_msg_id_resume(const char *datetime)
{
    if (datetime == NULL || state.day[0] == '\0' || strncmp(state.day, datetime, 10) != 0) return 0;
    return state.msg_id_next;
}

void tracker_state_msg_id_reserve(const char *datetime, uint32_t msg_id)
{
    if (datetime == NULL || strlen(datetime) < 10) return;
    if (strncmp(state.day, datetime, 10) == 0 && msg_id < state.msg_id_next) return;  /* còn trong khối đã dành */

    memcpy(state.day, datetime, 10);
    state.day[10] = '\0';
    state.msg_id_next = msg_id + TRACKER_MSG_ID_RESERVE_BLOCK;
    tracker_state_save();
}

/* ===========================================================================
 * LỆNH 38: TRẠNG THÁI TẮT/MỞ THIẾT BỊ (TRACKER_DEVICE_STATE_PATH, 1 byte '0'/'1')
 * Không có file hoặc file lỗi -> coi là mở (mặc định 1 theo bảng lệnh).
 * =========================================================================== */

/** Đọc trạng thái tắt/mở đã lưu. */
static void tracker_device_state_load(void)
{
    char c = '1';
    SCFILE *fp = sAPI_fopen(TRACKER_DEVICE_STATE_PATH, "rb");

    if (fp == NULL) return;                               /* chưa từng cấu hình: mở */
    if (sAPI_fread(&c, 1, 1, fp) == 1 && c == '0') device_enabled = FALSE;
    sAPI_fclose(fp);
    sAPI_Debug("[NASA] Device state: %s", device_enabled ? "ON" : "OFF (lenh 38)");
}

BOOL tracker_device_set_enabled(BOOL enabled)
{
    char c = enabled ? '1' : '0';
    BOOL ok;
    SCFILE *fp;

    device_enabled = enabled;
    fp = sAPI_fopen(TRACKER_DEVICE_STATE_PATH, "wb");
    if (fp == NULL) {
        sAPI_Debug("[NASA] Device state: khong mo duoc file de ghi");
        return FALSE;
    }
    /* sAPI_fwrite trả về số BYTE đã ghi */
    ok = (sAPI_fwrite(&c, 1, 1, fp) == 1) ? TRUE : FALSE;
    sAPI_fsync(fp);
    sAPI_fclose(fp);
    sAPI_Debug("[NASA] Device state -> %s, luu %s", enabled ? "ON" : "OFF", ok ? "OK" : "FAIL");
    return ok;
}

void tracker_backup_startup(void)
{
    /* Log dung lượng ổ C:/ (LittleFS) 1 lần khi khởi động, để đánh giá độ mòn flash.
     * In dạng unsigned long (%lu): sAPI_Debug hỗ trợ %lld hay không là CHƯA XÁC MINH. */
    {
        char disc[] = "C:/";
        long long total = sAPI_GetSize(disc);
        long long free_sz = sAPI_GetFreeSize(disc);
        long long used = sAPI_GetUsedSize(disc);
        sAPI_Debug("[NASA] Flash C: total=%lu B, free=%lu B, used=%lu B",
                   (unsigned long)total, (unsigned long)free_sz, (unsigned long)used);
    }

    tracker_state_load();
    tracker_device_state_load();

    /* Startup probe: kiểm tra file backup còn sót từ lần reset trước.
     * sAPI_access() trả 0 nếu file tồn tại. Nếu có → bật cache để gửi lại khi có mạng. */
    if (sAPI_access(TRACKER_BACKUP_FLASH_PATH, 0) == 0) {
        flash_has_data    = TRUE;
        flash_read_offset = tracker_state_flash_offset_resume();
        sAPI_Debug("[NASA] Startup: phat hien file backup cu tren flash (offset=%ld), se gui lai khi co mang",
                   flash_read_offset);
    } else if (state.flash_offset != 0) {
        state.flash_offset = 0;                           /* không còn file: vị trí cũ không còn ý nghĩa */
        tracker_state_save();
    }
}

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
 * nối thêm vào cuối, giữ dữ liệu cũ nếu có), rồi làm rỗng RAM.
 * Được gọi theo lô nhỏ (RAM đủ TRACKER_BACKUP_FLUSH_EVERY bản) và khi RAM đầy (BACKUP_QUEUE_MAX).
 *
 * LUẦU Ý API SDK SIMCom: sAPI_fwrite/fread trả về số BYTES đã ghi/đọc (không phải số phần tử
 * như fwrite/fread chuẩn C). Phải so sánh với sizeof(rec), không phải với 1.
 *
 * Xử lý lỗi:
 *   - fopen lỗi / không ghi được bản nào: drop_on_fail = TRUE (RAM đầy, cần chỗ) thì reset RAM để
 *     tránh vòng lặp vô hạn; FALSE (ghi theo lô) thì giữ nguyên RAM, thử lại ở lô sau.
 *   - fwrite lỗi giữa chừng: trả bản ghi vừa pop về RAM, dừng, đóng file.
 *   - Ghi xong (n > 0): đặt flash_has_data = TRUE để vòng lặp session biết có flash cần gửi.
 */
static void tracker_flash_archive_ram(BOOL drop_on_fail)
{
    SCFILE *fp;
    backup_record_t rec;
    int n = 0;
    BOOL write_ok = TRUE;

    fp = sAPI_fopen(TRACKER_BACKUP_FLASH_PATH, "ab");
    if (fp == NULL) {
        flash_ok = FALSE;
        if (drop_on_fail) {
            sAPI_Debug("[NASA] Flash archive: khong mo duoc file, reset RAM tranh lap vo han");
            backup_head = backup_tail = backup_count = 0;
        } else {
            sAPI_Debug("[NASA] Flash archive: khong mo duoc file, giu lai %d ban trong RAM", backup_count);
        }
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
    } else if (drop_on_fail) {
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
        tracker_state_offset_note(TRUE);
        return FALSE;
    }
    size = sAPI_fsize(fp);
    sAPI_fclose(fp);

    if (size <= 0 || (long)size <= flash_read_offset) {
        sAPI_remove(TRACKER_BACKUP_FLASH_PATH);
        flash_read_offset = 0;
        flash_has_data = FALSE;
        tracker_state_offset_note(TRUE);
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
        tracker_state_offset_note(TRUE);
        return FALSE;
    }
    if (sAPI_fseek(fp, flash_read_offset, FS_SEEK_BEGIN) == 0 &&
        sAPI_fread(out, sizeof(*out), 1, fp) == (int)sizeof(*out)) {
        /* Đọc được 1 bản ghi (sizeof bytes) → tiến con trỏ */
        flash_read_offset += (long)sizeof(*out);
        sAPI_fclose(fp);
        tracker_state_offset_note(FALSE);                 /* ghi flash sau mỗi TRACKER_STATE_OFFSET_SAVE_EVERY bản */
        return TRUE;
    }
    /* Đọc lỗi hoặc hết file: dọn sạch */
    sAPI_fclose(fp);
    sAPI_remove(TRACKER_BACKUP_FLASH_PATH);
    flash_read_offset = 0;
    flash_has_data = FALSE;
    tracker_state_offset_note(TRUE);
    sAPI_Debug("[NASA] Flash pop: doc loi, xoa file");
    return FALSE;
}

/**
 * Thêm một bản ghi vào hàng đợi RAM.
 *   - RAM đầy: archive toàn bộ xuống flash trước (lỗi flash thì bỏ RAM để có chỗ).
 *   - Sau khi thêm, RAM đủ TRACKER_BACKUP_FLUSH_EVERY bản: ghi cả lô xuống flash (lỗi thì giữ RAM),
 *     để mất nguồn chỉ mất các bản chưa đủ lô. Thứ tự gửi vẫn đúng: session gửi flash (cũ) trước, RAM (mới) sau.
 */
void tracker_backup_push(const char *dt, const tracker_location_t *loc, BOOL acc_on, double voltage)
{
    if (backup_count >= BACKUP_QUEUE_MAX) {
        tracker_flash_archive_ram(TRUE);                 /* RAM đầy: lưu hết xuống flash, làm rỗng RAM */
    }
    snprintf(backup_queue[backup_head].datetime, sizeof(backup_queue[backup_head].datetime), "%s", dt);
    backup_queue[backup_head].loc    = *loc;
    backup_queue[backup_head].acc_on = acc_on;
    backup_queue[backup_head].voltage = voltage;
    backup_head = (backup_head + 1) % BACKUP_QUEUE_MAX;
    backup_count++;

    if (backup_count >= TRACKER_BACKUP_FLUSH_EVERY) {
        tracker_flash_archive_ram(FALSE);                /* ghi theo lô nhỏ, lỗi thì giữ RAM thử lại lô sau */
    }
}

/* ===========================================================================
 * 12. BACKUP KHI MẤT KẾT NỐI VÀ CHỜ CÓ GIỜ
 * =========================================================================== */

/**
 * Lưu vị trí vào hàng đợi backup khi đang mất kết nối. Kiểm tra mỗi giây, lưu khi:
 *   - bản đầu tiên (chưa lưu bản nào), hoặc
 *   - ACC đổi trạng thái so với bản trước, hoặc
 *   - đã qua đủ chu kỳ theo ACC: TRACKER_BACKUP_ACC_ON_SEC (20 s) / TRACKER_BACKUP_ACC_OFF_SEC (300 s).
 * Chỉ chạy khi: đã từng đăng nhập, hiện không có phiên, và đã có giờ hợp lệ.
 * Được gọi trong mọi vòng chờ (tracker_delay, chờ giờ, chờ ACK) để không bỏ sót dữ liệu.
 */
void tracker_process_backup(void)
{
    static int last_acc = -1;                     /* ACC của bản gần nhất, -1 = chưa có */
    tracker_location_t loc;
    char dt[32];
    uint32_t now, delta;
    BOOL acc_on;
    const char *reason;

    if (is_logged_in || !has_logged_in_once || !device_enabled) return;   /* thiết bị tắt (lệnh 38): không lưu */
    now = tracker_get_seconds_today();
    if (now == 0) return;

    acc_on = tracker_get_acc_status();

    /* Khoảng thời gian với lần backup trước (xử lý qua nửa đêm) */
    delta = (now >= last_backup_time) ? (now - last_backup_time) : (now + 86400 - last_backup_time);

    if (last_backup_time == 0)                          reason = "first";
    else if (last_acc >= 0 && (int)acc_on != last_acc)  reason = "acc";
    else if (delta >= (uint32_t)(acc_on ? TRACKER_BACKUP_ACC_ON_SEC : TRACKER_BACKUP_ACC_OFF_SEC))
                                                        reason = acc_on ? "acc_on" : "acc_off";
    else return;

    if (tracker_get_location(&loc) && tracker_get_datetime(dt, sizeof(dt))) {
        tracker_backup_push(dt, &loc, acc_on, tracker_get_voltage());
        last_backup_time = now;
        last_acc = (int)acc_on;
        sAPI_Debug("[NASA] Backup saved (%s, ACC=%d, fix=%d, dt=%lus), queue=%d/%d",
                   reason, acc_on, loc.fixValid, (unsigned long)delta, backup_count, BACKUP_QUEUE_MAX);
    }
}

/** Ngủ `seconds` giây, mỗi giây kiểm tra backup một lần (thay cho sleep thường). */
void tracker_delay(int seconds)
{
    while (seconds-- > 0) {
        sAPI_TaskSleep(SEC_TO_TICKS(1));
        tracker_alive();
        tracker_process_backup();
        tracker_sms_poll();                                /* lệnh cấu hình qua SMS, kể cả khi mất kết nối server */
    }
}
