#include "main.h"
#include "fatfs.h"
#include "Wrappers_System.h"
#include "Wrappers_Clock.h"
#include "Wrappers_I2c.h"
#include "Wrappers_Spi.h"
#include "Wrappers_Gpio.h"
#include "Wrappers_Pwm.h"

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

