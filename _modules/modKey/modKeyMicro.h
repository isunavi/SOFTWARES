/*
 * @file    hal LCD
 * @author  AKsenov, Ht3h5793
 * @date    05.03.2014
 * @version V6.5.2
 * @brief  
    @todo Добавить проверку на разных платформах
    
modKey_init ();
...
while (1) {
    modKey_run ();
    if (modKey_getState (KEY_UP) == MODKEY_STATE_PRESSED) {
        
    }
}
    

*/
#ifndef MODKEY_H
#define MODKEY_H 20150506

#include <defines.h>
#include <modKey_local.h.>

// Состояния кнопки
typedef enum {
    MODKEY_STATE_NOT_PRESSED  = 0, /** Кнопка не нажата */
    MODKEY_STATE_PRESSED      = 1, /** Кнопка была нажата */
    MODKEY_STATE_RELEASED     = 2, /** Кнопка была нажата и отпущена */
    MODKEY_STATE_HOLD         = 3, /** Кнопка была нажата и удержана */
    MODKEY_STATE_HOLD_TIME    = 4  /** Кнопка была удержана в течение заданного времени удержания */
} KEY_STATE_t;


#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Инициализация модуля.
 */
void modKey_init (void);

/**
 * Передача состояния кнопки вызывающему
 * @param keyNumber - номер кнопки
 * @return          - состояние кнопки
 */
uint8_t modKey_getState (uint8_t keyNumber);

/**
 * Определение состояния кнопки
 * @param keyNumber - номер кнопки.
 * @param pinState - физическое состояние кнопки(вывода).
 */
void modKey_run (void);

/**
 * установить время удержания кнопки
 * @param holdTimePeriod необходимый период для установки состояния удержания
 */
void modKey_setHoldTimePeriod (uint8_t keyNumber, systime_t holdTimePeriod);

/**
 * установить время отпускания кнопки
 * @param holdTimePeriod необходимый период для установки состояния удержания
 */
void modKey_setReleaseTimeout (uint8_t keyNumber, systime_t releaseTimeout);


#ifdef	__cplusplus
}
#endif

#endif /* MODKEY_H */
