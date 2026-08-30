//-----------------------------------
//	Libs_EspLink - implement
//-----------------------------------

#include "main.h"
#include "Libs_EspLink.h"
#include "Libs_System.h"
#include "Libs_SdCard.h"
#include "Wrappers_Uart.h"

// /************************************************************************************************************
//  * EXTERN VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * PRIVATE MACROS AND DEFINES
//  ************************************************************************************************************/
#define ESPLINK_ACK_STATUS_OK                   0
#define ESPLINK_ACK_STATUS_FAIL                 1
#define ESPLINK_RX_CHUNK                        32

// /************************************************************************************************************
//  * PRIVATE TYPEDEFS
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * STATIC VARIABLES
//  ************************************************************************************************************/
static LinkParser l_Parser_st;
static uint8_t l_CurrentApp_u8 = LINK_APP_CLOCK;
static uint8_t l_AppRequestPending_u8 = 0;
static uint8_t l_AppRequestId_u8 = 0;

static uint8_t l_TimeValid_u8 = 0;
static uint32_t l_LocalEpochAtSync_u32 = 0;
static uint32_t l_SyncTick_u32 = 0;

static Libs_EspLink_Weather_st l_Weather_st;

static Libs_EspLink_FileStart_fp l_FileStart_fp = 0;
static Libs_EspLink_FileData_fp l_FileData_fp = 0;
static Libs_EspLink_FileEnd_fp l_FileEnd_fp = 0;

// /************************************************************************************************************
//  * STATIC FUNCTION PROTOTYPES
//  ************************************************************************************************************/
static void Libs_EspLink_SendAck(uint8_t p_Seq_u8, uint8_t p_Cmd_u8, const uint8_t* p_Payload_u8, uint16_t p_Len_u16);
static void Libs_EspLink_HandleFrame(const LinkParser* p_Frame_st);

// /************************************************************************************************************
//  * STATIC FUNCTIONS
//  ************************************************************************************************************/
/* Dong goi va gui 1 frame ACK (CMD | 0x80, giu nguyen SEQ cua lenh goc) */
static void Libs_EspLink_SendAck(uint8_t p_Seq_u8, uint8_t p_Cmd_u8, const uint8_t* p_Payload_u8, uint16_t p_Len_u16)
{
    uint8_t f_Frame_u8[LINK_FRAME_OVERHEAD + 8];
    uint16_t f_FrameLen_u16 = 0;

    f_FrameLen_u16 = Link_BuildFrame(f_Frame_u8, p_Seq_u8, (uint8_t)(p_Cmd_u8 | LINK_ACK_FLAG), p_Payload_u8, p_Len_u16);
    if (f_FrameLen_u16 > 0)
    {
        Wrappers_Uart_Transmit(UART_Esp, f_Frame_u8, f_FrameLen_u16);
    }
}

/* Xu ly 1 frame hop le (CRC da dung) va tra ACK tuong ung */
static void Libs_EspLink_HandleFrame(const LinkParser* p_Frame_st)
{
    uint8_t f_Status_u8 = ESPLINK_ACK_STATUS_FAIL;
    uint8_t f_AckPayload_u8[3] = {0};
    uint16_t f_AckLen_u16 = 1;
    uint16_t f_Index_u16 = 0;

    switch (p_Frame_st->Cmd)
    {
    case LINK_CMD_PING:
        f_Status_u8 = ESPLINK_ACK_STATUS_OK;
        break;

    case LINK_CMD_SELECT_APP:
        if ((p_Frame_st->Len >= 1) && (p_Frame_st->Payload[0] <= LINK_APP_HWTEST))
        {
            l_AppRequestId_u8 = p_Frame_st->Payload[0];
            l_AppRequestPending_u8 = 1;
            f_Status_u8 = ESPLINK_ACK_STATUS_OK;
        }
        break;

    case LINK_CMD_TIME_SYNC:
        if (p_Frame_st->Len >= 6)
        {
            uint32_t f_Epoch_u32 = (uint32_t)p_Frame_st->Payload[0]
                                 | ((uint32_t)p_Frame_st->Payload[1] << 8)
                                 | ((uint32_t)p_Frame_st->Payload[2] << 16)
                                 | ((uint32_t)p_Frame_st->Payload[3] << 24);
            int16_t f_TzMin_i16 = (int16_t)((uint16_t)p_Frame_st->Payload[4]
                                 | ((uint16_t)p_Frame_st->Payload[5] << 8));

            l_LocalEpochAtSync_u32 = f_Epoch_u32 + (int32_t)f_TzMin_i16 * 60;
            l_SyncTick_u32 = Libs_System_GetTick();
            l_TimeValid_u8 = 1;
            f_Status_u8 = ESPLINK_ACK_STATUS_OK;
        }
        break;

    case LINK_CMD_WEATHER:
        if (p_Frame_st->Len >= 4)
        {
            l_Weather_st.TempCx10 = (int16_t)((uint16_t)p_Frame_st->Payload[0]
                                  | ((uint16_t)p_Frame_st->Payload[1] << 8));
            l_Weather_st.Humidity = p_Frame_st->Payload[2];
            for (f_Index_u16 = 0; f_Index_u16 < ESPLINK_WEATHER_DESC_MAX - 1; f_Index_u16++)
            {
                if ((3 + f_Index_u16) >= p_Frame_st->Len)
                {
                    break;
                }
                l_Weather_st.Description[f_Index_u16] = (char)p_Frame_st->Payload[3 + f_Index_u16];
                if (l_Weather_st.Description[f_Index_u16] == '\0')
                {
                    break;
                }
            }
            l_Weather_st.Description[(f_Index_u16 < ESPLINK_WEATHER_DESC_MAX) ? f_Index_u16 : (ESPLINK_WEATHER_DESC_MAX - 1)] = '\0';
            l_Weather_st.Valid = 1;
            f_Status_u8 = ESPLINK_ACK_STATUS_OK;
        }
        break;

    case LINK_CMD_FILE_START:
        if ((l_FileStart_fp != 0) && (p_Frame_st->Len >= 6))
        {
            uint32_t f_Size_u32 = (uint32_t)p_Frame_st->Payload[0]
                                | ((uint32_t)p_Frame_st->Payload[1] << 8)
                                | ((uint32_t)p_Frame_st->Payload[2] << 16)
                                | ((uint32_t)p_Frame_st->Payload[3] << 24);
            /* Ten file la zstr bat dau tu byte 5; parser dam bao co '\0' trong payload */
            f_Status_u8 = l_FileStart_fp(f_Size_u32, p_Frame_st->Payload[4], (const char*)&p_Frame_st->Payload[5]);
        }
        break;

    case LINK_CMD_FILE_DATA:
        if ((l_FileData_fp != 0) && (p_Frame_st->Len >= 5))
        {
            uint32_t f_Offset_u32 = (uint32_t)p_Frame_st->Payload[0]
                                  | ((uint32_t)p_Frame_st->Payload[1] << 8)
                                  | ((uint32_t)p_Frame_st->Payload[2] << 16)
                                  | ((uint32_t)p_Frame_st->Payload[3] << 24);
            f_Status_u8 = l_FileData_fp(f_Offset_u32, &p_Frame_st->Payload[4], (uint16_t)(p_Frame_st->Len - 4));
        }
        break;

    case LINK_CMD_FILE_END:
        if ((l_FileEnd_fp != 0) && (p_Frame_st->Len >= 4))
        {
            uint32_t f_Crc_u32 = (uint32_t)p_Frame_st->Payload[0]
                               | ((uint32_t)p_Frame_st->Payload[1] << 8)
                               | ((uint32_t)p_Frame_st->Payload[2] << 16)
                               | ((uint32_t)p_Frame_st->Payload[3] << 24);
            f_Status_u8 = l_FileEnd_fp(f_Crc_u32);
        }
        break;

    case LINK_CMD_GET_STATUS:
        f_Status_u8 = ESPLINK_ACK_STATUS_OK;
        f_AckPayload_u8[1] = l_CurrentApp_u8;
        f_AckPayload_u8[2] = Libs_SdCard_IsInitialized();
        f_AckLen_u16 = 3;
        break;

    default:
        /* Lenh la: van ACK voi status FAIL de ESP khong retry vo ich */
        break;
    }

    f_AckPayload_u8[0] = f_Status_u8;
    Libs_EspLink_SendAck(p_Frame_st->Seq, p_Frame_st->Cmd, f_AckPayload_u8, f_AckLen_u16);
}

// ************************************************************************************************************
//  * GLOBAL FUNCTIONS
//  ************************************************************************************************************/
void Libs_EspLink_Init(void)
{
    Link_ParserReset(&l_Parser_st);
    l_AppRequestPending_u8 = 0;
    l_TimeValid_u8 = 0;
    l_Weather_st.Valid = 0;
    l_Weather_st.Description[0] = '\0';
}

/* Drain UART RX, nap tung byte vao parser, xu ly moi frame hoan chinh */
void Libs_EspLink_Process(void)
{
    uint8_t f_Buffer_u8[ESPLINK_RX_CHUNK];
    uint16_t f_Count_u16 = 0;
    uint16_t f_Index_u16 = 0;

    do
    {
        f_Count_u16 = Wrappers_Uart_Receive(UART_Esp, f_Buffer_u8, sizeof(f_Buffer_u8));
        for (f_Index_u16 = 0; f_Index_u16 < f_Count_u16; f_Index_u16++)
        {
            if (Link_ParserFeed(&l_Parser_st, f_Buffer_u8[f_Index_u16]))
            {
                Libs_EspLink_HandleFrame(&l_Parser_st);
            }
        }
    } while (f_Count_u16 > 0);
}

uint8_t Libs_EspLink_TakeAppRequest(uint8_t* p_AppId_u8)
{
    if (!l_AppRequestPending_u8)
    {
        return 0;
    }
    l_AppRequestPending_u8 = 0;
    *p_AppId_u8 = l_AppRequestId_u8;
    return 1;
}

void Libs_EspLink_SetCurrentApp(uint8_t p_AppId_u8)
{
    l_CurrentApp_u8 = p_AppId_u8;
}

uint8_t Libs_EspLink_IsTimeValid(void)
{
    return l_TimeValid_u8;
}

/* Soft RTC: moc sync + so giay tick da troi. Luu y tick wrap sau ~49.7 ngay:
   phep tru unsigned van cho hieu dung khi qua 1 lan wrap */
uint32_t Libs_EspLink_GetLocalEpoch(void)
{
    return l_LocalEpochAtSync_u32 + ((Libs_System_GetTick() - l_SyncTick_u32) / 1000u);
}

const Libs_EspLink_Weather_st* Libs_EspLink_GetWeather(void)
{
    return &l_Weather_st;
}

void Libs_EspLink_SetFileCallbacks(Libs_EspLink_FileStart_fp p_Start_fp,
                                   Libs_EspLink_FileData_fp p_Data_fp,
                                   Libs_EspLink_FileEnd_fp p_End_fp)
{
    l_FileStart_fp = p_Start_fp;
    l_FileData_fp = p_Data_fp;
    l_FileEnd_fp = p_End_fp;
}
