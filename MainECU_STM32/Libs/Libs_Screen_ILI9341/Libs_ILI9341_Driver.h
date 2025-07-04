//	--SPI @ 50MHz 
//	--Redraw entire screen
//
//	++		Theoretical maximum FPS with 50Mhz SPI calculated to be 40.69 FPS
//	++		320*240 = 76800 pixels, each pixel contains 16bit colour information (2x8)
//	++		Theoretical Max FPS: 1/((320*240*16)/50000000)
//
//	With ART Accelerator, instruction prefetch, CPI ICACHE and CPU DCACHE enabled:
//
//	-FPS:				39.62
//	-SPI utilization:	97.37%
//	-MB/Second:			6.09
//
//	With ART Accelerator, instruction prefetch, CPI ICACHE and CPU DCACHE disabled:
//
//	-FPS:				35.45
//	-SPI utilization:	87.12%
//	-MB/Second:			5.44

#ifndef ILI9341_DRIVER_H
#define ILI9341_DRIVER_H

// /************************************************************************************************************
//  * INCLUDES
//  ************************************************************************************************************/


// /************************************************************************************************************
//  * MACROS AND DEFINES
//  ************************************************************************************************************/
#define ILI9341_SCREEN_HEIGHT		            240
#define ILI9341_SCREEN_WIDTH 		            320

#define ILI9341_BURST_MAX_SIZE 	                500

#define ILI9341_BLACK                           0x0000      
#define ILI9341_NAVY                            0x000F      
#define ILI9341_DARKGREEN                       0x03E0      
#define ILI9341_DARKCYAN                        0x03EF      
#define ILI9341_MAROON                          0x7800      
#define ILI9341_PURPLE                          0x780F      
#define ILI9341_OLIVE                           0x7BE0      
#define ILI9341_LIGHTGREY                       0xC618      
#define ILI9341_DARKGREY                        0x7BEF      
#define ILI9341_BLUE                            0x001F      
#define ILI9341_GREEN                           0x07E0      
#define ILI9341_CYAN                            0x07FF      
#define ILI9341_RED                             0xF800     
#define ILI9341_MAGENTA                         0xF81F      
#define ILI9341_YELLOW                          0xFFE0      
#define ILI9341_WHITE                           0xFFFF      
#define ILI9341_ORANGE                          0xFD20      
#define ILI9341_GREENYELLOW                     0xAFE5     
#define ILI9341_PINK                            0xF81F

#define ILI9341_SCREEN_VERTICAL_1			    0
#define ILI9341_SCREEN_HORIZONTAL_1	    		1
#define ILI9341_SCREEN_VERTICAL_2			    2
#define ILI9341_SCREEN_HORIZONTAL_2	    		3

// /************************************************************************************************************
//  * TYPEDEFS
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * EXPORTED VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * GLOBAL FUNCTION PROTOTYPES
//  ************************************************************************************************************/
void Libs_ILI9341_Init(void);
void Libs_ILI9341_Reset(void);
void Libs_ILI9341_SetRotation(uint8_t p_Rotation_u8);
void Libs_ILI9341_SetAddress(uint16_t p_LocationX1_u16, uint16_t p_LocationY1_u16, uint16_t p_LocationX2_u16, uint16_t p_LocationY2_u16);
void Libs_ILI9341_FillScreen(uint16_t p_Colour_u16);
void Libs_ILI9341_DrawPixel(uint16_t p_LocationX_u16, uint16_t p_LocationY_u16, uint16_t p_Colour_u16);
void Libs_ILI9341_DrawRectangle(uint16_t p_LocationX_u16, uint16_t p_LocationY_u16, uint16_t p_Width_u16, uint16_t p_Height_u16, uint16_t p_Colour_u16);
void Libs_ILI9341_DrawHorizontalLine(uint16_t p_LocationX_u16, uint16_t p_LocationY_u16, uint16_t p_Width_u16, uint16_t p_Colour_u16);
void Libs_ILI9341_DrawVerticalLine(uint16_t p_LocationX_u16, uint16_t p_LocationY_u16, uint16_t p_Height_u16, uint16_t p_Colour_u16);
	
#endif

