/* 

 */

#ifndef HALFLASH_STM32F10X_V1_H
#define	HALFLASH_STM32F10X_V1_H 20150716
/**
 *  Раздел для "include"
 */

//===== Окончание раздела include ==============================================


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
    
void      halFLASH_unlock (void);
void      halFLASH_lock (void);


//Функция стирает ВСЕ страницы. 
// При её вызове прошивка самоуничтожается. Суицид!
void      halFLASH_eraseAllPages (void);

//Функция стирает одну страницу. В качестве адреса можно использовать любой
//принадлежащий диапазону адресов той странице которую нужно очистить.
void      halFLASH_erasePage (uint32_t address);


//data - указатель на записываемые данные
//address - адрес во flash
//count - количество записываемых байт, должно быть кратно 2
void      halFLASH_flashWrite (uint32_t address, uint8_t *data, uint32_t cnt);

/**
 * 
 * @param adress - адресс в памяти, максимум 65565 вроде
 * @param pBuf   - указатель на буфер
 * @param numBuf - число байт, максимум 32
 * @return I2C_FUNCTION_RETURN_READY         - готово, данные записались
 *         I2C_FUNCTION_RETURN_ERROR_TIMEOUT - где-то ошибка, при следующем обращении будет выполняться повторная запись
 */
uint8_t  halFLASH_read (uint32_t address);
void      halFLASH_flashRead (uint32_t address, uint8_t *data, uint32_t cnt);


#ifdef	__cplusplus
}
#endif

#endif	/* HALFLASH_STM32F10X_V1_H */

