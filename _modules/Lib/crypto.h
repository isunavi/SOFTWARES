/**
 * @file    crypto.h
 * @author  Антон Логинов, Ht3h5793, CD45
 * @date    12.10.2015  15:18
 * @version V1.0.0
 * @brief   
 * @todo 
 
 */

#ifndef CRYPTO_H
#define	CRYPTO_H 10151012

/** Раздел для "include" */

#include <defines.h>
#include <board.h>


/** Раздел для "define" */ 
/** Раскомментировать, будет чуть быстрее, но больше флеши необходимо */
//#define NEED_RC4        1
//#define NEED_TEA       1


/** Определения ответов функций */

/** Раздел для "typedef" */


#ifdef	__cplusplus
extern "C" {
#endif

    
#if NEED_RC4
void cryptoRC4_encrypt (uint8_t* key);
void cryptoRC4_encrypt (uint8_t* v);
void cryptoRC4_decrypt (uint8_t* v);
#endif
    
    
/**
https://ru.wikipedia.org/wiki/TEA

    value — исходный текст состоящий из двух частей по 32 бита
    key — ключ состоящий из четырёх 32-битных частей

*/
#if NEED_TEA
void cryptoTEA_encrypt (uint32_t *value, uint32_t *key);
void cryptoTEA_decrypt (uint32_t *value, uint32_t *key);
#endif


#ifdef	__cplusplus
}
#endif

#endif	/* CRYPTO_H */
