/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * Copyright (c) 2016 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "fatfs.h"
#include "usb_device.h"

/* USER CODE BEGIN Includes */
#include "board.h"
#include "xprintf.h"
#include "conv.h"
#include "debug.h"
#include "ff.h"
FIL file_test;
FRESULT result;
FATFS FATFS_obj;
uint32_t str_size;
static const char file_path [] = "ADC_RAW_DATA.bin";
char _str [128];
uint8_t ch [256] = {"HELL\r\n"};
uint32_t tmpu32 = 0;
uint16_t tmpu16;

uint32_t test_core_ticks_new, test_core_ticks_old = 0;
uint32_t test_total_time_remain = 0;

UINT frcnt, fwcnt;         // счетчик записи файла
uint32_t ready_to_close = 0;
//#define show_message(a,b)

#include "modParser.h"

uint32_t ADC_flag_ready = 0;

uint32_t cnt_wr = 0;
uint32_t cnt_rd = 0;
uint32_t cnt_error_wr = 0;
uint32_t cnt_error_rd = 0;

volatile static const uint16_t compile_date []    = { 2016, 12, 8 };
volatile static const uint16_t compile_version [] = { 7 };
volatile static const uint16_t firmware_date []   = { 2016, 12, 8 };

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;
DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_adc2;

CRC_HandleTypeDef hcrc;

DAC_HandleTypeDef hdac;

RTC_HandleTypeDef hrtc;

SD_HandleTypeDef hsd;
HAL_SD_CardInfoTypedef SDCardInfo;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim12;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_ADC3_Init(void);
static void MX_CRC_Init(void);
static void MX_DAC_Init(void);
static void MX_SDIO_SD_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM12_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
                                

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

uint32_t error_cnt = 0;
uint8_t show_message (char *buf)
{
    uint32_t test_cnt = 0;
    uint16_t size;
    
    size = _strlen (buf);
    //memcpy ((void *)_str, buf, size);
    strcat ((char *)buf, "\r\n");
    size += 2;
    if (HAL_OK != HAL_UART_Transmit_IT (&huart1, (uint8_t *)&buf[0], size))
    {
        //do nothing
    }
    else
    {
repeat:
        if (USBD_OK != CDC_Transmit_FS((uint8_t *)&buf[0], size))
        {
            error_cnt++;
            if (++test_cnt >= 10)
            {
                return 1;
            }
            else
            {
                HAL_Delay (1); //~10ms
                goto repeat;
            }
        }
        else
        {

            return 0;
        }
    }
}


uint8_t show_message_c (const char *str)
{
    xsprintf (_str, "%s", str);
    show_message (_str);
}




typedef struct FIFO_t_ //struct
{
    int32_t in; // Next In Index
    int32_t out; // Next Out Index
    uint8_t *buf; // Buffer
    int32_t size;
} FIFO_t;

#define FIFO_POS     (s->in - s->out)

void FIFO_flush ( FIFO_t *s)
{
    s->in = s->out;
}


bool_t FIFO_init ( FIFO_t *s, uint8_t *buf, uint32_t FIFO_size)
{
    if ((NULL == buf) || 
        (FIFO_size < 2) || // FIFO size is too small.  It must be larger than 1
        ((FIFO_size & (FIFO_size - 1)) != 0)) //must be a power of 2
    {
        return FALSE;
    }
    else
    {
        __disable_irq ();
        s->buf = buf;
        s->size = FIFO_size;
        __enable_irq ();
        FIFO_flush (s);
        return TRUE;
    }
}


uint32_t FIFO_available ( FIFO_t *s) //TODO data size
{
    int32_t i;

    __disable_irq ();
    i = FIFO_POS;
    __enable_irq ();
    if (i < 0)
      i = -i;
    return (uint32_t)i;
}


bool_t FIFO_get ( FIFO_t *s, uint8_t *c) //TODO data size
{
    if (FIFO_available (s) > 0)
    {
        __disable_irq ();
        *c = (s->buf [(s->out) & (s->size - 1)]);
        s->out++;
        __enable_irq ();
        return  TRUE;
    }
    return FALSE;
}


bool_t FIFO_put ( FIFO_t *s, uint8_t c) 
{
    __disable_irq ();
    if (FIFO_POS < s->size) // If the buffer is full, return an error value
    {
        s->buf [s->in & (s->size - 1)] = c; 
        s->in++;
        __enable_irq ();
        return TRUE;
    }
    __enable_irq ();
    return FALSE;
}


bool_t FIFO_gets ( FIFO_t *s, uint8_t *c, uint32_t data_size) //TODO data size
{
    uint32_t i;

    if (NULL == c)
        return FALSE;
    if (data_size > s->size)
        return FALSE;
    if (FIFO_available (s) >= data_size)
    {
        __disable_irq ();
        for (i = 0; i < data_size; i++)
        {
            c [i] = (s->buf [(s->out) & (s->size - 1)]);
            s->out++;
        }
        __enable_irq ();
        return TRUE;
    }
    return FALSE;
}


bool_t FIFO_puts ( FIFO_t *s, uint8_t *c, uint32_t data_size) 
{
    uint32_t i;
    
    if (NULL == c)
        return FALSE;
    if (data_size > s->size)
        return FALSE;
    if (s->size - FIFO_available (s) >= data_size) // If the buffer is full, return an error value
    {
        __disable_irq ();
        for (i = 0; i < data_size; i++)
        {
            s->buf [s->in & (s->size - 1)] = c [i]; 
            s->in++;
        }
        __enable_irq ();
        return TRUE;
    }
    return FALSE;
}




static FIFO_t FIFO_UART_RX; //TODO malloc
#define FIFO_UART_RX_SIZE          128 /*** Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/
static uint8_t FIFO_UART_RX_buf [FIFO_UART_RX_SIZE];


static FIFO_t FIFO_ADC; //TODO malloc
#define FIFO_ADC_SIZE              4096 /*** Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/
static uint8_t FIFO_ADC_buf [FIFO_ADC_SIZE];



//------------------------------------------------------------------------------


#define ADC_BUF_SIZE                4
#define ADC_DATA_COLLECT            1000000 // all data = ADC_BUF_SIZE * ADC_DATA_COLLECT


#pragma pack(push, 1) //for pucked data structures
struct parser_data_t {
    uint8_t     start; // позиция внутреннего автомата, не трогать
    uint8_t     adress; // адрес, на который нужно ответить
    uint8_t     reciv_cnt;
    uint16_t    adc1_value_out [ADC_BUF_SIZE];
    uint16_t    adc2_value_out [ADC_BUF_SIZE];
    //uint32_t     timer;
    uint32_t    cnt;
    uint16_t    crc;
    uint8_t     end;
} parser_data;
#pragma pack(pop) 

#define ADC_BUF_ALL_SIZE (ADC_BUF_SIZE * 2 )
#define PARSER_OUT_SIZE     (10+ ADC_BUF_ALL_SIZE) //size our packet



//__IO uint16_t ADC1_buf [ADC_BUF_SIZE];
//__IO uint16_t ADC2_buf [ADC_BUF_SIZE];
__IO uint16_t ADC3_buf [ADC_BUF_SIZE];
uint16_t ADC3_pointer = 0;

enum { ADC_BUFFER_LENGTH = 10 };
uint32_t g_ADCBuffer [ADC_BUFFER_LENGTH];
uint16_t g_ADCValue;
uint32_t g_MeasurementNumber = 0;

void HAL_ADC_ConvCpltCallback (ADC_HandleTypeDef* hadc) //TODO check that adc1 first!!!
{
    if (hadc->Instance == ADC1)
    {
        /*
        for (i = 0; i < ADC_BUFFER_LENGTH; i++)
        {
            tmp32 += ADC1_buf [i];
        }
        parser_data.adc_value_out [0] =  tmp32 / ADC_BUF_SIZE; //std::accumulate(g_ADCBuffer, g_ADCBuffer + ADC_BUFFER_LENGTH, 0) / ADC_BUFFER_LENGTH;
        //g_MeasurementNumber += 1;//ADC_BUFFER_LENGTH;
       */
        /*
        for (i = 0; i < ADC_BUFFER_LENGTH; i++)
        {
            parser_data.adc_value_out [0 + i] = ADC1_buf [i];
        }
        */
    }
    
    if (hadc->Instance == ADC2)
    {
        /*
        for (i = 0; i < ADC_BUFFER_LENGTH; i++)
        {
            parser_data.adc_value_out [10 + i] = ADC2_buf [i];
        }
        */
        //g_MeasurementNumber += 1;
        //parser_data.cnt = g_MeasurementNumber;

        //crc...

        //if (USBD_OK != CDC_Transmit_FS ((uint8_t *)&parser_data, PARSER_OUT_SIZE))
        {
        }
        
        if (ADC_flag_ready > 0)
        {
            ADC_flag_ready++;

            cnt_wr++;
            if (TRUE != FIFO_puts (&FIFO_ADC, (uint8_t *)&parser_data.adc1_value_out [0], ADC_BUF_ALL_SIZE))
            {
                cnt_error_wr++;
            }

            if (ADC_flag_ready >= (ADC_DATA_COLLECT +1)) //ТОМУ ЩО
            {
                HAL_TIM_Base_Stop (&htim2); 
            }
        }
    }
}


typedef enum {
    FMT_CMD_TEST                    = 'T',
    FMT_CMD_GET_DEV_ID              = 'I',
    FMT_CMD_GET_FIRMWARE_VER        = 'V',
   
    FMT_CMD_RESET                   = 'R', //not implemented!
        
    FMT_CMD_ADC_START               = '1',
    FMT_CMD_ADC_SET_CHANNEL         = 'C',
    FMT_CMD_ADC_SET_FRQ             = 'F',
    FMT_CMD_ADC_SET_BUF_SIZE        = 'S',
    FMT_CMD_ADC_STOP                = '2',
    
    FMT_CMD_ADC_GET_VBAT            = 'B',
    
    FMT_CMD_DAC_SET_CHANNEL_VALUE   = 'D',
    
} FMT_CMD;


parser_t parser;

__IO ITStatus UartReady = RESET;
/* Buffer used for reception */
uint8_t aRxBuffer [128];


void HAL_UART_TxCpltCallback (UART_HandleTypeDef *UartHandle)
{
  /* Set transmission flag: transfer complete */
    UartReady = SET;
}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of DMA Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_RxCpltCallback (UART_HandleTypeDef *UartHandle)
{
    /* Set transmission flag: transfer complete */
    FIFO_put (&FIFO_UART_RX, aRxBuffer [0]);
    UartReady = SET;
    if (HAL_UART_Receive_IT (&huart1, (uint8_t *)aRxBuffer, 1) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
  /* Turn LED_ORANGE on: Transfer error in reception/transmission process */
  //BSP_LED_On(LED_ORANGE); 
  //Error_Handler();
}

#include "modParser.h"
uint8_t tmpchar;
uint8_t respBuf [PARSER_MAX_DATA_SIZE];
uint8_t IOBuf [PARSER_MAX_DATA_SIZE];
uint8_t size8;

static uint32_t *UID = (uint32_t *)0x1FFF7A10; // 0x1FFFF7E8


// for drvSPIFLASH -----------------------------------------
void halSPIFLASH_GPIO_SPI_init (void) //already init ready
{
    __HAL_SPI_ENABLE (&hspi1);
}


uint8_t halSPIFLASH_xspi (uint8_t byte)
{
    while (0 == (SPI1->SR & SPI_SR_TXE)) {};
    SPI1->DR = byte;
    while (!(SPI1->SR & SPI_SR_RXNE)) {};
    //HAL_Delay (1);
    return SPI1->DR;
}
//-----------------------------------------------------------


void drvBUZZER_on (void)
{
    if (HAL_TIM_PWM_Start (&htim12, TIM_CHANNEL_2) != HAL_OK)
    {
    }
}

void drvBUZZER_off (void)
{
    if (HAL_TIM_PWM_Stop (&htim12, TIM_CHANNEL_2) != HAL_OK)
    {
    }
}

void drvBUZZER_peep (uint8_t i)
{
    while (i)
    {
        drvBUZZER_on (); //end init pika
        HAL_Delay (40);
        drvBUZZER_off ();
        HAL_Delay (90);
        i--;
    }
}


void FAT_reinit (void)
{
    // CD card off-on!
    HAL_GPIO_WritePin (GPIOC, GPIO_SD_ONOFF_Pin, GPIO_PIN_SET);
    HAL_Delay (100);
    HAL_GPIO_WritePin (GPIOC, GPIO_SD_ONOFF_Pin, GPIO_PIN_RESET);
    
    result = f_mount (&FATFS_obj, "0", 1);
    if ((FR_NOT_READY == result) ||
        (FR_DISK_ERR == result) ||
        (FR_INVALID_DRIVE == result))
    {
        show_message_c ("   SD/FAT init ERROR");
        drvBUZZER_peep (1);
    }
    else
    {
        
    }
    
    tmpu32 = 0;
    if (1 == tmpu32)
    {
        result = f_mkfs ("0", 0, 512);
        if (FR_OK != result)
        {
            show_message_c ("SD/FAT mount disk");
        }
        else
        {
            drvBUZZER_peep (2);
        }
    }
    /*
    if (FR_NO_FILESYSTEM == result)
    {
        result = f_mkfs ("0", 0, 512);
        if (FR_OK != result)
        {
            show_message_c ("FAT reinit");
        }
    }
    */
    
    if (FR_OK == result)
    {
        show_message_c ("   SD/FAT init OK");
        //...
    }
}


/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_ADC3_Init();
  MX_CRC_Init();
  MX_DAC_Init();
  MX_SDIO_SD_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_USB_DEVICE_Init();
  MX_TIM2_Init();
  MX_FATFS_Init();
  MX_RTC_Init();
  MX_TIM12_Init();

  /* USER CODE BEGIN 2 */
ALL_RESET:
    _debug_init ();
    
    drvSPIFLASH_init (0);

    // init CDC out struct
    parser_data.start = PIK_START;
    parser_data.reciv_cnt = 8;
    parser_data.crc = 0x4545;
    parser_data.end = PIK_FIN;

    modParser_init (&parser, '#'); //'#' - address
    modParser_reset (&parser);
   
    //TODO check init
    FIFO_init (&FIFO_UART_RX, &FIFO_UART_RX_buf[0], FIFO_UART_RX_SIZE);
    FIFO_flush (&FIFO_UART_RX);
    
    FIFO_init (&FIFO_ADC, &FIFO_ADC_buf[0], FIFO_ADC_SIZE);
    FIFO_flush (&FIFO_ADC);
    
    //TIM_ClockConfigTypeDef sClockSourceConfig;
    //TIM_MasterConfigTypeDef sMasterConfig;
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 168 -1;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 10;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    if (HAL_TIM_Base_Init (&htim2) != HAL_OK)
    {
        Error_Handler();
    }

    // step (1) - start TIMER
    HAL_TIM_Base_Start (&htim2);
    
    // step (2) - start ADC
    if (HAL_ADC_Start_DMA (&hadc1,
        (uint32_t *)&parser_data.adc1_value_out [0],
        ADC_BUF_SIZE) != HAL_OK)
    {
        
    }
        
    if (HAL_ADC_Start_DMA (&hadc2,
        (uint32_t *)&parser_data.adc2_value_out [0],
        ADC_BUF_SIZE) != HAL_OK)
    {
        
    }
    
    
    //DAC on
    HAL_DAC_SetValue (&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0);
    HAL_DAC_SetValue (&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 0);
    uint32_t DACcnt = 0;
    if (HAL_DAC_Start (&hdac, DAC_CHANNEL_1) != HAL_OK)
    {
    }
    if (HAL_DAC_Start (&hdac, DAC_CHANNEL_2) != HAL_OK)
    {
    }
   
    __enable_irq (); // enable interrupts!!!
    
    if (HAL_UART_Transmit_IT (&huart1, (uint8_t *)"\r\n\r\nInit start\r\n", 6)!= HAL_OK)
    {
        Error_Handler();
    }
  
  
    if (HAL_UART_Receive_IT (&huart1, (uint8_t *)aRxBuffer, 1) != HAL_OK)
    {
        Error_Handler();
    }
    
    HAL_Delay (1000); //for init SD-CARD and not often mount\open\etc

    // CD card on!
    HAL_GPIO_WritePin (GPIOC, GPIO_SD_ONOFF_Pin, GPIO_PIN_RESET);
    
    result = f_mount (&FATFS_obj, "0", 1);
    if ((FR_NOT_READY == result) ||
        (FR_DISK_ERR == result) ||
        (FR_INVALID_DRIVE == result))
    {
        show_message_c ("   SD/FAT init ERROR");
        drvBUZZER_peep (1);
    }
    else
    {
        
    }
    
    tmpu32 = 0;
    if (1 == tmpu32)
    {
        result = f_mkfs ("0", 0, 512);
        if (FR_OK != result)
        {
            show_message_c ("SD/FAT mount disk");
        }
        else
        {
            drvBUZZER_peep (2);
        }
    }
    /*
    if (FR_NO_FILESYSTEM == result)
    {
        result = f_mkfs ("0", 0, 512);
        if (FR_OK != result)
        {
            show_message_c ("FAT reinit");
        }
    }
    */
    
    if (FR_OK == result)
    {
        show_message_c ("   SD/FAT init OK");
        //...
    }
    
    test_core_ticks_new = 0;
    test_core_ticks_old = 0;
    
    drvBUZZER_peep (1); //end init
    show_message_c ("Init stop");
    show_message_c ("---------------------------------------------");
    while (1)
    {
        RESET_CORE_COUNT;
 
        if (TRUE == FIFO_gets (&FIFO_UART_RX, &tmpchar, 1)) 
        {
            if (FUNCTION_RETURN_OK == modParser_reciv (&parser, tmpchar, &IOBuf[0], &size8))
            {
                // parsing command
                switch (IOBuf [0])
                {
                case FMT_CMD_TEST:
                    size8 = xsprintf (_str, "[%u.%u]%u ", HAL_GetTick (), test_core_ticks_old);
                    break;
                    
                case FMT_CMD_GET_DEV_ID:
                    size8 = xsprintf (_str, "ID:0x%08X%08X%08X ", UID [0], UID [1], UID [2]);
                    break;
                    
                case FMT_CMD_GET_FIRMWARE_VER:
                    size8 = xsprintf (_str, "date %u.%u.%u v[%u] ", compile_date [0], compile_date [1], compile_date [2], compile_version [0]);
                    break;
                    
                case FMT_CMD_RESET:
                    //goto ALL_RESET; //*(uint32_t *)0xFFFFFFFF = 55;; //to hardcore but TRUE!
                    
                    FAT_reinit ();
                    
                    break;

                case FMT_CMD_ADC_GET_VBAT:
                    ADC_ChannelConfTypeDef sConfig;
                    if (IOBuf [1] == '2')
                    {
                        sConfig.Channel = ADC_CHANNEL_2;
                        sConfig.Rank = 1;
                        sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
                        if (HAL_ADC_ConfigChannel (&hadc3, &sConfig) != HAL_OK)
                        {
                            Error_Handler();
                        }
                        HAL_ADC_Start (&hadc3);
                        HAL_ADC_PollForConversion (&hadc3, 1000);
                        if (++ADC3_pointer >= ADC_BUF_SIZE)
                        {
                            ADC3_pointer = 0;
                        }
                        tmpu16 = HAL_ADC_GetValue (&hadc3);
                        //ADC3_buf [ADC3_pointer] = tmpu16;
                        size8 = xsprintf (_str, "ADC3 ch%c :%u", IOBuf [1], tmpu16);
                    }
                    else if (IOBuf [1] == '3')
                    {
                        sConfig.Channel = ADC_CHANNEL_3;
                        sConfig.Rank = 1;
                        sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
                        if (HAL_ADC_ConfigChannel (&hadc3, &sConfig) != HAL_OK)
                        {
                            Error_Handler();
                        }
                        HAL_ADC_Start (&hadc3);
                        HAL_ADC_PollForConversion (&hadc3, 1000);
                        if (++ADC3_pointer >= ADC_BUF_SIZE)
                        {
                            ADC3_pointer = 0;
                        }
                        tmpu16 = HAL_ADC_GetValue (&hadc3);
                        //ADC3_buf [ADC3_pointer] = tmpu16;
                        size8 = xsprintf (_str, "ADC3 ch%c :%u", IOBuf [1], tmpu16);
                    }
                    else
                    {
                        size8 = xsprintf (_str, "ADC3 channel error!");
                        drvBUZZER_peep (1);
                    }
                    break;
                    
                case FMT_CMD_DAC_SET_CHANNEL_VALUE: //Dac set
                    if (1 == IOBuf [1])
                        HAL_DAC_SetValue (&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (uint16_t)(IOBuf [2] << 8) | IOBuf [3]);
                    if (2 == IOBuf [1])
                        HAL_DAC_SetValue (&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, (uint16_t)(IOBuf [2] << 8) | IOBuf [3]);
                    size8 = xsprintf (_str, "OK ");
                    break;
                    
               case FMT_CMD_ADC_START: // start collect adc data
                   
                    result = f_open (&file_test, file_path, FA_WRITE);
                    if (FR_OK != result)
                    {
                        result = f_open (&file_test, file_path, FA_CREATE_ALWAYS | FA_WRITE);
                        if (FR_OK != result)
                        {
                            show_message_c ("   SD/FAT file fist creating error!");
                            drvBUZZER_peep (3);
                            //while (1) ;
                        }
                        else
                        {
                            show_message_c ("   SD/FAT file fist creating");
                        }
                    }
                    show_message_c ("   FILE open OK");
                    //...
        
                    result = f_lseek (&file_test, 0);
                    if (FR_OK != result)
                    {
                        show_message_c ("   FILE seek ERROR");
                        //while (1) {};
                    }
                    else
                    {
                        show_message_c ("   FILE seek OK");
                        test_total_time_remain = HAL_GetTick ();
                        xsprintf (_str, "[%u.%u] START ADC COLLECT DATA", test_total_time_remain, SysTick->VAL);
                        show_message (_str);
                        ADC_flag_ready = 1; //START!!!!
                        uint32_t delay_blink = HAL_GetTick();
                        while (1)
                        {
                            if ((HAL_GetTick() - delay_blink) >= 5000) //status output
                            {
                                delay_blink = HAL_GetTick();
                                if (0 != ADC_flag_ready)
                                {
                                    xsprintf (_str, "%u0 * 2 collect...", ADC_flag_ready);
                                    show_message (_str);
                                }
                            }
                            if (TRUE == FIFO_gets (&FIFO_ADC, (uint8_t *)_str, ADC_BUF_ALL_SIZE))
                            {
                                cnt_rd++;
                                result = f_write (&file_test, (uint8_t *)&_str[0], ADC_BUF_ALL_SIZE, &fwcnt); // Запись куска в файл
                                if ((FR_OK != result) || (fwcnt < str_size)) //ошибка, если диск переполнен
                                {
                                    show_message_c ("FILE wr ERROR");
                                    result = f_close (&file_test); //Update file size in here
                                    if (result != FR_OK)
                                    {
                                        show_message_c ("FILE close ERROR");
                                    }
                                    while (1) {};
                                }
                                if (ADC_flag_ready >= (ADC_DATA_COLLECT + 1)) //ТОМУ ЩО
                                {
                                    result = f_close (&file_test); //Update file size in here
                                    if (result != FR_OK)
                                    {
                                        show_message_c ("FILE close ERROR");
                                    }
                                    xsprintf (_str, "[%u.%u] ADC END COLLECT %u DATA", HAL_GetTick () - test_total_time_remain, SysTick->VAL, (ADC_flag_ready-1));
                                    show_message (_str);
                                    ADC_flag_ready = 0;
                                    drvBUZZER_peep (1);
                                    break; //while (1) {};
                                }
                            }
                        };
                        /*
                        str_size = 4;//xsprintf (_str, file_kml_head);
                        result = f_write (&fileKML, _str, str_size, &fwcnt); // Запись куска в файл
                        if ((FR_OK != result) || (fwcnt < str_size)) //ошибка, если диск переполнен
                        {
                            show_message ("FILE wr ERROR", 24);
                            while (1) {};
                        }
                        else
                        {
                            // emergency out!!!
                            break;
                        }
                        */
                    } //else 
                    size8 = xsprintf (_str, "ADC collect data ready!:%u ", tmpu16);
                    break;
            
                default: 
                    size8 = xsprintf (_str, "ERROR ");
                    break;
                }
                // create respond
                modParser_transmit (&parser, (uint8_t *)_str, size8, &IOBuf [0], &size8);
                
                //respond
                if (HAL_UART_Transmit_IT (&huart1, &IOBuf[0], size8)!= HAL_OK)
                {
                    Error_Handler();
                }
            }//if (HAL_OK != HAL_UART_Transmit (&huart1, (uint8_t *)&buf[0], size, 0xFFFF))
            
        }
        
        
        test_core_ticks_new = GET_CORE_COUNT;
        if (test_core_ticks_new > test_core_ticks_old)
        {
            test_core_ticks_old = test_core_ticks_new;
        }
        
        //so slow funktions
        /*
        xsprintf (_str, "[%u.%u] CNT:%u", HAL_GetTick (), SysTick->VAL);
        show_message (_str);
		HAL_Delay (3000);
        */
    }
    
    
    while (1)
    {

	
        
        
	} //while (1)
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
      

	  

  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

}

/* ADC2 init function */
static void MX_ADC2_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.ScanConvMode = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc2.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DMAContinuousRequests = ENABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

}

/* ADC3 init function */
static void MX_ADC3_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.ScanConvMode = DISABLE;
  hadc3.Init.ContinuousConvMode = ENABLE;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DMAContinuousRequests = DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

}

/* CRC init function */
static void MX_CRC_Init(void)
{

  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }

}

/* DAC init function */
static void MX_DAC_Init(void)
{

  DAC_ChannelConfTypeDef sConfig;

    /**DAC Initialization 
    */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }

    /**DAC channel OUT1 config 
    */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

    /**DAC channel OUT2 config 
    */
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }

}

/* RTC init function */
static void MX_RTC_Init(void)
{

  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
  RTC_AlarmTypeDef sAlarm;

    /**Initialize RTC Only 
    */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initialize RTC and set the Time and Date 
    */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_DECEMBER;
  sDate.Date = 0x1;
  sDate.Year = 0x16;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

    /**Enable the Alarm A 
    */
  sAlarm.AlarmTime.Hours = 0x0;
  sAlarm.AlarmTime.Minutes = 0x0;
  sAlarm.AlarmTime.Seconds = 0x0;
  sAlarm.AlarmTime.SubSeconds = 0x0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 0x1;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

}

/* SDIO init function */
static void MX_SDIO_SD_Init(void)
{

  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.ClockDiv = 2;

}

/* SPI1 init function */
static void MX_SPI1_Init(void)
{

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }

}

/* TIM2 init function */
static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 16800;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 10000;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

}

/* TIM12 init function */
static void MX_TIM12_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim12.Instance = TIM12;
  htim12.Init.Prescaler = 839;
  htim12.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim12.Init.Period = 100;
  htim12.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim12) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim12, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_Init(&htim12) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 25;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim12, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim12);

}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin : GPIO_SDIO_CD_Pin */
  GPIO_InitStruct.Pin = GPIO_SDIO_CD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIO_SDIO_CD_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO_SPI_25DF_CS_Pin GPIO_SD_ONOFF_Pin */
  GPIO_InitStruct.Pin = GPIO_SPI_25DF_CS_Pin|GPIO_SD_ONOFF_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : GPIO_WIFI_ONOFF_Pin */
  GPIO_InitStruct.Pin = GPIO_WIFI_ONOFF_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIO_WIFI_ONOFF_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_SPI_25DF_CS_Pin|GPIO_SD_ONOFF_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIO_WIFI_ONOFF_GPIO_Port, GPIO_WIFI_ONOFF_Pin, GPIO_PIN_RESET);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
