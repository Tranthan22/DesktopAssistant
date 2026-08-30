//-----------------------------------
//	App_Manager - implement
//-----------------------------------

#include "App_Manager.h"
#include "fatfs.h"
#include "Libs_EspLink.h"
#include "App_Clock.h"
#include "App_Video.h"
#include "App_HwTest.h"

// /************************************************************************************************************
//  * EXTERN VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * PRIVATE MACROS AND DEFINES
//  ************************************************************************************************************/
#define APP_MANAGER_DEFAULT_APP                 LINK_APP_CLOCK

// /************************************************************************************************************
//  * PRIVATE TYPEDEFS
//  ************************************************************************************************************/
typedef struct
{
    void (*Init)(void);
    void (*Run)(void);
    void (*Deinit)(void);       // duoc phep NULL neu app khong can don dep
} App_Manager_Entry_st;

// /************************************************************************************************************
//  * STATIC VARIABLES
//  ************************************************************************************************************/
/* Chi so bang = app id cua protocol (LINK_APP_CLOCK/VIDEO/HWTEST) */
static const App_Manager_Entry_st l_AppTable_st[] =
{
    { App_Clock_Init,  App_Clock_Run,  0                },  /* 0: LINK_APP_CLOCK  */
    { App_Video_Init,  App_Video_Run,  App_Video_Deinit },  /* 1: LINK_APP_VIDEO  */
    { App_HwTest_Init, App_HwTest_Run, 0                },  /* 2: LINK_APP_HWTEST */
};
#define APP_MANAGER_APP_COUNT                   (sizeof(l_AppTable_st) / sizeof(l_AppTable_st[0]))

static uint8_t l_CurrentApp_u8 = APP_MANAGER_DEFAULT_APP;

/* Nhan file tu ESP: FIL rieng de khong dung do USERFile cua App_HwTest */
static FIL l_RxFile_st;
static uint8_t l_RxOpen_u8 = 0;
static uint32_t l_RxExpectedOffset_u32 = 0;
static uint32_t l_RxCrc_u32 = 0;

// /************************************************************************************************************
//  * STATIC FUNCTION PROTOTYPES
//  ************************************************************************************************************/
static uint32_t App_Manager_Crc32Update(uint32_t p_Crc_u32, const uint8_t* p_Data_u8, uint32_t p_Len_u32);
static uint8_t App_Manager_FileStart(uint32_t p_Size_u32, uint8_t p_Type_u8, const char* p_Name_char);
static uint8_t App_Manager_FileData(uint32_t p_Offset_u32, const uint8_t* p_Data_u8, uint16_t p_Len_u16);
static uint8_t App_Manager_FileEnd(uint32_t p_Crc32_u32);
static void App_Manager_SwitchTo(uint8_t p_AppId_u8);

// /************************************************************************************************************
//  * STATIC FUNCTIONS
//  ************************************************************************************************************/
/* CRC32 (poly dao 0xEDB88320) - PHAI khop thuat toan phia ESP (StationWeb.cpp) */
static uint32_t App_Manager_Crc32Update(uint32_t p_Crc_u32, const uint8_t* p_Data_u8, uint32_t p_Len_u32)
{
    uint32_t f_Index_u32 = 0;
    uint8_t f_Bit_u8 = 0;

    p_Crc_u32 = ~p_Crc_u32;
    for (f_Index_u32 = 0; f_Index_u32 < p_Len_u32; f_Index_u32++)
    {
        p_Crc_u32 ^= p_Data_u8[f_Index_u32];
        for (f_Bit_u8 = 0; f_Bit_u8 < 8; f_Bit_u8++)
        {
            p_Crc_u32 = (p_Crc_u32 >> 1) ^ (0xEDB88320uL & (uint32_t)(-(int32_t)(p_Crc_u32 & 1u)));
        }
    }
    return ~p_Crc_u32;
}

/* Bat dau nhan file: tao/ghi de tren the SD. Tra 0 = OK (thanh ACK cho ESP) */
static uint8_t App_Manager_FileStart(uint32_t p_Size_u32, uint8_t p_Type_u8, const char* p_Name_char)
{
    (void)p_Size_u32;   /* ESP gui 0 (chua biet truoc), chot bang FILE_END */
    (void)p_Type_u8;

    if (l_RxOpen_u8)
    {
        (void)f_close(&l_RxFile_st);    /* transfer truoc bi bo do */
        l_RxOpen_u8 = 0;
    }

    /* Luu y: FatFs dang cau hinh 8.3, ten dai se fail -> ESP nhan bao loi */
    if (f_open(&l_RxFile_st, p_Name_char, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
    {
        return 1;
    }

    l_RxOpen_u8 = 1;
    l_RxExpectedOffset_u32 = 0;
    l_RxCrc_u32 = 0;
    return 0;
}

/* Nhan 1 khoi du lieu. ESP co the gui lai khoi cu khi ACK bi mat -> bo qua khoi trung lap */
static uint8_t App_Manager_FileData(uint32_t p_Offset_u32, const uint8_t* p_Data_u8, uint16_t p_Len_u16)
{
    UINT f_Written_u32 = 0;

    if (!l_RxOpen_u8)
    {
        return 1;
    }
    if (p_Offset_u32 < l_RxExpectedOffset_u32)
    {
        return 0;   /* khoi trung lap (retransmit): da ghi roi, ACK OK */
    }
    if (p_Offset_u32 > l_RxExpectedOffset_u32)
    {
        return 1;   /* thung du lieu: bao loi de ESP dung transfer */
    }

    if ((f_write(&l_RxFile_st, p_Data_u8, p_Len_u16, &f_Written_u32) != FR_OK) || (f_Written_u32 != p_Len_u16))
    {
        (void)f_close(&l_RxFile_st);
        l_RxOpen_u8 = 0;
        return 1;
    }

    l_RxCrc_u32 = App_Manager_Crc32Update(l_RxCrc_u32, p_Data_u8, p_Len_u16);
    l_RxExpectedOffset_u32 += p_Len_u16;
    return 0;
}

/* Ket thuc: dong file + so CRC32 toan file voi phia gui */
static uint8_t App_Manager_FileEnd(uint32_t p_Crc32_u32)
{
    if (!l_RxOpen_u8)
    {
        return 1;
    }
    (void)f_close(&l_RxFile_st);
    l_RxOpen_u8 = 0;

    return (l_RxCrc_u32 == p_Crc32_u32) ? 0 : 1;
}

/* Doi app: don dep app cu (neu co) roi khoi tao app moi */
static void App_Manager_SwitchTo(uint8_t p_AppId_u8)
{
    if (p_AppId_u8 >= APP_MANAGER_APP_COUNT)
    {
        return;
    }
    if (l_AppTable_st[l_CurrentApp_u8].Deinit != 0)
    {
        l_AppTable_st[l_CurrentApp_u8].Deinit();
    }
    l_CurrentApp_u8 = p_AppId_u8;
    Libs_EspLink_SetCurrentApp(p_AppId_u8);
    l_AppTable_st[p_AppId_u8].Init();
}

// ************************************************************************************************************
//  * GLOBAL FUNCTIONS
//  ************************************************************************************************************/
void App_Manager_Init(void)
{
    /* Mount SD 1 lan cho moi app dung chung (khong co the -> app tu bao loi khi dung) */
    (void)f_mount(&USERFatFS, (TCHAR const*)USERPath, 1);

    Libs_EspLink_Init();
    Libs_EspLink_SetFileCallbacks(App_Manager_FileStart, App_Manager_FileData, App_Manager_FileEnd);
    Libs_EspLink_SetCurrentApp(APP_MANAGER_DEFAULT_APP);

    l_CurrentApp_u8 = APP_MANAGER_DEFAULT_APP;
    l_AppTable_st[l_CurrentApp_u8].Init();
}

void App_Manager_Run(void)
{
    uint8_t f_RequestedApp_u8 = 0;

    /* 1. Nhan/tra lenh tu ESP Station */
    Libs_EspLink_Process();

    /* 2. Co yeu cau doi app (id da duoc EspLink validate <= LINK_APP_HWTEST) */
    if (Libs_EspLink_TakeAppRequest(&f_RequestedApp_u8))
    {
        if (f_RequestedApp_u8 != l_CurrentApp_u8)
        {
            App_Manager_SwitchTo(f_RequestedApp_u8);
        }
    }

    /* 3. Chay app hien hanh */
    l_AppTable_st[l_CurrentApp_u8].Run();
}

uint8_t App_Manager_GetCurrentApp(void)
{
    return l_CurrentApp_u8;
}
