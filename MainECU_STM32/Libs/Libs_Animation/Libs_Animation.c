//-----------------------------------
//	ILI9341 Animation library
//-----------------------------------
//	Xem huong dan trong Libs_Animation.h
//-----------------------------------

#include "main.h"
#include "Libs_Animation.h"
#include "Libs_System.h"
#include "Libs_ILI9341_Driver.h"
#include "Libs_ILI9341_GFX.h"
#include "Libs_ILI9341_Font.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

// /************************************************************************************************************
//  * EXTERN VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * PRIVATE MACROS AND DEFINES
//  ************************************************************************************************************/
#define ANIMATION_PI                            3.14159265f

// /************************************************************************************************************
//  * PRIVATE TYPEDEFS
//  ************************************************************************************************************/
typedef struct
{
	float X;				// toa do 3D, Z cang nho cang gan nguoi xem
	float Y;
	float Z;
	int16_t OldScreenX;		// vi tri da ve tren man hinh
	int16_t OldScreenY;
} Libs_Animation_Star_st;

// /************************************************************************************************************
//  * STATIC VARIABLES
//  ************************************************************************************************************/
static Libs_Animation_Star_st l_Stars_st[ANIMATION_STARFIELD_COUNT];
static uint16_t l_StarfieldBg_u16 = ILI9341_BLACK;
static uint32_t l_RandState_u32 = 0x12345678;

/* Wave cho demo: mang OldY[320] kha lon (~650 byte), de static tranh tran stack */
static Libs_Animation_Wave_st l_DemoWave_st;

// /************************************************************************************************************
//  * STATIC FUNCTION PROTOTYPES
//  ************************************************************************************************************/
static uint32_t Libs_Animation_Rand(void);
static void Libs_Animation_StarReset(Libs_Animation_Star_st* p_Star_st);

// /************************************************************************************************************
//  * STATIC FUNCTIONS
//  ************************************************************************************************************/
/* Bo sinh so ngau nhien don gian (xorshift) - khong can bat RNG cua chip */
static uint32_t Libs_Animation_Rand(void)
{
	l_RandState_u32 ^= l_RandState_u32 << 13;
	l_RandState_u32 ^= l_RandState_u32 >> 17;
	l_RandState_u32 ^= l_RandState_u32 << 5;
	return l_RandState_u32;
}

static void Libs_Animation_StarReset(Libs_Animation_Star_st* p_Star_st)
{
	p_Star_st->X = (float)((int32_t)(Libs_Animation_Rand() % 2000) - 1000);
	p_Star_st->Y = (float)((int32_t)(Libs_Animation_Rand() % 2000) - 1000);
	p_Star_st->Z = (float)(Libs_Animation_Rand() % 900) + 100.0f;
	p_Star_st->OldScreenX = -1;
	p_Star_st->OldScreenY = -1;
}

// ************************************************************************************************************
//  * GLOBAL FUNCTIONS
//  ************************************************************************************************************/
//-----------------------------------
// Easing
//-----------------------------------
float Libs_Animation_EaseLinear(float p_Progress_float)
{
	return p_Progress_float;
}

float Libs_Animation_EaseInQuad(float p_Progress_float)
{
	return p_Progress_float * p_Progress_float;
}

float Libs_Animation_EaseOutQuad(float p_Progress_float)
{
	return 1.0f - (1.0f - p_Progress_float) * (1.0f - p_Progress_float);
}

float Libs_Animation_EaseInOutCubic(float p_Progress_float)
{
	float f_Factor_float = 0.0f;

	if (p_Progress_float < 0.5f)
	{
		return 4.0f * p_Progress_float * p_Progress_float * p_Progress_float;
	}
	f_Factor_float = -2.0f * p_Progress_float + 2.0f;
	return 1.0f - (f_Factor_float * f_Factor_float * f_Factor_float) / 2.0f;
}

float Libs_Animation_EaseOutBounce(float p_Progress_float)
{
	const float f_N1_float = 7.5625f;
	const float f_D1_float = 2.75f;
	float f_Time_float = p_Progress_float;

	if (f_Time_float < 1.0f / f_D1_float)
	{
		return f_N1_float * f_Time_float * f_Time_float;
	}
	else if (f_Time_float < 2.0f / f_D1_float)
	{
		f_Time_float -= 1.5f / f_D1_float;
		return f_N1_float * f_Time_float * f_Time_float + 0.75f;
	}
	else if (f_Time_float < 2.5f / f_D1_float)
	{
		f_Time_float -= 2.25f / f_D1_float;
		return f_N1_float * f_Time_float * f_Time_float + 0.9375f;
	}
	else
	{
		f_Time_float -= 2.625f / f_D1_float;
		return f_N1_float * f_Time_float * f_Time_float + 0.984375f;
	}
}

float Libs_Animation_EaseOutBack(float p_Progress_float)
{
	const float f_C1_float = 1.70158f;
	const float f_C3_float = f_C1_float + 1.0f;
	float f_Factor_float = p_Progress_float - 1.0f;

	return 1.0f + f_C3_float * f_Factor_float * f_Factor_float * f_Factor_float + f_C1_float * f_Factor_float * f_Factor_float;
}

/* Tron 2 mau RGB565: tach R/G/B, noi suy tuyen tinh tung kenh (so co dau) roi ghep lai */
uint16_t Libs_Animation_BlendColour(uint16_t p_Colour1_u16, uint16_t p_Colour2_u16, float p_Progress_float)
{
	int16_t f_Red1_i16, f_Green1_i16, f_Blue1_i16;
	int16_t f_Red2_i16, f_Green2_i16, f_Blue2_i16;
	int16_t f_Red_i16, f_Green_i16, f_Blue_i16;

	if (p_Progress_float <= 0.0f)
	{
		return p_Colour1_u16;
	}
	if (p_Progress_float >= 1.0f)
	{
		return p_Colour2_u16;
	}

	f_Red1_i16   = (p_Colour1_u16 >> 11) & 0x1F;
	f_Green1_i16 = (p_Colour1_u16 >> 5) & 0x3F;
	f_Blue1_i16  = p_Colour1_u16 & 0x1F;
	f_Red2_i16   = (p_Colour2_u16 >> 11) & 0x1F;
	f_Green2_i16 = (p_Colour2_u16 >> 5) & 0x3F;
	f_Blue2_i16  = p_Colour2_u16 & 0x1F;

	/* Noi suy bang int16: kenh mau giam dan cho hieu so am hop le
	   (ban goc cast float am sang uint8_t la undefined behavior) */
	f_Red_i16   = f_Red1_i16   + (int16_t)((f_Red2_i16   - f_Red1_i16)   * p_Progress_float);
	f_Green_i16 = f_Green1_i16 + (int16_t)((f_Green2_i16 - f_Green1_i16) * p_Progress_float);
	f_Blue_i16  = f_Blue1_i16  + (int16_t)((f_Blue2_i16  - f_Blue1_i16)  * p_Progress_float);

	return (uint16_t)((f_Red_i16 << 11) | (f_Green_i16 << 5) | f_Blue_i16);
}

//-----------------------------------
// Bong nay
//-----------------------------------
void Libs_Animation_BallInit(Libs_Animation_Ball_st* p_Ball_st, float p_X_float, float p_Y_float, uint16_t p_Radius_u16,
                             float p_VelocityX_float, float p_VelocityY_float, uint16_t p_Colour_u16, uint16_t p_BgColour_u16)
{
	p_Ball_st->X = p_X_float;
	p_Ball_st->Y = p_Y_float;
	p_Ball_st->VelocityX = p_VelocityX_float;
	p_Ball_st->VelocityY = p_VelocityY_float;
	p_Ball_st->Radius = p_Radius_u16;
	p_Ball_st->Colour = p_Colour_u16;
	p_Ball_st->BgColour = p_BgColour_u16;
	p_Ball_st->OldX = -1;
	p_Ball_st->OldY = -1;
}

void Libs_Animation_BallUpdate(Libs_Animation_Ball_st* p_Ball_st)
{
	uint16_t f_ScreenWidth_u16 = Libs_ILI9341_GetScreenWidth();
	uint16_t f_ScreenHeight_u16 = Libs_ILI9341_GetScreenHeight();
	int16_t f_NewX_i16 = 0;
	int16_t f_NewY_i16 = 0;

	p_Ball_st->X += p_Ball_st->VelocityX;
	p_Ball_st->Y += p_Ball_st->VelocityY;

	/* Dap vao tuong thi doi huong */
	if (p_Ball_st->X <= p_Ball_st->Radius)
	{
		p_Ball_st->X = p_Ball_st->Radius;
		p_Ball_st->VelocityX = -p_Ball_st->VelocityX;
	}
	if (p_Ball_st->X >= f_ScreenWidth_u16 - p_Ball_st->Radius)
	{
		p_Ball_st->X = f_ScreenWidth_u16 - p_Ball_st->Radius;
		p_Ball_st->VelocityX = -p_Ball_st->VelocityX;
	}
	if (p_Ball_st->Y <= p_Ball_st->Radius)
	{
		p_Ball_st->Y = p_Ball_st->Radius;
		p_Ball_st->VelocityY = -p_Ball_st->VelocityY;
	}
	if (p_Ball_st->Y >= f_ScreenHeight_u16 - p_Ball_st->Radius)
	{
		p_Ball_st->Y = f_ScreenHeight_u16 - p_Ball_st->Radius;
		p_Ball_st->VelocityY = -p_Ball_st->VelocityY;
	}

	f_NewX_i16 = (int16_t)p_Ball_st->X;
	f_NewY_i16 = (int16_t)p_Ball_st->Y;

	/* Chi xoa va ve lai khi vi tri thuc su thay doi */
	if ((f_NewX_i16 != p_Ball_st->OldX) || (f_NewY_i16 != p_Ball_st->OldY))
	{
		if (p_Ball_st->OldX >= 0)
		{
			Libs_ILI9341_DrawFilledCircle(p_Ball_st->OldX, p_Ball_st->OldY, p_Ball_st->Radius, p_Ball_st->BgColour);
		}

		Libs_ILI9341_DrawFilledCircle(f_NewX_i16, f_NewY_i16, p_Ball_st->Radius, p_Ball_st->Colour);
		p_Ball_st->OldX = f_NewX_i16;
		p_Ball_st->OldY = f_NewY_i16;
	}
}

//-----------------------------------
// Spinner
//-----------------------------------
void Libs_Animation_SpinnerInit(Libs_Animation_Spinner_st* p_Spinner_st, uint16_t p_X_u16, uint16_t p_Y_u16, uint16_t p_Radius_u16,
                                uint16_t p_DotRadius_u16, uint16_t p_Colour_u16, uint16_t p_BgColour_u16, uint32_t p_PeriodMs_u32)
{
	p_Spinner_st->X = p_X_u16;
	p_Spinner_st->Y = p_Y_u16;
	p_Spinner_st->Radius = p_Radius_u16;
	p_Spinner_st->DotRadius = p_DotRadius_u16;
	p_Spinner_st->Colour = p_Colour_u16;
	p_Spinner_st->BgColour = p_BgColour_u16;
	p_Spinner_st->PeriodMs = (p_PeriodMs_u32 == 0) ? 1000 : p_PeriodMs_u32;	// tranh chia cho 0
}

void Libs_Animation_SpinnerUpdate(Libs_Animation_Spinner_st* p_Spinner_st)
{
	const uint8_t f_DotCount_u8 = 8;
	float f_Phase_float = 0.0f;
	uint8_t f_Head_u8 = 0;
	uint8_t f_Index_u8 = 0;
	uint8_t f_Distance_u8 = 0;
	float f_Angle_float = 0.0f;
	float f_Fade_float = 0.0f;
	uint16_t f_DotX_u16 = 0;
	uint16_t f_DotY_u16 = 0;
	uint16_t f_DotColour_u16 = 0;

	/* Goc quay tinh theo thoi gian thuc de toc do luon on dinh */
	f_Phase_float = (float)(Libs_System_GetTick() % p_Spinner_st->PeriodMs) / (float)p_Spinner_st->PeriodMs;
	f_Head_u8 = (uint8_t)(f_Phase_float * f_DotCount_u8) % f_DotCount_u8;

	for (f_Index_u8 = 0; f_Index_u8 < f_DotCount_u8; f_Index_u8++)
	{
		f_Angle_float = 2.0f * ANIMATION_PI * f_Index_u8 / f_DotCount_u8;
		f_DotX_u16 = p_Spinner_st->X + (int16_t)(p_Spinner_st->Radius * cosf(f_Angle_float));
		f_DotY_u16 = p_Spinner_st->Y + (int16_t)(p_Spinner_st->Radius * sinf(f_Angle_float));

		/* Cham dau sang nhat, cac cham phia sau mo dan ve mau nen */
		f_Distance_u8 = (uint8_t)((f_Head_u8 - f_Index_u8 + f_DotCount_u8) % f_DotCount_u8);
		f_Fade_float = (float)f_Distance_u8 / (float)f_DotCount_u8;
		f_DotColour_u16 = Libs_Animation_BlendColour(p_Spinner_st->Colour, p_Spinner_st->BgColour, f_Fade_float);

		Libs_ILI9341_DrawFilledCircle(f_DotX_u16, f_DotY_u16, p_Spinner_st->DotRadius, f_DotColour_u16);
	}
}

//-----------------------------------
// Thanh tien trinh
//-----------------------------------
void Libs_Animation_ProgressBarInit(Libs_Animation_ProgressBar_st* p_Bar_st, uint16_t p_X_u16, uint16_t p_Y_u16, uint16_t p_Width_u16,
                                    uint16_t p_Height_u16, uint16_t p_BarColour_u16, uint16_t p_BgColour_u16, uint16_t p_BorderColour_u16)
{
	p_Bar_st->X = p_X_u16;
	p_Bar_st->Y = p_Y_u16;
	p_Bar_st->Width = p_Width_u16;
	p_Bar_st->Height = p_Height_u16;
	p_Bar_st->BarColour = p_BarColour_u16;
	p_Bar_st->BgColour = p_BgColour_u16;
	p_Bar_st->BorderColour = p_BorderColour_u16;
	p_Bar_st->Current = 0.0f;
	p_Bar_st->Target = 0.0f;
	p_Bar_st->DrawnPixels = 0;

	Libs_ILI9341_DrawHollowRectangleCoord(p_X_u16, p_Y_u16, p_X_u16 + p_Width_u16, p_Y_u16 + p_Height_u16, p_BorderColour_u16);
	Libs_ILI9341_DrawFilledRectangleCoord(p_X_u16 + 1, p_Y_u16 + 1, p_X_u16 + p_Width_u16 - 1, p_Y_u16 + p_Height_u16 - 1, p_BgColour_u16);
}

void Libs_Animation_ProgressBarSet(Libs_Animation_ProgressBar_st* p_Bar_st, float p_Target_float)
{
	if (p_Target_float < 0.0f)
	{
		p_Target_float = 0.0f;
	}
	if (p_Target_float > 1.0f)
	{
		p_Target_float = 1.0f;
	}
	p_Bar_st->Target = p_Target_float;
}

void Libs_Animation_ProgressBarUpdate(Libs_Animation_ProgressBar_st* p_Bar_st)
{
	uint16_t f_NewPixels_u16 = 0;

	/* Current duoi dan ve Target - moi frame di 15% khoang cach con lai
	   tao hieu ung giam toc muot ma khong can luu thoi gian */
	p_Bar_st->Current += (p_Bar_st->Target - p_Bar_st->Current) * 0.15f;

	f_NewPixels_u16 = (uint16_t)(p_Bar_st->Current * (p_Bar_st->Width - 2));

	if (f_NewPixels_u16 > p_Bar_st->DrawnPixels)
	{
		/* Chi to them phan chenh lech */
		Libs_ILI9341_DrawFilledRectangleCoord(
			p_Bar_st->X + 1 + p_Bar_st->DrawnPixels, p_Bar_st->Y + 1,
			p_Bar_st->X + 1 + f_NewPixels_u16,       p_Bar_st->Y + p_Bar_st->Height - 1,
			p_Bar_st->BarColour);
		p_Bar_st->DrawnPixels = f_NewPixels_u16;
	}
	else if (f_NewPixels_u16 < p_Bar_st->DrawnPixels)
	{
		/* Target giam thi xoa phan thua */
		Libs_ILI9341_DrawFilledRectangleCoord(
			p_Bar_st->X + 1 + f_NewPixels_u16,       p_Bar_st->Y + 1,
			p_Bar_st->X + 1 + p_Bar_st->DrawnPixels, p_Bar_st->Y + p_Bar_st->Height - 1,
			p_Bar_st->BgColour);
		p_Bar_st->DrawnPixels = f_NewPixels_u16;
	}
	else
	{
	}
}

//-----------------------------------
// Song sin
//-----------------------------------
void Libs_Animation_WaveInit(Libs_Animation_Wave_st* p_Wave_st, uint16_t p_YCenter_u16, uint16_t p_Amplitude_u16,
                             float p_Wavelength_float, float p_Speed_float, uint16_t p_Colour_u16, uint16_t p_BgColour_u16)
{
	p_Wave_st->YCenter = p_YCenter_u16;
	p_Wave_st->Amplitude = p_Amplitude_u16;
	p_Wave_st->Wavelength = p_Wavelength_float;
	p_Wave_st->Speed = p_Speed_float;
	p_Wave_st->Colour = p_Colour_u16;
	p_Wave_st->BgColour = p_BgColour_u16;
	p_Wave_st->FirstFrame = 1;
}

void Libs_Animation_WaveUpdate(Libs_Animation_Wave_st* p_Wave_st)
{
	uint16_t f_ScreenWidth_u16 = Libs_ILI9341_GetScreenWidth();
	uint16_t f_IndexX_u16 = 0;
	int16_t f_NewY_i16 = 0;
	float f_Phase_float = 0.0f;

	/* Pha dich theo thoi gian -> song troi ngang */
	f_Phase_float = (float)Libs_System_GetTick() / 1000.0f * p_Wave_st->Speed;

	/* Mang OldY chi co 320 phan tu */
	if (f_ScreenWidth_u16 > 320)
	{
		f_ScreenWidth_u16 = 320;
	}

	for (f_IndexX_u16 = 0; f_IndexX_u16 < f_ScreenWidth_u16; f_IndexX_u16++)
	{
		f_NewY_i16 = p_Wave_st->YCenter + (int16_t)(p_Wave_st->Amplitude * sinf(2.0f * ANIMATION_PI * (f_IndexX_u16 + f_Phase_float) / p_Wave_st->Wavelength));

		if ((!p_Wave_st->FirstFrame) && (p_Wave_st->OldY[f_IndexX_u16] != f_NewY_i16))
		{
			Libs_ILI9341_DrawPixel(f_IndexX_u16, p_Wave_st->OldY[f_IndexX_u16], p_Wave_st->BgColour);
		}

		Libs_ILI9341_DrawPixel(f_IndexX_u16, f_NewY_i16, p_Wave_st->Colour);
		p_Wave_st->OldY[f_IndexX_u16] = f_NewY_i16;
	}
	p_Wave_st->FirstFrame = 0;
}

//-----------------------------------
// Starfield
//-----------------------------------
void Libs_Animation_StarfieldInit(uint16_t p_BgColour_u16)
{
	uint8_t f_Index_u8 = 0;

	l_StarfieldBg_u16 = p_BgColour_u16;
	l_RandState_u32 = Libs_System_GetTick() | 1;	// seed khac nhau moi lan chay

	for (f_Index_u8 = 0; f_Index_u8 < ANIMATION_STARFIELD_COUNT; f_Index_u8++)
	{
		Libs_Animation_StarReset(&l_Stars_st[f_Index_u8]);
	}
}

void Libs_Animation_StarfieldUpdate(void)
{
	const uint16_t f_CenterX_u16 = Libs_ILI9341_GetScreenWidth() / 2;
	const uint16_t f_CenterY_u16 = Libs_ILI9341_GetScreenHeight() / 2;
	uint8_t f_Index_u8 = 0;
	int16_t f_ScreenX_i16 = 0;
	int16_t f_ScreenY_i16 = 0;
	uint16_t f_Size_u16 = 0;
	uint16_t f_Colour_u16 = 0;
	Libs_Animation_Star_st* f_Star_st = 0;

	for (f_Index_u8 = 0; f_Index_u8 < ANIMATION_STARFIELD_COUNT; f_Index_u8++)
	{
		f_Star_st = &l_Stars_st[f_Index_u8];

		f_Star_st->Z -= 8.0f;	// sao bay ve phia nguoi xem
		if (f_Star_st->Z < 1.0f)
		{
			if (f_Star_st->OldScreenX >= 0)
			{
				Libs_ILI9341_DrawFilledCircle(f_Star_st->OldScreenX, f_Star_st->OldScreenY, 2, l_StarfieldBg_u16);
			}
			Libs_Animation_StarReset(f_Star_st);
			continue;
		}

		/* Chieu phoi canh: cang gan (Z nho) cang xa tam va cang to */
		f_ScreenX_i16 = f_CenterX_u16 + (int16_t)(f_Star_st->X * 100.0f / f_Star_st->Z);
		f_ScreenY_i16 = f_CenterY_u16 + (int16_t)(f_Star_st->Y * 100.0f / f_Star_st->Z);

		if ((f_ScreenX_i16 < 3) || (f_ScreenX_i16 >= (int16_t)Libs_ILI9341_GetScreenWidth() - 3)
		 || (f_ScreenY_i16 < 3) || (f_ScreenY_i16 >= (int16_t)Libs_ILI9341_GetScreenHeight() - 3))
		{
			if (f_Star_st->OldScreenX >= 0)
			{
				Libs_ILI9341_DrawFilledCircle(f_Star_st->OldScreenX, f_Star_st->OldScreenY, 2, l_StarfieldBg_u16);
			}
			Libs_Animation_StarReset(f_Star_st);
			continue;
		}

		if ((f_ScreenX_i16 != f_Star_st->OldScreenX) || (f_ScreenY_i16 != f_Star_st->OldScreenY))
		{
			if (f_Star_st->OldScreenX >= 0)
			{
				Libs_ILI9341_DrawFilledCircle(f_Star_st->OldScreenX, f_Star_st->OldScreenY, 2, l_StarfieldBg_u16);
			}

			/* Sao gan sang va to hon */
			f_Size_u16 = (f_Star_st->Z < 300.0f) ? 2 : 1;
			f_Colour_u16 = Libs_Animation_BlendColour(ILI9341_WHITE, l_StarfieldBg_u16, f_Star_st->Z / 1000.0f);
			Libs_ILI9341_DrawFilledCircle(f_ScreenX_i16, f_ScreenY_i16, f_Size_u16, f_Colour_u16);

			f_Star_st->OldScreenX = f_ScreenX_i16;
			f_Star_st->OldScreenY = f_ScreenY_i16;
		}
	}
}

//-----------------------------------
// Chu truot vao
//-----------------------------------
void Libs_Animation_TextSlideIn(const char* p_Text_char, uint16_t p_FinalX_u16, uint16_t p_Y_u16, uint16_t p_Colour_u16,
                                uint16_t p_Size_u16, uint16_t p_BgColour_u16, uint32_t p_DurationMs_u32)
{
	uint32_t f_StartTick_u32 = Libs_System_GetTick();
	int16_t f_StartX_i16 = (int16_t)Libs_ILI9341_GetScreenWidth();	// bat dau ngoai man hinh ben phai
	int16_t f_OldX_i16 = -1;
	int16_t f_X_i16 = 0;
	uint16_t f_TextWidth_u16 = 0;
	uint32_t f_Elapsed_u32 = 0;
	float f_Progress_float = 0.0f;
	float f_Eased_float = 0.0f;
	const char* f_Char_char = 0;

	/* Do rong chu de xoa vet (dung kich thuoc font that: 6x8) */
	for (f_Char_char = p_Text_char; *f_Char_char; f_Char_char++)
	{
		f_TextWidth_u16 += ILI9341_CHAR_WIDTH * p_Size_u16;
	}

	while (1)
	{
		f_Elapsed_u32 = Libs_System_GetTick() - f_StartTick_u32;
		f_Progress_float = (f_Elapsed_u32 >= p_DurationMs_u32) ? 1.0f : (float)f_Elapsed_u32 / (float)p_DurationMs_u32;
		f_Eased_float = Libs_Animation_EaseOutBack(f_Progress_float);

		f_X_i16 = f_StartX_i16 + (int16_t)((p_FinalX_u16 - f_StartX_i16) * f_Eased_float);
		if (f_X_i16 < 0)
		{
			f_X_i16 = 0;
		}

		if (f_X_i16 != f_OldX_i16)
		{
			/* Xoa vet ben phai khi chu di sang trai (EaseOutBack co the vuot qua dich) */
			if ((f_OldX_i16 >= 0) && (f_OldX_i16 != f_X_i16))
			{
				if (f_X_i16 < f_OldX_i16)
				{
					Libs_ILI9341_DrawFilledRectangleCoord(f_X_i16 + f_TextWidth_u16, p_Y_u16, f_OldX_i16 + f_TextWidth_u16, p_Y_u16 + ILI9341_CHAR_HEIGHT * p_Size_u16, p_BgColour_u16);
				}
				else
				{
					Libs_ILI9341_DrawFilledRectangleCoord(f_OldX_i16, p_Y_u16, f_X_i16, p_Y_u16 + ILI9341_CHAR_HEIGHT * p_Size_u16, p_BgColour_u16);
				}
			}
			Libs_ILI9341_DrawText(p_Text_char, f_X_i16, p_Y_u16, p_Colour_u16, p_Size_u16, p_BgColour_u16);
			f_OldX_i16 = f_X_i16;
		}

		if (f_Progress_float >= 1.0f)
		{
			break;
		}
	}
}

//-----------------------------------
// So dem chay
//-----------------------------------
void Libs_Animation_Counter(int32_t p_From_i32, int32_t p_To_i32, uint16_t p_X_u16, uint16_t p_Y_u16, uint16_t p_Colour_u16,
                            uint16_t p_Size_u16, uint16_t p_BgColour_u16, uint32_t p_DurationMs_u32)
{
	uint32_t f_StartTick_u32 = Libs_System_GetTick();
	int32_t f_OldValue_i32 = p_From_i32 - 1;
	int32_t f_Value_i32 = 0;
	char f_Buffer_char[16];
	char f_OldBuffer_char[16] = "";
	uint32_t f_Elapsed_u32 = 0;
	float f_Progress_float = 0.0f;
	float f_Eased_float = 0.0f;
	size_t f_NewLength = 0;
	size_t f_OldLength = 0;

	while (1)
	{
		f_Elapsed_u32 = Libs_System_GetTick() - f_StartTick_u32;
		f_Progress_float = (f_Elapsed_u32 >= p_DurationMs_u32) ? 1.0f : (float)f_Elapsed_u32 / (float)p_DurationMs_u32;
		f_Eased_float = Libs_Animation_EaseOutQuad(f_Progress_float);	// dem nhanh luc dau, cham dan ve cuoi

		f_Value_i32 = p_From_i32 + (int32_t)((p_To_i32 - p_From_i32) * f_Eased_float);

		if (f_Value_i32 != f_OldValue_i32)
		{
			snprintf(f_Buffer_char, sizeof(f_Buffer_char), "%ld", (long)f_Value_i32);

			/* So moi ngan hon so cu (vd 100 -> 99) thi xoa duoi thua */
			f_NewLength = strlen(f_Buffer_char);
			f_OldLength = strlen(f_OldBuffer_char);
			if (f_OldLength > f_NewLength)
			{
				Libs_ILI9341_DrawFilledRectangleCoord(p_X_u16 + f_NewLength * ILI9341_CHAR_WIDTH * p_Size_u16, p_Y_u16,
				                                      p_X_u16 + f_OldLength * ILI9341_CHAR_WIDTH * p_Size_u16, p_Y_u16 + ILI9341_CHAR_HEIGHT * p_Size_u16,
				                                      p_BgColour_u16);
			}

			Libs_ILI9341_DrawText(f_Buffer_char, p_X_u16, p_Y_u16, p_Colour_u16, p_Size_u16, p_BgColour_u16);
			snprintf(f_OldBuffer_char, sizeof(f_OldBuffer_char), "%s", f_Buffer_char);
			f_OldValue_i32 = f_Value_i32;
		}

		if (f_Progress_float >= 1.0f)
		{
			break;
		}
	}
}

//-----------------------------------
// Fade man hinh
//-----------------------------------
void Libs_Animation_ScreenFade(uint16_t p_FromColour_u16, uint16_t p_ToColour_u16, uint16_t p_Steps_u16, uint32_t p_StepDelayMs_u32)
{
	uint16_t f_Index_u16 = 0;
	float f_Progress_float = 0.0f;

	for (f_Index_u16 = 1; f_Index_u16 <= p_Steps_u16; f_Index_u16++)
	{
		f_Progress_float = Libs_Animation_EaseInOutCubic((float)f_Index_u16 / (float)p_Steps_u16);
		Libs_ILI9341_FillScreen(Libs_Animation_BlendColour(p_FromColour_u16, p_ToColour_u16, f_Progress_float));
		Libs_System_DelayMs(p_StepDelayMs_u32);
	}
}

//-----------------------------------
// Demo
//-----------------------------------
void Libs_Animation_Demo(void)
{
	Libs_Animation_Spinner_st f_Spinner_st;
	Libs_Animation_ProgressBar_st f_Bar_st;
	Libs_Animation_Ball_st f_Balls_st[3];
	uint32_t f_StartTick_u32 = 0;
	uint8_t f_Index_u8 = 0;

	//--- 1. Tieu de truot vao ---
	Libs_ILI9341_FillScreen(ILI9341_BLACK);
	Libs_Animation_TextSlideIn("ANIMATION DEMO", 60, 30, ILI9341_CYAN, 3, ILI9341_BLACK, 800);
	Libs_System_DelayMs(500);

	//--- 2. Spinner + thanh tien trinh + so dem ---
	Libs_ILI9341_FillScreen(ILI9341_BLACK);
	Libs_ILI9341_DrawText("LOADING...", 120, 40, ILI9341_WHITE, 2, ILI9341_BLACK);

	Libs_Animation_SpinnerInit(&f_Spinner_st, 160, 110, 25, 4, ILI9341_CYAN, ILI9341_BLACK, 1000);

	Libs_Animation_ProgressBarInit(&f_Bar_st, 40, 170, 240, 20, ILI9341_GREEN, ILI9341_DARKGREY, ILI9341_WHITE);
	Libs_Animation_ProgressBarSet(&f_Bar_st, 1.0f);

	f_StartTick_u32 = Libs_System_GetTick();
	while (Libs_System_GetTick() - f_StartTick_u32 < 3000)
	{
		Libs_Animation_SpinnerUpdate(&f_Spinner_st);
		Libs_Animation_ProgressBarUpdate(&f_Bar_st);
		Libs_System_DelayMs(30);
	}

	Libs_Animation_Counter(0, 100, 145, 200, ILI9341_YELLOW, 2, ILI9341_BLACK, 1500);
	Libs_System_DelayMs(500);

	//--- 3. Bong nay - 3 qua cung luc ---
	Libs_ILI9341_FillScreen(ILI9341_BLACK);
	Libs_Animation_BallInit(&f_Balls_st[0],  50.0f,  60.0f, 10,  3.5f,  2.3f, ILI9341_RED,    ILI9341_BLACK);
	Libs_Animation_BallInit(&f_Balls_st[1], 200.0f, 100.0f,  7, -2.7f,  3.1f, ILI9341_YELLOW, ILI9341_BLACK);
	Libs_Animation_BallInit(&f_Balls_st[2], 120.0f, 180.0f, 12,  2.1f, -2.9f, ILI9341_CYAN,   ILI9341_BLACK);

	f_StartTick_u32 = Libs_System_GetTick();
	while (Libs_System_GetTick() - f_StartTick_u32 < 5000)
	{
		for (f_Index_u8 = 0; f_Index_u8 < 3; f_Index_u8++)
		{
			Libs_Animation_BallUpdate(&f_Balls_st[f_Index_u8]);
		}
		Libs_System_DelayMs(10);
	}

	//--- 4. Song sin ---
	Libs_ILI9341_FillScreen(ILI9341_BLACK);
	Libs_Animation_WaveInit(&l_DemoWave_st, 120, 50, 80.0f, 60.0f, ILI9341_GREEN, ILI9341_BLACK);

	f_StartTick_u32 = Libs_System_GetTick();
	while (Libs_System_GetTick() - f_StartTick_u32 < 5000)
	{
		Libs_Animation_WaveUpdate(&l_DemoWave_st);
		Libs_System_DelayMs(15);
	}

	//--- 5. Starfield ---
	Libs_ILI9341_FillScreen(ILI9341_BLACK);
	Libs_Animation_StarfieldInit(ILI9341_BLACK);

	f_StartTick_u32 = Libs_System_GetTick();
	while (Libs_System_GetTick() - f_StartTick_u32 < 6000)
	{
		Libs_Animation_StarfieldUpdate();
		Libs_System_DelayMs(20);
	}

	//--- Ket thuc ---
	Libs_Animation_ScreenFade(ILI9341_BLACK, ILI9341_NAVY, 10, 30);
	Libs_Animation_TextSlideIn("DONE!", 120, 110, ILI9341_WHITE, 4, ILI9341_NAVY, 700);
}
