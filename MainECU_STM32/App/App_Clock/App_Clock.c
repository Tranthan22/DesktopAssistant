//-----------------------------------
//	App_Clock - implement
//-----------------------------------

#include "App_Clock.h"
#include "Libs_System.h"
#include "Libs_EspLink.h"
#include "Libs_ILI9341_Driver.h"
#include "Libs_ILI9341_GFX.h"

#include <stdio.h>
#include <string.h>

// /************************************************************************************************************
//  * EXTERN VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * PRIVATE MACROS AND DEFINES
//  ************************************************************************************************************/
#define CLOCK_UPDATE_PERIOD_MS                  200u

//Man hinh VERTICAL_1 (240x320), font 6x8
#define CLOCK_TIME_Y                            70
#define CLOCK_DATE_Y                            125
#define CLOCK_WEATHER_Y                         185
#define CLOCK_DESC_Y                            225

// /************************************************************************************************************
//  * PRIVATE TYPEDEFS
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * STATIC VARIABLES
//  ************************************************************************************************************/
static const char* const l_WeekdayName_char[7] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };

static uint32_t l_LastUpdateMs_u32 = 0;
static char l_TimeText_char[12];
static char l_DateText_char[20];
static char l_WeatherText_char[14];
static char l_DescText_char[24];

// /************************************************************************************************************
//  * STATIC FUNCTION PROTOTYPES
//  ************************************************************************************************************/
static void App_Clock_DrawIfChanged(const char* p_New_char, char* p_Cache_char, uint16_t p_CacheSize_u16,
                                    uint16_t p_X_u16, uint16_t p_Y_u16, uint16_t p_Colour_u16, uint16_t p_Size_u16);

// /************************************************************************************************************
//  * STATIC FUNCTIONS
//  ************************************************************************************************************/
/* Chi ve lai dong khi noi dung doi (chuoi pad co dinh nen tu de len chu cu) */
static void App_Clock_DrawIfChanged(const char* p_New_char, char* p_Cache_char, uint16_t p_CacheSize_u16,
                                    uint16_t p_X_u16, uint16_t p_Y_u16, uint16_t p_Colour_u16, uint16_t p_Size_u16)
{
    if (strncmp(p_New_char, p_Cache_char, p_CacheSize_u16) != 0)
    {
        strncpy(p_Cache_char, p_New_char, p_CacheSize_u16 - 1);
        p_Cache_char[p_CacheSize_u16 - 1] = '\0';
        Libs_ILI9341_DrawText(p_Cache_char, p_X_u16, p_Y_u16, p_Colour_u16, p_Size_u16, ILI9341_BLACK);
    }
}

// ************************************************************************************************************
//  * GLOBAL FUNCTIONS
//  ************************************************************************************************************/
/* Doi epoch -> lich dan su (thuat toan civil_from_days cua Howard Hinnant) */
void App_Clock_EpochToCivil(uint32_t p_Epoch_u32, App_Clock_Civil_st* p_Out_st)
{
    uint32_t f_Days_u32 = p_Epoch_u32 / 86400u;
    uint32_t f_Secs_u32 = p_Epoch_u32 % 86400u;
    uint32_t f_Z_u32 = f_Days_u32 + 719468u;
    uint32_t f_Era_u32 = f_Z_u32 / 146097u;
    uint32_t f_DayOfEra_u32 = f_Z_u32 - f_Era_u32 * 146097u;
    uint32_t f_YearOfEra_u32 = (f_DayOfEra_u32 - f_DayOfEra_u32 / 1460u
                              + f_DayOfEra_u32 / 36524u - f_DayOfEra_u32 / 146096u) / 365u;
    uint32_t f_DayOfYear_u32 = f_DayOfEra_u32 - (365u * f_YearOfEra_u32 + f_YearOfEra_u32 / 4u - f_YearOfEra_u32 / 100u);
    uint32_t f_MonthPrime_u32 = (5u * f_DayOfYear_u32 + 2u) / 153u;

    p_Out_st->Day = (uint8_t)(f_DayOfYear_u32 - (153u * f_MonthPrime_u32 + 2u) / 5u + 1u);
    p_Out_st->Month = (uint8_t)((f_MonthPrime_u32 < 10u) ? (f_MonthPrime_u32 + 3u) : (f_MonthPrime_u32 - 9u));
    p_Out_st->Year = (uint16_t)(f_YearOfEra_u32 + f_Era_u32 * 400u + ((p_Out_st->Month <= 2u) ? 1u : 0u));
    p_Out_st->Weekday = (uint8_t)((f_Days_u32 + 4u) % 7u);      // 1970-01-01 la thu Nam
    p_Out_st->Hour = (uint8_t)(f_Secs_u32 / 3600u);
    p_Out_st->Minute = (uint8_t)((f_Secs_u32 / 60u) % 60u);
    p_Out_st->Second = (uint8_t)(f_Secs_u32 % 60u);
}

void App_Clock_Init(void)
{
    Libs_ILI9341_SetRotation(ILI9341_SCREEN_VERTICAL_1);
    Libs_ILI9341_FillScreen(ILI9341_BLACK);

    /* Phan tinh (ve 1 lan) */
    Libs_ILI9341_DrawText("DESKTOP ASSISTANT", 18, 14, ILI9341_WHITE, 2, ILI9341_BLACK);
    Libs_ILI9341_DrawHorizontalLine(18, 34, 204, ILI9341_DARKGREY);
    Libs_ILI9341_DrawText("DOI APP QUA WEB UI CUA STATION", 30, 300, ILI9341_DARKGREY, 1, ILI9341_BLACK);

    /* Xoa cache de Run() ve lai toan bo */
    l_TimeText_char[0] = '\0';
    l_DateText_char[0] = '\0';
    l_WeatherText_char[0] = '\0';
    l_DescText_char[0] = '\0';
    l_LastUpdateMs_u32 = 0;
}

void App_Clock_Run(void)
{
    uint32_t f_Now_u32 = Libs_System_GetTick();
    char f_Text_char[24];
    App_Clock_Civil_st f_Civil_st;
    const Libs_EspLink_Weather_st* f_Weather_st = 0;

    if ((f_Now_u32 - l_LastUpdateMs_u32) < CLOCK_UPDATE_PERIOD_MS)
    {
        return;
    }
    l_LastUpdateMs_u32 = f_Now_u32;

    /* Gio + lich (8 va 14 ky tu co dinh) */
    if (Libs_EspLink_IsTimeValid())
    {
        App_Clock_EpochToCivil(Libs_EspLink_GetLocalEpoch(), &f_Civil_st);
        snprintf(f_Text_char, sizeof(f_Text_char), "%02u:%02u:%02u",
                 (unsigned)f_Civil_st.Hour, (unsigned)f_Civil_st.Minute, (unsigned)f_Civil_st.Second);
        App_Clock_DrawIfChanged(f_Text_char, l_TimeText_char, sizeof(l_TimeText_char),
                                24, CLOCK_TIME_Y, ILI9341_CYAN, 4);

        snprintf(f_Text_char, sizeof(f_Text_char), "%s %02u/%02u/%04u",
                 l_WeekdayName_char[f_Civil_st.Weekday % 7u],
                 (unsigned)f_Civil_st.Day, (unsigned)f_Civil_st.Month, (unsigned)f_Civil_st.Year);
        App_Clock_DrawIfChanged(f_Text_char, l_DateText_char, sizeof(l_DateText_char),
                                36, CLOCK_DATE_Y, ILI9341_YELLOW, 2);
    }
    else
    {
        App_Clock_DrawIfChanged("--:--:--", l_TimeText_char, sizeof(l_TimeText_char),
                                24, CLOCK_TIME_Y, ILI9341_CYAN, 4);
        App_Clock_DrawIfChanged("CHO GIO TU ESP", l_DateText_char, sizeof(l_DateText_char),
                                36, CLOCK_DATE_Y, ILI9341_DARKGREY, 2);
    }

    /* Thoi tiet (chuoi pad co dinh 10 va 18 ky tu) */
    f_Weather_st = Libs_EspLink_GetWeather();
    if (f_Weather_st->Valid)
    {
        int16_t f_Temp_i16 = f_Weather_st->TempCx10;
        int16_t f_Frac_i16 = (int16_t)((f_Temp_i16 < 0) ? -(f_Temp_i16 % 10) : (f_Temp_i16 % 10));

        snprintf(f_Text_char, sizeof(f_Text_char), "%d.%dC %u%%   ",
                 (int)(f_Temp_i16 / 10), (int)f_Frac_i16, (unsigned)f_Weather_st->Humidity);
        f_Text_char[10] = '\0';
        App_Clock_DrawIfChanged(f_Text_char, l_WeatherText_char, sizeof(l_WeatherText_char),
                                30, CLOCK_WEATHER_Y, ILI9341_GREEN, 3);

        snprintf(f_Text_char, sizeof(f_Text_char), "%-18.18s", f_Weather_st->Description);
        App_Clock_DrawIfChanged(f_Text_char, l_DescText_char, sizeof(l_DescText_char),
                                12, CLOCK_DESC_Y, ILI9341_WHITE, 2);
    }
    else
    {
        App_Clock_DrawIfChanged("--.-C --%%", l_WeatherText_char, sizeof(l_WeatherText_char),
                                30, CLOCK_WEATHER_Y, ILI9341_DARKGREY, 3);
        App_Clock_DrawIfChanged("CHO THOI TIET...  ", l_DescText_char, sizeof(l_DescText_char),
                                12, CLOCK_DESC_Y, ILI9341_DARKGREY, 2);
    }
}
