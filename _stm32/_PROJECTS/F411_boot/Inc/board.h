#include "stm32f4xx_hal.h"
#include "usb_device.h"

#define BOARD_STM32F411_REBORN      20160824

#define STM32                       1 // for debug.h
//#define TEST    					0 //Отключает вывод на LCD если 1
#define DEBUG                       1 //for printf

//------------------------------- UART -----------------------------------------
#define HAL_USART1                  1
#define USART1_TX_HOOK              //do {Uart1TX_Ready = SET; } while (0)
#define USART1_RX_HOOK              //do {Uart1RX_Ready = SET; ; } while (0)
#define USART1_TX_ERROR_HOOK        //ERROR_ACTION(ERROR_USART1_HAL)
#define USART1_BUF_TX_SIZE          (128)
#define USART1_BUF_RX_SIZE          (512)
#define USART1_LED_INV              //LED_BLUE_INV


//------------------------------ XSPRINTF --------------------------------------
#define _USE_XFUNC_OUT	            1 // 1: Use output functions
#define	_CR_CRLF		            0 // 1: Convert \n ==> \r\n in the output char
#define _USE_XFUNC_IN	            0 // 1: Use input function
#define	_LINE_ECHO		            0 // 1: Echo back input chars in xgets function

//#define MOD_RTC                     1
//#define RTC_CORRECT_TIC             1
//#define RTC_NEED_CALENDAR           1
//#define RTC_NEED_ALARM              1

#define NEED_CRC32_CCITT        1
#define NEED_CRC16              1
//#define NEED_CRC8               1
//#define NEED_CRC8_DS            1

//#define __NOP()
#define __DI()                  __disable_irq() // do { #asm("cli") } while(0) // Global interrupt disable
#define __EI()                  __enable_irq() //do { #asm("sei") } while(0) // Global interrupt enable
#define __CLRWDT()              do { IWDG->KR = 0x0000AAAA; } while (0) // clear wdt

// ------------------------------ some defines ---------------------------------
#define TRUE                    1
#define FALSE                   0
#define BOOL                    uint8_t

#ifndef msg_t
    #define msg_t           		uint32_t
#endif
#define FUNCTION_RETURN_OK          0
#define FUNCTION_RETURN_ERROR       1

#define ERROR_ACTION(a)              //fERROR_ACTION(a,__MODULE__,__LINE__)

#define systime_t                   uint32_t
#define TIC_PERIOD                  (1000UL) //us
#define bool_t                      uint8_t
//#define NULL                        0


// LEDS ------------------------------------------------------------------------              
#define LED_RED_ON              do { GPIOB->BSRR = (uint32_t)GPIO_PIN_1 << 16U; } while (0)
#define LED_RED_OFF             do { GPIOB->BSRR = GPIO_PIN_1; } while (0)

#define LED_GREEN_ON            do { GPIOB->BSRR = (uint32_t)GPIO_PIN_2 << 16U; } while (0)
#define LED_GREEN_OFF           do { GPIOB->BSRR = GPIO_PIN_2; } while (0)

#define LED_BLUE_ON             do { GPIOB->BSRR = (uint32_t)GPIO_PIN_10 << 16U; } while (0)
#define LED_BLUE_OFF            do { GPIOB->BSRR = GPIO_PIN_10; } while (0)

#define COLOR_RED               0x000000FF
#define COLOR_GREEN             0x0000FF00
#define COLOR_BLUE              0x00FF0000
#define LED_SET_COLOR(a)        //do { halWS2812_set_color (0, a); halWS2812_send_pixels (); } while (0)


// SD CARD --------------------------------------------

// SPI fake
#define PIN_SPI_CS              GPIO_Pin_6
#define PIN_SPI_MOSI            GPIO_Pin_5
#define PIN_SPI_SCK             GPIO_Pin_7
#define PIN_SPI_MISO            GPIO_Pin_4


#define PIN_SPI_CSN_L           GPIOB->ODR &= ~PIN_SPI_CS
#define PIN_SPI_CSN_H           GPIOB->ODR |= PIN_SPI_CS

#define PIN_SPI_MOSI_L          GPIOB->ODR &= ~PIN_SPI_MOSI
#define PIN_SPI_MOSI_H          GPIOB->ODR |= PIN_SPI_MOSI

#define PIN_SPI_SCK_L           GPIOB->ODR &= ~PIN_SPI_SCK
#define PIN_SPI_SCK_H           GPIOB->ODR |= PIN_SPI_SCK

#define PIN_SPI_MISO_I          (GPIOB->IDR & PIN_SPI_MISO)


// SPI HAL
#define CARD_SUPPLY_SWITCHABLE  0
#define GPIO_PWR                GPIO
#define RCC_APB1Periph_GPIO_PWR RCC_APB1Periph_GPIOD
#define GPIO_Pin_PWR            GPIO_Pin_10
#define GPIO_Mode_PWR           GPIO_Mode_Out_OD /* pull-up resistor at power FET */

#define SOCKET_WP_CONNECTED     0 /* write-protect socket-switch */
#define SOCKET_CP_CONNECTED     0 /* card-present socket-switch */
#define GPIO_WP                 GPIO
#define GPIO_CP                 GPIO
#define RCC_APBxPeriph_GPIO_WP  RCC_AHB1Periph_GPIOB
#define RCC_APBxPeriph_GPIO_CP  RCC_APB1Periph_GPIOB
#define GPIO_Pin_WP             GPIO_Pin_
#define GPIO_Pin_CP             GPIO_Pin_
#define GPIO_Mode_WP            GPIO_Mode_IN /* external resistor */
#define GPIO_Mode_CP            GPIO_Mode_IN /* external resistor */

#define GPIO_CS                 GPIOB
#define RCC_AHB1Periph_GPIO_CS  RCC_APB2Periph_GPIOB
#define GPIO_Pin_CS             GPIO_Pin_6

#define SPI_SD                  SPI1
#define GPIO_SPI_SD             GPIOB
#define GPIO_Pin_SPI_SD_SCK     GPIO_Pin_3
#define GPIO_Pin_SPI_SD_MISO    GPIO_Pin_4
#define GPIO_Pin_SPI_SD_MOSI    GPIO_Pin_5
#define RCC_APBPeriphClockCmd_SPI_SD  RCC_APB2PeriphClockCmd
#define RCC_APBPeriph_SPI_SD    RCC_APB2Periph_SPI1

#define DMA_Channel_SPI_SD_RX   DMA1_Channel4
#define DMA_Channel_SPI_SD_TX   DMA1_Channel5
#define DMA_FLAG_SPI_SD_TC_RX   DMA1_FLAG_TC4
#define DMA_FLAG_SPI_SD_TC_TX   DMA1_FLAG_TC5

 /* - for SPI1 and full-speed APB2: 72MHz/4 */
#define SPI_BaudRatePrescaler_SPI_SD  SPI_BaudRatePrescaler_8

#define SELECT()                { __NOP(); delay_spi (1); PIN_SPI_CSN_L; __NOP(); delay_spi (1); } /* CS = L */
#define DESELECT()              { __NOP(); delay_spi (1); PIN_SPI_CSN_H; __NOP(); delay_spi (1);  } /* CS = H */



// for drwWS2812.h
//#define WS2812_SPI1		1 //USE SPI1 & DMA1 ch3
#define WS2812_SPI2		1 //USE SPI2 & DMA1 ch5
#define LEDS_NUM 			1 // leds all number

// флаг обращения к файловой системе
#define DATA_READ_MARKER_HIGH	LED_GREEN_ON
#define DATA_READ_MARKER_LOW	LED_GREEN_OFF


#define GPIO_SPIFLASH_CS_L              do { GPIOD->BSRR = (uint32_t)GPIO_PIN_2 << 16U; } while (0)
#define GPIO_SPIFLASH_CS_H              do { while ((SPI1->SR & SPI_SR_BSY) != 0) {}; GPIOD->BSRR = GPIO_PIN_2; } while (0)

#define GPIO_SPIF_SCK_L                 do { GPIOA->BSRR = (uint32_t)GPIO_PIN_4 << 16U; } while (0)
#define GPIO_SPIF_SCK_H                 do { GPIOA->BSRR = GPIO_PIN_4; } while (0)
#define GPIO_SPIF_MOSI_L                do { GPIOA->BSRR = (uint32_t)GPIO_PIN_6 << 16U; } while (0)
#define GPIO_SPIF_MOSI_H                do { GPIOA->BSRR = GPIO_PIN_6; } while (0)
#define GPIO_SPIF_MISO_IN               (GPIOA->IDR & GPIO_PIN_7)

// 25Q16 constants
#define SPIFLASH_BLOCK_SIZE         (4096UL)//512UL) //4096UL //N25Q128A_SUBSECTOR_SIZE
#define SPIFLASH_BLOCK_NUM          (128UL)
#define SPIFLASH_FLASH_SIZE         (SPIFLASH_BLOCK_SIZE * SPIFLASH_BLOCK_NUM) //4194304UL) 

// for BOOTer
#define FLASH_PAGE_SIZE             2048    //2 Kbyte per page
#define FLASH_START_ADDR            0x08000000					//Origin
#define FLASH_MAX_SIZE              0x00080000					//Max FLASH size - 512 Kbyte
#define FLASH_END_ADDR              (FLASH_START_ADDR + FLASH_MAX_SIZE)		//FLASH end address
#define FLASH_BOOT_START_ADDR       (FLASH_START_ADDR)				//Bootloader start address
#define FLASH_BOOT_SIZE             0x00010000					//64 Kbyte for bootloader
#define FLASH_USER_START_ADDR       (FLASH_BOOT_START_ADDR + FLASH_BOOT_SIZE)	//User application start address
#define FLASH_USER_SIZE             0x00032000					//200 Kbyte for user application
#define FLASH_MSD_START_ADDR        (FLASH_USER_START_ADDR + FLASH_USER_SIZE)	//USB MSD start address
#define FLASH_MSD_SIZE              0x00032000					//200 Kbyte for USB MASS Storage
#define FLASH_OTHER_START_ADDR      (FLASH_MSD_START_ADDR + FLASH_MSD_SIZE)		//Other free memory start address
#define FLASH_OTHER_SIZE            (FLASH_END_ADDR - FLASH_OTHER_START_ADDR)	//Free memory size

