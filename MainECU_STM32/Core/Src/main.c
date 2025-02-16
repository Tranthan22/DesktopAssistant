#include "main.h"
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

int main(void)
{
	Libs_System_Init();
	Libs_Clock_SystemClock_Config();
	Libs_Spi_Init();
	Libs_I2c_Init();
	Libs_Gpio_Init();
	Libs_Pwm_Init();

	/* SD card middleware */
	MX_FATFS_Init();
	Libs_ILI9341_Init();
	Libs_Gpio_Write(LogicalChannel_4, 1);
	Libs_ILI9341_FillScreen(ILI9341_ORANGE);
	HAL_Delay(5000);
	Libs_ILI9341_DrawImage(Image, ILI9341_SCREEN_HORIZONTAL_2);
	while (1)
	{

	}
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

