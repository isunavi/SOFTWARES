/*-----------------------------------------------------------------------*/
/* MMC/SDSC/SDHC (in SPI mode) control module for STM32 Version 1.1.6    */
/* (C) Martin Thomas, 2010 - based on the AVR MMC module (C)ChaN, 2007   */
/*-----------------------------------------------------------------------*/

/* Copyright (c) 2010, Martin Thomas, ChaN
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */


#include "integer.h"
#include "ffconf.h"
#include "diskio.h" /* FatFs lower layer API */
#include "board.h"
#include "drvSPIFLASH.h"
#include "sysReport.h"

#ifdef STM32_SD_USE_DMA
    #warning "Information only: using DMA"
    #pragma message "*** Using DMA ***"
#endif

/* set to 1 to provide a disk_ioctrl function even if not needed by the FatFs */
//#define STM32_SD_DISK_IOCTRL_FORCE      0

// demo uses a command line option to define this (see Makefile):
#define STM32L476
//#define STM32F100
//#define USE_EK_STM32F
//#define USE_STM32_P103
//#define USE_MINI_STM32

#if defined(STM32L476)
#define CARD_SUPPLY_SWITCHABLE   0
// #define GPIO_PWR                 GPIOB
// #define RCC_APB1Periph_GPIO_PWR  RCC_APB1Periph_GPIOD
// #define GPIO_Pin_PWR             GPIO_Pin_10
// #define GPIO_Mode_PWR            GPIO_Mode_Out_OD /* pull-up resistor at power FET */

#define SOCKET_WP_CONNECTED      0 /* write-protect socket-switch */
#define SOCKET_CP_CONNECTED      0 /* card-present socket-switch */
// #define GPIO_WP                  GPIOB
// #define GPIO_CP                  GPIOB
// #define RCC_APBxPeriph_GPIO_WP   RCC_AHB1Periph_GPIOB
// #define RCC_APBxPeriph_GPIO_CP   RCC_APB1Periph_GPIOB
// #define GPIO_Pin_WP              GPIO_Pin_6
// #define GPIO_Pin_CP              GPIO_Pin_7
// #define GPIO_Mode_WP             GPIO_Mode_IN /* external resistor */
// #define GPIO_Mode_CP             GPIO_Mode_IN /* external resistor */
//  
// #define SPI_SD                   SPI1
// #define GPIO_CS                  GPIOA
// #define RCC_AHB1Periph_GPIO_CS   RCC_AHB1Periph_GPIOA
// #define GPIO_Pin_CS              GPIO_Pin_4

// #define GPIO_SPI_SD              GPIOA
// #define GPIO_Pin_SPI_SD_SCK      GPIO_Pin_5
// #define GPIO_Pin_SPI_SD_MISO     GPIO_Pin_6
// #define GPIO_Pin_SPI_SD_MOSI     GPIO_Pin_7
// #define RCC_APBPeriphClockCmd_SPI_SD  RCC_APB2PeriphClockCmd
// #define RCC_APBPeriph_SPI_SD     RCC_APB2Periph_SPI1
//  
// #define DMA_Channel_SPI_SD_RX    DMA1_Channel4
// #define DMA_Channel_SPI_SD_TX    DMA1_Channel5
// #define DMA_FLAG_SPI_SD_TC_RX    DMA1_FLAG_TC4
// #define DMA_FLAG_SPI_SD_TC_TX    DMA1_FLAG_TC5
//  
//  /* - for SPI1 and full-speed APB2: 72MHz/4 */
// #define SPI_BaudRatePrescaler_SPI_SD  SPI_BaudRatePrescaler_256

#endif


// #elif defined(USE_EK_STM32F)
//  #define CARD_SUPPLY_SWITCHABLE   1
//  #define GPIO_PWR                 GPIOD
//  #define RCC_APB2Periph_GPIO_PWR  RCC_APB2Periph_GPIOD
//  #define GPIO_Pin_PWR             GPIO_Pin_10
//  #define GPIO_Mode_PWR            GPIO_Mode_Out_OD /* pull-up resistor at power FET */
//  #define SOCKET_WP_CONNECTED      0
//  #define SOCKET_CP_CONNECTED      0
//  #define SPI_SD                   SPI1
//  #define GPIO_CS                  GPIOD
//  #define RCC_APB2Periph_GPIO_CS   RCC_APB2Periph_GPIOD
//  #define GPIO_Pin_CS              GPIO_Pin_9
//  #define DMA_Channel_SPI_SD_RX    DMA1_Channel2
//  #define DMA_Channel_SPI_SD_TX    DMA1_Channel3
//  #define DMA_FLAG_SPI_SD_TC_RX    DMA1_FLAG_TC2
//  #define DMA_FLAG_SPI_SD_TC_TX    DMA1_FLAG_TC3
//  #define GPIO_SPI_SD              GPIOA
//  #define GPIO_Pin_SPI_SD_SCK      GPIO_Pin_5
//  #define GPIO_Pin_SPI_SD_MISO     GPIO_Pin_6
//  #define GPIO_Pin_SPI_SD_MOSI     GPIO_Pin_7
//  #define RCC_APBPeriphClockCmd_SPI_SD  RCC_APB2PeriphClockCmd
//  #define RCC_APBPeriph_SPI_SD     RCC_APB2Periph_SPI1
//  /* - for SPI1 and full-speed APB2: 72MHz/4 */
//  #define SPI_BaudRatePrescaler_SPI_SD  SPI_BaudRatePrescaler_4

// #elif defined(USE_STM32_P103)
//  // Olimex STM32-P103 not tested!
//  #define CARD_SUPPLY_SWITCHABLE   0
//  #define SOCKET_WP_CONNECTED      1 /* write-protect socket-switch */
//  #define SOCKET_CP_CONNECTED      1 /* card-present socket-switch */
//  #define GPIO_WP                  GPIOC
//  #define GPIO_CP                  GPIOC
//  #define RCC_APBxPeriph_GPIO_WP   RCC_APB2Periph_GPIOC
//  #define RCC_APBxPeriph_GPIO_CP   RCC_APB2Periph_GPIOC
//  #define GPIO_Pin_WP              GPIO_Pin_6
//  #define GPIO_Pin_CP              GPIO_Pin_7
//  #define GPIO_Mode_WP             GPIO_Mode_IN_FLOATING /* external resistor */
//  #define GPIO_Mode_CP             GPIO_Mode_IN_FLOATING /* external resistor */
//  #define SPI_SD                   SPI2
//  #define GPIO_CS                  GPIOB
//  #define RCC_APB2Periph_GPIO_CS   RCC_APB2Periph_GPIOB
//  #define GPIO_Pin_CS              GPIO_Pin_12
//  #define DMA_Channel_SPI_SD_RX    DMA1_Channel4
//  #define DMA_Channel_SPI_SD_TX    DMA1_Channel5
//  #define DMA_FLAG_SPI_SD_TC_RX    DMA1_FLAG_TC4
//  #define DMA_FLAG_SPI_SD_TC_TX    DMA1_FLAG_TC5
//  #define GPIO_SPI_SD              GPIOB
//  #define GPIO_Pin_SPI_SD_SCK      GPIO_Pin_13
//  #define GPIO_Pin_SPI_SD_MISO     GPIO_Pin_14
//  #define GPIO_Pin_SPI_SD_MOSI     GPIO_Pin_15
//  #define RCC_APBPeriphClockCmd_SPI_SD  RCC_APB1PeriphClockCmd
//  #define RCC_APBPeriph_SPI_SD     RCC_APB1Periph_SPI2
//  /* for SPI2 and full-speed APB1: 36MHz/2 */
//  /* !! PRESCALE 4 used here - 2 does not work, maybe because
//        of the poor wiring on the HELI_V1 prototype hardware */
//  #define SPI_BaudRatePrescaler_SPI_SD  SPI_BaudRatePrescaler_4



/* Manley EK-STM32F board does not offer socket contacts -> dummy values: */
#define SOCKPORT	1			/* Socket contact port */
#define SOCKWP		0			/* Write protect switch (PB5) */
#define SOCKINS		0			/* Card detect switch (PB4) */
/*
#if (_MAX_SS != 512) || (_FS_READONLY == 0) || (STM32_SD_DISK_IOCTRL_FORCE == 1)
#define STM32_SD_DISK_IOCTRL   1
#else
#define STM32_SD_DISK_IOCTRL   0
#endif
*/


/*--------------------------------------------------------------------------

   Module Private Functions and Variables

---------------------------------------------------------------------------*/

//static const DWORD socket_state_mask_cp = (1 << 0);
//static const DWORD socket_state_mask_wp = (1 << 1);

static volatile DSTATUS Stat = STA_NOINIT;  /* Disk status */

//static volatile DWORD Timer1, Timer2;	/* 100Hz decrement timers */

//static BYTE CardType;   /* Card type flags */

/*-----------------------------------------------------------------------*/
/* Power Control and interface-initialization (Platform dependent)       */
/*-----------------------------------------------------------------------*/


/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE drv		/* Physical drive number (0) */
)
{
	//BYTE n, cmd, ty, ocr[4];
    uint16_t SPIFLASH_ID;

	if (drv) return STA_NOINIT;			/* Supports only single drive */
	if (Stat & STA_NODISK) return Stat;	/* No card in the socket */

    // init QSPI memory 
    /* Initialize the quad SPI flash interface */
    if (drvSPIFLASH_init (0, &SPIFLASH_ID) != TRUE)
    {
        ERROR_ACTION (ERROR_SPI_INIT);
    }
    
    Stat &= ~STA_NOINIT;

	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status (
	BYTE drv		/* Physical drive number (0) */
)
{
	if (drv) return STA_NOINIT;		/* Supports only single drive */
	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
/*************************************************************************************************
 * @brief	Функция disk_read читает сектор (секторы) с привода диска.
 *
 * @param 	pdrv   - Номер физического диска
 * @param 	buff   - Указатель на буфер чтения данных
 * @param 	sector - Начальный номер сектора
 * @param 	count  - Количество читаемых подряд секторов
 *
 * @return	RES_OK (0)	- Функция выполнилась успешно.
 * @return	RES_ERROR	- Произошла какая-то аппаратная невосстановимая ошибка (hard error)
 * 	 	 	 	  		 при операции чтения.
 * @return	RES_PARERR	- Неправильный параметр.
 * @return	RES_NOTRDY	- Привод диска не был инициализирован.
 ************************************************************************************************/
DRESULT disk_read (
	BYTE drv,			/* Physical drive number (0) */
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Sector count (1..255) */
)
{
	//if (drv || !count) return RES_PARERR;
	//if (Stat & STA_NOINIT) return RES_NOTRDY;
    
    uint32_t i, j, adr;
    
    //LED_SET_COLOR(COLOR_GREEN); //LED_GREEN_ON;
    
    for (i = 0; i < count; i++)
    {
        adr = ((sector + i) * SPIFLASH_BLOCK_SIZE);
        if (adr < SPIFLASH_FLASH_SIZE)
        {
            while (TRUE == halSPIFLASH_busy ()) {};
            halSPIFLASH_readBytes (&buff [i * SPIFLASH_BLOCK_SIZE], adr, SPIFLASH_BLOCK_SIZE);
            count--;
        }
        else
        {
            //TODO
        }
    }
    
    //LED_SET_COLOR(0); //LED_GREEN_OFF;
    
    
	return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
/*************************************************************************************************
 * @brief	Функция disk_write пишет сектор (секторы) на диск.
 *
 * @param pdrv   - Номер физического диска
 * @param buff   - Указатель на буфер данных для записи
 * @param sector - Начальный номер сектора
 * @param count  - Количество записываемых подряд секторов (1-128)
 *
 * @return	RES_OK (0)	- Функция выполнилась успешно.
 * @return	RES_ERROR	- Произошла какая-то аппаратная невосстановимая ошибка (hard error)
 * 	 	 	 	  		 при операции чтения.
 * @return	RES_PARERR	- Неправильный параметр.
 * @return	RES_NOTRDY	- Привод диска не был инициализирован.
 ************************************************************************************************/
#if _FS_READONLY == 0

uint8_t msd_temp_buf [SPIFLASH_BLOCK_SIZE];

    
DRESULT disk_write (
	BYTE drv,			/* Physical drive number (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Sector count (1..255) */
)
{
	//if (drv || !count) return RES_PARERR;
	//if (Stat & STA_NOINIT) return RES_NOTRDY;
	//if (Stat & STA_PROTECT) return RES_WRPRT;

    uint32_t i, j, adr;
    
    //LED_SET_COLOR(COLOR_BLUE); //LED_BLUE_ON;
    
    for (i = 0; i < count; i++)
    {
        adr = ((sector + i) * SPIFLASH_BLOCK_SIZE);
        
        if (adr < SPIFLASH_FLASH_SIZE)
        {
            // test read
            if (count == 1) //if blok one, try check memory for reduce erasing cycles
            {
                while (TRUE == halSPIFLASH_busy ()) {};
                halSPIFLASH_readBytes (&msd_temp_buf [0], adr, SPIFLASH_BLOCK_SIZE);
                for (j = 0; j < SPIFLASH_BLOCK_SIZE; j++)
                {
                    if (0xFF != msd_temp_buf[j]) // память нечистая, мол лорд
                    {
                        while (TRUE == halSPIFLASH_busy ()) {};
                        halSPIFLASH_blockErase4K (adr);
                        break; //эхх...
                    }
                }
            }
            else
            {
                while (TRUE == halSPIFLASH_busy ()) {};
                halSPIFLASH_blockErase4K (adr);
            }
            // only 256 bytes for time
            for (j = 0; j < SPIFLASH_BLOCK_SIZE / 256; j++)
            {
                while (TRUE == halSPIFLASH_busy ()) {};
                halSPIFLASH_writeBytes (&buff [(i * SPIFLASH_BLOCK_SIZE)  + (256 * j)], adr + (256 * j), 256);
            }
            count--;
        }
        else
        {
            //TODO
        }
    }
    
    //LED_SET_COLOR(0); //LED_BLUE_OFF;
    
	return count ? RES_ERROR : RES_OK;
}
#endif /* _READONLY == 0 */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

/*************************************************************************************************
 * @brief	Функция disk_ioctl управляет специальными функциями диска
 *
 * @param 	pdrv	Указывает номер привода (0-9).
 * @param 	cmd 	Указывает код команды.
 * @param 	buff	Указатель на буфер, в котором может содержаться параметр или данные
 * 				- в зависимости от кода команды. Когда не используется, то указатель Buffer равен NULL
 *
 * @return 	RES_OK (0)	Функция завершилась успешно
 * @return 	RES_ERROR	Произошла какая-то ошибка
 * @return 	RES_PARERR	Недопустимый код команды
 * @return 	RES_NOTRDY	Привод диска не был инициализирован.
 *
 * @note	FatFs не использует функции, которые зависят от устройства.
 * 			Используемые команды:
 * 				CTRL_SYNC,
 * 				GET_SECTOR_SIZE,
 * 				GET_SECTOR_COUNT,
 * 				GET_BLOCK_SIZE,
 * 				CTRL_ERASE_SECTOR
 ************************************************************************************************/
#if _USE_IOCTL
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void *buff)
{
	switch (cmd)
	{
		// аппаратно-независимые функции, используемые fatfs

		/* Flush disk cache (for write functions) */
		case CTRL_SYNC:
			return RES_OK;

		/* Get media size (for only f_mkfs()) */
		case GET_SECTOR_COUNT:
			*(DWORD*)buff = SPIFLASH_BLOCK_NUM;//SDCardInfo.CardCapacity / SDCardInfo.CardBlockSize;
			return RES_OK;

		// для _MAX_SS не используется
		/* Get sector size (for multiple sector size (_MAX_SS >= 1024)) */
		case GET_SECTOR_SIZE:
			*(WORD*)buff = SPIFLASH_BLOCK_SIZE; //SDCardInfo.CardBlockSize;
			return RES_OK;

		/* Get erase block size (for only f_mkfs()) */
		case GET_BLOCK_SIZE:
			*(WORD*)buff = 1;
			return RES_OK;

		/* Force erased a block of sectors (for only _USE_ERASE) */
		// тоже не реализовано
		case CTRL_ERASE_SECTOR: //@todo
			return RES_OK;
	}
	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Device Timer Interrupt Procedure  (Platform dependent)                */
/*-----------------------------------------------------------------------*/
/* This function must be called in period of 10ms                        */

//RAMFUNC 
void disk_timerproc (void)
{
}
