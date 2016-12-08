/**
 * @file    
 * @author  Ht3h5793, CD45, Ruben H. Meleca
 * @date    13.01.2013
 * @version V2.0.0
 * @brief   Преобразователи данных, работа со строками
 * @HTTP    http://www.artekit.eu
 * @todo    Проверить на максимальное значение и выход за него!!! (буфер переполняется
 *
 */

#ifndef	MATAN_H
#define	MATAN_H 5

#include "defines.h"
#include <math.h>

/** Раздел для "define" */
#define M_PI      3.1415926535897932384626433832795 // примерно

#define fixed     int32_t
#define roundf(x) floor(x + 0.5f)
#define round(x)  floor(x + 0.5)

/** Раздел для "typedef" */

/** Определения ответов функций */

/** Вычисление числа пи
int a=10000,b,c=2800,d,e,f[2801],g;main(){for(;b-c;)f[b++]=a/5; 
for(;d=0,g=c*2;c-=14,printf("%.4d",e+d/a),e=d%a)for(b=c;d+=f[b]*a, f[b]=d%--g,d/=g--,--b;d*=b);} 
*/

#ifdef __cplusplus
extern "C" {
#endif


//*********************************
//	Return sine value from 'angle' parameter in degrees. The returned value
//	is multiplied by 10000 (-10000..0..10000). The angle must be an integer
//	value from 0 to 359.
//********************************
int16_t		_sin(uint16_t angle);		// Return sine value from 0 to +-10000
    //*********************************
//	Return cosine value from 'angle' parameter in degrees. The returned
//	value is multiplied by 10000 (-10000..0..10000). The angle must be an
//	integer value from 0 to 359
//**********************************
int16_t		_cos(uint16_t angle);		// Return cosine value from 0 to +-10000

// хеш-функция - http://habrahabr.ru/post/219139/
uint32_t hashLy(char *str);
    

//------------------ Fixed point --------------
// http://compgraphics.info/2D/fixed.php

// представляет целое число в формате чисел с фиксированной точкой
inline fixed int_to_fixed(int32_t value)
{
    return (value << 16);
}

// целая часть числа с фиксированной точкой
inline int32_t fixed_to_int(fixed  value)
{
    if (value < 0) return ((value >> 16) - 1);
    if (value >= 0) return (value >> 16);
}

// округление до ближайшего целого
inline int round_fixed(fixed value)
{
    return fixed_to_int(value + 5 << 15);
}

// представляет число с плавающей точкой в формате чисел с фиксированной точкой
// здесь происходят большие потери точности
inline fixed double_to_fixed(double value)
{
      return round(value * (65536.0));
}

inline fixed float_to_fixed(float value)
{
    return roundf(value * (65536.0f));
}

// записывает отношение (a / b) в формате чисел с фиксированной точкой
inline fixed frac_to_fixed(int32_t a, int32_t b)
{
    return (a << 16) / b;
}





/**
http://ru.wikipedia.org/wiki/Direct_Digital_Synthesizer

Здесь dph — приращение фазы, phase — текущая (мгновенная) фаза,
amp — текущая (мгновенная) амплитуда синтезированного 
гармонического сигнала. Если функция next_amp вызывается с
тактовой частотой Fc, то ее возвращаемые значения будут
представлять собой выборки синусоидального сигнала с частотой
Fc*dph/2^{32} и амплитудой 511. Эта амплитуда соответствует 
диапазону входных значений 10-разрядного ЦАП. Здесь также 
использовано свойство периодичности функции синуса, а именно 
тот факт, что при переполнении аккумулятора фазы phase, его 
значение изменяется на 232, а аргумент синуса — на 2?, что не
влияет на результат.
*/

int next_amp(int dph)
{
    static int phase = 0;
    int amp;
    phase += dph;
    amp = 511.5 * sin(2 * M_PI * phase / 0x100000000L);
    return amp;
}

uint32_t _next_amp(uint32_t dph)
{
    static uint32_t phase = 0;
    uint32_t amp;
    phase += dph;
    amp = 511.5 * sin(2 * M_PI * phase / 0x100000000L);
    return amp;
}


#ifdef	__cplusplus
}
#endif

#endif	// MATAN_H
