/*
 * @file
 * @author  ZAY_079
 * @date    05.03.2014
 * @version V1.0.6
 * @brief


//#define COLORS_1BIT            1
//#define COLORS_4BIT            1
//#define COLORS_8BIT            1
#define COLORS_12BIT            1
//#define COLORS_16BIT            1
//#define COLORS_24BIT            1


*/

#ifndef COLORS_H
#define	COLORS_H 20150820 /* Revision ID */

#include "board.h"
//#include "modPaint_local.h"


typedef enum
{
#ifdef COLORS_1BIT
    COLOR_WHITE       = 0x1,
    COLOR_BLAGK       = 0x0,
#endif

#ifdef COLORS_8BIT
//8-ми битовая цветовая палитра (256 цветов)
    COLOR_GREEN       = 0x1C,
    COLOR_DARK_GREEN  = 0x15,
    COLOR_RED         = 0xE0,
    COLOR_BLUE        = 0x1F,
    COLOR_DARK_BLUE   = 0x03,
    COLOR_YELLOW      = 0xFC,
    COLOR_ORANGE      = 0xEC,
    COLOR_VIOLET      = 0xE3,
    COLOR_WHITE       = 0xFF,
    COLOR_BLAGK       = 0x00,
    COLOR_GREY        = 0x6D,
#endif
 
#ifdef COLORS_12BIT
	COLOR_NONE			= 0x0000,
	COLOR_BLACK			= 0x0000,
	COLOR_WHITE			= 0x0FFF,
    
    COLOR_RED			= 0x0F00,
    COLOR_GREEN			= 0x00F0,
    COLOR_BLUE			= 0x000F,
#endif

#ifdef COLORS_16BIT //16-ти битовая цветовая палитра (65536 цветов)
    COLOR_BLACK         = 0x0000,
    COLOR_WHITE         = 0xFFFF,
    COLOR_GRAY          = 0xE79C,
    COLOR_GREEN         = 0x07E0,
    COLOR_BLUE                 = 0x001F,
    COLOR_RED                  = 0xF800,
    COLOR_SKY                  = 0x5d1c,
    COLOR_YELLOW               = 0xffe0,
    COLOR_MAGENTA              = 0xf81f,
    COLOR_CYAN                 = 0x07ff,
    COLOR_ORANGE               = 0xfca0,
    COLOR_PINK                 = 0xF97F,
    COLOR_BROWN                = 0x8200,
    COLOR_VIOLET               = 0x9199,
    COLOR_SILVER               = 0xa510,
    COLOR_GOLD                 = 0xa508,
    COLOR_BEGH                 = 0xf77b,
    COLOR_NAVY                 = 0x000F,
    COLOR_DARK_GREEN           = 0x03E0,
    COLOR_DARK_CYAN            = 0x03EF,
    COLOR_MAROON               = 0x7800,
    COLOR_PURPLE               = 0x780F,
    COLOR_OLIVE                = 0x7BE0,
    COLOR_LIGHT_GREY           = 0xC618,
    COLOR_DARK_GREY            = 0x7BEF,
#endif

#ifdef COLORS_24BIT
    COLOR_NONE			    = 0x00000000,
	COLOR_BLACK			    = 0x00000000,
	COLOR_RED			    = 0x00FF0000,
	COLOR_RED_HALF		    = 0x00800000,
	COLOR_GREEN			    = 0x0000FF00,
	COLOR_GREEN_HALF	    = 0x00008000,
	COLOR_BLUE			    = 0x000000FF,
	COLOR_BLUE_HALF		    = 0x00000080,
	COLOR_WHITE			    = 0x00FFFFFF,
#endif

/*
#define COLOR_SILVER                rgb_color_repack(0xC618)
#define COLOR_GRAY                  rgb_color_repack(0x8410)
#define COLOR_MAROON                rgb_color_repack(0x8000)
#define COLOR_YELLOW                rgb_color_repack(0xFFE0)
#define COLOR_OLIVE                 rgb_color_repack(0x8400)
#define COLOR_LIME                  rgb_color_repack(0x07E0)
#define COLOR_AQUA                  rgb_color_repack(0x07FF)
#define COLOR_TEAL                  rgb_color_repack(0x0410)
#define COLOR_NAVY                  rgb_color_repack(0x0010)
#define COLOR_FUCHSIA               rgb_color_repack(0xF81F)
#define COLOR_PURPLE                rgb_color_repack(0x8010)
*/

} COLOR_RGB_t;


#ifdef	__cplusplus
extern "C" {
#endif


#ifdef	__cplusplus
}
#endif

#endif	/* COLORS_H */
