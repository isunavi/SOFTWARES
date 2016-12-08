#include "modSysClockMicro.h"
#include "board.h"


/**
 * Максимально возможное время, мкс
 */
#define SYSCLOCK_MAX_TIME          0xFFFF

/**
 * Значение "по умолчанию" для тактирования SysClock = 1 мс
 */
#define SYSCLOCK_DEFAULT_PERIOD    1

//=========================================================================
/**
 * Преобразует микросекундные тики в требуемый масштаб
 * @param pastTime - время в микросекундах
 * @param retFormat - требуемый формат времени:
 *              SYSCLOCK_GET_TIME_MKS_10         - в десятках микросекунд
 *              SYSCLOCK_GET_TIME_MKS_100        - в сотнях микросекунд
 *              SYSCLOCK_GET_TIME_MS_1           - в миллисекундах
 *              SYSCLOCK_GET_TIME_MS_10          - в десятках микросекунд
 *              SYSCLOCK_GET_TIME_MS_100         - в сотнях микросекунд
 *              SYSCLOCK_GET_TIME_S_1            - в секундах
 *              SYSCLOCK_GET_TIME_S_10           - в десятках секунд
 * 
 * @return время в единицах требуемого формата
 */
#define modSysClock_formatTime(a) a
//=========================================================================


static systime_t currentTime;
//=========================================================================

/**
 * Счет времени. 
 * Данная функция должна вызываться с периодом, заданным функцией modSysClock_setRunPeriod.
 * По умолчанию должна вызываться с периодом 1 мс.
 */
void modSysClock_run (void)
{
    currentTime++;
}


/**
 * Функция возвращает значение счётчика времени
 * @return текущее время в миллисекундах
 */
systime_t modSysClock_getTime(void)
{
    return currentTime;
}


/**
 * Возвращает время, прошедшее с момента startTime в запрошенном формате
 * обрабатывает переполнение счетчика currentTime
 * @param startTime - начальное значение счётчика времени
 * @return Время, прошедшее от startTime, в требуемых единицах
 */
systime_t modSysClock_getPastTime (systime_t startTime, uint8_t retFormat)
{
    systime_t pastTime = 0;

    if (startTime <= currentTime)
    {
        pastTime = currentTime - startTime;
    } else {
        pastTime = SYSCLOCK_MAX_TIME - startTime + currentTime + 1;
    }

    pastTime = modSysClock_formatTime(pastTime);

    return pastTime;
}


msg_t modSysClock_timeout (systime_t *pdelay, systime_t delay, uint8_t retFormat)
{
    msg_t respond = FUNCTION_RETURN_ERROR;
    
    if (modSysClock_getPastTime (*pdelay, retFormat) >= delay)
    {  
        *pdelay = modSysClock_getTime();
        respond = FUNCTION_RETURN_OK;
    }
    
    return respond;
}
