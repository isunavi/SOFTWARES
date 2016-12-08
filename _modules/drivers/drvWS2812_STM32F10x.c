#include "drvWS2812.h"
#include "board.h"


/** tested on:
1. STM32F100CB
    CPU speed - 24MHz
    FREQ - 728-762KHz
    0, hi ~0.31-0.37us, low ~1us
    1, hi ~0.63-0.69us, low ~0.69us
*/

// variables
uint16_t halWS2812_DMA_buf [WS2812_DMA_SIZE /2];
uint8_t halWS2812_DMAComplete = 0;


#ifdef WS2812_SPI1
void DMA1_Channel3_IRQHandler (void)
{
	DMA1_Channel3->CCR &= (uint16_t)(~DMA_CCR_EN);
	halWS2812_DMAComplete = 1;
	DMA1->IFCR = DMA_IFCR_CTCIF3;
}
#endif

#ifdef WS2812_SPI2
void DMA1_Channel5_IRQHandler (void)
{
	DMA1_Channel5->CCR &= (uint16_t)(~DMA_CCR_EN);
	halWS2812_DMAComplete = 1; 
	DMA1->IFCR = DMA_IFCR_CTCIF5;
}
#endif


/**
 * Init perifirial (SPI & DMA)
 */
void halWS2812_init (void)
{
    halWS2812_DMA_buf [WS2812_DMA_SIZE - 1] = 0x0000;
    
#ifdef WS2812_SPI1
    // Enable clocks
    RCC->APB2ENR |= (RCC_APB2ENR_IOPBEN | RCC_APB2ENR_SPI1EN | RCC_APB2ENR_AFIOEN);

    // off jtag, on swd
    AFIO->MAPR &= ~(AFIO_MAPR_SWJ_CFG_0 | AFIO_MAPR_SWJ_CFG_2);
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_1;

    AFIO->MAPR   |= AFIO_MAPR_SPI1_REMAP; // Remap SPI1 pins (using JTAG pins)

	// Configure SPI pin: MOSI with default alternate function push-pull
	GPIOB->CRL &= ~(GPIO_CRL_CNF5); //clear
	GPIOB->CRL |= GPIO_CRL_CNF5_1; //PA5(MOSI) - AF, Output, PP
	GPIOB->CRL |=  GPIO_CRL_MODE5; //OUT MOSI - 50MHz

	// Set SPI1
	SPI1->CR1 =
		SPI_CR1_CPHA * 0 |		// Clock Phase
		SPI_CR1_CPOL * 0 |		// Clock Polarity
		SPI_CR1_MSTR * 1 |		// Master Selection
		SPI_CR1_BR_0 * 0 |		// Baud Rate Control - fpclk2 / 8 = 15 MHz
		SPI_CR1_BR_1 * 1 |		//
		SPI_CR1_BR_2 * 0 |		//
		SPI_CR1_SPE * 0 |		// SPI Enable
		SPI_CR1_LSBFIRST * 0 |		// Frame Format
		SPI_CR1_SSI * 1 |		// Internal slave select
		SPI_CR1_SSM * 1 |		// Software slave management
		SPI_CR1_RXONLY * 0 |		// Receive only
		SPI_CR1_DFF * 0 |		// Data Frame Format
		SPI_CR1_CRCNEXT * 0 |		// Transmit CRC next
		SPI_CR1_CRCEN * 0 |		// Hardware CRC calculation enable
		SPI_CR1_BIDIOE * 1 |		// Output enable in bidirectional mode
		SPI_CR1_BIDIMODE * 1;		// Bidirectional data mode enable

	SPI1->CR2 =
		SPI_CR2_RXDMAEN * 0 |		// Rx Buffer DMA Enable
		SPI_CR2_TXDMAEN * 1 |		// Tx Buffer DMA Enable
		SPI_CR2_SSOE * 0 |		// SS Output Enable
		//SPI_CR2_FRF * 0 |	// Protocol format - 0: SPI Motorola mode, 1: SPI TI mode
		SPI_CR2_ERRIE * 0 |		// Error Interrupt Enable
		SPI_CR2_RXNEIE * 0 |		// RX buffer Not Empty Interrupt Enable
		SPI_CR2_TXEIE * 0;		// Tx buffer Empty Interrupt Enable

	SPI1->CR1 |= SPI_CR1_SPE;	// Разрешить, Мастер
    
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;  // On DMA1

	//DMA_Init
	DMA1_Channel3->CCR = 0; //off stream
	DMA1_Channel3->CPAR = (uint32_t)&(SPI1->DR);
	DMA1_Channel3->CMAR = (uint32_t)&halWS2812_DMA_buf;
	DMA1_Channel3->CNDTR = 0;
	DMA1_Channel3->CCR =
		DMA_CCR_EN * 0 | // EN: Stream enable | 1: Stream enabled
		DMA_CCR_TCIE * 1 | //Transfer interrupt enable
		DMA_CCR_HTIE * 0 | //Half transfer interrupt enable
		DMA_CCR_TEIE * 0 | //Error interrupt enable

		//LSHIFT(1, 10) + // MINC: Memory increment mode | 1: Memory address pointer is incremented after each data transfer (increment is done according to MSIZE)
		  // PINC: Peripheral increment mode | 0: Peripheral address pointer is fixed
		DMA_CCR_DIR * 1 |  // DIR: Data transfer direction | 1: Memory-to-peripheral
		DMA_CCR_CIRC * 0 | // CIRC: Circular mode | 1: Circular mode enabled
		DMA_CCR_PINC * 0 | //
		DMA_CCR_MINC * 1 |
		DMA_CCR_PSIZE_0 * 1 |// PSIZE[1:0]: Peripheral data size | 01: 16-bit
		DMA_CCR_PSIZE_1 * 0 |

		DMA_CCR_MSIZE_0 * 0 | // MSIZE[1:0]: Memory data size | 01: 16-bit
		DMA_CCR_MSIZE_1 * 0 | //TODO почему только 8-бит???

		DMA_CCR_PL_0 * 0 | // PL[1:0]: Priority level | 11: Very high.  PL[1:0]: Priority level
		DMA_CCR_PL_1 * 1 |
		DMA_CCR_MEM2MEM * 0;
    
	NVIC_EnableIRQ (DMA1_Channel3_IRQn);
#endif

#ifdef WS2812_SPI2
    // Enable clocks
	RCC->APB2ENR |= (RCC_APB2ENR_IOPBEN);
	RCC->APB1ENR |= (RCC_APB1ENR_SPI2EN);

	// Configure SPI pin: MOSI with default alternate function push-pull
	GPIOB->CRH &= ~(GPIO_CRH_CNF15); //clear
	GPIOB->CRH |= GPIO_CRH_CNF15_1; //PB15(MOSI) - AF, Output, PP
	GPIOB->CRH |=  GPIO_CRH_MODE15; //OUT MOSI - 50MHz

	// Set SPI1
	SPI2->CR1 =
		SPI_CR1_CPHA * 0 |		// Clock Phase
		SPI_CR1_CPOL * 0 |		// Clock Polarity
		SPI_CR1_MSTR * 1 |		// Master Selection
		SPI_CR1_BR_0 * 0 |		// Baud Rate Control - fpclk2 / 2 / 8 = 0.75 MHz
		SPI_CR1_BR_1 * 1 |		//
		SPI_CR1_BR_2 * 0 |		//
		SPI_CR1_SPE * 0 |		// SPI Enable
		SPI_CR1_LSBFIRST * 0 |		// Frame Format
		SPI_CR1_SSI * 1 |		// Internal slave select
		SPI_CR1_SSM * 1 |		// Software slave management
		SPI_CR1_RXONLY * 0 |		// Receive only
		SPI_CR1_DFF * 0 |		// Data Frame Format
		SPI_CR1_CRCNEXT * 0 |		// Transmit CRC next
		SPI_CR1_CRCEN * 0 |		// Hardware CRC calculation enable
		SPI_CR1_BIDIOE * 1 |		// Output enable in bidirectional mode
		SPI_CR1_BIDIMODE * 1;		// Bidirectional data mode enable

	SPI2->CR2 =
		SPI_CR2_RXDMAEN * 0 |		// Rx Buffer DMA Enable
		SPI_CR2_TXDMAEN * 1 |		// Tx Buffer DMA Enable
		SPI_CR2_SSOE * 0 |		// SS Output Enable
		//SPI_CR2_FRF * 0 |	// Protocol format - 0: SPI Motorola mode, 1: SPI TI mode
		SPI_CR2_ERRIE * 0 |		// Error Interrupt Enable
		SPI_CR2_RXNEIE * 0 |		// RX buffer Not Empty Interrupt Enable
		SPI_CR2_TXEIE * 0;		// Tx buffer Empty Interrupt Enable

	SPI2->CR1 |= SPI_CR1_SPE;	// Разрешить, Мастер

	RCC->AHBENR |= RCC_AHBENR_DMA1EN;  // On DMA1

	//DMA_Init
	DMA1_Channel5->CCR = 0; //off stream
	DMA1_Channel5->CPAR = (uint32_t)&(SPI2->DR);
	DMA1_Channel5->CMAR = (uint32_t)&halWS2812_DMA_buf;
	DMA1_Channel5->CNDTR = 0;
	DMA1_Channel5->CCR =
		DMA_CCR_EN * 0 | // EN: Stream enable | 1: Stream enabled
		DMA_CCR_TCIE * 1 | //Transfer interrupt enable
		DMA_CCR_HTIE * 0 | //Half transfer interrupt enable
		DMA_CCR_TEIE * 0 | //Error interrupt enable

		//LSHIFT(1, 10) + // MINC: Memory increment mode | 1: Memory address pointer is incremented after each data transfer (increment is done according to MSIZE)
		  // PINC: Peripheral increment mode | 0: Peripheral address pointer is fixed
		DMA_CCR_DIR * 1 |  // DIR: Data transfer direction | 1: Memory-to-peripheral
		DMA_CCR_CIRC * 0 | // CIRC: Circular mode | 1: Circular mode enabled
		DMA_CCR_PINC * 0 | //
		DMA_CCR_MINC * 1 |
		DMA_CCR_PSIZE_0 * 1 |// PSIZE[1:0]: Peripheral data size | 01: 16-bit
		DMA_CCR_PSIZE_1 * 0 |

		DMA_CCR_MSIZE_0 * 0 | // MSIZE[1:0]: Memory data size | 01: 16-bit
		DMA_CCR_MSIZE_1 * 0 | //TODO почему только 8-бит???

		DMA_CCR_PL_0 * 0 | // PL[1:0]: Priority level | 11: Very high.  PL[1:0]: Priority level
		DMA_CCR_PL_1 * 1 |
		DMA_CCR_MEM2MEM * 0;

	NVIC_EnableIRQ (DMA1_Channel5_IRQn);
#endif
    
#ifdef WS2812_UART1
    // Enable clocks
	RCC->APB2ENR |= (RCC_APB2ENR_IOPBEN);
	RCC->APB1ENR |= (RCC_APB1ENR_SPI2EN);

	// Configure SPI pin: MOSI with default alternate function push-pull
	GPIOB->CRH &= ~(GPIO_CRH_CNF15); //clear
	GPIOB->CRH |= GPIO_CRH_CNF15_1; //PB15(MOSI) - AF, Output, PP
	GPIOB->CRH |=  GPIO_CRH_MODE15; //OUT MOSI - 50MHz

	// Set SPI1
	SPI2->CR1 =
		SPI_CR1_CPHA * 0 |		// Clock Phase
		SPI_CR1_CPOL * 0 |		// Clock Polarity
		SPI_CR1_MSTR * 1 |		// Master Selection
		SPI_CR1_BR_0 * 0 |		// Baud Rate Control - fpclk2 / 2 / 8 = 0.75 MHz
		SPI_CR1_BR_1 * 1 |		//
		SPI_CR1_BR_2 * 0 |		//
		SPI_CR1_SPE * 0 |		// SPI Enable
		SPI_CR1_LSBFIRST * 0 |		// Frame Format
		SPI_CR1_SSI * 1 |		// Internal slave select
		SPI_CR1_SSM * 1 |		// Software slave management
		SPI_CR1_RXONLY * 0 |		// Receive only
		SPI_CR1_DFF * 0 |		// Data Frame Format
		SPI_CR1_CRCNEXT * 0 |		// Transmit CRC next
		SPI_CR1_CRCEN * 0 |		// Hardware CRC calculation enable
		SPI_CR1_BIDIOE * 1 |		// Output enable in bidirectional mode
		SPI_CR1_BIDIMODE * 1;		// Bidirectional data mode enable

	SPI2->CR2 =
		SPI_CR2_RXDMAEN * 0 |		// Rx Buffer DMA Enable
		SPI_CR2_TXDMAEN * 1 |		// Tx Buffer DMA Enable
		SPI_CR2_SSOE * 0 |		// SS Output Enable
		//SPI_CR2_FRF * 0 |	// Protocol format - 0: SPI Motorola mode, 1: SPI TI mode
		SPI_CR2_ERRIE * 0 |		// Error Interrupt Enable
		SPI_CR2_RXNEIE * 0 |		// RX buffer Not Empty Interrupt Enable
		SPI_CR2_TXEIE * 0;		// Tx buffer Empty Interrupt Enable

	SPI2->CR1 |= SPI_CR1_SPE;	// Разрешить, Мастер

	RCC->AHBENR |= RCC_AHBENR_DMA1EN;  // On DMA1

	//DMA_Init
	DMA1_Channel5->CCR = 0; //off stream
	DMA1_Channel5->CPAR = (uint32_t)&(UART1->DR);
	DMA1_Channel5->CMAR = (uint32_t)&halWS2812_DMA_buf;
	DMA1_Channel5->CNDTR = 0;
	DMA1_Channel5->CCR =
		DMA_CCR_EN * 0 | // EN: Stream enable | 1: Stream enabled
		DMA_CCR_TCIE * 1 | //Transfer interrupt enable
		DMA_CCR_HTIE * 0 | //Half transfer interrupt enable
		DMA_CCR_TEIE * 0 | //Error interrupt enable

		//LSHIFT(1, 10) + // MINC: Memory increment mode | 1: Memory address pointer is incremented after each data transfer (increment is done according to MSIZE)
		  // PINC: Peripheral increment mode | 0: Peripheral address pointer is fixed
		DMA_CCR_DIR * 1 |  // DIR: Data transfer direction | 1: Memory-to-peripheral
		DMA_CCR_CIRC * 0 | // CIRC: Circular mode | 1: Circular mode enabled
		DMA_CCR_PINC * 0 | //
		DMA_CCR_MINC * 1 |
		DMA_CCR_PSIZE_0 * 1 |// PSIZE[1:0]: Peripheral data size | 01: 16-bit
		DMA_CCR_PSIZE_1 * 0 |

		DMA_CCR_MSIZE_0 * 0 | // MSIZE[1:0]: Memory data size | 01: 16-bit
		DMA_CCR_MSIZE_1 * 0 | //TODO почему только 8-бит???

		DMA_CCR_PL_0 * 0 | // PL[1:0]: Priority level | 11: Very high.  PL[1:0]: Priority level
		DMA_CCR_PL_1 * 1 |
		DMA_CCR_MEM2MEM * 0;

	NVIC_EnableIRQ (DMA1_Channel5_IRQn);
#endif
    
    halWS2812_clear_all ();
    halWS2812_send_pixels ();
}


void halWS2812_clear_all (void)
{
    uint32_t i;
    
    for (i = 0; i < WS2812_LEDS_NUM; i++)
    {
        halWS2812_set_color (i, COLOR_BLACK);
    }
}


/**
 * Send pixel 2 all pixel data
 */
void halWS2812_send_pixels (void)
{
    halWS2812_DMAComplete = 0;
#ifdef WS2812_SPI1
    DMA1_Channel3->CNDTR = WS2812_DMA_SIZE; //reinit count
	DMA1_Channel3->CCR |= (uint16_t)(DMA_CCR_EN); // start!
#endif

#ifdef WS2812_SPI2
    DMA1_Channel5->CNDTR = WS2812_DMA_SIZE; //reinit count
	DMA1_Channel5->CCR |= (uint16_t)(DMA_CCR_EN); // start!
#endif

#ifdef WS2812_UART1
    DMA1_Channel5->CNDTR = WS2812_DMA_SIZE; //reinit count
	DMA1_Channel5->CCR |= (uint16_t)(DMA_CCR_EN); // start!
#endif
    while (halWS2812_DMAComplete == 0) {};
}


/**
 *  Set massive for DMA
 */
msg_t halWS2812_set_color (uint16_t num, uint32_t color)
{
	uint32_t i;

	if (WS2812_LEDS_NUM > num)
	{
		// make to 0x00RRGGBB from 0x00GGRRBB
		uint32_t tmp32;
		tmp32 = (color << 8) & 0x00FF0000; //GG
		tmp32 |= color & 0x000000FF; //BB
		tmp32 |= (color >> 8) & 0x0000FF00; //RR
        
        
		for (i = 0; i < 6; i++)
		{
			if (0 != (tmp32 & 0x00800000))
			{
				halWS2812_DMA_buf [num * 3 * (8/4) + i] = 0xC000;
			}
			else
			{
				halWS2812_DMA_buf [num * 3 * (8/4) + i] = 0x8000;
			}

			if (0 != (tmp32 & 0x00400000))
			{
				halWS2812_DMA_buf [num * 3 * (8/4) + i] |= 0x0C00;
			}
			else
			{
				halWS2812_DMA_buf [num * 3 * (8/4) + i] |= 0x0800;
			}

			if (0 != (tmp32 & 0x00200000))
			{
				halWS2812_DMA_buf [num * 3 * (8/4) + i] |= 0x00C0;
			}
			else
			{
				halWS2812_DMA_buf [num * 3 * (8/4) + i] |= 0x0080;
			}

			if (0 != (tmp32 & 0x00100000))
			{
				halWS2812_DMA_buf [num * 3 * (8/4) + i] |= 0x000C;
			}
			else
			{
				halWS2812_DMA_buf [num * 3 * (8/4) + i] |= 0x0008;
			}

			tmp32 = tmp32 << 4;
		}
		return FUNCTION_RETURN_OK;
	}
	return FUNCTION_RETURN_ERROR;
}
