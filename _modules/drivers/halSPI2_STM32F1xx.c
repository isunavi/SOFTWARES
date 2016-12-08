#include "halSPI.h"
#include "defines.h"
#include "board.h"


#ifdef HAL_SPI2

//------------------------------------------------------------------------------
// ����� �����/��
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
    // ��������� �� ������� ����
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

    //������������� SPI � ������ - ������
    //SPI1->CR1 = 0; //clear @todo
    SPI2->CR2 = 0;
    // 1. ������ ������� �������� Fclck/4
    SPI2->CR1 = (SPI_CR1_BR_0 | //SPI_CR1_BR_2 | SPI_CR1_BR_1 | 
    // 2. ������ Clock Polarity 1 when idle
        //SPI_CR1_CPOL |
    // 3. ������ ����������� Clock Phase
        //SPI_CR1_CPHA |
    // 4. ������ ����� - 8 ���
        //SPI_CR1_DFF |
    // 5. ������������� ������ - �����
        SPI_CR1_MSTR |
    // 6. ������������� ��� ������������ � 1 �����������
        //SPI_CR1_BIDIMODE |
    // 7. �������� ������ ��������
        //SPI_CR1_BIDIOE
        
        SPI_CR1_SSM | //����������� ����� NSS
        SPI_CR1_SSI //���������� ���������, ����� �� ����� NSS ������� �������

        );
    SPI2->CR2 |= SPI_CR2_SSOE; //����� NSS - ����� ���������� slave select
    // 6. �������� SPI
    SPI2->CR1 |= SPI_CR1_SPE;
#else

#endif //BOARD_xxx
}

#endif

/**
 * ����������
 */
void halSPI2_IRQ (void) { // ����� ���������� ��� �������� � ������
#ifdef HAL_SPI2
    volatile uint16_t fStatus;
//    struct buf_st_3 *p;
    
#else
    while (1) {};
#endif
}
