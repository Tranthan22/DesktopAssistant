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
SPI_HandleTypeDef l_Spi1_Handler;
SPI_HandleTypeDef l_Spi2_Handler;

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
void Wrappers_Spi_Init(void)
{
	/* SPI1 parameter configuration*/
	l_Spi1_Handler.Instance = SPI1;
	l_Spi1_Handler.Init.Mode = SPI_MODE_MASTER;
	l_Spi1_Handler.Init.Direction = SPI_DIRECTION_2LINES;
	l_Spi1_Handler.Init.DataSize = SPI_DATASIZE_8BIT;
	l_Spi1_Handler.Init.CLKPolarity = SPI_POLARITY_LOW;
	l_Spi1_Handler.Init.CLKPhase = SPI_PHASE_1EDGE;
	l_Spi1_Handler.Init.NSS = SPI_NSS_SOFT;
	l_Spi1_Handler.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
	l_Spi1_Handler.Init.FirstBit = SPI_FIRSTBIT_MSB;
	l_Spi1_Handler.Init.TIMode = SPI_TIMODE_DISABLE;
	l_Spi1_Handler.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	l_Spi1_Handler.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&l_Spi1_Handler) != HAL_OK)
	{
		Error_Handler();
	}

	/* SPI2 parameter configuration*/
	l_Spi2_Handler.Instance = SPI2;
	l_Spi2_Handler.Init.Mode = SPI_MODE_MASTER;
	l_Spi2_Handler.Init.Direction = SPI_DIRECTION_2LINES;
	l_Spi2_Handler.Init.DataSize = SPI_DATASIZE_8BIT;
	l_Spi2_Handler.Init.CLKPolarity = SPI_POLARITY_LOW;
	l_Spi2_Handler.Init.CLKPhase = SPI_PHASE_1EDGE;
	l_Spi2_Handler.Init.NSS = SPI_NSS_SOFT;
	l_Spi2_Handler.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	l_Spi2_Handler.Init.FirstBit = SPI_FIRSTBIT_MSB;
	l_Spi2_Handler.Init.TIMode = SPI_TIMODE_DISABLE;
	l_Spi2_Handler.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	l_Spi2_Handler.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&l_Spi2_Handler) != HAL_OK)
	{
		Error_Handler();
	}
}

void Wrappers_Spi_Transmit(uint8_t p_LogicalChannel_u8, uint8_t* p_Data_u8, uint16_t p_Size_u16)
{
	if(p_LogicalChannel_u8 == 1)
	{
		HAL_SPI_Transmit(&l_Spi1_Handler, p_Data_u8, p_Size_u16, 100);
	}
	else if(p_LogicalChannel_u8 == 2)
	{
		HAL_SPI_Transmit(&l_Spi2_Handler, p_Data_u8, p_Size_u16, 100);
	}
	else
	{
	}
}