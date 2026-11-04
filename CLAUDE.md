# CLAUDE.md — NASA-4G Tracker (SIMCom A7677S OpenSDK)

Tài liệu hướng dẫn cho Claude Code khi làm việc trong repository này.
Quy ước: mục nào ghi **CHƯA XÁC MINH** nghĩa là chưa kiểm chứng được từ mã nguồn, header, cấu hình build hay tài liệu có trong repo. Không được coi đó là sự thật khi viết code.

---

## 1. PROJECT OVERVIEW

Firmware cho thiết bị giám sát hành trình (GPS tracker) dùng module SIMCom A7677S. Code chạy trực tiếp trên module theo mô hình SIMCom OpenSDK, nên không có MCU ngoài.

Chức năng hiện có, theo `SIMCOM_SDK_SET/sc_application.c`:
- Đọc vị trí GNSS (GPS + BDS + QZSS, tốc độ cập nhật 1 Hz), trạng thái ACC (khóa xe), CSQ, điện áp nguồn xe qua ADC.
- Kết nối TCP tới server và chạy giao thức **NASA v1.3**:
  - `!NASA,<type>,<msg_id>,<fields...>*<checksum>\r\n`, trong đó checksum là tổng giá trị byte của phần đứng trước `*`.
  - NASA-1: đăng nhập. NASA-2: hành trình realtime. NASA-7: gửi bù dữ liệu backup.
- Chu kỳ gửi NASA-2: ACC bật thì 10 s/lần, ACC tắt thì 300 s/lần. Gửi ngay khi ACC đổi trạng thái.
- Khi mất kết nối: lưu tối đa 100 bản ghi trong RAM. RAM đầy thì ghi cả lô xuống `C:/nasa_backup.dat`. Có mạng lại thì gửi bù bằng NASA-7.
- Tự phục hồi: reset RF (`AT+CFUN=0/1`) khi không đăng ký được mạng, tự kết nối lại TCP.
- Đồng bộ giờ theo thứ tự GNSS → RTC (`AT+CCLK`) → giờ mạng → NTP (`pool.ntp.org`).
- 3 LED trạng thái: PWR, NET, GNSS.

Thông số server, biển số, mã đăng nhập và phiên bản đang hard-code ở `sc_application.c` (khoảng dòng 110–194).

## 2. HARDWARE PLATFORM

| Mục | Giá trị | Nguồn / trạng thái |
|---|---|---|
| Module | SIMCom A7677S | Tên biến thể trong `kernel/`, `sc_lib/` |
| Chipset | ASR 1606 (LTE Cat-1) | Tên target `..._1606_...`, `aboot/images/ASR1606C_*` |
| Build target đang dùng | `A7677S_MANV_1606_V702_OPENSDK` | Chỉ thư mục này có trong `out/` |
| Các biến thể khác trong SDK | `A7677S_LANS/LANV/MANS_1606_V702_OPENSDK` | `kernel/`, `sc_lib/` |
| CPU | Toolchain biên dịch với `-mcpu=cortex-r4 -mthumb -mfloat-abi=soft` | `config/ToolChain.cmake:17-18`. "Cortex-R5" theo thông tin dự án: **CHƯA XÁC MINH** trong repo |
| RAM userspace | 256 KB (`ORIGIN 0x7e052000, LENGTH 0x40000`) | `config/A7677S_MANV_1606_V702_OPENSDK_linkscript.ld` |
| Bản build gần nhất | text ≈ 197 KB, data ≈ 5 KB, bss ≈ 35 KB | `arm-none-eabi-size customer_app.elf` |

Ngoại vi do firmware sử dụng (lấy từ code; comment ghi "sơ đồ RV26"). **Phần cứng thực tế: CHƯA XÁC MINH.** Repo không có schematic của board.

| Chức năng | Chân / kênh | Ghi chú |
|---|---|---|
| LED NET | GPIO 32 | Output, mức 1 = sáng |
| LED PWR | GPIO 80 | Output |
| LED GNSS | GPIO 74 | Output |
| DOUT | GPIO 73 | Đã khai báo, chưa dùng |
| ACC | GPIO 37 | Input pull-up, mức thấp = ACC bật |
| Điện áp xe | ADC kênh 1 (`sAPI_ReadAdc(1)`) | Cầu chia R306 220K / R307 10K, hệ số 23. Kênh ADC và đơn vị trả về (mV): **CHƯA XÁC MINH** |
| Nguồn anten GNSS | `AT+CVAUX=1` | |
| UART1 | 115200 8N1 | Hiện thuộc menu demo SIMCom (`SIMCOM_UI_DEMO_TO_UART1_PORT`), tracker không dùng |

Tài liệu phần cứng trong `Doc/hardware/` là của A7630C/A7670C/A7680C. **Repo không có tài liệu phần cứng riêng cho A7677S.**

## 3. SDK ARCHITECTURE

SDK: `22110B01V06A7677MxA_SDK_240721`. Bản build trước đó được tạo tại `E:/22110B01V06A7677MxA_SDK_240721/` (đường dẫn này còn nằm trong `out/.../build.ninja`).

```
SIMCOM_SDK_SET/
├── Makefile, GNUmake.exe      Điểm vào build (make → cmake → ninja)
├── CMakeLists.txt             Chọn source app, link sc_lib/sc_demo/third libs
├── sc_application.c           ★ Firmware tracker (được build khi PROTOCOL_V1)
├── sc_main.c                  Entry kiểu protocol V2 (không dùng)
├── config/                    .config, Kconfig, userspaceConfig.h(.in), linkscript, ToolChain.*
├── kernel/<target>/           cp.bin, KernelDefinitions.*, aboot (prebuilt của SIMCom)
├── sc_lib/inc/                Header dùng chung (simcom_tcpip_old.h, GPIO/, mbedtls/, ...)
├── sc_lib/<target>/inc/       ★ Header API sAPI_* cho từng target
├── sc_lib/<target>/lib/       libsc_lib.a (thư viện đóng)
├── sc_lib_v2/                 Thư viện protocol V2 (không dùng)
├── sc_demo/src/               ★ Demo API chính thức: demo_gps.c, demo_tcpip.c, demo_uart.c, demo_file_system.c, demo_pm.c, ...
├── sc_demo_v2/                Demo V2 (không dùng)
├── examples/                  uart/, mqtt/, ssl/
├── old_fw_demo/               Được build thành cuslibold_fw_demo.a (qua old_fw_Makefile)
├── public_libs/               cjson, zlib, miracl, sm2 (header + .a)
├── tools/win32|linux/         cmake 3.22, ninja, gcc-arm-none-eabi 9.2.1, crc_set, 7z, aboot
└── out/<target>/              Kết quả build
```

Header API chính, nằm trong `sc_lib/A7677S_MANV_1606_V702_OPENSDK/inc/`:
`simcom_os.h` (task, sleep, malloc), `simcom_gps.h`, `simcom_tcpip.h`, `scfw_socket.h`, `scfw_netdb.h`, `scfw_inet.h`, `simcom_uart.h` (UART + `sAPI_SendATCMDWaitResp`), `simcom_gpio.h`, `simcom_file.h`, `sc_power.h` (ADC), `simcom_network.h`, `simcom_simcard.h`, `simcom_rtc.h`, `simcom_wtd.h`, `simcom_ntp_client.h`, `api_map.h`.

Tài liệu API (PDF tiếng Trung) nằm trong `Doc/software/`, ví dụ `A76xx Series Open SDK_GNSS_...`, `..._TCP UDP...`, `..._UART...`, `A76xx_R3_Series_OpenSDK_软件用户指南_V1.04.pdf`. Ngoài ra có `Doc/software/A76xx_Series_OpenSDK_Q&A.md`.

Cấu hình build hiện tại (`config/.config`, `config/userspaceConfig.h`):
- `CONFIG_SIMCOM_PROTOCOL_V1=y`, nên build `sc_application.c`.
- `HAS_DEMO`, `HAS_UART`, `HAS_URC`, `HAS_USB`, `HAS_FS`, `HAS_TCPIP` và nhiều `HAS_DEMO_*` đang **bật**. Toàn bộ `sc_demo` được link và chạy cùng tracker.

## 4. FIRMWARE ARCHITECTURE

Số dòng tham chiếu theo `SIMCOM_SDK_SET/sc_application.c`. Số dòng có thể lệch sau khi sửa file, nên tìm lại theo tên hàm.

**Đăng ký và entry point**
- `userSpaceEntry` được đặt trong section `.userSpaceRegTable`, stack 10 KB, priority 30 (cuối file). Kernel dùng bảng này để gọi `userSpace_Main(void *arg)`.

**`userSpace_Main`**
1. `ApiMapInit(arg)`.
2. `tracker_acc_init()`.
3. Tạo task `sTask_LedControl` (stack 4 KB, prio 105) và `sTask_NasaTracker` (stack 12 KB, prio 100).
4. Do `HAS_DEMO` đang bật: sleep 5 s, rồi chạy URC task, USB VCOM, HelloWorld, `sDemo_UartInit()` và `sAPP_SimcomUIDemo()` trên UART1.

**Task `sTask_LedControl`**
- Chu kỳ 100 ms. Đọc biến `volatile led_net_mode` / `led_gnss_mode` để nháy LED.

**Task `sTask_NasaTracker`**
- Chờ 5 s → `sAPI_NetworkInit()` → `tracker_gnss_init()` → kiểm tra file backup cũ.
- Vòng lặp vô hạn:
  1. `tracker_wait_network`: CPIN, CEREG/CGREG; lỗi 15 lần thì CFUN=0/1.
  2. `tracker_activate_pdp`: `sAPI_TcpipPdpActive(1, 1)`.
  3. `tracker_tcp_connect`: `getaddrinfo` / `socket` / `connect` (blocking).
  4. `tracker_login`: chờ có giờ, gửi NASA-1, chờ ACK 15 s bằng `select`.
  5. `tracker_run_session`: vòng 100 ms gồm poll socket → theo dõi GNSS → gửi 1 bản NASA-7 → NASA-2 khi đến hạn.
  6. Mất kết nối thì `tracker_delay(5)` rồi lặp lại.

**Các thành phần khác**
- **Backup:** `tracker_process_backup` được gọi trong mọi vòng chờ. Nó chỉ chạy sau khi đã đăng nhập ít nhất 1 lần và đang mất phiên.
- **GNSS:** poll `sAPI_GnssNavdataGet()` (trả `SC_GNSSINFO *`, các trường là chuỗi NMEA). Callback `sAPI_GnssStateBindCB` đang để trống. Đơn vị `info->speed` (km/h hay knot): **CHƯA XÁC MINH**, code hiện coi là km/h.
- **AT:** dùng `sAPI_SendATCMDWaitResp` trên kênh 10. Demo UART cũng dùng kênh 10.
- **UART:** tracker không tự cấu hình UART. Callback UART/URC thuộc phần demo (`sc_demo/src/uart_api.c`, `cus_urc.c`).
- **TCP/IP:** socket lwIP kiểu BSD, blocking. Theo `scfw_socket_define.h`, `SO_SNDTIMEO` chưa được hỗ trợ. Dữ liệu server gửi xuống trong phiên hiện bị bỏ qua.
- **Thời gian:** `sAPI_TaskSleep(200)` = 1 giây (1 tick = 5 ms).

**Vấn đề đã biết** (từ lần phân tích trước; chưa sửa, chỉ sửa khi được yêu cầu):
- RTC được ghi giờ UTC nhưng gắn hậu tố `+28`, trong khi nguồn GNSS trả UTC+7. Khi mất GNSS, timestamp có thể lệch 7 giờ.
- `HAS_DEMO` bật trong firmware sản phẩm.
- Chưa có watchdog.
- `flash_read_offset` không được lưu bền, nên sau khi reboot có thể gửi trùng NASA-7.
- `heading` và `odometer` luôn bằng 0.
- Task priority 100/105, trong khi `Doc/software/A76xx_Series_OpenSDK_Q&A.md` khuyến nghị 150–250 (hệ thống dùng < 120).

## 5. CODING RULES

- **Chuẩn C:** C99 (`-std=c99 -fgnu89-inline -Os -Wall`), GCC arm-none-eabi 9.2.1.
  - Code phải biên dịch **không có warning**. `sc_demo` đang bật `-Werror`, và Q&A của SIMCom yêu cầu không để warning.
  - Không include `unistd.h`: nó gây trùng định nghĩa `close` với `scfw_socket.h` (theo Q&A).
- **Chỉ dùng API đã xác minh** trong `sc_lib/A7677S_MANV_1606_V702_OPENSDK/inc/*.h`, `sc_lib/inc/` hoặc trong `sc_demo/src/`.
  - Trước khi dùng một API mới, mở header để kiểm tra prototype, kiểu tham số và ý nghĩa giá trị trả về. Nếu có, tham khảo cách demo dùng nó.
  - Không đoán tên API, enum, macro hay kiểu dữ liệu. Nếu không tìm thấy thì báo "CHƯA XÁC MINH" và hỏi lại.
  - Lưu ý đã xác minh: `sAPI_fread` / `sAPI_fwrite` trả về **số byte**, không phải số phần tử (`simcom_file.h`).
- **RAM / stack:**
  - Tổng RAM userspace là 256 KB, dùng chung với demo khi `HAS_DEMO` bật.
  - Stack task tracker 12 KB, LED 4 KB. Không đặt buffer lớn trên stack. Ước lượng stack khi thêm buffer cục bộ.
  - Sau khi build, kiểm tra `.bss`/`.data` bằng `arm-none-eabi-size` và file `.map`.
- **Bộ nhớ động:**
  - Tránh `malloc` / `sAPI_Malloc` (là macro tới `malloc`, xem `simcom_os.h`). Ưu tiên buffer tĩnh có kích thước cố định.
  - Nếu bắt buộc phải cấp phát động, phải giải phóng đúng chỗ và kiểm tra NULL.
- **An toàn chuỗi và buffer:** dùng `snprintf`, luôn giới hạn độ dài, luôn kết thúc chuỗi bằng `\0`, kiểm tra giá trị trả về của `sscanf`.
- **GNSS:** kiểm tra con trỏ NULL và `fixValid` trước khi dùng tọa độ. Struct navdata do driver cập nhật đồng thời, nên copy ra trước khi parse.
- **TCP:** luôn kiểm tra giá trị trả về của `send` / `recv` / `select`. Đóng socket khi lỗi. Không để vòng lặp chặn vô hạn.
- **UART / AT:** kiểm tra mã trả về, giới hạn timeout, đặt kích thước buffer phản hồi đủ lớn. Lưu ý xung đột kênh AT 10 với demo.
- **Task:**
  - Mọi vòng `while(1)` phải có `sAPI_TaskSleep`.
  - Biến chia sẻ giữa các task phải là `volatile` hoặc được bảo vệ.
  - Muốn xóa task thì hàm task phải thoát vòng lặp và return trước (Q&A).
- **File:** gọi `sAPI_fsync` trước `sAPI_fclose` khi cần dữ liệu bền qua mất nguồn. Đóng file ngay sau khi dùng.
- **Không thay đổi chức năng đang hoạt động** nếu không có yêu cầu: giao thức NASA, chu kỳ gửi, chân GPIO, thông số server.
- Giữ phong cách hiện tại: tiền tố `tracker_`, hằng số `TRACKER_*`, comment tiếng Việt, log qua `sAPI_Debug("[NASA] ...")`.

## 6. FILE MODIFICATION RULES

- **Giai đoạn đầu chỉ được sửa `SIMCOM_SDK_SET/sc_application.c`.** Đây là file thực sự được build (`CMakeLists.txt`, `SET(app_src sc_application.c)`).
- Bản `sc_application.c` cũ ở thư mục gốc repo (không được build) **đã được người dùng xóa** ngày 04/11/2026 vì là file thừa. Bản này vẫn còn trong commit `bd25f7b` nếu cần tham khảo. Không tạo lại file `sc_application.c` ở thư mục gốc.
- **Không sửa vendor SDK:** `sc_lib/`, `sc_lib_v2/`, `sc_demo/`, `sc_demo_v2/`, `kernel/`, `public_libs/`, `examples/`, `old_fw_demo/`, `tools/`, `sc_main.c`.
- **Không sửa build system:** `Makefile`, `CMakeLists.txt`, `old_fw_Makefile`, `makeDepend.mak`, `config/*` (gồm `.config`, `userspaceConfig.h`, linker script `*.ld`, `ToolChain.*`), `out/**/build.ninja`.
- Không xóa, đổi tên hay di chuyển file.
- Nếu cần sửa file khác (ví dụ tắt `HAS_DEMO` trong `config/.config`): **giải thích lý do và chờ người dùng đồng ý** trước.
- `CLAUDE.md` chỉ được cập nhật khi người dùng yêu cầu.

## 7. BUILD AND VALIDATION

> **Không tự chạy build khi chưa được yêu cầu.**

Lệnh build, dựa trên `Makefile` và `tools/script/env.mak`; trên Windows dùng ninja + cmake trong `tools/win32`:
```
cd SIMCOM_SDK_SET
GNUmake.exe A7677S_MANV_1606_V702_OPENSDK          # build app + đóng gói burn.zip
GNUmake.exe A7677S_MANV_1606_V702_OPENSDK_remake   # clean rồi build lại
```

Chuỗi xử lý: Makefile → CMake sinh `out/<target>/build.ninja` → ninja → `customer_app.elf` → `objcopy -O binary` → `crc_set` → `burn.zip`.

Đầu ra trong `SIMCOM_SDK_SET/out/A7677S_MANV_1606_V702_OPENSDK/`:
- `customer_app.elf`, `customer_app.elf.map`, `customer_app.bin` (đã gắn CRC)
- `burn.zip`: gói để nạp bằng công cụ Aboot (`tools/win32/aboot`, xem `Doc/software/A76xx_Series_Open_SDK_Debug_and_Download_Application_Note_V1.00.06.pdf`)
- `burn_factory.zip`

Lưu ý về build:
- `out/<target>/build.ninja` và `CMakeCache.txt` đang chứa đường dẫn tuyệt đối `E:/22110B01V06A7677MxA_SDK_240721/...`. Build lại mà không tạo lại thư mục `out/<target>` thì có thể biên dịch **nhầm file ở thư mục cũ**. Chỉ xử lý việc này khi được đồng ý.
- Q&A của SIMCom: sau khi đưa SDK lên git, timestamp file thay đổi và có thể kích hoạt sai các bước trong Makefile.
- `tools/win32/msys64.zip` và `cross_tool.zip` nằm trong `.gitignore`. Trạng thái giải nén trên máy build: **CHƯA XÁC MINH** (hiện có `tools/win32/cross_tool/gcc-arm-none-eabi`).

Kiểm tra sau khi build (khi được phép):
1. Build không có warning hay error.
2. `tools/win32/cross_tool/gcc-arm-none-eabi/bin/arm-none-eabi-size.exe out/.../customer_app.elf`: so sánh bss/data với bản trước (≈ 35 KB / 5 KB).
3. Kiểm tra chuỗi phiên bản trong `.bin` khớp `TRACKER_FW_VERSION`.
4. Chạy thử trên thiết bị và đọc log `sAPI_Debug` (CATStudio/USB, theo comment trong `sc_main.c`). Quy trình log cụ thể: **CHƯA XÁC MINH**.

## 8. SAFETY RULES

- **Không chạy** `git reset`, `git clean`, `git push`, `git checkout -- <file>` hay bất kỳ lệnh git mang tính phá hủy nào.
- Không tự xóa hoặc ghi đè file. Trước khi ghi một file, phải đọc nó.
- Không tự thay đổi cấu hình phần cứng: chân GPIO, kênh ADC, hệ số chia áp, `AT+CFUN`/`CVAUX`, chế độ GNSS, cấu hình UART.
- Không tự build, nạp firmware hay chạy công cụ trong `tools/` khi chưa được yêu cầu.
- **Trước mỗi thay đổi code:** giải thích mục đích, phạm vi (hàm/dòng sẽ sửa) và ảnh hưởng tới chức năng hiện có.
- **Sau khi sửa code:** báo cáo danh sách file đã thay đổi (xác nhận bằng `git status` / `git diff --stat`) và tóm tắt thay đổi.
- Thông tin chưa kiểm chứng phải ghi rõ **CHƯA XÁC MINH**.
