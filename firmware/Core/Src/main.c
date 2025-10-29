/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include <stdlib.h>
#include <time.h>
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
TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart2;
volatile unsigned char rectran = 0;
unsigned char rectran_prec = 0;

/* USER CODE BEGIN PV */

int classUSB;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */

#include "usbd_hid.h"
#include "usb_device.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

//only if keyboard descriptor is used
void send_keyboard_report(uint8_t key) {
	uint8_t report[8] = {0x00, 0x00, 0x04, 0, 0, 0, 0, 0};
	report[2] = key;  // Esempio: HID_KEY_A = 0x04
	USBD_HID_SendReport(&hUsbDeviceFS, report, sizeof(report));
	HAL_Delay(100);
}
void send_keyboard_comb_report(uint8_t modifier, uint8_t key) {
	uint8_t report[8] = {0x00, 0x00, 0x04, 0, 0, 0, 0, 0};
	report[0] = modifier;  // Esempio: HID_KEY_A = 0x04
	report[2] = key;  // Esempio: HID_KEY_A = 0x04
	USBD_HID_SendReport(&hUsbDeviceFS, report, sizeof(report));
	HAL_Delay(100);
}

void send_mouse_report(int8_t x, int8_t y, uint8_t buttons) {
	uint8_t report[4] = {buttons, x, y, 0}; // HID mouse report (4 bytes)
	USBD_HID_SendReport(&hUsbDeviceFS, report, sizeof(report));
	HAL_Delay(100);
}


#define NONE						0
#define ANTI_HIBERNATION			1
#define WRITE_WORD					2
#define LOCK_PC						3
#define THROWING_THE_DICE			4
#define TRANSCEIVER					5
#define MODBUS						6
#define NONE_CASUAL  			7

uint8_t ReadSwitcher(void)
{
	uint8_t valore = 0;

	// Leggi PA9 (bit 2)
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9) == GPIO_PIN_SET)
		valore |= (1 << 2);

	// Leggi PA8 (bit 1)
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == GPIO_PIN_SET)
		valore |= (1 << 1);

	// Leggi PB15 (bit 0)
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == GPIO_PIN_SET)
		valore |= (1 << 0);

	return valore;
}


GPIO_TypeDef* ledPorts[8] = {GPIOC, GPIOA, GPIOA, GPIOA, GPIOB, GPIOB, GPIOB, GPIOB};
uint16_t ledPins[8] = {GPIO_PIN_13, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7, GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_11, GPIO_PIN_10};

void ChaseLedStep(uint8_t *indexl) {
	// Spegni tutti
	for (int i = 0; i < 8; i++) {
		HAL_GPIO_WritePin(ledPorts[i], ledPins[i], GPIO_PIN_RESET);
	}
	// Accendi il LED corrente
	HAL_GPIO_WritePin(ledPorts[*indexl], ledPins[*indexl], GPIO_PIN_SET);
	// Passa al prossimo indice (con avvolgimento)
	*indexl = (*indexl + 1) % 8;
}

void GestisciLedDado(void)
{

	if (!capsLockState)
		return;

	// Genera numero casuale tra 1 e 6
	uint8_t numLedAccesi = (rand() % 6) + 1;

	// Prima spegni tutti i LED
	for (int i = 0; i < 8; i++)
	{
		HAL_GPIO_WritePin(ledPorts[i], ledPins[i], GPIO_PIN_RESET);
	}

	// Accendi numLedAccesi LED in modo casuale

	// Creo un array di indici da 0 a 7
	uint8_t indici[8] = {0,1,2,3,4,5,6,7};

	// Mescola l'array (Fisher-Yates shuffle)
	for (int i = 7; i > 0; i--)
	{
		int j = rand() % (i + 1);
		uint8_t temp = indici[i];
		indici[i] = indici[j];
		indici[j] = temp;
	}

	// Accende i primi numLedAccesi indici
	for (int i = 0; i < numLedAccesi; i++)
	{
		HAL_GPIO_WritePin(ledPorts[indici[i]], ledPins[indici[i]], GPIO_PIN_SET);
	}
}

#define BUF_SIZE        2000
#define FRAME_TIMEOUT_MS 5   // timeout in ms tra byte per determinare fine frame

static uint8_t buffer[BUF_SIZE];
static uint16_t indexb = 0;
uint8_t uart_rx;
uint32_t last_rx_tick;

void main_loop() {
	uint8_t currentLed = 0;
	while (1) {
		switch(IoInputSwitches)
		{
		case NONE:
			ChaseLedStep(&currentLed);
			HAL_Delay(200);
			break;
		case NONE_CASUAL:
			currentLed = rand() % 8;
			ChaseLedStep(&currentLed);
			HAL_Delay((rand() % 60) +1);
			break;
		case ANTI_HIBERNATION:
			send_mouse_report(5, 0, 0); // Muove il mouse di 10 pixel
			HAL_Delay(10);
			send_mouse_report(0, 5, 0); // Muove il mouse di 10 pixel
			HAL_Delay(10);
			send_mouse_report(-5, 0, 0); // Muove il mouse di 10 pixel
			HAL_Delay(10);
			send_mouse_report(0, -5, 0); // Muove il mouse di 10 pixel
			HAL_Delay(10);
			break;
		case LOCK_PC:
			send_keyboard_comb_report(0x08,0x0F);
			HAL_Delay(100);
			break;
		case WRITE_WORD:
			HAL_Delay(100);
			send_keyboard_report(0x06);
			HAL_Delay(100);
			send_keyboard_report(0x0c);
			HAL_Delay(100);
			send_keyboard_report(0x04);
			HAL_Delay(100);
			send_keyboard_report(0x12);
			HAL_Delay(100);
			send_keyboard_report(0x2C);
			break;
		case THROWING_THE_DICE:
			GestisciLedDado();
			HAL_Delay(100);
			break;
		case TRANSCEIVER:
			uint8_t uart_rx;

			if(rectran!=rectran_prec)
			{
				rectran_prec = rectran;
				ChaseLedStep(&currentLed);
			}
			// prova a ricevere 1 byte con timeout breve
			if (HAL_UART_Receive(&huart2, &uart_rx, 1, 0) == HAL_OK)
			{
				buffer[indexb++] = uart_rx;          // accumula byte
				last_rx_tick = HAL_GetTick();       // aggiorna timestamp ultimo byte

				if (indexb >= BUF_SIZE)
					indexb = BUF_SIZE - 1;           // evita overflow
			}

			// verifica timeout tra byte: fine frame Modbus
			if (indexb > 0 && (HAL_GetTick() - last_rx_tick) > FRAME_TIMEOUT_MS)
			{
				CDC_Transmit_FS(buffer, indexb);     // invia frame completo via USB CDC
				rectran=3;
				indexb = 0;                          // reset buffer
			}
			if (uart_needs_update) {
				huart2.Init.BaudRate = NewLineCoding.BaudRate;

				// ParitC
				switch (NewLineCoding.Parity) {
				case 0:
					huart2.Init.Parity = UART_PARITY_NONE;
					break;
				case 1:
					huart2.Init.Parity = UART_PARITY_ODD;
					break;
				case 2:
					huart2.Init.Parity = UART_PARITY_EVEN;
					break;
				default:
					huart2.Init.Parity = UART_PARITY_NONE;
					break;
				}

				// Stop bit
				switch (NewLineCoding.StopBits) {
				case 0:
					huart2.Init.StopBits = UART_STOPBITS_1;
					break;
				case 2:
					huart2.Init.StopBits = UART_STOPBITS_2;
					break;
				default:
					huart2.Init.StopBits = UART_STOPBITS_1;
					break;
				}

				// Data bit
				switch (NewLineCoding.DataBits) {
				case 8:
					huart2.Init.WordLength = UART_WORDLENGTH_8B; // SEMPRE 8B, anche con paritC
					break;

				case 9:
					if (NewLineCoding.Parity == 0) {
						huart2.Init.WordLength = UART_WORDLENGTH_9B;
					} else {
						// 9 bit + paritC  non supportato, fallback
						huart2.Init.WordLength = UART_WORDLENGTH_8B;
					}
					break;

				default:
					huart2.Init.WordLength = UART_WORDLENGTH_8B;
					break;
				}

				HAL_UART_DeInit(&huart2);
				if (HAL_UART_Init(&huart2) != HAL_OK) {
					Error_Handler();
				}

				uart_needs_update = 0;
			}
			break;
		case MODBUS:

			break;
			/**/
		}
	}
}

char isUsb = 0;
int main(void)
{
	classUSB = 0;
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
	IoInputSwitches = ReadSwitcher();

	switch(IoInputSwitches)
	{
	case ANTI_HIBERNATION:
		SetIsMouse(1);
		isUsb = 1;
		break; //mouse
	case WRITE_WORD:
	case LOCK_PC:
	case THROWING_THE_DICE:
		isUsb = 1;
		SetIsMouse(0);
		classUSB = 0;
		break; //keyboard
	case MODBUS:
	case TRANSCEIVER:
		isUsb = 1;
		classUSB = 1;
		break;
	}

	MX_TIM1_Init();
	MX_USART2_UART_Init();
	if(isUsb==1)
	{
		MX_USB_DEVICE_Init(classUSB);
	}
	/* USER CODE BEGIN 2 */

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */
		main_loop();
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
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
	                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
	{
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
	PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

	/* USER CODE BEGIN TIM1_Init 0 */

	/* USER CODE END TIM1_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	/* USER CODE BEGIN TIM1_Init 1 */

	/* USER CODE END TIM1_Init 1 */
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 0;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 65535;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
	{
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM1_Init 2 */

	/* USER CODE END TIM1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, USART2_EN_Pin|LED1_Pin|LED2_Pin|LED3_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LED4_Pin|LED5_Pin|LED7_Pin|LED6_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : LED_Pin */
	GPIO_InitStruct.Pin = LED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : USART2_EN_Pin LED1_Pin LED2_Pin LED3_Pin */
	GPIO_InitStruct.Pin = USART2_EN_Pin|LED1_Pin|LED2_Pin|LED3_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : LED4_Pin LED5_Pin LED7_Pin LED6_Pin */
	GPIO_InitStruct.Pin = LED4_Pin|LED5_Pin|LED7_Pin|LED6_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : INBIT3_Pin */
	GPIO_InitStruct.Pin = INBIT3_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(INBIT3_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : INBIT2_Pin INBIT1_Pin */
	GPIO_InitStruct.Pin = INBIT2_Pin|INBIT1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
