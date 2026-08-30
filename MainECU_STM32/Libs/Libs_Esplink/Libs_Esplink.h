//-----------------------------------
//	Libs_EspLink - nhan/tra lenh tu ESP8266 Station qua UART (phia slave)
//-----------------------------------
//	Protocol chung 2 phia: StationLinkCore.h (frame 0xA5 + CRC16 + ACK|0x80).
//	Vai tro:
//	- Nhan frame tu Station, tu tra ACK (PING/SELECT_APP/TIME_SYNC/WEATHER/FILE_*/GET_STATUS)
//	- Giu "soft RTC": epoch local = moc sync NTP + thoi gian troi (Libs_System_GetTick)
//	- Giu du lieu thoi tiet moi nhat
//	- Chuyen file (FILE_START/DATA/END) qua callback do tang App dang ky
//	  (Libs khong duoc goi FatFs truc tiep theo convention)
//
//	Cach dung (App_Manager da goi san):
//		Libs_EspLink_Init();
//		while(1) { Libs_EspLink_Process(); ... }
//-----------------------------------

#ifndef LIBS_ESPLINK_LIBS_ESPLINK_H_
#define LIBS_ESPLINK_LIBS_ESPLINK_H_

// /************************************************************************************************************
//  * INCLUDES
//  ************************************************************************************************************/
#include <stdint.h>
#include "StationLinkCore.h"

// /************************************************************************************************************
//  * MACROS AND DEFINES
//  ************************************************************************************************************/
#define ESPLINK_WEATHER_DESC_MAX                32

// /************************************************************************************************************
//  * TYPEDEFS
//  ************************************************************************************************************/
typedef struct
{
    int16_t TempCx10;                           // nhiet do x10 (305 = 30.5 do C)
    uint8_t Humidity;                           // %
    char Description[ESPLINK_WEATHER_DESC_MAX];
    uint8_t Valid;                              // 1 = da nhan it nhat 1 lan
} Libs_EspLink_Weather_st;

// Callback nhan file (tang App implement, tra ve 0 = OK de ACK cho ESP)
typedef uint8_t (*Libs_EspLink_FileStart_fp)(uint32_t p_Size_u32, uint8_t p_Type_u8, const char* p_Name_char);
typedef uint8_t (*Libs_EspLink_FileData_fp)(uint32_t p_Offset_u32, const uint8_t* p_Data_u8, uint16_t p_Len_u16);
typedef uint8_t (*Libs_EspLink_FileEnd_fp)(uint32_t p_Crc32_u32);

// /************************************************************************************************************
//  * EXPORTED VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * GLOBAL FUNCTION PROTOTYPES
//  ************************************************************************************************************/
void Libs_EspLink_Init(void);
void Libs_EspLink_Process(void);                // goi moi vong lap chinh: drain UART + xu ly frame + tra ACK

// Yeu cau doi app tu Station: tra ve 1 neu co yeu cau moi (doc xong tu xoa co)
uint8_t Libs_EspLink_TakeAppRequest(uint8_t* p_AppId_u8);
void Libs_EspLink_SetCurrentApp(uint8_t p_AppId_u8);        // cap nhat cho GET_STATUS

uint8_t Libs_EspLink_IsTimeValid(void);
uint32_t Libs_EspLink_GetLocalEpoch(void);      // epoch DA cong mui gio (soft RTC chay bang tick)

const Libs_EspLink_Weather_st* Libs_EspLink_GetWeather(void);

void Libs_EspLink_SetFileCallbacks(Libs_EspLink_FileStart_fp p_Start_fp,
                                   Libs_EspLink_FileData_fp p_Data_fp,
                                   Libs_EspLink_FileEnd_fp p_End_fp);

#endif /* LIBS_ESPLINK_LIBS_ESPLINK_H_ */
