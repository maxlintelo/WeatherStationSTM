/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sensor.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

/* Definitions for readSensor */
osThreadId_t readSensorHandle;
const osThreadAttr_t readSensor_attributes = {
  .name = "readSensor",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for sendData */
osThreadId_t sendDataHandle;
const osThreadAttr_t sendData_attributes = {
  .name = "sendData",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* USER CODE BEGIN PV */

float float_finalTemp = 0.0;
float float_finalHumid = 0.0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
void ReadSensorTask(void *argument);
void StartSendData(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  initSensor(&hi2c1);

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of readSensor */
  readSensorHandle = osThreadNew(ReadSensorTask, NULL, &readSensor_attributes);

  /* creation of sendData */
  sendDataHandle = osThreadNew(StartSendData, NULL, &sendData_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x2000090E;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void debugPrint(UART_HandleTypeDef *huart, char _out[]){
  HAL_UART_Transmit(huart, (uint8_t *) _out, strlen(_out), 10);
}

void debugPrintln(UART_HandleTypeDef *huart, char _out[]){
  HAL_UART_Transmit(huart, (uint8_t *) _out, strlen(_out), 10);
  char newline[2] = "\r\n";
  HAL_UART_Transmit(huart, (uint8_t *) newline, 2, 10);
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_ReadSensorTask */
/**
  * @brief  Function implementing the readSensor thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_ReadSensorTask */
void ReadSensorTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  const TickType_t xDelay = 2500 / portTICK_PERIOD_MS;
  /* Infinite loop */
  for(;;)
  {
	// Update values float_finalTemp and float_finalHumid
	updateSensor(&float_finalTemp, &float_finalHumid, &hi2c1);
	// Non-blocking delay
	osDelay(xDelay);

	// TODO Fix UART
	// char *msg = "Hello Nucleo Fun!\n\r";
	// HAL_UART_Transmit(&huart2, (uint8_t*)msg, 20, 0xFFFF);
	// HAL_Delay(1000);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartSendData */

char* rep(char* input, char find, char replace) {
	char * output = (char*)malloc(strlen(input));

	for (int i = 0; i < strlen(input); i++)
	{
	    if (input[i] == find) output[i] = replace;
	    else output[i] = input[i];
	}

	output[strlen(input)] = '\0';

	return output;
}
/**
* @brief Function implementing the sendData thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSendData */
void StartSendData(void *argument)
{
  /* USER CODE BEGIN StartSendData */
//	const TickType_t xDelay = 10000 / portTICK_PERIOD_MS;
	//const TickType_t xConnectDelay = 5000 / portTICK_PERIOD_MS;
  /* Infinite loop */
  for(;;) {
	  debugPrintln(&huart1, "AT");
	  osDelay(5000);

	  //debugPrintln(&huart1, "AT+RST");
	  //osDelay(5000);

	  debugPrintln(&huart1, "AT+CWMODE=1");
	  osDelay(3000);

	  debugPrintln(&huart1, "AT+CWJAP=\"iPhone\",\"0odpvgt2bf0od\"");
	  osDelay(15000);

	  debugPrintln(&huart1, "AT+CIPSTART=\"TCP\",\"d0b6cd176e6ed9.localhost.run\",80");
	  osDelay(5000);

	  float temp = (float)rand()/((float)RAND_MAX/50.0);
	  float humid = (float)rand()/((float)RAND_MAX/100.0);

	  int tempLen = snprintf(NULL, 0, "%f", temp);
	  char* tempResult = (char*)malloc(tempLen + 1);
	  snprintf(tempResult, tempLen + 1, "%f", temp);

	  int humidLen = snprintf(NULL, 0, "%f", humid);
	  char* humidResult = (char*)malloc(humidLen + 1);
	  snprintf(humidResult, humidLen + 1, "%f", humid);

	  const char* first = "GET /api/v2?temperature=";
	  const char* second = "&humidity=";
	  const char* third = " HTTP/1.1\r\nHost: d0b6cd176e6ed9.localhost.run\r\n\r\n";

	  const int MAX_BUF = 512;
	  char buffer[MAX_BUF];
	  strcat(buffer, first);
	  strcat(buffer, tempResult);
	  strcat(buffer, second);
	  strcat(buffer, humidResult);
	  strcat(buffer, third);

	  free(tempResult);
	  free(humidResult);

	  int num = strlen(buffer);
	  char snum[6];
	  itoa(num, snum, 10);

	  debugPrint(&huart1, "AT+CIPSEND=");
	  debugPrintln(&huart1, snum);
	  osDelay(2500);

	  debugPrint(&huart1, buffer);
	  osDelay(10000);

	  debugPrintln(&huart1, "AT+CIPCLOSE");
	  osDelay(1000);
	  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);


	  //	  STM(tx) -> ESP(rx)
	  //	  ESP(tx) -> Kabel(rx)

  }




//  {
//	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
//
//	  char line1[] = "AT+CWMODE=1\r\n";											//set wifi mode
//	  HAL_UART_Transmit(&huart1, (uint8_t*) line1, strlen(line1), 10);
//	  osDelay(1000);
//
//	  char line2[] = "AT+CWJAP=\"KPNC12816\",\"VVWbKchF7R3J3wvN\"\r\n";			//connect to an AP(router)
//	  HAL_UART_Transmit(&huart1, (uint8_t*) line2, strlen(line2), 10);
//	  osDelay(xConnectDelay);
//
//	  char line3[] = "AT+CIPSTART=\"TCP\",\"82.170.159.85\",8081\r\n";			//target IP when sending data (hier maak TCP connectie)
//	  HAL_UART_Transmit(&huart1, (uint8_t*) line3, strlen(line3), 10);
//	  osDelay(1000);
//
//	  char line4[] = "AT+CIPSEND=18\r\n";										//hoeveel data verzenden
//	  HAL_UART_Transmit(&huart1, (uint8_t*) line4, strlen(line4), 10);
//	  osDelay(1000);
//
//	  char line5[] = "GET / HTTP/1.1\r\n\r\n";									//data daadwerkelijk verzenden (request)
//	  HAL_UART_Transmit(&huart1, (uint8_t*) line5, strlen(line5), 10);
//	  osDelay(1000);
//
//	  char line6[] = "AT+CIPCLOSE\r\n";											//close TCP
//	  HAL_UART_Transmit(&huart1, (uint8_t*) line6, strlen(line6), 10);
//	  osDelay(1000);
//	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
//    osDelay(xDelay);
//
//																				//dan return response met statuscode to STM32
//
//
//  }
  /* USER CODE END StartSendData */
}

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
  if (htim->Instance == TIM6) {
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

#ifdef  USE_FULL_ASSERT
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
