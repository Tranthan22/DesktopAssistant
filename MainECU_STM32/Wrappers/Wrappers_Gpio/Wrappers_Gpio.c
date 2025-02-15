#include "main.h"

void Wrappers_Gpio_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA, Touch_CS_Pin|SDCard_CS_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, Screen_CS_Pin|Screen_RST_Pin|Screen_DC_Pin|Screen_Led_Pin
                            |ESP_CS_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin : Touch_IRQ_Pin */
    GPIO_InitStruct.Pin = Touch_IRQ_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(Touch_IRQ_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : Touch_CS_Pin SDCard_CS_Pin */
    GPIO_InitStruct.Pin = Touch_CS_Pin|SDCard_CS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pins : Screen_CS_Pin Screen_RST_Pin Screen_DC_Pin Screen_Led_Pin
                            ESP_CS_Pin */
    GPIO_InitStruct.Pin = Screen_CS_Pin|Screen_RST_Pin|Screen_DC_Pin|Screen_Led_Pin
                            |ESP_CS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}