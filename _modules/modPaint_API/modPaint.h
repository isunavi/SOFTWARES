/*
 * @file    LCD grafic
 * @author  Ht3h5793
 * @date    05.03.2014
 * @version V13.8.6
 * @brief
 *
 * использовалс§ TheDotFactory
 *
 *

 Уестовые проверочные панграммы
// "!"#$%&'()*+,-./0123456789:;<=>?@[\]^_`"
// "The quick brown fox jumps over the lazy dog."
// "Чъешь [же] ещ™ этих м§гких французских булок да выпей чаю."

//     paint_setBackgroundColor (COLOR_BLUE);
//     paint_setFont (PAINT_FONT_Generic_8pt, PAINT_FONT_MS);
// //     paint_strClearRow (7);
// //     paint_strClearRow (8);
// //     paint_strClearRow (9);
// //     paint_strClearRow (10);
// //     paint_strClearRow (11);
// //     paint_strClearRow (12);
//     paint_putStrXY (0, 256, (char *)" !\"#$%&'()*+,-./0123456789");
//     paint_putStrXY (0, 288, (char *)":;<=>?@[\]^_`{|}~");
//     paint_putStrXY (0, 320, (char *)"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
//     paint_putStrXY (0, 352, (char *)"abcdefghijklmnopqrstuvwxyz");
//     paint_putStrXY (0, 384, (char *)"Љљђv???ЂїЕ ОЮНЭЊЦЧУФСТ?Д°ҐПЯє®Є€");
//     paint_putStrXY (0, 416, (char *)"абвгдежзийклмнопрстуфхцчшщъыьэю§");

   /// ђеро§тно дл§ загрузки бмп файлов http://c-site.h1.ru/infa/bmp_struct.htm
     // http://code.google.com/p/lcd-image-converter/

*/

#ifndef MODPAINT_H
#define	MODPAINT_H 20150820 /* Revision ID */

#include <board.h>
#include "modPaint_local.h"
#include "colors.h"

/* Point mode type */
typedef enum {
    PAINT_PIXEL_CLR = 0,
    PAINT_PIXEL_SET = 1,
    PAINT_PIXEL_XOR = 2	// Note: PIXEL_XOR mode affects only to vram_put_point().
                    // Graph functions only transmit parameter to vram_put_point() and result of graph functions
                    // depends on it's implementation. However Line() and Bar() functions work well.
} MODPAINT_point_mode;

/** quarters (for circle)
  * II  | I
  * III | IV
  */
typedef enum  {
    PAINT_QUARTERS_I   = 0x01,
    PAINT_QUARTERS_II  = 0x02,
    PAINT_QUARTERS_III = 0x04,
    PAINT_QUARTERS_IV  = 0x08,
    PAINT_QUARTERS_ALL = 0x0F,

} MODPAINT_QUARTERS;

typedef enum {
    PAINT_FONT_MS = 0, //monospaced
    PAINT_FONT_PP = 1 //proportional
        
} PAINT_FONT_TYPE;

/* Font descriptor type */
typedef struct {
    uint16_t char_width;
    uint16_t char_offset;
} font_descriptor_t;

typedef struct {
    uint16_t width; // max width
    uint16_t height; // height
    uint16_t vSpacing; // vSpacing промежутки между символами
    uint16_t hSpacing; // hSpacing
    uint16_t offset; // offset
    uint16_t rezerv_1; // rezerv
    uint16_t rezerv_2; // rezerv

    unsigned char start_char;
    unsigned char end_char;
    const font_descriptor_t *descr_array;
    const unsigned char *font_bitmap_array;
} font_info_t;



#define SEG_LENGHT       17 //размер программного сегментного шрифта


#ifdef	__cplusplus
extern "C" {
#endif

//------------------------ PAINT H-level -------------------------
#define  paint_getWidth()  SCREEN_W
#define  paint_getHeight() SCREEN_H

msg_t paint_init (uint8_t mode);

void paint_setBackgroundColor (color_t color);
void paint_setColor (color_t color);

void paint_clearScreen (void);

void paint_pixel (coord_t x, coord_t y);
void paint_pixelColor (coord_t x, coord_t y, color_t color);
void paint_pixelBig (coord_t x, coord_t y, uint16_t size);

void paint_lineX (coord_t x0, coord_t y0, uint16_t w);
void paint_lineY (coord_t x0, coord_t y0, uint16_t h);
void paint_line (coord_t x1, coord_t y1, coord_t x2, coord_t y2);
//void paint_lineWidth (coord_t x1, coord_t y1, coord_t x2, coord_t y2, uint16_t width);

void paint_rect (coord_t x1, coord_t y1, uint16_t xw, uint16_t yh);
void paint_rectFill (coord_t x1, coord_t y1, uint16_t xw, uint16_t yh);

void paint_circ (coord_t x, coord_t y, uint16_t radius, MODPAINT_QUARTERS corner_type);
void paint_circFill (coord_t x, coord_t y, uint16_t radius, MODPAINT_QUARTERS corner_type);

// Draw a rounded rectangle
void paint_rectRound (coord_t x, coord_t y, uint16_t w, uint16_t h, uint16_t r);
// Fill a rounded rectangle
void paint_rectFillRound (coord_t x, coord_t y, uint16_t w, uint16_t h, uint16_t r);

void paint_bitmapMono (coord_t x1, coord_t y1, uint16_t w, uint16_t h, const uint8_t *buf);
void setBitmapColor (coord_t x1, coord_t y1, uint16_t w, uint16_t h, color_t  *buf);

void paint_triangle (coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t x2, coord_t y2);
void paint_triangleFill (coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t x2, coord_t y2);

void paint_ellipse (coord_t X, coord_t Y, coord_t A, coord_t B);
void paint_ellipsFill (coord_t x, coord_t y, uint16_t w, uint16_t h, uint8_t corner_type);

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t paint_toColor565 (uint8_t r, uint8_t g, uint8_t b);

color_t paint_RGB888_RGB16 (uint8_t r, uint8_t g, uint8_t b);
void paint_RGB16_RGB888 (color_t color, uint8_t *pColor);
color_t paint_blendColor (color_t fg, color_t bg, uint8_t alpha);


// вывод на экран, фактически запуск вывода либо переключение слоев
void paint_repaint (void);

//---------------------------- TEXT -------------------------------
void paint_setFont (uint8_t num, PAINT_FONT_TYPE type);
void paint_setTextSize (uint8_t s);

// console type
coord_t paint_getMaxCol (void);
coord_t paint_getMaxRow (void);
msg_t paint_gotoColRow (coord_t col, coord_t row);
void paint_putChar (char c);
void paint_putCharColRow (coord_t col, coord_t row, char c);
void paint_strClearRow (coord_t row);
void paint_putStr (const char *str);
void paint_putStrColRow (coord_t col, coord_t row, const char *str);

// normal type
coord_t paint_putCharXY  (coord_t x, coord_t y, char c);
coord_t paint_putStrXY (coord_t x, coord_t y, const char *str);

coord_t paint_getCursorCol (void);
coord_t paint_getCursorRow (void);

//void _paint_putStrXY (coord_t x, coord_t y, char *str);
void paint_draw7SegHEX (uint8_t type, coord_t x, coord_t y, char c);


#ifdef	__cplusplus
}
#endif

#endif	/* MODPAINT_H */
