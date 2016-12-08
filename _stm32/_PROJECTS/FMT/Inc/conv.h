/**
 * @file    conv.h
 * @author  s.aksenov, Ht3h5793, CD45
 * @date    13.01.2013
 * @version V2.1.0
 * @brief   ѕреобразователи данных, работа со строками
 * @todo    ѕроверить на максимальное значение и выход за него!!! (буфер переполн€етс€
 *
 */

#ifndef CONV_H
#define	CONV_H  20150723

#include "board.h"

#ifdef	__cplusplus
extern "C" {
#endif

// Float в Int @todo
    
uint16_t _strlen (const char *str);
    
int32_t float2int32 (float number, uint8_t zeros);
char *_strncmp (char *strA, char *strB);
char *skip_space (char *ptr);
    
//return hex number in range 0-0xf or error 0xff if char is not a hexadecimal digit
uint8_t char2hex (char c);

uint8_t bcd2u8 (uint8_t val);
uint8_t dec2bcd (uint8_t val);



#ifdef	__cplusplus
}
#endif

#endif	/* CONV_H */
