#include "halUSART.h"
#include "defines.h"
#include "board.h"


#ifdef HAL_USART1

#define TBUF1_SIZE   (USART1_BUF_SIZE / 2) /*** Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/
#define RBUF1_SIZE   (USART1_BUF_SIZE / 2) /*** Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/

#if TBUF1_SIZE < 2
    #error "TBUF1_SIZE is too small.  It must be larger than 1."
#elif ((TBUF1_SIZE & (TBUF1_SIZE - 1)) != 0)
    #error "TBUF1_SIZE must be a power of 2."
#endif

#if RBUF1_SIZE < 2
    #error "RBUF_SIZE_1 is too small.  It must be larger than 1."
#elif ((RBUF1_SIZE & (RBUF1_SIZE - 1)) != 0)
    #error "RBUF_SIZE_1 must be a power of 2."
#endif

struct buf_st_1 {
    uint8_t in; // Next In Index
    uint8_t out; // Next Out Index
    uint8_t buf [RBUF1_SIZE]; // Buffer
};

static struct buf_st_1 rbuf_1 = { 0, 0, };
#define SIO_RBUFLEN_1 ((uint8_t)(rbuf_1.in - rbuf_1.out)) 

static struct buf_st_1 tbuf_1 = { 0, 0, };
#define SIO_TBUFLEN_1 ((uint8_t)(tbuf_1.in - tbuf_1.out))

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
    struct buf_st_1 *p = &tbuf_1;
    
    if (SIO_TBUFLEN_1 < TBUF1_SIZE)
    { // If the buffer is full, return an error value
        p->buf [p->in & (TBUF1_SIZE - 1)] = dat; // Add data to the transmit buffer.
        p->in++;
        if (tx_restart_1)
        { // If transmit interrupt is disabled, enable it
            tx_restart_1 = 0;
            resp = FUNCTION_RETURN_OK;
            UCSRB |= (1 << UDRIE); // enable TX interrupt
        }
    }

    return resp;
}


msg_t halUSART1_sndM (uint8_t *pDat, uint16_t dataSize)
{
    msg_t resp = FUNCTION_RETURN_ERROR;
    struct buf_st_1 *p = &tbuf_1;
    uint16_t i;
    
    if ((0 < dataSize) && (TBUF1_SIZE >= dataSize))
    {
        if ((TBUF1_SIZE - SIO_TBUFLEN_1) < dataSize) // 
        { return resp; }
        if (SIO_TBUFLEN_1 >= TBUF1_SIZE) // If the buffer is full, return an error value
        { return resp; }
        for (i = 0; i < dataSize; i++)
        { // Add data to the transmit buffer.
            p->buf [p->in & (TBUF1_SIZE - 1)] = pDat[i]; 
            p->in++;
        }
        if (0 != tx_restart_1)
        { // If transmit interrupt is disabled, enable it                  
            tx_restart_1 = 0;
            UCSRB |= (1 << UDRIE); // enable TX interrupt
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
    struct buf_st_1 *p = &rbuf_1;

    if (SIO_RBUFLEN_1 != 0)
    {
        *pDat = (p->buf [(p->out) & (RBUF1_SIZE - 1)]);
        p->out++;
        resp = FUNCTION_RETURN_OK;
    }
    
    return resp;
}


msg_t halUSART1_rcvM (uint8_t *pDat, uint16_t dataSize)
{
    msg_t resp = FUNCTION_RETURN_ERROR;
    struct buf_st_1 *p = &rbuf_1;
    uint16_t i;

    if (0 < dataSize)
    {
        if ((0 < SIO_RBUFLEN_1) && (SIO_RBUFLEN_1 >= dataSize))
        {
            for (i = 0; i < dataSize; i++)
            {
                pDat[i] = (p->buf [(p->out) & (RBUF1_SIZE - 1)]);
                p->out++;
            }
            resp = FUNCTION_RETURN_OK;
        }
    }
    
    return resp;
}


void halUSART1_setBaud (uint16_t baud)
{
    UBRRH = (uint8_t)(baud >> 8);
    UBRRL = (uint8_t)baud;
}


void halUSART1_init (uint16_t baud)
{
    UCSRA = 0; //off & clear all
    UCSRB = 0;
    UCSRC = 0;

    halUSART1_setBaud (baud);
    
     // On reciver, transmitter, enable recieve complete interrupt, enable the trancive complete interrupt
    UCSRB = (1 << TXEN) | (1 << RXEN) | (1 << RXCIE);// | (1 << TXCIE);
 
    //UCSRC = 0x86;
    /* Set frame format: 8data, 2stop bit */
    UCSRC = (1 << URSEL) | (1 << USBS) | (3 << UCSZ0);
    //UCSRC != ~(1 << URSEL); //for acces to UBRRH

    //__EI();
}


/**
 * Прерывания
 */
ISR (USART_RXC_vect)
{
    struct buf_st_1 *p;
    
    /*
	if (UCSRA & ((1 << FE) | (1 << DOR) | (1 << PE)))  //!<framing, data overrun, parity error
    {
        USART1_errCount++;
    } 
    else
	*/
    {
        USART1_LED_INV; // инвертируем индикацию
        p = &rbuf_1;
        if (((p->in - p->out) & ~(RBUF1_SIZE - 1)) == 0)
        {
            p->buf [p->in & (RBUF1_SIZE - 1)] = UDR;
            p->in++;
            USART1_RecBytes++;
        }
        else
        { // Если буфер заполнен, то вырубаем прерывания во избежания спама
                
        }
    }
}


ISR (USART_UDRE_vect)
{
    struct buf_st_1 *p;
    
    USART1_LED_INV; // инвертируем индикацию
    p = &tbuf_1;
    if (p->in != p->out)
    {
        UDR = (p->buf [p->out & (TBUF1_SIZE - 1)] & 0x1FF);
        p->out++;
        tx_restart_1 = 0;
        USART1_SendBytes++;
    }
    else
    {
        tx_restart_1 = 1;
        UCSRB &= ~(1 << UDRIE); // disable TX interrupt if nothing to send
    }  
}


#endif
