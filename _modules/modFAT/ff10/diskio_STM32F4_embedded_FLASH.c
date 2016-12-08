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


/* Manley EK-STM32F board does not offer socket contacts -> dummy values: */
#define SOCKPORT	1			/* Socket contact port */
#define SOCKWP		0			/* Write protect switch (PB5) */
#define SOCKINS		0			/* Card detect switch (PB4) */

#if (_MAX_SS != 512) || (_FS_READONLY == 0) || (STM32_SD_DISK_IOCTRL_FORCE == 1)
#define STM32_SD_DISK_IOCTRL   1
#else
#define STM32_SD_DISK_IOCTRL   0
#endif

/*--------------------------------------------------------------------------

   Module Private Functions and Variables

---------------------------------------------------------------------------*/

//static const DWORD socket_state_mask_cp = (1 << 0);
//static const DWORD socket_state_mask_wp = (1 << 1);

static volatile DSTATUS Stat = STA_NOINIT;  /* Disk status */

//static volatile DWORD Timer1, Timer2;	/* 100Hz decrement timers */

static BYTE CardType;   /* Card type flags */


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
	if (drv) return STA_NOINIT;			/* Supports only single drive */
	if (Stat & STA_NODISK) return Stat;	/* No card in the socket */
    
    //HAL_FLASH_Unlock();
    //FLASH->KEYR = FLASH_KEY1;
    //FLASH->KEYR = FLASH_KEY2;
            
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
	uint32_t i;

	for (i = 0; i < (count * FLASH_PAGE_SIZE); i++)
	{
		buff[i] = *((volatile uint8_t *)(FLASH_MSD_START_ADDR + sector * FLASH_PAGE_SIZE + i));
	}
    
    return RES_OK;
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

uint8_t msd_temp_buf [FLASH_PAGE_SIZE];
//#define FLASH_KEY1 ((uint32_t)0x45670123)
//#define FLASH_KEY2 ((uint32_t)0xCDEF89AB)


void halFLASH_unlock (void)
{
    if((FLASH->CR & FLASH_CR_LOCK) != RESET)
    {
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
    //FLASH->SR &= ~(FLASH_SR_EOP | FLASH_SR_PGERR | FLASH_SR_WRPRTERR);
}


void halFLASH_lock (void)
{
    FLASH->CR |= FLASH_CR_LOCK;
}


void halFLASH_erasePage (uint32_t address)
{
    if (0x08000000 < address)
    {
        FLASH_Erase_Sector (address, FLASH_VOLTAGE_RANGE_3); //TODO - check FLASH_VOLTAGE_RANGE_x
    }
}


void halFLASH_flashWrite (uint32_t address, uint8_t *data, uint32_t cnt)
{
    uint32_t i;
    
    if (0x08000000 < address)
    {
        while (FLASH->SR & FLASH_SR_BSY) {}; //Ожидаем готовности флеша к записи
        if (FLASH->SR & FLASH_SR_EOP)
        {
            FLASH->SR = FLASH_SR_EOP;
        }

        CLEAR_BIT (FLASH->CR, FLASH_CR_PSIZE);
        FLASH->CR |= FLASH_PSIZE_BYTE;
        FLASH->CR |= FLASH_CR_PG; //Разрешаем программирование флеша

        for (i = 0; i < cnt; i++)
        {   //Пишем старшие 2 байта
            *(__IO uint8_t *)address  = data[i];
            while (!(FLASH->SR & FLASH_SR_EOP)) {};
            FLASH->SR = FLASH_SR_EOP;
        }

        FLASH->CR &= ~(FLASH_CR_PG); //Запрещаем программирование флеша
    }
}

    
DRESULT disk_write (
	BYTE drv,			/* Physical drive number (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Sector count (1..255) */
)
{
    uint32_t i, j;
    uint32_t address;
    
	for (i = 0; i < 1; i++)
	{
        address = FLASH_MSD_START_ADDR + ((sector + i) * FLASH_PAGE_SIZE);
        for (j = 0; j < FLASH_PAGE_SIZE; j++)
        {
            if (0xFF != *((volatile uint8_t *)(address + j)))
            {
                halFLASH_unlock();
                halFLASH_erasePage (address);
                halFLASH_lock ();
                break;
            }
        }
        halFLASH_unlock ();
        halFLASH_flashWrite (address, (uint8_t *)buff [(i * FLASH_PAGE_SIZE)], FLASH_PAGE_SIZE);
        halFLASH_lock ();
    }
   
	return RES_OK;
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
			*(DWORD*)buff = (FLASH_MSD_SIZE / FLASH_PAGE_SIZE);
			return RES_OK;

		// для _MAX_SS не используется
		/* Get sector size (for multiple sector size (_MAX_SS >= 1024)) */
		case GET_SECTOR_SIZE:
			*(WORD*)buff = FLASH_PAGE_SIZE; //SDCardInfo.CardBlockSize;
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
