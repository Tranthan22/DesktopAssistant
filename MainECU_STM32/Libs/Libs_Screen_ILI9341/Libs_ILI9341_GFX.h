#ifndef ILI9341_GFX_H
#define ILI9341_GFX_H

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
void Libs_ILI9341_DrawFilledCircle(uint16_t p_LocationX_u16, uint16_t p_LocationY_u16, uint16_t p_Radius_u16, uint16_t p_Colour_u16);
void Libs_ILI9341_DrawHollowCircle(uint16_t p_LocationX_u16, uint16_t p_LocationY_u16, uint16_t p_Radius_u16, uint16_t p_Colour_u16);
void Libs_ILI9341_DrawHollowRectangleCoord(uint16_t p_CoordX0_u16, uint16_t p_CoordY0_u16, uint16_t p_CoordX1_u16, uint16_t p_CoordY1_u16, uint16_t p_Colour_u16);
void Libs_ILI9341_DrawFilledRectangleCoord(uint16_t p_CoordX0_u16, uint16_t p_CoordY0_u16, uint16_t p_CoordX1_u16, uint16_t p_CoordY1_u16, uint16_t p_Colour_u16);
void Libs_ILI9341_DrawChar(char p_Character_char, uint8_t p_LocationX_u16, uint8_t p_LocationY_u16, uint16_t p_Colour_u16, uint16_t p_Size_u16, uint16_t p_BackgroudColour_u16);
void Libs_ILI9341_DrawText(const char* p_Text_char, uint16_t p_LocationX_u16, uint16_t p_LocationY_u16, uint16_t p_Colour_u16, uint16_t p_Size_u16, uint16_t p_BackgroudColour_u16);
void Libs_ILI9341_DrawImage(const uint8_t* p_Image_u8, uint8_t p_Orientation_u8);

#endif
