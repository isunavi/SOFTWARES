/* 
 * @file    
 * @author  Ht3h5793
 * @date    08.09.2013
 * @version V3.5.0
 * @brief   
 */

#ifndef DEBUG_H
#define	DEBUG_H 20150828

//#define DEBUG_ // если нужен дебаг, раскоментировать
#ifdef DEBUG_
    #include <stdio.h>
    #define dprintf(x) printf x
#else
    #define dprintf(x)
#endif
    
#include "board.h" 

    
#define    DWT_CYCCNT           *(volatile unsigned long *)0xE0001004
#define    DWT_CONTROL          *(volatile unsigned long *)0xE0001000

#define    SCB_DEMCR            *(volatile unsigned long *)0xE000EDFC
#define    DBGMCU_CR            *(volatile unsigned long *)0xE0042004 // debug
// макросы
#define    RESET_CORE_COUNT     (DWT_CYCCNT = 0)
#define    GET_CORE_COUNT       (DWT_CYCCNT - 1)

// // для дебага
// #define    DWT_CYCCNT    *(volatile unsigned long *)0xE0001004
// #define    DWT_CONTROL   *(volatile unsigned long *)0xE0001000
// #define    SCB_DEMCR     *(volatile unsigned long *)0xE000EDFC
// unsigned long tic = 0 ;
// //SCB_DEMCR   |= 0x01000000; // Настраиваем
// //DWT_CONTROL |= 1; // включаем счётчик

// //DWT_CYCCNT   = 0; // обнуляем счетчик
//  //   delay_us(1000); //Delay( (52000-1000)/375 );  // 375 nS
// //tic = DWT_CYCCNT-1;

#ifdef	__cplusplus
extern "C" {
#endif
    
void _delay_init (void);
void _debug_init (void);

//void _debug_setup(void); // эта функция всеголишь записывает в регистр SWO, тем самым настраивая дебаг

void _delay_ms (uint16_t delay);
void _delay_us (uint16_t delay);

#ifdef	__cplusplus
}
#endif

#endif /** DEBUG_H */
