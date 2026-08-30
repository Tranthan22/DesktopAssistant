//-----------------------------------
//	ILI9341 Animation library
//-----------------------------------
//
//	Thu vien animation cho ILI9341, xay dung tren Libs_ILI9341_Driver + Libs_ILI9341_GFX.
//
//	Nguyen tac de animation chay muot tren SPI (khong co framebuffer):
//	1. KHONG BAO GIO xoa ca man hinh moi frame (FillScreen rat cham)
//	   -> chi xoa dung vung ma vat the vua roi khoi (dirty rectangle)
//	2. Animation theo THOI GIAN THUC (Libs_System_GetTick) thay vi theo so frame
//	   -> toc do khong doi du MCU nhanh hay cham
//	3. Dung ham easing de chuyen dong co gia toc tu nhien thay vi tuyen tinh
//
//	Cach dung nhanh (trong main.c):
//
//		Libs_ILI9341_Init();
//		Libs_ILI9341_SetRotation(ILI9341_SCREEN_HORIZONTAL_1);
//		Libs_Animation_Demo();			// chay demo tat ca hieu ung
//
//	Hoac dung tung hieu ung rieng, vi du bong nay:
//
//		Libs_Animation_Ball_st f_Ball_st;
//		Libs_Animation_BallInit(&f_Ball_st, 50, 50, 8, 3.2f, 2.1f, ILI9341_RED, ILI9341_BLACK);
//		while(1) { Libs_Animation_BallUpdate(&f_Ball_st); Libs_System_DelayMs(10); }
//
//-----------------------------------

#ifndef LIBS_ANIMATION_LIBS_ANIMATION_H_
#define LIBS_ANIMATION_LIBS_ANIMATION_H_

// /************************************************************************************************************
//  * INCLUDES
//  ************************************************************************************************************/
#include <stdint.h>

// /************************************************************************************************************
//  * MACROS AND DEFINES
//  ************************************************************************************************************/
#define ANIMATION_STARFIELD_COUNT               40

// /************************************************************************************************************
//  * TYPEDEFS
//  ************************************************************************************************************/
//	Bong nay trong man hinh (co the tao nhieu qua bong cung luc)
typedef struct
{
	float X;				// vi tri hien tai
	float Y;
	float VelocityX;		// van toc (pixel / frame)
	float VelocityY;
	uint16_t Radius;
	uint16_t Colour;
	uint16_t BgColour;
	int16_t OldX;			// vi tri frame truoc, de xoa
	int16_t OldY;
} Libs_Animation_Ball_st;

//	Vong xoay loading (spinner) - 8 cham xoay tron, cham duoi mo dan
typedef struct
{
	uint16_t X;				// tam
	uint16_t Y;
	uint16_t Radius;		// ban kinh quy dao
	uint16_t DotRadius;		// ban kinh moi cham
	uint16_t Colour;
	uint16_t BgColour;
	uint32_t PeriodMs;		// thoi gian xoay het 1 vong
} Libs_Animation_Spinner_st;

//	Thanh tien trinh muot - tu chay den gia tri dich voi easing
typedef struct
{
	uint16_t X;
	uint16_t Y;
	uint16_t Width;
	uint16_t Height;
	uint16_t BarColour;
	uint16_t BgColour;
	uint16_t BorderColour;
	float Current;			// 0.0 -> 1.0 dang hien thi
	float Target;			// 0.0 -> 1.0 muon den
	uint16_t DrawnPixels;	// so pixel da to, de chi ve phan chenh lech
} Libs_Animation_ProgressBar_st;

//	Song sin chay ngang man hinh
typedef struct
{
	uint16_t YCenter;
	uint16_t Amplitude;
	float Wavelength;		// do dai buoc song (pixel)
	float Speed;			// pixel / giay
	uint16_t Colour;
	uint16_t BgColour;
	int16_t OldY[320];		// y cua frame truoc tai moi cot de xoa (320 = be rong man hinh lon nhat)
	uint8_t FirstFrame;
} Libs_Animation_Wave_st;

// /************************************************************************************************************
//  * EXPORTED VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * GLOBAL FUNCTION PROTOTYPES
//  ************************************************************************************************************/
//	Easing - nhan tien trinh t (0.0 -> 1.0), tra ve tien trinh da lam muot
float Libs_Animation_EaseLinear(float p_Progress_float);
float Libs_Animation_EaseInQuad(float p_Progress_float);		// bat dau cham, tang toc dan
float Libs_Animation_EaseOutQuad(float p_Progress_float);		// bat dau nhanh, giam toc dan
float Libs_Animation_EaseInOutCubic(float p_Progress_float);	// cham - nhanh - cham (tu nhien nhat)
float Libs_Animation_EaseOutBounce(float p_Progress_float);		// nay len o cuoi nhu bong roi
float Libs_Animation_EaseOutBack(float p_Progress_float);		// vuot qua dich mot chut roi quay lai

//	Tron 2 mau RGB565 theo ti le t (0.0 = Colour1, 1.0 = Colour2) - dung cho hieu ung fade
uint16_t Libs_Animation_BlendColour(uint16_t p_Colour1_u16, uint16_t p_Colour2_u16, float p_Progress_float);

//	Bong nay
void Libs_Animation_BallInit(Libs_Animation_Ball_st* p_Ball_st, float p_X_float, float p_Y_float, uint16_t p_Radius_u16,
                             float p_VelocityX_float, float p_VelocityY_float, uint16_t p_Colour_u16, uint16_t p_BgColour_u16);
void Libs_Animation_BallUpdate(Libs_Animation_Ball_st* p_Ball_st);

//	Spinner
void Libs_Animation_SpinnerInit(Libs_Animation_Spinner_st* p_Spinner_st, uint16_t p_X_u16, uint16_t p_Y_u16, uint16_t p_Radius_u16,
                                uint16_t p_DotRadius_u16, uint16_t p_Colour_u16, uint16_t p_BgColour_u16, uint32_t p_PeriodMs_u32);
void Libs_Animation_SpinnerUpdate(Libs_Animation_Spinner_st* p_Spinner_st);

//	Thanh tien trinh
void Libs_Animation_ProgressBarInit(Libs_Animation_ProgressBar_st* p_Bar_st, uint16_t p_X_u16, uint16_t p_Y_u16, uint16_t p_Width_u16,
                                    uint16_t p_Height_u16, uint16_t p_BarColour_u16, uint16_t p_BgColour_u16, uint16_t p_BorderColour_u16);
void Libs_Animation_ProgressBarSet(Libs_Animation_ProgressBar_st* p_Bar_st, float p_Target_float);	// p_Target_float: 0.0 -> 1.0
void Libs_Animation_ProgressBarUpdate(Libs_Animation_ProgressBar_st* p_Bar_st);

//	Song sin
void Libs_Animation_WaveInit(Libs_Animation_Wave_st* p_Wave_st, uint16_t p_YCenter_u16, uint16_t p_Amplitude_u16,
                             float p_Wavelength_float, float p_Speed_float, uint16_t p_Colour_u16, uint16_t p_BgColour_u16);
void Libs_Animation_WaveUpdate(Libs_Animation_Wave_st* p_Wave_st);

//	Bau troi sao bay ve phia nguoi xem (starfield 3D)
void Libs_Animation_StarfieldInit(uint16_t p_BgColour_u16);
void Libs_Animation_StarfieldUpdate(void);

//	Chu truot vao tu ben phai voi easing (blocking)
void Libs_Animation_TextSlideIn(const char* p_Text_char, uint16_t p_FinalX_u16, uint16_t p_Y_u16, uint16_t p_Colour_u16,
                                uint16_t p_Size_u16, uint16_t p_BgColour_u16, uint32_t p_DurationMs_u32);

//	So dem chay muot tu From den To, vd: 0 -> 1250 (blocking)
void Libs_Animation_Counter(int32_t p_From_i32, int32_t p_To_i32, uint16_t p_X_u16, uint16_t p_Y_u16, uint16_t p_Colour_u16,
                            uint16_t p_Size_u16, uint16_t p_BgColour_u16, uint32_t p_DurationMs_u32);

//	Man hinh fade tu mau nay sang mau khac (blocking)
void Libs_Animation_ScreenFade(uint16_t p_FromColour_u16, uint16_t p_ToColour_u16, uint16_t p_Steps_u16, uint32_t p_StepDelayMs_u32);

//	Demo lan luot tat ca hieu ung (blocking, ~25s)
void Libs_Animation_Demo(void);

#endif /* LIBS_ANIMATION_LIBS_ANIMATION_H_ */
