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





//#define US_KEYBOARD 1



#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include <stdlib.h>
#include <stdbool.h>
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
volatile bool break_value=false;
unsigned char rectran_prec = 0;

/* USER CODE BEGIN PV */

int classUSB;
// Parametri PWM software
#define NUM_LEDS 8
#define PWM_RESOLUTION 100  // 0–100 step di luminosità
#define PWM_BASE_FREQ 200   // frequenza PWM percepita (Hz)

#define DUTY_CYCLE_LED_ON  50
/* USER CODE END PV */
// Variabili PWM
volatile uint8_t pwm_duty[NUM_LEDS] = {0}; // intensità 0–100 per ogni LED
volatile uint8_t pwm_counter = 0;

// Handle timer
TIM_HandleTypeDef htim4;
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM4_Init(void);
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
	HAL_Delay(10);
}
void send_keyboard_comb_report(uint8_t modifier, uint8_t key) {
	uint8_t report[8] = {0x00, 0x00, 0x04, 0, 0, 0, 0, 0};
	report[0] = modifier;  // Esempio: HID_KEY_A = 0x04
	report[2] = key;  // Esempio: HID_KEY_A = 0x04
	USBD_HID_SendReport(&hUsbDeviceFS, report, sizeof(report));
	HAL_Delay(10);
}

void send_mouse_report(int8_t x, int8_t y, uint8_t buttons) {
	uint8_t report[4] = {buttons, x, y, 0}; // HID mouse report (4 bytes)
	USBD_HID_SendReport(&hUsbDeviceFS, report, sizeof(report));
	HAL_Delay(100);
}


#define NONE_CASUAL  			    0
#define ANTI_HIBERNATION			1
#define WRITE_WORD					2
#define LOCK_PC						3
#define THROWING_THE_DICE			4
#define TRANSCEIVER					5
#define MODBUS						6
#define NONE						7

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

// 1 3 8 5 6 4 2 7
//GPIO_TypeDef* ledPorts[8] = {GPIOC, GPIOA, GPIOA, GPIOA, GPIOB, GPIOB, GPIOB, GPIOB};
//uint16_t ledPins[8] = {GPIO_PIN_13, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7, GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_11, GPIO_PIN_10};


// ordine: 1 2 3 4 5 6 7 8
GPIO_TypeDef* ledPorts[8] = {GPIOC      , GPIOA      , GPIOB      , GPIOB     , GPIOB     , GPIOA     , GPIOA      , GPIOB};
uint16_t ledPins[8]       = {GPIO_PIN_13, GPIO_PIN_6 , GPIO_PIN_10, GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_7, GPIO_PIN_5 , GPIO_PIN_11};

int8_t FindLedIndex(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    for (uint8_t i = 0; i < NUM_LEDS; i++)
    {
        if (ledPorts[i] == GPIOx && ledPins[i] == GPIO_Pin)
            return i;
    }
    return -1; // non trovato
}

void HAL_GPIO_WritePin_EX(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, int PinState)
{
    int8_t ledIndex = FindLedIndex(GPIOx, GPIO_Pin);

    if (ledIndex >= 0)
    {
        // Se il LED è nel gruppo PWM, gestiamo noi
        if (PinState == 0)
            pwm_duty[ledIndex] = 0;     // LED spento (0% duty)
        else
        {
        	if(PinState == 2)
        	{
        		if(pwm_duty[ledIndex] > DUTY_CYCLE_LED_ON/4)
        			pwm_duty[ledIndex] -= DUTY_CYCLE_LED_ON/4;
        		else
        			pwm_duty[ledIndex] =0;
        	}
        	else
        		pwm_duty[ledIndex] = DUTY_CYCLE_LED_ON;   // LED acceso pieno (100% duty)
        }
    }
    else
    {
        // Pin normale → uso HAL standard
        HAL_GPIO_WritePin(GPIOx, GPIO_Pin, PinState);
    }
}

// Parametri PWM software

// Handle timer
TIM_HandleTypeDef htim4;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM4)
    {
        pwm_counter++;
        if (pwm_counter >= PWM_RESOLUTION)
            pwm_counter = 0;

        for (uint8_t i = 0; i < NUM_LEDS; i++)
        {
            if (pwm_counter < pwm_duty[i])
                HAL_GPIO_WritePin(ledPorts[i], ledPins[i], GPIO_PIN_SET); // LED ON (active low)
            else
                HAL_GPIO_WritePin(ledPorts[i], ledPins[i], GPIO_PIN_RESET);   // LED OFF
        }
    }
}
void TIM4_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim4);
}

void ChaseLedStep(uint8_t *indexl,int type) {
	// Spegni tutti
	for (int i = 0; i < 8; i++) {
		HAL_GPIO_WritePin_EX(ledPorts[i], ledPins[i], type);
	}
	// Accendi il LED corrente
	HAL_GPIO_WritePin_EX(ledPorts[*indexl], ledPins[*indexl], GPIO_PIN_SET);
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
		HAL_GPIO_WritePin_EX(ledPorts[i], ledPins[i], GPIO_PIN_RESET);
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
		HAL_GPIO_WritePin_EX(ledPorts[indici[i]], ledPins[indici[i]], GPIO_PIN_SET);
	}
}
typedef struct {
    char ascii;
    uint8_t keycode;
    uint8_t shift;
} hid_map_t;

#ifdef US_KEYBOARD
const hid_map_t hid_map[] = {
    {'a', 0x04, 0}, {'b', 0x05, 0}, {'c', 0x06, 0}, {'d', 0x07, 0},
    {'e', 0x08, 0}, {'f', 0x09, 0}, {'g', 0x0A, 0}, {'h', 0x0B, 0},
    {'i', 0x0C, 0}, {'j', 0x0D, 0}, {'k', 0x0E, 0}, {'l', 0x0F, 0},
    {'m', 0x10, 0}, {'n', 0x11, 0}, {'o', 0x12, 0}, {'p', 0x13, 0},
    {'q', 0x14, 0}, {'r', 0x15, 0}, {'s', 0x16, 0}, {'t', 0x17, 0},
    {'u', 0x18, 0}, {'v', 0x19, 0}, {'w', 0x1A, 0}, {'x', 0x1B, 0},
    {'y', 0x1C, 0}, {'z', 0x1D, 0},
    {' ', 0x2C, 0},
    {'/', 0x38, 0},
    {':', 0x33, 1},
    {'-', 0x2D, 0},
    {'>', 0x37, 1},
};
#else
const hid_map_t hid_map[] = {

	    {'a', 0x04, 0}, {'b', 0x05, 0}, {'c', 0x06, 0}, {'d', 0x07, 0},
	    {'e', 0x08, 0}, {'f', 0x09, 0}, {'g', 0x0A, 0}, {'h', 0x0B, 0},
	    {'i', 0x0C, 0}, {'j', 0x0D, 0}, {'k', 0x0E, 0}, {'l', 0x0F, 0},
	    {'m', 0x10, 0}, {'n', 0x11, 0}, {'o', 0x12, 0}, {'p', 0x13, 0},
	    {'q', 0x14, 0}, {'r', 0x15, 0}, {'s', 0x16, 0}, {'t', 0x17, 0},
	    {'u', 0x18, 0}, {'v', 0x19, 0}, {'w', 0x1A, 0}, {'x', 0x1B, 0},
	    {'y', 0x1C, 0}, {'z', 0x1D, 0},


	    {' ', 0x2C, 0},


	    {'.', 0x37, 0},
	    {'-', 0x2D, 0},
	    {':', 0x33, 1},   // SHIFT + ;
	    {'/', 0x38, 1},   // SHIFT + '
	    {'>', 0x64, 1},   // SHIFT + <
};
#endif

int find_hid(char c, uint8_t *keycode, uint8_t *shift) {
    for (int i = 0; i < sizeof(hid_map)/sizeof(hid_map[0]); i++) {
        if (hid_map[i].ascii == c) {
            *keycode = hid_map[i].keycode;
            *shift = hid_map[i].shift;
            return 1;
        }
    }
    return 0;
}
void send_string(const char *str) {
    uint8_t key, shift;

    while (*str) {
        if (find_hid(*str, &key, &shift)) {

            if (shift)
                send_keyboard_comb_report(0x02, key); // SHIFT + key
            else
                send_keyboard_report(key);

            HAL_Delay(10);
			send_keyboard_report(0x00);
            HAL_Delay(10);
        }
        str++;
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
			ChaseLedStep(&currentLed,2);
			HAL_Delay(100);
			break;
		case NONE_CASUAL:
			currentLed = rand() % 8;
			ChaseLedStep(&currentLed,2);
			HAL_Delay((rand() % 60) +1);
			break;
		case ANTI_HIBERNATION:
			ChaseLedStep(&currentLed,2);
			send_mouse_report(5, 0, 0); // Muove il mouse di 10 pixel
			HAL_Delay(10);
			ChaseLedStep(&currentLed,2);
			send_mouse_report(0, 5, 0); // Muove il mouse di 10 pixel
			HAL_Delay(10);
			ChaseLedStep(&currentLed,2);
			send_mouse_report(-5, 0, 0); // Muove il mouse di 10 pixel
			HAL_Delay(10);
			ChaseLedStep(&currentLed,2);
			send_mouse_report(0, -5, 0); // Muove il mouse di 10 pixel
			HAL_Delay(10);
			break;
		case LOCK_PC:
			ChaseLedStep(&currentLed,2);
			send_keyboard_comb_report(0x08,0x0F);
			HAL_Delay(100);
			break;
		case WRITE_WORD: //STONE RULER
				send_string("STONE RULER -> https://github.com/Socketto/RulerFirmware/releases");
				send_keyboard_report(0x28);
				HAL_Delay(10);
				send_keyboard_report(0x00);
				send_string("Firmware build: " __DATE__ " " __TIME__);
				send_keyboard_report(0x28);
				HAL_Delay(10);
				send_keyboard_report(0x00);
				send_string("STone -> the new development tool designed to solve the specific challenges of software programmers for HVAC/R units and systems");
				send_keyboard_report(0x28);
				HAL_Delay(10);
				send_keyboard_report(0x00);
				ChaseLedStep(&currentLed,2);
			break;
		case THROWING_THE_DICE:
			GestisciLedDado();
			HAL_Delay(100);
			break;
		case TRANSCEIVER:
			uint8_t uart_rx;

			if(break_value)
			{
			    for(int i=0;i<8;i++)
			    	HAL_GPIO_WritePin_EX(ledPorts[i], ledPins[i], GPIO_PIN_SET);
			    HAL_Delay(100);
			    for(int i=0;i<8;i++)
			    	HAL_GPIO_WritePin_EX(ledPorts[i], ledPins[i], GPIO_PIN_RESET);
			    HAL_Delay(100);
			    return;
			}

			if(rectran!=rectran_prec)
			{
				rectran_prec = rectran;
				ChaseLedStep(&currentLed,0);
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
	MX_TIM4_Init();  // Attiva PWM software
	IoInputSwitches = ReadSwitcher();

	switch(IoInputSwitches)
	{
	case ANTI_HIBERNATION:
		SetIsMouse(1);
		break; //mouse
	case WRITE_WORD:
	case LOCK_PC:
	case THROWING_THE_DICE:
		SetIsMouse(0);
		classUSB = 0;
		break; //keyboard
	case MODBUS:
	case TRANSCEIVER:
		classUSB = 1;
		break;
	default:
		SetIsMouse(0);
		classUSB = 0;
	break;
	}

	MX_TIM1_Init();
	MX_USART2_UART_Init();

	MX_USB_DEVICE_Init(classUSB);
	
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

static void MX_TIM4_Init(void)
{
    __HAL_RCC_TIM4_CLK_ENABLE();

    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 72 - 1;      // 1 MHz base clock (72MHz/72)
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;

    // Calcolo periodo per ottenere PWM_BASE_FREQ × PWM_RESOLUTION
    // Esempio: 200 Hz × 100 step = 20 kHz → periodo = 1000000 / 20000 = 50
    htim4.Init.Period = (1000000 / (PWM_BASE_FREQ * PWM_RESOLUTION)) - 1;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim4);

    HAL_TIM_Base_Start_IT(&htim4); // Avvia timer con interrupt
	HAL_NVIC_SetPriority(TIM4_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
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
