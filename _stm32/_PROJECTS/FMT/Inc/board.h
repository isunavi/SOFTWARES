/* 
 * File:   board.h

 */

#ifndef BOARD_H
#define	BOARD_H 20160809


#define BOARD_FMT_V1            (1)//целевая плата
#define STM32                   1//for debug.h


//#define HSE_VALUE                   ((uint32_t)6000000UL)
//#define STM32F10X_MD               
//#define CPU_VALUE                 12000000UL
//#define STM32                       1 //процессор\ядро
//#define USE_STDPERIPH_DRIVER


/**
 *  Раздел для "include"
 */
#include "stm32f4xx_hal.h"
#include "fatfs.h"
#include "usb_device.h"


/**
 *  Раздел для "define"
 */
#ifndef bool_t
typedef int8_t				bool_t;
#endif
//#define bool_t          uint8_t 
#ifndef msg_t
    #define msg_t           uint32_t
#endif
#define FUNCTION_RETURN_OK          0
#define FUNCTION_RETURN_ERROR       1

#define ERROR_ACTION(a)              //fERROR_ACTION(a,__MODULE__,__LINE__)

#define systime_t                   uint32_t
#define TIC_PERIOD                  (1000UL) //us

#ifndef TRUE
#define TRUE    -1
#endif
#ifndef FALSE
#define FALSE    0
#endif




// PB7  - LED_GREEN
// GPIO ------------------------------------------
// #define LED_GREEN  GPIO_Pin_7

// #define LED_GREEN_OFF   GPIOB->BRR  = LED_GREEN
// #define LED_GREEN_ON    GPIOB->BSRR = LED_GREEN
// #define LED_GREEN_INV   GPIOB->ODR ^= LED_GREEN

#define GPIO_SPIFLASH_CS_L      GPIOC->BSRR = (uint32_t)GPIO_SPI_25DF_CS_Pin << 16U;
#define GPIO_SPIFLASH_CS_H      GPIOC->BSRR = GPIO_SPI_25DF_CS_Pin;



//------------------------------- UART -----------------------------------------
#define HAL_USART1                  1
#define USART1_TX_HOOK              do {Uart1TX_Ready = SET;} while (0)
#define USART1_RX_HOOK              do {Uart1RX_Ready = SET; ; } while (0)
#define USART1_TX_ERROR_HOOK        ERROR_ACTION(ERROR_USART1_HAL)
#define USART1_BUF_TX_SIZE          (1024)
#define USART1_BUF_RX_SIZE          (1024)
#define USART1_LED_INV              //LED_BLUE_INV 



#define HAL_SPI1                        1



// for xprintf.h
#define _USE_XFUNC_OUT	            1 // 1: Use output functions
#define	_CR_CRLF		            0 // 1: Convert \n ==> \r\n in the output char
#define _USE_XFUNC_IN	            0 // 1: Use input function
#define	_LINE_ECHO		            0 // 1: Echo back input chars in xgets function

//for conv.h
#define STR_MAX_SIZE        (65535 -1)

// for modRTC.h
//#define MOD_RTC                     1
//#define RTC_CORRECT_TIC             1
//#define RTC_NEED_CALENDAR           1
//#define RTC_NEED_ALARM              1

// vor _crc.h
//#define NEED_CRC32_CCITT        1 
#define NEED_CRC16              1
//#define NEED_CRC8               1
//#define NEED_CRC8_DS            1


//#define __NOP()               
#define __DI()                  __disable_irq() // do { #asm("cli") } while(0) // Global interrupt disable
#define __EI()                  __enable_irq() //do { #asm("sei") } while(0) // Global interrupt enable
#define __CLRWDT()              do { IWDG->KR = 0x0000AAAA; } while (0) // перезагрузка вачдоге


    
#endif	/* BOARD_H */
