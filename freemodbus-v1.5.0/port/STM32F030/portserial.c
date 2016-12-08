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
 * File: $Id: portserial.c,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

#include "board.h"
     
/* ----------------------- static functions ---------------------------------*/
//static 
//extern UART_HandleTypeDef huart1;
uint32_t USART1_errCount;



/* ----------------------- Start implementation -----------------------------*/
void vMBPortSerialEnable (BOOL xRxEnable, BOOL xTxEnable)
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
	if (xRxEnable)
	{
		//HAL_GPIO_WritePin (DE_Port, DE_Pin, GPIO_PIN_RESET);
		GPIO_USART1_DIR_L;
		//__HAL_UART_ENABLE_IT (&huart1, UART_IT_RXNE);
		USART1->CR1 |= USART_CR1_RXNEIE;
	}
	else
	{
		//__HAL_UART_DISABLE_IT (&huart1, UART_IT_RXNE);
		USART1->CR1 &= ~USART_CR1_RXNEIE;
	}

	if (xTxEnable)
	{
		GPIO_USART1_DIR_H;
		USART1->CR1 |= USART_CR1_TXEIE;
	}
	else
	{
		USART1->CR1 &= ~USART_CR1_TXEIE;
	}
}


BOOL xMBPortSerialInit (UCHAR ucPORT,
		ULONG ulBaudRate,
		UCHAR ucDataBits,
		eMBParity eParity)
{

    /* Enable the peripheral clock of GPIOA */
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    /* GPIO configuration for USART1 signals */
    /* (4) Select AF mode (10) on PA9, PA10*/
    /* (5) AF1 for PA9 */
    /* (6) AF1 for PA10 */
    GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10))\
               | (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1);
    GPIOA->AFR[1] = (GPIOA->AFR[1] &~ (GPIO_AFRH_AFRH1)) | (1<<(1*4)); /* (5) */
    GPIOA->AFR[1] = (GPIOA->AFR[1] &~ (GPIO_AFRH_AFRH2)) | (1<<(2*4)); /* (6) */


    //------------- DIR ---------------------
    // Enable the peripheral clock of GPIO
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    // Select output mode (01)
    GPIOB->MODER = (GPIOB->MODER & ~(GPIO_MODER_MODER1)) | (GPIO_MODER_MODER1_0);
    GPIO_USART1_DIR_L; //DIR RX - on


    /* Enable the peripheral clock USART1 */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* Configure USART1 */
    /* (1) oversampling by 16, 9600 baud */
    /* (3) 8 data bit, 1 start bit, 1 stop bit, no parity, reception and transmission enabled */
    USART1->BRR = 480000 / 96; /* (1) */

    //uint32_t temp, fck;
    //fck = 48000000;
    //temp = fck + (ulBaudRate >> 1);
    //temp /= ulBaudRate;
    USART1->BRR = (48000000 << 4) / ulBaudRate; //temp;


    USART1->CR1 = USART_CR1_TE | USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_UE; /* (3) */

    while ((USART1->ISR & USART_ISR_TC) != USART_ISR_TC) /* polling idle frame Transmission */
    {
        /* add time out here for a robust application */
    }
    USART1->ICR |= USART_ICR_TCCF;/* Clear TC flag */
    //USART1->CR1 |= USART_CR1_TCIE;/* Enable TC interrupt */

    /* Configure IT */
    /* (4) Set priority for USART1_IRQn */
    /* (5) Enable USART1_IRQn */
    NVIC_SetPriority (USART1_IRQn, 0); /* (3) */
    NVIC_EnableIRQ (USART1_IRQn); /* (4) */


	/*

	switch (ucDataBits) {
		case 8:
			huart1.Init.WordLength = UART_WORDLENGTH_8B;
			break;
		case 9:
			huart1.Init.WordLength = UART_WORDLENGTH_9B;
			break;
		default:
			return FALSE;
	}

	switch (eParity) {
	case MB_PAR_NONE:
		huart1.Init.Parity = UART_PARITY_NONE;
		break;
	case MB_PAR_EVEN:
		huart1.Init.Parity = UART_PARITY_EVEN;
		break;
	case MB_PAR_ODD:
		huart1.Init.Parity = UART_PARITY_ODD;
		break;
	default:
		return FALSE;
	}

    return HAL_OK == HAL_RS485Ex_Init (&huart1, UART_DE_POLARITY_HIGH, 0, 0);
    */
    return TRUE;
}

BOOL xMBPortSerialPutByte (CHAR ucByte)
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
	//huart1.Instance->TDR = ucByte;
	USART1->TDR = (ucByte & 0x0FF);
    return TRUE;
}

BOOL xMBPortSerialGetByte (CHAR * pucByte)
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */
	//*pucByte = huart1.Instance->RDR;
	*pucByte = (USART1->RDR & 0x0FF);
    return TRUE;
}

BOOL UART_IRQ_Handler(void) //USART_TypeDef * usart)
{
	volatile uint16_t status;

	status = USART1->ISR;
    if ((USART_ISR_TXE & status) != 0)
    { // Проверяем, действительно ли прерывание вызвано окончанием передачи
        USART1_LED_INV; // инвертируем индикацию
		pxMBFrameCBByteReceived();
		//__HAL_UART_SEND_REQ (&huart1, UART_RXDATA_FLUSH_REQUEST);
		return TRUE;

    }
    if ((USART_ISR_RXNE & status) != 0)
    {
        USART1_LED_INV; // инвертируем индикацию
        pxMBFrameCBByteReceived ();
		//__HAL_UART_SEND_REQ (&huart1, UART_RXDATA_FLUSH_REQUEST);
		return TRUE;
    }

    if ((USART_ISR_NE & status)  /*!<Noise Error Flag */
        || (USART_ISR_FE & status)  /*!<Framing Error */
        || (USART_ISR_PE & status)  /*!<Parity Error */
        || (USART_ISR_ORE & status)) // /*!<OverRun Error */
    {
    	//USART_ClearITPendingBit(USARTX, USART_IT_ORE);
    	USART1->ISR &= ~USART_ISR_ORE;
        USART1_errCount++;
    }

	/*
	//if (usart == huart.Instance) 
    {
		if((__HAL_UART_GET_IT (&huart1, UART_IT_RXNE) != RESET) && (__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_RXNE) != RESET)) {
			pxMBFrameCBByteReceived();
			__HAL_UART_SEND_REQ(&huart1, UART_RXDATA_FLUSH_REQUEST);
			return TRUE;
		}
		if((__HAL_UART_GET_IT (&huart1, UART_IT_TXE) != RESET) &&(__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_TXE) != RESET)) {
			pxMBFrameCBTransmitterEmpty();
			return TRUE;
		}
	}
    */

	return FALSE;
}


/**
* @brief This function handles USART1 global interrupt.
*/
void USART1_IRQHandler(void)
{
	UART_IRQ_Handler ();
}

