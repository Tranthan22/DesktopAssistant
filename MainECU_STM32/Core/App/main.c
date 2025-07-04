#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "Typedef.h"
#include "fatfs.h"
#include "Libs_System.h"
#include "Libs_Clock.h"
#include "Libs_I2c.h"
#include "Libs_Spi.h"
#include "Libs_Gpio.h"
#include "Libs_Pwm.h"
#include "Libs_ILI9341_Driver.h"
#include "Libs_ILI9341_GFX.h"
#include "Photo_Sources.h"

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

volatile uint8_t hour = 12, minute = 34, second = 56;

void UpdateTime() {
    second++;
    if (second >= 60) {
        second = 0;
        minute++;
        if (minute >= 60) {
            minute = 0;
            hour++;
            if (hour >= 24) hour = 0;
        }
    }
}

void DisplayClock() {
    char timeStr[10];  // "HH:MM:SS"
    sprintf(timeStr, "%02d:%02d:%02d", hour, minute, second);
    Libs_ILI9341_DrawFilledRectangleCoord(60, 100, 260, 140, 0x0000);
    Libs_ILI9341_DrawText(timeStr, 80, 110, 0xFFFF, 3, 0x0000);
}



// ************************************************************************************************************
//  * GLOBAL FUNCTIONS
//  ************************************************************************************************************/
int main(void)
{
	/* Initialize system */
	Libs_System_Init();
	Libs_Clock_SystemClock_Config();
	Libs_Spi_Init();
	Libs_I2c_Init();
	Libs_Gpio_Init();
	Libs_Pwm_Init();
	Libs_ILI9341_Init();

	/* SD card middleware */
	MX_FATFS_Init();
	
	/* Application */
	Libs_Gpio_Write(LogicalChannel_4, 1);		// Turn on Screen LED
	Libs_ILI9341_DrawImage(Image, ILI9341_SCREEN_HORIZONTAL_1);	//Welcome screen
	HAL_Delay(5000);
	Libs_ILI9341_FillScreen(ILI9341_BLACK);	// Fill init Screen Black
	Libs_ILI9341_DrawText("Digital Clock", 90, 40, 0xFFFF, 2, ILI9341_BLACK);
//
//#include "stm32f4xx.h"
//extern I2C_HandleTypeDef l_I2c_Handler_st;
//	HAL_I2C_EnableListen_IT(&l_I2c_Handler_st);


	while (1)
	{
		DisplayClock();
        UpdateTime();
        HAL_Delay(1000);
	}
}
//
//extern void HAL_I2C_ListenCpltCallback (I2C_HandleTypeDef *hi2c)
//{
//	HAL_I2C_EnableListen_IT(hi2c);
//}

//extern void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
//{
//	if(TransferDirection == I2C_DIRECTION_TRANSMIT)  // if the master wants to transmit the data
//	{
//		HAL_I2C_Slave_Sequential_Receive_IT(hi2c, &second, 1, I2C_FIRST_AND_LAST_FRAME);
//	}
//	else  // I2C_DIRECTION_RECEIVE
//	{
//		HAL_I2C_Slave_Sequential_Receive_IT(hi2c, &second, 1, I2C_FIRST_AND_LAST_FRAME);
//		HAL_I2C_Slave_Transmit(hi2c, &second, 1, 5000);
//
//	}
//}
//
//void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
//{
//	static uint8_t count = 0;
//	count++;
//}
//
//void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
//{
//	HAL_I2C_EnableListen_IT(hi2c);
//}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

