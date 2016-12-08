#include "modSysClock.h"
#include "board.h"

/**
 * Максимально возможное время, мкс
 */
#define SYSCLOCK_MAX_TIME          0xFFFFFFFF

/**
 * Значение "по умолчанию" для тактирования SysClock = 1 мс
 */
#define SYSCLOCK_DEFAULT_PERIOD    TIC_PERIOD   


//====================================================================================================================================================
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
systime_t modSysClock_formatTime (systime_t pastTime, uint8_t retFormat);
//====================================================================================================================================================


static systime_t currentTime;
static systime_t modSysClockPeriod = SYSCLOCK_DEFAULT_PERIOD;
//====================================================================================================================================================


/**
 * Функция изменения периода вызова 
 * @param Period - периодичность вызова функции modSysClock_run, мкс
 */
void modSysClock_setRunPeriod (systime_t period)
{
    modSysClockPeriod = period;
}


/**
 * Счет времени. 
 * Данная функция должна вызываться с периодом 1мс. 
 */
void  modSysClock_run (void)
{
    currentTime += modSysClockPeriod;
}


/**
 * Функция возвращает текущее значение счётчика времени
 * @return текущее время в миллисекундах
 */
systime_t modSysClock_getTime (void)
{
    return currentTime;
}


/**
 * Возвращает время, прошедшее с момента startTime в запрошенном формате
 * обрабатывает переполнение счетчика currentTime
 * @param startTime - начальное значение счётчика времени
 * @param retFormat - формат возвращаемого времени:
 *              SYSCLOCK_GET_TIME_MKS_10         - в десятках микросекунд
 *              SYSCLOCK_GET_TIME_MKS_100        - в сотнях микросекунд
 *              SYSCLOCK_GET_TIME_MS_1           - в миллисекундах
 *              SYSCLOCK_GET_TIME_MS_10          - в десятках микросекунд
 *              SYSCLOCK_GET_TIME_MS_100         - в сотнях микросекунд
 *              SYSCLOCK_GET_TIME_S_1            - в секундах
 *              SYSCLOCK_GET_TIME_S_10           - в десятках секунд
 * @return Время, прошедшее от startTime, в требуемых единицах
 */
systime_t modSysClock_getPastTime (systime_t startTime, uint8_t retFormat)
{
    systime_t pastTime = 0;

    if (startTime <= currentTime)
    {
        pastTime = currentTime - startTime;
    } else {
        pastTime = (systime_t)SYSCLOCK_MAX_TIME - startTime + currentTime + 1;
    }
    
    pastTime = modSysClock_formatTime (pastTime, retFormat);
    
    return pastTime;
}


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
systime_t modSysClock_formatTime (systime_t pastTime, uint8_t retFormat)
{
    uint32_t retVal = 0;
    uint32_t divider = 1;
    
    switch(retFormat)
    {
        case SYSCLOCK_GET_TIME_MKS_10:
            divider = (TIC_PERIOD / 100);
            break;
        
        case SYSCLOCK_GET_TIME_MKS_100:
            divider = (TIC_PERIOD / 10);
            break;
        
        case SYSCLOCK_GET_TIME_MS_1:
            divider = (TIC_PERIOD);
            break;
        
        case SYSCLOCK_GET_TIME_MS_10:
            divider = (TIC_PERIOD * 10);
            break;
        
        case SYSCLOCK_GET_TIME_MS_100:
            divider = (TIC_PERIOD * 100);
            break;
        
        case SYSCLOCK_GET_TIME_S_1:
            divider = (TIC_PERIOD * 1000);
            break;
        
        case SYSCLOCK_GET_TIME_S_10:
            divider = (TIC_PERIOD * 10000);
            break;
        
        default: break;
    }
            
    retVal = pastTime / divider;
    
    return retVal;
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
