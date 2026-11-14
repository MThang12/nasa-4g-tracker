#ifndef TRACKER_BACKUP_H
#define TRACKER_BACKUP_H

/**
 * @file    tracker_backup.h
 * @brief   Hàng đợi backup RAM (100 bản) + archive xuống flash khi đầy,
 *          lưu backup khi mất kết nối, tracker_delay (ngủ có backup).
 */
#include "tracker_common.h"

/* Trạng thái backup dùng chung với vòng phiên làm việc và trường status */
extern int  backup_count;      /* số bản ghi đang có trong hàng đợi RAM */
extern long flash_read_offset; /* vị trí đang đọc dở trong file backup trên flash */
extern BOOL flash_has_data;    /* cache: có file flash chưa gửi? */
extern BOOL flash_ok;          /* bit 1 trạng thái: FALSE khi lần ghi flash gần nhất bị lỗi */

/* Lệnh 38: TRUE = thiết bị hoạt động, FALSE = tắt (ngừng server/GPS/backup, chỉ nhận SMS).
 * Chỉ task tracker ghi (qua tracker_device_set_enabled). */
extern volatile BOOL device_enabled;

/** Lệnh 38: đặt trạng thái tắt/mở và lưu xuống TRACKER_DEVICE_STATE_PATH. @return FALSE nếu ghi file lỗi. */
BOOL tracker_device_set_enabled(BOOL enabled);

/**
 * Gọi 1 lần khi task tracker khởi động: đọc trạng thái lưu bền (TRACKER_STATE_PATH, TRACKER_DEVICE_STATE_PATH),
 * phát hiện file backup còn sót và khôi phục vị trí đọc đã gửi.
 */
void tracker_backup_startup(void);

/** ID đã dành trước trên flash nếu cùng ngày với datetime ("YYYY-MM-DD ..."), ngược lại 0. */
uint32_t tracker_state_msg_id_resume(const char *datetime);

/** Gọi trước khi dùng msg_id: hết khối ID đã dành (hoặc sang ngày mới) thì ghi khối mới xuống flash. */
void tracker_state_msg_id_reserve(const char *datetime, uint32_t msg_id);

/** Lấy bản ghi cũ nhất ra khỏi hàng đợi RAM. @return FALSE nếu hàng đợi rỗng. */
BOOL tracker_backup_pop(backup_record_t *out);

/** Còn bản ghi nào trong file flash chưa gửi không (đọc hết thì xóa file). */
BOOL tracker_flash_has_backup(void);

/** Lấy 1 bản ghi kế tiếp từ file flash, đúng thứ tự đã lưu. */
BOOL tracker_flash_pop_one(backup_record_t *out);

/** Thêm một bản ghi vào hàng đợi RAM; nếu đầy thì archive toàn bộ xuống flash trước. */
void tracker_backup_push(const char *dt, const tracker_location_t *loc, BOOL acc_on, double voltage);

/** Ghi các bản ghi còn trong RAM xuống flash (trước khi reset bằng lệnh 9). */
void tracker_backup_flush_ram(void);

/** Lưu vị trí vào backup khi đang mất kết nối (chỉ chạy sau khi đã đăng nhập ít nhất 1 lần). */
void tracker_process_backup(void);

/** Ngủ `seconds` giây, mỗi giây báo sống và kiểm tra backup một lần. */
void tracker_delay(int seconds);

#endif /* TRACKER_BACKUP_H */
