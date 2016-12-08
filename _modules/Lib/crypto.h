/**
 * @file    crypto.h
 * @author  ����� �������, Ht3h5793, CD45
 * @date    12.10.2015  15:18
 * @version V1.0.0
 * @brief   
 * @todo 
 
 */

#ifndef CRYPTO_H
#define	CRYPTO_H 10151012

/** ������ ��� "include" */

#include <defines.h>
#include <board.h>


/** ������ ��� "define" */ 
/** �����������������, ����� ���� �������, �� ������ ����� ���������� */
//#define NEED_RC4        1
//#define NEED_TEA       1


/** ����������� ������� ������� */

/** ������ ��� "typedef" */


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

    value � �������� ����� ��������� �� ���� ������ �� 32 ����
    key � ���� ��������� �� ������ 32-������ ������

*/
#if NEED_TEA
void cryptoTEA_encrypt (uint32_t *value, uint32_t *key);
void cryptoTEA_decrypt (uint32_t *value, uint32_t *key);
#endif


#ifdef	__cplusplus
}
#endif

#endif	/* CRYPTO_H */
