/**
 * @file    crc.h
 * @author  Антон Логинов, Ht3h5793, CD45
 * @date    13.6.2013  15:50
 * @version V1.0.2
 * @brief   Табличное вычисление, быстрее, но больше памяти требует
 * @todo сделать норм программную 32CRC ? добавить inline? 
 * проверить максовскую СКС на корректность
 * 
 Пример:
    uint8_t buf[] = {"123456789"}; //
    uint8_t crc_8;
    uint8_t crc_8DS;
    uint16_t crc_16;
    uint32_t crc_32;
    uint8_t size = 9; //sizeof(buf);

    // Dallas CRC
    crc_8DS = crc8_DS (&buf[0], size);
    if (0xA1 != crc_8DS) while(1){};
    crc_8DS = 0x00; // см. описание почему для данного crc
    for (i = 0; i < size; i++)
    {
        crc8_DS_s (&crc_8DS, &buf[i]);
    }
    if (0xA1 != crc_8DS) while(1){};

    // 
    crc_8 = crc8 (&buf[0], size);
    if (0xF7 != crc_8) while(1){};
    crc_8 = 0xFF; // см. описание почему для данного crc
    for (i = 0; i < size; i++)
    {
        crc8_s (&crc_8, &buf[i]);
    }
    if (0xF7 != crc_8) while(1){};

    // CRC 16 CITT
    crc_16 = crc16_CITT (&buf[0], size);
    if (0x29B1 != crc_16) while(1){};
    crc_16 = 0xFFFF; // см. описание почему для данного crc
    for (i = 0; i < size; i++)
    {
        crc16_CITT_s (&crc_16, &buf[i]);
    }
    if (0x29B1 != crc_16) while(1){};

    // CRC 32
    crc_32 = crc32_CCITT (&buf[0], size);
    if (0x89A1897F != crc_32) while(1){};
    crc_32 = 0x00000000;//FFFFFFFF; // см. описание почему для данного crc
    for (i = 0; i < size; i++)
    {
        crc32_CCITT_s (&crc_32, &buf[i]);
    }
    //crc32 ^= 0xFFFFFFFF; // см. описание почему для данного crc
    if (0x89A1897F != crc_32) while(1){};

    
!!! Входные массивы должны быть кратной длины слова,
   например для crc32 массив должен быть кратен 4-ем байтам!!!
  Указаны два знака, первый это дл табличного, второй для программного
  Например, + -  - табличный ОК, программный неработает


Название алгоритма (name);
Степень порождающего контрольную сумму многочлена (width);
Сам производящий полином (poly). Для того, чтобы записать его в виде значения, его сначала записывают как битовую последовательность, при этом старший бит опускается — он всегда равен 1. К примеру, многочлен  в данной нотации будет записан числом . Для удобства полученное двоичное представление записывают в шестнадцатеричной форме. Для нашего случая оно будет равно  или 0x11;
Стартовые данные (init), то есть значения регистров на момент начала вычислений;
Флаг (RefIn), указывающий на начало и направление вычислений. Существует два варианта: False — начиная со старшего значащего бита (MSB-first), или True — с младшего (LSB-first);
Флаг (RefOut), определяющий, инвертируется ли порядок битов регистра при входе на элемент XOR;
Число (XorOut), с которым складывается по модулю 2 полученный результат;
Значение CRC (check) для строки «123456789» 

 * http://www.zorc.breitbandkatze.de/crc.html
 * http://ghsi.de/CRC/
 * 
 */

#ifndef CRC_H
#define	CRC_H 20160611

/** Раздел для "include" */

#include "board.h"


/** Раздел для "define" */ 
/** Раскомментировать, будет чуть быстрее, но больше флеши необходимо */
//#define NEED_CRC32_CCITT        1
//#define CRC32_CCITT_TABLE       1
//#define NEED_CRC16              1
//#define CRC16_TABLE             0
//#define NEED_CRC8               1
//#define CRC8_TABLE              0
//#define NEED_CRC8_DS            1
//#define CRC8_DS_TABLE           1


/** Определения ответов функций */

/** Раздел для "typedef" */


#ifdef	__cplusplus
extern "C" {
#endif

/*
  Name  : CRC-8 ? - type unknov
  Poly  : 0x31    x^8 + x^5 + x^4 + 1
  Init  : 0xFF
  Revert: false
  XorOut: 0x00
  Check : 0xF7 ("123456789") size = 9
  MaxLen: 15 байт (127 бит) - обнаружение
    одинарных, двойных, тройных и всех нечетных ошибок
*/
#if NEED_CRC8
uint8_t   crc8 (uint8_t *inData, uint8_t len); // + +
void      crc8_s (uint8_t *crc, uint8_t *inData); // + +
#endif


/*
  Name  : CRC-8 Dallas/Maxim 
  Poly  : 0x31    x^8 + x^5 + x^4 + 1 (0x31 / 0x8C / 0x98)
  Init  : 0x00
  Revert: false
  XorOut: 0x00
  Check : 0xA1 ("123456789") size = 9
  MaxLen: 
*/
#if NEED_CRC8_DS
uint8_t   crc8_DS (uint8_t *inData, uint8_t len); // + + 
void      crc8_DS_s (uint8_t *crc, uint8_t *inData); // + +
#endif


/**
  Name:   CRC 16/CITT
  Width:  16
  Poly:   0x1021    x^16 + x^12 + x^5 + 1
  Init:   0xFFFF
  RefIn:  False
  RefOut: False
  XorOut: 0x0000
  Check:  0x29B1 ("123456789") size = 9
  MaxLen: 4095 байт (32767 бит) - обнаружение
    одинарных, двойных, тройных и всех нечетных ошибок
*/
#if NEED_CRC16
uint16_t   crc16_CITT (uint8_t *inData, uint16_t len); // + + 
void       crc16_CITT_s (uint16_t *crc, uint8_t *inData); // + +  
uint16_t   _crc16M (uint8_t *inData, uint16_t len); // + + 
void       _crc16S (uint16_t *crc, uint8_t *inData); // + +  
#endif


/**
  Name  : CRC 32 CCITT IEEE 802.3
  Width : 32
  Poly  : 0x04C11DB7    x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 
                       + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1
  Init  : 0xFFFFFFFF
  RefIn : True
  RefOut: True
  XorOut: 0xFFFFFFFF
  Check : 0xCBF43926 ("123456789") size = 9
  MaxLen: 268 435 455 байт (2 147 483 647 бит) - обнаружение
   одинарных, двойных, пакетных и всех нечетных ошибок
*/
#if NEED_CRC32_CCITT
uint32_t   crc32_CCITT (uint8_t *inData, uint32_t len); // + -
void       crc32_CCITT_s (uint32_t *crc, uint8_t *inData); // + - 
#endif


#ifdef	__cplusplus
}
#endif

#endif	/* CRC_H */
