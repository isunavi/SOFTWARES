#include "drvNRF24L01P.h"
#include "board.h"
//#include "debug.h"

#include "halSPI.h"

//------------------------------------------------------------------------------
static const uint8_t nRF24_TEST_ADDR[] = {"nRF24"};

static const uint8_t RX_PW_PIPES[6] = {
    nRF24_REG_RX_PW_P0,
    nRF24_REG_RX_PW_P1,
    nRF24_REG_RX_PW_P2,
    nRF24_REG_RX_PW_P3,
    nRF24_REG_RX_PW_P4,
    nRF24_REG_RX_PW_P5
};

static const uint8_t RX_ADDR_PIPES[6] = {
    nRF24_REG_RX_ADDR_P0,
    nRF24_REG_RX_ADDR_P1,
    nRF24_REG_RX_ADDR_P2,
    nRF24_REG_RX_ADDR_P3,
    nRF24_REG_RX_ADDR_P4,
    nRF24_REG_RX_ADDR_P5
};


extern uint8_t halSPI_NRF24LC01P_xput (uint8_t);

// GPIO and SPI initialization
void nRF24_init (void)
{
#ifdef BOARD_STM8LDISCOVERY
    GPIOD->DDR &= ~((1 << 2) | (1 << 3)); //
    GPIOD->CR1 |=  ((1 << 2) | (1 << 3));
    GPIOD->CR2 &= ~((1 << 2) | (1 << 3));
    GPIOB->DDR |= ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
    GPIOB->CR1 |= ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
#endif //BOARD_STM8LDISCOVERY
#if BOARD_STM8TEST
    // IRQ  --> PC0
    // CE   <-- PB3
    // CSN  <-- PB4
    // SCK  <-- PB5
    // MOSI <-- PB6
    // MISO --> PB7

    /*
    // Configure the SPI
    SPI1_CR1_bit.BR       = 0; // Baud = Fsysclk/2
    SPI1_CR1_bit.CPHA     = 0; // CPHA = 1st edge
    SPI1_CR1_bit.CPOL     = 0; // CPOL = low (SCK low when idle)
    SPI1_CR1_bit.LSBFIRST = 0; // first bit is MSB
    SPI1_CR1_bit.MSTR     = 1; // Master configuration
    SPI1_CR1_bit.SPE      = 0; // Peripheral enabled
    SPI1_CR2_bit.BDM     = 0; // 2-line unidirectional data mode
    SPI1_CR2_bit.BD0E    = 0; // don't care when BDM set to 0
    SPI1_CR2_bit.CRCEN   = 1; // CRC enabled
    SPI1_CR2_bit.CRCNEXT = 0;
    SPI1_CR2_bit.RXOnly  = 0; // Full duplex
    SPI1_CR2_bit.SSM     = 1; // Software slave management enabled
    SPI1_CR2_bit.SSI     = 1; // Master mode
    */

/*
    // Configure the SPI
    //   - MSB first
    //   - Baud = Fsysclk/2
    //   - Master mode
    //   - CPOL = low
    //   - CPHA = 1st edge
    SPI1_CR1 = 0x04;
    //   - 2-line unidirectional data mode
    //   - full duplex
    //   - software slave management enabled
    //   - CRC generation enabled
    SPI1_CR2 = 0x23;

    // SPI CRC polynominal value
    SPI1_CRCPR = 0x07;

    // SPI enabled
    SPI1_CR1_bit.SPE = 1;
*/
   
#endif //BOARD_STM8TEST
#if BOARD_STM8L051
    CLK_PeripheralClockConfig (CLK_Peripheral_SPI1, ENABLE);
    SPI_Init (SPI1,
        SPI_FirstBit_MSB,
        SPI_BaudRatePrescaler_4, //SPI_BAUDRATEPRESCALER_16
        SPI_Mode_Master,
        SPI_CPOL_Low,
        SPI_CPHA_1Edge,
        SPI_Direction_2Lines_FullDuplex,
        SPI_NSS_Soft,
        (uint8_t)0x07
        );
    SPI_Cmd(SPI1, ENABLE);
#endif    
#if BOARD_STM32F4DISCOVERY
    GPIO_InitTypeDef GPIO_InitStruct;

    /* GPIO Ports Clock Enable */
    __GPIOA_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = NRF24L01P_GPIO_CSN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init (GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = NRF24L01P_GPIO_SCK_PIN | NRF24L01P_GPIO_MOSI_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init (GPIOC, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = NRF24L01P_GPIO_CE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init (GPIOD, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = NRF24L01P_GPIO_MISO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = NRF24L01P_GPIO_IRQ_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init (GPIOD, &GPIO_InitStruct);
  
#endif
    
    halSPI1_init (0);
    NRF24L01P_GPIO_CE_L; // CE pin low -> power down mode at startup
    NRF24L01P_GPIO_CSN_H;
    
    HAL_Delay (5); //wait for the radio to init
    
    //nRF24_WriteReg (nRF24_REG_SETUP_RETR, (0x4 << 4) | (0xF << 0));
    
    nRF24_WriteReg (nRF24_REG_DYNPD, 0);
    nRF24_WriteReg (nRF24_REG_FEATURE, 0);
    
#ifndef IRQ_POLL
    nRF24_ClearIRQFlags();
#endif

#ifdef SPI_USE_DMATX
    // Initialize the DMA peripheral and DMA SPI TX channel
    SPI1_InitDMA();
#endif

}


void nRF24_WriteReg (uint8_t reg, uint8_t value)
{
    NRF24L01P_GPIO_CSN_L;
    halSPI_NRF24LC01P_xput (nRF24_CMD_WREG | reg); // Select register
    halSPI_NRF24LC01P_xput (value); // Write value to register
    NRF24L01P_GPIO_CSN_H;
}


uint8_t nRF24_ReadReg (uint8_t reg)
{
    uint8_t value;

    NRF24L01P_GPIO_CSN_L;
    halSPI_NRF24LC01P_xput (reg & 0x1F); // Select register to read from
    value = halSPI_NRF24LC01P_xput (nRF24_CMD_NOP); // Read register value 
    NRF24L01P_GPIO_CSN_H;

    return value;
}


uint8_t nRF24_getStatus (void)
{
    uint8_t status = 0;;

    NRF24L01P_GPIO_CSN_L;
    status = halSPI_NRF24LC01P_xput (nRF24_CMD_NOP); // Read register value
    NRF24L01P_GPIO_CSN_H;

    return status;
}


// Read specified amount of data from the nRF24L01 into data buffer
// input:
//   reg - register number
//   pBuf - pointer to the data buffer
//   count - number of bytes to read
void nRF24_ReadBuf (uint8_t reg, uint8_t *pBuf, uint8_t count) 
{
    NRF24L01P_GPIO_CSN_L;
    halSPI_NRF24LC01P_xput (reg & 0x1F); // Send buffer address
    while (count)
    {
        *pBuf = halSPI_NRF24LC01P_xput (nRF24_CMD_NOP); // Read received byte into buffer (clears the RXNE flag)
        pBuf++;
        count--;
    }
    NRF24L01P_GPIO_CSN_H;
}


// Send data buffer to the nRF24L01
// input:
//   reg - register number
//   pBuf - pointer to the data buffer
//   count - number of bytes to send
void nRF24_WriteBuf (uint8_t reg, uint8_t *pBuf, uint8_t count) 
{
    NRF24L01P_GPIO_CSN_L;
    halSPI_NRF24LC01P_xput (nRF24_CMD_WREG | reg); // Send buffer address
    while (count)
    {
        halSPI_NRF24LC01P_xput (*pBuf); // Transmit byte
        pBuf++;
        count--;
    }
    NRF24L01P_GPIO_CSN_H;
}



bool_t nRF24_check (void) 
{
    uint8_t rxbuf[5];
    uint8_t i;
    
    // Write fake TX address
    nRF24_WriteBuf (nRF24_REG_TX_ADDR, (uint8_t *)nRF24_TEST_ADDR, 5);
    nRF24_ReadBuf (nRF24_REG_TX_ADDR, &rxbuf[0], 5); // Read TX_ADDR register
    for (i = 0; i < 5; i++)
    {
        if (rxbuf[i] != nRF24_TEST_ADDR[i])
        {
            return FALSE;
        }
    }

    return TRUE;
}



void nRF24_SetRFChannel(uint8_t RFChannel)
{
    nRF24_WriteReg (nRF24_REG_RF_CH, RFChannel);
}


void nRF24_FlushTX (void) // Flush nRF24L01 TX FIFO buffer
{
    NRF24L01P_GPIO_CSN_L;
    halSPI_NRF24LC01P_xput (nRF24_CMD_FLUSH_TX);
    NRF24L01P_GPIO_CSN_H;
}


void nRF24_FlushRX (void) // Flush nRF24L01 RX FIFO buffer
{
    NRF24L01P_GPIO_CSN_L;
    halSPI_NRF24LC01P_xput (nRF24_CMD_FLUSH_RX);
    NRF24L01P_GPIO_CSN_H;
}



void nRF24_TXMode (uint8_t RetrCnt, uint8_t RetrDelay, uint8_t RFChan, nRF24_DataRate_TypeDef DataRate,
                  nRF24_TXPower_TypeDef TXPower, nRF24_CRC_TypeDef CRCS, nRF24_PWR_TypeDef Power, uint8_t *TX_Addr,
                  uint8_t TX_Addr_Width)
{
    uint8_t rreg;

    NRF24L01P_GPIO_CE_L;
    nRF24_ReadReg (0x00); // Dummy read
    nRF24_WriteReg (nRF24_REG_SETUP_AW, TX_Addr_Width - 2); // Set address width
    nRF24_WriteBuf (nRF24_REG_TX_ADDR, TX_Addr, TX_Addr_Width); // Set static TX address
    nRF24_WriteReg (nRF24_REG_RF_SETUP, (uint8_t)DataRate | (uint8_t)TXPower); // Setup register
    nRF24_WriteReg (nRF24_REG_CONFIG, (uint8_t)CRCS | (uint8_t)Power | nRF24_PRIM_TX); // Config register
    nRF24_SetRFChannel (RFChan); // Set frequency channel (OBSERVER_TX part PLOS_CNT will be cleared)
    rreg = nRF24_ReadReg (nRF24_REG_EN_AA);
    nRF24_WriteReg (nRF24_REG_SETUP_RETR, (RetrDelay << 4) | (RetrCnt & 0x0F)); // Auto retransmit settings
    if (RetrCnt)
    {
        // Enable auto acknowledgment for data pipe 0
        rreg |= nRF24_ENAA_P0;
        // Static RX address of the PIPE0 must be same as TX address for auto ack
        nRF24_WriteBuf (nRF24_REG_RX_ADDR_P0, TX_Addr, TX_Addr_Width);
    }
    else
    {
        // Disable auto acknowledgment for data pipe 0
        rreg &= ~nRF24_ENAA_P0;
    }
    nRF24_WriteReg (nRF24_REG_EN_AA, rreg);
}


void nRF24_RXMode (nRF24_RX_PIPE_TypeDef PIPE,
    nRF24_ENAA_TypeDef PIPE_AA,
    uint8_t RFChan,
    nRF24_DataRate_TypeDef DataRate,
    nRF24_CRC_TypeDef CRCS,
    uint8_t *RX_Addr,
    uint8_t RX_Addr_Width,
    uint8_t RX_PAYLOAD,
    nRF24_TXPower_TypeDef TXPower)
{
    uint8_t rreg;

    NRF24L01P_GPIO_CE_L;
    nRF24_ReadReg (0x00); // Dummy read nRF24_CMD_NOP
    nRF24_WriteReg (nRF24_REG_EN_AA, PIPE_AA);
    nRF24_WriteReg (nRF24_REG_EN_RXADDR, (1 << PIPE)); // Enable given data pipe
    nRF24_WriteReg (RX_PW_PIPES[(uint8_t)PIPE], RX_PAYLOAD); // Set RX payload length
    nRF24_WriteReg (nRF24_REG_RF_SETUP, (uint8_t)DataRate | (uint8_t)TXPower); // SETUP register
    nRF24_WriteReg (nRF24_REG_CONFIG, (uint8_t)CRCS | nRF24_PWR_Up | nRF24_PRIM_RX); // Config register
    nRF24_SetRFChannel (RFChan); // Set frequency channel
    nRF24_WriteReg (nRF24_REG_SETUP_AW, RX_Addr_Width - 2); // Set of address widths (common for all data pipes)
    nRF24_WriteBuf (RX_ADDR_PIPES[(uint8_t)PIPE], RX_Addr, RX_Addr_Width); // Set static RX address for given data pipe
    
    nRF24_ClearIRQFlags ();
    nRF24_FlushRX ();
    NRF24L01P_GPIO_CE_H; // RX mode
    HAL_Delay (1); //_delay_us (150);
}


nRF24_TX_PCKT_TypeDef nRF24_TXPacket (uint8_t *pBuf, uint8_t TX_PAYLOAD)
{
    uint8_t status;
    uint16_t wait;

#ifdef IRQ_POLL
    // Wait for an IRQ from the nRF24L01 through a GPIO polling
    wait = nRF24_WAIT_TIMEOUT;
    NRF24L01P_GPIO_CE_L; // Release CE pin (in case if it still high)
    // Transfer data from specified buffer to the TX FIFO
    //nRF24_WriteBuf_CRC (nRF24_CMD_W_TX_PAYLOAD, pBuf, TX_PAYLOAD);
    nRF24_WriteBuf (nRF24_CMD_W_TX_PAYLOAD, pBuf, TX_PAYLOAD);

    
    NRF24L01P_GPIO_CE_H; // CE pin high => Start transmit (must hold pin at least 10us)
    
    //HAL_Delay (10); //TODO
    
    while (NRF24L01P_GPIO_IRQ_IN && --wait) // Wait for IRQ from nRF24L01
    {
        HAL_Delay (1); //asm("WFI")
    }
    
    NRF24L01P_GPIO_CE_L; // Release CE pin
    
    if (!wait) return nRF24_TX_TIMEOUT; // Timeout?
#endif //IRQ_POLL
    
    // Read the status register
    status = nRF24_getStatus ();
    // Clear pending IRQ flags
    nRF24_WriteReg (nRF24_REG_STATUS, status | 0x70);
    if (status & nRF24_MASK_MAX_RT)
    {
        // Auto retransmit counter exceeds the programmed maximum limit. FIFO is not removed.
        nRF24_FlushTX ();
        return nRF24_TX_MAXRT;
    }
    if (status & nRF24_MASK_TX_DS)
    {
        // Transmit successful
        return nRF24_TX_SUCCESS;
    }

    // Some banana happens -zay WTF?
    nRF24_FlushTX ();
    nRF24_ClearIRQFlags ();

    return nRF24_TX_ERROR;
}


nRF24_RX_PCKT_TypeDef nRF24_RXPacket (uint8_t *pBuf, uint8_t RX_PAYLOAD)
{
    uint8_t status;
    nRF24_RX_PCKT_TypeDef result = nRF24_RX_PCKT_ERROR;

    status = nRF24_getStatus (); //nRF24_ReadReg (nRF24_REG_STATUS); // Read the status register
    if (status & nRF24_MASK_RX_DR)
    {
        // RX_DR bit set (Data ready RX FIFO interrupt)
        result = (nRF24_RX_PCKT_TypeDef)((status & 0x0e) > 1); // Pipe number
        if ((uint8_t)result < 6)
        {
M1:
            // Read received payload from RX FIFO buffer
            //nRF24_ReadBuf (nRF24_CMD_R_RX_PAYLOAD, pBuf, RX_PAYLOAD);
            NRF24L01P_GPIO_CSN_L;
            halSPI_NRF24LC01P_xput (nRF24_CMD_R_RX_PAYLOAD);
            for (uint8_t i = 0; i < RX_PAYLOAD; i++)
            {
                pBuf [i] = halSPI_NRF24LC01P_xput (0xFF);
            }
            NRF24L01P_GPIO_CSN_H;
            
            // Clear pending IRQ flags
            nRF24_WriteReg (nRF24_REG_STATUS, status | 0x70);
            // Check if RX FIFO is empty and flush it if not
            status = nRF24_ReadReg (nRF24_REG_FIFO_STATUS);
            if (status & nRF24_FIFO_RX_EMPTY) 
            {
                nRF24_FlushRX();
            }
            else 
            {
                //goto M1;
            }
            return result; // Data pipe number
        }
        else
        {
            // RX FIFO is empty
            return nRF24_RX_PCKT_EMPTY;
        }
    }

    // Some banana happens
    nRF24_FlushRX (); // Flush the RX FIFO buffer
    nRF24_ClearIRQFlags ();

    return result;
}

void nRF24_ClearIRQFlags(void)
{
    uint8_t status;

    status = nRF24_ReadReg (nRF24_REG_STATUS);
    nRF24_WriteReg (nRF24_REG_STATUS, status | 0x70);
}


void nRF24_PowerDown(void)
{
    uint8_t conf;

    NRF24L01P_GPIO_CE_L; // CE pin to low
    conf  = nRF24_ReadReg (nRF24_REG_CONFIG);
    conf &= ~(1 << 1); // Clear PWR_UP bit
    nRF24_WriteReg (nRF24_REG_CONFIG, conf); // Go Power down mode
}


void nRF24_Wake(void)
{
    uint8_t conf;

    conf = nRF24_ReadReg (nRF24_REG_CONFIG) | (1 << 1); // Set PWR_UP bit
    nRF24_WriteReg (nRF24_REG_CONFIG, conf); // Wake-up
    HAL_Delay (1);
}


void nRF24_SetTXPower(nRF24_TXPower_TypeDef TXPower)
{
    uint8_t rf_setup;

    rf_setup  = nRF24_ReadReg(nRF24_REG_RF_SETUP);
    rf_setup &= 0xF9; // Clear RF_PWR bits
    nRF24_WriteReg (nRF24_REG_RF_SETUP, rf_setup | (uint8_t)TXPower);
}
