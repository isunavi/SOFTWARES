// http://git.rockbox.org/?p=rockbox.git;a=blob;f=apps/plugins/mosaique.c;h=68938a1f136ebd6f03ef0d7dfe49e13acfd8678c;hb=HEAD

#include "board.h"
#include "modPaint_local.h"
#include "halPaint.h"
#include "modPaint.h"
//#include "modSysClock.h"
#include "modRandom.h"



// Рабочая зона
#define MOSAIQUE_MASHTAB        1
#define MOSAIQUE_GFX_X          ((SCREEN_W / 2 -1) / MOSAIQUE_MASHTAB) //@todo переделать!
#define MOSAIQUE_GFX_Y          ((SCREEN_H / 2 -1) / MOSAIQUE_MASHTAB)
#define MOSAIQUE_COLOR_SET_A    COLOR_WHITE // COLOR_BLUE
#define MOSAIQUE_COLOR_SET_B    COLOR_BLACK

int16_t mosaique_x;
int16_t mosaique_y;
int16_t mosaique_sx;
int16_t mosaique_sy;


    
void mosaique_init (void);
void mosaique_draw (void);
    
/**
 * @brief mosaique_pixel - invertor pixel (xor emulator)
 * @param x
 * @param y
 */
void mosaique_pixel (uint16_t x, uint16_t y) {
    color_t tmp_color;
    uint16_t i, j;

    x *= MOSAIQUE_MASHTAB;
    y *= MOSAIQUE_MASHTAB;
    tmp_color = halLCD_getPixel (x + 0, y + 0); 
    for (j = 0; j < MOSAIQUE_MASHTAB; j++) {
        for (i = 0; i < MOSAIQUE_MASHTAB; i++) {
            if (SCREEN_W <= x + i) x = (SCREEN_W -1);
            if (SCREEN_H <= y + j) y = (SCREEN_H -1);
            //tmp_color = halLCD_getPixel (x + i, y + j);
            if (MOSAIQUE_COLOR_SET_A == tmp_color) // inverse bits
                paint_pixelColor (x + i, y + j, MOSAIQUE_COLOR_SET_B);
            else
                paint_pixelColor (x + i, y + j, MOSAIQUE_COLOR_SET_A);
        }
    }
}


/**
 * @brief mosaique_rect - invertiv rectangle
 * @param x1 - coord x
 * @param y1 - coord y
 * @param xd - width
 * @param yd - height
 */
void  mosaique_rect (uint16_t x1,
    uint16_t y1,
    uint16_t xd,
    uint16_t yd) {
    uint16_t i;

    for (i = 0; i < xd; i++)
    {
        mosaique_pixel (x1 + i, y1);
    }
    for (i = 0; i < xd; i++)
    {
        mosaique_pixel (x1 + i, y1 + yd);
    }
    for (i = 1; i < yd; i++)
    {
        mosaique_pixel (x1, y1 + i);
    }
    for (i = 0; i < yd +1; i++)
    {
        mosaique_pixel (x1 + xd, y1 + i);
    }
}


void mosaique_init (void) {
    mosaique_sx = _rand32() % (MOSAIQUE_GFX_Y / 2) + 1;
    mosaique_sy = _rand32() % (MOSAIQUE_GFX_Y / 2) + 1;
    mosaique_x = 0;
    mosaique_y = 0;
    paint_setColor (COLOR_WHITE);
    paint_setBackgroundColor (COLOR_BLACK);
    paint_clearScreen ();
}


void mosaique_draw (void) {
    mosaique_x += mosaique_sx;
    if (mosaique_x > MOSAIQUE_GFX_X) { // Вышел за пределы экрана?
        mosaique_x  = 2 * MOSAIQUE_GFX_X - mosaique_x; // Возвращаемся назад
        mosaique_sx = -mosaique_sx; // Инверсия направления
    }

    if (mosaique_x < 0) {
        mosaique_x  = -mosaique_x;
        mosaique_sx = -mosaique_sx;
    }

    mosaique_y += mosaique_sy;
    if (mosaique_y > MOSAIQUE_GFX_Y) {
        mosaique_y  = 2 * MOSAIQUE_GFX_Y - mosaique_y;
        mosaique_sy = -mosaique_sy;
    }

    if (mosaique_y < 0) {
        mosaique_y  = -mosaique_y;
        mosaique_sy = -mosaique_sy;
    }
    mosaique_rect (MOSAIQUE_GFX_X -  mosaique_x, MOSAIQUE_GFX_Y -  mosaique_y,
                   2 * mosaique_x , 2 * mosaique_y);
}

