#include "halSPI.h"
#include "defines.h"
#include "board.h"

#define HAL_SPI1 1
#ifdef HAL_SPI1
//#define USART3 USART3

#define TBUF3_SIZE   (8 / 2) /*** Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/
#define RBUF3_SIZE   (8 / 2) /*** Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/

#if TBUF3_SIZE < 2
    #error "TBUF3_SIZE is too small.  It must be larger than 1."
#elif ((TBUF3_SIZE & (TBUF3_SIZE - 1)) != 0)
    #error "TBUF3_SIZE must be a power of 2."
#endif

#if RBUF3_SIZE < 2
    #error "RBUF_SIZE_3 is too small.  It must be larger than 1."
#elif ((RBUF3_SIZE & (RBUF3_SIZE - 1)) != 0)
    #error "RBUF_SIZE_3 must be a power of 2."
#endif

struct buf_st_3 {
    uint32_t in; // Next In Index
    uint32_t out; // Next Out Index
    uint8_t buf [RBUF3_SIZE]; // Buffer
};

static struct buf_st_3 rbuf_3 = { 0, 0, };
#define SIO_RBUFLEN_3 ((uint16_t)(rbuf_3.in - rbuf_3.out)) 

static struct buf_st_3 tbuf_3 = { 0, 0, };
#define SIO_TBUFLEN_3 ((uint16_t)(tbuf_3.in - tbuf_3.out))

static uint32_t tx_restart_3 = 1; // NZ if TX restart is required


// static uint32_t USART3_RecBytes = 0;
// uint32_t    halSPI1_getRecBytes (void) {
//     return USART3_RecBytes;
// }

// static uint32_t USART3_SendBytes = 0;
// uint32_t    halSPI1_getSendBytes (void) {
//     return USART3_SendBytes;
// }

// static uint32_t USART3_errCount = 0; // просто счетчик ошибок
// uint32_t    halSPI1_getErrors (void) {
//     return USART3_errCount;
// }


void halSPI1_flush (void) {
    tbuf_3.in = 0; // Clear com buffer indexes
    tbuf_3.out = 0;
    tx_restart_3 = 1;

    rbuf_3.in = 0;
    rbuf_3.out = 0;
}

//------------------------------------------------------------------------------
// Прием байта/ов
//------------------------------------------------------------------------------
uint8_t halSPI1_sndS (uint8_t dat) {
    msg_t resp = FUNCTION_RETURN_ERROR;
    struct buf_st_3 *p = &tbuf_3;
    
    if (SIO_TBUFLEN_3 < TBUF3_SIZE) { // If the buffer is full, return an error value
        p->buf [p->in & (TBUF3_SIZE - 1)] = dat; // Add data to the transmit buffer.
        p->in++;
        if (tx_restart_3) { // If transmit interrupt is disabled, enable it
            tx_restart_3 = 0;
            resp = FUNCTION_RETURN_OK;
            USART3->CR1 |= USART_FLAG_TXE; // enable TX interrupt
        }
    }

    //GPIO_WriteBit(GPIOA, SS_PIN, Bit_RESET); // Select slave

	/* Wait until TXE and send byte */
	while (!SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)){};
	SPI_I2S_SendData(SPI1, dat);

	//GPIO_WriteBit(GPIOA, SS_PIN, Bit_SET); // Release slave
    
    return resp;
}


msg_t halSPI1_sndM (uint8_t *pDat, uint16_t dataSize) {
    msg_t resp = FUNCTION_RETURN_ERROR;
    struct buf_st_3 *p = &tbuf_3;
    uint32_t i;
    
    if ((0 < dataSize) && (TBUF3_SIZE >= dataSize)) { // что мы не поехавшие, зам. на деф
        if ((TBUF3_SIZE - SIO_TBUFLEN_3) < dataSize) // 
        { return resp; }
        if (SIO_TBUFLEN_3 >= TBUF3_SIZE) // If the buffer is full, return an error value
        { return resp; }
        for(i = 0; i < dataSize; i++) // Add data to the transmit buffer.
        {
            p->buf [p->in & (TBUF3_SIZE - 1)] = pDat[i]; 
            p->in++;
        }
        if (tx_restart_3) { // If transmit interrupt is disabled, enable it                  
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
msg_t halSPI1_rcvS (uint8_t *pDat) {
    msg_t resp = FUNCTION_RETURN_ERROR;
    struct buf_st_3 *p = &rbuf_3;

    if (SIO_RBUFLEN_3 != 0) {
        *pDat =  (p->buf [(p->out) & (RBUF3_SIZE - 1)]);
        p->out++;
        resp = FUNCTION_RETURN_OK;
    }
    
    return resp;
}


msg_t halSPI1_rcvM (uint8_t *pDat, uint16_t dataSize) {
    msg_t resp = FUNCTION_RETURN_ERROR;
    struct buf_st_3 *p = &rbuf_3;
    uint32_t i;

    if (0 < dataSize) {
        if ((0 < SIO_RBUFLEN_3) && (SIO_RBUFLEN_3 >= dataSize)) {
            for (i = 0; i < dataSize; i++) {
                pDat[i] = (p->buf [(p->out) & (RBUF3_SIZE - 1)]);
                p->out++;
            }
            resp = FUNCTION_RETURN_OK;
        }
    }
    
    return resp;
}


void halSPI1_setBaud (uint32_t spi_baud) {
    USART3->BRR = spi_baud;
}

//#include <misc.h>
//#include <core_cm4.h>
void halSPI1_init (void) {
    SPI_InitTypeDef SPI_InitStructure;
    GPIO_InitTypeDef   GPIO_InitStructure;
    
    halUSART3_flush ();
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    
    /* Outputs config for SPI1:
    * PA4 - NSS
    * PA5 - SCK
    * PA6 - MISO
    * PA7 - MOSI
    */
    GPIO_StructInit( &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10 | GPIO_Pin_11; // PB10-TX, PB11-RX
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    


    //USART_DeInit(USART3);
//     USART_StructInit(&USART_InitStructure);
//     USART_InitStructure.USART_BaudRate = 19200;	// @todo!!!
// 	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
// 	USART_InitStructure.USART_StopBits = USART_StopBits_2; // @todo!
// 	USART_InitStructure.USART_Parity = USART_Parity_No;
// 	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; 
// 	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; 
// 	USART_Init(USART3, &USART_InitStructure);



    /* Assert prescaler */
    assert_param(IS_SPI_BAUDRATE_PRESCALER(spi1_brp));

    /* RCC config */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);



//     /* Config GPIOA output for SS */
//     GPIO_init_str.GPIO_Pin = SS_PIN;
//     GPIO_init_str.GPIO_Mode = GPIO_Mode_Out_PP;
//     GPIO_Init(GPIOA, &GPIO_init_str);
//     GPIO_WriteBit(GPIOA,SS_PIN,Bit_SET); // Initially deselect slave

//     GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_SPI);
//     GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);


    /* SPI1 init str prepare */
    SPI_StructInit(&SPI_InitStructure);

    /* Config SPI1 */
    SPI_InitStructure.SPI_BaudRatePrescaler = 10000; // Config BRP externally
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_Init(SPI1, &SPI_InitStructure);

    /* Turn on SPI1 */
    SPI_Cmd(SPI1, ENABLE);
}

#endif

/**
 * Прерывание
 */
void halSPI1_IRQ (void) { // общее прерывание для передачи и приема
#ifdef HAL_SPI1
    volatile uint16_t fStatus;
    struct buf_st_3 *p;
    
    fStatus = USART3->SR;
    if ((USART_SR_TXE & fStatus) != 0) { // Проверяем, действительно ли прерывание вызвано окончанием передачи
        USART3_LED_INV; // инвертируем индикацию
        USART3->SR &= ~USART_FLAG_TXE; // clear interrupt
        p = &tbuf_3;
        if (p->in != p->out) {
            USART3->DR = (p->buf [p->out & (TBUF3_SIZE - 1)] & 0x1FF);
            p->out++;
            tx_restart_3 = 0;

        } else {
            tx_restart_3 = 1;
            USART3->CR1 &= ~USART_FLAG_TXE; // disable TX interrupt if nothing to send
        }
    }
    if ((USART_SR_RXNE & fStatus) != 0) {
        USART3_LED_INV; // инвертируем индикацию
        USART3->SR &= ~USART_FLAG_RXNE;
        p = &rbuf_3;
        if (((p->in - p->out) & ~(RBUF3_SIZE - 1)) == 0) {
            p->buf [p->in & (RBUF3_SIZE - 1)] = (USART3->DR & 0x1FF);
            p->in++;

        } else { // Если буфер заполнен, то вырубаем прерывания во избежания спама
            
        }
    }
    
//     if ((USART_SR_NE & fStatus)  /*!<Noise Error Flag */
//         || (USART_SR_FE & fStatus)  /*!<Framing Error */
//         || (USART_SR_PE & fStatus)  /*!<Parity Error */
//         || (USART_SR_ORE & fStatus)) { // /*!<OverRun Error */
//     //USART_ClearITPendingBit(USARTX, USART_IT_ORE);
//         USART3_errCount++;
//     }
#else
    while (1) {};
#endif
}
