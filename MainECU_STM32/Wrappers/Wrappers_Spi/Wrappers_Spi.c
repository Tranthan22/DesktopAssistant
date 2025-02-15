#include "main.h"

SPI_HandleTypeDef g_Spi1_Handler;
SPI_HandleTypeDef g_Spi2_Handler;

void Wrappers_Spi_Init(void)
{
	/* SPI1 parameter configuration*/
	g_Spi1_Handler.Instance = SPI1;
	g_Spi1_Handler.Init.Mode = SPI_MODE_MASTER;
	g_Spi1_Handler.Init.Direction = SPI_DIRECTION_2LINES;
	g_Spi1_Handler.Init.DataSize = SPI_DATASIZE_8BIT;
	g_Spi1_Handler.Init.CLKPolarity = SPI_POLARITY_LOW;
	g_Spi1_Handler.Init.CLKPhase = SPI_PHASE_1EDGE;
	g_Spi1_Handler.Init.NSS = SPI_NSS_SOFT;
	g_Spi1_Handler.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
	g_Spi1_Handler.Init.FirstBit = SPI_FIRSTBIT_MSB;
	g_Spi1_Handler.Init.TIMode = SPI_TIMODE_DISABLE;
	g_Spi1_Handler.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	g_Spi1_Handler.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&g_Spi1_Handler) != HAL_OK)
	{
		Error_Handler();
	}

	/* SPI2 parameter configuration*/
	g_Spi2_Handler.Instance = SPI2;
	g_Spi2_Handler.Init.Mode = SPI_MODE_MASTER;
	g_Spi2_Handler.Init.Direction = SPI_DIRECTION_2LINES;
	g_Spi2_Handler.Init.DataSize = SPI_DATASIZE_8BIT;
	g_Spi2_Handler.Init.CLKPolarity = SPI_POLARITY_LOW;
	g_Spi2_Handler.Init.CLKPhase = SPI_PHASE_1EDGE;
	g_Spi2_Handler.Init.NSS = SPI_NSS_SOFT;
	g_Spi2_Handler.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	g_Spi2_Handler.Init.FirstBit = SPI_FIRSTBIT_MSB;
	g_Spi2_Handler.Init.TIMode = SPI_TIMODE_DISABLE;
	g_Spi2_Handler.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	g_Spi2_Handler.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&g_Spi2_Handler) != HAL_OK)
	{
		Error_Handler();
	}
}

void Wrappers_Spi_Transmit(uint8_t p_LogicalChannel_u8, uint8_t p_Data_u8)
{
	if(p_LogicalChannel_u8 == 1)
	{
		HAL_SPI_Transmit(&g_Spi1_Handler, &p_Data_u8, 1, 1);
	}
	else if(p_LogicalChannel_u8 == 2)
	{
		HAL_SPI_Transmit(&g_Spi2_Handler, &p_Data_u8, 1, 1);
	}
	else
	{
	}
}