/**
 * @file    
 * @author  Ht3h5793
 * @date    13.01.2012
 * @version V2.0.0
 * @brief   ��������� ���������� �������, 
 ��� STM32F100C4T6B ��� ���1 ������ ��������� � ������ PA4
 * @HTTP    http://we.easyelectronics.ru/KT3012/odnostrochnye-pochti-melodii-na-stm32.html
 http://entropedia.co.uk/generative_music/
 * @todo    
///////
    ZEROBIT_INIT();
    while(1){};
///////
 */

#ifndef	ZEROBIT_H
#define	ZEROBIT_H 1

#include "defines.h"

/** ������ ��� "define" */

/** ������ ��� "typedef" */

/** ����������� ������� ������� */

#ifdef __cplusplus
extern "C" {
#endif

void ZEROBIT_INIT(void);

void ZEROBIT_TIM_IRQ(void);

#ifdef	__cplusplus
}
#endif

#endif	// ZEROBIT_H
