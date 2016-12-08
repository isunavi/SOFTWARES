#include "drvStorage.h"
#include "board.h"
#include "sysReport.h"


#if BOARD_RS485_STTM

//#define __INLINE    


void halFLASH_unlock (void)
{  
    /* (1) Wait till no operation is on going */
    /* (2) Check that the Flash is unlocked */
    /* (3) Perform unlock sequence */
    while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (1) */  
    {
        /* For robust implementation, add here time-out management */
    }

    if ((FLASH->CR & FLASH_CR_LOCK) != 0) /* (2) */
    {    
        FLASH->KEYR = FLASH_KEY1; /* (3) */
        FLASH->KEYR = FLASH_KEY2;
    }        
}

void halFLASH_lock (void)
{  
    /* (1) Wait till no operation is on going */
    /* (2) Check that the Flash is unlocked */
    /* (3) Perform unlock sequence */
    while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (1) */  
    {
        /* For robust implementation, add here time-out management */
    }
    
    if ((FLASH->CR & FLASH_CR_LOCK) == 0) /* (2) */
    {    
        FLASH->KEYR = 0; /* (3) */
        FLASH->KEYR = 0;
    }
}


msg_t halFLASH_erase_page (uint32_t page_addr)
{   
    msg_t error;
    
    /* (1) Set the PER bit in the FLASH_CR register to enable page erasing */
    /* (2) Program the FLASH_AR register to select a page to erase */
    /* (3) Set the STRT bit in the FLASH_CR register to start the erasing */
    /* (4) Wait until the BSY bit is reset in the FLASH_SR register */
    /* (5) Check the EOP flag in the FLASH_SR register */
    /* (6) Clear EOP flag by software by writing EOP at 1 */
    /* (7) Reset the PER Bit to disable the page erase */
    FLASH->CR |= FLASH_CR_PER; /* (1) */    
    FLASH->AR =  page_addr; /* (2) */    
    FLASH->CR |= FLASH_CR_STRT; /* (3) */    
    while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (4) */ 
    {
        /* For robust implementation, add here time-out management */
    }  
    if ((FLASH->SR & FLASH_SR_EOP) != 0)  /* (5) */
    {  
        FLASH->SR |= FLASH_SR_EOP; /* (6)*/
    }    
    /* Manage the error cases */
    else if ((FLASH->SR & FLASH_SR_WRPERR) != 0) /* Check Write protection error */
    {
        error |= ERROR_EEPROM_WRITE_PROTECTION; /* Report the error to the main progran */
        FLASH->SR |= FLASH_SR_WRPERR; /* Clear the flag by software by writing it at 1*/
    }
    else
    {
        error |= ERROR_UNKNOWN; /* Report the error to the main progran */
    }
    FLASH->CR &= ~FLASH_CR_PER; /* (7) */
    
    return error;
}


msg_t halFLASH_erase_check (uint32_t first_page_addr)
{
    uint32_t i;  

    for (i = FLASH_PAGE_SIZE; i > 0; i-=4) /* Check the erasing of the page by reading all the page value */
    {
        if (*(uint32_t *)(first_page_addr + i -4) != (uint32_t)0xFFFFFFFF) /* compare with erased value, all bits at1 */
        {
            return ERROR_EEPROM_ERASE; /* report the error to the main progran */
        }
    }
}


msg_t halFLASH_FlashWord16Prog (uint32_t flash_addr, uint16_t data)
{    
    msg_t error;
    /* (1) Set the PG bit in the FLASH_CR register to enable programming */
    /* (2) Perform the data write (half-word) at the desired address */
    /* (3) Wait until the BSY bit is reset in the FLASH_SR register */
    /* (4) Check the EOP flag in the FLASH_SR register */
    /* (5) clear it by software by writing it at 1 */
    /* (6) Reset the PG Bit to disable programming */
    FLASH->CR |= FLASH_CR_PG; /* (1) */
    *(__IO uint16_t*)(flash_addr) = data; /* (2) */
    while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (3) */
    {
        /* For robust implementation, add here time-out management */
    }  
    if ((FLASH->SR & FLASH_SR_EOP) != 0)  /* (4) */
    {
        FLASH->SR |= FLASH_SR_EOP; /* (5) */
    }
    /* Manage the error cases */
    else if ((FLASH->SR & FLASH_SR_PGERR) != 0) /* Check Programming error */
    {      
        error = ERROR_EEPROM_PROG;
        FLASH->SR |= FLASH_SR_PGERR; /* Clear it by software by writing EOP at 1*/
    }
    else if ((FLASH->SR & FLASH_SR_WRPERR) != 0) /* Check write protection */
    {      
        error = ERROR_EEPROM_WRITE_PROTECTION; 
        FLASH->SR |= FLASH_SR_WRPERR; /* Clear it by software by writing it at 1*/
    }
    else
    {
        error = ERROR_UNKNOWN; 
    }
    FLASH->CR &= ~FLASH_CR_PG; /* (6) */
    
    return error;
}

#endif

void halFLASH_eraseAllPages (void)
{
    /*
    FLASH->CR |= FLASH_CR_MER; //Устанавливаем бит стирания ВСЕХ страниц
    FLASH->CR |= FLASH_CR_STRT; //Начать стирание
//     while(!halFLASH_ready()){}; // Ожидание готовности.. Хотя оно уже наверное ни к чему здесь...

//     FLASH->CR &= FLASH_CR_MER;
    */
}


void halFLASH_flashWrite (uint32_t address, uint8_t *data, uint32_t cnt)
{
/*    uint32_t i;

    while (FLASH->SR & FLASH_SR_BSY){}; //Ожидаем готовности флеша к записи
    if (FLASH->SR & FLASH_SR_EOP)
    {
        FLASH->SR = FLASH_SR_EOP;
    }

    FLASH->CR |= FLASH_CR_PG; //Разрешаем программирование флеша

    for (i = 0; i < cnt; i += 2)
    {   //Пишем старшие 2 байта
        *(volatile uint16_t *)(address + i) = (((uint16_t)data[i + 1]) << 8) + data[i];
        while (!(FLASH->SR & FLASH_SR_EOP)){};
        FLASH->SR = FLASH_SR_EOP;
    }

    FLASH->CR &= ~(FLASH_CR_PG); //Запрещаем программирование флеша
    */
}


uint8_t halFLASH_read8 (uint32_t address)
{
    return (*(volatile uint8_t *) address);
}


uint16_t halFLASH_read16 (uint32_t address)
{
    return (*(volatile uint16_t *) address);
}


uint32_t halFLASH_read32 (uint32_t address)
{
    return (*(volatile uint32_t *) address);
}


void halFLASH_flashRead (uint32_t address, uint8_t *data, uint32_t cnt)
{
    uint32_t i;

    for (i = 0; i < cnt; i += 2)
    {
        data[i]     = halFLASH_read8 (address + i);
    }
}



