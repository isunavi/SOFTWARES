/**
 * @file    drvWS2812.h
 * @author  ZAY_079
 * @date
 * @version V1.0.0
 * @brief



 https://geektimes.ru/post/258046/
 http://catethysis.ru/stm32_dma/
 https://geektimes.ru/post/255548/
 https://github.com/g4lvanix/0xWS2812/blob/master/main.c
 http://digitrode.ru/articles/58-stm32-upravlyaet-rgb-svetodiodami-ws2812.html
 http://we.easyelectronics.ru/STM32/dac-v-stm32.html
 http://we.easyelectronics.ru/STM32/stm32-primery-koda.html
 https://habrahabr.ru/post/276605/
 http://www.getchip.net/posts/119-umnye-svetodiody-ws2812b-neopixels/
 https://wp.josh.com/2014/05/13/ws2812-neopixels-are-not-so-finicky-once-you-get-to-know-them/


board.h->
// for drwWS2812.h
//#define WS2812_SPI1	    	1 //USE SPI1 & DMA1 ch3
#define WS2812_SPI2				1 //USE SPI2 & DMA1 ch5
#define WS2812_LEDS_NUM 		    8 // leds all number (max 1000 or memory size (1 leds - 12 bytes)

main.c->
    _srand (); // init random generator

    halWS2812_init ();
    halWS2812_set_color (0, WS2812_COLOR_RED); //red
    halWS2812_set_color (1, WS2812_COLOR_GREEN); //green
    halWS2812_set_color (2, WS2812_COLOR_BLUE); //blue
    uint32_t i;
    __EI(); //start all IRQ!
    while (1)
	{
		halWS2812_send_pixels (); //set all leds
    	for (i = 0; i < LEDS_NUM; i++)
    	{
    		halWS2812_set_color (i, _rand32 ()); //set random color
    	}
        _delay_ms(300); //wait
	}

 */


#ifndef DRVWS2812_H
#define	DRVWS2812_H 20161020

#include "board.h"
// цвета в формате RGB (00000000 RRRRRRRR GGGGGGGG BBBBBBBB)
#include "colors.h"

#define WS2812_COLORS              3 //RGB leds :)

#if (WS2812_SPI1 || WS2812_SPI2)
#define WS2812_DMA_SIZE     ((WS2812_LEDS_NUM + 2) * ((WS2812_COLORS * 8) / 2)) //+1 - dymmy reset, *2 - in bytes
#endif
#if (WS2812_USART1)
#define WS2812_DMA_SIZE     ((WS2812_LEDS_NUM + 2) * ((WS2812_COLORS * 8) / 2)) //+1 - dymmy reset, *2 - in bytes
#endif
#if (WS2812_TIM1)
#define WS2812_DMA_SIZE     ((WS2812_LEDS_NUM + 2) * (WS2812_COLORS * 8)) //+1 - dymmy reset, *2 - in bytes
#endif


#ifdef	__cplusplus
extern "C" {
#endif


/**
 * Init perifirial (SPI & DMA)
 */
void halWS2812_init (void);


/**
 * Send pixel 2 all pixel data
 */
void halWS2812_send_pixels (void);


/**
 * Clear all leds (in memory)
 */
void halWS2812_clear_all (void);


/**
 *  Set massive for DMA
 */
msg_t halWS2812_set_color (uint16_t number, uint32_t color);


#ifdef	__cplusplus
}
#endif

#endif	/** DRVWS2812_H */
