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

/** Lấy bản ghi cũ nhất ra khỏi hàng đợi RAM. @return FALSE nếu hàng đợi rỗng. */
BOOL tracker_backup_pop(backup_record_t *out);

/** Còn bản ghi nào trong file flash chưa gửi không (đọc hết thì xóa file). */
BOOL tracker_flash_has_backup(void);

/** Lấy 1 bản ghi kế tiếp từ file flash, đúng thứ tự đã lưu. */
BOOL tracker_flash_pop_one(backup_record_t *out);

/** Thêm một bản ghi vào hàng đợi RAM; nếu đầy thì archive toàn bộ xuống flash trước. */
void tracker_backup_push(const char *dt, const tracker_location_t *loc, BOOL acc_on, double voltage);

/** Lưu vị trí vào backup khi đang mất kết nối (chỉ chạy sau khi đã đăng nhập ít nhất 1 lần). */
void tracker_process_backup(void);

/** Ngủ `seconds` giây, mỗi giây báo sống và kiểm tra backup một lần. */
void tracker_delay(int seconds);

#endif /* TRACKER_BACKUP_H */
