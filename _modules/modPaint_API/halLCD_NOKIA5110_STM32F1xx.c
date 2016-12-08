
// https://github.com/adafruit/Adafruit-PCD8544-Nokia-5110-LCD-library/blob/master/Adafruit_PCD8544.h

#include "halPaint.h"
#include "board.h"
#include "modPaint.h"
#include "modPaint_local.h"

#include <string.h> // for memset


#if (LCD_NOKIA5110)

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



// #define BLACK 1
// #define WHITE 0

#define PCD8544_POWERDOWN 0x04
#define PCD8544_ENTRYMODE 0x02
#define PCD8544_EXTENDEDINSTRUCTION 0x01

#define PCD8544_DISPLAYBLANK 0x0
#define PCD8544_DISPLAYNORMAL 0x4
#define PCD8544_DISPLAYALLON 0x1
#define PCD8544_DISPLAYINVERTED 0x5

// H = 0
#define PCD8544_FUNCTIONSET 0x20
#define PCD8544_DISPLAYCONTROL 0x08
#define PCD8544_SETYADDR 0x40
#define PCD8544_SETXADDR 0x80

// H = 1
#define PCD8544_SETTEMP 0x04
#define PCD8544_SETBIAS 0x10
#define PCD8544_SETVOP 0x80


// the memory buffer for the LCD
volatile uint8_t pcd8544_buffer [SCREEN_W * SCREEN_H / 8] = 
{ 0
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFC, 0xFE, 0xFF, 0xFC, 0xE0,
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,
// 0xF8, 0xF0, 0xF0, 0xE0, 0xE0, 0xC0, 0x80, 0xC0, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F, 0x7F,
// 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0xFF,
// 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0xC7, 0xC7, 0x87, 0x8F, 0x9F, 0x9F, 0xFF, 0xFF, 0xFF,
// 0xC1, 0xC0, 0xE0, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFC, 0xFC, 0xFC, 0xFE, 0xFE, 0xFE,
// 0xFC, 0xFC, 0xF8, 0xF8, 0xF0, 0xE0, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x00, 0x80, 0xC0, 0xE0, 0xF1, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x1F, 0x0F, 0x0F, 0x87,
// 0xE7, 0xFF, 0xFF, 0xFF, 0x1F, 0x1F, 0x3F, 0xF9, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xFD, 0xFF,
// 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F, 0x0F, 0x07, 0x01, 0x00, 0x00, 0x00,
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x00, 0x00, 0x00, 0xF0, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
// 0x7E, 0x3F, 0x3F, 0x0F, 0x1F, 0xFF, 0xFF, 0xFF, 0xFC, 0xF0, 0xE0, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF,
// 0xFF, 0xFC, 0xF0, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01,
// 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0F, 0x1F, 0x3F, 0x7F, 0x7F,
// 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0x1F, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};


//-------------------------------------------------
// --- PCD8544 COMMANDS ---
#define CMD_NOOP               0x00
#define CMD_FS                 0x20
#define CMD_SET_Y              0x40
#define CMD_SET_X              0x80
 
#define CMD_DISPLAY_BLANK      0x08
#define CMD_DISPLAY_NORMAL     0x0C
#define CMD_DISPLAY_BLACK      0x0B
#define CMD_DISPLAY_INVERSE    0x0D
 
#define CMD_FS_CHIP_ON         0x00
#define CMD_FS_CHIP_OFF        0x04
#define CMD_FS_H_ALIGN         0x00
#define CMD_FS_V_ALIGN         0x02
#define CMD_FS_BASIC           0x00
#define CMD_FS_EXTENDED        0x01
 
#define CMD_EXT_TEMP           0x04
#define CMD_EXT_BIAS           0x10
#define CMD_EXT_VOP            0x80

//-------------------------------------------------
//inline 
void _LCD_spiWrite (uint8_t byte) {
//#ifdef TRUE //__DEBUG
    uint8_t bit;
    // for(uint8_t bit = 0x80; bit; bit >>= 1) {
    for (bit = 0x80; bit; bit = bit >> 1) {
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
/**
#else
    //while(!SPI1STATbits.SPIRBF); //while (0 != SPI1STATbits.SPITBF){}; // wait until bus cycle complete
    SPI2BUF = byte;    // write byte to SSP1BUF register
    while(!SPI2STATbits.SPIRBF); //while (0 != SPI1STATbits.SPIRBF){};
    return SPI2BUF;
#endif 
*/
}



void halLCD_sendCommand (uint8_t c) {
	LCD_DC_L;
    //__NOP();
    //__NOP();
	LCD_CS_L;
    //__NOP();
    //__NOP();
	_LCD_spiWrite(c);
    //__NOP();
    //__NOP();
	LCD_CS_H;
    //__NOP();
}


void halLCD_sendData (uint8_t c) {
	LCD_DC_H;
    //__NOP();
    //__NOP();
	LCD_CS_L;
    //__NOP();
    //__NOP();
	_LCD_spiWrite(c);
    //__NOP();
    //__NOP();
	LCD_CS_H;
    //__NOP();
}


void _LCD_setContrast (uint8_t val) {
    if (val > 0x7F) {
        val = 0x7F;
    }
    halLCD_sendCommand (PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION);
    halLCD_sendCommand (PCD8544_SETVOP | val); 
    halLCD_sendCommand (PCD8544_FUNCTIONSET);
}


void hal_paint_init (uint8_t mode) {
    // Setup software SPI.
    GPIO_InitTypeDef GPIO_InitStructure;

    LCD_CLK_L;
    LCD_CS_H;
    //LCD_RESET_H;
    LCD_RESET_L;
    halLCD_delay (1);
    LCD_RESET_H;
    


    


//     halLCD_sendCommand (CMD_SET_X | 0); // x
//     halLCD_sendCommand (CMD_SET_Y | 0); // y

    // get into the EXTENDED mode!
    halLCD_sendCommand (PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION);

    // LCD bias select (4 is optimal?)
    halLCD_sendCommand (PCD8544_SETBIAS | 0x04);

    // set VOP
    halLCD_sendCommand (PCD8544_SETVOP | 0x7F); // Experimentally determined
    
    // normal mode
    halLCD_sendCommand (PCD8544_FUNCTIONSET);

    // Set display to Normal
    halLCD_sendCommand (PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);

    // initial display line
    // set page address
    // set column address
    // write display data

    // set up a bounding box for screen updates

    //  updateBoundingBox(0, 0, LCDWIDTH-1, LCDHEIGHT-1);
    // Push out pcd8544_buffer to the Display (will show the AFI logo)
}


void     hal_paint_setPixelColor (coord_t x, coord_t y, color_t color) {
    if ((x >= SCREEN_W) || (y >= SCREEN_H))
        return;
    if (COLOR_WHITE != color)     // x is which column
        pcd8544_buffer[x + (y / 8) * SCREEN_W] |=  (1 << (y % 8));  
    else
        pcd8544_buffer[x + (y / 8) * SCREEN_W] &= ~(1 << (y % 8)); 
}


void hal_paint_setPixel (coord_t x, coord_t y) {
    if ((x >= SCREEN_W) || (y >= SCREEN_H))
        return;
    if (COLOR_WHITE != halLCD_lcdStruct.color)     // x is which column
        pcd8544_buffer[x + (y / 8) * SCREEN_W] |=  (1 << (y % 8));  
    else
        pcd8544_buffer[x + (y / 8) * SCREEN_W] &= ~(1 << (y % 8)); 
}


color_t hal_paint_getPixel (coord_t x, coord_t y) {
    if ((x >= SCREEN_W) || (y >= SCREEN_H))
        return COLOR_WHITE;
    if (COLOR_WHITE != ((pcd8544_buffer[x + (y / 8) * SCREEN_W] >> (y % 8)) & 0x01))
        return COLOR_BLACK;
    else
        return COLOR_WHITE;    
}


//#define enablePartialUpdate

// #ifdef enablePartialUpdate
// static uint8_t xUpdateMin, xUpdateMax, yUpdateMin, yUpdateMax;
// #endif
#if LCD_ENABLE_PARTIAL_UPDATE
uint8_t halLCD_cnt = 0;
#endif

void hal_paint_repaint (void) {
    uint8_t col, maxcol, p;
    
#if LCD_ENABLE_PARTIAL_UPDATE
    if (++halLCD_cnt >= 6)
        halLCD_cnt = 0;
    p = halLCD_cnt;
#else
    for (p = 0; p < 6; p++)
    {

//         // check if this page is part of update
//         if ( yUpdateMin >= ((p + 1) * 8) ) {
//             continue;   // nope, skip it!
//         }
//         if (yUpdateMax < p * 8) {
//             break;
//         }
#endif
        halLCD_sendCommand (PCD8544_SETYADDR | p);
//#ifdef enablePartialUpdate
//        col = xUpdateMin;
//        maxcol = xUpdateMax;
//#else
        // start at the beginning of the row
        col = 0;
        maxcol = SCREEN_W - 1;
//#endif
        halLCD_sendCommand (PCD8544_SETXADDR | col);
        LCD_DC_H;
        LCD_CS_L;
        for (; col <= maxcol; col++) {
            _LCD_spiWrite (pcd8544_buffer[(SCREEN_W * p) + col]);
        }
        LCD_CS_H;
#if LCD_ENABLE_PARTIAL_UPDATE
#else
    }
#endif
    halLCD_sendCommand (PCD8544_SETYADDR);  // no idea why this is necessary but it is to finish the last byte?
// #ifdef enablePartialUpdate
//     xUpdateMin = LCD_CTRL_WIDTH - 1;
//     xUpdateMax = 0;
//     yUpdateMin = LCD_CTRL_HEIGHT - 1;
//     yUpdateMax = 0;
// #endif
}


void hal_paint_repaint_ (void) {



    halLCD_sendCommand (PCD8544_SETYADDR | 0);

    halLCD_sendCommand (PCD8544_SETXADDR | 0);
    LCD_DC_H;
    LCD_CS_L;

        _LCD_spiWrite (0x55);

    LCD_CS_H;

    halLCD_sendCommand (PCD8544_SETYADDR);  // no idea why this is necessary but it is to finish the last byte?
}


// clear everything
void hal_paint_cls (color_t color) {
    uint8_t c;
    if (COLOR_BLACK == color)
        c = 0xFF;
    else
        c = 0x00;
    
    memset ((void *)&pcd8544_buffer[0], c, SCREEN_W * SCREEN_H / 8);
//  updateBoundingBox(0, 0, LCDWIDTH-1, LCDHEIGHT-1);
//  cursor_y = cursor_x = 0;
}


// массив всегда заполняется слева направао и с верху в низ
void     hal_paint_fillBlock (coord_t x, coord_t y, uint16_t w, uint16_t h, color_t *buf) {
    coord_t xx, yy;
    
    for (xx = 0; xx < w; xx++)
        for (yy = 0; yy < h; yy++)
            hal_paint_setPixelColor (x + xx, y + yy, *buf++);
}


void     hal_paint_fillBlockColor (coord_t x, coord_t y,
uint16_t w,
uint16_t h,
color_t color) {
    uint8_t c;
    if (COLOR_BLACK == color)
        c = 0xFF;
    else
        c = 0x00;
    memset ((void *)&pcd8544_buffer[0], c, SCREEN_W * SCREEN_H / 8);
}


void halLCD_setOrientation (uint8_t orient) {

}

#endif
