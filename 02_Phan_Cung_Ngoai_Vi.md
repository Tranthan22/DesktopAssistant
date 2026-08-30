# 02. Phần cứng & Ngoại vi

## 1. MCU

- **STM32F411CEUx** (theo tên file startup [startup_stm32f411ceux.s](../Core/Startup/startup_stm32f411ceux.s)) — ARM Cortex-M4F, 512KB Flash, 128KB RAM. Package UFQFPN48 (kiểu board "Black Pill").
- Firmware dùng **STM32 HAL** (stm32f4xx), khung project sinh bởi STM32CubeMX/CubeIDE.

## 2. Cấu hình Clock

Nguồn: [Wrappers_Clock.c](../Wrappers/Wrappers_Clock/Wrappers_Clock.c)

| Thông số | Giá trị |
|---|---|
| Nguồn dao động | HSI nội 16 MHz (không dùng thạch anh ngoài) |
| PLL | M=8, N=100, P=2, Q=4 → 16/8×100/2 = **SYSCLK 100 MHz** |
| AHB (HCLK) | 100 MHz (DIV1) |
| APB1 (PCLK1) | 50 MHz (DIV2) — SPI2, I2C1, TIM2 (timer clock = 100 MHz) |
| APB2 (PCLK2) | 100 MHz (DIV1) — SPI1 |
| Flash latency | 3 wait states |
| Regulator | Voltage scale 1 |

## 3. Bảng mapping chân (pin map)

Nguồn: [main.h](../Core/Inc/main.h) và [stm32f4xx_hal_msp.c](../Core/Src/stm32f4xx_hal_msp.c)

| Chân | Tên define | Chức năng | Cấu hình |
|---|---|---|---|
| PC13 | `Led_Status_Pin` | LED status (blink 1s, **active low**) | Output PP — LogicalChannel_8 |
| PA9 | — | USART1 TX → ESP8266 RX | AF7, 115200 8N1 — `UART_Esp` |
| PA10 | — | USART1 RX ← ESP8266 TX | AF7 |
| PA3 | `Touch_IRQ_Pin` | PENIRQ cảm ứng (low = chạm) | GPIO EXTI rising + **pull-up** (PENIRQ open-drain), NVIC chưa enable, đang polling — LogicalChannel_6 |
| PA4 | `Touch_CS_Pin` | CS cảm ứng XPT2046 | Output PP, **idle high** (nhả bus SPI1 dùng chung) — LogicalChannel_5 |
| PA5 | — | SPI1 SCK | AF5 |
| PA6 | — | SPI1 MISO | AF5 |
| PA7 | — | SPI1 MOSI | AF5 |
| PA8 | `SDCard_CS_Pin` | CS thẻ SD (bus SPI2) | Output PP, **idle high** (nhả bus SPI2 dùng chung với ESP) — LogicalChannel_7 |
| PB0 | `Screen_CS_Pin` | CS màn hình ILI9341 | Output PP — LogicalChannel_1 |
| PB1 | `Screen_RST_Pin` | Reset màn hình | Output PP — LogicalChannel_2 |
| PB2 | `Screen_DC_Pin` | Data/Command màn hình | Output PP — LogicalChannel_3 |
| PB10 | `Screen_Led_Pin` | Đèn nền màn hình | Output PP — LogicalChannel_4 |
| PB13 | — | SPI2 SCK | AF5 |
| PB14 | — | SPI2 MISO | AF5 |
| PB15 | — | SPI2 MOSI | AF5 |

## 4. Cấu hình ngoại vi

### SPI ([Wrappers_Spi.c](../Wrappers/Wrappers_Spi/Wrappers_Spi.c))

| | SPI1 (kênh logic 1) | SPI2 (kênh logic 2) |
|---|---|---|
| Dùng cho | Màn hình ILI9341 (`SPI_Screen`) + cảm ứng XPT2046 dùng chung bus (CS riêng) | Thẻ SD (ESP đã chuyển sang UART) |
| Mode | Master, 2 lines, 8-bit, MSB first | Master, 2 lines, 8-bit, MSB first |
| CPOL/CPHA | Low / 1 edge (mode 0) | Low / 1 edge (mode 0) |
| NSS | Software (CS điều khiển bằng GPIO) | Software |
| Prescaler | /8 → **12.5 Mbit/s** (APB2 100MHz) | /2 → **25 Mbit/s** (APB1 50MHz) |
| Transmit | `HAL_SPI_Transmit`, blocking, timeout 100ms | như SPI1 |

**Kênh logic 3 (`SPI_Touch`)**: cũng là SPI1 nhưng mỗi transaction tạm hạ prescaler xuống **/64 → 1.5625 Mbit/s** (XPT2046 chỉ chịu ~2MHz) rồi trả về /8 — xử lý trong `Wrappers_Spi_Transmit/TransmitReceive`. `Wrappers_Spi_TransmitReceive()` (full-duplex) được thêm cho cảm ứng.

**Kênh logic 4 (`SPI_SdCard`)**: SPI2 full speed (**25 Mbit/s**, APB1 50MHz /2) cho pha dữ liệu thẻ SD. **Kênh logic 5 (`SPI_SdCardInit`)**: SPI2 hạ prescaler **/128 → 390.6 kbit/s** vì chuẩn SD yêu cầu ≤400kHz trong pha init. `Libs_SdCard` tự chuyển từ kênh 5 sang kênh 4 sau khi init thành công. Lưu ý: 25Mbit/s là mức trần của SD default-speed mode — nếu dây dài / đọc lỗi chập chờn, hạ prescaler SPI2 trong `Wrappers_Spi_Init` xuống /4 (12.5Mbit/s).

### USART1 — link ESP8266 Station ([Wrappers_Uart.c](../Wrappers/Wrappers_Uart/Wrappers_Uart.c))
- PA9 TX / PA10 RX, 115200 8N1, kênh logic `UART_Esp` = 1. Cấu hình do CubeMX (`MX_USART1_UART_Init`).
- `Wrappers_Uart_Transmit` (blocking, timeout 100ms) và `Wrappers_Uart_Receive` (polling, đọc hết byte đang chờ, tự xóa lỗi overrun).
- **TODO**: bật USART1 NVIC trong CubeMX + chuyển RX sang IRQ ring buffer khi làm `Libs_EspLink` — polling sẽ rớt byte nếu vòng lặp chính bận quá ~90µs/byte. Protocol chung 2 phía: `Station/src/Link/StationLinkCore.h`.

### TIM1 — HAL timebase
- Từ lần gen CubeMX 2026-08-28, tick HAL (`HAL_GetTick/HAL_Delay`) chạy bằng **TIM1** ([stm32f4xx_hal_timebase_tim.c](../Core/Src/stm32f4xx_hal_timebase_tim.c), IRQ `TIM1_UP_TIM10_IRQHandler`), không còn dùng SysTick. Code trong `SysTick_Handler` (FatFsCnt/Timer1/Timer2) hiện **không chạy nữa** — không sao vì driver SD không dùng chúng.

### TIM2 ([Wrappers_Pwm.c](../Wrappers/Wrappers_Pwm/Wrappers_Pwm.c))
- Base timer: prescaler 10000, period 999 → tick 10 kHz, update ~10 Hz (timer clock 100 MHz).
- Clock source internal, TRGO reset. **Chưa cấu hình PWM output channel** và chưa start timer.

### GPIO ([Wrappers_Gpio.c](../Wrappers/Wrappers_Gpio/Wrappers_Gpio.c))
- Enable clock port A + B; các chân output khởi tạo mức RESET (0), **riêng Touch_CS và SDCard_CS khởi tạo SET** (CS active-low, nhả bus SPI dùng chung — touch trên SPI1, SD trên SPI2).
- Đọc/ghi qua macro `Wrappers_Gpio_Write(LogicalChannel_x, level)` / `Wrappers_Gpio_Read(LogicalChannel_x)`.

### Cảm ứng XPT2046 ([Libs_XPT2046_Driver.c](../Libs/Libs_Touch_XPT2046/Libs_XPT2046_Driver.c))
- Nối chung bus SPI1 với màn hình, CS riêng (PA4), PENIRQ (PA3, low = đang chạm).
- Đọc bằng polling: lệnh `0x90` (Y) / `0xD0` (X), mỗi lần đọc lấy trung bình `XPT2046_POSITION_SAMPLES` (256) mẫu, giữ CS low suốt chuỗi mẫu (~20ms).
- Trả tọa độ theo hệ 240×320 (calibration cho rotation `VERTICAL_1`, hardcode trong header — cần đo lại theo panel).

## 5. Màn hình ILI9341

- Panel 320×240, màu RGB565 (2 byte/pixel), giao tiếp SPI1 + 4 chân điều khiển (CS/DC/RST/LED).
- Header [Libs_ILI9341_Driver.h](../Libs/Libs_Screen_ILI9341/Libs_ILI9341_Driver.h) ghi chú benchmark: với SPI 50MHz đạt tối đa lý thuyết ~40 FPS full-screen; hiện SPI1 chạy 12.5 Mbit/s nên FPS thực tế thấp hơn (~10 FPS full-screen lý thuyết).
- Đèn nền bật bằng `Libs_Gpio_Write(LogicalChannel_4, 1)` (chưa dùng PWM để chỉnh độ sáng — TIM2 có thể dành cho việc này sau).
- Lưu ý: define `ILI9341_PINK` đang trùng giá trị với `ILI9341_MAGENTA` (0xF81F) — bug nhỏ trong bảng màu.

### Quy tắc giao tiếp SPI với ILI9341 (quan trọng khi viết hàm vẽ mới)

Panel **chỉ nhận dữ liệu khi CS = 0**, và **DC phải = 1** khi gửi data. Vì `Libs_ILI9341_WriteCommand()` tự nhả CS lên 1 sau khi gửi lệnh, mọi dữ liệu gửi tiếp sau đó **phải tự kéo CS xuống và DC lên** — nếu không, LCD bỏ qua toàn bộ và màn hình không đổi. Dùng sẵn các helper trong driver thay vì gọi thẳng `Wrappers_Spi_Transmit`:

| Helper | Dùng khi |
|---|---|
| `Libs_ILI9341_WriteCommand(cmd)` | gửi 1 byte lệnh |
| `Libs_ILI9341_WriteData(byte)` | gửi 1 byte data |
| `Libs_ILI9341_WriteDataBuffer(buf, size)` | gửi nhiều byte data (tự set DC=1, CS=0, nhả CS khi xong) |

Đây từng là bug làm mọi hiệu ứng vẽ qua `DrawPixel` (spinner, bóng nảy, starfield, sóng sin) hiển thị đen — đã sửa 2026-08-23.

### Hiệu năng hàm vẽ

- `Libs_ILI9341_DrawPixel()` tốn 6 transaction SPI cho 1 điểm ảnh → **rất chậm**, chỉ dùng cho điểm lẻ.
- `DrawHorizontalLine` / `DrawVerticalLine` / `DrawRectangle` / `DrawFilledCircle` gửi theo **burst** (tối đa 500 byte/lần) → nhanh hơn hàng chục lần. Ưu tiên các hàm này khi vẽ khối.
- `Libs_ILI9341_DrawFilledCircle()` đã được viết lại theo từng dòng ngang (burst) thay vì từng pixel — cần thiết để animation chạy mượt.

## 6. Thẻ SD + FatFs

- Middleware: FatFs (bản đi kèm CubeMX) tại [Middlewares/Third_Party/FatFs/](../Middlewares/Third_Party/FatFs/src/) — **code third-party, không sửa trực tiếp**.
- Glue code: [FATFS/App/fatfs.c](../FATFS/App/fatfs.c) (link driver `USER_Driver`, `get_fattime()` trả 0 vì chưa có RTC) và [FATFS/Target/user_diskio.c](../FATFS/Target/user_diskio.c).
- **Trạng thái: hoạt động.** [Libs_SdCard](../Libs/Libs_SdCard/Libs_SdCard.c) implement giao thức SD-SPI (CMD0/CMD8/ACMD41/CMD58; hỗ trợ SDv1/SDv2/SDHC/MMC, nhận diện block-addressing qua bit CCS trong OCR); `user_diskio.c` chỉ là glue gọi xuống driver trong các block `USER CODE`.
- Bus: **SPI2** (dùng chung với ESP, CS riêng PA8). Init ở 390kHz (kênh `SPI_SdCardInit`), chạy ở 25Mbit/s (kênh `SPI_SdCard`). Đọc/ghi đơn block (CMD17/CMD24) lặp cho nhiều block; `GET_SECTOR_COUNT` đọc CSD nên `f_mkfs` dùng được.
- Toàn chuỗi FatFs → user_diskio → Libs_SdCard đã verify bằng simulator thẻ SDHC trên host (mkfs → mount → ghi/đọc file round-trip).

## 7. Ngắt (Interrupts)

Nguồn: [stm32f4xx_it.c](../Core/Src/stm32f4xx_it.c)

- Chỉ có các system handler mặc định (NMI, HardFault, SysTick...). `SysTick_Handler` phục vụ `HAL_Delay`/tick.
- Fault handler (HardFault, BusFault...) đều rơi vào vòng lặp vô hạn.
- Chưa có IRQ ngoại vi nào được enable (kể cả EXTI3 cho touch).

## 8. Xử lý lỗi

- Mọi lời gọi `HAL_xxx_Init` đều kiểm tra `!= HAL_OK` → gọi `Error_Handler()` ([main.c](../Core/Src/main.c)): tắt ngắt (`__disable_irq`) và dừng trong vòng lặp vô hạn.
- Chưa có cơ chế báo lỗi ra ngoài (LED nháy, log...) — có thể bổ sung sau.
