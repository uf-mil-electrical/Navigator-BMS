/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "INA236.h"
#include "bq76920.h"
#include "tmp117.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"
#include "fdcan.h"
#include "gpio.h"
#include "i2c.h"
#include "ina236.h"
#include "stm32g4xx_hal.h"
#include "tim.h"
#include "tmp117.h"

BQ76920 bq;
INA236 ina;
TMP117 tmp;
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void BQ76920Task(void *argument);
void INA236Task(void *argument);
void TMP117Task(void *argument);
/* USER CODE END 0 */

// RTOS Tasks
osThreadId_t BQ76920TaskHandle;
const osThreadAttr_t BQ76920Task_attr = {
    .name = "BQ76920 Task",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

osThreadId_t INA236TaskHandle;
const osThreadAttr_t INA236Task_attr = {
    .name = "INA236 Task",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

osThreadId_t TMP117TaskHandle;
const osThreadAttr_t TMP117Task_attr = {
    .name = "TMP117 Task",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

// Mutexes
osMutexId_t i2cMutex;
const osMutexAttr_t i2cMutex_attr = {.name = "i2cMutex"};
/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  SCB->VTOR = 0x08000000;
  __DSB();
  __ISB();
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();
  /* USER CODE BEGIN SysInit */
  MX_NVIC_Init();
  /* USER CODE END SysInit */
  NVIC_ClearPendingIRQ(TIM6_DAC_IRQn);
  NVIC_SetPendingIRQ(TIM6_DAC_IRQn);

  extern TIM_HandleTypeDef htim6;

  __enable_irq();

  HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 15, 0);
  HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);

  __HAL_TIM_CLEAR_FLAG(&htim6, TIM_FLAG_UPDATE);
  __HAL_TIM_ENABLE_IT(&htim6, TIM_IT_UPDATE);
  HAL_TIM_Base_Start_IT(&htim6);
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FDCAN1_Init();
  MX_I2C1_Init();
  MX_TIM16_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
  BQ76920_Initialize(&bq, &hi2c1, BQ76920_ALERT_GPIO_Port, BQ76920_ALERT_Pin,
                     BQ76920_BOOT_GPIO_Port, BQ76920_BOOT_Pin);
  readCells(&bq);

  INA236_Initialize(&ina, &hi2c1);
  getINA236(&ina);

  TMP117_Initialize(&tmp, &hi2c1);
  readTMP117(&tmp);
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize(); /* Call init function for freertos objects (in
                           cmsis_os2.c) */

  i2cMutex = osMutexNew(&i2cMutex_attr);
  if (i2cMutex == NULL) {
    Error_Handler();
  }
  INA236TaskHandle = osThreadNew(INA236Task, NULL, &INA236Task_attr);
  BQ76920TaskHandle = osThreadNew(BQ76920Task, NULL, &BQ76920Task_attr);
  TMP117TaskHandle = osThreadNew(TMP117Task, NULL, &TMP117Task_attr);
  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
void BQ76920Task(void *argument) {
  for (;;) {
    osMutexAcquire(i2cMutex, osWaitForever);
    readCells(&bq);
    osMutexRelease(i2cMutex);
    osDelay(5);
  }
}

void INA236Task(void *argument) {
  for (;;) {
    osMutexAcquire(i2cMutex, osWaitForever);
    getINA236(&ina);
    osMutexRelease(i2cMutex);
    osDelay(5);
  }
}
void TMP117Task(void *argument) 
{
  for (;;) {
    osMutexAcquire(i2cMutex, osWaitForever);
    readTMP117(&tmp);
    osMutexRelease(i2cMutex);
    osDelay(5);
  }
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* RCC_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(RCC_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(RCC_IRQn);
  /* EXTI3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
  /* FDCAN1_IT0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
  /* EXTI9_5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
  /* EXTI15_10_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 15, 0);
  HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
