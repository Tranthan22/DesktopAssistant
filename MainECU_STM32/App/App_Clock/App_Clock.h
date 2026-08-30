//-----------------------------------
//	App_Clock - app mac dinh: gio / lich / thoi tiet (du lieu tu ESP qua Libs_EspLink)
//-----------------------------------
//	- Gio: soft RTC (moc NTP tu ESP + tick). Chua sync thi hien "--:--:--".
//	- Thoi tiet: ban tin WEATHER moi nhat tu ESP.
//	- TODO(photo): anh nen tu the SD theo SRS (can Libs_Video_DrawFrame + file anh).
//-----------------------------------

#ifndef APP_APP_CLOCK_APP_CLOCK_H_
#define APP_APP_CLOCK_APP_CLOCK_H_

// /************************************************************************************************************
//  * INCLUDES
//  ************************************************************************************************************/
#include <stdint.h>

// /************************************************************************************************************
//  * MACROS AND DEFINES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * TYPEDEFS
//  ************************************************************************************************************/
typedef struct
{
    uint16_t Year;
    uint8_t Month;
    uint8_t Day;
    uint8_t Weekday;    // 0 = Chu nhat
    uint8_t Hour;
    uint8_t Minute;
    uint8_t Second;
} App_Clock_Civil_st;

// /************************************************************************************************************
//  * EXPORTED VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * GLOBAL FUNCTION PROTOTYPES
//  ************************************************************************************************************/
void App_Clock_Init(void);
void App_Clock_Run(void);

// Doi epoch (da cong mui gio) -> ngay gio lich. Public de test tren host va tai su dung
void App_Clock_EpochToCivil(uint32_t p_Epoch_u32, App_Clock_Civil_st* p_Out_st);

#endif /* APP_APP_CLOCK_APP_CLOCK_H_ */
