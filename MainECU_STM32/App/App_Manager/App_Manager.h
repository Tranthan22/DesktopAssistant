//-----------------------------------
//	App_Manager - dieu phoi vong doi cac App + nhan lenh tu ESP Station
//-----------------------------------
//	- Bang app: 0 = Clock (mac dinh), 1 = Video, 2 = HwTest (id khop LINK_APP_* cua protocol).
//	- Moi vong lap: xu ly UART tu ESP (Libs_EspLink_Process), doi app khi co SELECT_APP,
//	  roi chay Run() cua app hien hanh (moi app tu dam bao Run() khong blocking dai).
//	- Nhan file tu ESP (FILE_START/DATA/END) va ghi xuong the SD qua FatFs,
//	  kiem CRC32 toan file truoc khi bao OK.
//
//	main.c chi can:  App_Manager_Init();  while(1) { App_Manager_Run(); }
//-----------------------------------

#ifndef APP_APP_MANAGER_APP_MANAGER_H_
#define APP_APP_MANAGER_APP_MANAGER_H_

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

// /************************************************************************************************************
//  * EXPORTED VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * GLOBAL FUNCTION PROTOTYPES
//  ************************************************************************************************************/
void App_Manager_Init(void);
void App_Manager_Run(void);
uint8_t App_Manager_GetCurrentApp(void);

#endif /* APP_APP_MANAGER_APP_MANAGER_H_ */
