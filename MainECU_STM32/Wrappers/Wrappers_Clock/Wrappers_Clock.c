#include "Wrappers_Clock.h"

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
void Wrappers_Clock_SystemClock_Config(void)
{
  RCC_OscInitTypeDef f_Clock_RCC_OscInitStruct_st = {0};
  RCC_ClkInitTypeDef f_Clock_RCC_ClkInitStruct_st = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  f_Clock_RCC_OscInitStruct_st.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  f_Clock_RCC_OscInitStruct_st.HSIState = RCC_HSI_ON;
  f_Clock_RCC_OscInitStruct_st.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  f_Clock_RCC_OscInitStruct_st.PLL.PLLState = RCC_PLL_ON;
  f_Clock_RCC_OscInitStruct_st.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  f_Clock_RCC_OscInitStruct_st.PLL.PLLM = 8;
  f_Clock_RCC_OscInitStruct_st.PLL.PLLN = 100;
  f_Clock_RCC_OscInitStruct_st.PLL.PLLP = RCC_PLLP_DIV2;
  f_Clock_RCC_OscInitStruct_st.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&f_Clock_RCC_OscInitStruct_st) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  f_Clock_RCC_ClkInitStruct_st.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  f_Clock_RCC_ClkInitStruct_st.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  f_Clock_RCC_ClkInitStruct_st.AHBCLKDivider = RCC_SYSCLK_DIV1;
  f_Clock_RCC_ClkInitStruct_st.APB1CLKDivider = RCC_HCLK_DIV2;
  f_Clock_RCC_ClkInitStruct_st.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&f_Clock_RCC_ClkInitStruct_st, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}
