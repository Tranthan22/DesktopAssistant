/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
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
#define SDCard_CS_Pin GPIO_PIN_8
#define SDCard_CS_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
// CAC DEFINE TUY BIEN PHAI NAM TRONG USER CODE BLOCK de song sot qua cac lan CubeMX re-gen.
// Status LED (PC13 onboard BlackPill, ACTIVE LOW: 0 = sang)
#define Led_Status_Pin                	GPIO_PIN_13
#define Led_Status_GPIO_Port          	GPIOC

// GPIO module - logical channels
#define LogicalChannel_1              	Screen_CS_GPIO_Port,Screen_CS_Pin       // ILI9341 Select Pin
#define LogicalChannel_2              	Screen_RST_GPIO_Port,Screen_RST_Pin     // ILI9341 Reset Pin
#define LogicalChannel_3              	Screen_DC_GPIO_Port,Screen_DC_Pin       // ILI9341 Data Control Pin
#define LogicalChannel_4              	Screen_Led_GPIO_Port,Screen_Led_Pin     // ILI9341 LED
#define LogicalChannel_5              	Touch_CS_GPIO_Port,Touch_CS_Pin         // XPT2046 Touch Select Pin
#define LogicalChannel_6              	Touch_IRQ_GPIO_Port,Touch_IRQ_Pin       // XPT2046 Touch IRQ (PENIRQ, low = pressed)
#define LogicalChannel_7              	SDCard_CS_GPIO_Port,SDCard_CS_Pin       // SD card Select Pin
#define LogicalChannel_8              	Led_Status_GPIO_Port,Led_Status_Pin     // Status LED blink

// SPI Module
#define SPI_Screen                    	1										// SPI1, full speed
#define SPI_Touch                     	3										// SPI1 shared bus, slowed down for XPT2046 (max ~2MHz)
#define SPI_SdCard                    	4										// SPI2 (danh rieng cho SD), full speed 25Mbit/s
#define SPI_SdCardInit                	5										// SPI2, ~390kHz (SD init phase needs <=400kHz)

// UART Module
#define UART_Esp                      	1										// USART1 <-> ESP8266 Station (115200 8N1)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
