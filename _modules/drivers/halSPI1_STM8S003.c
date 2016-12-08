#include "halSPI.h"
#include "board.h"


#if HAL_SPI1
void      halSPI1_flush (void)
{
}



uint8_t      halSPI1_xput (uint8_t data)
{
#if BOARD_STM8TEST
// Transmit byte via SPI
// input:
//   data - byte to send
// return: received byte from SPI
    uint8_t rcv;

    SPI->DR = data; // Send byte to SPI (TXE cleared)
    while (!(SPI->SR & SPI_SR_RXNE)) {}; // Wait until byte is received
      //  _delay_us (1);
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

        rcv = SPI->DR; // Read received byte (RXNE cleared)
    while (!(SPI->SR & SPI_SR_TXE)) {}; // Wait for TXE flag --> transmit buffer is empty
    while (SPI->SR & SPI_SR_BSY) {}; // Wait until the transmission is complete
    

    return rcv;
#endif //BOARD_STM8TEST
}


void      halSPI1_init (uint8_t speed)
{
    
    //SPI
    SPI_Init(
      SPI_FIRSTBIT_MSB,
      SPI_BAUDRATEPRESCALER_4, //SPI_BAUDRATEPRESCALER_16
      SPI_MODE_MASTER,
      SPI_CLOCKPOLARITY_LOW,
      SPI_CLOCKPHASE_1EDGE,
      SPI_DATADIRECTION_2LINES_FULLDUPLEX,
      SPI_NSS_SOFT,
      (uint8_t)0x07
    );
    SPI_Cmd(ENABLE);
    
}







//------------------------------------------------------------------------------
// Отправка байта/ов
//------------------------------------------------------------------------------

// Отправка байта
uint8_t     halSPI1_sndS (uint8_t data)
{
    uint8_t rcv;

    SPI->DR = data; // Send byte to SPI (TXE cleared)
    while (!(SPI->SR & SPI_SR_RXNE)) {}; // Wait until byte is received
    //    _delay_us (1);
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    rcv = SPI->DR; // Read received byte (RXNE cleared)
    while (!(SPI->SR & SPI_SR_TXE)) {}; // Wait for TXE flag --> transmit buffer is empty
    while (SPI->SR & SPI_SR_BSY) {}; // Wait until the transmission is complete

    return rcv;
}




#endif