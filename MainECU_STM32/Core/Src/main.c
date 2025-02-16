#include "main.h"
#include "fatfs.h"
#include "Wrappers_System.h"
#include "Wrappers_Clock.h"
#include "Wrappers_I2c.h"
#include "Wrappers_Spi.h"
#include "Wrappers_Gpio.h"
#include "Wrappers_Pwm.h"
#include "Libs_ILI9341_Driver.h"
#include "Libs_ILI9341_GFX.h"
#include "Photo_Sources.h"
int main(void)
{
	Wrappers_System_Init();
	Wrappers_Clock_SystemClock_Config();
	Wrappers_Spi_Init();
	Wrappers_I2c_Init();
	Wrappers_Gpio_Init();
	Wrappers_Pwm_Init();

	/* SD card middleware */
	MX_FATFS_Init();
	Libs_ILI9341_Init();
	Wrappers_Gpio_Write(LogicalChannel_4, 1);
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

