//-----------------------------------
//	App_HwTest - man hinh kiem tra phan cung
//-----------------------------------
//	Xem mo ta trong App_HwTest.h
//-----------------------------------

#include "App_HwTest.h"
#include "fatfs.h"
#include "Libs_System.h"
#include "Libs_ILI9341_Driver.h"
#include "Libs_ILI9341_GFX.h"
#include "Libs_XPT2046_Driver.h"

#include <stdio.h>
#include <string.h>

// /************************************************************************************************************
//  * EXTERN VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * PRIVATE MACROS AND DEFINES
//  ************************************************************************************************************/
//Man hinh dat o VERTICAL_1 (240x320) de khop voi calibration cua cam ung
#define HWTEST_COLOUR_BG                        ILI9341_BLACK
#define HWTEST_COLOUR_LABEL                     ILI9341_WHITE
#define HWTEST_COLOUR_OK                        ILI9341_GREEN
#define HWTEST_COLOUR_FAIL                      ILI9341_RED
#define HWTEST_COLOUR_VALUE                     ILI9341_YELLOW
#define HWTEST_COLOUR_PAINT                     ILI9341_GREENYELLOW

//Toa do cac dong trang thai (text size 2 = cao 16px, toi da 20 ky tu/dong)
#define HWTEST_TEXT_SIZE                        2
#define HWTEST_LINE_SCREEN_Y                    48
#define HWTEST_LINE_SD_Y                        72
#define HWTEST_LINE_SDRW_Y                      96
#define HWTEST_LINE_SDSIZE_Y                    120
#define HWTEST_LINE_TOUCH_Y                     144
#define HWTEST_LINE_COORD_Y                     166

//Khung ve cam ung
#define HWTEST_PAINT_X0                         4
#define HWTEST_PAINT_Y0                         190
#define HWTEST_PAINT_X1                         235
#define HWTEST_PAINT_Y1                         315
#define HWTEST_PAINT_DOT_RADIUS                 3

// /************************************************************************************************************
//  * PRIVATE TYPEDEFS
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * STATIC VARIABLES
//  ************************************************************************************************************/
static uint8_t l_TouchSeen_u8 = 0;
static uint16_t l_LastX_u16 = 0xFFFF;
static uint16_t l_LastY_u16 = 0xFFFF;

// /************************************************************************************************************
//  * GLOBAL VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * STATIC FUNCTION PROTOTYPES
//  ************************************************************************************************************/
static void App_HwTest_DrawLine(uint16_t p_Y_u16, const char* p_Text_char, uint16_t p_Colour_u16);
static void App_HwTest_SdTest(void);

// /************************************************************************************************************
//  * STATIC FUNCTIONS
//  ************************************************************************************************************/
/* Ve 1 dong trang thai: xoa het dong cu roi ve chu moi */
static void App_HwTest_DrawLine(uint16_t p_Y_u16, const char* p_Text_char, uint16_t p_Colour_u16)
{
    Libs_ILI9341_DrawFilledRectangleCoord(0, p_Y_u16, 239, p_Y_u16 + (8 * HWTEST_TEXT_SIZE), HWTEST_COLOUR_BG);
    Libs_ILI9341_DrawText(p_Text_char, 4, p_Y_u16, p_Colour_u16, HWTEST_TEXT_SIZE, HWTEST_COLOUR_BG);
}

/* Test the SD qua FatFs: mount -> ghi file -> doc lai so sanh -> tinh dung luong */
static void App_HwTest_SdTest(void)
{
    static const char f_TestData_char[] = "DesktopAssistant SD test";
    FRESULT f_Result;
    UINT f_ByteCount = 0;
    DWORD f_FreeClusters = 0;
    FATFS* f_Fs_ptr = 0;
    DWORD f_TotalMb_u32 = 0;
    uint8_t f_RwOk_u8 = 0;
    char f_TextBuffer_char[24];
    char f_ReadBuffer_char[sizeof(f_TestData_char)];

    /* 1. Mount */
    App_HwTest_DrawLine(HWTEST_LINE_SD_Y, "SD CARD: ...", HWTEST_COLOUR_LABEL);
    f_Result = f_mount(&USERFatFS, USERPath, 1);
    if (f_Result != FR_OK)
    {
        /* E3 = khong co the / SPI loi, E13 = the OK nhung chua format FAT */
        snprintf(f_TextBuffer_char, sizeof(f_TextBuffer_char), "SD CARD: FAIL E%d", (int)f_Result);
        App_HwTest_DrawLine(HWTEST_LINE_SD_Y, f_TextBuffer_char, HWTEST_COLOUR_FAIL);
        App_HwTest_DrawLine(HWTEST_LINE_SDRW_Y, "SD R/W : SKIP", HWTEST_COLOUR_LABEL);
        App_HwTest_DrawLine(HWTEST_LINE_SDSIZE_Y, "SD SIZE: --", HWTEST_COLOUR_LABEL);
        return;
    }
    App_HwTest_DrawLine(HWTEST_LINE_SD_Y, "SD CARD: OK", HWTEST_COLOUR_OK);

    /* 2. Ghi file roi doc lai so sanh */
    App_HwTest_DrawLine(HWTEST_LINE_SDRW_Y, "SD R/W : ...", HWTEST_COLOUR_LABEL);
    if (f_open(&USERFile, "HWTEST.TXT", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
    {
        f_Result = f_write(&USERFile, f_TestData_char, sizeof(f_TestData_char), &f_ByteCount);
        f_close(&USERFile);

        if ((f_Result == FR_OK) && (f_ByteCount == sizeof(f_TestData_char))
         && (f_open(&USERFile, "HWTEST.TXT", FA_READ) == FR_OK))
        {
            f_Result = f_read(&USERFile, f_ReadBuffer_char, sizeof(f_ReadBuffer_char), &f_ByteCount);
            f_close(&USERFile);

            if ((f_Result == FR_OK) && (f_ByteCount == sizeof(f_TestData_char))
             && (memcmp(f_ReadBuffer_char, f_TestData_char, sizeof(f_TestData_char)) == 0))
            {
                f_RwOk_u8 = 1;
            }
        }
    }
    if (f_RwOk_u8)
    {
        App_HwTest_DrawLine(HWTEST_LINE_SDRW_Y, "SD R/W : OK", HWTEST_COLOUR_OK);
    }
    else
    {
        App_HwTest_DrawLine(HWTEST_LINE_SDRW_Y, "SD R/W : FAIL", HWTEST_COLOUR_FAIL);
    }

    /* 3. Dung luong volume (tu bang FAT) */
    if (f_getfree(USERPath, &f_FreeClusters, &f_Fs_ptr) == FR_OK)
    {
        f_TotalMb_u32 = ((f_Fs_ptr->n_fatent - 2) * f_Fs_ptr->csize) / 2048;	// sector 512B -> MB
        snprintf(f_TextBuffer_char, sizeof(f_TextBuffer_char), "SD SIZE: %luMB", (unsigned long)f_TotalMb_u32);
        App_HwTest_DrawLine(HWTEST_LINE_SDSIZE_Y, f_TextBuffer_char, HWTEST_COLOUR_VALUE);
    }
    else
    {
        App_HwTest_DrawLine(HWTEST_LINE_SDSIZE_Y, "SD SIZE: FAIL", HWTEST_COLOUR_FAIL);
    }
}

// ************************************************************************************************************
//  * GLOBAL FUNCTIONS
//  ************************************************************************************************************/
/* Ve UI + chay test man hinh/SD mot lan. Goi sau khi da init screen + touch + FATFS */
void App_HwTest_Init(void)
{
    l_TouchSeen_u8 = 0;
    l_LastX_u16 = 0xFFFF;
    l_LastY_u16 = 0xFFFF;

    /* VERTICAL_1 (240x320) de toa do cam ung khop truc tiep voi man hinh */
    Libs_ILI9341_SetRotation(ILI9341_SCREEN_VERTICAL_1);
    Libs_ILI9341_FillScreen(HWTEST_COLOUR_BG);

    /* Test man hinh: tieu de + 3 o mau RGB - doc duoc va dung mau la dat */
    Libs_ILI9341_DrawText("HW TEST", 57, 8, ILI9341_CYAN, 3, HWTEST_COLOUR_BG);
    Libs_ILI9341_DrawFilledRectangleCoord(57, 34, 97, 42, ILI9341_RED);
    Libs_ILI9341_DrawFilledRectangleCoord(100, 34, 140, 42, ILI9341_GREEN);
    Libs_ILI9341_DrawFilledRectangleCoord(143, 34, 183, 42, ILI9341_BLUE);
    App_HwTest_DrawLine(HWTEST_LINE_SCREEN_Y, "SCREEN : OK", HWTEST_COLOUR_OK);

    /* Test SD (mount + doc/ghi + dung luong) */
    App_HwTest_SdTest();

    /* Khu vuc test cam ung */
    App_HwTest_DrawLine(HWTEST_LINE_TOUCH_Y, "TOUCH : VE O DUOI", HWTEST_COLOUR_LABEL);
    Libs_ILI9341_DrawHollowRectangleCoord(HWTEST_PAINT_X0, HWTEST_PAINT_Y0, HWTEST_PAINT_X1, HWTEST_PAINT_Y1, ILI9341_DARKGREY);
}

/* Goi lien tuc trong while(1): poll cam ung, ve cham + hien toa do */
void App_HwTest_Run(void)
{
    uint16_t f_Position_u16[2] = {0, 0};
    char f_TextBuffer_char[16];

    if (Libs_XPT2046_IsPressed() != XPT2046_PRESSED)
    {
        return;
    }
    if (Libs_XPT2046_ReadCoordinates(f_Position_u16) != XPT2046_DATA_OK)
    {
        return;
    }

    /* Lan cham hop le dau tien: bao TOUCH OK */
    if (!l_TouchSeen_u8)
    {
        App_HwTest_DrawLine(HWTEST_LINE_TOUCH_Y, "TOUCH : OK", HWTEST_COLOUR_OK);
        l_TouchSeen_u8 = 1;
    }

    /* Toa do realtime (chi ve lai khi thay doi de do nhay) */
    if ((f_Position_u16[0] != l_LastX_u16) || (f_Position_u16[1] != l_LastY_u16))
    {
        snprintf(f_TextBuffer_char, sizeof(f_TextBuffer_char), "X=%3u Y=%3u",
                 (unsigned)f_Position_u16[0], (unsigned)f_Position_u16[1]);
        App_HwTest_DrawLine(HWTEST_LINE_COORD_Y, f_TextBuffer_char, HWTEST_COLOUR_VALUE);
        l_LastX_u16 = f_Position_u16[0];
        l_LastY_u16 = f_Position_u16[1];
    }

    /* Ve cham trong khung (giu cham nam gon trong khung) */
    if ((f_Position_u16[0] >= (HWTEST_PAINT_X0 + HWTEST_PAINT_DOT_RADIUS + 1))
     && (f_Position_u16[0] <= (HWTEST_PAINT_X1 - HWTEST_PAINT_DOT_RADIUS - 1))
     && (f_Position_u16[1] >= (HWTEST_PAINT_Y0 + HWTEST_PAINT_DOT_RADIUS + 1))
     && (f_Position_u16[1] <= (HWTEST_PAINT_Y1 - HWTEST_PAINT_DOT_RADIUS - 1)))
    {
        Libs_ILI9341_DrawFilledCircle(f_Position_u16[0], f_Position_u16[1], HWTEST_PAINT_DOT_RADIUS, HWTEST_COLOUR_PAINT);
    }
}
