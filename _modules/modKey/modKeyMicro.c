#include "modKey.h"
#include "halKey.h"
#include "defines.h"
#include "modSysClock.h"


typedef struct {
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
    uint8_t returnValue   = key.state; //сразу запоминаем!

    switch (key.state)
    {
        case MODKEY_STATE_NOT_PRESSED:
            nextState   = MODKEY_STATE_NOT_PRESSED;
            break;

        case MODKEY_STATE_PRESSED:
            nextState   = MODKEY_STATE_HOLD;
            break;

        case MODKEY_STATE_HOLD_TIME:
            nextState   = MODKEY_STATE_HOLD;
            key.holdTimeStart = modSysClock_getTime();
            break;

        case MODKEY_STATE_HOLD:
            nextState   = MODKEY_STATE_HOLD;
            break;

        case MODKEY_STATE_RELEASED:
            nextState   = MODKEY_STATE_NOT_PRESSED;
            break;

//        default:
//            // @todo : Вернуть ошибку
//            // returnValue = !!!ОШИБКА!!!
//            nextState   = MODKEY_STATE_NOT_PRESSED;
//            break;
    }
    key.state = nextState;

    return returnValue;
}


void modKey_run (void)
{
    uint8_t nextState = 0;
    systime_t TimePass = 0;
    //uint8_t keyNumber = 0;

    nextState = key.state;
    switch (key.state)
    {
        case MODKEY_STATE_NOT_PRESSED:
            if (halKey_getCount (keyNumber) > MODKEY_COUNTER_VALUE_ON)
            {
                nextState = MODKEY_STATE_PRESSED;
                key.holdTimeStart = modSysClock_getTime();
            }
            break;

        case MODKEY_STATE_PRESSED:
            if (halKey_getCount(keyNumber) < MODKEY_COUNTER_VALUE_OFF)
            {
                nextState = MODKEY_STATE_RELEASED;
                key.releasedTimeStart = modSysClock_getTime();
            } else {
                TimePass = modSysClock_getPastTime(key.holdTimeStart);
                if (TimePass >= key.holdTimePeriod)
                {
                    nextState = MODKEY_STATE_HOLD_TIME;
                }
            }
            break;

        case MODKEY_STATE_HOLD_TIME:
            if (halKey_getCount(keyNumber) < MODKEY_COUNTER_VALUE_OFF)
            {
                nextState = MODKEY_STATE_RELEASED;
                key.releasedTimeStart = modSysClock_getTime();
            }
            break;

        case MODKEY_STATE_HOLD:
            if (halKey_getCount(keyNumber) < MODKEY_COUNTER_VALUE_OFF)
            {
                nextState = MODKEY_STATE_RELEASED;
                key.releasedTimeStart = modSysClock_getTime();
            }
            else
            {
                TimePass = modSysClock_getPastTime(key.holdTimeStart);
                if (TimePass >= key.holdTimePeriod)
                {
                    nextState = MODKEY_STATE_HOLD_TIME;
                }
            }
            break;

        case MODKEY_STATE_RELEASED:
//            TimePass = modSysClock_getPastTime(key.releasedTimeStart);
//            if (TimePass > key.releaseTimeOut)
            {
                nextState = MODKEY_STATE_NOT_PRESSED;
            }
            break;

//        default:
//            nextState = MODKEY_STATE_NOT_PRESSED; // @todo Доспбросить все остальное, а то, наверное, заглючило
//            break;
    }
    key.state = nextState; // Переназначаем состояние кнопки
}


void modKey_setHoldTimePeriod (uint8_t keyNumber, systime_t holdTimePeriod)
{
    keyboard[keyNumber].holdTimePeriod = holdTimePeriod;
}
