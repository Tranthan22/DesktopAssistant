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
| PA3 | `Touch_IRQ_Pin` | PENIRQ cảm ứng (low = chạm) | GPIO EXTI rising + **pull-up** (PENIRQ open-drain), NVIC chưa enable, đang polling — LogicalChannel_6 |
| PA4 | `Touch_CS_Pin` | CS cảm ứng XPT2046 | Output PP, **idle high** (nhả bus SPI1 dùng chung) — LogicalChannel_5 |
| PA5 | — | SPI1 SCK | AF5 |
| PA6 | — | SPI1 MISO | AF5 |
| PA7 | — | SPI1 MOSI | AF5 |
| PA8 | `SDCard_CS_Pin` | CS thẻ SD | Output PP, low speed |
| PB0 | `Screen_CS_Pin` | CS màn hình ILI9341 | Output PP — LogicalChannel_1 |
| PB1 | `Screen_RST_Pin` | Reset màn hình | Output PP — LogicalChannel_2 |
| PB2 | `Screen_DC_Pin` | Data/Command màn hình | Output PP — LogicalChannel_3 |
| PB6 | `ESP_SCL_Pin` | I2C1 SCL (ESP) | AF4 |
| PB7 | `ESP_SDA_Pin` | I2C1 SDA (ESP) | AF4 |
| PB10 | `Screen_Led_Pin` | Đèn nền màn hình | Output PP — LogicalChannel_4 |
| PB12 | `ESP_CS_Pin` | CS module ESP | Output PP |
| PB13 | — | SPI2 SCK | AF5 |
| PB14 | — | SPI2 MISO | AF5 |
| PB15 | — | SPI2 MOSI | AF5 |

## 4. Cấu hình ngoại vi

### SPI ([Wrappers_Spi.c](../Wrappers/Wrappers_Spi/Wrappers_Spi.c))

| | SPI1 (kênh logic 1) | SPI2 (kênh logic 2) |
|---|---|---|
| Dùng cho | Màn hình ILI9341 (`SPI_Screen`), touch + SD dùng chung bus (CS riêng) | Module ESP |
| Mode | Master, 2 lines, 8-bit, MSB first | Master, 2 lines, 8-bit, MSB first |
| CPOL/CPHA | Low / 1 edge (mode 0) | Low / 1 edge (mode 0) |
| NSS | Software (CS điều khiển bằng GPIO) | Software |
| Prescaler | /8 → **12.5 Mbit/s** (APB2 100MHz) | /2 → **25 Mbit/s** (APB1 50MHz) |
| Transmit | `HAL_SPI_Transmit`, blocking, timeout 100ms | như SPI1 |

**Kênh logic 3 (`SPI_Touch`)**: cũng là SPI1 nhưng mỗi transaction tạm hạ prescaler xuống **/64 → 1.5625 Mbit/s** (XPT2046 chỉ chịu ~2MHz) rồi trả về /8 — xử lý trong `Wrappers_Spi_Transmit/TransmitReceive`. `Wrappers_Spi_TransmitReceive()` (full-duplex) được thêm cho cảm ứng.

### I2C1 ([Wrappers_I2c.c](../Wrappers/Wrappers_I2c/Wrappers_I2c.c))
- 100 kHz (standard mode), duty cycle 2, addressing 7-bit. Chưa có hàm read/write — mới chỉ init.

### TIM2 ([Wrappers_Pwm.c](../Wrappers/Wrappers_Pwm/Wrappers_Pwm.c))
- Base timer: prescaler 10000, period 999 → tick 10 kHz, update ~10 Hz (timer clock 100 MHz).
- Clock source internal, TRGO reset. **Chưa cấu hình PWM output channel** và chưa start timer.

### GPIO ([Wrappers_Gpio.c](../Wrappers/Wrappers_Gpio/Wrappers_Gpio.c))
- Enable clock port A + B; các chân output khởi tạo mức RESET (0), **riêng Touch_CS khởi tạo SET** (CS active-low, nhả bus SPI1 dùng chung).
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

## 6. Thẻ SD + FatFs

- Middleware: FatFs (bản đi kèm CubeMX) tại [Middlewares/Third_Party/FatFs/](../Middlewares/Third_Party/FatFs/src/) — **code third-party, không sửa trực tiếp**.
- Glue code: [FATFS/App/fatfs.c](../FATFS/App/fatfs.c) (link driver `USER_Driver`, `get_fattime()` trả 0 vì chưa có RTC) và [FATFS/Target/user_diskio.c](../FATFS/Target/user_diskio.c).
- **Trạng thái: chưa hoạt động.** `user_diskio.c` là stub — cần implement `USER_initialize/status/read/write/ioctl` bằng giao thức SD-SPI (qua SPI1 + `SDCard_CS_Pin`) trong các block `USER CODE`.

## 7. Ngắt (Interrupts)

Nguồn: [stm32f4xx_it.c](../Core/Src/stm32f4xx_it.c)

- Chỉ có các system handler mặc định (NMI, HardFault, SysTick...). `SysTick_Handler` phục vụ `HAL_Delay`/tick.
- Fault handler (HardFault, BusFault...) đều rơi vào vòng lặp vô hạn.
- Chưa có IRQ ngoại vi nào được enable (kể cả EXTI3 cho touch).

## 8. Xử lý lỗi

- Mọi lời gọi `HAL_xxx_Init` đều kiểm tra `!= HAL_OK` → gọi `Error_Handler()` ([main.c](../Core/Src/main.c)): tắt ngắt (`__disable_irq`) và dừng trong vòng lặp vô hạn.
- Chưa có cơ chế báo lỗi ra ngoài (LED nháy, log...) — có thể bổ sung sau.
