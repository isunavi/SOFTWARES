/**
 * http://cxem.net/mc/mc221.php
 * http://cxem.net/mc/mc255.php
https://geektimes.ru/post/255436/
 */

#include "halPaint.h"
#include "board.h"
#include "modPaint.h"
#include "modPaint_local.h"
#include "colors.h"
#include <string.h> // for memset


#if (LCD_LPH9157)

// структура параметров дисплея
struct halLCD_lcdStruct_t {
	uint8_t       orientation; // Ориентация
    color_t       color;
} halLCD_lcdStruct;


// некалиброванная задержка для процедуры инициализации LCD
void halLCD_delay (uint32_t delay)
{
    delay = delay * 1000UL;
    while (delay){ delay--; }
}


void halLCD_setColor (color_t color)
{
    halLCD_lcdStruct.color = color;
}


void _LCD_setContrast (uint8_t contrast)
{

}


void _LCD_spiWrite (uint8_t byte)
{
//#ifdef TRUE //__DEBUG
    uint8_t bit;
    // for(uint8_t bit = 0x80; bit; bit >>= 1) {
    for (bit = 0x80; bit; bit = bit >> 1)
    {
        __NOP();
        //__NOP();
        LCD_CLK_L;
        __NOP();
        //__NOP();
        if (0x80 & byte) {
            LCD_DAT_H;
        } else {
            LCD_DAT_L;
        }
        byte = byte << 1;
        //halLCD_delay (1);
        __NOP();
        //__NOP();
        LCD_CLK_H;
        __NOP();
        //__NOP();
        //if (PORTGbits.RG7) byte |= 0x01;
        //halLCD_delay (1);
    }
}

//Команда/Данные
#define CMD 0
#define DAT 1
char RS_old;


 //===============================================================
 //Функция записи команды/данных в LCD (RS==0 - команда, RS==1 - данные)
 //===============================================================
void Send_to_lcd (unsigned char RS, unsigned char data)
{
    LCD_CLK_L;
    LCD_DAT_L;
    if ((RS_old != RS) || (!RS_old && !RS)) //проверяем старое значение RS (если поступают одни команды то дергаем CS)
    {
        LCD_CS_H;
        if(RS) 
            LCD_RS_H
        else 
            LCD_RS_L;
        LCD_CS_L;
    }
    
    _LCD_spiWrite (data);
    /*
    LCD_DAT_L;
    if ((data & 128) == 128) LCD_DAT_H;
    LCD_CLK_H;
    LCD_CLK_L;
    LCD_DAT_L;
    if ((data & 64) == 64)  LCD_DAT_H;
    LCD_CLK_H;
    LCD_CLK_L;
    LCD_DAT_L;
    if ((data & 32) == 32)  LCD_DAT_H;
    LCD_CLK_H;
    LCD_CLK_L;
    LCD_DAT_L;
    if ((data & 16) ==16)  LCD_DAT_H;
    LCD_CLK_H;
    LCD_CLK_L;
    LCD_DAT_L;
    if ((data & 8) == 8)  LCD_DAT_H;
    LCD_CLK_H;
    LCD_CLK_L;
    LCD_DAT_L;
    if ((data & 4) == 4)  LCD_DAT_H;
    LCD_CLK_H;
    LCD_CLK_L;
    LCD_DAT_L;
    if ((data & 2) == 2)  LCD_DAT_H;
    LCD_CLK_H;
    LCD_CLK_L;
    LCD_DAT_L;
    if ((data & 1) == 1)  LCD_DAT_H;
    LCD_CLK_H;
    LCD_CLK_L;
    */
    
    RS_old=RS;  //запоминаю значение RS
    LCD_DAT_L;
}


void hal_paint_init (uint8_t mode)
{
    // Setup software SPI.
    GPIO_InitTypeDef GPIO_InitStructure;
    uint32_t i;

    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin : PB2 */
    GPIO_InitStructure.Pin = (GPIO_PIN_LCD_RESET |
        GPIO_PIN_LCD_RS |
        GPIO_PIN_LCD_CS |
        GPIO_PIN_LCD_CLK |
        GPIO_PIN_LCD_DAT);
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init (GPIOB, &GPIO_InitStructure);

  
    LCD_RESET_L;
  halLCD_delay (1000);
  LCD_RESET_H;
  halLCD_delay (1000);
  Send_to_lcd(0, 0x01); //Программный сброс
  Send_to_lcd(0, 0x36); //Memory Access Control (Направление заполнения области дисплея (памяти): 0bVHRXXXXX, V - заполнение по вертикали (0 - сверху-вниз, 1 - снизу-вверх),
                          //H - заполнение по горизонтали (0 - слева-направо, 1 - справа-налево), R - меняются местами строки и столбцы (при этом заполнение остается сверху-вниз, слева-направо))
  Send_to_lcd(1, 0x00);
  Send_to_lcd(0, 0x11); //Выход из спящего режима
  halLCD_delay (200);
  Send_to_lcd(0, 0x3a); //Установка цветовой палитры
  #ifdef _8_BIT_COLOR
  Send_to_lcd(1, 0x02); //Байт на пиксель 256 цветов
  #else
  Send_to_lcd(1, 0x05); //Два байта на пиксель 65536 цветов
  #endif
  halLCD_delay (200);
  Send_to_lcd(0, 0x29); //Включение дисплея
}


void SetArea (coord_t x1, coord_t x2, coord_t y1, coord_t y2)
{
    Send_to_lcd ( 0, 0x2A );  //задаем область по X
    Send_to_lcd ( 1, x1 );    //начальная
    Send_to_lcd ( 1, x2 );    //конечная

    Send_to_lcd ( 0, 0x2B );  //задаем область по Y
    Send_to_lcd ( 1, y1 );    //начальная
    Send_to_lcd ( 1, y2 );    //конечная

    Send_to_lcd ( 0, 0x2C );  //отправляем команду на начало записи в память и начинаем посылать данные
}




void     hal_paint_setPixelColor (coord_t x, coord_t y, color_t color)
{
  signed char i;
  SetArea( x, x, y, y );
  LCD_RS_H;

#ifdef _8_BIT_COLOR	//(8-ми битовая цветовая палитра (256 цветов))
    Send_to_lcd( 1, color ); //Данные - задаём цвет пикселя
#else			        //(16-ти битовая цветовая палитра (65536 цветов))
    Send_to_lcd( 1, (color >> 8) );
    Send_to_lcd( 1, color );
#endif
}


void hal_paint_setPixel (coord_t x, coord_t y)
{

}


color_t hal_paint_getPixel (coord_t x, coord_t y)
{
    return COLOR_BLACK;   
}


#if LCD_ENABLE_PARTIAL_UPDATE
uint8_t halLCD_cnt = 0;
#endif
void hal_paint_repaint (void)
{
 
}


void hal_paint_cls (color_t color)
{
    uint32_t x;
    signed char i;
    SetArea( 0, 131, 0, 175 );   //Область всего экрана
    LCD_RS_H;

    //Данные - задаём цвет пикселя
    for (x = 0; x < 23232; x++)  // 23232 - это 132 * 176
    {
        #ifdef _8_BIT_COLOR	//(8-ми битовая цветовая палитра (256 цветов))
        Send_to_lcd( DAT, color ); //Данные - задаём цвет пикселя
        #else			//(16-ти битовая цветовая палитра (65536 цветов))
        Send_to_lcd( DAT, (color >> 8) ); Send_to_lcd( DAT, color );
        #endif
    }
}


void     hal_paint_fillBlock (coord_t x, coord_t y, uint16_t w, uint16_t h, color_t *buf)
{
    coord_t xx, yy;
    
    for (xx = 0; xx < w; xx++)
        for (yy = 0; yy < h; yy++)
            hal_paint_setPixelColor (x + xx, y + yy, *buf++);
}


void     hal_paint_fillBlockColor (coord_t x, coord_t y,
    uint16_t w, uint16_t h,
    color_t color)
{
    coord_t xx, yy;
    
    for (xx = 0; xx < w; xx++)
        for (yy = 0; yy < h; yy++)
            hal_paint_setPixelColor (x + xx, y + yy, color);
}


void halLCD_setOrientation (uint8_t orient)
{
    
}

#endif
