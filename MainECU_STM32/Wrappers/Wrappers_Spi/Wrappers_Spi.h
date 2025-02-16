#ifndef _WRAPPERS_SPI_H_
#define _WRAPPERS_SPI_H_


void Wrappers_Spi_Init(void);
void Wrappers_Spi_Transmit(uint8_t p_LogicalChannel_u8, uint8_t* p_Data_u8, uint16_t p_Size_u16);

#endif /* _WRAPPERS_SPI_H_ */
