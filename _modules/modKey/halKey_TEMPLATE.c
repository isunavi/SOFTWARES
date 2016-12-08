/*
 * @file    
 * @author  Ht3h5793
 * @date    10.03.2014
 * @version V9.0.0
 * @brief  
*/

#include <.\modKey\halKey.h>
#include <modKey_local.h>

#include <board.h>
#include <defines.h>

/** Активное состояние кнопки */
#define MODKEY_PIN_STATE_ACTIVE         1
#define MODKEY_PIN_STATE_NOACTIVE       0

// Тут явно объявлен массив фильтрационных счетчиков
uint8_t halKeyCnt [NUMBER_KEYS];


void halKey_init(void) {
    uint8_t i;
    
#ifdef BOARD_STM32F4DISCOVERY
    GPIO_InitTypeDef   GPIO_InitStructure;  
    /** Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    
    GPIO_StructInit( &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_KEY_OK | 
                                    GPIO_Pin_KEY_UP |
                                    GPIO_Pin_KEY_DOWN |
                                    GPIO_Pin_KEY_LEFT |
                                    GPIO_Pin_KEY_RIGHT;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP; // Подтяжка к 3.0V !
    GPIO_Init(GPIO_KEY, &GPIO_InitStructure);
#endif

    // Заполнения массива кнопок значениями по умолчанию
    for (i = 0; i < NUMBER_KEYS; i++) {
        halKeyCnt[i] = HALKEY_COUNTER_V_MIN;
    }
}


/**
 * Устранение дребезга контактов
 * @param keyNumber - номер кнопки.
 * @param pinState - физическое состояние кнопки(вывода)
 */

uint8_t halKey_getPinState(uint8_t pin) {
    uint8_t PinState = MODKEY_PIN_STATE_NOACTIVE;
    
    switch(pin) { // Сравнивать только с 0!
#ifdef BOARD_STM32F4DISCOVERY
        case KEY_UP:
            if ((GPIO_KEY->IDR &  GPIO_Pin_KEY_UP) == 0) { // Нажатие - к GND!
                PinState =  MODKEY_PIN_STATE_ACTIVE;
            }
            break;
            
        case KEY_DOWN:
            if ((GPIO_KEY->IDR &  GPIO_Pin_KEY_DOWN) == 0) {
                PinState =  MODKEY_PIN_STATE_ACTIVE;
            }
            break;
            
        case KEY_LEFT:
            if ((GPIO_KEY->IDR &  GPIO_Pin_KEY_LEFT) == 0) {
                PinState =  MODKEY_PIN_STATE_ACTIVE;
            }
            break;
            
        case KEY_RIGHT:
            if ((GPIO_KEY->IDR &  GPIO_Pin_KEY_RIGHT) == 0) {
                PinState =  MODKEY_PIN_STATE_ACTIVE;
            }
            break;
            
        case KEY_OK:
            if ((GPIO_KEY->IDR &  GPIO_Pin_KEY_OK) == 0) {
                PinState =  MODKEY_PIN_STATE_ACTIVE;
            }
            break;
#endif

        default: break;
    }

    return PinState;
}


void halKey_run (void) {
    uint8_t i = 0;
    
    for (i = 0; i < NUMBER_KEYS; i++) { // Устранение дребезга контактов
        if (MODKEY_PIN_STATE_ACTIVE == halKey_getPinState(i)) {
            halKeyCnt[i]++;
            if (halKeyCnt[i] > HALKEY_COUNTER_V_MAX) {
                halKeyCnt[i] = HALKEY_COUNTER_V_MAX;
            } 
        } else {
            halKeyCnt[i]--;
            if (halKeyCnt[i] < HALKEY_COUNTER_V_MIN) {
                halKeyCnt[i] = HALKEY_COUNTER_V_MIN;
            } 
        }
    }
}


uint8_t halKey_getCount (uint8_t num) {
    return halKeyCnt[num];
}
