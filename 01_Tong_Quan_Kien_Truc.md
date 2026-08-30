# 01. Tổng quan dự án & Kiến trúc phần mềm

## 1. Dự án là gì?

**DesktopAssistant** là một thiết bị trợ lý để bàn. Repo này chứa firmware cho **ECU chính (MainECU)** chạy trên vi điều khiển **STM32F411CEUx** (Cortex-M4).

Nhiệm vụ hiện tại của MainECU:

- Điều khiển **màn hình TFT ILI9341** 320×240 (SPI) — vẽ màu nền, vẽ hình, vẽ chữ, hiển thị ảnh.
- Phát **video** (từ flash hoặc thẻ SD) và chạy **animation** (bóng nảy, spinner, progress bar, sóng sin, starfield, chữ trượt, số đếm, fade) trên màn hình.
- Đọc **cảm ứng điện trở XPT2046** (dùng chung bus SPI1 với màn hình, polling).
- Đọc/ghi **thẻ SD** qua FatFs (driver SD-SPI tự viết, hoạt động).
- Kết nối **ESP8266 Station** qua **USART1** (PA9/PA10, 115200) — base code ESP ở folder `Station/`, protocol chung tại `Station/src/Link/StationLinkCore.h` (Libs_EspLink phía STM32: TBD).
- **LED PC13** blink chu kỳ 1s báo hệ thống còn sống.

## 2. Cấu trúc thư mục

```
MainECU_STM32/
├── Core/                  # Entry point (main.c) + file hệ thống CubeMX sinh ra
│   ├── Inc/               #   main.h (pin map, logical channel), cấu hình HAL, IT
│   ├── Src/               #   main.c (entry point), stm32f4xx_it.c, hal_msp.c, ...
│   └── Startup/           #   startup_stm32f411ceux.s
├── Libs/                  # Tầng thư viện logic (API cho ứng dụng) — 1 thư mục / module
│   ├── Libs_System/       #   GetTick/DelayMs (init hệ thống do CubeMX)
│   ├── Libs_Clock/        #   (giữ API; clock config do CubeMX)
│   ├── Libs_Gpio/         #   GPIO logic
│   ├── Libs_Spi/          #   SPI logic
│   ├── Libs_Uart/         #   UART logic (link ESP8266 Station)
│   ├── Libs_Pwm/          #   PWM/Timer logic
│   ├── Libs_Screen_ILI9341/  # Driver màn hình: Driver (lệnh LCD) + GFX (vẽ hình/chữ/ảnh) + Font
│   ├── Libs_Touch_XPT2046/   # Driver cảm ứng điện trở XPT2046 (SPI1 dùng chung, polling)
│   ├── Libs_Video/        #   Phát video RGB565: từ flash (mảng C) hoặc thẻ SD (file .rvd)
│   ├── Libs_Animation/    #   Hiệu ứng animation: easing, blend màu, ball/spinner/progress/wave/starfield/text/counter/fade
│   └── Libs_SdCard/       #   Driver thẻ SD/SDHC chế độ SPI (FatFs gọi qua user_diskio.c)
├── App/                   # Tầng ứng dụng — 1 thư mục / app
│   └── App_HwTest/        #   App test phần cứng: screen / SD card / touch, báo kết quả lên màn hình
├── Wrappers/              # Tầng bọc HAL — CHỈ tầng này được gọi HAL trực tiếp
│   ├── Wrappers_System/   #   HAL_Init
│   ├── Wrappers_Clock/    #   RCC/PLL config
│   ├── Wrappers_Gpio/     #   GPIO Write/Read/Toggle (init do CubeMX)
│   ├── Wrappers_Spi/      #   SPI transmit + đổi prescaler theo kênh (handle CubeMX)
│   ├── Wrappers_Uart/     #   USART1 transmit/receive (handle CubeMX)
│   └── Wrappers_Pwm/      #   TIM2 (init do CubeMX)
├── FATFS/                 # Glue code FatFs do CubeMX sinh (App/ + Target/)
├── Middlewares/           # Third-party: FatFs source
└── Doc/                   # Tài liệu dự án (bộ tài liệu này)
```

Quy tắc: **mỗi module một thư mục riêng**, tên thư mục = tên file = `<Tầng>_<Module>` (ví dụ [Libs/Libs_Spi/Libs_Spi.c](../Libs/Libs_Spi/Libs_Spi.c)).

## 3. Kiến trúc phân lớp

```
┌─────────────────────────────────────────────┐
│  Core — main.c                              │   Entry: init hệ thống rồi gọi App
├─────────────────────────────────────────────┤
│  App (Application) — App_Xxx                │   Logic ứng dụng; gọi Libs_* và
│                                             │   API middleware (FatFs f_*)
├─────────────────────────────────────────────┤
│  Libs (Logical layer) — Libs_Xxx            │   API logic, driver thiết bị (ILI9341)
│                                             │   Chỉ gọi Wrappers_*, KHÔNG gọi HAL
├─────────────────────────────────────────────┤
│  Wrappers (HAL wrapper) — Wrappers_Xxx      │   Nơi DUY NHẤT gọi HAL_* / __HAL_*
│                                             │   Dùng HAL handle CubeMX sinh (extern),
│                                             │   map logical channel → peripheral vật lý
├─────────────────────────────────────────────┤
│  STM32 HAL / CMSIS / FatFs (Middlewares)    │   Code hãng / third-party, không sửa
└─────────────────────────────────────────────┘
```

Mục đích của kiến trúc này:

- **Tách phần cứng khỏi logic**: đổi chân, đổi peripheral, hoặc port sang MCU khác chỉ cần sửa tầng Wrappers và [main.h](../Core/Inc/main.h); tầng Libs và ứng dụng không đổi.
- **Logical channel**: ứng dụng làm việc với "kênh logic" (số nguyên / macro), không biết đến port/pin/SPI instance vật lý. Xem chi tiết mục 4.

### Quy tắc phụ thuộc giữa các tầng

| Tầng | Được phép gọi | Không được gọi |
|---|---|---|
| Core (main.c) | `App_*`, `Libs_*` (phần init), `MX_FATFS_Init` | `Wrappers_*`, `HAL_*` (ngoại lệ: `Error_Handler`) |
| App | `Libs_*`, API middleware (FatFs `f_*`) | `Wrappers_*`, `HAL_*` |
| Libs | `Wrappers_*`, `Libs_*` khác | `HAL_*` (ngoại lệ hiện tại: `HAL_Delay` trong driver ILI9341) |
| Wrappers | `HAL_*`, `__HAL_*` | `Libs_*` (không gọi ngược lên trên) |

> Ghi chú: `HAL_Delay()` hiện được dùng thẳng ở Core và Libs. Nếu muốn triệt để, nên bọc thành `Libs_System_DelayMs()` — hiện tại đây là ngoại lệ được chấp nhận.

### Hai kiểu ủy quyền (delegation) từ Libs xuống Wrappers

1. **Hàm pass-through** — khi cần giữ chỗ để sau này thêm logic:
   ```c
   void Libs_Spi_Transmit(uint8_t p_LogicalChannel_u8, uint8_t* p_Data_ptr, uint16_t p_Size_u16)
   {
       Wrappers_Spi_Transmit(p_LogicalChannel_u8, p_Data_ptr, p_Size_u16);
   }
   ```
2. **Macro alias** — khi chắc chắn chỉ chuyển tiếp thuần túy (tránh tốn call overhead), như trong [Libs_Gpio.h](../Libs/Libs_Gpio/Libs_Gpio.h):
   ```c
   #define Libs_Gpio_Write    Wrappers_Gpio_Write
   #define Libs_Gpio_Read     Wrappers_Gpio_Read
   ```

## 4. Cơ chế Logical Channel

Toàn bộ mapping tài nguyên vật lý được tập trung ở [main.h](../Core/Inc/main.h):

```c
// GPIO: mỗi LogicalChannel_x expand thành cặp (GPIO_Port, Pin)
#define LogicalChannel_1    Screen_CS_GPIO_Port,Screen_CS_Pin    // ILI9341 Chip Select
#define LogicalChannel_2    Screen_RST_GPIO_Port,Screen_RST_Pin  // ILI9341 Reset
#define LogicalChannel_3    Screen_DC_GPIO_Port,Screen_DC_Pin    // ILI9341 Data/Command
#define LogicalChannel_4    Screen_Led_GPIO_Port,Screen_Led_Pin  // ILI9341 Backlight LED

// SPI: kênh logic là số nguyên, Wrappers_Spi_Transmit() switch theo số này
#define SPI_Screen          1        // 1 → SPI1, 2 → SPI2
```

- Với **GPIO**: macro `LogicalChannel_x` chứa sẵn dấu phẩy nên 1 "tham số" khi expand thành 2 tham số của `HAL_GPIO_WritePin`. Vì vậy `Wrappers_Gpio_Write` bắt buộc là macro (xem [Wrappers_Gpio.h](../Wrappers/Wrappers_Gpio/Wrappers_Gpio.h)), không thể là hàm.
- Với **SPI**: kênh là `uint8_t`, [Wrappers_Spi.c](../Wrappers/Wrappers_Spi/Wrappers_Spi.c) chọn handle `SPI1`/`SPI2` bằng if/else theo giá trị kênh.
- **Khi thêm thiết bị mới**: khai báo pin define + LogicalChannel mới trong main.h, thêm nhánh xử lý kênh trong Wrappers tương ứng. Không hard-code port/pin ở tầng Libs.

## 5. Luồng khởi động (main.c) — từ 2026-08-28 Core thuộc quyền CubeMX

`main.c`/`main.h` giữ nguyên skeleton do CubeMX sinh; **mọi code tay nằm trong các block
`/* USER CODE BEGIN/END */`** để sống sót qua các lần re-generate:

```c
int main(void)                       // CubeMX skeleton
{
    HAL_Init();                      // CubeMX (timebase TIM1)
    SystemClock_Config();            // CubeMX (HSI+PLL 100MHz)
    MX_GPIO_Init();                  // CubeMX (PC13 LED, CS idle-high, ...)
    MX_SPI1_Init(); MX_FATFS_Init(); MX_TIM2_Init(); MX_SPI2_Init();
    MX_USART1_UART_Init();           // CubeMX (ESP link 115200)
    /* USER CODE BEGIN 2 */
    Libs_ILI9341_Init();             // từ đây trở đi: chỉ qua tầng Libs/App
    Libs_XPT2046_Init();
    Libs_Gpio_Write(LogicalChannel_4, 1);   // backlight
    App_HwTest_Init();
    /* USER CODE END 2 */
    while (1)
    {
        /* USER CODE BEGIN 3 */
        App_HwTest_Run();
        // blink PC13 mỗi 500ms (chu kỳ 1s) qua Libs_Gpio_Toggle
        /* USER CODE END 3 */
    }
}
```

Hệ quả với tầng Wrappers: HAL handle (`hspi1`, `hspi2`, `huart1`, `htim2`) do CubeMX sinh trong
main.c; Wrappers `extern` chúng và vẫn là **nơi duy nhất sử dụng** — các hàm `Wrappers_*_Init`
thành no-op (giữ API). Định nghĩa pin/kênh logic trong `main.h` phải đặt trong block
`USER CODE Private defines`.

## 6. Module màn hình ILI9341

Module lớn nhất và là ứng dụng chính hiện tại, gồm 3 file trong [Libs/Libs_Screen_ILI9341/](../Libs/Libs_Screen_ILI9341/):

| File | Vai trò |
|---|---|
| [Libs_ILI9341_Driver.c/h](../Libs/Libs_Screen_ILI9341/Libs_ILI9341_Driver.c) | Tầng thấp: reset, chuỗi lệnh init, set address window, set rotation, fill screen, draw pixel/rectangle/line. Giao tiếp LCD qua `Wrappers_Spi_Transmit` + `Wrappers_Gpio_Write` (CS/DC/RST) |
| [Libs_ILI9341_GFX.c/h](../Libs/Libs_Screen_ILI9341/Libs_ILI9341_GFX.c) | Tầng vẽ: hình tròn (rỗng/đặc), chữ nhật theo tọa độ, ký tự / chuỗi (font 6×8, scale được), ảnh full-screen RGB565 |
| [Libs_ILI9341_Font.h](../Libs/Libs_Screen_ILI9341/Libs_ILI9341_Font.h) | Font bitmap 96 ký tự ASCII (từ ' '), mảng `g_Font_aa[96][6]` |

Kiến thức quan trọng:

- Màu là **RGB565 16-bit**, gửi big-endian (byte cao trước). Các màu chuẩn define sẵn trong Driver.h (`ILI9341_ORANGE`, ...).
- Gửi dữ liệu màu theo **burst tối đa 500 byte** (`ILI9341_BURST_MAX_SIZE`) để cân bằng RAM/tốc độ.
- Protocol: kéo DC=0 + CS=0 để gửi lệnh, DC=1 để gửi data; mỗi lần gửi xong nhả CS=1.
- 4 hướng xoay màn hình: `ILI9341_SCREEN_VERTICAL_1/2`, `ILI9341_SCREEN_HORIZONTAL_1/2`. Kích thước hiện hành lưu trong biến static `l_ScreenWidth_u16` / `l_ScreenHeight_u16`.
- `Libs_ILI9341_DrawImage()` nhận mảng ảnh 320×240×2 byte, convert bằng tool: http://www.digole.com/tools/PicturetoC_Hex_converter.php
- Vẽ pixel đơn rất chậm — ưu tiên rectangle/line (dùng burst) khi vẽ khối lớn.
- `Libs_ILI9341_GetScreenWidth()/GetScreenHeight()` trả về kích thước theo rotation hiện hành — các module vẽ khác (Video, Animation) dùng để check biên.

## 6b. Các module hiển thị & cảm ứng mở rộng (thêm 2026-08-22)

| Module | Vai trò | Ghi chú |
|---|---|---|
| [Libs_Touch_XPT2046](../Libs/Libs_Touch_XPT2046/Libs_XPT2046_Driver.h) | Đọc cảm ứng điện trở (polling): `Libs_XPT2046_IsPressed()`, `Libs_XPT2046_ReadCoordinates()` | Port từ lib của Matej Artnak (MIT) — chuyển từ bit-bang sang **SPI1 phần cứng dùng chung với màn hình**, hạ tốc độ xuống 1.5625MHz mỗi transaction qua kênh logic `SPI_Touch`. Calibration hardcode cho `VERTICAL_1`, cần đo lại theo panel thực tế |
| [Libs_Video](../Libs/Libs_Video/Libs_Video.h) | Phát video RGB565BE: chế độ **flash** (mảng frame trong code, non-blocking theo FPS) và chế độ **SD** (stream file `.rvd` — bật bằng `VIDEO_USE_SDCARD=1`, SD driver đã có) | `Libs_Video_DrawFrame()` blit thẳng từ bộ nhớ, nhanh hơn `DrawImage`. Script `Tools/video2c.py` được nhắc đến trong header **chưa có trong repo** |
| [Libs_Animation](../Libs/Libs_Animation/Libs_Animation.h) | 6 hàm easing, blend màu RGB565, và các hiệu ứng: ball, spinner, progress bar, wave, starfield 3D, text slide-in, counter, screen fade + `Libs_Animation_Demo()` | Nguyên tắc: không xóa cả màn hình mỗi frame (dirty rectangle), animation theo thời gian thực (`Libs_System_GetTick`) |

## 7. Trạng thái hiện tại & phần còn thiếu (tính đến 2026-08-22)

**Đã chạy được:**
- Khởi tạo hệ thống, clock, SPI/I2C/GPIO/TIM.
- Màn hình ILI9341: init, fill màu, vẽ hình/chữ, hiển thị ảnh.
- Cảm ứng XPT2046 (polling qua SPI1 dùng chung — cần calibrate lại theo panel).
- Video từ flash + toàn bộ hiệu ứng animation (xem mục 6b).
- Thẻ SD qua FatFs: driver SPI-mode [Libs_SdCard](../Libs/Libs_SdCard/Libs_SdCard.h) + `user_diskio.c` (mount / đọc / ghi file).
- App test phần cứng [App_HwTest](../App/App_HwTest/App_HwTest.h) — app hiện hành chạy trong `while(1)` của main.c.

**Còn thiếu / cần lưu ý:**

| Hạng mục | Trạng thái |
|---|---|
| `Photo_Sources.h` | main.c **không còn dùng** (đã chuyển sang App_HwTest) nên không chặn build nữa; khi cần hiển thị ảnh tĩnh thì tạo lại file này bằng tool convert |
| `Tools/video2c.py` | Script convert video được nhắc trong [Libs_Video.h](../Libs/Libs_Video/Libs_Video.h) nhưng **chưa có trong repo** — cần bổ sung để tạo dữ liệu frame/.rvd |
| SD card | **Đã implement** ([Libs_SdCard](../Libs/Libs_SdCard/Libs_SdCard.c) + [user_diskio.c](../FATFS/Target/user_diskio.c)). Đọc/ghi theo đơn block (CMD17/CMD24) — đủ cho test và file nhỏ; khi stream video từ SD nên nâng cấp multi-block (CMD18/CMD25). Có thể bật `VIDEO_USE_SDCARD=1` trong Libs_Video.h |
| Touch calibration | Các hằng `XPT2046_X/Y_OFFSET`, `_MAGNITUDE`, `_TRANSLATION` lấy nguyên từ lib gốc (panel khác) — cần đo lại trên phần cứng thật. Chế độ ngắt EXTI3 vẫn chưa dùng (đang polling) |
| ESP (SPI2 + I2C1) | Peripheral đã init nhưng **chưa có protocol/logic ứng dụng** |
| PWM | Module tên là Pwm nhưng [Wrappers_Pwm.c](../Wrappers/Wrappers_Pwm/Wrappers_Pwm.c) mới init **TIM2 base timer**, chưa cấu hình PWM output channel |
| File project (.ioc, .project, ...) | Không có trong repo — project gốc tạo bằng STM32CubeIDE/CubeMX (dựa vào cấu trúc file sinh ra). Nhớ thêm các thư mục `Libs_*` mới và `App/App_HwTest` vào include path + source path khi build |
| README.md gốc | Chỉ có tiêu đề, chưa có nội dung |
| `DesktopAssistant/References/` | Bản gốc của 3 lib touch/video/animation (trước khi port) — giữ để đối chiếu, **không đưa vào build** |
