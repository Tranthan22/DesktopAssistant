//	*** BAN SAO DONG BO tu Station/src/Link/StationLinkCore.h - SUA O CA HAI NOI, giu byte-identical phan logic ***
//-----------------------------------
//	StationLinkCore - khung truyen UART tin cay (phan thuan logic, khong dinh Arduino)
//-----------------------------------
//	Frame: [SOF 0xA5][SEQ][CMD][LEN_L][LEN_H][payload...][CRC_H][CRC_L]
//	- CRC16-CCITT (poly 0x1021, init 0xFFFF) tinh tren SEQ..payload
//	- Ben nhan tra ACK: frame cung SEQ, CMD = CMD_goc | 0x80, payload[0] = status (0 = OK)
//	- Phia STM32 se implement dung dinh dang nay (Libs_EspLink - TBD)
//-----------------------------------

#ifndef STATION_LINK_CORE_H
#define STATION_LINK_CORE_H

#include <stdint.h>

#define LINK_SOF                    0xA5
#define LINK_MAX_PAYLOAD            512u
#define LINK_FRAME_OVERHEAD         7u                                  // SOF+SEQ+CMD+LEN2+CRC2
#define LINK_MAX_FRAME              (LINK_MAX_PAYLOAD + LINK_FRAME_OVERHEAD)
#define LINK_ACK_FLAG               0x80

// Commands (ESP -> STM32). ACK = cmd | LINK_ACK_FLAG
#define LINK_CMD_PING               0x01
#define LINK_CMD_SELECT_APP         0x02    // [app_id u8]
#define LINK_CMD_TIME_SYNC          0x03    // [epoch u32 LE][tz_offset_min i16 LE]
#define LINK_CMD_WEATHER            0x04    // [temp_c_x10 i16 LE][humidity u8][desc zstr]
#define LINK_CMD_FILE_START         0x10    // [size u32 LE][type u8][name zstr]
#define LINK_CMD_FILE_DATA          0x11    // [offset u32 LE][data...]
#define LINK_CMD_FILE_END           0x12    // [crc32 u32 LE]
#define LINK_CMD_GET_STATUS         0x20    // ACK payload: [status u8][app u8][sd_ok u8]

// App id (phai khop voi STM32)
#define LINK_APP_CLOCK              0
#define LINK_APP_VIDEO              1
#define LINK_APP_HWTEST             2

// File type cho FILE_START
#define LINK_FILE_TYPE_IMAGE        0
#define LINK_FILE_TYPE_VIDEO        1
#define LINK_FILE_TYPE_OTHER        2

typedef struct
{
    uint8_t State;
    uint8_t Seq;
    uint8_t Cmd;
    uint16_t Len;
    uint16_t Index;
    uint16_t Crc;
    uint8_t Payload[LINK_MAX_PAYLOAD];
} LinkParser;

#ifdef __cplusplus
extern "C" {
#endif

uint16_t Link_Crc16(const uint8_t* p_Data, uint16_t p_Len);
// Dong goi 1 frame vao p_Out (>= LINK_MAX_FRAME byte). Tra ve do dai frame, 0 neu payload qua lon
uint16_t Link_BuildFrame(uint8_t* p_Out, uint8_t p_Seq, uint8_t p_Cmd, const uint8_t* p_Payload, uint16_t p_Len);
void Link_ParserReset(LinkParser* p_Parser);
// Nap tung byte nhan duoc. Tra ve 1 khi co frame hop le (CRC dung) trong parser
uint8_t Link_ParserFeed(LinkParser* p_Parser, uint8_t p_Byte);

#ifdef __cplusplus
}
#endif

#endif // STATION_LINK_CORE_H
