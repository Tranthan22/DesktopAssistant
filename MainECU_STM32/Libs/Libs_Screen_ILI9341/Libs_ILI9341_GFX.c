// /************************************************************************************************************
//  * INCLUDES
//  ************************************************************************************************************/
#include "Typedef.h"
#include "Libs_ILI9341_Font.h"
#include "Libs_ILI9341_GFX.h"
#include "Libs_ILI9341_Driver.h"
#include "Wrappers_Gpio.h"
#include "Wrappers_Spi.h"

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
/*Draw hollow circle at X,Y location with specified radius and colour. X and Y represent circles center */
void Libs_ILI9341_DrawHollowCircle(uint16_t p_LocationX_u16, uint16_t p_LocationY_u16, uint16_t p_Radius_u16, uint16_t p_Colour_u16)
{
	int32_t f_IndexX_i32 = p_Radius_u16-1;
    int32_t f_IndexY_i32 = 0;
    int32_t f_Dx_i32 = 1;
    int32_t f_Dy_i32 = 1;
    int32_t f_Error_i32 = f_Dx_i32 - (p_Radius_u16 << 1);

    while (f_IndexX_i32 >= f_IndexY_i32)
    {
        Libs_ILI9341_DrawPixel(p_LocationX_u16 + f_IndexX_i32, p_LocationY_u16 + f_IndexY_i32, p_Colour_u16);
        Libs_ILI9341_DrawPixel(p_LocationX_u16 + f_IndexY_i32, p_LocationY_u16 + f_IndexX_i32, p_Colour_u16);
        Libs_ILI9341_DrawPixel(p_LocationX_u16 - f_IndexY_i32, p_LocationY_u16 + f_IndexX_i32, p_Colour_u16);
        Libs_ILI9341_DrawPixel(p_LocationX_u16 - f_IndexX_i32, p_LocationY_u16 + f_IndexY_i32, p_Colour_u16);
        Libs_ILI9341_DrawPixel(p_LocationX_u16 - f_IndexX_i32, p_LocationY_u16 - f_IndexY_i32, p_Colour_u16);
        Libs_ILI9341_DrawPixel(p_LocationX_u16 - f_IndexY_i32, p_LocationY_u16 - f_IndexX_i32, p_Colour_u16);
        Libs_ILI9341_DrawPixel(p_LocationX_u16 + f_IndexY_i32, p_LocationY_u16 - f_IndexX_i32, p_Colour_u16);
        Libs_ILI9341_DrawPixel(p_LocationX_u16 + f_IndexX_i32, p_LocationY_u16 - f_IndexY_i32, p_Colour_u16);

        if (f_Error_i32 <= 0)
        {
            f_IndexY_i32++;
            f_Error_i32 += f_Dy_i32;
            f_Dy_i32 += 2;
        }
        if (f_Error_i32 > 0)
        {
            f_IndexX_i32--;
            f_Dx_i32 += 2;
            f_Error_i32 += (-p_Radius_u16 << 1) + f_Dx_i32;
        }
    }
}

/*Draw filled circle at X,Y location with specified radius and colour. X and Y represent circles center */
void Libs_ILI9341_DrawFilledCircle(uint16_t p_LocationX_u16, uint16_t p_LocationY_u16, uint16_t p_Radius_u16, uint16_t p_Colour_u16)
{
	int32_t f_IndexX_i32 = p_Radius_u16;
    int32_t f_IndexY_i32 = 0;
    int32_t f_ChangeX_i32 = 1 - (p_Radius_u16 << 1);
    int32_t f_ChangeY_i32 = 0;
    int32_t f_RadiusError_i32 = 0;
	int32_t f_Counter1_i32;

    while (f_IndexX_i32 >= f_IndexY_i32)
    {
        for (f_Counter1_i32 = p_LocationX_u16 - f_IndexX_i32; f_Counter1_i32 <= p_LocationX_u16 + f_IndexX_i32; f_Counter1_i32++)
        {
            Libs_ILI9341_DrawPixel(f_Counter1_i32, p_LocationY_u16 + f_IndexY_i32,p_Colour_u16);
            Libs_ILI9341_DrawPixel(f_Counter1_i32, p_LocationY_u16 - f_IndexY_i32,p_Colour_u16);
        }
        for (f_Counter1_i32 = p_LocationX_u16 - f_IndexY_i32; f_Counter1_i32 <= p_LocationX_u16 + f_IndexY_i32; f_Counter1_i32++)
        {
            Libs_ILI9341_DrawPixel(f_Counter1_i32, p_LocationY_u16 + f_IndexX_i32,p_Colour_u16);
            Libs_ILI9341_DrawPixel(f_Counter1_i32, p_LocationY_u16 - f_IndexX_i32,p_Colour_u16);
        }

        f_IndexY_i32++;
        f_RadiusError_i32 += f_ChangeY_i32;
        f_ChangeY_i32 += 2;
        if (((f_RadiusError_i32 << 1) + f_ChangeX_i32) > 0)
        {
            f_IndexX_i32--;
            f_RadiusError_i32 += f_ChangeX_i32;
            f_ChangeX_i32 += 2;
        }
    }
}

/*Draw a hollow rectangle between positions X0,Y0 and X1,Y1 with specified colour*/
void Libs_ILI9341_DrawHollowRectangleCoord(uint16_t p_CoordX0_u16, uint16_t p_CoordY0_u16, uint16_t p_CoordX1_u16, uint16_t p_CoordY1_u16, uint16_t p_Colour_u16)
{
	uint8_t		f_NegativeX_u8 = 0;
	uint8_t 	f_NegativeY_u8 = 0;
	uint16_t 	f_LengthX_u16 = 0;
	uint16_t 	f_LengthY_u16 = 0;
	float 		f_CalcNegative_float = 0;
	
	f_CalcNegative_float = p_CoordX1_u16 - p_CoordX0_u16;
	if(f_CalcNegative_float < 0) f_NegativeX_u8 = 1;
	f_CalcNegative_float = 0;
	
	f_CalcNegative_float = p_CoordY1_u16 - p_CoordY0_u16;
	if(f_CalcNegative_float < 0) f_NegativeY_u8 = 1;
	
	//DRAW HORIZONTAL!
	if(!f_NegativeX_u8)
	{
		f_LengthX_u16 = p_CoordX1_u16 - p_CoordX0_u16;		
	}
	else
	{
		f_LengthX_u16 = p_CoordX0_u16 - p_CoordX1_u16;		
	}
	Libs_ILI9341_DrawHorizontalLine(p_CoordX0_u16, p_CoordY0_u16, f_LengthX_u16, p_Colour_u16);
	Libs_ILI9341_DrawHorizontalLine(p_CoordX0_u16, p_CoordY1_u16, f_LengthX_u16, p_Colour_u16);
	
	//DRAW VERTICAL!
	if(!f_NegativeY_u8)
	{
		f_LengthY_u16 = p_CoordY1_u16 - p_CoordY0_u16;		
	}
	else
	{
		f_LengthY_u16 = p_CoordY0_u16 - p_CoordY1_u16;		
	}

	Libs_ILI9341_DrawVerticalLine(p_CoordX0_u16, p_CoordY0_u16, f_LengthY_u16, p_Colour_u16);
	Libs_ILI9341_DrawVerticalLine(p_CoordX1_u16, p_CoordY0_u16, f_LengthY_u16, p_Colour_u16);
	
	if((f_LengthX_u16 > 0)||(f_LengthY_u16 > 0)) 
	{
		Libs_ILI9341_DrawPixel(p_CoordX1_u16, p_CoordY1_u16, p_Colour_u16);
	}
	
}

/*Draw a filled rectangle between positions X0,Y0 and X1,Y1 with specified colour*/
void Libs_ILI9341_DrawFilledRectangleCoord(uint16_t p_CoordX0_u16, uint16_t p_CoordY0_u16, uint16_t p_CoordX1_u16, uint16_t p_CoordY1_u16, uint16_t p_Colour_u16)
{
	uint8_t		f_NegativeX_u8 = 0;
	uint8_t 	f_NegativeY_u8 = 0;
	uint16_t 	f_LengthX_u16 = 0;
	uint16_t 	f_LengthY_u16 = 0;
	uint16_t 	f_TrueX0_u16 = 0;
	uint16_t 	f_TrueY0_u16 = 0;
	int32_t 	f_CalcNegative_i32 = 0;
	
	f_CalcNegative_i32 = p_CoordX1_u16 - p_CoordX0_u16;
	if(f_CalcNegative_i32 < 0) f_NegativeX_u8 = 1;
	f_CalcNegative_i32 = 0;
	
	f_CalcNegative_i32 = p_CoordY1_u16 - p_CoordY0_u16;
	if(f_CalcNegative_i32 < 0) f_NegativeY_u8 = 1;
	
	
	//DRAW HORIZONTAL!
	if(!f_NegativeX_u8)
	{
		f_LengthX_u16 = p_CoordX1_u16 - p_CoordX0_u16;
		f_TrueX0_u16 = p_CoordX0_u16;
	}
	else
	{
		f_LengthX_u16 = p_CoordX0_u16 - p_CoordX1_u16;
		f_TrueX0_u16 = p_CoordY1_u16;
	}
	
	//DRAW VERTICAL!
	if(!f_NegativeY_u8)
	{
		f_LengthY_u16 = p_CoordY1_u16 - p_CoordY0_u16;
		f_TrueY0_u16 = p_CoordY0_u16;		
	}
	else
	{
		f_LengthY_u16 = p_CoordY0_u16 - p_CoordY1_u16;
		f_TrueY0_u16 = p_CoordY1_u16;	
	}
	
	Libs_ILI9341_DrawRectangle(f_TrueX0_u16, f_TrueY0_u16, f_LengthX_u16, f_LengthY_u16, p_Colour_u16);	
}

/*Draws a character (fonts imported from fonts.h) at X,Y location with specified font colour, size and Background colour*/
void Libs_ILI9341_DrawChar(char p_Character_char, uint8_t p_LocationX_u16, uint8_t p_LocationY_u16, uint16_t p_Colour_u16, uint16_t p_Size_u16, uint16_t p_BackgroudColour_u16)
{
	uint8_t f_FunctionChar_u8;
    uint8_t f_Counter1_u8,f_Counter2_u8;
	char f_Temp_char[ILI9341_CHAR_WIDTH];
		
	f_FunctionChar_u8 = p_Character_char;
		
    if (f_FunctionChar_u8 < ' ') {
        p_Character_char = 0;
    }
	else
	{
        f_FunctionChar_u8 -= 32;
	}
   	
	for(f_Counter1_u8 = 0; f_Counter1_u8<ILI9341_CHAR_WIDTH; f_Counter1_u8++)
	{
		f_Temp_char[f_Counter1_u8] = g_Font_aa[f_FunctionChar_u8][f_Counter1_u8];
	}
		
    // Draw pixels
	Libs_ILI9341_DrawRectangle(p_LocationX_u16, p_LocationY_u16, ILI9341_CHAR_WIDTH*p_Size_u16, ILI9341_CHAR_HEIGHT*p_Size_u16, p_BackgroudColour_u16);
    for (f_Counter1_u8=0; f_Counter1_u8<ILI9341_CHAR_WIDTH; f_Counter1_u8++)
	{
        for (f_Counter2_u8=0; f_Counter2_u8<ILI9341_CHAR_HEIGHT; f_Counter2_u8++)
		{
            if (f_Temp_char[f_Counter1_u8] & (1<<f_Counter2_u8))
			{			
				if(p_Size_u16 == 1)
				{
					Libs_ILI9341_DrawPixel(p_LocationX_u16+f_Counter1_u8, p_LocationY_u16+f_Counter2_u8, p_Colour_u16);
				}
				else
				{
					Libs_ILI9341_DrawRectangle(p_LocationX_u16+(f_Counter1_u8*p_Size_u16), p_LocationY_u16+(f_Counter2_u8*p_Size_u16), p_Size_u16, p_Size_u16, p_Colour_u16);
				}
            }						
        }
    }
}

/*Draws an array of characters (fonts imported from fonts.h) at X,Y location with specified font colour, size and Background colour*/
void Libs_ILI9341_DrawText(const char* p_Text_char, uint16_t p_LocationX_u16, uint16_t p_LocationY_u16, uint16_t p_Colour_u16, uint16_t p_Size_u16, uint16_t p_BackgroudColour_u16)
{
    while (*p_Text_char)
	{
        Libs_ILI9341_DrawChar(*p_Text_char++, p_LocationX_u16, p_LocationY_u16, p_Colour_u16, p_Size_u16, p_BackgroudColour_u16);
        p_LocationX_u16 += ILI9341_CHAR_WIDTH*p_Size_u16;
    }
}

//USING CONVERTER: http://www.digole.com/tools/PicturetoC_Hex_converter.php
//65K colour (2Bytes / Pixel)
void Libs_ILI9341_DrawImage(const uint8_t* p_Image_u8, uint8_t p_Orientation_u8)
{
	uint8_t f_TempBuffer_u8[ILI9341_BURST_MAX_SIZE];
	uint8_t f_ValidArgument_u8 = 1;
	uint32_t f_Counter_u32 = 0;
	uint32_t f_TempCouter1_u32, f_TempCouter2_u32;
	uint32_t f_TempCalc_u32 = ILI9341_SCREEN_WIDTH*ILI9341_SCREEN_HEIGHT*2/ILI9341_BURST_MAX_SIZE;

	switch (p_Orientation_u8)
	{
		case ILI9341_SCREEN_HORIZONTAL_1:
			Libs_ILI9341_SetRotation(ILI9341_SCREEN_HORIZONTAL_1);
			Libs_ILI9341_SetAddress(0,0,ILI9341_SCREEN_WIDTH,ILI9341_SCREEN_HEIGHT);
			break;
		case ILI9341_SCREEN_HORIZONTAL_2:
			Libs_ILI9341_SetRotation(ILI9341_SCREEN_HORIZONTAL_2);
			Libs_ILI9341_SetAddress(0,0,ILI9341_SCREEN_WIDTH,ILI9341_SCREEN_HEIGHT);
			break;
		case ILI9341_SCREEN_VERTICAL_2:
			Libs_ILI9341_SetRotation(ILI9341_SCREEN_VERTICAL_2);
			Libs_ILI9341_SetAddress(0,0,ILI9341_SCREEN_HEIGHT,ILI9341_SCREEN_WIDTH);
			break;
		case ILI9341_SCREEN_VERTICAL_1:
			Libs_ILI9341_SetRotation(ILI9341_SCREEN_VERTICAL_1);
			Libs_ILI9341_SetAddress(0,0,ILI9341_SCREEN_HEIGHT,ILI9341_SCREEN_WIDTH);
			break;
		default:
			f_ValidArgument_u8 = 0;
			break;
	}
	if(f_ValidArgument_u8)
	{
		Wrappers_Gpio_Write(LogicalChannel_3, 1);
		Wrappers_Gpio_Write(LogicalChannel_1, 0);
		for(f_TempCouter1_u32 = 0; f_TempCouter1_u32 < f_TempCalc_u32; f_TempCouter1_u32++)
		{
			for(f_TempCouter2_u32 = 0; f_TempCouter2_u32< ILI9341_BURST_MAX_SIZE; f_TempCouter2_u32++)
			{
				f_TempBuffer_u8[f_TempCouter2_u32] = p_Image_u8[f_Counter_u32 + f_TempCouter2_u32];
			}
			Wrappers_Spi_Transmit(SPI_Screen, f_TempBuffer_u8, ILI9341_BURST_MAX_SIZE);
			f_Counter_u32 += ILI9341_BURST_MAX_SIZE;
		}
		Wrappers_Gpio_Write(LogicalChannel_1, 1);
	}
}
