/**
 * @file    .h
 * @author  Ht3h5793
 * @date    15.12.2014
 * @version V1587.2320.10
 * @brief    emulation EEPROM MCU
*/

#ifndef DRVSTORAGE_H
#define	DRVSTORAGE_H 20161019 
/**
 *  Раздел для "include"
 */

#include "board.h"

/**
 *  Раздел для "define"
 */


/*
 * Определения ответов функций
 */

/**
 *  Раздел для "typedef"
 */

/**
 *  Раздел для прототипов функций
 */

#ifdef	__cplusplus
extern "C" {
#endif

/**
  * @brief  This function prepares the flash to be erased or programmed.
  *         It first checks no flash operation is on going,
  *         then unlocks the flash if it is locked.
  * @param  None
  * @retval None
  */
void    halFLASH_unlock (void);
void    halFLASH_lock (void);

    
/**
  * @brief  This function erases a page of flash.
  *         The Page Erase bit (PER) is set at the beginning and reset at the end
  *         of the function, in case of successive erase, these two operations
  *         could be performed outside the function.
  * @param  page_addr is an address inside the page to erase
  * @retval None
  */
msg_t   halFLASH_erase_page (uint32_t );


/**
  * @brief  This function checks that the whole page has been correctly erased
  *         A word is erased while all its bits are set.
  * @param  first_page_addr is the first address of the page to erase
  * @retval None
  */
msg_t   halFLASH_erase_check (uint32_t );


void    halFLASH_erase_all (void);



  
  /**
  * @brief  This function programs a 16-bit word.
  *         The Programming bit (PG) is set at the beginning and reset at the end
  *         of the function, in case of successive programming, these two operations
  *         could be performed outside the function.
  *         This function waits the end of programming, clears the appropriate bit in 
  *         the Status register and eventually reports an error. 
  * @param  flash_addr is the address to be programmed
  *         data is the 16-bit word to program
  * @retval None
  */
msg_t   halFLASH_FlashWord16Prog (uint32_t flash_addr, uint16_t data);
     
     
     
     
     
     
//data - указатель на записываемые данные
//address - адрес во flash
//count - количество записываемых байт, должно быть кратно 2
msg_t   halEEPROM_write (uint16_t address, uint8_t *data, uint16_t cnt);


/**
 * 
 * @param adress - адресс в памяти, максимум 65565 вроде
 * @param pBuf   - указатель на буфер
 * @param numBuf - число байт, максимум 32
 * @return I2C_FUNCTION_RETURN_READY         - готово, данные записались
 *         I2C_FUNCTION_RETURN_ERROR_TIMEOUT - где-то ошибка, при следующем обращении будет выполняться повторная запись
 */
uint8_t     halFLASH_read8 (uint32_t address);
uint16_t    halFLASH_read16 (uint32_t address);
uint32_t    halFLASH_read32 (uint32_t address);

msg_t       halFLASH_read (uint16_t address, uint8_t *data, uint16_t cnt);








#ifdef	__cplusplus
}
#endif

#endif	/* DRVSTORAGE_H */
