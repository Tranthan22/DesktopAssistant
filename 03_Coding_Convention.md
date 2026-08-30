# 03. Coding Convention

Quy tắc dưới đây được trích xuất từ code hiện tại của project. Code mới **bắt buộc** tuân theo; code cũ chưa đúng chuẩn (chủ yếu phần driver ILI9341 port từ open-source) sẽ chỉnh dần khi có dịp sửa.

## 1. Đặt tên file & thư mục

- Mỗi module một thư mục: `<Tầng>/<Tầng>_<Module>/<Tầng>_<Module>.c|.h`
  - Ví dụ: `Libs/Libs_Spi/Libs_Spi.c`, `Wrappers/Wrappers_Gpio/Wrappers_Gpio.h`
- Module nhiều file thì thêm hậu tố chức năng: `Libs_ILI9341_Driver.c`, `Libs_ILI9341_GFX.c`, `Libs_ILI9341_Font.h`
- Tên module viết PascalCase, viết tắt ngoại vi theo kiểu `Spi`, `I2c`, `Gpio`, `Pwm` (chỉ chữ cái đầu hoa).

## 2. Đặt tên hàm

Công thức: **`<Tầng>_<Module>_<HànhĐộng>`** — PascalCase từng phần, nối bằng `_`.

```c
void Libs_Spi_Init(void);
void Libs_Spi_Transmit(...);
void Wrappers_Gpio_Init(void);
void Libs_ILI9341_DrawFilledCircle(...);
```

- Hàm `static` trong file vẫn mang đầy đủ prefix tầng + module: `static void Libs_ILI9341_DrawColourBurst(...)`.
- Hàm static nhỏ, gọi nhiều nơi có thể khai báo `inline static`.
- Hàm init của mọi module đặt tên `<Tầng>_<Module>_Init`.

## 3. Đặt tên biến — prefix theo scope + suffix theo kiểu dữ liệu

### Prefix (phạm vi)

| Prefix | Ý nghĩa | Ví dụ |
|---|---|---|
| `p_` | Tham số hàm (parameter) | `p_LogicalChannel_u8`, `p_Data_ptr` |
| `f_` | Biến cục bộ trong hàm (function-local) | `f_Gpio_InitStruct_st`, `f_Counter1_u8` |
| `l_` | Biến static phạm vi file (local to file) | `l_Spi_Handler1_st`, `l_ScreenWidth_u16` |
| `g_` | Biến toàn cục / exported (global) | `g_Font_aa` |

### Suffix (kiểu dữ liệu)

| Suffix | Kiểu | Ví dụ |
|---|---|---|
| `_u8` / `_u16` / `_u32` | `uint8_t` / `uint16_t` / `uint32_t` | `p_Size_u16` |
| `_i32` | `int32_t` (tương tự `_i8`, `_i16`) | `f_Error_i32` |
| `_char` | `char` / chuỗi | `p_Text_char` |
| `_float` | `float` | `f_CalcNegative_float` |
| `_st` | struct | `l_I2c_Handler_st` |
| `_ptr` | con trỏ (khi không nhấn mạnh kiểu phần tử) | `p_Data_ptr` |
| `_aa` | mảng 2 chiều (array of arrays) | `g_Font_aa[96][6]` |

Phần tên giữa prefix và suffix viết PascalCase: `p_LocationX_u16`, `f_RadiusError_i32`.

- Dùng kiểu độ rộng cố định từ `<stdint.h>` (`uint8_t`...), không dùng `int`/`unsigned` trần trong API.
- Tham số hàm luôn có prefix `p_` kể cả trong prototype ở header.

## 4. Đặt tên macro / hằng số

- Hằng số cấu hình / giá trị: **UPPER_SNAKE_CASE** có prefix module: `ILI9341_SCREEN_WIDTH`, `ILI9341_BURST_MAX_SIZE`, `ILI9341_ORANGE`.
- Macro định danh tài nguyên (pin, kênh logic): PascalCase theo kiểu CubeMX: `Screen_CS_Pin`, `Screen_CS_GPIO_Port`, `LogicalChannel_1`, `SPI_Screen`.
- Macro alias hàm giữ nguyên tên hàm chuẩn: `#define Libs_Gpio_Write  Wrappers_Gpio_Write`.

## 5. Cấu trúc file bắt buộc (section banner)

Mọi file `.c`/`.h` tự viết đều chia section bằng banner comment cố định, **giữ đủ section kể cả khi rỗng**.

### Template file `.h`

```c
#ifndef LIBS_XXX_LIBS_XXX_H_
#define LIBS_XXX_LIBS_XXX_H_

// /************************************************************************************************************
//  * INCLUDES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * MACROS AND DEFINES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * TYPEDEFS
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * EXPORTED VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * GLOBAL FUNCTION PROTOTYPES
//  ************************************************************************************************************/
void Libs_Xxx_Init(void);

#endif /* LIBS_XXX_LIBS_XXX_H_ */
```

Include guard theo mẫu hiện có:
- Libs: `LIBS_<MODULE>_LIBS_<MODULE>_H_` (ví dụ `LIBS_SPI_LIBS_SPI_H_`)
- Wrappers: `_WRAPPERS_<MODULE>_H_` (ví dụ `_WRAPPERS_SPI_H_`)

### Template file `.c`

```c
#include "Libs_Xxx.h"        // Header của chính module đứng đầu
#include "Wrappers_Xxx.h"    // Sau đó đến các dependency

// /************************************************************************************************************
//  * EXTERN VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * PRIVATE MACROS AND DEFINES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * PRIVATE TYPEDEFS
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * STATIC VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * GLOBAL VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * STATIC FUNCTION PROTOTYPES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * STATIC FUNCTIONS
//  ************************************************************************************************************/

// ************************************************************************************************************
//  * GLOBAL FUNCTIONS
//  ************************************************************************************************************/
void Libs_Xxx_Init(void)
{
}
```

## 6. Include

- File `.c` include header của chính nó **đầu tiên**, sau đó đến các header phụ thuộc.
- Header Wrappers include `"main.h"` (để lấy HAL + pin define). Header Libs chỉ include cái tối thiểu (`<stdint.h>`, hoặc header Wrappers nếu dùng macro alias).
- Không include HAL trực tiếp ở tầng Libs — HAL đến từ chuỗi `main.h` → `stm32f4xx_hal.h` khi thật sự cần.

## 7. Quy tắc phân tầng (tóm tắt — chi tiết ở [01_Tong_Quan_Kien_Truc.md](01_Tong_Quan_Kien_Truc.md))

1. **Chỉ Wrappers được gọi `HAL_*` / `__HAL_*`.**
2. Logic ứng dụng nằm ở tầng **App** (`App/App_<Tên>/App_<Tên>.c|h`, hàm `App_<Tên>_<Action>`); App chỉ gọi `Libs_*` và API middleware (FatFs `f_*`). Core (main.c) chỉ init hệ thống rồi gọi `App_*` trong `while(1)`.
3. HAL handle (`hspi1`, `huart1`...) do **CubeMX sinh trong main.c**; Wrappers `extern` chúng ở section EXTERN VARIABLES và là **nơi duy nhất sử dụng** (xem mục 11). Không tự tạo handle riêng.
4. Tài nguyên vật lý (port/pin/instance) chỉ xuất hiện ở `main.h` (define) và tầng Wrappers (sử dụng). Tầng Libs trở lên chỉ dùng **logical channel**.
5. Không sửa code third-party (`Middlewares/`) và hạn chế sửa ngoài block `USER CODE` trong file CubeMX sinh ra (`Core/Src/stm32f4xx_*.c`, `FATFS/`).

## 8. Xử lý lỗi & style khác

- Mọi lời gọi HAL init phải kiểm tra kết quả:
  ```c
  if (HAL_SPI_Init(&l_Spi_Handler1_st) != HAL_OK)
  {
      Error_Handler();
  }
  ```
- Switch/if phân nhánh theo kênh logic phải có nhánh `default:`/`else` (dù rỗng) — xem `Wrappers_Spi_Transmit`, `Libs_ILI9341_SetRotation`.
- Hàm vẽ/thao tác theo tọa độ phải **kiểm tra biên** trước khi thực hiện (out-of-bounds thì `return` sớm hoặc clamp) — xem `Libs_ILI9341_DrawPixel`, `DrawRectangle`.
- **Ngoặc nhọn kiểu Allman** — `{` xuống dòng riêng, áp dụng cho cả hàm và khối if/else/for/while:
  ```c
  void Libs_Spi_Init(void)
  {
      if (...)
      {
          ...
      }
      else
      {
      }
  }
  ```
- Thụt lề: 4 space hoặc tab (code hiện tại dùng lẫn theo file) — **thống nhất trong cùng một file**, file mới ưu tiên 4 space.
- Comment giải thích đặt trên dòng code, dạng `/* ... */` cho mô tả khối và `//` cho ghi chú ngắn. Comment mô tả chức năng hàm đặt ngay trên định nghĩa hàm.

## 9. Checklist khi thêm module ngoại vi mới (ví dụ: UART)

1. Tạo `Wrappers/Wrappers_Uart/Wrappers_Uart.c|.h` theo template mục 5:
   - Handle static: `UART_HandleTypeDef l_Uart_Handler_st;`
   - `Wrappers_Uart_Init()` cấu hình + kiểm tra `HAL_OK`.
   - Các hàm thao tác nhận `p_LogicalChannel_u8` nếu có nhiều instance.
   - Cấu hình chân AF thêm vào `HAL_UART_MspInit` trong [stm32f4xx_hal_msp.c](MainECU_STM32/Core/Src/stm32f4xx_hal_msp.c).
2. Tạo `Libs/Libs_Uart/Libs_Uart.c|.h`: pass-through hoặc macro alias xuống Wrappers.
3. Khai báo pin define + logical channel (nếu cần) trong [main.h](MainECU_STM32/Core/Inc/main.h).
4. Gọi `Libs_Uart_Init()` trong [main.c](MainECU_STM32/Core/Src/main.c) theo đúng thứ tự khởi động (sau Clock, cùng nhóm ngoại vi).
5. Thêm đường dẫn include của 2 thư mục mới vào cấu hình build (project settings của CubeIDE).

## 10. Điểm chưa nhất quán trong code hiện tại (biết để không sao chép theo)

- Driver ILI9341 (port từ open-source) còn nhiều tham số/biến **chưa theo convention**: `uint16_t X`, `Colour`, `Buffer_Size`, `chifted`, `burst_buffer`... Code mới không viết kiểu này; khi refactor thì đổi dần về chuẩn `p_`/`f_` (phần GFX đã convert xong, phần Driver mới convert một phần).
- `HAL_Delay()` được gọi thẳng ở Core/Libs thay vì qua Wrappers (ngoại lệ chấp nhận, xem tài liệu 01).
- Banner `GLOBAL FUNCTIONS` ở một số file thiếu `// /` mở đầu so với các banner khác — không ảnh hưởng, nhưng file mới nên dùng thống nhất một kiểu.
- `ILI9341_PINK` trùng giá trị `ILI9341_MAGENTA` (0xF81F).

## 11. Sống chung với CubeMX (bắt buộc từ 2026-08-28)

Thư mục `Core/` thuộc quyền CubeMX — nó sẽ **ghi đè toàn bộ code ngoài block USER CODE** mỗi lần
Generate. Quy tắc:

1. Code tay trong `Core/` (include, define, init app, vòng lặp) **chỉ** được viết trong
   `/* USER CODE BEGIN/END */`.
2. Pin define + LogicalChannel + kênh SPI/UART đặt trong block `USER CODE Private defines` của main.h.
3. HAL handle do CubeMX sinh (main.c); Wrappers `extern` và là nơi duy nhất dùng chúng.
   `Wrappers_*_Init` là no-op — tham số peripheral chỉnh trong file `.ioc`, không chỉnh tay.
4. **Checklist sau mỗi lần Generate**: (a) diff `Core/` xem CubeMX đổi gì; (b) block USER CODE
   còn nguyên không; (c) peripheral thêm/bớt → cập nhật Wrappers tương ứng + Doc/02;
   (d) mức idle CS (Touch/SD phải SET) và pull-up Touch_IRQ cấu hình trong `.ioc`.

## 12. Bài học từ các bug đã gặp (đừng lặp lại)

- **Gửi data SPI phải tự quản CS/DC.** `Libs_ILI9341_DrawPixel` từng gọi thẳng `Wrappers_Spi_Transmit` ngay sau `WriteCommand` — lúc đó CS đã bị nhả lên 1 nên LCD bỏ qua toàn bộ, mọi hình vẽ bằng pixel ra màn hình đen. Luôn dùng `Libs_ILI9341_WriteData/WriteDataBuffer` (xem tài liệu 02).
- **Buffer tính theo byte, không theo pixel.** `DrawColourBurst` từng đặt `Buffer_Size = Size` (số pixel) rồi ghi 2 byte/pixel → tràn mảng VLA khi số lẻ và lệch cặp byte màu; `Size = 0` còn gây chia cho 0 → HardFault. Khi buffer chứa cặp byte, kích thước phải luôn chẵn và phải guard trường hợp rỗng.
- **Cast float âm sang unsigned là undefined behavior.** Hàm blend màu ban đầu dùng `uint8_t` cho hiệu số kênh màu nên fade sai khi màu giảm dần — dùng kiểu có dấu (`int16_t`) cho phép nội suy.
- **Khởi tạo nhiều biến trên một dòng dễ sót.** `uint32_t a, b = 0;` chỉ khởi tạo `b`; lỗi này từng làm tọa độ X của cảm ứng tính từ giá trị rác. Khai báo mỗi biến một dòng, có giá trị khởi tạo rõ ràng.
- **CS của slave dùng chung bus phải idle ở mức 1.** Touch CS từng được init về 0 → XPT2046 giữ bus SPI1 và phá dữ liệu gửi cho màn hình.
