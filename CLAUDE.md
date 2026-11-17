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
  - NASA-1: đăng nhập. NASA-2: hành trình realtime. NASA-7: gửi bù dữ liệu backup. NASA-13: báo lên server lệnh nhận qua SMS kèm phản hồi (mục 4). NASA-3 đã bỏ (12/11/2026, bản patch lưu ngoài repo ở `E:\nasa-4g-backup\nasa3_v1.0.261112a.patch`).
  - Server mặc định: `tcp.rovigps.vn:2598` (`TRACKER_SERVER_HOST/PORT`). Đổi được bằng lệnh 1 (lưu `C:/nasa_server.dat`).
- Chu kỳ gửi NASA-2: ACC bật thì 10 s/lần, ACC tắt thì 300 s/lần. Gửi ngay khi ACC đổi trạng thái. Hẹn giờ theo `sAPI_GetTicks()` với mốc `due_tick` cố định (commit `196bfa0`), không đếm vòng lặp, không gửi dồn bù.
- Khi mất kết nối: lưu backup theo ACC (bật 20 s/bản, tắt 300 s/bản, ACC đổi trạng thái thì lưu ngay; `TRACKER_BACKUP_ACC_ON_SEC/OFF_SEC`). RAM giữ tối đa 100 bản; cứ đủ `TRACKER_BACKUP_FLUSH_EVERY` (10) bản thì ghi cả lô xuống `C:/nasa_backup.dat`, nên mất nguồn chỉ mất < 10 bản. Có mạng lại thì gửi bù bằng NASA-7.
  - Flash `C:/` (log khởi động 11/11/2026): tổng 446464 B, trống ≈ 258048 B. Mỗi bản ghi backup ≈ 88 B → đủ ≈ 16 giờ mất mạng khi ACC bật.
- **Lệnh cấu hình** (bảng `Config Command NASA-4G latest.xlsx` ở thư mục gốc, file chưa commit): nhận qua TCP (server gửi chữ trần, mỗi lệnh 1 dòng) và qua SMS, dùng chung `tracker_cmd_execute()`. Đọc `sa,<mã>`, ghi `na,<mã>,<nội dung>`. Đã có mã 1, 9, 27, 29, 33, 38, 58 (bảng ở mục 4); mã khác bị bỏ qua, không phản hồi.
- **Cập nhật firmware từ xa (FOTA app)** bằng lệnh 33: tải `customer_app.bin` qua HTTP (mục 4).
- Tự phục hồi: reset RF (`AT+CFUN=0/1`) khi không đăng ký được mạng, tự kết nối lại TCP.
- Đồng bộ giờ theo thứ tự GNSS → RTC (`AT+CCLK`) → giờ mạng → NTP (`pool.ntp.org`).
- 3 LED trạng thái: PWR, NET, GNSS.

Thông số server, biển số, mã đăng nhập, phiên bản và mọi cấu hình `TRACKER_*` đang hard-code ở `SIMCOM_SDK_SET/tracker/tracker_config.h`.

## 2. HARDWARE PLATFORM

| Mục | Giá trị | Nguồn / trạng thái |
|---|---|---|
| Module | SIMCom A7677S | Tên biến thể trong `kernel/`, `sc_lib/` |
| Chipset | ASR 1606 (LTE Cat-1) | Tên target `..._1606_...`, `aboot/images/ASR1606C_*` |
| Build target đang dùng | `A7677S_MANV_1606_V702_OPENSDK` | Chỉ thư mục này có trong `out/` |
| Các biến thể khác trong SDK | `A7677S_LANS/LANV/MANS_1606_V702_OPENSDK` | `kernel/`, `sc_lib/` |
| CPU | Toolchain biên dịch với `-mcpu=cortex-r4 -mthumb -mfloat-abi=soft` | `config/ToolChain.cmake:17-18`. "Cortex-R5" theo thông tin dự án: **CHƯA XÁC MINH** trong repo |
| RAM userspace | 256 KB (`ORIGIN 0x7e052000, LENGTH 0x40000`) | `config/A7677S_MANV_1606_V702_OPENSDK_linkscript.ld` |
| Bản build gần nhất | text 68689 / data 1581 / bss 14645 B (`v1.0.261117b`). Bản `v1.0.261113e`: 66916 / 1581 / 14609. Bản `v1.0.261112d`: 62417 / 1505 / 12242. Bản `v1.0.261109` (vừa tắt `HAS_DEMO`): 55159 / 1467 / 10756. Khi còn demo: text ≈ 197 KB, data ≈ 5 KB, bss ≈ 35 KB | `arm-none-eabi-size customer_app.elf` |

Ngoại vi do firmware sử dụng (lấy từ code; comment ghi "sơ đồ RV26"). **Phần cứng thực tế: CHƯA XÁC MINH.** Repo không có schematic của board.

| Chức năng | Chân / kênh | Ghi chú |
|---|---|---|
| LED NET | GPIO 32 | Output, mức 1 = sáng |
| LED PWR | GPIO 80 | Output. Sáng = hoạt động, nháy chậm = đã tắt bằng lệnh 38 |
| LED GNSS | GPIO 74 | Output |
| DOUT | GPIO 73 | Đã khai báo, chưa dùng |
| ACC | GPIO 37 | Input pull-up, mức thấp = ACC bật |
| Điện áp xe | ADC kênh 1 (`sAPI_ReadAdc(1)`) | Cầu chia R306 220K / R307 10K, hệ số 23. Kênh ADC và đơn vị trả về (mV): **CHƯA XÁC MINH** |
| Nguồn anten GNSS | `AT+CVAUX=1` | |
| UART1 | 115200 8N1 | Tracker không dùng. Menu demo SIMCom trên UART1 đã bị bỏ từ `v1.0.261109` (tắt `HAS_DEMO`) |

Tài liệu phần cứng trong `Doc/hardware/` là của A7630C/A7670C/A7680C. **Repo không có tài liệu phần cứng riêng cho A7677S.**

## 3. SDK ARCHITECTURE

SDK: `22110B01V06A7677MxA_SDK_240721`. Bản build trước đó được tạo tại `E:/22110B01V06A7677MxA_SDK_240721/` (đường dẫn này còn nằm trong `out/.../build.ninja`).

```
SIMCOM_SDK_SET/
├── Makefile, GNUmake.exe      Điểm vào build (make → cmake → ninja)
├── CMakeLists.txt             Chọn source app, link sc_lib/sc_demo/third libs
├── sc_application.c           ★ Firmware tracker: task chính, vòng phiên, userSpace_Main (build khi PROTOCOL_V1)
├── tracker/                   ★ Các module tracker (tracker_config.h, tracker_common.h, tracker_*.c/.h)
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
- `CONFIG_HAS_DEMO` **đã tắt** (commit `dbb5477`): `sc_demo` không còn được biên dịch và link (`CMakeLists.txt` `if(HAS_DEMO)`), không còn task demo nào chạy.
  - `HAS_UART`, `HAS_URC`, `HAS_USB`, `HAS_FS`, `HAS_TCPIP` vẫn được định nghĩa: `config/buildOptions.cmake` bật chúng theo header có trong SDK, không phụ thuộc `CONFIG_HAS_DEMO`.
  - Các dòng `CONFIG_HAS_DEMO_*` còn trong `.config` nhưng không có tác dụng khi không build `sc_demo`.
- `config/userspaceConfig.h` được CMake **sinh lại** từ `userspaceConfig.h.in` (`configure_file`) mỗi khi cấu hình đổi. CMake ghi file dạng CRLF nên Git có thể báo thay đổi toàn file dù nội dung không đổi.
- Cờ trong `.config` đi tới CMake qua `tools/win32/buildConfig` (`Makefile:478`) thành `-DCONFIG_*=ON`, rồi `config/buildOptions.cmake` đổi thành `HAS_*`.

## 4. FIRMWARE ARCHITECTURE

Từ commit tách module, mã nguồn chia theo tính năng (tìm hàm theo tên, số dòng có thể lệch):

| File (`SIMCOM_SDK_SET/`) | Nội dung |
|---|---|
| `tracker/tracker_config.h` | Mọi `#define TRACKER_*`: server, chu kỳ, thiết bị, ADC, GPIO, watchdog, bit trạng thái, `TRACKER_FW_VERSION` |
| `tracker/tracker_common.h` | Include SDK, kiểu dữ liệu chung (`tracker_location_t`, `backup_record_t`, `tracker_ctx_t`…), `extern is_logged_in` |
| `tracker/tracker_util.c` | `tracker_at`, `tracker_gpio_setup`, `tracker_checksum`, `tracker_nmea_to_decimal` |
| `tracker/tracker_led.c` | Task LED, watchdog, `tracker_alive()`, `led_pwr_mode`/`led_net_mode`/`led_gnss_mode`, nguyên nhân reset (`tracker_reset_reason_*`, `tracker_reset_mark`) |
| `tracker/tracker_backup.c` | Hàng đợi RAM + flash, `tracker_process_backup`, `tracker_delay`, trạng thái lưu bền (`tracker_backup_startup`, `tracker_state_msg_id_*`), cờ lệnh 38 `device_enabled` / `tracker_device_set_enabled` |
| `tracker/tracker_device.c` | IMEI, ICCID |
| `tracker/tracker_time.c` | Giờ GNSS/RTC/mạng/NTP, `tracker_get_datetime` |
| `tracker/tracker_sensor.c` | ACC, GNSS (`tracker_gnss_init`, `tracker_gnss_power_off`, `tracker_gnss_fix_valid`, `tracker_get_last_fix`), hướng, điện áp ADC |
| `tracker/tracker_net.c` | SIM, mạng, PDP, TCP, `tracker_wait_network`, `tracker_activate_pdp`, server cấu hình (lệnh 1: `tracker_server_*`), FOTA (lệnh 33: `tracker_fota_run`) |
| `tracker/tracker_protocol.c` | Đóng gói NASA-1/2/7, `msg_id`, ACK login, `tracker_poll_server` (gom dòng lệnh từ server), lệnh cấu hình `tracker_cmd_execute` (1, 9, 27, 29, 33, 38, 58), NASA-13 `tracker_send_sms_report` |
| `tracker/tracker_sms.c` | Nhận lệnh cấu hình qua SMS, giải mã PDU, gửi SMS phản hồi (`tracker_sms_init`, `tracker_sms_poll`), phản hồi chờ báo NASA-13 (`tracker_sms_report_take`) |
| `sc_application.c` | `tracker_login`, `tracker_run_session`, `tracker_device_off_wait`, chế độ ngủ lệnh 58 (`tracker_sleep_due`, `tracker_sleep_wait`), `sTask_NasaTracker`, `userSpace_Main`, bảng `userSpaceEntry` |

Hàm chỉ dùng trong một file thì giữ `static`; hàm/biến dùng chéo được khai báo trong `tracker_*.h` tương ứng.

**Đăng ký và entry point**
- `userSpaceEntry` được đặt trong section `.userSpaceRegTable`, stack 10 KB, priority 30 (cuối file). Kernel dùng bảng này để gọi `userSpace_Main(void *arg)`.

**`userSpace_Main`**
1. `ApiMapInit(arg)`.
2. `tracker_acc_init()`.
3. Tạo task `sTask_LedControl` (stack 4 KB, prio 105) và `sTask_NasaTracker` (stack 12 KB, prio 100).
4. `HAS_DEMO` đã tắt nên khối demo (URC task, USB VCOM, HelloWorld, menu UART1) không được biên dịch; hàm trả về ngay sau khi tạo task.

**Task `sTask_LedControl`**
- Chu kỳ 100 ms. Đọc biến `volatile led_pwr_mode` / `led_net_mode` / `led_gnss_mode` để nháy LED.
- Kiêm giám sát watchdog (xem mục Watchdog bên dưới).

**Task `sTask_NasaTracker`**
- Chờ 5 s → `sAPI_NetworkInit()` → `tracker_gnss_init()` → `tracker_sms_init()` → `tracker_backup_startup()` (trạng thái lưu bền, file backup cũ, trạng thái lệnh 38) → `tracker_server_load()` (server lệnh 1).
- Vòng lặp vô hạn:
  0. Nếu `device_enabled == FALSE` (lệnh 38): `tracker_device_off_wait()` rồi quay lại đầu vòng. Nếu đến hạn ngủ (lệnh 58): `tracker_sleep_wait()` rồi quay lại đầu vòng (cũng kiểm tra trước `tracker_delay` khi mất kết nối).
  1. `tracker_wait_network`: CPIN, CEREG/CGREG; lỗi 15 lần thì CFUN=0/1.
  2. `tracker_activate_pdp`: `sAPI_TcpipPdpActive(1, 1)`. Sau đó nếu có lệnh 33 đang chờ thì `tracker_fota_run()` (thành công thì reset, lỗi thì quay lại bước 1).
  3. `tracker_tcp_connect`: `getaddrinfo` / `socket` / `connect` (blocking).
  4. `tracker_login`: chờ có giờ, gửi NASA-1, chờ ACK 15 s bằng `select`.
  5. `tracker_run_session`: vòng 100 ms gồm poll socket (kèm xử lý lệnh từ server) → theo dõi GNSS → poll SMS rồi gửi NASA-13 nếu có → thoát nếu lệnh 38 tắt / lệnh 1 đổi server / lệnh 9 chờ reset / lệnh 33 chờ cập nhật / đến hạn ngủ lệnh 58 (gửi NASA-13 trước) → gửi 1 bản NASA-7 → NASA-2 khi đến hạn.
  6. Mất kết nối thì `tracker_delay(5)` rồi lặp lại.

**Các thành phần khác**
- **Backup:** `tracker_process_backup` được gọi trong mọi vòng chờ. Nó chỉ chạy sau khi đã đăng nhập ít nhất 1 lần, đang mất phiên và thiết bị không bị tắt bằng lệnh 38.
- **GNSS:** poll `sAPI_GnssNavdataGet()` (trả `SC_GNSSINFO *`, các trường là chuỗi NMEA). Callback `sAPI_GnssStateBindCB` đang để trống. Đơn vị `info->speed` (km/h hay knot): **CHƯA XÁC MINH**, code hiện coi là km/h.
  - **Dữ liệu cũ sau khi tắt GNSS** (log 17/11/2026): sau `sAPI_GnssPowerStatusSet(OFF)`, `sAPI_GnssNavdataGet()` vẫn giữ giá trị lúc tắt (`fixValid = TRUE`, giờ/tọa độ cũ). Khi bật lại, NASA-1/2 từng mang giờ lúc vào ngủ, và RTC bị đồng bộ lùi theo giờ cũ. Sửa (`v1.0.261117b`): `tracker_gnss_power_off()` lưu chuỗi giờ/ngày GPS lúc tắt; `tracker_gnss_fix_valid(info)` coi là chưa fix cho đến khi GPS báo giờ/ngày khác. **Không đọc `info->fixValid` trực tiếp**, luôn dùng `tracker_gnss_fix_valid()`. Kết quả trên thiết bị: **CHƯA XÁC MINH**.
  - Thời gian bắt GPS (đo 12/11/2026, ngoài trời): tắt/bật nguồn → dưới 1 phút. Lần chạy đầu ngay sau khi nạp firmware mất ≈ 10 phút; giả thuyết là nạp firmware làm mất dữ liệu vệ tinh đã lưu (**CHƯA XÁC MINH**). Chưa dùng AGPS; SDK có `sAPI_GnssAgpsSeviceOpen()` (`simcom_gps.h`, `demo_gps.c`) nếu cần rút ngắn.
- **AT:** dùng `sAPI_SendATCMDWaitResp` trên kênh 10 (đồng bộ, không cần task URC).
- **UART:** tracker không tự cấu hình UART. Callback UART/URC thuộc phần demo (`sc_demo/src/uart_api.c`, `cus_urc.c`).
- **TCP/IP:** socket lwIP kiểu BSD, blocking. Theo `scfw_socket_define.h`, `SO_SNDTIMEO` chưa được hỗ trợ. Dữ liệu server gửi xuống được gom theo dòng (`\r`/`\n`) trong `tracker_poll_server`; dòng `sa,`/`na,` được chạy như lệnh cấu hình và phản hồi kèm `\r\n`, dòng khác chỉ ghi log `[NASA] RX:`.
- **Lệnh cấu hình (`tracker_cmd_execute`, `tracker_protocol.c`):** trả về độ dài phản hồi, 0 = không phản hồi. Phản hồi nhiều dòng nối bằng `\n`. Tham số sai → `loi tham so`.

  | Mã | Lệnh | Chức năng | Phản hồi | Ràng buộc / lưu trữ |
  |---|---|---|---|---|
  | 1 | `sa,1` / `na,1,<host>,<port>` | Đọc/đổi server | `ip/port: <host>,<port>` | Host 1–63 ký tự `[A-Za-z0-9.-]`, port 1–65535. Lưu `C:/nasa_server.dat` (`host,port`) |
  | 9 | `na,9` | Reset thiết bị | `OK` | Reset sau 5 s (`TRACKER_CMD_RESET_DELAY_SEC`) |
  | 27 | `sa,27` | Vị trí hiện tại | `https://maps.google.com/maps?q=N%20<độ>%20<phút>%20E%20<độ>%20<phút>#OK` | Vị trí cũ → đuôi `#LAST`; chưa từng fix → `chua xac dinh duoc vi tri GPS` |
  | 29 | `sa,29` | Thông tin thiết bị | 6 dòng: `NASA4G,…` / `GSM,…` / `GPS,…` / `SV,…` / `PW,…` / `TI,…` | Chỉ đọc |
  | 33 | `na,33,<link>` | Cập nhật firmware (FOTA) | `dang update firmware` | Link bắt đầu `http://`, `https://`, `ftp://`, `ftps://`, ≤ 255 ký tự, không khoảng trắng |
  | 38 | `sa,38` / `na,38,0` / `na,38,1` | Tắt/mở thiết bị | `trang thai thiet bi: <0/1>` / `thiet bi ngung hoat dong` / `thiet bi hoat dong tro lai` | Lưu `C:/nasa_power.dat` (1 byte `'0'`/`'1'`, không có file = mở) |
  | 58 | `sa,58` / `na,58,<xx>,<yy>` | Chế độ ngủ | `che do ngu: <xx>,<yy>` hoặc `che do ngu: tat` | `xx` 10–65535 s, `yy` 60–86400 s, `0,0` = tắt (mặc định). Lưu `C:/nasa_sleep.dat` (`xx,yy`) |

  Chi tiết từng lệnh:
  - **Lệnh 1:** lưu xong thì đóng phiên để kết nối server mới. Server cấu hình lỗi kết nối/đăng nhập 10 lần liên tiếp (`TRACKER_SERVER_FALLBACK_FAILS`) thì quay về server mặc định đến hết lần chạy. Dòng `SV` của `sa,29` báo server đang dùng.
  - **Lệnh 9:** trước khi reset, ghi backup RAM xuống flash, ghi dấu `R-3.0` vào `C:/nasa_reset.dat` rồi `sAPI_SysReset()` (`sc_power.h`). Login đầu sau đó gửi `R-3.0`.
  - **Lệnh 27:** chưa có fix (hoặc GPS đang tắt do lệnh 38 / đang ngủ do lệnh 58) nhưng đã có fix trong lần chạy này → trả vị trí cũ. Vị trí cũ chỉ lưu RAM.
  - **Lệnh 29:**

    | Dòng | Định dạng |
    |---|---|
    | 1 | `NASA4G,<imei>,<fw>,<hw>,<khóa 0>,<số ngày chạy>,<giờ khởi động DDMMYYhhmmss>` |
    | 2 | `GSM,<0/1>,<csq>` |
    | 3 | `GPS,<0 OK/1 lỗi/2 ngủ>,<fix>,<số vệ tinh>` |
    | 4 | `SV,<host>,<port>,<0/1>` |
    | 5 | `PW,<0 normal/1 sleeping>,<điện áp>` |
    | 6 | `TI,<ddmmyy>-<hh:mm:ss>` |

    Số ngày chạy và giờ khởi động tính từ `sAPI_GetTicks()` của lần chạy hiện tại; chức năng khóa chưa có nên luôn 0. Khi tắt bằng lệnh 38 hoặc đang ngủ (lệnh 58): `GPS,2`, `PW,1`.
  - **Lệnh 33:** xem mục **FOTA** dưới đây.
  - **Lệnh 38 (`tracker_device_off_wait`):** khi tắt, phản hồi gửi trước, rồi đóng TCP, ngừng NASA và backup, tắt GNSS (`sAPI_GnssPowerStatusSet(OFF)`, `AT+CVAUX` giữ nguyên), LED NET/GNSS tắt, LED PWR nháy chậm. Mạng di động và SMS vẫn chạy để mở lại bằng SMS `na,38,1`. Giữ qua mất nguồn. Đã xác minh trên thiết bị 12/11/2026.
  - **Lệnh 58 (giai đoạn 1, mức ứng dụng; `tracker_sleep_due` / `tracker_sleep_wait` trong `sc_application.c`):**
    - Vào ngủ khi ACC tắt liên tục `xx` giây, thiết bị đang mở (lệnh 38) và không có lệnh 9/33 đang chờ. Khi đang có phiên: gửi NASA-13 `info. thiet bi vao che do ngu.#OK,` rồi đóng TCP.
    - Khi ngủ: tắt GNSS, LED NET/GNSS tắt, LED PWR nháy chậm, không gửi NASA, không lưu backup; vẫn nhận SMS và chạy lệnh (lệnh 9 vẫn reset).
    - Thức khi: hết `yy` giây, ACC bật (kiểm tra mỗi 1 s), `na,38,0`, `na,58,0,0`, lệnh 1 hoặc lệnh 33. Thức xong bật GNSS, kết nối lại server; ACC vẫn tắt thì đếm lại `xx` rồi ngủ tiếp.
    - Đã thử trên thiết bị 17/11/2026 (`na,58,30,120`): vào ngủ, gửi NASA-13 và thức dậy đúng.
    - Chưa dùng chế độ ngủ thật của module (`sAPI_SystemSleepSet`, `sAPI_SystemSleepExSet`, `simcom_system.h`; dòng ≈ 1 mA theo `Doc/software/A76xx Series Open SDK_低功耗开发_应用指导_V1.00.pdf`, yêu cầu rút USB, tắt GPS). Ảnh hưởng tới SMS/watchdog/log USB **CHƯA XÁC MINH**. Mức tiết kiệm thực tế của giai đoạn 1: chưa đo.
- **SMS (`tracker_sms.c`):**
  - `sAPI_UrcRefRegister(q, SC_URC_SMS_MASK)`; khi `getSmsReady()` thì `sAPI_SmsSetNewMsgInd(1,1,2,0,0,0)` → SMS mới đến qua URC `SC_URC_FLASH_MSG` (bên nhận `free(arg3)`).
  - Thiết bị thực tế báo dạng **PDU**: `+CMT: "",<độ dài TPDU><PDU hex>` (log không thấy xuống dòng giữa độ dài và PDU) → giải mã bằng `sAPI_SmsDecodePdu()`. Vẫn hỗ trợ dạng text.
  - Phản hồi bằng `sAPI_SmsSendLongMsg()`, chờ kết quả tối đa 30 s (có `tracker_alive()`). Cùng 1 số chỉ được phản hồi 1 lần / 60 s (`TRACKER_SMS_REPLY_MIN_SEC`): lệnh vẫn chạy nhưng không gửi SMS trả lời.
  - **`sAPI_SmsSendLongMsg()` để module ở chế độ text** (xác minh qua log 13/11/2026): SMS đến sau đó có dạng `+CMT: "","","<giờ>"`, không có số gửi và nội dung → chỉ phản hồi được 1 lệnh sau mỗi lần khởi động. Sửa (`v1.0.261113c`): sau mỗi lần gửi, `tracker_sms_restore_pdu()` gọi `sAPI_SmsSetFormat(0)` (0 = PDU, 1 = text theo `demo_sms.c`). Đã xác minh trên thiết bị.
  - **NASA-13:** khi đang có phiên, phản hồi của lệnh SMS được báo lên server ngay sau `tracker_sms_poll()` trong vòng phiên (mất kết nối thì bỏ). Định dạng theo mẫu của server (checksum đối chiếu khớp): `!NASA,13,<msg_id>,<YYYY-MM-DD hh:mm:ss>,info. <dòng 1>. <dòng 2>.#OK,*<checksum>`. Trong phản hồi, `,` đổi thành `.`, mỗi dòng kết thúc `.`, các dòng cách nhau 1 dấu cách. `info` và `#OK` dùng cố định cho mọi lệnh (ý nghĩa **CHƯA XÁC MINH**). Dùng chung `msg_id` với NASA-1/2/7. Chỉ lưu phản hồi của SMS gần nhất.
  - `tracker_sms_poll()` không chặn, được gọi trong vòng phiên, `tracker_login`, chờ ACK, `tracker_delay` và vòng chờ khi tắt.
  - Mỗi SMS có lúc được báo 2 lần (nguyên nhân **CHƯA XÁC MINH**): lệnh chạy 2 lần và có thể gửi 2 bản NASA-13; SMS trả lời lần 2 bị chặn bởi giới hạn 60 s.
  - Link có dấu `_` gửi qua SMS từng bị `loi tham so` (14/11/2026). Giả thuyết: SMS dùng bảng mã GSM 7-bit (`_` = 0x11, `@` = 0x00, `$` = 0x02) và `sAPI_SmsDecodePdu()` không đổi sang ASCII (**CHƯA XÁC MINH**, chưa có log của lần lỗi). Tạm thời đặt tên file firmware không có `_` (vd `fw.bin`).
- **FOTA app (lệnh 33, `tracker_fota_run`, `tracker_net.c`):** theo `sc_demo/src/demo_app_download.c`.
  - Phản hồi gửi trước → đóng phiên → vòng chính chờ có PDP → ghi backup RAM xuống flash, LED NET nháy nhanh → tạo task `NASA-FOTA` (stack 10 KB, prio 100) chạy `sAPI_FBF_Disable()` → `sAPI_AppDownload()` (`recvtimeout` 20 s) → `sAPI_AppPackageCrc()`. Task tracker chờ, vẫn gọi `tracker_alive()` mỗi giây.
  - Tải xong và CRC = 0 → ghi dấu `R-3.0`, `sAPI_SysReset()`; khởi động lại chạy firmware mới. Lỗi → xóa task, chạy tiếp firmware cũ, kết nối lại server. Quá 600 s (`TRACKER_FOTA_TIMEOUT_SEC`) → reset mềm.
  - File đặt trên server là `customer_app.bin` trong `out/…/` (đã gắn CRC), **không** phải `burn.zip`.
  - Đã thử 14/11/2026 (từ `v1.0.261113d`, file `v1.0.261113e`, server = web server Python trên máy tính + localtunnel):
    - Link `https://…loca.lt/fw.bin`: `AppDownload ret=11` (`SC_APP_DOWNLOAD_NO_MEMORY`), firmware cũ chạy tiếp. Giả thuyết: phiên SSL thiếu heap (**CHƯA XÁC MINH**; SDK không có API báo heap còn trống).
    - Link `http://…/fw.bin`: thiết bị tải được file (server log có `GET`), người dùng báo thành công. → **Dùng link `http://`.**
  - Mã lỗi `SCAppDwonLoadReturnCode` (`sc_app_download.h`): 1 PDP, 2 bận, 3 không kết nối, 4 tên miền, 5 gửi, 6 nhận, 7 tham số, 9/10 mở/ghi phân vùng, 11 thiếu bộ nhớ, 12 không có file, 13 lỗi chung.
  - Không có rollback: bản mới khởi động được nhưng lỗi kết nối thì phải nạp lại bằng cáp. Thử bản mới trên thiết bị tại bàn trước. Không lưu backup trong lúc tải (tối đa 10 phút). Lệnh đến khi thiết bị đang tắt (lệnh 38) thì chờ đến khi mở lại.
  - Cơ chế bootloader áp gói sau reset và trường hợp mất điện giữa chừng: **CHƯA XÁC MINH**. FOTA hệ thống của module (`sAPI_FotaServiceBegin`, gói vi sai `adiff`, `Doc/software/FOTA/`) không dùng.
- **Thời gian:** `sAPI_TaskSleep(200)` = 1 giây (1 tick = 5 ms).
- **RTC (`AT+CCLK`):** phải ghi **giờ UTC** kèm hậu tố `+28`. Module tự cộng múi giờ khi trả `AT+CCLK?`, nên đọc ra đúng giờ Việt Nam. **Đã xác minh trên thiết bị ngày 05/11/2026:** bản thử ghi giờ Việt Nam kèm `+28` thì đọc lại bị dư 7 giờ (11:19 thành 18:19). Không đổi `tracker_set_rtc()` sang giờ địa phương.
- **Watchdog** (commit `dbda0df`): WDT phần cứng bật theo trình tự `sc_demo/src/demo_wtd.c`, timeout reg `0x04` = 16 s (theo comment của demo; các giá trị khác **CHƯA XÁC MINH**).
  - Task LED cho ăn mỗi 1 s, **chỉ khi** task tracker còn gọi `tracker_alive()` trong vòng `TRACKER_WDT_STALL_SEC` = 300 s. Quá ngưỡng thì ngừng cho ăn và module tự reset sau ≤ 16 s.
  - Mọi vòng lặp/vòng chờ mới trong task tracker **phải gọi `tracker_alive()`**. Không gọi `sAPI_FeedWtd()` từ task tracker.
  - 300 s là ngưỡng tự chọn để lớn hơn các lệnh chặn lâu (`connect()`, `AT+CFUN`); thời gian chặn thực tế **CHƯA XÁC MINH**.
  - Đã chạy liên tục 05–09/11/2026 không có reset ngoài ý muốn.
- **ID bản tin (`msg_id`):** số thứ tự tính từ đầu ngày theo giao thức; login dùng 1 ID riêng; đặt lại về 1 khi sang ngày mới (`tracker_msg_id_check_new_day`, gọi ngay trước khi dùng ID cho NASA-1/2/7). **Không** đặt lại khi kết nối lại TCP.
- **Nguyên nhân reset (trường 11 NASA-1, commit `ba8e4af`):** lần login đầu sau khởi động gửi `R-2.0` (bật nguồn) hoặc `R-1.0` (có file dấu `C:/nasa_reset.dat` do task LED ghi ngay trước khi ngừng cho WDT ăn); `R-3.0` khi có file dấu chứa chuỗi `R-3.0` (reset mềm do lệnh 9 hoặc FOTA, `tracker_reset_mark()`); sau khi login thành công, các lần login sau trong cùng lần chạy gửi `R-0.0`. Task LED đã ngừng cho ăn thì **chốt** luôn, không cho ăn lại. Đã xác minh trên thiết bị: `R-2.0` khi bật nguồn, `R-0.0` khi kết nối lại; `R-1.0` chưa thử thực tế.
- **Trạng thái lưu bền (`C:/nasa_state.dat`, `tracker_backup.c`):**
  - `msg_id`: dành trước khối `TRACKER_MSG_ID_RESERVE_BLOCK` (100) ID, chỉ ghi flash khi dùng hết khối (~86 lần/ngày). Reboot trong cùng ngày tiếp tục từ ID đã dành: không trùng ID, nhưng ID có thể **nhảy tới 100**.
  - Vị trí đọc file backup: ghi sau mỗi `TRACKER_STATE_OFFSET_SAVE_EVERY` (10) bản NASA-7 lấy từ flash và ngay khi file backup bị xóa. Sau reboot có thể gửi lặp **tối đa 9 bản** NASA-7. Vị trí được kiểm tra hợp lệ với kích thước file trước khi dùng.
- **Phiên bản firmware:** `TRACKER_FW_VERSION` dạng `v1.0.yyMMdd` (thêm hậu tố `a`, `b`… nếu nạp nhiều bản trong ngày). Đổi phiên bản cho **mỗi bản đem nạp** để nhận biết qua bản tin login.

**Vấn đề đã biết** (từ lần phân tích trước; chưa sửa, chỉ sửa khi được yêu cầu):
- Cả hệ thống treo thì WDT reset mà không ai ghi được file dấu, nên lần khởi động sau báo `R-2.0` thay vì `R-1.0` (SDK không có API đọc lý do reset).
- Giờ trong NASA-2 tính bằng `dt_next` (+10 s mỗi bản) trong khi chu kỳ thực ≈ 10,2 s, nên trôi tới 5 s rồi nhảy (vd 16 s) khi đồng bộ lại theo GPS.
- `odometer` luôn bằng 0.
- `heading` chỉ có trong NASA-2 (NASA-7 gửi 0 vì bản ghi backup chưa lưu hướng). Ngưỡng cập nhật 3 km/h chưa đủ lọc nhiễu: log 05/11/2026 cho thấy GPS báo tới 15 km/h khi thiết bị đứng yên. Giá trị hướng khi xe chạy thật: **CHƯA XÁC MINH**.
- Task priority 100/105, trong khi `Doc/software/A76xx_Series_OpenSDK_Q&A.md` khuyến nghị 150–250 (hệ thống dùng < 120).

## 5. CODING RULES

- **Chuẩn C:** C99 (`-std=c99 -fgnu89-inline -Os -Wall`), GCC arm-none-eabi 9.2.1.
  - Code phải biên dịch **không có warning** (Q&A của SIMCom yêu cầu không để warning).
  - Target `userspace` (sc_application.c + tracker/*.c) được biên dịch với `-Wall -Werror` (`CMakeLists.txt`, `target_compile_options`). Trước đây `-Werror` chỉ có nhờ kế thừa từ `sc_demo`; khi tắt `HAS_DEMO` đã mất và được bật lại trực tiếp. Vẫn kiểm tra log: số dòng `warning:` phải bằng 0.
  - Hàm `static` không dùng tới sẽ gây warning `-Wunused-function` → build lỗi; xóa hoặc dùng nó.
  - Không include `unistd.h`: nó gây trùng định nghĩa `close` với `scfw_socket.h` (theo Q&A).
- **Chỉ dùng API đã xác minh** trong `sc_lib/A7677S_MANV_1606_V702_OPENSDK/inc/*.h`, `sc_lib/inc/` hoặc trong `sc_demo/src/`.
  - Trước khi dùng một API mới, mở header để kiểm tra prototype, kiểu tham số và ý nghĩa giá trị trả về. Nếu có, tham khảo cách demo dùng nó.
  - Không đoán tên API, enum, macro hay kiểu dữ liệu. Nếu không tìm thấy thì báo "CHƯA XÁC MINH" và hỏi lại.
  - Lưu ý đã xác minh: `sAPI_fread` / `sAPI_fwrite` trả về **số byte**, không phải số phần tử (`simcom_file.h`).
- **RAM / stack:**
  - Tổng RAM userspace là 256 KB. Bản `v1.0.261109` dùng ≈ 12 KB tĩnh (data + bss).
  - Stack task tracker 12 KB, LED 4 KB. Không đặt buffer lớn trên stack. Ước lượng stack khi thêm buffer cục bộ.
  - Sau khi build, kiểm tra `.bss`/`.data` bằng `arm-none-eabi-size` và file `.map`.
- **Bộ nhớ động:**
  - Tránh `malloc` / `sAPI_Malloc` (là macro tới `malloc`, xem `simcom_os.h`). Ưu tiên buffer tĩnh có kích thước cố định.
  - Nếu bắt buộc phải cấp phát động, phải giải phóng đúng chỗ và kiểm tra NULL.
- **An toàn chuỗi và buffer:** dùng `snprintf`, luôn giới hạn độ dài, luôn kết thúc chuỗi bằng `\0`, kiểm tra giá trị trả về của `sscanf`.
- **GNSS:** kiểm tra con trỏ NULL và `fixValid` trước khi dùng tọa độ. Struct navdata do driver cập nhật đồng thời, nên copy ra trước khi parse.
- **TCP:** luôn kiểm tra giá trị trả về của `send` / `recv` / `select`. Đóng socket khi lỗi. Không để vòng lặp chặn vô hạn.
- **UART / AT:** kiểm tra mã trả về, giới hạn timeout, đặt kích thước buffer phản hồi đủ lớn.
- **Task:**
  - Mọi vòng `while(1)` phải có `sAPI_TaskSleep`. Vòng lặp trong task tracker còn phải gọi `tracker_alive()` (watchdog).
  - Biến chia sẻ giữa các task phải là `volatile` hoặc được bảo vệ.
  - Muốn xóa task thì hàm task phải thoát vòng lặp và return trước (Q&A).
- **File:** gọi `sAPI_fsync` trước `sAPI_fclose` khi cần dữ liệu bền qua mất nguồn. Đóng file ngay sau khi dùng.
- **Không thay đổi chức năng đang hoạt động** nếu không có yêu cầu: giao thức NASA, chu kỳ gửi, chân GPIO, thông số server.
- Giữ phong cách hiện tại: tiền tố `tracker_`, hằng số `TRACKER_*`, comment tiếng Việt, log qua `sAPI_Debug("[NASA] ...")`.

## 6. FILE MODIFICATION RULES

- **Được sửa:** `SIMCOM_SDK_SET/sc_application.c` và các file `tracker_*.c` / `tracker_*.h` trong `SIMCOM_SDK_SET/tracker/`. Đây là các file được build (`CMakeLists.txt`, `SET(app_src sc_application.c tracker/...)`).
  - Thêm/xóa/đổi tên file `.c` trong `tracker/` thì phải sửa danh sách `app_src` trong `CMakeLists.txt` → **giải thích và chờ người dùng đồng ý** trước.
  - Cấu hình mới đặt trong `tracker_config.h`; kiểu dữ liệu dùng chung đặt trong `tracker_common.h`.
- Bản `sc_application.c` cũ ở thư mục gốc repo (không được build) **đã được người dùng xóa** ngày 04/11/2026 vì là file thừa. Bản này vẫn còn trong commit `bd25f7b` nếu cần tham khảo. Không tạo lại file `sc_application.c` ở thư mục gốc.
- **Không sửa vendor SDK:** `sc_lib/`, `sc_lib_v2/`, `sc_demo/`, `sc_demo_v2/`, `kernel/`, `public_libs/`, `examples/`, `old_fw_demo/`, `tools/`, `sc_main.c`.
- **Không sửa build system:** `Makefile`, `CMakeLists.txt`, `old_fw_Makefile`, `makeDepend.mak`, `config/*` (gồm `.config`, `userspaceConfig.h`, linker script `*.ld`, `ToolChain.*`), `out/**/build.ninja`.
  - Các thay đổi đã được người dùng duyệt: `config/.config` tắt `HAS_DEMO` (commit `dbb5477`); `CMakeLists.txt` thêm `tracker/*.c` vào `app_src`, thêm `INCLUDE_DIRECTORIES(./tracker)` và `-Werror` cho target `userspace` (commit tách module); thêm `tracker/tracker_sms.c` vào `app_src` (commit `daa025b`). Thay đổi khác vẫn phải xin duyệt.
- **Không sửa thư mục output `SIMCOM_SDK_SET/out/`** (gồm `build.ninja`, `rules.ninja`, `CMakeCache.txt`, `.elf`/`.bin`/`.map`, `burn*.zip`). Nội dung `out/` chỉ được thay đổi gián tiếp qua lệnh build đã được người dùng duyệt.
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

Trên máy build này, shell của Claude Code đặt `NoDefaultCurrentDirectoryInExePath=1`, nên phải gọi kèm `.\` và qua `cmd` (Makefile viết cho `cmd.exe`):
```powershell
Set-Location E:\nasa-4g-tracker\SIMCOM_SDK_SET
cmd /c ".\GNUmake.exe A7677S_MANV_1606_V702_OPENSDK > <log> 2>&1"
```
Không dùng `_remake`/`clean`: chúng xóa toàn bộ `out/`.

Chuỗi xử lý: Makefile → CMake sinh `out/<target>/build.ninja` → ninja → `customer_app.elf` → `objcopy -O binary` → `crc_set` → `burn.zip`.

Đầu ra trong `SIMCOM_SDK_SET/out/A7677S_MANV_1606_V702_OPENSDK/`:
- `customer_app.elf`, `customer_app.elf.map`, `customer_app.bin` (đã gắn CRC)
- `burn.zip`: gói để nạp bằng công cụ Aboot (`tools/win32/aboot`, xem `Doc/software/A76xx_Series_Open_SDK_Debug_and_Download_Application_Note_V1.00.06.pdf`)
- `burn_factory.zip`

Lưu ý về build:
- Mỗi lần chạy lệnh build, Makefile **xóa và tạo lại** `out/<target>` (`Makefile:462-463`), nên `build.ninja` mới luôn trỏ đúng repo này. Bản `out/` trong commit cũ chứa đường dẫn `E:/22110B01V06A7677MxA_SDK_240721/...`: **không chạy `ninja` trực tiếp** trên đó.
- `out/` đang được Git theo dõi: mỗi lần build làm ≈ 237 file trong `out/` hiện là thay đổi. Không commit `out/` nếu không được yêu cầu.
- Trên máy này có nhiều `burn.zip` ở thư mục SDK cũ; khi nạp phải chọn đúng `E:\nasa-4g-tracker\SIMCOM_SDK_SET\out\A7677S_MANV_1606_V702_OPENSDK\burn.zip` và đối chiếu kích thước.
- Windows Storage Sense có thể xóa file trong `%TEMP%` (đã mất backup và log build ở đó); lưu log/backup ở nơi khác.
- Q&A của SIMCom: sau khi đưa SDK lên git, timestamp file thay đổi và có thể kích hoạt sai các bước trong Makefile.
- `tools/win32/msys64.zip` và `cross_tool.zip` nằm trong `.gitignore`. Trạng thái giải nén trên máy build: **CHƯA XÁC MINH** (hiện có `tools/win32/cross_tool/gcc-arm-none-eabi`).

Kiểm tra sau khi build (khi được phép):
1. Build không có warning hay error (đếm `warning:` trong log; target `userspace` có `-Werror`).
2. `tools/win32/cross_tool/gcc-arm-none-eabi/bin/arm-none-eabi-size.exe out/.../customer_app.elf`: so sánh text/data/bss với bản trước (bản `v1.0.261117b`: 68689 / 1581 / 14645).
   - `-Wformat-truncation` (bật bởi `-Wall`, thành lỗi do `-Werror`) báo khi `snprintf("%s")` copy từ mảng lớn sang mảng nhỏ hơn; dùng hàm copy có giới hạn (như `tracker_copy_str` trong `tracker_sms.c`).
   - Sau khi build, nếu `git diff --ignore-cr-at-eol -- config/userspaceConfig.h` rỗng thì đổi file về LF để Git không báo thay đổi.
3. Kiểm tra chuỗi phiên bản trong `.bin` khớp `TRACKER_FW_VERSION`, và `customer_app.bin` bên trong `burn.zip` trùng hash với bản vừa build.
4. Chạy thử trên thiết bị: kiểm tra bản tin login mang đúng phiên bản và log server.
5. Log `sAPI_Debug` (`simcom_debug.h`): xuất qua cổng **USB "SimTech HS-USB Diagnostics"**, xem bằng CATStudio (chế độ Generic Target Online, Protocol Diag, Serial, tắt Auto Detection), cần nạp MDB `kernel/A7677S_MANV_1606_V702_OPENSDK/cp_MDB.txt`. Nguồn: `Doc/software/A76xx Series Open SDK_日志抓取指导_V1.00.pdf`. Log in quá sớm khi USB chưa sẵn sàng có thể bị mất (**CHƯA XÁC MINH**).

## 8. SAFETY RULES

- **Không chạy** `git reset`, `git clean`, `git push`, `git checkout -- <file>` hay bất kỳ lệnh git mang tính phá hủy nào.
  - `git push` chỉ chạy khi người dùng yêu cầu rõ trong lần đó (ví dụ 12/11/2026: push `main` lên `origin`, gộp commit xóa file từ GitHub bằng merge, không rebase/force).
- Không commit `out/` và các file tài liệu chưa theo dõi ở thư mục gốc (`NASA4G_Protocol Manual v1.3.docx`, `Config Command NASA-4G latest.xlsx`, `~$...xlsx`).
- Không tự xóa hoặc ghi đè file. Trước khi ghi một file, phải đọc nó.
- Không tự thay đổi cấu hình phần cứng: chân GPIO, kênh ADC, hệ số chia áp, `AT+CFUN`/`CVAUX`, chế độ GNSS, cấu hình UART.
- Không tự build, nạp firmware hay chạy công cụ trong `tools/` khi chưa được yêu cầu.
- **Trước mỗi thay đổi code:** giải thích mục đích, phạm vi (hàm/dòng sẽ sửa) và ảnh hưởng tới chức năng hiện có.
- **Sau khi sửa code:** báo cáo danh sách file đã thay đổi (xác nhận bằng `git status` / `git diff --stat`) và tóm tắt thay đổi.
- Thông tin chưa kiểm chứng phải ghi rõ **CHƯA XÁC MINH**.
