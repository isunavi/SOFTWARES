#include "halUSART.h"
#include "board.h"

#if HAL_USART1



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
    uint16_t in; // Next In Index
    uint16_t out; // Next Out Index
    uint8_t buf [USART1_TBUF_SIZE]; // Buffer
} tbuf_1;

static struct rbuf_1_ {
    uint16_t in; // Next In Index
    uint16_t out; // Next Out Index
    uint8_t buf [USART1_RBUF_SIZE]; // Buffer
} rbuf_1;


//static struct buf_st_1 rbuf_1 = { 0, 0, };
#define SIO_RBUFLEN_1 ((uint16_t)(rbuf_1.in - rbuf_1.out)) 

//static struct buf_st_1 tbuf_1 = { 0, 0, };
#define SIO_TBUFLEN_1 ((uint16_t)(tbuf_1.in - tbuf_1.out))

static uint32_t tx_restart_1 = 1; // NZ if TX restart is required


static uint32_t USART1_RecBytes = 0;
uint32_t    halUSART1_getRecBytes (void) {
    return USART1_RecBytes;
}

static uint32_t USART1_SendBytes = 0;
uint32_t    halUSART1_getSendBytes (void) {
    return USART1_SendBytes;
}

static uint32_t USART1_errCount = 0; // просто счетчик ошибок
uint32_t    halUSART1_getErrors (void) {
    return USART1_errCount;
}


void halUSART1_flush (void) {
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
    
    if (SIO_TBUFLEN_1 < USART1_TBUF_SIZE) { // If the buffer is full, return an error value
        p->buf [p->in & (USART1_TBUF_SIZE - 1)] = dat; // Add data to the transmit buffer.
        p->in++;
        if (tx_restart_1) { // If transmit interrupt is disabled, enable it
            tx_restart_1 = 0;
            resp = FUNCTION_RETURN_OK;
            UART1->CR2 |= UART1_CR2_TCIEN; // enable TX interrupt
        }
    }

    return resp;
}


msg_t halUSART1_sndM (uint8_t *pDat, uint16_t dataSize)
{
    msg_t resp = FUNCTION_RETURN_ERROR;
    struct tbuf_1_ *p = &tbuf_1;
    uint16_t i;
    
    if ((0 < dataSize) && (USART1_TBUF_SIZE >= dataSize)) { // что мы не поехавшие, зам. на деф
        if ((USART1_TBUF_SIZE - SIO_TBUFLEN_1) < dataSize) // 
        { return resp; }
        if (SIO_TBUFLEN_1 >= USART1_TBUF_SIZE) // If the buffer is full, return an error value
        { return resp; }
        for (i = 0; i < dataSize; i++) { // Add data to the transmit buffer.
            p->buf [p->in & (USART1_TBUF_SIZE - 1)] = pDat[i]; 
            p->in++;
        }
        if (0 != tx_restart_1) { // If transmit interrupt is disabled, enable it                  
            tx_restart_1 = 0;
            UART1->CR2 |= UART1_CR2_TCIEN; // enable TX interrupt
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

    if (SIO_RBUFLEN_1 != 0) {
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

    if (0 < dataSize) {
        if ((0 < SIO_RBUFLEN_1) && (SIO_RBUFLEN_1 >= dataSize)) {
            for (i = 0; i < dataSize; i++) {
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
    switch (baud)
    {
        //case USART_BAUD_110: baud = (16000000UL / 110UL); break;
        //case USART_BAUD_300: baud = (16000000UL / 300UL); break;
        //case USART_BAUD_600: baud = (16000000UL / 600UL); break;
        //case USART_BAUD_1200: baud = (16000000UL / 1200UL); break;
        case USART_BAUD_2400: baud = (CPU_FRQ / 2400UL); break;
        case USART_BAUD_4800: baud = (CPU_FRQ / 4800UL); break;
        case USART_BAUD_9600: baud = (CPU_FRQ / 9600UL); break;
        case USART_BAUD_19200: baud = (CPU_FRQ / 19200UL); break;
        case USART_BAUD_14400: baud = (CPU_FRQ / 14400UL); break;
        case USART_BAUD_28800: baud = (CPU_FRQ / 28800UL); break;
        case USART_BAUD_57600: baud = (CPU_FRQ / 57600UL); break;
        case USART_BAUD_115200: baud = (CPU_FRQ / 115200UL); break;
        //case USART_BAUD_230400: baud = (16000000UL / 230400UL); break;
        //case USART_BAUD_460800: baud = (16000000UL / 460800UL); break;
        //case USART_BAUD_921600: baud = (16000000UL / 921600UL); break;
        default: baud = (CPU_FRQ / 9600UL); break;
    }
    
    //UART1->BRR2 = (unsigned char)(((baud  >> 8) & 0xF0) | (baud  & 0x0F));
    //UART1->BRR1 = (unsigned char)(baud);
    UART1->BRR2 = baud & 0x000F;
    UART1->BRR2 |= baud >> 12;
    UART1->BRR1 = (baud >> 4) & 0x00FF;
}


void halUSART1_init (uint16_t baud)
{
//#ifdef ENABLE_BUFFER
    halUSART1_flush ();
//#endif
    //Значение регистра BRR
    
    /**
    GPIO_Init (GPIOD, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init (GPIOD, GPIO_PIN_6, GPIO_MODE_IN_PU_NO_IT);
    */
    
    //Настраиваем TX на выход, а RX на вход
    GPIOD->DDR |= GPIO_PIN_5; // //TX
    //GPIOD->CR1 |= GPIO_PIN_5;
    
    GPIOD->DDR &= ~GPIO_PIN_6; //RX
    GPIOD->CR1 &= ~GPIO_PIN_6; //RX - плавающий вход
    GPIOD->CR2 &= ~GPIO_PIN_6; //Отключает внешние прерывания для RX
    
    //Настройка скорости передачи
    // http://eugenemcu.ru/publ/11-1-0-69
    //brr = BAUD_19200; //F_MASTER / UART_BAUD_RATE;
    //brr = ((16000000UL / 1UL) / 19200UL );

    halUSART1_setBaud (baud);
    /*
    UART1->BRR2 = brr & 0x000F;
    UART1->BRR2 |= brr >> 12;
    UART1->BRR1 = (brr >> 4) & 0x00FF;
    */
    //Четность отключена
    UART1->CR1 &= ~UART1_CR1_PIEN;
    //Контроль четности отключен
    UART1->CR1 &= ~UART1_CR1_PCEN;
    //8-битный режим
    UART1->CR1 &= ~UART1_CR1_M;
    //Включить UART
    UART1->CR1 &= ~UART1_CR1_UARTD;
    
    //Запретить прерывание по опустошению передающ. регистра
    UART1->CR2 &= ~UART1_CR2_TIEN;
    //Запретить прерывание по завершению передачи
    UART1->CR2 &= ~UART1_CR2_TCIEN;
    
//#ifdef UART_INT_ENABLE  
    UART1->CR2 |= UART1_CR2_RIEN;
//#else
//    UART1->CR2 &= ~UART1_CR2_RIEN; //Запретить прерывание по заполнению приемного регистра
//#endif
  
    //Запретить прерывание по освобождению линии
    UART1->CR2 &= ~UART1_CR2_ILIEN;
    //Передатчик включить
    UART1->CR2 |= UART1_CR2_TEN;
    //Приемник включить
    UART1->CR2 |= UART1_CR2_REN;
    //Не посылать break-символ
    UART1->CR2 &= ~UART1_CR2_SBK;
    //Один стоп-бит
    UART1->CR3 &= ~UART1_CR3_STOP;

}


void halUART1_RX_IRQ (void) // прерывание для приема
{
    volatile uint8_t fStatus;
    struct rbuf_1_ *p;
    
    fStatus = UART1->SR;
    if ((UART1_SR_RXNE & fStatus) != 0)
    {
        USART1_LED_RX_INV; // инвертируем индикацию
        UART1->SR &= ~UART1_SR_RXNE;
        p = &rbuf_1;
        if (((p->in - p->out) & ~(USART1_RBUF_SIZE - 1)) == 0)
        {
            p->buf [p->in & (USART1_RBUF_SIZE - 1)] = (UART1->DR);
            p->in++;
            USART1_RecBytes++;
        }
        else
        { // Если буфер заполнен, то вырубаем прерывания во избежания спама
            
        }
    }
    
    if ((UART1_SR_NF & fStatus)  //<Noise Error Flag
        || (UART1_SR_FE & fStatus)  //<Framing Error
        || (UART1_SR_PE & fStatus)  //<Parity Error
        || (UART1_SR_OR & fStatus)) // <OverRun Error
    {
        //USART_ClearITPendingBit(USARTX, USART_IT_ORE);
        USART1_errCount++;
    }
    
}


void halUART1_TX_IRQ (void)
{ // прерывание для передачи
    volatile uint8_t fStatus;
    struct tbuf_1_ *p;
    
    fStatus = UART1->SR;
    if ((UART1_SR_TXE & fStatus) != 0)
    { // Проверяем, действительно ли прерывание вызвано окончанием передачи
        USART1_LED_TX_INV; // инвертируем индикацию
        UART1->SR &= ~UART1_SR_TXE; // clear interrupt
        p = &tbuf_1;
        if (p->in != p->out)
        {
            UART1->DR = p->buf [p->out & (USART1_TBUF_SIZE - 1)];
            p->out++;
            tx_restart_1 = 0;
            USART1_SendBytes++;
        }
        else
        {
            tx_restart_1 = 1;
            UART1->CR2 &= ~UART1_CR2_TCIEN; // disable TX interrupt if nothing to send
        }
    }
    if ((UART1_SR_NF & fStatus)  //<Noise Error Flag
        || (UART1_SR_FE & fStatus)  //<Framing Error
        || (UART1_SR_PE & fStatus)  //<Parity Error
        || (UART1_SR_OR & fStatus)) // <OverRun Error
    {
        //USART_ClearITPendingBit(USARTX, USART_IT_ORE);
        USART1_errCount++;
    }
}


#endif //HAL_USART1
