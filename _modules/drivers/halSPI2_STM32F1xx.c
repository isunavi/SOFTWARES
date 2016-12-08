#include "halSPI.h"
#include "defines.h"
#include "board.h"


#ifdef HAL_SPI2

//------------------------------------------------------------------------------
// Прием байта/ов
//------------------------------------------------------------------------------
uint8_t halSPI2_xput (uint8_t data)
{
    SPI2->DR = data;
		// All data transmitted/received but SPI may be busy so wait until done.
    //while (0 != (SPI2->SR & SPI_I2S_FLAG_BSY)) {};
        
    
    while (0 != (SPI2->SR & SPI_I2S_FLAG_BSY)) {}; // All data transmitted/received but SPI may be busy so wait until done.
    while (0 == (SPI2->SR & SPI_I2S_FLAG_RXNE)) {}; // Wait for any data on MISO pin to be received.
  while (0 == (SPI2->SR & SPI_I2S_FLAG_TXE)) {}; //{spi_a++;}; // Wait until the data has been transmitted.
      
    return (uint8_t)(SPI2->DR);
}


void halSPI2_init (uint8_t baud)
{
#if (BOARD_STM32EV103)
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    // Отречемся от старого мира
    RCC->APB1RSTR |= RCC_APB1Periph_SPI2;
    RCC->APB1RSTR &= ~RCC_APB1Periph_SPI2;
    
    /* Outputs config for SPI2:
    * 
    * PB13 - SCK
    * 
    * PB15 - MOSI
    */
    GPIOB->CRH &= ~(GPIO_CRH_MODE15 | GPIO_CRH_MODE13 |
        GPIO_CRH_CNF15 | GPIO_CRH_CNF13); //clear all
    
    GPIOB->CRH |=  (GPIO_CRH_MODE15 |  GPIO_CRH_MODE13 |
        GPIO_CRH_CNF15_1 | GPIO_CRH_CNF13_1); // push-pull alt-out 50MHz

    //GPIOA->CRH |=  (GPIO_CRL_CNF6_1); // input

    //Конфигурируем SPI в мастер - режиме
    //SPI1->CR1 = 0; //clear @todo
    SPI2->CR2 = 0;
    // 1. Ставим частоту передачи Fclck/4
    SPI2->CR1 = (SPI_CR1_BR_0 | //SPI_CR1_BR_2 | SPI_CR1_BR_1 | 
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
    SPI2->CR2 |= SPI_CR2_SSOE; //Вывод NSS - выход управления slave select
    // 6. Включаем SPI
    SPI2->CR1 |= SPI_CR1_SPE;
#else

#endif //BOARD_xxx
}

#endif

/**
 * Прерывание
 */
void halSPI2_IRQ (void) { // общее прерывание для передачи и приема
#ifdef HAL_SPI2
    volatile uint16_t fStatus;
//    struct buf_st_3 *p;
    
#else
    while (1) {};
#endif
}
