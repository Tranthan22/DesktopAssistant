//-----------------------------------
//	ILI9341 Video player library
//-----------------------------------
//	Xem huong dan trong Libs_Video.h
//-----------------------------------

#include "main.h"
#include "Libs_Video.h"
#include "Libs_System.h"
#include "Libs_ILI9341_Driver.h"
#include "Wrappers_Spi.h"
#include "Wrappers_Gpio.h"

#include <string.h>

// /************************************************************************************************************
//  * EXTERN VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * PRIVATE MACROS AND DEFINES
//  ************************************************************************************************************/
//	Moi lan Wrappers_Spi_Transmit gui toi da 65535 byte (Size la uint16_t).
//	32KB la an toan va it lan goi ham nhat.
#define VIDEO_SPI_CHUNK                         32768u

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
/* Blit nhanh: khac voi Libs_ILI9341_DrawImage, khong copy tung byte qua buffer trung gian
   ma truyen thang tu vung nho nguon (flash doc truc tiep duoc qua SPI). */
void Libs_Video_DrawFrame(uint16_t p_X_u16, uint16_t p_Y_u16, uint16_t p_Width_u16, uint16_t p_Height_u16, const uint8_t* p_Data_u8)
{
	uint32_t f_Total_u32 = (uint32_t)p_Width_u16 * p_Height_u16 * 2u;
	uint16_t f_Chunk_u16 = 0;

	/* Nam ngoai man hinh (theo rotation hien tai) thi bo qua */
	if (((uint32_t)p_X_u16 + p_Width_u16) > Libs_ILI9341_GetScreenWidth())
	{
		return;
	}
	if (((uint32_t)p_Y_u16 + p_Height_u16) > Libs_ILI9341_GetScreenHeight())
	{
		return;
	}

	Libs_ILI9341_SetAddress(p_X_u16, p_Y_u16, p_X_u16 + p_Width_u16 - 1, p_Y_u16 + p_Height_u16 - 1);

	Wrappers_Gpio_Write(LogicalChannel_3, 1);	// DC = data
	Wrappers_Gpio_Write(LogicalChannel_1, 0);	// CS on

	while (f_Total_u32 > 0)
	{
		f_Chunk_u16 = (f_Total_u32 > VIDEO_SPI_CHUNK) ? (uint16_t)VIDEO_SPI_CHUNK : (uint16_t)f_Total_u32;
		Wrappers_Spi_Transmit(SPI_Screen, (uint8_t*)p_Data_u8, f_Chunk_u16);
		p_Data_u8   += f_Chunk_u16;
		f_Total_u32 -= f_Chunk_u16;
	}

	Wrappers_Gpio_Write(LogicalChannel_1, 1);	// CS off
}

void Libs_Video_FlashInit(Libs_Video_Flash_st* p_Video_st, const uint8_t* const* p_Frames_ptr, uint16_t p_FrameCount_u16,
                          uint16_t p_Width_u16, uint16_t p_Height_u16, uint16_t p_X_u16, uint16_t p_Y_u16,
                          uint8_t p_Fps_u8, uint8_t p_Loop_u8)
{
	p_Video_st->Frames = p_Frames_ptr;
	p_Video_st->FrameCount = p_FrameCount_u16;
	p_Video_st->Width = p_Width_u16;
	p_Video_st->Height = p_Height_u16;
	p_Video_st->X = p_X_u16;
	p_Video_st->Y = p_Y_u16;
	p_Video_st->Fps = (p_Fps_u8 == 0) ? 1 : p_Fps_u8;
	p_Video_st->Loop = p_Loop_u8;
	p_Video_st->CurrentFrame = 0;
	p_Video_st->NextFrameTick = Libs_System_GetTick();
}

uint8_t Libs_Video_FlashUpdate(Libs_Video_Flash_st* p_Video_st)
{
	if (p_Video_st->CurrentFrame >= p_Video_st->FrameCount)
	{
		if (!p_Video_st->Loop)
		{
			return VIDEO_END;
		}
		p_Video_st->CurrentFrame = 0;
	}

	/* Chua den thoi diem frame ke -> khong lam gi (non-blocking) */
	if ((int32_t)(Libs_System_GetTick() - p_Video_st->NextFrameTick) < 0)
	{
		return VIDEO_WAITING;
	}

	Libs_Video_DrawFrame(p_Video_st->X, p_Video_st->Y, p_Video_st->Width, p_Video_st->Height,
	                     p_Video_st->Frames[p_Video_st->CurrentFrame]);

	p_Video_st->CurrentFrame++;
	/* Cong don theo moc cu (khong lay tick moi) de FPS trung binh chinh xac,
	   frame nao ve cham thi frame sau tu dong bu lai */
	p_Video_st->NextFrameTick += 1000u / p_Video_st->Fps;

	return VIDEO_OK;
}

/* Blocking: tam tat Loop de Update() co the ket thuc, phat het clip 1 lan roi tra Loop lai.
   (Ban goc de nguyen Loop -> khi Loop = 1 vong while khong bao gio thoat) */
void Libs_Video_FlashPlay(Libs_Video_Flash_st* p_Video_st)
{
	uint8_t f_SavedLoop_u8 = p_Video_st->Loop;

	p_Video_st->Loop = 0;
	p_Video_st->CurrentFrame = 0;
	p_Video_st->NextFrameTick = Libs_System_GetTick();

	while (Libs_Video_FlashUpdate(p_Video_st) != VIDEO_END)
	{
	}

	p_Video_st->Loop = f_SavedLoop_u8;
}

//-----------------------------------
//	CHE DO 2: stream tu the SD
//-----------------------------------
#if VIDEO_USE_SDCARD

//	Buffer doc file: cang lon doc SD cang nhanh (it lenh doc hon).
//	9600 byte = 15 dong man hinh 320px. Neu RAM du, tang len 19200 hoac 38400.
#define VIDEO_SD_BUFFER_SIZE                    9600u

static uint8_t l_VideoSdBuffer_u8[VIDEO_SD_BUFFER_SIZE];

uint8_t Libs_Video_SdOpen(Libs_Video_Sd_st* p_Video_st, const char* p_Path_char, uint16_t p_X_u16, uint16_t p_Y_u16)
{
	uint8_t f_Header_u8[16];
	UINT f_BytesRead_u32 = 0;

	if (f_open(&p_Video_st->File, p_Path_char, FA_READ) != FR_OK)
	{
		return VIDEO_ERROR;
	}

	if ((f_read(&p_Video_st->File, f_Header_u8, 16, &f_BytesRead_u32) != FR_OK) || (f_BytesRead_u32 != 16))
	{
		f_close(&p_Video_st->File);
		return VIDEO_ERROR;
	}

	if (memcmp(f_Header_u8, "RVID", 4) != 0)
	{
		f_close(&p_Video_st->File);
		return VIDEO_ERROR;
	}

	p_Video_st->Width      = (uint16_t)(f_Header_u8[4] | (f_Header_u8[5] << 8));
	p_Video_st->Height     = (uint16_t)(f_Header_u8[6] | (f_Header_u8[7] << 8));
	p_Video_st->Fps        = (f_Header_u8[8] == 0) ? 1 : f_Header_u8[8];
	p_Video_st->FrameCount = (uint32_t)f_Header_u8[10] | ((uint32_t)f_Header_u8[11] << 8)
	                       | ((uint32_t)f_Header_u8[12] << 16) | ((uint32_t)f_Header_u8[13] << 24);
	p_Video_st->X = p_X_u16;
	p_Video_st->Y = p_Y_u16;
	p_Video_st->CurrentFrame = 0;
	p_Video_st->NextFrameTick = Libs_System_GetTick();

	return VIDEO_OK;
}

uint8_t Libs_Video_SdUpdate(Libs_Video_Sd_st* p_Video_st)
{
	uint32_t f_Remaining_u32 = 0;
	UINT f_ToRead_u32 = 0;
	UINT f_BytesRead_u32 = 0;

	if (p_Video_st->CurrentFrame >= p_Video_st->FrameCount)
	{
		return VIDEO_END;
	}

	if ((int32_t)(Libs_System_GetTick() - p_Video_st->NextFrameTick) < 0)
	{
		return VIDEO_WAITING;
	}

	f_Remaining_u32 = (uint32_t)p_Video_st->Width * p_Video_st->Height * 2u;

	/* Mo cua so dia chi 1 lan cho ca frame, roi doc SD -> day SPI theo tung khoi */
	Libs_ILI9341_SetAddress(p_Video_st->X, p_Video_st->Y,
	                        p_Video_st->X + p_Video_st->Width - 1, p_Video_st->Y + p_Video_st->Height - 1);
	Wrappers_Gpio_Write(LogicalChannel_3, 1);	// DC = data
	Wrappers_Gpio_Write(LogicalChannel_1, 0);	// CS on

	while (f_Remaining_u32 > 0)
	{
		f_ToRead_u32 = (f_Remaining_u32 > VIDEO_SD_BUFFER_SIZE) ? VIDEO_SD_BUFFER_SIZE : f_Remaining_u32;

		if ((f_read(&p_Video_st->File, l_VideoSdBuffer_u8, f_ToRead_u32, &f_BytesRead_u32) != FR_OK) || (f_BytesRead_u32 != f_ToRead_u32))
		{
			Wrappers_Gpio_Write(LogicalChannel_1, 1);
			return VIDEO_ERROR;
		}

		Wrappers_Spi_Transmit(SPI_Screen, l_VideoSdBuffer_u8, (uint16_t)f_BytesRead_u32);
		f_Remaining_u32 -= f_BytesRead_u32;
	}

	Wrappers_Gpio_Write(LogicalChannel_1, 1);	// CS off

	p_Video_st->CurrentFrame++;
	p_Video_st->NextFrameTick += 1000u / p_Video_st->Fps;

	return VIDEO_OK;
}

void Libs_Video_SdRewind(Libs_Video_Sd_st* p_Video_st)
{
	f_lseek(&p_Video_st->File, 16);	// bo qua header
	p_Video_st->CurrentFrame = 0;
	p_Video_st->NextFrameTick = Libs_System_GetTick();
}

void Libs_Video_SdClose(Libs_Video_Sd_st* p_Video_st)
{
	f_close(&p_Video_st->File);
}

#endif /* VIDEO_USE_SDCARD */
