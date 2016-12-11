#include "drvSPIFLASH.h"
#include "board.h"

//http://blablacode.ru/%D0%BC%D0%B8%D0%BA%D1%80%D0%BE%D0%BA%D0%BE%D0%BD%D1%82%D1%80%D0%BE%D0%BB%D0%BB%D0%B5%D1%80%D1%8B/%D1%80%D0%B0%D0%B1%D0%BE%D1%82%D0%B0-%D1%81-%D1%84%D0%BB%D0%B5%D1%88-%D0%BF%D0%B0%D0%BC%D1%8F%D1%82%D1%8C%D1%8E-winbond-w25q128.html

//#include "halSPI.h"

extern void halSPIFLASH_GPIO_SPI_init (void);
extern uint8_t halSPIFLASH_xspi (uint8_t byte);
//extern void halSPIFLASH_spi_rd (uint8_t *buf, uint16_t len);
//extern void halSPIFLASH_spi_wr (uint8_t *buf, uint16_t len);

volatile uint8_t UNIQUEID [8];



/// IMPORTANT: NAND FLASH memory requires erase before write, because
///            it can only transition from 1s to 0s and only the erase command can reset all 0s to 1s
/// See http://en.wikipedia.org/wiki/Flash_memory
/// The smallest range that can be erased is a sector (4K, 32K, 64K); there is also a chip erase command

/// Constructor. JedecID is optional but recommended, since this will ensure that the device is present and has a valid response
/// get this from the datasheet of your flash chip
/// Example for Atmel-Adesto 4Mbit AT25DF041A: 0x1F44 (page 27: http://www.adestotech.com/sites/default/files/datasheets/doc3668.pdf)
/// Example for Winbond 4Mbit W25X40CL: 0xEF30 (page 14: http://www.winbond.com/NR/rdonlyres/6E25084C-0BFE-4B25-903D-AE10221A0929/0/W25X40CL.pdf)
// SPIFlash::SPIFlash(uint8_t slaveSelectPin, uint16_t jedecID) {
//   _slaveSelectPin = slaveSelectPin;
//   _jedecID = jedecID;
// }

void drvSPIFLASH_reset (void)
{
    GPIO_SPIFLASH_CS_L;
    halSPIFLASH_xspi (SPIFLASH_ENABLE_RESET);
    GPIO_SPIFLASH_CS_H;
 
    GPIO_SPIFLASH_CS_L;
    halSPIFLASH_xspi (SPIFLASH_RESET);
    GPIO_SPIFLASH_CS_H;
}


/// check if the chip is busy erasing/writing
bool_t halSPIFLASH_busy (void)
{
    //select();
    return (0 != (halSPIFLASH_readStatus () & 0x01)) ? TRUE: FALSE;
}


uint8_t halSPIFLASH_readStatus (void)
{
    uint8_t status;
    
    GPIO_SPIFLASH_CS_L;
    halSPIFLASH_xspi (SPIFLASH_READ_STATUS_1);
    status = halSPIFLASH_xspi (0);
    GPIO_SPIFLASH_CS_H;
    return status;
}


/// Send a command to the flash chip, pass TRUE for isWrite when its a write command
void drvSPIFLASH_command (uint8_t cmd, bool_t isWrite)
{
    if (FALSE != isWrite)
    {
        drvSPIFLASH_command (SPIFLASH_WRITE_ENABLE, FALSE); // Write Enable
        GPIO_SPIFLASH_CS_H;
    }
    while (halSPIFLASH_busy ()){}; //wait for any write/erase to complete
    GPIO_SPIFLASH_CS_L;
    halSPIFLASH_xspi (cmd);
}


void drvSPIFLASH_sleep (void)
{
    drvSPIFLASH_command (SPIFLASH_SLEEP, FALSE);
    GPIO_SPIFLASH_CS_H;
}


void drvSPIFLASH_wakeup (void)
{
    drvSPIFLASH_command (SPIFLASH_WAKE, FALSE);
    GPIO_SPIFLASH_CS_H;
}


uint32_t drvSPIFLASH_readDeviceID (void)
{
    uint32_t jedecid;
    
    drvSPIFLASH_command (SPIFLASH_GET_JEDEC_ID, FALSE);
    jedecid = 0;
    jedecid |= (uint32_t) halSPIFLASH_xspi (0);
    jedecid = jedecid << 8;
    jedecid |= (uint32_t) halSPIFLASH_xspi (0);
    jedecid = jedecid << 8;
    jedecid |= (uint32_t) halSPIFLASH_xspi (0);
    GPIO_SPIFLASH_CS_H;
    return jedecid;
}



/**
 * Infos from
 *  http://www.wlxmall.com/images/stock_item/att/A1010004.pdf
 *  http://www.gigadevice.com/product-series/5.html?locale=en_US
 *  http://www.elinux.org/images/f/f5/Winbond-w25q32.pdf
 */
uint32_t drvSPIFLASH_getFlashChipSizeByChipId (void)
{
    uint32_t chipId = drvSPIFLASH_readDeviceID (); //getFlashChipId();
    /**
     * Chip ID
     * 00 - always 00 (Chip ID use only 3 byte)
     * 17 - ? looks like 2^xx is size in Byte ?     //todo: find docu to this
     * 40 - ? may be Speed ?                        //todo: find docu to this
     * C8 - manufacturer ID
     */
    switch (chipId)
    {
        // GigaDevice
        case 0x1740C8: // GD25Q64B
            return (SPIFLASH_SIZE_8_MB);
        case 0x1640C8: // GD25Q32B
            return (SPIFLASH_SIZE_4_MB);
        case 0x1540C8: // GD25Q16B
            return (SPIFLASH_SIZE_2_MB);
        case 0x1440C8: // GD25Q80
            return (SPIFLASH_SIZE_1_MB);
        case 0x1340C8: // GD25Q40
            return (SPIFLASH_SIZE_512_kB);
        case 0x1240C8: // GD25Q20
            return (SPIFLASH_SIZE_256_kB);
        case 0x1140C8: // GD25Q10
            return (SPIFLASH_SIZE_128_kB);
        case 0x1040C8: // GD25Q12
            return (SPIFLASH_SIZE_64_kB);

        // Winbond
        case 0x1640EF: // W25Q32
            return (SPIFLASH_SIZE_4_MB);
        case 0x1540EF: // W25Q16
            return (SPIFLASH_SIZE_2_MB);
        case 0x1440EF: // W25Q80
            return (SPIFLASH_SIZE_1_MB);
        case 0x1340EF: // W25Q40
            return (SPIFLASH_SIZE_512_kB);

        // BergMicro
        case 0x1640E0: // BG25Q32
            return (SPIFLASH_SIZE_4_MB);
        case 0x1540E0: // BG25Q16
            return (SPIFLASH_SIZE_2_MB);
        case 0x1440E0: // BG25Q80
            return (SPIFLASH_SIZE_1_MB);
        case 0x1340E0: // BG25Q40
            return (SPIFLASH_SIZE_512_kB);

        // Atmel
        case 0x1F4700: // 25DF321
        case 0x1F4701: // 25DF321A
            return (SPIFLASH_SIZE_4_MB);
            
        default:
            return 0;
    }
}


/// Get the 64 bit unique identifier, stores it in UNIQUEID[8]. Only needs to be called once, ie after initialize
/// Returns the byte pointer to the UNIQUEID byte array
/// Read UNIQUEID like this:
/// flash.readUniqueId(); for (byte i=0;i<8;i++) { Serial.print(flash.UNIQUEID[i], HEX); Serial.print(' '); }
/// or like this:
/// flash.readUniqueId(); byte* MAC = flash.readUniqueId(); for (byte i=0;i<8;i++) { Serial.print(MAC[i], HEX); Serial.print(' '); }
void drvSPIFLASH_readUniqueId (uint8_t *UNIQUEID)
{
    uint8_t i;
    
    drvSPIFLASH_command (SPIFLASH_GET_DEV_ID, FALSE); //
    halSPIFLASH_xspi (0);
    halSPIFLASH_xspi (0);
    halSPIFLASH_xspi (0);
    halSPIFLASH_xspi (0);
    for (i = 0; i < 8; i++)
    {
        UNIQUEID [i] = halSPIFLASH_xspi (0);
    }
    GPIO_SPIFLASH_CS_H;
}


void drvSPIFLASH_global_unprotect_sector (void)
{
    //write_enable();
    GPIO_SPIFLASH_CS_L;
    halSPIFLASH_xspi (0x01);
    halSPIFLASH_xspi (0x00);
    GPIO_SPIFLASH_CS_H;
    //write_disable();
}

/// setup SPI, read device ID etc...
msg_t drvSPIFLASH_init (uint8_t num) //, uint16_t *id)
{
    uint32_t id;
    
    halSPIFLASH_GPIO_SPI_init ();
    GPIO_SPIFLASH_CS_H; //off chip

    //drvSPIFLASH_wakeup ();
    GPIO_SPIFLASH_CS_L;
    halSPIFLASH_xspi (SPIFLASH_WAKE); //если что-то записывали (но проц сбросился), то будет плохо
    GPIO_SPIFLASH_CS_H;
    //drvSPIFLASH_reset (); //not recommendet
    //if (SPIFLASH_JEDEC_ID == drvSPIFLASH_readDeviceID ())
    id = drvSPIFLASH_readDeviceID ();
    if ((0x0000 != id) && (0xFFFF != id))
    {
        drvSPIFLASH_command (SPIFLASH_WRITE_STATUS_1, TRUE); // Write Status Register
        halSPIFLASH_xspi (0);                     // Global Unprotect
        GPIO_SPIFLASH_CS_H;
        //drvSPIFLASH_readUniqueId (); //@todo!!!
        return TRUE;
    }
    
    return FALSE;
}


/// read 1 byte from flash memory
uint8_t halSPIFLASH_readByte (uint32_t addr)
{
    uint8_t result;
    
    drvSPIFLASH_command (SPIFLASH_READ, FALSE);
    halSPIFLASH_xspi (addr >> 16);
    halSPIFLASH_xspi (addr >> 8);
    halSPIFLASH_xspi (addr);
    result = halSPIFLASH_xspi (0);
    GPIO_SPIFLASH_CS_H;
    return result;
}


/// read unlimited # of bytes
void halSPIFLASH_readBytes (void *buf, uint32_t addr, uint16_t len)
{
    uint16_t i;
    
    drvSPIFLASH_command (SPIFLASH_READ_ARRAY, FALSE);
    halSPIFLASH_xspi (addr >> 16);
    halSPIFLASH_xspi (addr >> 8);
    halSPIFLASH_xspi (addr);
    halSPIFLASH_xspi  (0); //"dont care"
    for (i = 0; i < len; i++)
    {
        ((uint8_t *) buf)[i] = halSPIFLASH_xspi (0);
    }
    //halSPIFLASH_spi_rd (buf, len);
    GPIO_SPIFLASH_CS_H;
}






/// Write 1 byte to flash memory
/// WARNING: you can only write to previously erased memory locations (see datasheet)
///          use the block erase commands to first clear memory (write 0xFFs)
void halSPIFLASH_writeByte (uint32_t addr, uint8_t byte)
{
    drvSPIFLASH_command (SPIFLASH_PAGE_PROGRAMM, TRUE);  // Byte/Page Program
    halSPIFLASH_xspi (addr >> 16);
    halSPIFLASH_xspi (addr >> 8);
    halSPIFLASH_xspi (addr);
    halSPIFLASH_xspi (byte);
    GPIO_SPIFLASH_CS_H;
}


/// write 1-256 bytes to flash memory
/// WARNING: you can only write to previously erased memory locations (see datasheet)
///          use the block erase commands to first clear memory (write 0xFFs)
/// WARNING: if you write beyond a page boundary (or more than 256bytes),
///          the bytes will wrap around and start overwriting at the beginning of that same page
///          see datasheet for more details
void halSPIFLASH_writeBytes (const void  *buf, uint32_t addr, uint16_t len)
{
    uint16_t  i;
    
    drvSPIFLASH_command (SPIFLASH_PAGE_PROGRAMM, TRUE);  // Byte/Page Program
    halSPIFLASH_xspi (addr >> 16);
    halSPIFLASH_xspi (addr >> 8);
    halSPIFLASH_xspi (addr);
    
    for (i = 0; i < len; i++)
    {
        halSPIFLASH_xspi (((uint8_t *) buf)[i]);
    }
    
    //halSPIFLASH_spi_wr ((uint8_t *)buf, len);
    GPIO_SPIFLASH_CS_H;
}


/// erase entire flash memory array
/// may take several seconds depending on size, but is non blocking
/// so you may wait for this to complete using busy() or continue doing
/// other things and later check if the chip is done with busy()
/// note that any command will first wait for chip to become available using busy()
/// so no need to do that twice
void halSPIFLASH_chipErase (void)
{
    drvSPIFLASH_command (SPIFLASH_CHIP_ERASE, TRUE);
    GPIO_SPIFLASH_CS_H;
}


/// erase a 4Kbyte block
void halSPIFLASH_blockErase4K (uint32_t addr)
{
    drvSPIFLASH_command (SPIFLASH_BLOCK_ERASE_4K, TRUE); // Block Erase
    halSPIFLASH_xspi (addr >> 16);
    halSPIFLASH_xspi (addr >> 8);
    halSPIFLASH_xspi (addr);
    GPIO_SPIFLASH_CS_H;
}

/// erase a 32Kbyte block
void halSPIFLASH_blockErase32K (uint32_t addr)
{
    drvSPIFLASH_command (SPIFLASH_BLOCK_ERASE_32K, TRUE); // Block Erase
    halSPIFLASH_xspi (addr >> 16);
    halSPIFLASH_xspi (addr >> 8);
    halSPIFLASH_xspi (addr);
    GPIO_SPIFLASH_CS_H;
}
