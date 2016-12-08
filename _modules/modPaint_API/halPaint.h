/*
 * @file
 * @author  Ht3h5793
 * @date    29.09.2014
 * @version V6.5.7
 * @brief
 * @todo
 */

#ifndef HALSCREEN_H // макрозащита, include guard
#define	HALSCREEN_H 20151221 /* Revision ID */

#include "board.h"
#include "modPaint_local.h"
#include "colors.h"

// !!!WARNING!!! not all LCD!
enum LCD_TYPE_DRAW {
  GDI_ROP_COPY,
  GDI_ROP_XOR,
  GDI_ROP_AND,
  GDI_ROP_OR,
  
  GDI_INVERSE
};


#ifdef	__cplusplus
extern "C" {
#endif
//обязательные функции
void        hal_paint_init (uint8_t mode);
void        hal_paint_DeInit (uint8_t mode);

void        hal_paint_setColor (color_t color);
void        hal_paint_cls (color_t color);

void        hal_paint_setPixelColor (coord_t x, coord_t y, color_t color);
color_t     hal_paint_getPixel (coord_t x , coord_t y);

void        hal_paint_fillBlockColor (coord_t x, coord_t y, uint16_t w, uint16_t h, color_t color);

void        hal_paint_fillBlock (coord_t x, coord_t y, uint16_t w, uint16_t h, color_t *color);

void        hal_paint_fillScreen (color_t color);

void        hal_paint_setOrientation (uint8_t orient);

uint16_t    hal_paint_getWidth(void);
uint16_t    hal_paint_getHeight(void);

/** Далее функции если есть, вызывайте сами */
void        hal_paint_setLayer (uint8_t num);
void        hal_paint_paintLayer (uint8_t num);
void        hal_paint_repaint (void);

void        hal_paint_setBackLight (uint8_t value);
//trash
void _paint_rect (coord_t x1, coord_t y1, uint16_t xw, uint16_t yh);
void        hal_paint_layer_copy (uint8_t layerA, uint8_t layerB);

void        hal_paint_WriteDMA (coord_t x, coord_t y, uint16_t w, uint16_t h);


void paint_pixel_set (uint16_t x, uint16_t y);
void paint_pixel_clr (uint16_t x, uint16_t y);


uint16_t paint_RGB888_RGB16 (uint8_t r, uint8_t g, uint8_t b);


void hal_paint_picture (void);

void        halLCD_writeCommand (uint8_t com);
void        halLCD_writeData8bit (uint8_t data);
void        halLCD_writeData16bit (uint16_t data);


#ifdef	__cplusplus
}
#endif

#endif	/* HALLCD_H */
