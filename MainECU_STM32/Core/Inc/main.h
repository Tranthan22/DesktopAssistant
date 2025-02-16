
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

// /************************************************************************************************************
//  * INCLUDES
//  ************************************************************************************************************/
#include "stm32f4xx_hal.h"

// /************************************************************************************************************
//  * MACROS AND DEFINES
//  ************************************************************************************************************/
#define Touch_IRQ_Pin GPIO_PIN_3
#define Touch_IRQ_GPIO_Port GPIOA
#define Touch_CS_Pin GPIO_PIN_4
#define Touch_CS_GPIO_Port GPIOA
#define Screen_CS_Pin GPIO_PIN_0
#define Screen_CS_GPIO_Port GPIOB
#define Screen_RST_Pin GPIO_PIN_1
#define Screen_RST_GPIO_Port GPIOB
#define Screen_DC_Pin GPIO_PIN_2
#define Screen_DC_GPIO_Port GPIOB
#define Screen_Led_Pin GPIO_PIN_10
#define Screen_Led_GPIO_Port GPIOB
#define ESP_CS_Pin GPIO_PIN_12
#define ESP_CS_GPIO_Port GPIOB
#define SDCard_CS_Pin GPIO_PIN_8
#define SDCard_CS_GPIO_Port GPIOA
#define ESP_SCL_Pin GPIO_PIN_6
#define ESP_SCL_GPIO_Port GPIOB
#define ESP_SDA_Pin GPIO_PIN_7
#define ESP_SDA_GPIO_Port GPIOB

// GPIO module
#define LogicalChannel_1              	Screen_CS_GPIO_Port,Screen_CS_Pin       // ILI9341 Select Pin
#define LogicalChannel_2              	Screen_RST_GPIO_Port,Screen_RST_Pin     // ILI9341 Reset Pin
#define LogicalChannel_3              	Screen_DC_GPIO_Port,Screen_DC_Pin       // ILI9341 Data Control Pin
#define LogicalChannel_4				Screen_Led_GPIO_Port,Screen_Led_Pin		// ILI9341 LED
// SPI Module
#define SPI_Screen                    	1

// /************************************************************************************************************
//  * TYPEDEFS
//  ************************************************************************************************************/


// /************************************************************************************************************
//  * EXPORTED VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * GLOBAL FUNCTION PROTOTYPES
//  ************************************************************************************************************/
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
