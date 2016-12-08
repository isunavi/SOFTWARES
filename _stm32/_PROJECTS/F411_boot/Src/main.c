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
#include "usb_device.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
//volatile static const uint16_t compile_date[] = { 2016, 9, 12 };
volatile static const char compile_version[] = "2.2.4";
volatile static const char compile_date[12] = __DATE__;
volatile static const char compile_time[10] = __TIME__;
volatile static const char device_descriptor[] = { "BOOTER F411" };
#include "board.h"
#include "debug.h"
#include "ff.h"
FIL file_test;
FRESULT result;
FATFS FATFS_obj;
uint32_t str_size;
static const char file_patch[] = "ADC_RAW_DATA.bin";
char _str[128];
uint8_t ch[256] = {"HELL\r\n"};
uint32_t tmpu32 = 0;
UINT frcnt, fwcnt;         // счетчик записи файла
uint32_t ready_to_close = 0;
//for debug
uint32_t test_core_ticks_new, test_core_ticks_old = 0;
uint32_t test_total_time_remain = 0;

#define DRIVE_LABEL_OFFSET (0x2000)
uint8_t driveName[11] = "BOOT_LOADER";
uint32_t pageBuf[512];

/*
 * Variables for all file systems and file objects
 */
FRESULT	FATFS_Status = FR_NOT_READY, FILE_Status = FR_NO_FILE;
FATFS	FATFS_Obj;
UINT	readBytes;

/*
 * Variables for application file
 */
#define APP_BLOCK_TRANSFER_SIZE	512
FIL		appFile;
uint32_t appSize;
uint32_t		appBodySize, appTailSize, appAddrPointer;
uint8_t		appBuffer[APP_BLOCK_TRANSFER_SIZE];

uint32_t i, j, crc32;
uint8_t debugOut;
static const uint32_t AppCRC = 0x12345678;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
static void MX_GPIO_Init(void);
static void MX_CRC_Init(void);
static void MX_SPI1_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

void GoToUserApp(void)
{
	uint32_t appJumpAddress;
	void (*GoToApp)(void);

	appJumpAddress = *((volatile uint32_t*)(FLASH_USER_START_ADDR + 4));
	GoToApp = (void (*)(void))appJumpAddress;
	SCB->VTOR = FLASH_USER_START_ADDR;
	__set_MSP(*((volatile uint32_t*) FLASH_USER_START_ADDR)); //stack pointer (to RAM) for USER app in this address
	GoToApp();
}

void CopyAppToUserMemory(void)
{
	f_lseek(&appFile, 0); //Go to the fist position of file

	appTailSize = appSize % APP_BLOCK_TRANSFER_SIZE;
	appBodySize = appSize - appTailSize;
	appAddrPointer = 0;
#if DEBUG
	if(debugOut == SET)
	{
		printf("File size = %d byte\r\nBody size = %d byte\r\nTail size = %d byte\r\n\r\n", appSize, appBodySize, appTailSize);
	}
#endif
	for(i = 0; i < ((appSize / FLASH_PAGE_SIZE) + 1); i++) //Erase n + 1 pages for new application
	{
		//while(FLASH_GetStatus() != FLASH_COMPLETE);
        if (HAL_OK != FLASH_WaitForLastOperation (1000))
        {
            while (1);
        }
		FLASH_Erase_Sector (FLASH_USER_START_ADDR + i * FLASH_PAGE_SIZE, FLASH_VOLTAGE_RANGE_3);
#if DEBUG
		if(debugOut == SET)
		{
			printf("Sector %3d (0x%08X - 0x%08X) erased\r\n", 	i,
																FLASH_USER_START_ADDR + i * FLASH_PAGE_SIZE,
																FLASH_USER_START_ADDR + (i + 1) * FLASH_PAGE_SIZE);
		}
#endif
	}
#if DEBUG
	if(debugOut == SET)
	{
		printf("\r\n");
	}
#endif
	for(i = 0; i < appBodySize; i += APP_BLOCK_TRANSFER_SIZE)
	{
		/*
		 * For example, size of File1 = 1030 bytes
		 * File1 = 2 * 512 bytes + 6 bytes
		 * "body" = 2 * 512, "tail" = 6
		 * Let's write "body" and "tail" to MCU FLASH byte after byte with 512-byte blocks
		 */
		FILE_Status = f_read (&appFile, appBuffer, APP_BLOCK_TRANSFER_SIZE, &readBytes); //Read 512 byte from file
#if DEBUG
		if(debugOut == SET)
		{
			printf("%d cycle, read status = %d, %d byte read\r\n", i / APP_BLOCK_TRANSFER_SIZE, FILE_Status, readBytes);
		}
#endif
		for(j = 0; j < APP_BLOCK_TRANSFER_SIZE; j += 4)
		{
			//while(FLASH_GetStatus() != FLASH_COMPLETE);
            if (HAL_OK != FLASH_WaitForLastOperation (1000))
            {
                while (1);
            }
//			FLASH_Program_Word (FLASH_USER_START_ADDR + i + j, *((volatile uint32_t*)(appBuffer + j))); //write 512 byte to FLASH
		}
#if DEBUG
		if(debugOut == SET)
		{
			printf("%d byte programmed: ", j);
			printf("0x%08X - 0x%08X\r\n",	FLASH_USER_START_ADDR + appAddrPointer,
											FLASH_USER_START_ADDR + appAddrPointer + APP_BLOCK_TRANSFER_SIZE);
		}
#endif
		appAddrPointer += APP_BLOCK_TRANSFER_SIZE; //pointer to current position in FLASH for write
	}

	FILE_Status = f_read(&appFile, appBuffer, appTailSize, &readBytes); //Read "tail" that < 512 bytes from file
#if DEBUG
	if(debugOut == SET)
	{
		printf("Tail read: read status = %d, %d byte read, size of tail = %d\r\n", FILE_Status, readBytes, appTailSize);
	}
#endif
	while((appTailSize % 4) != 0)		//if appTailSize MOD 4 != 0 (seems not possible, but still...)
	{
		appTailSize++;						//increase the tail to a multiple of 4
		appBuffer[appTailSize - 1] = 0xFF;	//and put 0xFF in this tail place
	}
#if DEBUG
	if(debugOut == SET)
	{
		printf("New size of tail = %d\r\n", appTailSize);
	}
#endif
	for(i = 0; i < appTailSize; i += 4)
	{
		//while(FLASH_GetStatus() != FLASH_COMPLETE);
        if (HAL_OK != FLASH_WaitForLastOperation (1000))
        {
            while (1);
        }
///		FLASH_Program_Word (FLASH_USER_START_ADDR + appAddrPointer + i, *((volatile uint32_t*)(appBuffer + i))); //write "tail" to FLASH
	}
#if DEBUG
	if(debugOut == SET)
	{
		printf("%d byte programmed: ", appTailSize);
		printf("0x%08X - 0x%08X\r\n", FLASH_USER_START_ADDR + appAddrPointer, FLASH_USER_START_ADDR + appAddrPointer + i);
		printf("\r\n");
	}
#endif
}

void PeriphDeInit(void)
{
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, DISABLE);

#if DEBUG
	if(debugOut == SET)
	{
		//while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}
		//USART_Cmd(USART1, DISABLE);
		//RCC_APB2PeriphResetCmd(USART1_CLK | USART1_CLK_PINS, DISABLE);
	}
#endif
    //HAL_PCD_MspDeInit ();
	//RCC_APB2PeriphClockCmd(BUTTON_CLK, DISABLE);
}




uint8_t halSPIFLASH_xspi (uint8_t byte)
{
    uint8_t tx_buf[4];
    uint8_t rx_buf[4];
    
    tx_buf [0] = byte;
    HAL_SPI_TransmitReceive (&hspi1, &tx_buf[0], &rx_buf[0], 1, 100);
    return rx_buf[0];
    
    while ((SPI1->SR & SPI_SR_TXE) == 0) {}; //wait for buffer NOT empty
    SPI1->DR =  byte;
    while ((SPI1->SR & SPI_SR_RXNE) == 0) { __NOP(); }; //wait for buffer empty
    byte = SPI1->DR;
    return byte;
}


void halSPIFLASH_spi_rd (uint8_t *buf, uint16_t len)
{
    HAL_SPI_Receive (&hspi1, &buf[0], len, 1000);
}


void halSPIFLASH_spi_wr (uint8_t *buf, uint16_t len)
{
    HAL_SPI_Transmit (&hspi1, &buf[0], len, 1000);
}


void halSPIFLASH_GPIO_SPI_init (void)
{
    //SPI1->CR1 |=  SPI_CR1_SPE; // enable the SPI peripheral
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
  MX_CRC_Init();
  MX_USB_DEVICE_Init();
  MX_SPI1_Init();

  /* USER CODE BEGIN 2 */
    debugOut = SET;
    //LED_BLUE_OFF;
    //SPIFLASH_ID = DBGMCU->IDCODE;
    
    if (0 == (GPIOA->IDR & GPIO_PIN_1)) //Bootloader or Mass Storage?
	{
		//LED_RGB_Config();
		//USB_Config();
		//Interrupts_Config();
		//USB_Init();
        GoToUserApp();
		while (1); {}
	}
    
    
#if DEBUG
	if(debugOut == SET)
	{
		printf("BOOTLOADER Mode...\r\n");
	}
#endif
	FATFS_Status = f_mount(&FATFS_Obj, "0", 1);
#if DEBUG
	if(debugOut == SET)
	{
		printf("FAT FS mount status = %d\r\n", FATFS_Status);
	}
#endif
	if(FATFS_Status == FR_OK)
	{
		FILE_Status = f_open(&appFile, "/APP.BIN", FA_READ);
#if DEBUG
		if(debugOut == SET)
		{
			printf("Application file open status = %d\r\n", FILE_Status);
		}
#endif
		if(FILE_Status == FR_OK)
		{
			appSize = f_size(&appFile);
            
			for(i = 0; i < appSize; i++) //Byte-to-byte compare files in MSD_MEM and USER_MEM
			{
				f_read(&appFile, &appBuffer, 1, &readBytes);

				if (*((volatile uint8_t*)(FLASH_USER_START_ADDR + i)) != appBuffer[0]) //if byte of USER_MEM != byte of MSD_MEM
				{
					break;
				}
			}

			if(i != appSize) //=> was done "break" instruction in for(;;) cycle => new firmware in MSD_FLASH
			{
#if DEBUG
				if(debugOut == SET)
				{
					printf("Difference between MSD_MEM and USER_MEM: %d byte from %d byte\r\n", i, appSize);
					printf("Start copy MSD_MEM to USER_MEM:\r\n\r\n");
				}
#endif
				CopyAppToUserMemory();
			}
			else
			{
#if DEBUG
				if(debugOut == SET)
				{
					printf("No difference between MSD_MEM and USER_MEM!\r\n");
				}
#endif
			}

			FILE_Status = f_close(&appFile);
#if DEBUG
			if(debugOut == SET)
			{
				printf("File close status = %d\r\n", FILE_Status);
			}
#endif
			FATFS_Status = f_mount(NULL, "0", 1);
#if DEBUG
			if(debugOut == SET)
			{
				printf("FAT FS unmount status = %d\r\n", FATFS_Status);
			}
#endif
			/*
			 * At last, check CRC of firmware, which is located in USER_MEM
			 */

			//CRC_ResetDR();
            crc32 = 0x00000000;
			for (i = 0; i < appSize; i += 4)
			{
                //AppCRC = CRC_CalcCRC(*((volatile uint32_t*)(FLASH_USER_START_ADDR + i)));
                uint8_t tmp8;
                tmp8 = *(uint8_t *)(uint32_t)(FLASH_USER_START_ADDR + i);
                crc32_CCITT_s ((uint32_t *)&crc32, &tmp8);
			}
#if DEBUG
			if(debugOut == SET)
			{
				printf("CRC of application in USER_FLASH = 0x%08X\r\n", AppCRC);
				printf("PeriphDeInit peripheral and jump to 0x%08X...\r\n", *((volatile uint32_t*)(FLASH_USER_START_ADDR + 4)));
			}
#endif
			PeriphDeInit();
			GoToUserApp();
		}
		else //if FILE_Status != FR_OK
		{
			if(FILE_Status == FR_NO_FILE)
			{
#if DEBUG
				if(debugOut == SET)
				{
					printf("ERROR: File not found in MSD_MEM\r\n");
				}
#endif
			}
			else //if FILE_Status != FR_NO_FILE
			{
#if DEBUG
				if(debugOut == SET)
				{
					printf("ERROR: Other error of file opening\r\n");
				}
#endif
			}
			FATFS_Status = f_mount(NULL, "0", 1);
#if DEBUG
			if(debugOut == SET)
			{
				printf("FAT FS unmount status = %d\r\n", FATFS_Status);
			}
#endif
			while (1) {};
		}
	}
	else //FATFS_Status != FR_OK
	{
#if DEBUG
		if(debugOut == SET)
		{
			printf("ERROR: FAT FS not mounted!\r\n");
		}
#endif
		while (1) {};
	}
    
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

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 13;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
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

/* CRC init function */
static void MX_CRC_Init(void)
{

  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }

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
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);

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
