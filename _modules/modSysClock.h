/* sysClock - Модуль системного времени. 
 * Предназначен для отсчета временных интервалов в программе
 * Отсчет времени ведется в миллисекундах
 * Автор: Аксенов Сергей e-mail: Aksonov.Sergei@gmail.com
 
 * @file    modSysClock.h
 * @author  Аксенов Сергей (e-mail: Aksonov.Sergei@gmail.com), Ht3h5793
 * @date    08-October-2013
 * @version V6.6.1
 * @brief  
 * @todo  сделать на базе аппаратного счетчика
 
 // Пример:
    modSysClock_setRunPeriod (1000); // число вызовов в секунду
    systime_t _delay = modSysClock_getTime();
    while (1)
    {
        if (modSysClock_getPastTime (_delay, SYSCLOCK_GET_TIME_MS_1) > 1000))
        {  
            _delay = modSysClock_getTime();
            ...
        }
...
or
        if (FUNCTION_RETURN_OK == modSysClock_timeout (&_delay, 1000, SYSCLOCK_GET_TIME_MS_1))
        {
            ...
        }
    };
    
 !!! Переполнение счетчика каждые сутки?
 !!! Более точный алгоритм
http://piclist.ru/S-ZERO-ERR-TIMER-RUS/S-ZERO-ERR-TIMER-RUS.html

 * Изменения (2012.03.13):
 * - отсчёт времени ведётся в требуемых пользователю единицах (задаётся при вызове modSysClock_getPastTime, вторым параметром)
 * - введена возможность программно сообщить модулю периодичность вызова функции modSysClock_run (см. modSysClock_setRunPeriod)
 * - "по умолчанию" modSysClock_run должна вызываться с частотой 1 мс
 * - с вопросами/пожеланиями/предложениями всё равно обращаться: Aksonov.Sergei@gmail.com
*/

#ifndef MODSYSCLOCK_H
#define MODSYSCLOCK_H 20150507

#include "board.h"


//Вернуть прошедшее время в 
typedef enum {
    SYSCLOCK_GET_TIME_MKS_1     = 0, // единицах микросекунд
    SYSCLOCK_GET_TIME_MKS_10       , // десятках микросекунд
    SYSCLOCK_GET_TIME_MKS_100      , // сотнях микросекунд
    SYSCLOCK_GET_TIME_MS_1         , // единицах миллисекунд
    SYSCLOCK_GET_TIME_MS_10        , // десятках миллисекунд
    SYSCLOCK_GET_TIME_MS_100       , // сотнях миллисекунд
    SYSCLOCK_GET_TIME_S_1          , // единицах секунд
    SYSCLOCK_GET_TIME_S_10           // десятках секунд
} SYSCLOCK_STRUCT;


#ifdef	__cplusplus
extern "C" {
#endif
    
/**
 * Функция изменения периода вызова 
 * @param Period - периодичность вызова функции modSysClock_run, мкс
 */
void modSysClock_setRunPeriod (systime_t period);

/**
 * Счет времени. 
 * Данная функция должна вызываться с периодом, заданным функцией modSysClock_setRunPeriod.
 * По умолчанию должна вызываться с периодом 1 мс.
 */
void  modSysClock_run (void);

/**
 * Функция возвращает значение счётчика времени
 * @return текущее время в миллисекундах
 */
systime_t modSysClock_getTime (void);

/**
 * Возвращает время, прошедшее с момента startTime в запрошенном формате
 * @param startTime - начальное значение счётчика времени
 * @param retFormat - формат возвращаемого времени:
 *              SYSCLOCK_GET_TIME_MKS_10         - в десятках микросекунд
 *              SYSCLOCK_GET_TIME_MKS_100        - в сотнях микросекунд
 *              SYSCLOCK_GET_TIME_MS_1           - в миллисекундах
 *              SYSCLOCK_GET_TIME_MS_10          - в десятках микросекунд
 *              SYSCLOCK_GET_TIME_MS_100         - в сотнях миллисекунд
 *              SYSCLOCK_GET_TIME_S_1            - в секундах
 *              SYSCLOCK_GET_TIME_S_10           - в десятках секунд
 * @return Время, прошедшее от startTime, в требуемых единицах
 */
systime_t modSysClock_getPastTime (systime_t startTime, uint8_t retFormat);


/**
 * Возвращает время, прошедшее с момента startTime в запрошенном формате
 * @param pdelay -  значение счётчика времени
 * @param delay - значение
 * @param retFormat - формат возвращаемого времени:
 *              SYSCLOCK_GET_TIME_MKS_10         - в десятках микросекунд
 *              SYSCLOCK_GET_TIME_MKS_100        - в сотнях микросекунд
 *              SYSCLOCK_GET_TIME_MS_1           - в миллисекундах
 *              SYSCLOCK_GET_TIME_MS_10          - в десятках микросекунд
 *              SYSCLOCK_GET_TIME_MS_100         - в сотнях миллисекунд
 *              SYSCLOCK_GET_TIME_S_1            - в секундах
 *              SYSCLOCK_GET_TIME_S_10           - в десятках секунд
 * @return FUNCTION_RETURN_OK or FUNCTION_RETURN_ERROR
 */
msg_t modSysClock_timeout (systime_t *pdelay, systime_t delay, uint8_t retFormat);


#ifdef	__cplusplus
}
#endif

#endif /* MODSYSCLOCK_H */
