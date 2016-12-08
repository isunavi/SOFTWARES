#include "halUSART.h"
#include "defines.h"
#include "board.h"


#ifdef HAL_USART1

//#define TBUF1_SIZE   (USART1_TBUF_SIZE) /*** Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/
//#define RBUF1_SIZE   (USART1_RBUF_SIZE) /*** Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/

#if USART1_TBUF_SIZE < 2
    #error "USART1_TBUF_SIZE is too small.  It must be larger than 1."
#elif ((USART1_TBUF_SIZE & (USART1_TBUF_SIZE - 1)) != 0)
    #error "USART1_TBUF_SIZE must be a power of 2."
#endif

#if USART1_RBUF_SIZE < 2
    #error "USART1_RBUF_SIZE is too small.  It must be larger than 1."
#elif ((USART1_RBUF_SIZE & (USART1_RBUF_SIZE - 1)) != 0)
    #error "USART1_RBUF_SIZE must be a power of 2."
#endif

static struct tbuf_1_ {
    uint32_t in; // Next In Index
    uint32_t out; // Next Out Index
    uint8_t buf [USART1_TBUF_SIZE]; // Buffer
} tbuf_1;

static struct rbuf_1_ {
    uint32_t in; // Next In Index
    uint32_t out; // Next Out Index
    uint8_t buf [USART1_RBUF_SIZE]; // Buffer
} rbuf_1;

//static struct buf_st_1 rbuf_1 = { 0, 0, };
#define SIO_RBUFLEN_1 ((uint16_t)(rbuf_1.in - rbuf_1.out)) 

//static struct buf_st_1 tbuf_1 = { 0, 0, };
#define SIO_TBUFLEN_1 ((uint16_t)(tbuf_1.in - tbuf_1.out))

static uint32_t tx_restart_1 = 1; // NZ if TX restart is required


static uint32_t USART1_RecBytes = 0;
uint32_t    halUSART1_getRecBytes (void)
{
    return USART1_RecBytes;
}

static uint32_t USART1_SendBytes = 0;
uint32_t    halUSART1_getSendBytes (void)
{
    return USART1_SendBytes;
}

static uint32_t USART1_errCount = 0; // просто счетчик ошибок
uint32_t    halUSART1_getErrors (void)
{
    return USART1_errCount;
}


void halUSART1_flush (void)
{
    tbuf_1.in = 0; // Clear com buffer indexes
    tbuf_1.out = 0;
    tx_restart_1 = 1;

    rbuf_1.in = 0;
    rbuf_1.out = 0;
}


//------------------------------------------------------------------------------
// Отправка байта/ов
//------------------------------------------------------------------------------
msg_t halUSART1_sndS (uint8_t dat)
{
    msg_t resp = FUNCTION_RETURN_ERROR;
    struct tbuf_1_ *p = &tbuf_1;
    
    if (SIO_TBUFLEN_1 < USART1_TBUF_SIZE)
	{ // If the buffer is full, return an error value
        p->buf [p->in & (USART1_TBUF_SIZE - 1)] = dat; // Add data to the transmit buffer.
        p->in++;
        if (tx_restart_1) { // If transmit interrupt is disabled, enable it
            tx_restart_1 = 0;
            resp = FUNCTION_RETURN_OK;
            USART1->CR1 |= USART_FLAG_TXE; // enable TX interrupt
        }
    }

    return resp;
}


msg_t halUSART1_sndM (uint8_t *pDat, uint16_t dataSize)
{
    msg_t resp = FUNCTION_RETURN_ERROR;
    struct tbuf_1_ *p = &tbuf_1;
    uint16_t i;
    
    if ((0 != dataSize) && (USART1_TBUF_SIZE >= dataSize))
    { // что мы не поехавшие, зам. на деф
        if ((USART1_TBUF_SIZE - SIO_TBUFLEN_1) < dataSize) // 
        { return resp; }
        if (SIO_TBUFLEN_1 >= USART1_TBUF_SIZE) // If the buffer is full, return an error value
        { return resp; }
        for (i = 0; i < dataSize; i++) // Add data to the transmit buffer.
		{
            p->buf [p->in & (USART1_TBUF_SIZE - 1)] = pDat[i]; 
            p->in++;
        }
        if (0 != tx_restart_1) // If transmit interrupt is disabled, enable it    
		{              
            tx_restart_1 = 0;
            USART1->CR1 |= USART_FLAG_TXE; // enable TX interrupt
        }
        resp = FUNCTION_RETURN_OK;
    }

    return resp;
}


//------------------------------------------------------------------------------
// Прием байта/ов
//------------------------------------------------------------------------------
msg_t halUSART1_rcvS (uint8_t *pDat)
{
    msg_t resp = FUNCTION_RETURN_ERROR;
    struct rbuf_1_ *p = &rbuf_1;

    if (SIO_RBUFLEN_1 != 0)
    {
        *pDat =  (p->buf [(p->out) & (USART1_RBUF_SIZE - 1)]);
        p->out++;
        resp = FUNCTION_RETURN_OK;
    }
    
    return resp;
}


msg_t halUSART1_rcvM (uint8_t *pDat, uint16_t dataSize)
{
    msg_t resp = FUNCTION_RETURN_ERROR;
    struct rbuf_1_ *p = &rbuf_1;
    uint16_t i;

    if (0 < dataSize)
    {
        if ((0 < SIO_RBUFLEN_1) && (SIO_RBUFLEN_1 >= dataSize))
         {
            for (i = 0; i < dataSize; i++)
            {
                pDat[i] = (p->buf [(p->out) & (USART1_RBUF_SIZE - 1)]);
                p->out++;
            }
            resp = FUNCTION_RETURN_OK;
        }
    }
    
    return resp;
}


void halUSART1_setBaud (uint16_t baud)
{
    USART1->BRR = baud;
}

//#include <misc.h>
//#include <core_cm4.h>
void halUSART1_init (uint16_t baud)
{
    //NVIC_InitTypeDef NVIC_InitStructure; // this is used to configure the NVIC (nes
    USART_InitTypeDef  USART_InitStructure;
    GPIO_InitTypeDef   GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    
    GPIO_StructInit (&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9 | GPIO_Pin_10; // PA9-TX, PA10-RX  // PB6-TX, PB7-RX
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init (GPIOA, &GPIO_InitStructure);
    
    USART_DeInit(USART1);
    USART_StructInit (&USART_InitStructure);
    switch (baud)
    {
        case USART_BAUD_110: USART_InitStructure.USART_BaudRate = 110; break;
        case USART_BAUD_300: USART_InitStructure.USART_BaudRate = 300; break;
        case USART_BAUD_600: USART_InitStructure.USART_BaudRate = 600; break;
        case USART_BAUD_1200: USART_InitStructure.USART_BaudRate = 1200; break;
        case USART_BAUD_2400: USART_InitStructure.USART_BaudRate = 2400; break;
        case USART_BAUD_4800: USART_InitStructure.USART_BaudRate = 4800; break;
        case USART_BAUD_9600: USART_InitStructure.USART_BaudRate = 9600; break;
        case USART_BAUD_19200: USART_InitStructure.USART_BaudRate = 19200; break;
        case USART_BAUD_14400: USART_InitStructure.USART_BaudRate = 14400; break;
        case USART_BAUD_28800: USART_InitStructure.USART_BaudRate = 28800; break;
        case USART_BAUD_57600: USART_InitStructure.USART_BaudRate = 57600; break;
        case USART_BAUD_115200: USART_InitStructure.USART_BaudRate = 115200; break;
        case USART_BAUD_230400: USART_InitStructure.USART_BaudRate = 230400; break;
        case USART_BAUD_460800: USART_InitStructure.USART_BaudRate = 460800; break;
        case USART_BAUD_921600: USART_InitStructure.USART_BaudRate = 921600; break;
        default: USART_InitStructure.USART_BaudRate = 9600; break;
    }
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; 
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; 
	USART_Init (USART1, &USART_InitStructure);
    
//     GPIO_PinAFConfig (GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
//     GPIO_PinAFConfig (GPIOB, GPIO_PinSource7, GPIO_AF_USART1);
    GPIO_PinAFConfig (GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig (GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
    
    NVIC_EnableIRQ (USART1_IRQn);
    NVIC_SetPriority (USART1_IRQn, 6);
    
    USART_Cmd (USART1, ENABLE);

    // Флаг передачи
    USART1_SendBytes = 0;
    USART1_RecBytes = 0;
    USART1_errCount = 0;
	USART_ITConfig (USART1, USART_IT_RXNE, ENABLE);
    
    halUSART1_flush ();
        
    __enable_irq();
}


/**
 * Common interrupt  
 */
void USART1_IRQHandler (void)
{
    volatile uint16_t fStatus;
    struct tbuf_1_ *p_tx;
    struct rbuf_1_ *p_rx;
    
    fStatus = USART1->SR;
    if ((USART_SR_TXE & fStatus) != 0) // действительно ли прерывание вызвано окончанием передачи
    {
        USART1_LED_INV; // инвертируем индикацию
        USART1->SR &= ~USART_FLAG_TXE; // clear interrupt
        p_tx = &tbuf_1;
        if (p_tx->in != p_tx->out)
        {
            USART1->DR = (p_tx->buf [p_tx->out & (USART1_TBUF_SIZE - 1)] & 0x1FF);
            p_tx->out++;
            tx_restart_1 = 0;
            USART1_SendBytes++;
        } else {
            tx_restart_1 = 1;
            USART1->CR1 &= ~USART_FLAG_TXE; // disable TX interrupt if nothing to send
        }
    }
    if ((USART_SR_RXNE & fStatus) != 0)
    {
        USART1_LED_INV; // инвертируем индикацию
        USART1->SR &= ~USART_FLAG_RXNE;
        p_rx = &rbuf_1;
        if (((p_rx->in - p_rx->out) & ~(USART1_RBUF_SIZE - 1)) == 0)
        {
            p_rx->buf [p_rx->in & (USART1_RBUF_SIZE - 1)] = (USART1->DR & 0x1FF);
            ////
            halUSART2_sndS (p_rx->buf [p_rx->in & (USART1_RBUF_SIZE - 1)]);
            ////
            p_rx->in++;
            USART1_RecBytes++;
        } else { // Если буфер заполнен, то вырубаем прерывания во избежания спама
            
        }
    }
    
    if ((USART_SR_NE & fStatus)  //!<Noise Error Flag 
        || (USART_SR_FE & fStatus)  //!<Framing Error 
        || (USART_SR_PE & fStatus)  //!<Parity Error 
        || (USART_SR_ORE & fStatus)) // !<OverRun Error 
    {
    //USART_ClearITPendingBit(USARTX, USART_IT_ORE);
        USART1_errCount++;
    }
}

#endif
