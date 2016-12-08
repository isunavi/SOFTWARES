#include "halSPI.h"
#include "defines.h"
#include "board.h"

#include "debug.h"

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

 


void halSPI1_flush (void)
{

}

//#define HAL_SPI1__  1

//volatile uint32_t spi_a, spi_b, spi_c;
//------------------------------------------------------------------------------
// Прием байта/ов
//------------------------------------------------------------------------------
uint8_t halSPI1_xput (uint8_t data)
{
#if (BOARD_STM32EV103)
#if HAL_SPI1__
    uint8_t i, res = 0;
    
    for (i = 0; i < 8; i++)
    {
        if (data & 0x80)//Выставить бит данных
        {
            PIN_NRF24L01P_MOSI_H;
        } else {
            PIN_NRF24L01P_MOSI_L;
        }
        data = data << 1;
        PIN_NRF24L01P_SCK_H;//Импульс
        res = res << 1;
        _delay_us (10);
        if(0 != PIN_NRF24L01P_MISO_I) 
        {
            res = res | 0x01; 
        }
        PIN_NRF24L01P_SCK_L;
        _delay_us (10);
    }
    return res;
#else

    SPI1->DR = data;
    while (0 != (SPI1->SR & SPI_I2S_FLAG_BSY)) {}; // All data transmitted/received but SPI may be busy so wait until done.
    while (0 == (SPI1->SR & SPI_I2S_FLAG_RXNE)) {}; // Wait for any data on MISO pin to be received.
    while (0 == (SPI1->SR & SPI_I2S_FLAG_TXE)) {}; //{spi_a++;}; // Wait until the data has been transmitted.
            //_delay_us (100);
    data = (uint8_t)(SPI1->DR);
    //_delay_us (30);
    return data;
#endif //HAL_SPI1__
        
#else
    //spi_a = 0;
    //spi_b = 0;
    //spi_c = 0;
    SPI1->DR = data;
    
    while (0 == (SPI1->SR & SPI_I2S_FLAG_TXE)) {spi_a++;}; // Wait until the data has been transmitted.

    while (0 != (SPI1->SR & SPI_I2S_FLAG_BSY)) {}; // All data transmitted/received but SPI may be busy so wait until done.

    while (0 == (SPI1->SR & SPI_I2S_FLAG_RXNE)) {spi_b++;}; // Wait for any data on MISO pin to be received.

    //_delay_us (100);
    return (uint8_t)(SPI1->DR);
#endif
}


// msg_t halSPI1_sndM (uint8_t *pDat, uint16_t dataSize)
// {
//     msg_t resp = FUNCTION_RETURN_ERROR;
//     uint32_t i;
//     
//     if (0 < dataSize)
//     { // что мы не поехавшие, зам. на деф
//         for(i = 0; i < dataSize; i++) // Add data to the transmit buffer.
//         {
//             halSPI1_xput (*pDat++);
//         }
//         resp = FUNCTION_RETURN_OK;
//     }

//     return resp;
// }



// msg_t halSPI1_rcvM (uint8_t *pDat, uint16_t dataSize)
// {
//     msg_t resp = FUNCTION_RETURN_ERROR;
//     uint32_t i;

//     if (0 < dataSize)
//     {
//         for (i = 0; i < dataSize; i++)
//         {
//             halSPI1_rcvS (&pDat[i]);
//         }
//         resp = FUNCTION_RETURN_OK;
//     }
//     
//     return resp;
//}



void halSPI1_init (uint8_t baud)
{
#if (BOARD_STM32EV103)
    RCC->APB2ENR |= (RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_SPI1EN);
    //remap SPI
    AFIO->MAPR |= (AFIO_MAPR_SPI1_REMAP | AFIO_MAPR_SWJ_CFG_JTAGDISABLE);
    // Отречемся от старого мира
    RCC->APB2RSTR |= RCC_APB2Periph_SPI1;
    RCC->APB2RSTR &= ~RCC_APB2Periph_SPI1;
    
//http://chipspace.ru/stm32-spi/
//http://microsin.net/programming/arm/stm32f407-spi.html
    
    
    // NRF24
    //MOSI, SCK, CE
    GPIOB->CRL |=  (GPIO_CRL_MODE6 |
        GPIO_CRL_MODE5 |
        GPIO_CRL_MODE3);
    GPIOB->CRL &= ~(GPIO_CRL_CNF6 |
        GPIO_CRL_CNF5 |
        GPIO_CRL_CNF3);
#if HAL_SPI1__
#else
    GPIOB->CRL |= (GPIO_CRL_CNF5_1 | //alt function - 5,3
        GPIO_CRL_CNF3_1);
#endif //HAL_SPI1__


    //MISO
    GPIOB->CRL &= ~(GPIO_CRL_MODE4 |
        GPIO_CRL_CNF4);
    GPIOB->CRL |= (GPIO_CRL_CNF4_1);  
    GPIOB->ODR |= GPIO_Pin_4; //pull-up   
    
    //NSS
    GPIOA->CRH |=  (GPIO_CRH_MODE15);
    GPIOA->CRH &= ~(GPIO_CRH_CNF15);   
    
#if  HAL_SPI1__
#else
    //Конфигурируем SPI в мастер - режиме
    //SPI1->CR1 = 0; //clear @todo
    SPI1->CR2 = 0;
    // 1. Ставим частоту передачи Fclck/4
    SPI1->CR1 = (SPI_CR1_BR_1 | SPI_CR1_BR_0 | //SPI_CR1_BR_2 | 
    // 2. Ставим Clock Polarity 1 when idle
        //SPI_CR1_CPOL |
    // 3. Ставим необходимую Clock Phase
        //SPI_CR1_CPHA |
    // 4. Размер кадра - 8 бит
        //SPI_CR1_DFF |
    // 5. Устанавливаем мастер - режим
        SPI_CR1_MSTR |
    // 6. Устанавливаем тип коммуникации в 1 направлении
        //SPI_CR1_BIDIMODE |
    // 7. Включаем только передачу
        //SPI_CR1_BIDIOE
        
        SPI_CR1_SSM | //Программный режим NSS
        SPI_CR1_SSI //Аналогично состоянию, когда на входе NSS высокий уровень

        );
    SPI1->CR2 |= SPI_CR2_SSOE; //Вывод NSS - выход управления slave select
    
    
    // 6. Включаем SPI
    SPI1->CR1 |= SPI_CR1_SPE;
#endif //HAL_SPI1__
//   
// //     while (!(SPI1->SR & SPI_I2S_FLAG_TXE)){}; // Wait until the data has been transmitted.
// //     //while (!(SPI1->SR & SPI_I2S_FLAG_RXNE)); // Wait for any data on MISO pin to be received.
// //     //while (SPI1->SR & SPI_I2S_FLAG_BSY); // All data transmitted/received but SPI may be busy so wait until done.

// //     _delay_ms(1);
// //     
//     // drain SPI
//     while (0 == (SPI1->SR & SPI_I2S_FLAG_TXE)) {};
//     //while (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_TXE) == 0)// { ; }
//     //(void)SPI_I2S_ReceiveData (SPI);
//     (void)SPI1->DR;
    
#else
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_SPI1EN;
    
    /* Outputs config for SPI1:
    * PA4 - NSS
    * PA5 - SCK
    * PA6 - MISO
    * PA7 - MOSI
    */
    GPIOA->CRL &= ~(GPIO_CRL_MODE4 | GPIO_CRL_MODE5 | GPIO_CRL_MODE6 | GPIO_CRL_MODE7 |
        GPIO_CRL_CNF4 | GPIO_CRL_CNF5 | GPIO_CRL_CNF6 | GPIO_CRL_CNF7); //clear all
    
    GPIOA->CRL |=  (GPIO_CRL_MODE4); // push-pull out 50MHz
    
    GPIOA->CRL |=  (GPIO_CRL_MODE5 |  GPIO_CRL_MODE7 |
        GPIO_CRL_CNF5_1 | GPIO_CRL_CNF7_1); // push-pull alt-out 50MHz

    GPIOA->CRL |=  (GPIO_CRL_CNF6_1); // input


#endif //BOARD_STM32EV103
}

#endif

/**
 * Прерывание
 */
void halSPI1_IRQ (void) { // общее прерывание для передачи и приема
#ifdef HAL_SPI1
    volatile uint16_t fStatus;
//    struct buf_st_3 *p;
    
#else
    while (1) {};
#endif
}
