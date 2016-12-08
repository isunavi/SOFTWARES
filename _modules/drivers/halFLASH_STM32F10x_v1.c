// http://habrahabr.ru/post/213771/
// http://we.easyelectronics.ru/STM32/programmirovanie-flash.html
// https://github.com/nabilt/STM32F4-Discovery-Firmware/blob/master/Project/Peripheral_Examples/FLASH_Program/main.c
//

#include "halFLASH.h"

#include "defines.h"
#include "board.h"


// struct SavedDomain_t{
//     uint8_t hh;
//     uint8_t hh1;
//     uint8_t hh2;
//     uint8_t hh3;
//     uint8_t hh4;
//     uint8_t hh5;
// };

// const SavedDomain_t* SavedDomain = (SavedDomain_t *)(HAL_FLASH_BASE + 1024 * 4);
// // #define SavedDomain (*(SavedDomain_t*)(FLASH_BASE+1024*14));
// //const SavedDomain_t* SavedDomain = (SavedDomain_t*)(FLASH_BASE+1024*14);


// Секретные ключи, никому не говорите и не показывайте их!
#define FLASH_KEY1 ((uint32_t)0x45670123)
#define FLASH_KEY2 ((uint32_t)0xCDEF89AB)


void halFLASH_unlock (void)
{
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
    //FLASH_Unlock();
    //FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FLASH->SR &= ~(FLASH_SR_EOP | FLASH_SR_PGERR | FLASH_SR_WRPRTERR);
}


void halFLASH_lock (void)
{
    FLASH->CR |= FLASH_CR_LOCK;
}


// //Функция возврщает true когда можно стирать или писать память.
// uint8_t halFLASH_ready(void)
// {
//     return !(FLASH->SR & FLASH_SR_BSY);
// }



void halFLASH_eraseAllPages(void)
{
    FLASH->CR |= FLASH_CR_MER; //Устанавливаем бит стирания ВСЕХ страниц
    FLASH->CR |= FLASH_CR_STRT; //Начать стирание
//     while(!halFLASH_ready()){}; // Ожидание готовности.. Хотя оно уже наверное ни к чему здесь...

//     FLASH->CR &= FLASH_CR_MER;
}


void halFLASH_erasePage (uint32_t address)
{
    halFLASH_unlock ();
    while (FLASH->SR & FLASH_SR_BSY){};
    if (FLASH->SR & FLASH_SR_EOP)
    {
        FLASH->SR = FLASH_SR_EOP;
    }
    
    FLASH->CR |= FLASH_CR_PER; //Устанавливаем бит стирания одной страницы
    FLASH->AR = HAL_FLASH_EEPROM_ADRESS + address; // Задаем её адрес
    FLASH->CR |= FLASH_CR_STRT; // Запускаем стирание 
    while (!(FLASH->SR & FLASH_SR_EOP)){}; //Ждем пока страница сотрется. 
    FLASH->SR = FLASH_SR_EOP;
    FLASH->CR &= ~FLASH_CR_PER; //Сбрасываем бит обратно
    halFLASH_lock ();
}


void halFLASH_flashWrite (uint32_t address, uint8_t *data, uint32_t cnt)
{
    uint32_t i;

    halFLASH_unlock ();
    while (FLASH->SR & FLASH_SR_BSY){}; //Ожидаем готовности флеша к записи
    if (FLASH->SR & FLASH_SR_EOP)
    {
        FLASH->SR = FLASH_SR_EOP;
    }

    FLASH->CR |= FLASH_CR_PG; //Разрешаем программирование флеша

    for (i = 0; i < cnt; i += 2)
    {   //Пишем старшие 2 байта
        *(volatile uint16_t *)(HAL_FLASH_EEPROM_ADRESS + address + i) = (((uint16_t)data[i + 1]) << 8) + data[i];
        while (!(FLASH->SR & FLASH_SR_EOP)){};
        FLASH->SR = FLASH_SR_EOP;
    }

    FLASH->CR &= ~(FLASH_CR_PG); //Запрещаем программирование флеша
    halFLASH_lock ();
}


uint8_t halFLASH_read (uint32_t address)
{
    return (*(volatile uint8_t *) HAL_FLASH_EEPROM_ADRESS + address);
}


void halFLASH_flashRead(uint32_t address, uint8_t *data, uint32_t cnt)
{
    uint32_t i;

    for (i = 0; i < cnt; i += 2)
    {   //Пишем старшие 2 байта
        data[i]     = halFLASH_read(HAL_FLASH_EEPROM_ADRESS + address + i);// & 0x00FF;
        //data[i + 1] = halFLASH_read(HAL_FLASH_EEPROM_ADRESS + address + i) >> 8;
    }
}
