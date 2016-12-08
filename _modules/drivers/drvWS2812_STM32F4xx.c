#include "drvWS2812.h"
#include "board.h"


/** tested on:
1. STM32F411RG
    CPU speed - 96MHz
    FREQ - 728-762KHz (750KHz)
    0, hi ~0.31-0.37us, low ~1us
    1, hi ~0.63-0.69us, low ~0.69us
*/


#if (WS2812_SPI2)
uint16_t halWS2812_DMA_buf [WS2812_DMA_SIZE /2];
#endif

#if (WS2812_TIM1)
uint8_t halWS2812_DMA_buf [WS2812_DMA_SIZE];
#endif


#if WS2812_NEED_WAIT

bool_t halWS2812_DMAComplete = FALSE;


#if WS2812_SPI2
void DMA1_Stream4_IRQHandler (void)
{
    DMA1_Stream4->CR &= (uint16_t)(~DMA_SxCR_EN);
	halWS2812_DMAComplete = TRUE;
	DMA1->HIFCR = DMA_HIFCR_CTCIF4;
}
#endif


#if WS2812_TIM1
void DMA1_Stream2_IRQHandler (void)
{
	DMA1_Stream2->CR &= (uint16_t)(~DMA_SxCR_EN);
	halWS2812_DMAComplete = TRUE;
	DMA1->LIFCR = DMA_LIFCR_CTCIF2;
}
#endif

#endif //WS2812_NEED_WAIT


void halWS2812_init (void)
{
#if (WS2812_SPI1 || WS2812_SPI2)
	halWS2812_DMA_buf [WS2812_DMA_SIZE - 1] = 0x0000;
#endif

#if (WS2812_TIM1)
	halWS2812_DMA_buf [WS2812_DMA_SIZE - 2] = 0x00;
	halWS2812_DMA_buf [WS2812_DMA_SIZE - 1] = 0x00;
#endif
    
#if WS2812_SPI2
    // Enable clocks
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    
	// Configure SPI pin
	GPIOC->MODER  &= ~(GPIO_MODER_MODER3);
    GPIOC->MODER  |= GPIO_MODER_MODER3_1; //Alternate function mode
    GPIOC->OTYPER &= GPIO_OTYPER_OT_3; // Output push-pull
    GPIOC->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR3);
    GPIOC->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR3_1; //50 MHz Fast speed
    GPIOC->PUPDR &= GPIO_PUPDR_PUPDR3; //No pull-up, pull-down
    GPIOC->AFR[0] &= ~(0xF << (3 * 4));
    GPIOC->AFR[0] |= (GPIO_AF5_SPI2 << (3 * 4)); 
                    
    // Set SPI
    SPI2->CR1 = 0;
	SPI2->CR1 =
		SPI_CR1_CPHA * 0 |		// Clock Phase
		SPI_CR1_CPOL * 0 |		// Clock Polarity
		SPI_CR1_MSTR * 1 |		// Master Selection
            
		SPI_CR1_BR_0 * 1 |		// Baud Rate Control - fpclk2 / 128 = 0.75 MHz TODO!!!!!!!!!
		SPI_CR1_BR_1 * 1 |		//
		SPI_CR1_BR_2 * 0 |		//
            
		SPI_CR1_SPE * 0 |		// SPI Enable
		SPI_CR1_LSBFIRST * 0 |  // Frame Format
		SPI_CR1_SSI * 1 |		// Internal slave select
		SPI_CR1_SSM * 1 |		// Software slave management
		SPI_CR1_RXONLY * 0 |		// Receive only
		SPI_CR1_DFF * 0 |		// Data Frame Format
		SPI_CR1_CRCNEXT * 0 |		// Transmit CRC next
		SPI_CR1_CRCEN * 0 |		// Hardware CRC calculation enable
		SPI_CR1_BIDIOE * 1 |		// Output enable in bidirectional mode
		SPI_CR1_BIDIMODE * 1;		// Bidirectional data mode enable

	SPI2->CR2 =
		SPI_CR2_RXDMAEN * 0 |		// Rx Buffer DMA disable
		SPI_CR2_TXDMAEN * 1 |		// Tx Buffer DMA enable
		SPI_CR2_SSOE * 0 |		// SS Output Enable
		//SPI_CR2_FRF * 0 |	// Protocol format - 0: SPI Motorola mode, 1: SPI TI mode
		SPI_CR2_ERRIE * 0 |		// Error Interrupt Enable
		SPI_CR2_RXNEIE * 0 |		// RX buffer Not Empty Interrupt Enable
		SPI_CR2_TXEIE * 0;		// Tx buffer Empty Interrupt Enable

	SPI2->CR1 |= SPI_CR1_SPE;	// Разрешить

	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;  // On DMA1

	//DMA_Init
	DMA1_Stream4->CR = 0; //off stream
	DMA1_Stream4->PAR = (uint32_t)&(SPI2->DR);
	DMA1_Stream4->M0AR = (uint32_t)&halWS2812_DMA_buf; //TODO
	DMA1_Stream4->NDTR = 0;
	DMA1_Stream4->CR =
        DMA_SxCR_CHSEL_0 * 0 | //channel0
        DMA_SxCR_CHSEL_1 * 0 |
        DMA_SxCR_CHSEL_2 * 0 |
            
        DMA_SxCR_MBURST_0 * 0 |
        DMA_SxCR_MBURST_1 * 0 |
            
        DMA_SxCR_PBURST_0 * 0 |
        DMA_SxCR_PBURST_1 * 0 |
            
        DMA_SxCR_CT * 0 | //Current target - memory0
        DMA_SxCR_DBM * 0 | //Double buffer mode

		DMA_SxCR_PL_0 * 0 | // PL[1:0]: Priority level | 11: Very high.  PL[1:0]: Priority level
		DMA_SxCR_PL_1 * 1 |// |  
            
        DMA_SxCR_PINCOS * 0 | //Peripheral increment offset size
            
		DMA_SxCR_MSIZE_0 * 0 | // MSIZE[1:0]: Memory data size | 01: 16-bit
		DMA_SxCR_MSIZE_1 * 0 | //TODO почему только 8-бит???
            
		DMA_SxCR_PSIZE_0 * 0 |// PSIZE[1:0]: Peripheral data size | 01: 16-bit
		DMA_SxCR_PSIZE_1 * 0 |
            
		DMA_SxCR_MINC * 1 | // MINC: Memory increment mode | 1: Memory address pointer is incremented after
        DMA_SxCR_PINC * 0 | //// PINC: Peripheral increment mode | 0: Peripheral address pointer is fixed
		DMA_SxCR_CIRC * 0 | // CIRC: Circular mode disabled
		DMA_SxCR_DIR_0 * 1 |  // DIR: Data transfer direction - Memory-to-peripheral
        DMA_SxCR_DIR_1 * 0 |
        DMA_SxCR_PFCTRL * 0 |
		DMA_SxCR_TCIE * 1 | //Transfer interrupt
		DMA_SxCR_HTIE * 0 | //Half transfer interrupt
		DMA_SxCR_TEIE * 0 | //Error interrupt
        DMA_SxCR_DMEIE * 0 | //Direct mode error interrupt enable
		DMA_SxCR_EN * 0; // EN: Stream enable | 1: Stream enabled
        
#if WS2812_NEED_WAIT
	HAL_NVIC_SetPriority (DMA1_Stream4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ (DMA1_Stream4_IRQn);
#else
	DMA1_Stream4->CR |= DMA_SxCR_CIRC;
	DMA1_Stream4->NDTR = WS2812_DMA_SIZE; //reinit count
	DMA1_Stream4->CR |= (uint16_t)(DMA_SxCR_EN); // start!
#endif

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


void halWS2812_send_pixels (void)
{
#if WS2812_NEED_WAIT
	halWS2812_DMAComplete = FALSE;

#if WS2812_SPI2
    DMA1_Stream4->NDTR = WS2812_DMA_SIZE; //reinit count
	DMA1_Stream4->CR |= (uint16_t)(DMA_SxCR_EN); // start!
#endif

	while (halWS2812_DMAComplete == FALSE) {};
#else
	//do nothing
#endif
}


msg_t halWS2812_set_color (uint16_t number, uint32_t color)
{
	uint32_t i, mask;

	if (WS2812_LEDS_NUM > number) //check number
	{
		// make to 0x00RRGGBB from 0x00GGRRBB
		uint32_t tmp32 = color;

		tmp32 = (color << 8) & 0x00FF0000; //GG
		tmp32 |= color & 0x000000FF; //BB
		tmp32 |= (color >> 8) & 0x0000FF00; //RR

#if (WS2812_SPI1 || WS2812_SPI2 || WS2812_SPI3)
		for (i = 0; i < 6; i++) //load per 4 bits
		{
			if (0 != (tmp32 & 0x00800000))
			{
				halWS2812_DMA_buf [number * 3 * (8/4) + i] = 0xC000;
			}
			else
			{
				halWS2812_DMA_buf [number * 3 * (8/4) + i] = 0x8000;
			}

			if (0 != (tmp32 & 0x00400000))
			{
				halWS2812_DMA_buf [number * 3 * (8/4) + i] |= 0x0C00;
			}
			else
			{
				halWS2812_DMA_buf [number * 3 * (8/4) + i] |= 0x0800;
			}

			if (0 != (tmp32 & 0x00200000))
			{
				halWS2812_DMA_buf [number * 3 * (8/4) + i] |= 0x00C0;
			}
			else
			{
				halWS2812_DMA_buf [number * 3 * (8/4) + i] |= 0x0080;
			}

			if (0 != (tmp32 & 0x00100000))
			{
				halWS2812_DMA_buf [number * 3 * (8/4) + i] |= 0x000C;
			}
			else
			{
				halWS2812_DMA_buf [number * 3 * (8/4) + i] |= 0x0008;
			}

			tmp32 = tmp32 << 4;
		}
#endif

#if (WS2812_TIM1)
		for (i = 0; i < (WS2812_COLORS * 8); i++) //loading
		{
			if (0 != (tmp32 & 0x00100000))
			{
				halWS2812_DMA_buf [number * (WS2812_COLORS * 8) + i] = 17; //TODO
			}
			else
			{
				halWS2812_DMA_buf [number * (WS2812_COLORS * 8) + i] = 9; //TODO
			}
			tmp32 = tmp32 << 1;
		}
#endif
		return FUNCTION_RETURN_OK;
	}
	return FUNCTION_RETURN_ERROR;
}
