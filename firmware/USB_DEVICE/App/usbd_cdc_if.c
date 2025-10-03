/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v2.0_Cube
  * @brief          : Usb device for Virtual Com Port.
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
#include <stdbool.h>
#include "usbd_cdc_if.h"


#define NONE						0
#define ANTI_HIBERNATION			1
#define WRITE_WORD					2
#define LOCK_PC						3
#define THROWING_THE_DICE			4
#define TRANSCEIVER					5
#define MODBUS						6
#define NONE_CASUAL  			7

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */
/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

extern uint8_t IoInputSwitches;
// variabili globali
volatile LineCodingTypeDef NewLineCoding = {
    115200, // default baud
    0,      // stop bits = 1
    0,      // parity = none
    8       // data bits
};

volatile uint8_t uart_needs_update = 0;

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */



/*SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSs*/
void RS485_SetTransmit(void) {
    HAL_GPIO_WritePin(GPIOA, USART2_EN_Pin, GPIO_PIN_SET); // RE=1, DE=1
}

void RS485_SetReceive(void) {
    HAL_GPIO_WritePin(GPIOA, USART2_EN_Pin, GPIO_PIN_RESET); // RE=0, DE=0
}
/*SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSs*/


extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */

extern UART_HandleTypeDef huart2;
extern volatile unsigned char rectran;

static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */
  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

    case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

    case CDC_SET_COMM_FEATURE:

    break;

    case CDC_GET_COMM_FEATURE:

    break;

    case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
    case CDC_SET_LINE_CODING:
    	NewLineCoding.BaudRate = (uint32_t)(pbuf[0] | (pbuf[1] << 8) | (pbuf[2] << 16) | (pbuf[3] << 24));
    	NewLineCoding.StopBits = pbuf[4];
    	NewLineCoding.Parity   = pbuf[5];
    	NewLineCoding.DataBits = pbuf[6];
    	if(NewLineCoding.BaudRate != 0)
    	{
    		uart_needs_update = 1;  // Flag per il main loop
    	}
    break;

    case CDC_GET_LINE_CODING:

    break;

    case CDC_SET_CONTROL_LINE_STATE:

    break;

    case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
//static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
//{
//  //* USER CODE BEGIN 6 */
//  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
//  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
//  return (USBD_OK);
//  /* USER CODE END 6 */
//}*/



// Buffer globale per risposta Modbus (max 256 byte)
uint8_t modbus_response[256];
uint16_t modbus_response_len = 0;

// Struttura che rappresenta un frame Modbus decodificato
typedef struct {
    uint8_t address;
    uint8_t function;
    uint8_t data[252];   // spazio per i dati
    uint16_t data_len;
    uint16_t crc;
    bool crc_ok;
} ModbusFrame;

// Funzione CRC16 Modbus
uint16_t modbus_crc16(const uint8_t *buf, uint16_t len) {
    uint16_t crc = 0xFFFF;
    for (uint16_t pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos];
        for (int i = 0; i < 8; i++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// Funzione per preparare una risposta normale
void modbus_build_response(uint8_t address, uint8_t function, const uint8_t *data, uint16_t data_len) {
    modbus_response[0] = address;
    modbus_response[1] = function;
    for (uint16_t i = 0; i < data_len; i++) {
        modbus_response[2 + i] = data[i];
    }
    uint16_t crc = modbus_crc16(modbus_response, 2 + data_len);
    modbus_response[2 + data_len] = crc & 0xFF;       // CRC low
    modbus_response[3 + data_len] = (crc >> 8) & 0xFF; // CRC high
    modbus_response_len = 4 + data_len;
}

// Funzione per preparare una risposta di eccezione
void modbus_build_exception(uint8_t address, uint8_t function, uint8_t exception_code) {
    modbus_response[0] = address;
    modbus_response[1] = function | 0x80; // bit 7 = exception
    modbus_response[2] = exception_code;
    uint16_t crc = modbus_crc16(modbus_response, 3);
    modbus_response[3] = crc & 0xFF;
    modbus_response[4] = (crc >> 8) & 0xFF;
    modbus_response_len = 5;
}

extern GPIO_TypeDef* ledPorts[];
extern uint16_t ledPins[];
// Funzione di decodifica e risposta
bool decode_modbus_frame(const uint8_t *buffer, uint16_t len, ModbusFrame *frame) {
    if (len < 4) {
        return false;
    }

    frame->address = buffer[0];
    frame->function = buffer[1];
    frame->data_len = len - 4; // esclusi CRC e intestazione

    for (uint16_t i = 0; i < frame->data_len; i++) {
        frame->data[i] = buffer[2 + i];
    }

    frame->crc = buffer[len - 2] | (buffer[len - 1] << 8);
    uint16_t calc_crc = modbus_crc16(buffer, len - 2);
    frame->crc_ok = (calc_crc == frame->crc);

    if (!frame->crc_ok) {
        return false;
    }

    // Gestione funzioni
    switch (frame->function) {
        case 0x01: // Read Coils
          {
            uint8_t coil_status[2];
            coil_status[0] = 0x01; // numero di byte dati (8 coil = 1 byte)
            coil_status[1] = 0x00;

            for (int i = 0; i < 8; i++) {
                GPIO_PinState state = HAL_GPIO_ReadPin(ledPorts[i], ledPins[i]);
                if (state == GPIO_PIN_SET) {
                    coil_status[1] |= (1 << i); // set bit se coil attivo
                }
            }

            modbus_build_response(frame->address, frame->function, coil_status, 2);
          }
        break;

        case 0x05: // Write Single Coil
        {
            uint16_t coil_addr = (frame->data[0] << 8) | frame->data[1];
            uint16_t coil_val  = (frame->data[2] << 8) | frame->data[3];

            if (coil_addr < 8) {
                if (coil_val == 0xFF00) {
                    HAL_GPIO_WritePin(ledPorts[coil_addr], ledPins[coil_addr], GPIO_PIN_SET);
                } else if (coil_val == 0x0000) {
                    HAL_GPIO_WritePin(ledPorts[coil_addr], ledPins[coil_addr], GPIO_PIN_RESET);
                } else {
                    // valore illegale → exception
                    modbus_build_exception(frame->address, frame->function, 0x03); // Illegal Data Value
                    break;
                }
                // Risposta = echo della richiesta
                modbus_build_response(frame->address, frame->function, frame->data, 4);
            } else {
                // indirizzo coil fuori range
                modbus_build_exception(frame->address, frame->function, 0x02); // Illegal Data Address
            }
        }
        break;

        case 0x0F: // Write Multiple Coils
        {
            uint16_t start_addr = (frame->data[0] << 8) | frame->data[1];
            uint16_t quantity   = (frame->data[2] << 8) | frame->data[3];
   

            if ((start_addr + quantity) <= 8) {
                for (uint16_t i = 0; i < quantity; i++) {
                    uint8_t byte_index = 5 + (i / 8);
                    uint8_t bit_index = i % 8;
                    int coil = (frame->data[byte_index] >> bit_index) & 0x01;

                    HAL_GPIO_WritePin(ledPorts[start_addr + i],
                                      ledPins[start_addr + i],
                                      coil ? GPIO_PIN_SET : GPIO_PIN_RESET);
                }

                // Risposta = echo con start_addr + quantity
                uint8_t resp[4];
                resp[0] = frame->data[0];
                resp[1] = frame->data[1];
                resp[2] = frame->data[2];
                resp[3] = frame->data[3];
                modbus_build_response(frame->address, frame->function, resp, 4);
            } else {
                // fuori range
                modbus_build_exception(frame->address, frame->function, 0x02); // Illegal Data Address
            }
        }
        break;

        default:
            // Funzione non supportata
            modbus_build_exception(frame->address, frame->function, 0x01); // Illegal Function
            break;
    }

    return true;
}




extern UART_HandleTypeDef huart2;

static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  switch (IoInputSwitches)
  {
  case TRANSCEIVER:
        RS485_SetTransmit();
        rectran = 1;
          HAL_UART_Transmit(&huart2, Buf, *Len, 1000);  // evita HAL_MAX_DELAY
        // Aspetta la fine effettiva del frame
        //while (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TC) == RESET);
          RS485_SetReceive();

          // Riabilita la ricezione USB
          USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    break;
    case MODBUS:
      ModbusFrame frame;  
      if (decode_modbus_frame(Buf, sizeof(Buf), &frame)) 
      {
        CDC_Transmit_FS(modbus_response, modbus_response_len);
      } 
    break;
  default:
    break;
  }
    // Trasmetti i dati ricevuti su UART (RS-485)


    return (USBD_OK);
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  /* USER CODE END 7 */
  return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */
