//	MIT License - Copyright (c) 2017 Matej Artnak
//	See Libs_XPT2046_Driver.h for the full license text and usage notes.

#include "main.h"
#include "Libs_XPT2046_Driver.h"
#include "Wrappers_Spi.h"
#include "Wrappers_Gpio.h"

// /************************************************************************************************************
//  * EXTERN VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * PRIVATE MACROS AND DEFINES
//  ************************************************************************************************************/
//XPT2046 control bytes: 12bit differential mode, power down between conversions (PENIRQ stays enabled)
#define XPT2046_CMD_READ_Y                      0x90
#define XPT2046_CMD_READ_X                      0xD0

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
static uint16_t Libs_XPT2046_ReadChannel(uint8_t p_Command_u8);

// /************************************************************************************************************
//  * STATIC FUNCTIONS
//  ************************************************************************************************************/
/* One ADC conversion: send command byte then clock out 16 bits.
   Same bit stream as the original bit-banged read, so the calibration math is unchanged. */
static uint16_t Libs_XPT2046_ReadChannel(uint8_t p_Command_u8)
{
    uint8_t f_TxData_u8[3] = {0};
    uint8_t f_RxData_u8[3] = {0};

    f_TxData_u8[0] = p_Command_u8;
    Wrappers_Spi_TransmitReceive(SPI_Touch, f_TxData_u8, f_RxData_u8, 3);

    return (uint16_t)(((uint16_t)f_RxData_u8[1] << 8) | f_RxData_u8[2]);
}

// ************************************************************************************************************
//  * GLOBAL FUNCTIONS
//  ************************************************************************************************************/
/* Deselect the touch controller so it does not disturb the shared SPI1 bus */
void Libs_XPT2046_Init(void)
{
    Wrappers_Gpio_Write(LogicalChannel_5, 1);
}

/* Returns XPT2046_PRESSED while the panel is touched (PENIRQ low) */
uint8_t Libs_XPT2046_IsPressed(void)
{
    if (Wrappers_Gpio_Read(LogicalChannel_6) == 0)
    {
        return XPT2046_PRESSED;
    }
    else
    {
        return XPT2046_NOT_PRESSED;
    }
}

/* Read averaged touch position. p_Coordinates_u16[0] = X, p_Coordinates_u16[1] = Y.
   Returns XPT2046_DATA_OK only if the panel stayed pressed for all samples, else XPT2046_DATA_NOISY. */
uint8_t Libs_XPT2046_ReadCoordinates(uint16_t p_Coordinates_u16[2])
{
    uint16_t f_RawX_u16 = 0;
    uint16_t f_RawY_u16 = 0;
    uint32_t f_SumX_u32 = 0;
    uint32_t f_SumY_u32 = 0;
    uint32_t f_Samples_u32 = XPT2046_POSITION_SAMPLES;
    uint32_t f_CountedSamples_u32 = 0;
    int32_t f_CoordX_i32 = 0;
    int32_t f_CoordY_i32 = 0;

    /* Keep CS low for the whole sampling burst */
    Wrappers_Gpio_Write(LogicalChannel_5, 0);

    while ((f_Samples_u32 > 0) && (Wrappers_Gpio_Read(LogicalChannel_6) == 0))
    {
        f_SumY_u32 += Libs_XPT2046_ReadChannel(XPT2046_CMD_READ_Y);
        f_SumX_u32 += Libs_XPT2046_ReadChannel(XPT2046_CMD_READ_X);
        f_Samples_u32--;
        f_CountedSamples_u32++;
    }

    Wrappers_Gpio_Write(LogicalChannel_5, 1);

    if ((f_CountedSamples_u32 == XPT2046_POSITION_SAMPLES) && (Wrappers_Gpio_Read(LogicalChannel_6) == 0))
    {
        f_RawX_u16 = (uint16_t)(f_SumX_u32 / f_CountedSamples_u32);
        f_RawY_u16 = (uint16_t)(f_SumY_u32 / f_CountedSamples_u32);

        /* 16bit axis inversion, kept identical to the original library's "raw *= -1" */
        f_RawX_u16 = (uint16_t)(0u - f_RawX_u16);
        f_RawY_u16 = (uint16_t)(0u - f_RawY_u16);

        f_CoordX_i32 = (int32_t)(((240 - (int32_t)(f_RawX_u16 / XPT2046_X_TRANSLATION)) - XPT2046_X_OFFSET) * XPT2046_X_MAGNITUDE);
        f_CoordY_i32 = (int32_t)((((int32_t)(f_RawY_u16 / XPT2046_Y_TRANSLATION)) - XPT2046_Y_OFFSET) * XPT2046_Y_MAGNITUDE);

        /* Clamp: with edge touches the calibration math can leave the 240x320 range */
        if (f_CoordX_i32 < 0)   { f_CoordX_i32 = 0; }
        if (f_CoordX_i32 > 239) { f_CoordX_i32 = 239; }
        if (f_CoordY_i32 < 0)   { f_CoordY_i32 = 0; }
        if (f_CoordY_i32 > 319) { f_CoordY_i32 = 319; }

        p_Coordinates_u16[0] = (uint16_t)f_CoordX_i32;
        p_Coordinates_u16[1] = (uint16_t)f_CoordY_i32;

        return XPT2046_DATA_OK;
    }
    else
    {
        p_Coordinates_u16[0] = 0;
        p_Coordinates_u16[1] = 0;

        return XPT2046_DATA_NOISY;
    }
}
