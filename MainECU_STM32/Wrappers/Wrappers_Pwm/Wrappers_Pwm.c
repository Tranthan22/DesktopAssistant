#include "Wrappers_Pwm.h"

// /************************************************************************************************************
//  * EXTERN VARIABLES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * PRIVATE MACROS AND DEFINES
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * PRIVATE TYPEDEFS
//  ************************************************************************************************************/

// /************************************************************************************************************
//  * STATIC VARIABLES
//  ************************************************************************************************************/
TIM_HandleTypeDef l_Tim_Handler_st;

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

void Wrappers_Pwm_Init(void)
{
    TIM_ClockConfigTypeDef f_Clock_SourceConfig_st = {0};
    TIM_MasterConfigTypeDef f_Clock_sMasterConfig_st = {0};

    l_Tim_Handler_st.Instance = TIM2;
    l_Tim_Handler_st.Init.Prescaler = 10000;
    l_Tim_Handler_st.Init.CounterMode = TIM_COUNTERMODE_UP;
    l_Tim_Handler_st.Init.Period = 999;
    l_Tim_Handler_st.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    l_Tim_Handler_st.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&l_Tim_Handler_st) != HAL_OK)
    {
        Error_Handler();
    }
    f_Clock_SourceConfig_st.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&l_Tim_Handler_st, &f_Clock_SourceConfig_st) != HAL_OK)
    {
        Error_Handler();
    }
    f_Clock_sMasterConfig_st.MasterOutputTrigger = TIM_TRGO_RESET;
    f_Clock_sMasterConfig_st.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&l_Tim_Handler_st, &f_Clock_sMasterConfig_st) != HAL_OK)
    {
        Error_Handler();
    }
}