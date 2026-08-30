#include "Wrappers_Spi.h"

// /************************************************************************************************************
//  * EXTERN VARIABLES
//  ************************************************************************************************************/
extern SPI_HandleTypeDef hspi1;     /* CubeMX-generated handles (Core/Src/main.c) */
extern SPI_HandleTypeDef hspi2;

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
static void Wrappers_Spi_SetPrescaler(SPI_HandleTypeDef* p_Handler_st, uint32_t p_Prescaler_u32);

// /************************************************************************************************************
//  * STATIC FUNCTIONS
//  ************************************************************************************************************/
/* Change baudrate prescaler at runtime (SPI must be idle). Used for slow slaves sharing a fast bus. */
static void Wrappers_Spi_SetPrescaler(SPI_HandleTypeDef* p_Handler_st, uint32_t p_Prescaler_u32)
{
	__HAL_SPI_DISABLE(p_Handler_st);
	MODIFY_REG(p_Handler_st->Instance->CR1, SPI_CR1_BR, p_Prescaler_u32);
}

// ************************************************************************************************************
//  * GLOBAL FUNCTIONS
//  ************************************************************************************************************/
/* Cau hinh SPI1/SPI2 do CubeMX dam nhiem (MX_SPI1_Init / MX_SPI2_Init trong main.c) */
void Wrappers_Spi_Init(void)
{
}

void Wrappers_Spi_Transmit(uint8_t p_LogicalChannel_u8, uint8_t* p_Data_u8, uint16_t p_Size_u16)
{
	if(p_LogicalChannel_u8 == 1)
	{
		HAL_SPI_Transmit(&hspi1, p_Data_u8, p_Size_u16, 100);
	}
	else if(p_LogicalChannel_u8 == 2)
	{
		HAL_SPI_Transmit(&hspi2, p_Data_u8, p_Size_u16, 100);
	}
	else if(p_LogicalChannel_u8 == 3)
	{
		/* Channel 3 = SPI1 shared bus at low speed (XPT2046 max ~2MHz): PCLK2 100MHz / 64 = 1.5625MHz */
		Wrappers_Spi_SetPrescaler(&hspi1, SPI_BAUDRATEPRESCALER_64);
		HAL_SPI_Transmit(&hspi1, p_Data_u8, p_Size_u16, 100);
		Wrappers_Spi_SetPrescaler(&hspi1, SPI_BAUDRATEPRESCALER_8);
	}
	else if(p_LogicalChannel_u8 == 4)
	{
		/* Channel 4 = SPI2 at full speed 25Mbit/s - SD card data phase */
		HAL_SPI_Transmit(&hspi2, p_Data_u8, p_Size_u16, 100);
	}
	else if(p_LogicalChannel_u8 == 5)
	{
		/* Channel 5 = SPI2 at ~390kHz (PCLK1 50MHz / 128) - SD card init needs <=400kHz */
		Wrappers_Spi_SetPrescaler(&hspi2, SPI_BAUDRATEPRESCALER_128);
		HAL_SPI_Transmit(&hspi2, p_Data_u8, p_Size_u16, 100);
		Wrappers_Spi_SetPrescaler(&hspi2, SPI_BAUDRATEPRESCALER_2);
	}
	else
	{
	}
}

void Wrappers_Spi_TransmitReceive(uint8_t p_LogicalChannel_u8, uint8_t* p_TxData_u8, uint8_t* p_RxData_u8, uint16_t p_Size_u16)
{
	if(p_LogicalChannel_u8 == 1)
	{
		HAL_SPI_TransmitReceive(&hspi1, p_TxData_u8, p_RxData_u8, p_Size_u16, 100);
	}
	else if(p_LogicalChannel_u8 == 2)
	{
		HAL_SPI_TransmitReceive(&hspi2, p_TxData_u8, p_RxData_u8, p_Size_u16, 100);
	}
	else if(p_LogicalChannel_u8 == 3)
	{
		/* Channel 3 = SPI1 shared bus at low speed (XPT2046 max ~2MHz): PCLK2 100MHz / 64 = 1.5625MHz */
		Wrappers_Spi_SetPrescaler(&hspi1, SPI_BAUDRATEPRESCALER_64);
		HAL_SPI_TransmitReceive(&hspi1, p_TxData_u8, p_RxData_u8, p_Size_u16, 100);
		Wrappers_Spi_SetPrescaler(&hspi1, SPI_BAUDRATEPRESCALER_8);
	}
	else if(p_LogicalChannel_u8 == 4)
	{
		/* Channel 4 = SPI2 at full speed 25Mbit/s - SD card data phase */
		HAL_SPI_TransmitReceive(&hspi2, p_TxData_u8, p_RxData_u8, p_Size_u16, 100);
	}
	else if(p_LogicalChannel_u8 == 5)
	{
		/* Channel 5 = SPI2 at ~390kHz (PCLK1 50MHz / 128) - SD card init needs <=400kHz */
		Wrappers_Spi_SetPrescaler(&hspi2, SPI_BAUDRATEPRESCALER_128);
		HAL_SPI_TransmitReceive(&hspi2, p_TxData_u8, p_RxData_u8, p_Size_u16, 100);
		Wrappers_Spi_SetPrescaler(&hspi2, SPI_BAUDRATEPRESCALER_2);
	}
	else
	{
	}
}