# Tài liệu dự án MainECU_STM32 (DesktopAssistant)

Bộ tài liệu này tổng hợp kiến thức về dự án và quy tắc lập trình (coding convention), được trích xuất trực tiếp từ source code hiện tại.

| Tài liệu | Nội dung |
|---|---|
| [01_Tong_Quan_Kien_Truc.md](01_Tong_Quan_Kien_Truc.md) | Mục đích dự án, cấu trúc thư mục, kiến trúc phân lớp (Core → Libs → Wrappers → HAL), luồng khởi động, trạng thái hiện tại và các phần còn thiếu |
| [02_Phan_Cung_Ngoai_Vi.md](02_Phan_Cung_Ngoai_Vi.md) | MCU, cấu hình clock, bảng mapping chân, cấu hình SPI / UART / TIM / GPIO, màn hình ILI9341, thẻ SD |
| [03_Coding_Convention.md](03_Coding_Convention.md) | Quy tắc đặt tên (prefix/suffix), template file .c/.h, quy tắc phân lớp, xử lý lỗi, checklist khi thêm module mới |

**Cập nhật lần cuối:** 2026-08-31 — thêm tầng App hoàn chỉnh: `App_Manager` (điều phối app theo lệnh `SELECT_APP` từ ESP) + `App_Clock` (giờ/lịch/thời tiết, mặc định) + `App_Video` (phát VIDEO.RVD từ SD) + `Libs_EspLink` (nhận lệnh UART, soft RTC, nhận file có CRC32). Cùng ngày: chuyển Core sang mô hình CubeMX-owned (code tay trong USER CODE blocks), ESP link qua USART1, LED PC13 blink 1s, bỏ I2C/ESP_CS. Trước đó (08-27): thêm tầng **App** (`App_HwTest`: test màn hình/SD/cảm ứng), driver thẻ SD `Libs_SdCard` + `user_diskio.c` hoàn chỉnh. Trước đó (08-22): `Libs_Touch_XPT2046`, `Libs_Video`, `Libs_Animation`. Khi kiến trúc hoặc convention thay đổi, cập nhật lại các file tương ứng.
