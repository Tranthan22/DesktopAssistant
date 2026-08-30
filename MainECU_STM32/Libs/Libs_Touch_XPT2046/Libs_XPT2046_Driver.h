//	MIT License
//
//	Copyright (c) 2017 Matej Artnak
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in all
//	copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//	SOFTWARE.
//
//-----------------------------------
//	XPT2046 resistive touch driver for ILI9341 modules
//-----------------------------------
//	Based on the ILI9341 Touchscreen library by Matej Artnak (bit-banged SPI).
//	Ported to this project's layered architecture:
//	- Hardware SPI1 shared with the screen, slowed down per transaction (SPI_Touch channel)
//	- CS / PENIRQ via logical GPIO channels (LogicalChannel_5 / LogicalChannel_6 in main.h)
//	- No runtime calibration: hardcoded values below, tuned for ILI9341_SCREEN_VERTICAL_1.
//	  Re-measure X/Y offset & magnitude if the panel or rotation changes.
//
//	Usage:
//		Libs_XPT2046_Init();	// after Libs_Gpio_Init + Libs_Spi_Init
//		if (Libs_XPT2046_IsPressed())
//		{
//			uint16_t f_Position_u16[2];
//			if (Libs_XPT2046_ReadCoordinates(f_Position_u16) == XPT2046_DATA_OK)
//			{
//				// f_Position_u16[0] = X, f_Position_u16[1] = Y
//			}
//		}
//-----------------------------------

#ifndef LIBS_TOUCH_XPT2046_LIBS_XPT2046_DRIVER_H_
#define LIBS_TOUCH_XPT2046_LIBS_XPT2046_DRIVER_H_

// /************************************************************************************************************
//  * INCLUDES
//  ************************************************************************************************************/
#include <stdint.h>

// /************************************************************************************************************
//  * MACROS AND DEFINES
//  ************************************************************************************************************/
//RETURN VALUES FOR Libs_XPT2046_IsPressed
#define XPT2046_NOT_PRESSED                     0
#define XPT2046_PRESSED                         1

//RETURN VALUES FOR Libs_XPT2046_ReadCoordinates
#define XPT2046_DATA_OK                         1
#define XPT2046_DATA_NOISY                      0

//HARDCODED CALIBRATION (for ILI9341_SCREEN_VERTICAL_1), CHANGE IF REQUIRED
#define XPT2046_X_OFFSET                        13
#define XPT2046_Y_OFFSET                        15
#define XPT2046_X_MAGNITUDE                     1.16f
#define XPT2046_Y_MAGNITUDE                     1.16f

//Converting 16bit raw value to screen coordinates: 65535/273 = 240, 65535/204 = 320
#define XPT2046_X_TRANSLATION                   273
#define XPT2046_Y_TRANSLATION                   204

//Number of samples averaged per read: more samples = less noise but longer blocking time
//(~60us per sample over hardware SPI at 1.5625MHz)
#define XPT2046_POSITION_SAMPLES                256

// /************************************************************************************************************
//  * TYPEDEFS
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * EXPORTED VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * GLOBAL FUNCTION PROTOTYPES
//  ************************************************************************************************************/
void Libs_XPT2046_Init(void);
uint8_t Libs_XPT2046_IsPressed(void);
uint8_t Libs_XPT2046_ReadCoordinates(uint16_t p_Coordinates_u16[2]);

#endif /* LIBS_TOUCH_XPT2046_LIBS_XPT2046_DRIVER_H_ */
