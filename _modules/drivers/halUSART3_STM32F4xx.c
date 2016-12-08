#include "halUSART.h"
#include "defines.h"
#include "board.h"


#ifdef HAL_USART3

//#define TBUF3_SIZE   (USART3_TBUF_SIZE) /*** Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/
//#define RBUF3_SIZE   (USART3_RBUF_SIZE) /*** Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/

#if USART3_TBUF_SIZE < 2
    #error "USART3_TBUF_SIZE is too small.  It must be larger than 1."
#elif ((USART3_TBUF_SIZE & (USART3_TBUF_SIZE - 1)) != 0)
    #error "USART3_TBUF_SIZE must be a power of 2."
#endif

#if USART3_RBUF_SIZE < 2
    #error "USART3_RBUF_SIZE is too small.  It must be larger than 1."
#elif ((USART3_RBUF_SIZE & (USART3_RBUF_SIZE - 1)) != 0)
    #error "USART3_RBUF_SIZE must be a power of 2."
#endif

static struct tbuf_3_ {
    uint32_t in; // Next In Index
    uint32_t out; // Next Out Index
    uint8_t buf [USART3_TBUF_SIZE]; // Buffer
} tbuf_3;

static struct rbuf_3_ {
    uint32_t in; // Next In Index
    uint32_t out; // Next Out Index
    uint8_t buf [USART3_RBUF_SIZE]; // Buffer
} rbuf_3;


//static struct buf_st_3 rbuf_3 = { 0, 0, };
#define SIO_RBUFLEN_3 ((uint16_t)(rbuf_3.in - rbuf_3.out)) 

//static struct buf_st_3 tbuf_3 = { 0, 0, };
#define SIO_TBUFLEN_3 ((uint16_t)(tbuf_3.in - tbuf_3.out))

static uint32_t tx_restart_3 = 1; // NZ if TX restart is required


static uint32_t USART3_RecBytes = 0;
uint32_t    halUSART3_getRecBytes (void) {
    return USART3_RecBytes;
}

static uint32_t USART3_SendBytes = 0;
uint32_t    halUSART3_getSendBytes (void) {
    return USART3_SendBytes;
}

static uint32_t USART3_errCount = 0; // просто счетчик ошибок
uint32_t    halUSART3_getErrors (void) {
    return USART3_errCount;
}


void halUSART3_flush (void)
{
    tbuf_3.in = 0; // Clear com buffer indexes
    tbuf_3.out = 0;
    tx_restart_3 = 1;

    rbuf_3.in = 0;
    rbuf_3.out = 0;
}

//------------------------------------------------------------------------------
// Отправка байта/ов
//------------------------------------------------------------------------------
msg_t halUSART3_sndS (uint8_t dat) {
    msg_t resp = FUNCTION_RETURN_ERROR;
    struct tbuf_3_ *p = &tbuf_3;
    
    if (SIO_TBUFLEN_3 < USART3_TBUF_SIZE) { // If the buffer is full, return an error value
        p->buf [p->in & (USART3_TBUF_SIZE - 1)] = dat; // Add data to the transmit buffer.
        p->in++;
        if (tx_restart_3) { // If transmit interrupt is disabled, enable it
            tx_restart_3 = 0;
            resp = FUNCTION_RETURN_OK;
            USART3->CR1 |= USART_FLAG_TXE; // enable TX interrupt
        }
    }

    return resp;
}


msg_t halUSART3_sndM (uint8_t *pDat, uint16_t dataSize) {
    msg_t resp = FUNCTION_RETURN_ERROR;
    struct tbuf_3_ *p = &tbuf_3;
    uint16_t i;
    
    if ((0 != dataSize) && (USART3_TBUF_SIZE >= dataSize))
    { // что мы не поехавшие, зам. на деф
        if ((USART3_TBUF_SIZE - SIO_TBUFLEN_3) < dataSize) // 
        { return resp; }
        if (SIO_TBUFLEN_3 >= USART3_TBUF_SIZE) // If the buffer is full, return an error value
        { return resp; }
        for (i = 0; i < dataSize; i++) { // Add data to the transmit buffer.
            p->buf [p->in & (USART3_TBUF_SIZE - 1)] = pDat[i]; 
            p->in++;
        }
        if (0 != tx_restart_3) { // If transmit interrupt is disabled, enable it                  
            tx_restart_3 = 0;
            USART3->CR1 |= USART_FLAG_TXE; // enable TX interrupt
        }
        resp = FUNCTION_RETURN_OK;
    }

    return resp;
}


//------------------------------------------------------------------------------
// Прием байта/ов
//------------------------------------------------------------------------------
msg_t halUSART3_rcvS (uint8_t *pDat) {
    msg_t resp = FUNCTION_RETURN_ERROR;
    struct rbuf_3_ *p = &rbuf_3;

    if (SIO_RBUFLEN_3 != 0) {
        *pDat =  (p->buf [(p->out) & (USART3_RBUF_SIZE - 1)]);
        p->out++;
        resp = FUNCTION_RETURN_OK;
    }
    
    return resp;
}


msg_t halUSART3_rcvM (uint8_t *pDat, uint16_t dataSize) {
    msg_t resp = FUNCTION_RETURN_ERROR;
    struct rbuf_3_ *p = &rbuf_3;
    uint16_t i;

    if (0 < dataSize) {
        if ((0 < SIO_RBUFLEN_3) && (SIO_RBUFLEN_3 >= dataSize)) {
            for (i = 0; i < dataSize; i++) {
                pDat[i] = (p->buf [(p->out) & (USART3_RBUF_SIZE - 1)]);
                p->out++;
            }
            resp = FUNCTION_RETURN_OK;
        }
    }
    
    return resp;
}


void halUSART3_setBaud (uint16_t baud)
{
    USART3->BRR = baud;
}

//#include <misc.h>
//#include <core_cm4.h>
void halUSART3_init (uint16_t baud)
{
    //NVIC_InitTypeDef NVIC_InitStructure; // this is used to configure the NVIC (nes
    USART_InitTypeDef  USART_InitStructure;
    GPIO_InitTypeDef   GPIO_InitStructure;
    
    halUSART3_flush ();
    
    RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd (RCC_APB1Periph_USART3, ENABLE);
    
    GPIO_StructInit (&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10 | GPIO_Pin_11; // PB10-TX, PB11-RX
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD; // GPIO_OType_OD for 1Wire @todo
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init (GPIOB, &GPIO_InitStructure);
    
    //USART_DeInit(USART3);
    USART_StructInit(&USART_InitStructure);
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
	USART_Init (USART3, &USART_InitStructure);
    
    GPIO_PinAFConfig (GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig (GPIOB, GPIO_PinSource11, GPIO_AF_USART3);
    
    NVIC_EnableIRQ (USART3_IRQn);
    NVIC_SetPriority (USART3_IRQn, 6);
    
    USART_Cmd (USART3, ENABLE);
    
    USART3_SendBytes = 0;
    USART3_RecBytes = 0;
    USART3_errCount = 0;
	USART_ITConfig (USART3, USART_IT_RXNE, ENABLE);
    __enable_irq();
}


/**
 * Прерывание
 */
void USART3_IRQHandler (void) { // общее прерывание для передачи и приема
    volatile uint16_t fStatus;
    struct tbuf_3_ *p_tx;
    struct rbuf_3_ *p_rx;
    
    fStatus = USART3->SR;
    if ((USART_SR_TXE & fStatus) != 0) { // Проверяем, действительно ли прерывание вызвано окончанием передачи
        USART3_LED_INV; // инвертируем индикацию
        USART3->SR &= ~USART_FLAG_TXE; // clear interrupt
        p_tx = &tbuf_3;
        if (p_tx->in != p_tx->out) {
            USART3->DR = (p_tx->buf [p_tx->out & (USART3_TBUF_SIZE - 1)]);
            p_tx->out++;
            tx_restart_3 = 0;
            USART3_SendBytes++;
        } else {
            tx_restart_3 = 1;
            USART3->CR1 &= ~USART_FLAG_TXE; // disable TX interrupt if nothing to send
        }
    }
    if ((USART_SR_RXNE & fStatus) != 0) {
        USART3_LED_INV; // инвертируем индикацию
        USART3->SR &= ~USART_FLAG_RXNE;
        p_rx = &rbuf_3;
        if (((p_rx->in - p_rx->out) & ~(USART3_RBUF_SIZE - 1)) == 0) {
            p_rx->buf [p_rx->in & (USART3_RBUF_SIZE - 1)] = USART3->DR;
            p_rx->in++;
            USART3_RecBytes++;
        } else { // Если буфер заполнен, то вырубаем прерывания во избежания спама
            
        }
    }
    
    if ((USART_SR_NE & fStatus)  /*!<Noise Error Flag */
        || (USART_SR_FE & fStatus)  /*!<Framing Error */
        || (USART_SR_PE & fStatus)  /*!<Parity Error */
        || (USART_SR_ORE & fStatus)) { // /*!<OverRun Error */
    //USART_ClearITPendingBit(USARTX, USART_IT_ORE);
        USART3_errCount++;
    }
}

#endif
