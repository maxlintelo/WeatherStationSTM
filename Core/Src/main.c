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
#include "uartRingBufDMA.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BUF_MAX 128
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;

/* Definitions for readTemperature */
osThreadId_t readTemperatureHandle;
const osThreadAttr_t readTemperature_attributes = {
		.name = "readTemperature",
		.stack_size = 128 * 4,
		.priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for sendData */
osThreadId_t sendDataHandle;
const osThreadAttr_t sendData_attributes = {
		.name = "sendData",
		.stack_size = 192 * 4,
		.priority = (osPriority_t) osPriorityLow,
};
/* Definitions for readPressure */
osThreadId_t readPressureHandle;
const osThreadAttr_t readPressure_attributes = {
		.name = "readPressure",
		.stack_size = 128 * 4,
		.priority = (osPriority_t) osPriorityLow,
};
/* USER CODE BEGIN PV */

float float_finalTemp = 0.0;
float float_finalHumid = 0.0;
int32_t pressure;
int isOK = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C2_Init(void);
void StartReadTemperature(void *argument);
void StartSendData(void *argument);
void StartReadPressure(void *argument);

/* USER CODE BEGIN PFP */

void delay_ms(uint32_t period_ms);
int8_t i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
int8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
int8_t spi_reg_write(uint8_t cs, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
int8_t spi_reg_read(uint8_t cs, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
void print_rslt(const char api_name[], int8_t rslt);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

char buffer[100];


/* USER CODE END Header_StartSendData */
/* Wait for 1 minute to find OK response.
 */
int waitForOK(int iterations)
{
	/*
	 * 600 * 100 MS = 1 minute.
	 */
	isOK = 0;
	for (int i = 0; i < iterations; i++) {
		/* isOK becomes one if OK is found in serial feedback.
		 * (HAL_UARTEx_RxEventCallback) handles isOK.
		 * If OK is found, reset it, wait one second, and return.
		 */
		if(isOK) {
			isOK = 0;
			osDelay(1000 / portTICK_PERIOD_MS); // 1 Second
			return 1;
		}
		osDelay(100 / portTICK_PERIOD_MS); // 100 MS
	}
	/* This code will only be reached if no OK is detected.
	 * Only after 600 * 100 MS = 1 minute of checking.
	 */
	return 0;
}


void sendCommand(char out[]){
	HAL_UART_Transmit(&huart1, (uint8_t *) out, strlen(out), 10);
}
void sendAndWait(char out[], int timeout, int do_retry) {
	Ringbuf_Reset_Buf();
	Ringbuf_Reset_DMA();

	retry: sendCommand(out);
	if (!waitForOK(timeout)) {
		Ringbuf_Reset_Buf();
		Ringbuf_Reset_DMA();
		osDelay(1000 / portTICK_PERIOD_MS);
		if (do_retry) goto retry;
	}
}




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
	MX_DMA_Init();
	MX_USART1_UART_Init();
	MX_I2C2_Init();
	/* USER CODE BEGIN 2 */
	Ringbuf_Init();
	//  initSensor(&hi2c1);

	//MPB280 temperature
	//							BMP280_Init(&hi2c1, BMP280_TEMPERATURE_16BIT, BMP280_STANDARD, BMP280_FORCEDMODE);
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
	/* creation of readTemperature */
	readTemperatureHandle = osThreadNew(StartReadTemperature, NULL, &readTemperature_attributes);

	/* creation of sendData */
	sendDataHandle = osThreadNew(StartSendData, NULL, &sendData_attributes);

	/* creation of readPressure */
	readPressureHandle = osThreadNew(StartReadPressure, NULL, &readPressure_attributes);

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
#ifdef BMP280
		HAL_GPIO_WritePin(TEST_GPIO_Port, TEST_Pin, 1);
		BMP280_ReadTemperatureAndPressure(&temperature, &pressure);
		HAL_GPIO_WritePin(TEST_GPIO_Port, TEST_Pin, 0);
#endif
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
 * @brief I2C2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C2_Init(void)
{

	/* USER CODE BEGIN I2C2_Init 0 */

	/* USER CODE END I2C2_Init 0 */

	/* USER CODE BEGIN I2C2_Init 1 */

	/* USER CODE END I2C2_Init 1 */
	hi2c2.Instance = I2C2;
	hi2c2.Init.Timing = 0x2000090E;
	hi2c2.Init.OwnAddress1 = 0;
	hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c2.Init.OwnAddress2 = 0;
	hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c2) != HAL_OK)
	{
		Error_Handler();
	}
	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
	{
		Error_Handler();
	}
	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN I2C2_Init 2 */

	/* USER CODE END I2C2_Init 2 */

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
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void)
{

	/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Channel2_3_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

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

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartReadTemperature */
/**
 * @brief  Function implementing the readTemperature thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartReadTemperature */
void StartReadTemperature(void *argument)
{
	/* USER CODE BEGIN 5 */
	const TickType_t xDelay = 2500 / portTICK_PERIOD_MS;
	/* Infinite loop */
	for(;;)
	{
		//	// Update values float_finalTemp and float_finalHumid
		//	updateSensor(&float_finalTemp, &float_finalHumid, &hi2c1);
		//	// Non-blocking delay
		osDelay(xDelay);
	}
	/* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartSendData */

/**
 * @brief Function implementing the sendData thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartSendData */
void StartSendData(void *argument)
{
	/* USER CODE BEGIN StartSendData */
	/* Infinite loop */
	for(;;) {
		// Set mode to AP client
		sendAndWait("ATE0\r\n", 300, 0);

		sendAndWait("AT+CWMODE=1\r\n", 300, 1);

		// Connect to AP
		sendAndWait("AT+CWJAP=\"LAPTOP-VG095PM22913\",\"3987<Cs0\"\r\n", 300, 1);

		//Make TCP connection
		sendAndWait("AT+CIPSTART=\"TCP\",\"81.207.176.52\",8081\r\n", 300, 1);

		/*
		 * TODO Get real values from sensor
		 */
		int mockTemperature = (rand() % (50 + 1 - (-20))) + (-20);
		int mockHumidity = (rand() % (100 + 1 - 0)) + 0;
		int mockPressure = (rand() % (1100 + 1 - 900)) + 900;

		/*
		 * Concat the values in the request string
		 */
		char requestBuffer[128];
		snprintf(requestBuffer, 128, "GET /api/v3/post?temperature=%d&humidity=%d&pressure=%d HTTP/1.1\r\nHost: 81.207.176.52\r\n\r\n", mockTemperature, mockHumidity, mockPressure);

		/*
		 * Get the length of the request
		 */
		int num = strlen(requestBuffer);
		char snum[6];
		itoa(num, snum, 10);

		/*
		 * Send the length of the request
		 */
		sendCommand("AT+CIPSEND=");
		sendCommand(snum);
		sendCommand("\r\n");
		osDelay(2500);

		/*
		 * Send the request
		 */
		sendAndWait(requestBuffer, 300, 0);
		osDelay(5000);

		// Close the TCP connection
		sendCommand("\r\n");
		sendAndWait("AT+CIPCLOSE\r\n", 100, 0);

		// Toggle LED for status and wait
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		osDelay(1500 / portTICK_PERIOD_MS);
		Ringbuf_Reset_Buf();
		Ringbuf_Reset_DMA();
	}
	/* USER CODE END StartSendData */
}

/* USER CODE BEGIN Header_StartReadPressure */
/**
 * @brief Function implementing the readPressure thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartReadPressure */
void StartReadPressure(void *argument)
{
	/* USER CODE BEGIN StartReadPressure */
	/* Infinite loop */
	for(;;)
	{
		osDelay(1);
	}
	/* USER CODE END StartReadPressure */
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
