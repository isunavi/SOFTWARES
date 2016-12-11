/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: porttimer.c,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"
#include "stm32f0xx_hal.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- static functions ---------------------------------*/
static void prvvTIMERExpiredISR (void);

//static TIM_HandleTypeDef htim;

static uint16_t timeout = 0;
static uint16_t downcounter = 0;
extern uint16_t *modbus_delay;


/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortTimersInit (USHORT usTim1Timerout50us)
{

	RCC->APB2ENR |= RCC_APB2ENR_TIM16EN; /* (1) */
	 // 16.000.000MHz / 128 / 125 = 1000
    TIM16->CR1 = 0;
    TIM16->CR1 |= TIM_CR1_ARPE; //Включен режим предварительной записи регистра автоперезагрузки
	TIM16->PSC = 48 - 1;
	TIM16->ARR = 50; //50us, 20.000 kHz
    TIM16->DIER |= TIM_DIER_UIE;

    HAL_NVIC_SetPriority (TIM16_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ (TIM16_IRQn);

	/*
    htim.Instance = TIM16;
	htim.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim.Init.Prescaler = (HAL_RCC_GetPCLK1Freq() / 1000000) - 1;
	htim.Init.Period = 50 - 1;


    return HAL_OK == HAL_TIM_Base_Init (&htim) ? TRUE : FALSE;
    */

	timeout = usTim1Timerout50us;// + 20;//*modbus_delay; //2000; //ZAY - +20ms? тому что не успевал центральный контроллер перевести линию с передачи на прием и наш пакет ответа терялся?

	return TRUE;
}


//inline
void
vMBPortTimersEnable ()
{
    /* Enable the timer with the timeout passed to xMBPortTimersInit( ) */
	downcounter = timeout;
	//HAL_TIM_Base_Start_IT(&htim);
    // TIM_DIER_UIE
    //TIM16->DIER |= TIM_DIER_UIE;
    TIM16->CNT = 0;
    TIM16->CR1 |= TIM_CR1_CEN; //timer on
}

//inline
void
vMBPortTimersDisable ()
{
    /* Disable any pending timers. */
	//HAL_TIM_Base_Stop_IT(&htim);
     TIM16->CR1 &= ~TIM_CR1_CEN; //timer off
}

/* Create an ISR which is called whenever the timer has expired. This function
 * must then call pxMBPortCBTimerExpired( ) to notify the protocol stack that
 * the timer has expired.
 */
static void prvvTIMERExpiredISR (void)
{
    (void) pxMBPortCBTimerExpired ();
}


void TIM16_IRQHandler(void)
{
    volatile uint16_t status;

    status = TIM16->SR;
    if (((TIM_SR_UIF & status) != 0) && ((TIM16->DIER & TIM_DIER_UIE) != 0))
    {
        TIM16->SR &= ~TIM_SR_UIF;
        if (!--downcounter)
        {
			prvvTIMERExpiredISR ();
        }
    }
}
