#include "modKey.h"
#include "halKey.h"
#include "board.h"
#include "modSysClock.h"


typedef struct {
    //unsigned char   counter; /** Счетчик циклов состояния вывода */
    systime_t   holdTimeStart; /** Момент нажатия кнопки */
    systime_t   holdTimePeriod; /** Время по истечении которого кнопка считается удержанной */
    systime_t   releasedTimeStart; /** Момент отпускания кнопки */
    uint8_t     state; /** Состояние кнопки */
    systime_t   releaseTimeOut; /** Время сброса отпущенной кнопки, если ее состояние не опросили */
} T_KEY_STRUCT;

T_KEY_STRUCT keyboard [NUMBER_KEYS];


void modKey_init (void)
{
    uint8_t i = 0;
    
    halKey_init ();
    for (i = 0; i < NUMBER_KEYS; i++)  // set to default
    {
        keyboard[i].holdTimeStart       = 0; //modSysClock_getTime();
        keyboard[i].holdTimePeriod      = MODKEY_DEFAULT_HOLD_TIME_PERIOD;
        keyboard[i].releaseTimeOut      = MODKEY_DEFAULT_REALISED_TIMEOUT;
        keyboard[i].state               = MODKEY_STATE_NOT_PRESSED;
    }
}


uint8_t modKey_getState (uint8_t keyNumber)
{
    uint8_t nextState     = MODKEY_STATE_NOT_PRESSED;
    uint8_t returnValue   = keyboard[keyNumber].state;
    
    if (keyNumber < NUMBER_KEYS)
    {
        switch (keyboard[keyNumber].state)
        {
            case MODKEY_STATE_NOT_PRESSED:
                nextState   = MODKEY_STATE_NOT_PRESSED;
                break;
            
            case MODKEY_STATE_PRESSED:
                nextState   = MODKEY_STATE_HOLD;
                break;
            
            case MODKEY_STATE_HOLD_TIME:
                keyboard[keyNumber].holdTimeStart = modSysClock_getTime();
                nextState   = MODKEY_STATE_HOLD;
                break;
            
            case MODKEY_STATE_HOLD:
                nextState   = MODKEY_STATE_HOLD;
                break;
            
            case MODKEY_STATE_RELEASED:
                nextState   = MODKEY_STATE_NOT_PRESSED;
                break;
            
            default:
                // @todo : Вернуть ошибку
                // returnValue = !!!ОШИБКА!!!
                //nextState   = MODKEY_STATE_NOT_PRESSED;
                break;
        }
    } else {
        // @todo : Вернуть ошибку
        returnValue = MODKEY_STATE_NOT_PRESSED;
    }
    keyboard[keyNumber].state = nextState;
    
    return returnValue;
}


void modKey_run (void)
{
    uint8_t nextState = 0;
    systime_t time_tmp = 0;
    uint8_t keyNumber = 0;

    for (keyNumber = 0; keyNumber < NUMBER_KEYS; keyNumber++)
    {
        nextState = keyboard[keyNumber].state;
        switch (keyboard[keyNumber].state)
        {
            case MODKEY_STATE_NOT_PRESSED:
                if (halKey_getCount(keyNumber) >= MODKEY_COUNTER_VALUE_ON) //=
                {
                    nextState = MODKEY_STATE_PRESSED;
                    keyboard[keyNumber].holdTimeStart = modSysClock_getTime();
                }
                break;
                
            case MODKEY_STATE_PRESSED:
                if (halKey_getCount(keyNumber) < MODKEY_COUNTER_VALUE_OFF) //=
                {
                    nextState = MODKEY_STATE_RELEASED;
                    keyboard[keyNumber].releasedTimeStart = modSysClock_getTime();
                }
                else //Это нужно тут?
                {
                    time_tmp = modSysClock_getPastTime(keyboard[keyNumber].holdTimeStart, SYSCLOCK_GET_TIME_MS_1);
                    if (time_tmp >= keyboard[keyNumber].holdTimePeriod)
                    {
                        nextState = MODKEY_STATE_HOLD_TIME;
                    }
                }
                break;
                
            case MODKEY_STATE_HOLD_TIME:
                if (halKey_getCount(keyNumber) < MODKEY_COUNTER_VALUE_OFF) //=
                {
                    nextState = MODKEY_STATE_RELEASED;
                    keyboard[keyNumber].releasedTimeStart = modSysClock_getTime();
                }
                break;
                
            case MODKEY_STATE_HOLD:
                if (halKey_getCount(keyNumber) < MODKEY_COUNTER_VALUE_OFF) //=
                {
                    nextState = MODKEY_STATE_RELEASED;
                    keyboard[keyNumber].releasedTimeStart = modSysClock_getTime();
                }
                else
                {
                    time_tmp = modSysClock_getPastTime(keyboard[keyNumber].holdTimeStart, SYSCLOCK_GET_TIME_MS_1);
                    if (time_tmp >= keyboard[keyNumber].holdTimePeriod)
                    {
                        nextState = MODKEY_STATE_HOLD_TIME;
                    }
                }
                break;
                
            case MODKEY_STATE_RELEASED:
                time_tmp = modSysClock_getPastTime(keyboard[keyNumber].releasedTimeStart, SYSCLOCK_GET_TIME_MS_1);
                if (time_tmp >= keyboard[keyNumber].releaseTimeOut) 
                {
                    nextState = MODKEY_STATE_NOT_PRESSED;
                }
                break;
                
            default:
                nextState = MODKEY_STATE_NOT_PRESSED;
                break;
        }
        keyboard[keyNumber].state = nextState; // Переназначаем состояние кнопки
    }
}


void modKey_setHoldTimePeriod (uint8_t keyNumber, systime_t holdTimePeriod)
{
    keyboard[keyNumber].holdTimePeriod = holdTimePeriod;
}
