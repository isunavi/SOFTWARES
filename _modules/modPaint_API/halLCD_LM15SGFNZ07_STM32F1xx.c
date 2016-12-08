
// https://github.com/adafruit/Adafruit-PCD8544-Nokia-5110-LCD-library/blob/master/Adafruit_PCD8544.h
// https://github.com/DhrBaksteen/Arduino-SPI-LM15SGFNZ07-LCD-Library/blob/master/LM15SGFNZ07.h
#include "halPaint.h"
#include "board.h"
#include "modPaint.h"
#include "modPaint_local.h"
#include "colors.h"
#include <string.h> // for memset


#if (LCD_LM15SGFN07)

// структура параметров дисплея
struct halLCD_lcdStruct_t {
	uint8_t       orientation; // Ориентация
    color_t       color;
} halLCD_lcdStruct;


// некалиброванная задержка для процедуры инициализации LCD
void halLCD_delay (uint32_t delay) {
    delay = delay * 1000UL;
    while (delay){ delay--; }
}


void halLCD_setColor (color_t color) {
    halLCD_lcdStruct.color = color;
}

/*
const unsigned char initData[139]  =    // LCD Inicialization data
{
    0xF4 , 0x90 , 0xB3 , 0xA0 , 0xD0,  0xF0 , 0xE2 , 0xD4 , 0x70 , 0x66,
    0xB2 , 0xBA , 0xA1 , 0xA3 , 0xAB , 0x94 , 0x95 , 0x95 , 0x95 , 0xF5,
    0x90,  0xF1 , 0x00 , 0x10 , 0x22 , 0x30 , 0x45 , 0x50 , 0x68 , 0x70,
    0x8A , 0x90 , 0xAC , 0xB0 , 0xCE , 0xD0,  0xF2 , 0x0F , 0x10 , 0x20,
    0x30 , 0x43 , 0x50 , 0x66 , 0x70 , 0x89 , 0x90 , 0xAB , 0xB0 , 0xCD,
    0xD0,  0xF3 , 0x0E , 0x10 , 0x2F , 0x30 , 0x40 , 0x50 , 0x64 , 0x70,
    0x87 , 0x90 , 0xAA , 0xB0 , 0xCB , 0xD0,  0xF4 , 0x0D , 0x10 , 0x2E,
    0x30 , 0x4F , 0x50,  0xF5 , 0x91,  0xF1 , 0x01 , 0x11 , 0x22 , 0x31,
    0x43 , 0x51 , 0x64 , 0x71 , 0x86 , 0x91 , 0xA8 , 0xB1 , 0xCB , 0xD1,
    0xF2 , 0x0F , 0x11 , 0x21 , 0x31 , 0x42 , 0x51 , 0x63 , 0x71 , 0x85,
    0x91 , 0xA6 , 0xB1 , 0xC8 , 0xD1,  0xF3 , 0x0B , 0x11 , 0x2F , 0x31,
    0x41 , 0x51 , 0x62 , 0x71 , 0x83 , 0x91 , 0xA4 , 0xB1 , 0xC6 , 0xD1,
    0xF4 , 0x08 , 0x11 , 0x2B , 0x31 , 0x4F , 0x51 , 0x80 , 0x94,  0xF5,
    0xA2 , 0xF4 , 0x60 , 0xF0 , 0x40 , 0x50 , 0xC0 , 0xF4 , 0x70
};
*/
const unsigned char initData[134]  =
{ /*0xF4, 0x90, 0xB3, 0xA0, 0xD0,*/
    0xF0, 0xE2, 0xD4, 0x70, 0x66, 0xB2, 0xBA, 0xA1, 0xA3, 0xAB, 0x94, 0x95,
    0x95, 0x95, 0xF5, 0x90, 0xF1, 0x00, 0x10, 0x22, 0x30, 0x45, 0x50, 0x68,
    0x70, 0x8A, 0x90, 0xAC, 0xB0, 0xCE, 0xD0, 0xF2, 0x0F, 0x10, 0x20, 0x30,
    0x43, 0x50, 0x66, 0x70, 0x89, 0x90, 0xAB, 0xB0, 0xCD, 0xD0, 0xF3, 0x0E,
    0x10, 0x2F, 0x30, 0x40, 0x50, 0x64, 0x70, 0x87, 0x90, 0xAA, 0xB0, 0xCB,
    0xD0, 0xF4, 0x0D, 0x10, 0x2E, 0x30, 0x4F, 0x50, 0xF5, 0x91, 0xF1, 0x01,
    0x11, 0x22, 0x31, 0x43, 0x51, 0x64, 0x71, 0x86, 0x91, 0xA8, 0xB1, 0xCB,
    0xD1, 0xF2, 0x0F, 0x11, 0x21, 0x31, 0x42, 0x51, 0x63, 0x71, 0x85, 0x91,
    0xA6, 0xB1, 0xC8, 0xD1, 0xF3, 0x0B, 0x11, 0x2F, 0x31, 0x41, 0x51, 0x62,
    0x71, 0x83, 0x91, 0xA4, 0xB1, 0xC6, 0xD1, 0xF4, 0x08, 0x11, 0x2B, 0x31,
    0x4F, 0x51, 0x80, 0x94, 0xF5, 0xA2, 0xF4, 0x60, 0xF0, 0x40, 0x50, 0xC0,
    0xF4, 0x70
}; 

//-------------------------------------------------
//inline 
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
        if (0x80 & byte)
        {
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


void _LCD_setContrast (uint8_t contrast)
{
    LCD_RS_H;
    LCD_CS_L;
    _LCD_spiWrite (0xF4);
    _LCD_spiWrite (0xB0 | (contrast >> 4));
    _LCD_spiWrite (0xA0 | (contrast & 0x0F));
    LCD_CS_H;
}


void hal_paint_init (uint8_t mode)
{
    // Setup software SPI.
    GPIO_InitTypeDef GPIO_InitStructure;
    uint32_t i;

    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pin : PB2 */
    GPIO_InitStructure.Pin = (GPIO_PIN_LCD_RESET |
        GPIO_PIN_LCD_RS |
        GPIO_PIN_LCD_CS |
        GPIO_PIN_LCD_CLK |
        GPIO_PIN_LCD_DAT);
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init (GPIOA, &GPIO_InitStructure);

  
    LCD_CS_H;
    LCD_RESET_H;
    LCD_RS_L;
    LCD_RESET_L;   
    halLCD_delay (100);
    LCD_RESET_H;
    halLCD_delay (100);
    LCD_RS_H;
    LCD_CS_L;

   for (i = 0; i < 134; i++) //139
   {
	   _LCD_spiWrite (initData [i]);   // Sends bytes to LCD
   }
    halLCD_delay (10);
    LCD_CS_H;

    halLCD_delay (10);
    LCD_CS_L;
    
    _LCD_spiWrite (0xF0);
    _LCD_spiWrite (0x81);
    _LCD_spiWrite (0xF4);
    _LCD_spiWrite (0xB3);
    _LCD_spiWrite (0xA0);

    _LCD_spiWrite (0xF0);
    _LCD_spiWrite (0x06);
    _LCD_spiWrite (0x10);
    _LCD_spiWrite (0x20);
    _LCD_spiWrite (0x30);
    _LCD_spiWrite (0xF5);
    _LCD_spiWrite (0x0F);
    _LCD_spiWrite (0x1C);
    _LCD_spiWrite (0x2F);
    _LCD_spiWrite (0x34);
    
    _LCD_spiWrite (0xf4);
    _LCD_spiWrite (0x94);
    _LCD_spiWrite (0xb3);
    _LCD_spiWrite (0xa2);
    _LCD_spiWrite (0xd0);
    
    LCD_CS_H;
    LCD_RS_L;
    
    _LCD_setContrast (43);
}


//***************************************************************
//LCD window
//***************************************************************
void LM15SGFNZ07_window_lcd (coord_t xstart,
                             coord_t ystart,
                             coord_t xend,
                             coord_t yend)
{
    xstart <<= 1;
    xstart += 6;
    xend <<= 1;
    xend += 7;

    LCD_RS_H;
    LCD_CS_L;
    _LCD_spiWrite (0xF0);
    _LCD_spiWrite (0x00 | (xstart & 0x0F));
    _LCD_spiWrite (0x10 | (xstart >> 4));
    _LCD_spiWrite (0x20 | (ystart & 0x0F));
    _LCD_spiWrite (0x30 | (ystart >> 4));
    _LCD_spiWrite (0xF5);
    _LCD_spiWrite (0x00 | (xend & 0x0F));
    _LCD_spiWrite (0x10 | (xend >> 4));
    _LCD_spiWrite (0x20 | (yend & 0x0F));
    _LCD_spiWrite (0x30 | (yend >> 4));
    LCD_CS_H;
    LCD_RS_L;

}


//***************************************************************
//Setting the cursor
//***************************************************************
void LM15SGFNZ07_cursor_lcd (coord_t row, coord_t col)
{
   LM15SGFNZ07_window_lcd (row,col, 100, 79);
}


void     hal_paint_setPixelColor (coord_t x, coord_t y, color_t color)
{
   LM15SGFNZ07_cursor_lcd (x, 79-y);
   LCD_RS_L;
   LCD_CS_L;
   //SPI1_16bit;
   _LCD_spiWrite ((color & 0x0FFF) >> 8 );
   _LCD_spiWrite (color & 0x00FF);
   //SPI1_8bit;
   LCD_CS_H;
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
    unsigned int i;

   LM15SGFNZ07_cursor_lcd (0,0);
   LCD_RS_L;
   LCD_CS_L;
   for (i = 0; i < 8080; i++)
   {
      _LCD_spiWrite (color >> 8);
      _LCD_spiWrite (color & 0x00FF);
   }
   LM15SGFNZ07_cursor_lcd (0,0);
   LCD_CS_H;
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
