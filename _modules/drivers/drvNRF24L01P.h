/**
 * @file    drv24L01P.h
 * @author  Ht3h5793, CD45
 * @date    25.11.2014  8.19
 * @version V1.0.0
 * @brief 

основа - https://github.com/LonelyWolf/stm8/tree/master/stm8l051-cdcspd-sensor
http://zewaren.net/site/?q=node/110
http://www.avislab.com/blog/nrf24l01/

*/
 

#ifndef DRVNRF24L01P_H
#define	DRVNRF24L01P_H 20160718

/**
 *  Раздел для "include"
 */
#include "board.h"

/**
 *  Раздел для "define"
 */

typedef enum {
    nRF24_1,
    nRF24_2,
    //nRF24_3,
    //....
    
} nRF24_TypeDef;


// If defined - use DMA to transfer data packet via SPI
//#define SPI_USE_DMATX

// If defined wait for IRQ in polling mode, WFI otherwise
#define IRQ_POLL


////////////////////////////////////////////////////////////////////////////////////////////////


// nRF24L01 data rate
typedef enum {
    nRF24_DataRate_250kbps = (uint8_t)0x20, // 250kbps data rate
    nRF24_DataRate_1Mbps   = (uint8_t)0x00, // 1Mbps data rate
    nRF24_DataRate_2Mbps   = (uint8_t)0x08  // 2Mbps data rate
} nRF24_DataRate_TypeDef;

// nRF24L01 RF output power in TX mode
typedef enum {
    nRF24_TXPower_18dBm = (uint8_t)0x00, // -18dBm
    nRF24_TXPower_12dBm = (uint8_t)0x02, // -12dBm
    nRF24_TXPower_6dBm  = (uint8_t)0x04, //  -6dBm
    nRF24_TXPower_0dBm  = (uint8_t)0x06  //   0dBm
} nRF24_TXPower_TypeDef;

// nRF24L01 CRC encoding scheme
typedef enum {
    nRF24_CRC_off   = (uint8_t)0x00, // CRC disabled
    nRF24_CRC_1byte = (uint8_t)0x08, // 1-byte CRC
    nRF24_CRC_2byte = (uint8_t)0x0c  // 2-byte CRC
} nRF24_CRC_TypeDef;

// nRF24L01 power control
typedef enum {
    nRF24_PWR_Up   = (uint8_t)0x02, // Power up
    nRF24_PWR_Down = (uint8_t)0x00  // Power down
} nRF24_PWR_TypeDef;

// nRF24L01 RX/TX control
typedef enum {
    nRF24_PRIM_RX = (uint8_t)0x01, // PRX
    nRF24_PRIM_TX = (uint8_t)0x00  // PTX
} nRF24_PRIM_TypeDef;

// RX data pipe

typedef enum {
    nRF24_RX_PIPE0 = (uint8_t)0x00,
    nRF24_RX_PIPE1 = (uint8_t)0x01,
    nRF24_RX_PIPE2 = (uint8_t)0x02,
    nRF24_RX_PIPE3 = (uint8_t)0x03,
    nRF24_RX_PIPE4 = (uint8_t)0x04,
    nRF24_RX_PIPE5 = (uint8_t)0x05
} nRF24_RX_PIPE_TypeDef;

// nRF24L01 enable auto acknowledgment
typedef enum {
    nRF24_ENAA_OFF = (uint8_t)0x00, // Disable auto acknowledgment
    nRF24_ENAA_P0  = (uint8_t)0x01, // Enable auto acknowledgment for PIPE#0
    nRF24_ENAA_P1  = (uint8_t)0x02, // Enable auto acknowledgment for PIPE#1
    nRF24_ENAA_P2  = (uint8_t)0x04, // Enable auto acknowledgment for PIPE#2
    nRF24_ENAA_P3  = (uint8_t)0x08, // Enable auto acknowledgment for PIPE#3
    nRF24_ENAA_P4  = (uint8_t)0x10, // Enable auto acknowledgment for PIPE#4
    nRF24_ENAA_P5  = (uint8_t)0x20  // Enable auto acknowledgment for PIPE#5
} nRF24_ENAA_TypeDef;

// RX packet pipe
typedef enum {
    nRF24_RX_PCKT_PIPE0 = (uint8_t)0x00,
    nRF24_RX_PCKT_PIPE1 = (uint8_t)0x01,
    nRF24_RX_PCKT_PIPE2 = (uint8_t)0x02,
    nRF24_RX_PCKT_PIPE3 = (uint8_t)0x03,
    nRF24_RX_PCKT_PIPE4 = (uint8_t)0x04,
    nRF24_RX_PCKT_PIPE5 = (uint8_t)0x05,
    nRF24_RX_PCKT_EMPTY = (uint8_t)0xfe,
    nRF24_RX_PCKT_ERROR = (uint8_t)0xff
} nRF24_RX_PCKT_TypeDef;

// TX packet result
typedef enum {
    nRF24_TX_SUCCESS,   // Packet transmitted successfully
    nRF24_TX_TIMEOUT,   // It was timeout during packet transmit
    nRF24_TX_MAXRT,     // Transmit failed with maximum auto retransmit count
    nRF24_TX_ERROR      // Some error happens
} nRF24_TX_PCKT_TypeDef;


////////////////////////////////////////////////////////////////////////////////////////////////
// nRF24L0 commands
#define nRF24_CMD_RREG             0x00  // R_REGISTER -> Read command and status registers
#define nRF24_CMD_WREG             0x20  // W_REGISTER -> Write command and status registers
#define nRF24_CMD_R_RX_PAYLOAD     0x61  // R_RX_PAYLOAD -> Read RX payload
#define nRF24_CMD_W_TX_PAYLOAD     0xA0  // W_TX_PAYLOAD -> Write TX payload
#define nRF24_CMD_FLUSH_TX         0xE1  // FLUSH_TX -> Flush TX FIFO
#define nRF24_CMD_FLUSH_RX         0xE2  // FLUSH_RX -> Flush RX FIFO
#define nRF24_CMD_REUSE_TX_PL      0xE3  // REUSE_TX_PL -> Reuse last transmitted payload
#define nRF24_CMD_NOP              0xFF  // No operation (to read status register)

// nRF24L0 registers
#define nRF24_REG_CONFIG           0x00  // Configuration register
#define nRF24_REG_EN_AA            0x01  // Enable "Auto acknowledgment"
#define nRF24_REG_EN_RXADDR        0x02  // Enable RX addresses
#define nRF24_REG_SETUP_AW         0x03  // Setup of address widths
#define nRF24_REG_SETUP_RETR       0x04  // Setup of automatic retranslation
#define nRF24_REG_RF_CH            0x05  // RF channel
#define nRF24_REG_RF_SETUP         0x06  // RF setup register
#define nRF24_REG_STATUS           0x07  // Status register
#define nRF24_REG_OBSERVE_TX       0x08  // Transmit observe register
#define nRF24_REG_RPD              0x09  // Received power detector
#define nRF24_REG_RX_ADDR_P0       0x0A  // Receive address data pipe 0
#define nRF24_REG_RX_ADDR_P1       0x0B  // Receive address data pipe 1
#define nRF24_REG_RX_ADDR_P2       0x0C  // Receive address data pipe 2
#define nRF24_REG_RX_ADDR_P3       0x0D  // Receive address data pipe 3
#define nRF24_REG_RX_ADDR_P4       0x0E  // Receive address data pipe 4
#define nRF24_REG_RX_ADDR_P5       0x0F  // Receive address data pipe 5
#define nRF24_REG_TX_ADDR          0x10  // Transmit address
#define nRF24_REG_RX_PW_P0         0x11  // Number of bytes in RX payload id data pipe 0
#define nRF24_REG_RX_PW_P1         0x12  // Number of bytes in RX payload id data pipe 1
#define nRF24_REG_RX_PW_P2         0x13  // Number of bytes in RX payload id data pipe 2
#define nRF24_REG_RX_PW_P3         0x14  // Number of bytes in RX payload id data pipe 3
#define nRF24_REG_RX_PW_P4         0x15  // Number of bytes in RX payload id data pipe 4
#define nRF24_REG_RX_PW_P5         0x16  // Number of bytes in RX payload id data pipe 5
#define nRF24_REG_FIFO_STATUS      0x17  // FIFO status register
#define nRF24_REG_DYNPD            0x1C  // Enable dynamic payload length
#define nRF24_REG_FEATURE          0x1D  // Feature register

// nRF24L0 bits
#define nRF24_MASK_RX_DR           0x40  // Mask interrupt caused by RX_DR
#define nRF24_MASK_TX_DS           0x20  // Mask interrupt caused by TX_DS
#define nRF24_MASK_MAX_RT          0x10  // Mask interrupt caused by MAX_RT
#define nRF24_FIFO_RX_EMPTY        0x01  // RX FIFO empty flag
#define nRF24_FIFO_RX_FULL         0x02  // RX FIFO full flag

//#define nRF24_TEST_ADDR         "nRF24"  // Fake address to test nRF24 presence

#define nRF24_WAIT_TIMEOUT       0xFFFF  // Timeout counter




////////////////////////////////////////////////////////////////////////////////////////////////
//address size
#define NRF24L01_ADDRSIZE 5

//pipe address
#define NRF24L01_ADDRP0 {0xE8, 0xE8, 0xF0, 0xF0, 0xE2} //pipe 0, 5 byte address
#define NRF24L01_ADDRP1 {0xC1, 0xC2, 0xC2, 0xC2, 0xC2} //pipe 1, 5 byte address
#define NRF24L01_ADDRP2 {0xC1, 0xC2, 0xC2, 0xC2, 0xC3} //pipe 2, 5 byte address
#define NRF24L01_ADDRP3 {0xC1, 0xC2, 0xC2, 0xC2, 0xC4} //pipe 3, 5 byte address
#define NRF24L01_ADDRP4 {0xC1, 0xC2, 0xC2, 0xC2, 0xC5} //pipe 4, 5 byte address
#define NRF24L01_ADDRP5 {0xC1, 0xC2, 0xC2, 0xC2, 0xC6} //pipe 5, 5 byte address
#define NRF24L01_ADDRTX {0xE8, 0xE8, 0xF0, 0xF0, 0xE2} //tx default address*/



/**
 *  Раздел для "typedef"
 */

#ifdef	__cplusplus
extern "C" {
#endif

/**
 *  Раздел для прототипов функций
 */
    

void nRF24_init (void);

// Write new value to the nRF24L01 register
// input:
//   reg - register number
//   value - new value
// return: nRF24L01 status
void nRF24_WriteReg (uint8_t reg, uint8_t value);

// Read value of the nRF24L01 register
// input:
//   reg - register number
// return: register value
uint8_t nRF24_ReadReg (uint8_t reg);

// Check if nRF24L01 present (send byte sequence, read it back and compare)
// return:
//   1 - looks like an nRF24L01 is online
//   0 - received sequence differs from original
bool_t nRF24_check (void);

// Set nRF24L01 frequency channel
// input:
//   RFChannel - Frequency channel (0..127) (frequency = 2400 + RFChan [MHz])
// Note, what part of the OBSERVER_TX register called "PLOS_CNT" will be cleared!
void nRF24_SetRFChannel (uint8_t RFChannel);
void nRF24_FlushTX(void);
void nRF24_FlushRX(void);

// Put nRF24L01 in TX mode
// input:
//   RetrCnt - Auto retransmit count on fail of AA (1..15 or 0 for disable)
//   RetrDelay - Auto retransmit delay 250us+(0..15)*250us (0 = 250us, 15 = 4000us)
//   RFChan - Frequency channel (0..127) (frequency = 2400 + RFChan [MHz])
//   DataRate - Set data rate: nRF24_DataRate_1Mbps or nRF24_DataRate_2Mbps
//   TXPower - RF output power (-18dBm, -12dBm, -6dBm, 0dBm)
//   CRCS - CRC encoding scheme (nRF24_CRC_[off | 1byte | 2byte])
//   Power - power state (nRF24_PWR_Up or nRF24_PWR_Down)
//   TX_Addr - buffer with TX address
//   TX_Addr_Width - size of the TX address (3..5 bytes)
void nRF24_TXMode (uint8_t RetrCnt, 
                   uint8_t RetrDelay, 
                   uint8_t RFChan, 
                   nRF24_DataRate_TypeDef DataRate, 
                  nRF24_TXPower_TypeDef TXPower, 
                   nRF24_CRC_TypeDef CRCS, 
                   nRF24_PWR_TypeDef Power,
                   uint8_t *TX_Addr,
                  uint8_t TX_Addr_Width);

// Put nRF24L01 in RX mode
// input:
//   PIPE - RX data pipe (nRF24_RX_PIPE[0..5])
//   PIPE_AA - auto acknowledgment for data pipe (nRF24_ENAA_P[0..5] or nRF24_ENAA_OFF)
//   RFChan - Frequency channel (0..127) (frequency = 2400 + RFChan [MHz])
//   DataRate - Set data rate (nRF24_DataRate_[250kbps,1Mbps,2Mbps])
//   CRCS - CRC encoding scheme (nRF24_CRC_[off | 1byte | 2byte])
//   RX_Addr - buffer with TX address
//   RX_Addr_Width - size of TX address (3..5 byte)
//   RX_PAYLOAD - receive buffer length
//   TXPower - RF output power for ACK packets (-18dBm, -12dBm, -6dBm, 0dBm)
void nRF24_RXMode (nRF24_RX_PIPE_TypeDef PIPE,
                   nRF24_ENAA_TypeDef PIPE_AA,
                   uint8_t RFChan,
                  nRF24_DataRate_TypeDef DataRate,
                  nRF24_CRC_TypeDef CRCS,
                  uint8_t *RX_Addr,
                  uint8_t RX_Addr_Width,
                  uint8_t RX_PAYLOAD,
                  nRF24_TXPower_TypeDef TXPower);


// Send data packet
// input:
//   pBuf - buffer with data to send
//   TX_PAYLOAD - buffer size
// return:
//   nRF24_TX_XXX values
nRF24_TX_PCKT_TypeDef nRF24_TXPacket (uint8_t * pBuf, uint8_t TX_PAYLOAD);


// Read received data packet from the nRF24L01
// input:
//   pBuf - buffer for received data
//   RX_PAYLOAD - buffer size
// return:
//   nRF24_RX_PCKT_PIPE[0..5] - packet received from specific data pipe
//   nRF24_RX_PCKT_ERROR - RX_DR bit was not set
//   nRF24_RX_PCKT_EMPTY - RX FIFO is empty
nRF24_RX_PCKT_TypeDef nRF24_RXPacket (uint8_t * pBuf, uint8_t RX_PAYLOAD);

// Clear pending IRQ flags
void nRF24_ClearIRQFlags (void);

// Put nRF24 in Power Down mode
void nRF24_PowerDown (void);

// Wake nRF24 from Power Down mode
// note: with external crystal it wake to Standby-I mode within 1.5ms
void nRF24_Wake (void);

// Configure RF output power in TX mode
// input:
//   TXPower - RF output power (-18dBm, -12dBm, -6dBm, 0dBm)
void nRF24_SetTXPower (nRF24_TXPower_TypeDef TXPower);


#ifdef	__cplusplus
}
#endif

#endif	/** DRVNRF24L01P_H*/
