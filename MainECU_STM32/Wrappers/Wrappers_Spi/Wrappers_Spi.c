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
SPI_HandleTypeDef l_Spi_Handler1_st;
SPI_HandleTypeDef l_Spi_Handler2_st;

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
	l_Spi_Handler1_st.Instance = SPI1;
	l_Spi_Handler1_st.Init.Mode = SPI_MODE_MASTER;
	l_Spi_Handler1_st.Init.Direction = SPI_DIRECTION_2LINES;
	l_Spi_Handler1_st.Init.DataSize = SPI_DATASIZE_8BIT;
	l_Spi_Handler1_st.Init.CLKPolarity = SPI_POLARITY_LOW;
	l_Spi_Handler1_st.Init.CLKPhase = SPI_PHASE_1EDGE;
	l_Spi_Handler1_st.Init.NSS = SPI_NSS_SOFT;
	l_Spi_Handler1_st.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
	l_Spi_Handler1_st.Init.FirstBit = SPI_FIRSTBIT_MSB;
	l_Spi_Handler1_st.Init.TIMode = SPI_TIMODE_DISABLE;
	l_Spi_Handler1_st.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	l_Spi_Handler1_st.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&l_Spi_Handler1_st) != HAL_OK)
	{
		Error_Handler();
	}

	/* SPI2 parameter configuration*/
	l_Spi_Handler2_st.Instance = SPI2;
	l_Spi_Handler2_st.Init.Mode = SPI_MODE_MASTER;
	l_Spi_Handler2_st.Init.Direction = SPI_DIRECTION_2LINES;
	l_Spi_Handler2_st.Init.DataSize = SPI_DATASIZE_8BIT;
	l_Spi_Handler2_st.Init.CLKPolarity = SPI_POLARITY_LOW;
	l_Spi_Handler2_st.Init.CLKPhase = SPI_PHASE_1EDGE;
	l_Spi_Handler2_st.Init.NSS = SPI_NSS_SOFT;
	l_Spi_Handler2_st.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	l_Spi_Handler2_st.Init.FirstBit = SPI_FIRSTBIT_MSB;
	l_Spi_Handler2_st.Init.TIMode = SPI_TIMODE_DISABLE;
	l_Spi_Handler2_st.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	l_Spi_Handler2_st.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&l_Spi_Handler2_st) != HAL_OK)
	{
		Error_Handler();
	}
}

void Wrappers_Spi_Transmit(uint8_t p_LogicalChannel_u8, uint8_t* p_Data_u8, uint16_t p_Size_u16)
{
	if(p_LogicalChannel_u8 == 1)
	{
		HAL_SPI_Transmit(&l_Spi_Handler1_st, p_Data_u8, p_Size_u16, 100);
	}
	else if(p_LogicalChannel_u8 == 2)
	{
		HAL_SPI_Transmit(&l_Spi_Handler2_st, p_Data_u8, p_Size_u16, 100);
	}
	else
	{
	}
}