//-----------------------------------
//	SD card driver (SPI mode)
//-----------------------------------
//	Xem mo ta trong Libs_SdCard.h. Protocol tham khao chuan SD Physical Layer
//	Simplified Spec (SPI mode) va thu vien mau cua ChaN (FatFs).
//-----------------------------------

#include "main.h"
#include "Libs_SdCard.h"
#include "Libs_System.h"
#include "Wrappers_Spi.h"
#include "Wrappers_Gpio.h"

// /************************************************************************************************************
//  * EXTERN VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * PRIVATE MACROS AND DEFINES
//  ************************************************************************************************************/
//SD commands (SPI mode). Bit 0x80 = ACMD (tu dong gui CMD55 truoc)
#define SDCARD_CMD0_GO_IDLE                     0
#define SDCARD_CMD1_SEND_OP_COND                1
#define SDCARD_CMD8_SEND_IF_COND                8
#define SDCARD_CMD9_SEND_CSD                    9
#define SDCARD_CMD16_SET_BLOCKLEN               16
#define SDCARD_CMD17_READ_SINGLE                17
#define SDCARD_CMD24_WRITE_SINGLE               24
#define SDCARD_CMD55_APP_CMD                    55
#define SDCARD_CMD58_READ_OCR                   58
#define SDCARD_ACMD41_SD_OP_COND                (0x80 | 41)

//Card type flags
#define SDCARD_TYPE_MMC                         0x01
#define SDCARD_TYPE_SD1                         0x02
#define SDCARD_TYPE_SD2                         0x04
#define SDCARD_TYPE_BLOCK                       0x08	// dia chi theo block (SDHC/SDXC), khong theo byte

//Timeouts (ms)
#define SDCARD_INIT_TIMEOUT_MS                  1000
#define SDCARD_READY_TIMEOUT_MS                 500
#define SDCARD_READ_TOKEN_TIMEOUT_MS            200
#define SDCARD_WRITE_TIMEOUT_MS                 500

// /************************************************************************************************************
//  * PRIVATE TYPEDEFS
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * STATIC VARIABLES
//  ************************************************************************************************************/
static uint8_t l_Initialized_u8 = 0;
static uint8_t l_CardType_u8 = 0;
static uint8_t l_SpiChannel_u8 = SPI_SdCardInit;	// SPI_SdCardInit khi init, SPI_SdCard khi chay
static uint8_t l_DummyTx_u8[SDCARD_BLOCK_SIZE];		// toan 0xFF, de clock du lieu ra tu the

// /************************************************************************************************************
//  * STATIC FUNCTION PROTOTYPES
//  ************************************************************************************************************/
static uint8_t Libs_SdCard_SpiByte(uint8_t p_TxByte_u8);
static uint8_t Libs_SdCard_WaitReady(uint32_t p_TimeoutMs_u32);
static uint8_t Libs_SdCard_Select(void);
static void Libs_SdCard_Deselect(void);
static uint8_t Libs_SdCard_SendCommand(uint8_t p_Command_u8, uint32_t p_Argument_u32);
static uint8_t Libs_SdCard_ReadDataBlock(uint8_t* p_Buffer_u8, uint16_t p_Length_u16);
static uint8_t Libs_SdCard_WriteDataBlock(const uint8_t* p_Buffer_u8);

// /************************************************************************************************************
//  * STATIC FUNCTIONS
//  ************************************************************************************************************/
/* Trao doi 1 byte full-duplex tren kenh SPI hien hanh */
static uint8_t Libs_SdCard_SpiByte(uint8_t p_TxByte_u8)
{
    uint8_t f_RxByte_u8 = 0xFF;

    Wrappers_Spi_TransmitReceive(l_SpiChannel_u8, &p_TxByte_u8, &f_RxByte_u8, 1);
    return f_RxByte_u8;
}

/* Cho the het busy (MISO tra ve 0xFF). Tra ve 1 = san sang */
static uint8_t Libs_SdCard_WaitReady(uint32_t p_TimeoutMs_u32)
{
    uint32_t f_StartTick_u32 = Libs_System_GetTick();

    do
    {
        if (Libs_SdCard_SpiByte(0xFF) == 0xFF)
        {
            return 1;
        }
    } while ((Libs_System_GetTick() - f_StartTick_u32) < p_TimeoutMs_u32);

    return 0;
}

/* Keo CS xuong va cho the san sang. Tra ve 1 = OK, 0 = timeout (da tu nha CS) */
static uint8_t Libs_SdCard_Select(void)
{
    Wrappers_Gpio_Write(LogicalChannel_7, 0);
    (void)Libs_SdCard_SpiByte(0xFF);				// 1 byte dem sau khi doi CS

    if (Libs_SdCard_WaitReady(SDCARD_READY_TIMEOUT_MS))
    {
        return 1;
    }

    Libs_SdCard_Deselect();
    return 0;
}

/* Nha CS + 1 byte clock de the nha MISO */
static void Libs_SdCard_Deselect(void)
{
    Wrappers_Gpio_Write(LogicalChannel_7, 1);
    (void)Libs_SdCard_SpiByte(0xFF);
}

/* Gui 1 lenh, tra ve R1 (0xFF = loi/khong phan hoi). Giu nguyen CS de doc payload sau lenh */
static uint8_t Libs_SdCard_SendCommand(uint8_t p_Command_u8, uint32_t p_Argument_u32)
{
    uint8_t f_Response_u8 = 0xFF;
    uint8_t f_Crc_u8 = 0x01;
    uint8_t f_Retry_u8 = 10;

    if (p_Command_u8 & 0x80)
    {
        /* ACMD<n>: gui CMD55 truoc */
        p_Command_u8 &= 0x7F;
        f_Response_u8 = Libs_SdCard_SendCommand(SDCARD_CMD55_APP_CMD, 0);
        if (f_Response_u8 > 1)
        {
            return f_Response_u8;
        }
    }

    /* Chon lai the truoc moi lenh */
    Libs_SdCard_Deselect();
    if (!Libs_SdCard_Select())
    {
        return 0xFF;
    }

    (void)Libs_SdCard_SpiByte(0x40 | p_Command_u8);
    (void)Libs_SdCard_SpiByte((uint8_t)(p_Argument_u32 >> 24));
    (void)Libs_SdCard_SpiByte((uint8_t)(p_Argument_u32 >> 16));
    (void)Libs_SdCard_SpiByte((uint8_t)(p_Argument_u32 >> 8));
    (void)Libs_SdCard_SpiByte((uint8_t)p_Argument_u32);

    /* CRC chi bat buoc cho CMD0/CMD8 (truoc khi vao SPI mode) */
    if (p_Command_u8 == SDCARD_CMD0_GO_IDLE)
    {
        f_Crc_u8 = 0x95;
    }
    if (p_Command_u8 == SDCARD_CMD8_SEND_IF_COND)
    {
        f_Crc_u8 = 0x87;
    }
    (void)Libs_SdCard_SpiByte(f_Crc_u8);

    /* Doi R1: bit 7 = 0 */
    do
    {
        f_Response_u8 = Libs_SdCard_SpiByte(0xFF);
    } while ((f_Response_u8 & 0x80) && (--f_Retry_u8));

    return f_Response_u8;
}

/* Doc 1 data block: cho token 0xFE roi nhan p_Length_u16 byte + 2 byte CRC. Tra ve 1 = OK */
static uint8_t Libs_SdCard_ReadDataBlock(uint8_t* p_Buffer_u8, uint16_t p_Length_u16)
{
    uint8_t f_Token_u8 = 0xFF;
    uint32_t f_StartTick_u32 = Libs_System_GetTick();

    do
    {
        f_Token_u8 = Libs_SdCard_SpiByte(0xFF);
    } while ((f_Token_u8 == 0xFF) && ((Libs_System_GetTick() - f_StartTick_u32) < SDCARD_READ_TOKEN_TIMEOUT_MS));

    if (f_Token_u8 != 0xFE)
    {
        return 0;
    }

    Wrappers_Spi_TransmitReceive(l_SpiChannel_u8, l_DummyTx_u8, p_Buffer_u8, p_Length_u16);
    (void)Libs_SdCard_SpiByte(0xFF);	// bo 2 byte CRC
    (void)Libs_SdCard_SpiByte(0xFF);

    return 1;
}

/* Ghi 1 data block 512 byte voi token 0xFE. Tra ve 1 = the nhan du lieu */
static uint8_t Libs_SdCard_WriteDataBlock(const uint8_t* p_Buffer_u8)
{
    uint8_t f_Response_u8 = 0;

    if (!Libs_SdCard_WaitReady(SDCARD_WRITE_TIMEOUT_MS))
    {
        return 0;
    }

    (void)Libs_SdCard_SpiByte(0xFE);				// data token
    Wrappers_Spi_Transmit(l_SpiChannel_u8, (uint8_t*)p_Buffer_u8, SDCARD_BLOCK_SIZE);
    (void)Libs_SdCard_SpiByte(0xFF);				// CRC gia (khong dung o SPI mode)
    (void)Libs_SdCard_SpiByte(0xFF);

    f_Response_u8 = Libs_SdCard_SpiByte(0xFF);		// data response: xxx00101 = accepted
    return ((f_Response_u8 & 0x1F) == 0x05) ? 1 : 0;
}

// ************************************************************************************************************
//  * GLOBAL FUNCTIONS
//  ************************************************************************************************************/
/* Nhan dien va khoi tao the (SDv2/SDHC, SDv1, MMC). Tra ve SDCARD_OK/SDCARD_ERROR */
uint8_t Libs_SdCard_Init(void)
{
    uint8_t f_CardType_u8 = 0;
    uint8_t f_OcrBuffer_u8[4] = {0};
    uint8_t f_Timeout_u8 = 1;
    uint8_t f_Command_u8 = 0;
    uint32_t f_StartTick_u32 = 0;
    uint16_t f_Index_u16 = 0;

    l_Initialized_u8 = 0;
    l_CardType_u8 = 0;
    l_SpiChannel_u8 = SPI_SdCardInit;

    for (f_Index_u16 = 0; f_Index_u16 < SDCARD_BLOCK_SIZE; f_Index_u16++)
    {
        l_DummyTx_u8[f_Index_u16] = 0xFF;
    }

    /* >=74 xung clock voi CS nha de the vao SPI mode */
    Wrappers_Gpio_Write(LogicalChannel_7, 1);
    for (f_Index_u16 = 0; f_Index_u16 < 10; f_Index_u16++)
    {
        (void)Libs_SdCard_SpiByte(0xFF);
    }

    if (Libs_SdCard_SendCommand(SDCARD_CMD0_GO_IDLE, 0) == 1)
    {
        f_StartTick_u32 = Libs_System_GetTick();

        if (Libs_SdCard_SendCommand(SDCARD_CMD8_SEND_IF_COND, 0x000001AA) == 1)
        {
            /* SDv2: doc R7, kiem tra echo (2.7-3.6V + pattern 0xAA) */
            for (f_Index_u16 = 0; f_Index_u16 < 4; f_Index_u16++)
            {
                f_OcrBuffer_u8[f_Index_u16] = Libs_SdCard_SpiByte(0xFF);
            }

            if ((f_OcrBuffer_u8[2] == 0x01) && (f_OcrBuffer_u8[3] == 0xAA))
            {
                /* ACMD41 voi HCS=1 den khi the roi trang thai idle */
                while ((Libs_System_GetTick() - f_StartTick_u32) < SDCARD_INIT_TIMEOUT_MS)
                {
                    if (Libs_SdCard_SendCommand(SDCARD_ACMD41_SD_OP_COND, 1uL << 30) == 0)
                    {
                        f_Timeout_u8 = 0;
                        break;
                    }
                }

                if ((f_Timeout_u8 == 0) && (Libs_SdCard_SendCommand(SDCARD_CMD58_READ_OCR, 0) == 0))
                {
                    /* CCS bit trong OCR: 1 = SDHC/SDXC (dia chi theo block) */
                    for (f_Index_u16 = 0; f_Index_u16 < 4; f_Index_u16++)
                    {
                        f_OcrBuffer_u8[f_Index_u16] = Libs_SdCard_SpiByte(0xFF);
                    }
                    f_CardType_u8 = (f_OcrBuffer_u8[0] & 0x40) ? (SDCARD_TYPE_SD2 | SDCARD_TYPE_BLOCK) : SDCARD_TYPE_SD2;
                }
            }
        }
        else
        {
            /* SDv1 hoac MMC */
            if (Libs_SdCard_SendCommand(SDCARD_ACMD41_SD_OP_COND, 0) <= 1)
            {
                f_CardType_u8 = SDCARD_TYPE_SD1;
                f_Command_u8 = SDCARD_ACMD41_SD_OP_COND;
            }
            else
            {
                f_CardType_u8 = SDCARD_TYPE_MMC;
                f_Command_u8 = SDCARD_CMD1_SEND_OP_COND;
            }

            while ((Libs_System_GetTick() - f_StartTick_u32) < SDCARD_INIT_TIMEOUT_MS)
            {
                if (Libs_SdCard_SendCommand(f_Command_u8, 0) == 0)
                {
                    f_Timeout_u8 = 0;
                    break;
                }
            }

            /* The cu dia chi theo byte: ep block length 512 */
            if ((f_Timeout_u8 != 0) || (Libs_SdCard_SendCommand(SDCARD_CMD16_SET_BLOCKLEN, SDCARD_BLOCK_SIZE) != 0))
            {
                f_CardType_u8 = 0;
            }
        }
    }

    l_CardType_u8 = f_CardType_u8;
    Libs_SdCard_Deselect();

    if (f_CardType_u8 != 0)
    {
        l_SpiChannel_u8 = SPI_SdCard;		// init xong: chuyen sang full speed
        l_Initialized_u8 = 1;
        return SDCARD_OK;
    }

    return SDCARD_ERROR;
}

uint8_t Libs_SdCard_IsInitialized(void)
{
    return l_Initialized_u8;
}

/* Doc p_Count_u32 block 512 byte tu p_Sector_u32 (CMD17 lap tung block) */
uint8_t Libs_SdCard_ReadBlocks(uint8_t* p_Buffer_u8, uint32_t p_Sector_u32, uint32_t p_Count_u32)
{
    uint32_t f_Address_u32 = 0;

    if (!l_Initialized_u8)
    {
        return SDCARD_ERROR;
    }

    while (p_Count_u32 > 0)
    {
        f_Address_u32 = (l_CardType_u8 & SDCARD_TYPE_BLOCK) ? p_Sector_u32 : (p_Sector_u32 * SDCARD_BLOCK_SIZE);

        if (Libs_SdCard_SendCommand(SDCARD_CMD17_READ_SINGLE, f_Address_u32) != 0)
        {
            break;
        }
        if (!Libs_SdCard_ReadDataBlock(p_Buffer_u8, SDCARD_BLOCK_SIZE))
        {
            break;
        }

        p_Buffer_u8 += SDCARD_BLOCK_SIZE;
        p_Sector_u32++;
        p_Count_u32--;
    }

    Libs_SdCard_Deselect();
    return (p_Count_u32 == 0) ? SDCARD_OK : SDCARD_ERROR;
}

/* Ghi p_Count_u32 block 512 byte vao p_Sector_u32 (CMD24 lap tung block) */
uint8_t Libs_SdCard_WriteBlocks(const uint8_t* p_Buffer_u8, uint32_t p_Sector_u32, uint32_t p_Count_u32)
{
    uint32_t f_Address_u32 = 0;

    if (!l_Initialized_u8)
    {
        return SDCARD_ERROR;
    }

    while (p_Count_u32 > 0)
    {
        f_Address_u32 = (l_CardType_u8 & SDCARD_TYPE_BLOCK) ? p_Sector_u32 : (p_Sector_u32 * SDCARD_BLOCK_SIZE);

        if (Libs_SdCard_SendCommand(SDCARD_CMD24_WRITE_SINGLE, f_Address_u32) != 0)
        {
            break;
        }
        if (!Libs_SdCard_WriteDataBlock(p_Buffer_u8))
        {
            break;
        }

        p_Buffer_u8 += SDCARD_BLOCK_SIZE;
        p_Sector_u32++;
        p_Count_u32--;
    }

    /* Cho the ghi xong (het busy) truoc khi nha bus */
    (void)Libs_SdCard_WaitReady(SDCARD_WRITE_TIMEOUT_MS);
    Libs_SdCard_Deselect();
    return (p_Count_u32 == 0) ? SDCARD_OK : SDCARD_ERROR;
}

/* Tong so sector 512 byte cua the (doc tu CSD). Tra ve 0 neu loi */
uint32_t Libs_SdCard_GetSectorCount(void)
{
    uint8_t f_Csd_u8[16] = {0};
    uint32_t f_SectorCount_u32 = 0;
    uint32_t f_CSize_u32 = 0;
    uint8_t f_Shift_u8 = 0;

    if (!l_Initialized_u8)
    {
        return 0;
    }

    if ((Libs_SdCard_SendCommand(SDCARD_CMD9_SEND_CSD, 0) == 0) && Libs_SdCard_ReadDataBlock(f_Csd_u8, 16))
    {
        if ((f_Csd_u8[0] >> 6) == 1)
        {
            /* CSD v2 (SDHC/SDXC): dung luong = (C_SIZE + 1) * 512KB */
            f_CSize_u32 = ((uint32_t)(f_Csd_u8[7] & 0x3F) << 16) | ((uint32_t)f_Csd_u8[8] << 8) | f_Csd_u8[9];
            f_SectorCount_u32 = (f_CSize_u32 + 1) << 10;
        }
        else
        {
            /* CSD v1: dung luong = (C_SIZE+1) * 2^(C_SIZE_MULT+2) * 2^READ_BL_LEN byte */
            f_Shift_u8 = (uint8_t)((f_Csd_u8[5] & 0x0F) + ((f_Csd_u8[10] >> 7) | ((f_Csd_u8[9] & 0x03) << 1)) + 2 - 9);
            f_CSize_u32 = ((uint32_t)(f_Csd_u8[6] & 0x03) << 10) | ((uint32_t)f_Csd_u8[7] << 2) | (f_Csd_u8[8] >> 6);
            f_SectorCount_u32 = (f_CSize_u32 + 1) << f_Shift_u8;
        }
    }

    Libs_SdCard_Deselect();
    return f_SectorCount_u32;
}

/* Cho moi thao tac ghi truoc do hoan tat (CTRL_SYNC cua FatFs) */
uint8_t Libs_SdCard_Sync(void)
{
    uint8_t f_Result_u8 = SDCARD_ERROR;

    if (!l_Initialized_u8)
    {
        return SDCARD_ERROR;
    }

    if (Libs_SdCard_Select())
    {
        f_Result_u8 = SDCARD_OK;
    }
    Libs_SdCard_Deselect();

    return f_Result_u8;
}
