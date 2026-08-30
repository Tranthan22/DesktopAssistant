//-----------------------------------
//	ILI9341 Video player library
//-----------------------------------
//
//	Phat video tren ILI9341 theo 2 che do:
//
//	CHE DO 1: FLASH - chuoi frame nam trong flash duoi dang mang C
//	------------------------------------------------------------
//	Phu hop clip ngan / icon dong. Tinh toan bo nho:
//		1 frame = Width * Height * 2 byte
//		320x240 = 150 KB/frame  -> flash 512KB chi chua ~3 frame!
//		 96x72  = 13.5 KB/frame -> ~30 frame (~2s @ 15fps)
//		 64x48  =  6 KB/frame   -> ~70 frame (~4.5s @ 15fps)
//	Dung script Tools/video2c.py de chuyen GIF/MP4 thanh file .h
//
//	Vi du:
//		#include "my_clip.h"	// sinh ra boi video2c.py
//		Libs_Video_Flash_st f_Video_st;
//		Libs_Video_FlashInit(&f_Video_st, my_clip_frames, MY_CLIP_FRAME_COUNT,
//		                     MY_CLIP_WIDTH, MY_CLIP_HEIGHT, 112, 84, MY_CLIP_FPS, 1);
//		while(1) { Libs_Video_FlashUpdate(&f_Video_st); /* lam viec khac o day */ }
//
//	CHE DO 2: SD CARD - stream file .rvd tu the nho qua FatFS
//	------------------------------------------------------------
//	Phat video dai, full man hinh. Can:
//		- Implement xong user_diskio.c (hien tai van la stub -> chua dung duoc)
//		- Doi VIDEO_USE_SDCARD ben duoi thanh 1
//		- Tao file .rvd: python3 video2c.py input.mp4 -f rvd -W 320 -H 240 -r 20
//
//	Vi du:
//		Libs_Video_Sd_st f_Video_st;
//		if(Libs_Video_SdOpen(&f_Video_st, "clip.rvd", 0, 0) == VIDEO_OK)
//		{
//			while(Libs_Video_SdUpdate(&f_Video_st) != VIDEO_END);
//			Libs_Video_SdClose(&f_Video_st);
//		}
//
//	DINH DANG DU LIEU
//	------------------------------------------------------------
//	Pixel RGB565 big-endian (byte cao truoc) - dung nhu ILI9341 nhan.
//	ffmpeg xuat dung dinh dang nay voi: -pix_fmt rgb565be
//
//-----------------------------------

#ifndef LIBS_VIDEO_LIBS_VIDEO_H_
#define LIBS_VIDEO_LIBS_VIDEO_H_

// /************************************************************************************************************
//  * INCLUDES
//  ************************************************************************************************************/
#include <stdint.h>

// /************************************************************************************************************
//  * MACROS AND DEFINES
//  ************************************************************************************************************/
//	Doi thanh 1 khi user_diskio.c (SD card) da duoc implement de phat video tu the SD
#ifndef VIDEO_USE_SDCARD
#define VIDEO_USE_SDCARD                        0
#endif

//	Ma tra ve
#define VIDEO_OK                                0
#define VIDEO_END                               1	// da phat het (khong loop)
#define VIDEO_ERROR                             2	// loi doc file / file sai dinh dang / tham so sai
#define VIDEO_WAITING                           3	// chua den gio ve frame ke (non-blocking)

// /************************************************************************************************************
//  * TYPEDEFS
//  ************************************************************************************************************/
//	CHE DO 1: phat tu flash
typedef struct
{
	const uint8_t* const* Frames;	// mang con tro toi tung frame
	uint16_t FrameCount;
	uint16_t Width;
	uint16_t Height;
	uint16_t X;						// vi tri tren man hinh
	uint16_t Y;
	uint8_t  Fps;
	uint8_t  Loop;					// 1 = phat lai tu dau khi het

	// trang thai chay
	uint16_t CurrentFrame;
	uint32_t NextFrameTick;
} Libs_Video_Flash_st;

// /************************************************************************************************************
//  * EXPORTED VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * GLOBAL FUNCTION PROTOTYPES
//  ************************************************************************************************************/
//	Blit nhanh: gui thang 1 khung anh RGB565 tu bo nho len man hinh.
//	Khong copy qua buffer trung gian nhu Libs_ILI9341_DrawImage -> nhanh hon nhieu.
//	p_Data_u8: Width*Height*2 byte, big-endian. Dung duoc rieng cho game/GUI.
void Libs_Video_DrawFrame(uint16_t p_X_u16, uint16_t p_Y_u16, uint16_t p_Width_u16, uint16_t p_Height_u16, const uint8_t* p_Data_u8);

void Libs_Video_FlashInit(Libs_Video_Flash_st* p_Video_st, const uint8_t* const* p_Frames_ptr, uint16_t p_FrameCount_u16,
                          uint16_t p_Width_u16, uint16_t p_Height_u16, uint16_t p_X_u16, uint16_t p_Y_u16,
                          uint8_t p_Fps_u8, uint8_t p_Loop_u8);

//	Non-blocking: goi lien tuc trong vong lap chinh, tu can thoi gian theo FPS.
//	Tra ve VIDEO_OK (vua ve frame), VIDEO_WAITING (chua den gio), VIDEO_END (het).
uint8_t Libs_Video_FlashUpdate(Libs_Video_Flash_st* p_Video_st);

//	Blocking: phat tron ven het clip 1 lan roi moi thoat (Loop duoc bo qua)
void Libs_Video_FlashPlay(Libs_Video_Flash_st* p_Video_st);

//	CHE DO 2: stream tu the SD (file .rvd)
#if VIDEO_USE_SDCARD

#include "fatfs.h"

//	Header 16 byte o dau file .rvd (script video2c.py tu tao):
//	[0..3]  magic "RVID"
//	[4..5]  width  (little-endian)
//	[6..7]  height (little-endian)
//	[8]     fps
//	[9]     reserved
//	[10..13] frame_count (little-endian)
//	[14..15] reserved
//	Sau do: frame_count frame lien tiep, moi frame width*height*2 byte RGB565BE
typedef struct
{
	FIL      File;
	uint16_t Width;
	uint16_t Height;
	uint16_t X;
	uint16_t Y;
	uint8_t  Fps;
	uint32_t FrameCount;
	uint32_t CurrentFrame;
	uint32_t NextFrameTick;
} Libs_Video_Sd_st;

uint8_t Libs_Video_SdOpen(Libs_Video_Sd_st* p_Video_st, const char* p_Path_char, uint16_t p_X_u16, uint16_t p_Y_u16);
uint8_t Libs_Video_SdUpdate(Libs_Video_Sd_st* p_Video_st);	// doc + ve 1 frame, tu can FPS
void Libs_Video_SdRewind(Libs_Video_Sd_st* p_Video_st);		// quay ve frame dau (de loop)
void Libs_Video_SdClose(Libs_Video_Sd_st* p_Video_st);

#endif /* VIDEO_USE_SDCARD */

#endif /* LIBS_VIDEO_LIBS_VIDEO_H_ */
