//-----------------------------------
//	App_Video - implement
//-----------------------------------

#include "App_Video.h"
#include "Libs_System.h"
#include "Libs_Video.h"
#include "Libs_ILI9341_Driver.h"
#include "Libs_ILI9341_GFX.h"

// /************************************************************************************************************
//  * EXTERN VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * PRIVATE MACROS AND DEFINES
//  ************************************************************************************************************/
#define VIDEO_RETRY_PERIOD_MS                   2000u

// /************************************************************************************************************
//  * PRIVATE TYPEDEFS
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * STATIC VARIABLES
//  ************************************************************************************************************/
static Libs_Video_Sd_st l_Video_st;
static uint8_t l_Playing_u8 = 0;
static uint8_t l_ErrorShown_u8 = 0;
static uint32_t l_LastTryMs_u32 = 0;

// /************************************************************************************************************
//  * STATIC FUNCTION PROTOTYPES
//  ************************************************************************************************************/
static void App_Video_ShowError(void);
static void App_Video_TryOpen(void);

// /************************************************************************************************************
//  * STATIC FUNCTIONS
//  ************************************************************************************************************/
/* Man hinh huong dan khi chua co / khong mo duoc file (chi ve 1 lan) */
static void App_Video_ShowError(void)
{
    if (!l_ErrorShown_u8)
    {
        Libs_ILI9341_FillScreen(ILI9341_BLACK);
        Libs_ILI9341_DrawText("KHONG MO DUOC", 82, 90, ILI9341_RED, 2, ILI9341_BLACK);
        Libs_ILI9341_DrawText(APP_VIDEO_FILE_NAME, 106, 120, ILI9341_YELLOW, 2, ILI9341_BLACK);
        Libs_ILI9341_DrawText("UPLOAD FILE .RVD QUA WEB UI CUA STATION", 43, 160, ILI9341_WHITE, 1, ILI9341_BLACK);
        Libs_ILI9341_DrawText("(TAO FILE: video2c.py -f rvd)", 73, 175, ILI9341_DARKGREY, 1, ILI9341_BLACK);
        l_ErrorShown_u8 = 1;
    }
}

/* Mo file va can giua man hinh theo kich thuoc trong header .rvd */
static void App_Video_TryOpen(void)
{
    uint16_t f_ScreenWidth_u16 = 0;
    uint16_t f_ScreenHeight_u16 = 0;

    if (Libs_Video_SdOpen(&l_Video_st, APP_VIDEO_FILE_NAME, 0, 0) == VIDEO_OK)
    {
        f_ScreenWidth_u16 = Libs_ILI9341_GetScreenWidth();
        f_ScreenHeight_u16 = Libs_ILI9341_GetScreenHeight();
        l_Video_st.X = (l_Video_st.Width < f_ScreenWidth_u16) ? (uint16_t)((f_ScreenWidth_u16 - l_Video_st.Width) / 2u) : 0u;
        l_Video_st.Y = (l_Video_st.Height < f_ScreenHeight_u16) ? (uint16_t)((f_ScreenHeight_u16 - l_Video_st.Height) / 2u) : 0u;

        Libs_ILI9341_FillScreen(ILI9341_BLACK);
        l_Playing_u8 = 1;
        l_ErrorShown_u8 = 0;
    }
    else
    {
        App_Video_ShowError();
    }
}

// ************************************************************************************************************
//  * GLOBAL FUNCTIONS
//  ************************************************************************************************************/
void App_Video_Init(void)
{
    Libs_ILI9341_SetRotation(ILI9341_SCREEN_HORIZONTAL_1);
    Libs_ILI9341_FillScreen(ILI9341_BLACK);
    l_Playing_u8 = 0;
    l_ErrorShown_u8 = 0;
    l_LastTryMs_u32 = Libs_System_GetTick();
    App_Video_TryOpen();
}

void App_Video_Run(void)
{
    uint32_t f_Now_u32 = Libs_System_GetTick();
    uint8_t f_Status_u8 = 0;

    if (l_Playing_u8)
    {
        f_Status_u8 = Libs_Video_SdUpdate(&l_Video_st);
        if (f_Status_u8 == VIDEO_END)
        {
            Libs_Video_SdRewind(&l_Video_st);       // phat lap vo han
        }
        else if (f_Status_u8 == VIDEO_ERROR)
        {
            Libs_Video_SdClose(&l_Video_st);
            l_Playing_u8 = 0;
            App_Video_ShowError();
            l_LastTryMs_u32 = f_Now_u32;
        }
        else
        {
            /* VIDEO_OK / VIDEO_WAITING: khong lam gi them */
        }
    }
    else if ((f_Now_u32 - l_LastTryMs_u32) >= VIDEO_RETRY_PERIOD_MS)
    {
        /* Tu thu lai: sau khi user upload file / cam the la tu chay */
        l_LastTryMs_u32 = f_Now_u32;
        App_Video_TryOpen();
    }
}

void App_Video_Deinit(void)
{
    if (l_Playing_u8)
    {
        Libs_Video_SdClose(&l_Video_st);
        l_Playing_u8 = 0;
    }
}
