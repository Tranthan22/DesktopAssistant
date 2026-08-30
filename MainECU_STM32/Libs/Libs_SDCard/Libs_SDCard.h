//-----------------------------------
//	SD card driver (SPI mode)
//-----------------------------------
//	Driver the SD/SDHC/SDXC (va MMC) qua SPI, dung cho tang FatFs (user_diskio.c).
//	- Bus: SPI2 (danh rieng cho SD), CS rieng (LogicalChannel_7).
//	- Init o toc do <=400kHz (SPI_SdCardInit), sau do chay full speed (SPI_SdCard).
//	- Doc/ghi theo don block 512 byte (CMD17/CMD24 lap cho nhieu block).
//
//	Trinh tu su dung (user_diskio.c da goi san):
//		Libs_SdCard_Init();
//		Libs_SdCard_ReadBlocks(buf, sector, count);
//		Libs_SdCard_WriteBlocks(buf, sector, count);
//-----------------------------------

#ifndef LIBS_SDCARD_LIBS_SDCARD_H_
#define LIBS_SDCARD_LIBS_SDCARD_H_

// /************************************************************************************************************
//  * INCLUDES
//  ************************************************************************************************************/
#include <stdint.h>

// /************************************************************************************************************
//  * MACROS AND DEFINES
//  ************************************************************************************************************/
//RETURN VALUES
#define SDCARD_OK                               0
#define SDCARD_ERROR                            1

#define SDCARD_BLOCK_SIZE                       512

// /************************************************************************************************************
//  * TYPEDEFS
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * EXPORTED VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * GLOBAL FUNCTION PROTOTYPES
//  ************************************************************************************************************/
uint8_t Libs_SdCard_Init(void);
uint8_t Libs_SdCard_IsInitialized(void);
uint8_t Libs_SdCard_ReadBlocks(uint8_t* p_Buffer_u8, uint32_t p_Sector_u32, uint32_t p_Count_u32);
uint8_t Libs_SdCard_WriteBlocks(const uint8_t* p_Buffer_u8, uint32_t p_Sector_u32, uint32_t p_Count_u32);
uint32_t Libs_SdCard_GetSectorCount(void);
uint8_t Libs_SdCard_Sync(void);

#endif /* LIBS_SDCARD_LIBS_SDCARD_H_ */
