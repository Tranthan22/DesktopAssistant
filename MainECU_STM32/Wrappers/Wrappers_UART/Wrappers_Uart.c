#include "Wrappers_Uart.h"

// /************************************************************************************************************
//  * EXTERN VARIABLES
//  ************************************************************************************************************/
extern UART_HandleTypeDef huart1;   /* CubeMX-generated handle (Core/Src/main.c) */

// /************************************************************************************************************
//  * PRIVATE MACROS AND DEFINES
//  ************************************************************************************************************/
#define UART_TX_TIMEOUT_MS                      100

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

// /************************************************************************************************************
//  * STATIC FUNCTIONS
//  ************************************************************************************************************/

// ************************************************************************************************************
//  * GLOBAL FUNCTIONS
//  ************************************************************************************************************/
/* Cau hinh USART1 (PA9/PA10, 115200 8N1) do CubeMX dam nhiem (MX_USART1_UART_Init) */
void Wrappers_Uart_Init(void)
{
}

void Wrappers_Uart_Transmit(uint8_t p_LogicalChannel_u8, const uint8_t* p_Data_u8, uint16_t p_Size_u16)
{
    if (p_LogicalChannel_u8 == 1)
    {
        HAL_UART_Transmit(&huart1, (uint8_t*)p_Data_u8, p_Size_u16, UART_TX_TIMEOUT_MS);
    }
    else
    {
    }
}

/* Doc kieu polling: lay het byte dang cho trong data register.
   TODO(EspLink): chuyen sang IRQ + ring buffer khi bat USART1 NVIC trong CubeMX,
   polling se rot byte neu vong lap chinh ban qua ~90us/byte @115200. */
uint16_t Wrappers_Uart_Receive(uint8_t p_LogicalChannel_u8, uint8_t* p_Buffer_u8, uint16_t p_MaxSize_u16)
{
    uint16_t f_Count_u16 = 0;

    if (p_LogicalChannel_u8 == 1)
    {
        /* Xoa loi overrun neu co (ORE lam RXNE ket cung) */
        if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_ORE))
        {
            __HAL_UART_CLEAR_OREFLAG(&huart1);
        }

        while ((f_Count_u16 < p_MaxSize_u16) && __HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE))
        {
            p_Buffer_u8[f_Count_u16++] = (uint8_t)(huart1.Instance->DR & 0xFF);
        }
    }

    return f_Count_u16;
}
