/**
 * @file    
 * @author  Ht3h5793, CD45
 * @date    13.6.2013  2:12
 * @version 
 * @brief 

 * Copyright (c) 2013 by Felix Rusu <felix@lowpowerlab.com>
 * SPI Flash memory library for arduino/moteino.
 * This works with 256byte/page SPI flash memory
 * For instance a 4MBit (512Kbyte) flash chip will have 2048 pages: 256*2048 = 524288 bytes (512Kbytes)
 * Minimal modifications should allow chips that have different page size but modifications
 * DEPENDS ON: Arduino SPI library
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 
 #include "drvSPIFLASH.h"

#ifdef	__cplusplus
extern "C" {
#endif
    
uint8_t halSPIFLASH_xspi (uint8_t byte)
{
#ifdef SPIFLASH_SPI_BITBANG
    uint8_t i;
    
    for (i = 0; i < 8; i++)
    {
        if (0x80 & byte)
        {
            SPI_MOSI_H;
        } else {
            SPI_MOSI_L;
        }
        byte = byte << 1;
        __NOP();
        SPI_SCK_H;
        if (0 != SPI_MISO)
            byte |= 0x01;
        __NOP();
        SPI_SCK_L;
    }
    return byte;
#else
    ////while(!SPI1STATbits.SPIRBF); //while (0 != SPI1STATbits.SPITBF){}; // wait until bus cycle complete
    //SPI2BUF = byte;    // write byte to SSP1BUF register
    //while (!SPI2STATbits.SPIRBF); //while (0 != SPI1STATbits.SPIRBF){};
    //return SPI2BUF;
#endif
}


void halSPIFLASH_GPIO_init (void)
{

    
	// Configure I/O for Flash Chip select
    GPIO_StructInit (&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = (SPI_PIN_CLK | SPI_PIN_MOSI);
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_StructInit (&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = SPI_PIN_MISO;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    SPIFLASH_CS_H;
    
    GPIO_StructInit (&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = SPI_PIN_CS;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
#ifdef	__cplusplus
}
#endif

 */

#ifndef SPIFLASH_H
#define	SPIFLASH_H 20150502

/**
 *  –аздел дл€ "include"
 */
#include "board.h"


/**
 *  –аздел дл€ "define"
 */
/// IMPORTANT: NAND FLASH memory requires erase before write, because
///            it can only transition from 1s to 0s and only the erase command can reset all 0s to 1s
/// See http://en.wikipedia.org/wiki/Flash_memory
/// The smallest range that can be erased is a sector (4K, 32K, 64K); there is also a chip erase command

/// Standard SPI flash commands
/// Assuming the WP pin is pulled up (to disable hardware write protection)
/// To use any write commands the WEL bit in the status register must be set to 1.
/// This is accomplished by sending a 0x06 command before any such write/erase command.
/// The WEL bit in the status register resets to the logical У0Ф state after a
/// device power-up or reset. In addition, the WEL bit will be reset to the logical У0Ф state automatically under the following conditions:
/// Х Write Disable operation completes successfully
/// Х Write Status Register operation completes successfully or aborts
/// Х Protect Sector operation completes successfully or aborts
/// Х Unprotect Sector operation completes successfully or aborts
/// Х Byte/Page Program operation completes successfully or aborts
/// Х Sequential Program Mode reaches highest unprotected memory location
/// Х Sequential Program Mode reaches the end of the memory array
/// Х Sequential Program Mode aborts
/// Х Block Erase operation completes successfully or aborts
/// Х Chip Erase operation completes successfully or aborts
/// Х Hold condition aborts

      

//#define SPIFLASH_ARRAYREADLOWFREQ 0x03        

//#define SPIFLASH_BYTEPAGEPROGRAM  0x02        // write (1 to 256bytes)
//#define SPIFLASH_MACREAD          0x4B        // read unique ID number (MAC)



/**
 *  –аздел дл€ "typedef"
 */
typedef enum {
    SPIFLASH_WRITE_DISABLE      = 0x04, // write disable
    SPIFLASH_WRITE_ENABLE       = 0x06, // write enable
    
    SPIFLASH_READ_STATUS_1      = 0x05, // read status register
    SPIFLASH_READ_STATUS_2      = 0x35, // write status register
    SPIFLASH_READ_STATUS_3      = 0x15,
    
    SPIFLASH_WRITE_STATUS_1     = 0x01,
    SPIFLASH_WRITE_STATUS_2     = 0x31,
    SPIFLASH_WRITE_STATUS_3     = 0x11,
    
    SPIFLASH_GET_DEV_ID         = 0x90,
    SPIFLASH_GET_JEDEC_ID       = 0x9F,
    // read JEDEC manufacturer and device ID (2 bytes, specific bytes for each manufacturer and device)
                                              // Example for Atmel-Adesto 4Mbit AT25DF041A: 0x1F44 (page 27: http://www.adestotech.com/sites/default/files/datasheets/doc3668.pdf)
                                              // Example for Winbond 4Mbit W25X40CL: 0xEF30 (page 14: http://www.winbond.com/NR/rdonlyres/6E25084C-0BFE-4B25-903D-AE10221A0929/0/W25X40CL.pdf)
    
    SPIFLASH_ENABLE_RESET       = 0x66,
    SPIFLASH_RESET              = 0x99,
    
    SPIFLASH_PAGE_PROGRAMM      = 0x02,
    
    SPIFLASH_READ               = 0x03, // read array (low frequency)
    SPIFLASH_READ_ARRAY         = 0x0B, // read array (fast, need to add 1 dummy byte after 3 address bytes)

    SPIFLASH_BLOCK_ERASE_4K     = 0x20, // erase one 4K block of flash memory
    SPIFLASH_BLOCK_ERASE_32K    = 0x52, // erase one 32K block of flash memory
    SPIFLASH_BLOCK_ERASE_64K    = 0xD8, // erase one 64K block of flash memory
    //SPIFLASH_CHIP_ERASE         = 0x60, // chip erase (may take several seconds depending on size)
                                              // but no actual need to wait for completion (instead need to check the status register BUSY bit)
    SPIFLASH_CHIP_ERASE         = 0xC7, //0x60

    SPIFLASH_SLEEP              = 0xB9,        // deep power down
    SPIFLASH_WAKE               = 0xAB,        // deep power wake up
    
} SPIFLASH_Command_t; 




#ifdef	__cplusplus
extern "C" {
#endif

/**
 *  –аздел дл€ прототипов функций
 */
    
msg_t    drvSPIFLASH_init (uint8_t num, uint16_t *id);


//msg_t   drvSPIFLASH_getErrors (void);
// public:
//   static uint8_t UNIQUEID[8];
//   SPIFlash(uint8_t slaveSelectPin, uint16_t jedecID = 0); // конструктор класса
//bool_t halSPIFLASH_initialize (void);

void halSPIFLASH_command (uint8_t cmd, bool_t isWrite);

/** рапорпаопа
 */
uint8_t halSPIFLASH_readStatus (void);
uint8_t halSPIFLASH_readByte (uint32_t addr);
void halSPIFLASH_readBytes (void *buf, uint32_t addr, uint16_t len);
void halSPIFLASH_writeByte (uint32_t addr, uint8_t byt);
void halSPIFLASH_writeBytes (const void *buf, uint32_t addr, uint16_t len);

bool_t halSPIFLASH_busy (void);

void halSPIFLASH_chipErase (void);
void halSPIFLASH_blockErase4K (uint32_t address);
void halSPIFLASH_blockErase32K (uint32_t address);


uint16_t halSPIFLASH_readDeviceId (void);
uint8_t *drvSPIFLASH_readUniqueId (void);

void drvSPIFLASH_sleep (void);
void drvSPIFLASH_wakeup (void);
void halSPIFLASH_end (void);

// protected:
//   void select();
//   void unselect();
//   uint8_t _slaveSelectPin;
//   uint16_t _jedecID;
//   uint8_t _SPCR;
//   uint8_t _SPSR;
//   
  
#ifdef	__cplusplus
}
#endif

#endif	/** SPIFLASH_H */
