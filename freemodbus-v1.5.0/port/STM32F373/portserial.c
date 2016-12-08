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

#include "stm32f3xx_hal.h"

/* ----------------------- static functions ---------------------------------*/
//static 
UART_HandleTypeDef huart;
//static 
GPIO_TypeDef * DE_Port;
//static 
uint16_t DE_Pin;


/* ----------------------- Start implementation -----------------------------*/
void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
	if (xRxEnable) {
		HAL_GPIO_WritePin (DE_Port, DE_Pin, GPIO_PIN_RESET);
		__HAL_UART_ENABLE_IT (&huart, UART_IT_RXNE);
	} else {
		__HAL_UART_DISABLE_IT (&huart, UART_IT_RXNE);
	}

	if (xTxEnable) {
		HAL_GPIO_WritePin (DE_Port, DE_Pin, GPIO_PIN_SET);
		__HAL_UART_ENABLE_IT (&huart, UART_IT_TXE);
	} else {
		__HAL_UART_DISABLE_IT (&huart, UART_IT_TXE);
	}
}

BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
	huart.Init.Mode = UART_MODE_TX_RX;
	huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart.Init.OneBitSampling = UART_ONEBIT_SAMPLING_DISABLED;
	huart.Init.OverSampling = UART_OVERSAMPLING_16;
	huart.Init.StopBits = UART_STOPBITS_2;
	huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    
    /*
	switch (ucPORT) {
	case 0:
		huart.Instance = USART1;
		DE_Port = GPIOA;
		DE_Pin = GPIO_PIN_4;
		break;
	case 1:
		huart.Instance = USART2;
		DE_Port = GPIOA;
		DE_Pin = GPIO_PIN_1;
		break;
    case 2:
		huart.Instance = USART3;
		DE_Port = GPIOB;
		DE_Pin = GPIO_PIN_14;
		break;
	default:
		return FALSE;
	}*/

    huart.Instance = USART2;
    DE_Port = USART2_DIR_GPIO_Port;
    DE_Pin = USART2_DIR_Pin;
        
	huart.Init.BaudRate = ulBaudRate;

	switch (ucDataBits) {
		case 8:
			huart.Init.WordLength = UART_WORDLENGTH_8B;
			break;
		case 9:
			huart.Init.WordLength = UART_WORDLENGTH_9B;
			break;
		default:
			return FALSE;
	}

	switch (eParity) {
	case MB_PAR_NONE:
		huart.Init.Parity = UART_PARITY_NONE;
		break;
	case MB_PAR_EVEN:
		huart.Init.Parity = UART_PARITY_EVEN;
		break;
	case MB_PAR_ODD:
		huart.Init.Parity = UART_PARITY_ODD;
		break;
	default:
		return FALSE;
	}

    return HAL_OK == HAL_RS485Ex_Init (&huart, UART_DE_POLARITY_HIGH, 0, 0);
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
	huart.Instance->TDR = ucByte;
    return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */
	*pucByte = huart.Instance->RDR;
    return TRUE;
}

BOOL UART_IRQ_Handler(void) //USART_TypeDef * usart) 
{
	//if (usart == huart.Instance) 
    {
		if((__HAL_UART_GET_IT(&huart, UART_IT_RXNE) != RESET) && (__HAL_UART_GET_IT_SOURCE(&huart, UART_IT_RXNE) != RESET)) {
			pxMBFrameCBByteReceived();
			__HAL_UART_SEND_REQ(&huart, UART_RXDATA_FLUSH_REQUEST);
			return TRUE;
		}
		if((__HAL_UART_GET_IT(&huart, UART_IT_TXE) != RESET) &&(__HAL_UART_GET_IT_SOURCE(&huart, UART_IT_TXE) != RESET)) {
			pxMBFrameCBTransmitterEmpty();
			return TRUE;
		}
	}
	return FALSE;
}

