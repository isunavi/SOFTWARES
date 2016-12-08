#include "halUSART.h"
#include "defines.h"
#include "board.h"


#ifdef HAL_USART2

#define TBUF2_SIZE   (USART2_TBUF_SIZE) /*** Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/
#define RBUF2_SIZE   (USART2_RBUF_SIZE) /*** Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/

#if TBUF2_SIZE < 2
    #error "TBUF2_SIZE is too small.  It must be larger than 1."
#elif ((TBUF2_SIZE & (TBUF2_SIZE - 1)) != 0)
    #error "TBUF2_SIZE must be a power of 2."
#endif

#if RBUF2_SIZE < 2
    #error "RBUF_SIZE_2 is too small.  It must be larger than 1."
#elif ((RBUF2_SIZE & (RBUF2_SIZE - 1)) != 0)
    #error "RBUF_SIZE_2 must be a power of 2."
#endif

struct buf_st_2 {
    uint32_t in; // Next In Index
    uint32_t out; // Next Out Index
    uint8_t buf [RBUF2_SIZE]; // Buffer
};

static struct buf_st_2 rbuf_2 = { 0, 0, };
#define SIO_RBUFLEN_2 ((uint16_t)(rbuf_2.in - rbuf_2.out)) 

static struct buf_st_2 tbuf_2 = { 0, 0, };
#define SIO_TBUFLEN_2 ((uint16_t)(tbuf_2.in - tbuf_2.out))

static uint32_t tx_restart_2 = 1; // NZ if TX restart is required


static uint32_t USART2_RecBytes = 0;
uint32_t    halUSART2_getRecBytes (void) {
    return USART2_RecBytes;
}

static uint32_t USART2_SendBytes = 0;
uint32_t    halUSART2_getSendBytes (void) {
    return USART2_SendBytes;
}

static uint32_t USART2_errCount = 0; // просто счетчик ошибок
uint32_t    halUSART2_getErrors (void) {
    return USART2_errCount;
}


void halUSART2_flush (void) {
    tbuf_2.in = 0; // Clear com buffer indexes
    tbuf_2.out = 0;
    tx_restart_2 = 1;

    rbuf_2.in = 0;
    rbuf_2.out = 0;
}

//------------------------------------------------------------------------------
// Отправка байта/ов
//------------------------------------------------------------------------------
msg_t halUSART2_sndS (uint8_t dat) {
    msg_t resp = FUNCTION_RETURN_ERROR;
    struct buf_st_2 *p = &tbuf_2;
    
    if (SIO_TBUFLEN_2 < TBUF2_SIZE) { // If the buffer is full, return an error value
        p->buf [p->in & (TBUF2_SIZE - 1)] = dat; // Add data to the transmit buffer.
        p->in++;
        if (tx_restart_2) { // If transmit interrupt is disabled, enable it
            tx_restart_2 = 0;
            resp = FUNCTION_RETURN_OK;
            USART2->CR1 |= USART_FLAG_TXE; // enable TX interrupt
        }
    }

    return resp;
}


msg_t halUSART2_sndM (uint8_t *pDat, uint16_t dataSize) {
    msg_t resp = FUNCTION_RETURN_ERROR;
    struct buf_st_2 *p = &tbuf_2;
    uint16_t i;
    
    if ((0 < dataSize) && (TBUF2_SIZE >= dataSize)) { // что мы не поехавшие, зам. на деф
        if ((TBUF2_SIZE - SIO_TBUFLEN_2) < dataSize) // 
        { return resp; }
        if (SIO_TBUFLEN_2 >= TBUF2_SIZE) // If the buffer is full, return an error value
        { return resp; }
        for (i = 0; i < dataSize; i++) { // Add data to the transmit buffer.
            p->buf [p->in & (TBUF2_SIZE - 1)] = pDat[i]; 
            p->in++;
        }
        if (0 != tx_restart_2) { // If transmit interrupt is disabled, enable it                  
            tx_restart_2 = 0;
            USART2->CR1 |= USART_FLAG_TXE; // enable TX interrupt
        }
        resp = FUNCTION_RETURN_OK;
    }

    return resp;
}


//------------------------------------------------------------------------------
// Прием байта/ов
//------------------------------------------------------------------------------
msg_t halUSART2_rcvS (uint8_t *pDat) {
    msg_t resp = FUNCTION_RETURN_ERROR;
    struct buf_st_2 *p = &rbuf_2;

    if (SIO_RBUFLEN_2 != 0) {
        *pDat =  (p->buf [(p->out) & (RBUF2_SIZE - 1)]);
        p->out++;
        resp = FUNCTION_RETURN_OK;
    }
    
    return resp;
}


msg_t halUSART2_rcvM (uint8_t *pDat, uint16_t dataSize) {
    msg_t resp = FUNCTION_RETURN_ERROR;
    struct buf_st_2 *p = &rbuf_2;
    uint16_t i;

    if (0 < dataSize) {
        if ((0 < SIO_RBUFLEN_2) && (SIO_RBUFLEN_2 >= dataSize)) {
            for (i = 0; i < dataSize; i++) {
                pDat[i] = (p->buf [(p->out) & (RBUF2_SIZE - 1)]);
                p->out++;
            }
            resp = FUNCTION_RETURN_OK;
        }
    }
    
    return resp;
}


void halUSART2_setBaud (uint16_t baud)
{
    //uint32_t temp, fck;
    USART2->BRR = baud;
    
//     fck = 48000000;
//     temp = fck + (baud >> 1);
//     temp /= baud;
//     USART2->BRR = temp;
     
}


void halUSART2_init (uint16_t baud)
{
    //NVIC_InitTypeDef NVIC_InitStructure; // this is used to configure the NVIC (nes
    USART_InitTypeDef  USART_InitStructure;
    GPIO_InitTypeDef   GPIO_InitStructure;
    
    halUSART2_flush ();
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    
    GPIO_StructInit (&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2 | GPIO_Pin_3; // PA2-TX, PA3-RX
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init (GPIOA, &GPIO_InitStructure);
    
    //USART_DeInit(USART3);
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
	USART_Init (USART2, &USART_InitStructure);
    
    GPIO_PinAFConfig (GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig (GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
    
    NVIC_EnableIRQ (USART2_IRQn);
    NVIC_SetPriority (USART2_IRQn, 6);
    
    USART_Cmd (USART2, ENABLE);
    USART2_SendBytes = 0;
    USART2_RecBytes = 0;
    USART2_errCount = 0;
	USART_ITConfig (USART2, USART_IT_RXNE, ENABLE);
    __enable_irq();
}


/**
 * Прерывание
 */
void USART2_IRQHandler (void)
{ // общее прерывание для передачи и приема

    volatile uint16_t fStatus;
    struct buf_st_2 *p;
    
    fStatus = USART2->SR;
    if ((USART_SR_TXE & fStatus) != 0) { // Проверяем, действительно ли прерывание вызвано окончанием передачи
        USART2_LED_INV; // инвертируем индикацию
        USART2->SR &= ~USART_FLAG_TXE; // clear interrupt
        p = &tbuf_2;
        if (p->in != p->out) {
            USART2->DR = (p->buf [p->out & (TBUF2_SIZE - 1)] & 0x1FF);
            p->out++;
            tx_restart_2 = 0;
            USART2_SendBytes++;
        } else {
            tx_restart_2 = 1;
            USART2->CR1 &= ~USART_FLAG_TXE; // disable TX interrupt if nothing to send
        }
    }
    if ((USART_SR_RXNE & fStatus) != 0) {
        USART2_LED_INV; // инвертируем индикацию
        USART2->SR &= ~USART_FLAG_RXNE;
        p = &rbuf_2;
        if (((p->in - p->out) & ~(RBUF2_SIZE - 1)) == 0)
        {
            p->buf [p->in & (RBUF2_SIZE - 1)] = (USART2->DR & 0x1FF);
            p->in++;
            USART2_RecBytes++;
        } else { // Если буфер заполнен, то вырубаем прерывания во избежания спама
            
        }
    }
    
    if ((USART_SR_NE & fStatus)  /*!<Noise Error Flag */
        || (USART_SR_FE & fStatus)  /*!<Framing Error */
        || (USART_SR_PE & fStatus)  /*!<Parity Error */
        || (USART_SR_ORE & fStatus)) { // /*!<OverRun Error */
    //USART_ClearITPendingBit(USARTX, USART_IT_ORE);
        USART2_errCount++;
    }
}

#endif
