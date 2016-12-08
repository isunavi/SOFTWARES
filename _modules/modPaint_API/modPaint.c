#include "modPaint.h"
#include "board.h"
//#include "modPaint_local.h"
#include "halPaint.h"

#if PAINT_FONT_x3y5
    #include "fonts\font_x3y5.h"
#endif
#if PAINT_FONT_Generic_8pt
    #include "fonts\font_Generic_8pt.h"
#endif
#if PAINT_FONT_Arial_14pt
    #include "fonts\font_Arial_14pt.h"
#endif
#if PAINT_FONT_Arial_20pt
    #include "fonts\font_Arial_20pt.h"
#endif
#if PAINT_FONT_Arial_120pt
    #include "fonts\font_Arial_120pt.h"
#endif
#if PAINT_FONT_BookmanOldStyle_120pt
    #include "fonts\font_BookmanOldStyle_120pt.h"
#endif
#if PAINT_FONT_OmniblackOutline_120pt
    #include "fonts\font_OmniblackOutline_120pt.h"
#endif


struct FontSettings_t { // параметры символа
    //uint8_t   header_offset;
    //uint8_t   char_offset;
    uint8_t             numFont;
    uint8_t             width;
    uint8_t             height;
    uint8_t             vSpacing;
    uint8_t             hSpacing;
    uint16_t            offsetX;
    uint16_t            offsetY;
    uint16_t            maxCol;
    uint16_t            maxRow;
    uint32_t            offset; // смещение по-тексту
    coord_t             charX; // текущая координата курсора
    coord_t             charY;
    font_info_t        *pFontInfo;
    
    uint8_t             type;
};

struct tsPaint {
    color_t                 fColor; // рисуемый цвет
    color_t                 bColor; // цвет фона
    struct FontSettings_t   font;
    uint8_t                 orientation;

} sPaint;


color_t paintBuf [LCD_BUF_SIZE]; // буфер для пикселей, вывод построчный


msg_t paint_init (uint8_t mode) {
    hal_paint_init (mode);
    return mode;
}


void paint_clearScreen (void) {
    //paint_setColor (sPaint.bColor);
    hal_paint_cls (sPaint.bColor);
}


void paint_repaint (void) {
    hal_paint_repaint ();
}


void  paint_pixel (coord_t x, coord_t y)
{
#ifdef CHECK_BORDER
    if (x >= SCREEN_W  ||
        y >= SCREEN_H) {
        // error
    } else {
#endif
    hal_paint_setPixelColor (x, y, sPaint.fColor);
#ifdef CHECK_BORDER
    }
#endif
}


void paint_pixelColor (coord_t x, coord_t y, color_t color) {
#ifdef CHECK_BORDER
    if (x >= SCREEN_W  ||
        y >= SCREEN_H) {
        // error
    } else {
#endif
    hal_paint_setPixelColor (x, y, color);
#ifdef CHECK_BORDER
    }
#endif
}


void paint_setColor (color_t color)
{
    sPaint.fColor = color;
    //halLCD_setColor (color);
}


void paint_setBackgroundColor (color_t color)
{
    sPaint.bColor = color;
}


uint8_t paint_RGB888_RGB8 (uint8_t r, uint8_t g, uint8_t b)
{
    return (uint8_t)((((r & 0xE0) >> 5) << 5) | (((g & 0xE0) >> 5) << 2) | ((b & 0xC0) >> 6));
}


uint16_t paint_RGB888_RGB16 (uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)((r & 0xF8) << 8) | (uint16_t)((g & 0xFC) << 3) | (uint16_t)(b >> 3);
}


void paint_RGB16_RGB888 (uint16_t color, uint8_t *pColor)
{
    pColor[0] = ((color & 0x001F) << 3) | (color & 0x07);          /* Blue value */
    pColor[1] = ((color & 0x07E0) >> 3) | ((color >> 7) & 0x03);   /* Green value */
    pColor[2] = ((color & 0xF800) >> 8) | ((color >> 11) & 0x07);  /* Red value */
}


#if (1 == LCD_COLOR)
color_t paint_blendColor (color_t fg, color_t bg, uint8_t alpha)
{
    uint8_t colorBuf[3];
    uint16_t r, g, b;
    uint16_t fg_ratio = alpha + 1;
    uint16_t bg_ratio;
    
    paint_RGB16_RGB888 (fg, &colorBuf[0]);
    bg_ratio = 256 - alpha;
    r = PAINT_RGB_COLOR_RED(fg) * fg_ratio;
    g = PAINT_RGB_COLOR_GREEN(fg) * fg_ratio;
    b = PAINT_RGB_COLOR_BLUE(fg) * fg_ratio;
    r += PAINT_RGB_COLOR_RED(bg) * bg_ratio;
    g += PAINT_RGB_COLOR_GREEN(bg) * bg_ratio;
    b += PAINT_RGB_COLOR_BLUE(bg) * bg_ratio;
    r /= 256;
    g /= 256;
    b /= 256;

    return paint_RGB888_RGB16 (r, g, b);
}
#endif


//@todo #ifdef CHECK_BORDER
void paint_bitmapMono (coord_t x1, coord_t y1, uint16_t xw, uint16_t yh, const uint8_t *buf) {
    uint16_t i, j, m;
    uint8_t ch;

    for (j = 0; j < yh; j++)
    {
        for (i = 0; i < xw; i++)
        {
            ch = *buf++;
            for (m = 0; m < 8; m++)
            {
                if (ch & 0x80) {
                    paintBuf[i * 8 + m] = sPaint.fColor;
                } else {
                    paintBuf[i * 8 + m] = sPaint.bColor;
                }
                ch = ch << 1;
            }
        }
        hal_paint_fillBlock //halLCD_fillBlockDMA 
        ((coord_t)x1,
            (coord_t)(y1 + j),
            xw * 8,
            1,
            &paintBuf[0]);
    }
}


void setBitmapColor (coord_t x1, coord_t y1, uint16_t xw, uint16_t yh, color_t *buf) {
    hal_paint_fillBlock(x1, y1, xw, yh, buf);
}



#ifdef PAINT_NEED_LINE
void paint_lineX (coord_t x0, coord_t y0, uint16_t w) {
#ifdef CHECK_BORDER
    if (x0 >= SCREEN_W  ||
        w >= SCREEN_W  ||
        y0 >= SCREEN_H) {
        // error
    } else {
#endif
    hal_paint_fillBlockColor (x0, y0, w, 1, sPaint.fColor);
#ifdef CHECK_BORDER
    }
#endif
}


void paint_lineY (coord_t x0, coord_t y0, uint16_t h) {
#ifdef CHECK_BORDER
    if (x0 >= SCREEN_W  ||
        y0 >= SCREEN_H  ||
        h >= SCREEN_H)
    {
        // error
    } else {
#endif
    hal_paint_fillBlockColor (x0, y0, 1, h, sPaint.fColor);
#ifdef CHECK_BORDER
    }
#endif
}


void paint_line (coord_t x1, coord_t y1, coord_t x2, coord_t y2) {
#ifdef CHECK_BORDER
    if (x1 >= SCREEN_W  ||
        x2 >= SCREEN_W  ||
        y1 >= SCREEN_H ||
        y2 >= SCREEN_H  )
    {
        // error
    } else {
#endif
        int16_t dy = 0; // all signed short!
        int16_t dx = 0;
        int16_t stepx = 0;
        int16_t stepy = 0;
        int16_t fraction = 0;

        dy = y2 - y1;
        dx = x2 - x1;
        if (dy < 0)
        {
            dy = -dy;
            stepy = -1;
        }
        else
            stepy = 1;

        if (dx < 0)
        {
            dx = -dx;
            stepx = -1;
        }
        else
            stepx = 1;
        dy <<= 1;
        dx <<= 1;

        paint_pixel (x1, y1);

        if (dx > dy)
        {
            fraction = dy - (dx >> 1);
            while (x1 != x2) {
                if (fraction >= 0)
                {
                    y1 += stepy;
                    fraction -= dx;
                }
                x1 += stepx;
                fraction += dy;
                paint_pixel (x1, y1);
            }
        }
        else
        {
            fraction = dx - (dy >> 1);
            while (y1 != y2)
            {
                if (fraction >= 0)
                {
                    x1 += stepx;
                    fraction -= dy;
                }
                y1 += stepy;
                fraction += dx;
                paint_pixel (x1, y1);
            }
        }
#ifdef CHECK_BORDER
    }
#endif
}
#endif //PAINT_NEED_LINE


#ifdef PAINT_NEED_RECT
void paint_rect (coord_t x1, coord_t y1, uint16_t xw, uint16_t yh) {
    uint16_t x2, y2;

    x2 = x1 + (xw - 1);
    y2 = y1 + (yh - 1);
#ifdef CHECK_BORDER
    if (x1 >= SCREEN_W  ||
        x2 >= SCREEN_W  ||
        y1 >= SCREEN_H ||
        y2 >= SCREEN_H) 
    {
        // error
    } else {
#endif
        paint_lineX (x1, y1, xw);
        paint_lineX (x1, y2, xw);
        paint_lineY (x1, y1, yh);
        paint_lineY (x2, y1, yh);
#ifdef CHECK_BORDER
    }
#endif
}


void paint_rectFill (coord_t x1, coord_t y1, uint16_t xw, uint16_t yh) {
#ifdef CHECK_BORDER
    if (x1 >= SCREEN_W  ||
        xw >= SCREEN_W  ||
        y1 >= SCREEN_H || // @todo сделать проверку нормальной!
        yh >= SCREEN_H)
    {
        // error
    } else {
#endif
        hal_paint_fillBlockColor (x1, y1, xw, yh, sPaint.fColor);
#ifdef CHECK_BORDER
    }
#endif
}


#ifdef PAINT_NEED_CIRCLE
void paint_rectRound (coord_t x, coord_t y, uint16_t w, uint16_t h, uint16_t r) {
    paint_lineX (x + r, y, w - 2 * r); // Top
    paint_lineX (x + r, y + h - 1, w - 2 * r); // Bottom
    paint_lineY (x, y + r, h - 2 * r); // Left
    paint_lineY (x + w - 1, y + r , h - 2 * r); // Right
    paint_circ (x+r    , y+r    , r, PAINT_QUARTERS_II); // draw four corners
    paint_circ (x+w-r-1, y+r    , r, PAINT_QUARTERS_I);
    paint_circ (x+w-r-1, y+h - r-1, r, PAINT_QUARTERS_IV);
    paint_circ (x+r    , y+h - r-1, r, PAINT_QUARTERS_III);
}


void paint_rectFillRound (coord_t x, coord_t y, uint16_t w, uint16_t h, uint16_t r) {
    paint_rectFill (x + r, y, w - 2 * r, h); // @todo
    //paint_setColor (COLOR_BLUE);
    paint_circFill (x+r    , y + r        , r, PAINT_QUARTERS_II); // draw four corners
    paint_circFill (x+w-r-1, y + r        , r, PAINT_QUARTERS_I);
    paint_circFill (x+w-r-1, y + h - r - 1, r, PAINT_QUARTERS_IV);
    paint_circFill (x+r    , y + h - r - 1, r, PAINT_QUARTERS_III);
    //paint_setColor (COLOR_GREEN);
    paint_rectFill (x, y+r, r, h-2*r);
    paint_rectFill (x+w-r-1, y+r, r, h-2*r);
}
#endif // PAINT_NEED_CIRCLE
#endif // PAINT_NEED_RECT


#ifdef PAINT_NEED_CIRCLE
//инкрементный алгоритм Хардебурга.
void paint_circ (coord_t x, coord_t y, uint16_t radius, MODPAINT_QUARTERS corner_type)
{
    int16_t xc = 0; // signed int!
    int16_t yc;
    int16_t p;

#ifdef CHECK_BORDER
    if (x >= SCREEN_W  ||
        y >= SCREEN_H ||
        radius >= SCREEN_W) {
        // error
    } else {
#endif
        yc = radius;
        p = 3 - (radius << 1);
        while (xc <= yc) {
            if (corner_type & PAINT_QUARTERS_I) {
                paint_pixel(x + xc, y - yc);
                paint_pixel(x + yc, y - xc);
            }
            if (corner_type & PAINT_QUARTERS_IV) {
                paint_pixel(x + xc, y + yc);
                paint_pixel(x + yc, y + xc);
            }
            if (corner_type & PAINT_QUARTERS_III) {
                paint_pixel(x - xc, y + yc);
                paint_pixel(x - yc, y + xc);
            }
            if (corner_type & PAINT_QUARTERS_II) {
                paint_pixel(x - xc, y - yc);
                paint_pixel(x - yc, y - xc);
            }
            if (p < 0) {
                p += (xc++ << 2) + 6;
            } else {
                p += ((xc++ - yc--) << 2) + 10;
            }
        }
#ifdef CHECK_BORDER
    }
#endif
}


void paint_circFill (coord_t x, coord_t y, uint16_t radius, MODPAINT_QUARTERS corner_type) {
    int16_t xc = 0; // signed int!
    int16_t yc;
    int16_t p;
#ifdef CHECK_BORDER
    if (x >= SCREEN_W  ||
        y >= SCREEN_H ||
        radius >= SCREEN_W) {
        // error
    } else {
#endif
        yc = radius;
        p = 3 - (radius << 1);
        while (xc <= yc) {
            /**
            paint_lineX(x - xc, y + yc, xc + xc);
            paint_lineX(x - yc, y + xc, yc + yc);
            paint_lineX(x - xc, y - yc, xc + xc);
            paint_lineX(x - yc, y - xc, yc + yc);
            */
            if (corner_type & PAINT_QUARTERS_I) {
                paint_lineX (x, y - yc, xc);
                paint_lineX (x, y - xc, yc);
            }
            if (corner_type & PAINT_QUARTERS_IV) {
                paint_lineX (x, y + yc, xc);
                paint_lineX (x, y + xc, yc);
            }
            if (corner_type & PAINT_QUARTERS_III) {
                paint_lineX (x - xc, y + yc, xc);
                paint_lineX (x - yc, y + xc, yc);
            }
            if (corner_type & PAINT_QUARTERS_II) {
                paint_lineX (x - xc, y - yc, xc);
                paint_lineX (x - yc, y - xc, yc);
            }
            if (p < 0)
            {
                p += (xc++ << 2) + 6;
            } else {
                p += ((xc++ - yc--) << 2) + 10;
            }
        }
#ifdef CHECK_BORDER
    }
#endif
}
#endif // PAINT_NEED_CIRCLE


#ifdef PAINT_NEED_ELLIPS
void paint_ellipse (coord_t X, coord_t Y, coord_t A, coord_t B) {
	int16_t Xc = 0, Yc = B;
	long A2 = (long)A*A, B2 = (long)B*B;
	long C1 = -(A2/4 + A % 2 + B2);
	long C2 = -(B2/4 + B % 2 + A2);
	long C3 = -(B2/4 + B % 2);
	long t = -A2 * Yc;
	long dXt = B2*Xc*2, dYt = -A2*Yc*2;
	long dXt2 = B2*2, dYt2 = A2*2;
    
    //LCD_pixel_mode = GDI_ROP_OR;
    
	while (Yc >= 0 && Xc <= A)
    {
		paint_pixel (X + Xc, Y + Yc);
		if (Xc != 0 || Yc != 0)
          paint_pixel (X - Xc, Y - Yc);
		if (Xc != 0 && Yc != 0)
        {
			paint_pixel (X + Xc, Y - Yc);
			paint_pixel (X - Xc, Y + Yc);
		}
		if (t + Xc*B2 <= C1 || t + Yc*A2 <= C3)
        {
			Xc++;
			dXt += dXt2;
			t   += dXt;
		} else if (t - Yc*A2 > C2) {
			Yc--;
			dYt += dYt2;
			t   += dYt;
		} else {
			Xc++;
			Yc--;
			dXt += dXt2;
			dYt += dYt2;
			t   += dXt;
			t   += dYt;
		}
	}
}
#endif


//subdivision rasterization
#ifdef PAINT_NEED_TRIANGLE
void paint_triangle (coord_t x1, coord_t y1, coord_t x2, coord_t y2, coord_t x3, coord_t y3)
{
#ifdef CHECK_BORDER
    if (x1 >= SCREEN_W ||
        y1 >= SCREEN_H ||
        x2 >= SCREEN_W ||
        y2 >= SCREEN_H ||
        x3 >= SCREEN_W ||
        y3 >= SCREEN_H) {
        // error
    } else {
#endif
    paint_line (x1, y1, x2, y2);
    paint_line (x2, y2, x3, y3);
    paint_line (x3, y3, x1, y1);
#ifdef CHECK_BORDER
    }
#endif
}


void paint_triangleFill_A (coord_t x1, coord_t y1, coord_t x2, coord_t y2, coord_t x3, coord_t y3)
{
    int32_t x, y, addx, dx, dy;
    int32_t P;
    uint16_t i;
    uint32_t a1, a2, b1, b2;
    
    if (y1 > y2)
    { b1 = y2; b2 = y1; a1 = x2; a2 = x1; }
    else
    { b1 = y1; b2 = y2; a1 = x1; a2 = x2; }
    dx = a2 -a1;
    dy = b2 - b1;
    if (dx < 0) dx=-dx;
    if (dy < 0) dy=-dy;
    x = a1;
    y = b1;
   
    if(a1 > a2)
        addx = -1;
    else
        addx = 1;
   
    if (dx >= dy)
    {
        P = 2*dy - dx;
        for (i = 0; i <= dx; ++i)
        {
            paint_line ((coord_t)x, (coord_t)y, x3, y3);
            if(P < 0)
            {
                P += 2*dy;
                x += addx;
            }
            else
            {
                P += 2*dy - 2*dx;
                x += addx;
                y++;
            }
        }
    }
    else
    {
        P = 2*dx - dy;
        for (i = 0; i <= dy; ++i)
        {
            paint_line ((coord_t)x, (coord_t)y, x3, y3);
            if (P < 0)
            {
                P += 2*dx;
                y++;
            }
            else
            {
                P += 2*dx - 2*dy;
                x += addx;
                y ++;
            }
        }
    }
}


//http://compgraphics.info/2D/triangle_rasterization.php
void __swap (coord_t *a, coord_t *b)
{
      coord_t *t;
      *t = *a;
      *a = *b;
      *b = *t;
}
 

void paint_triangleFill (coord_t x1, coord_t y1, coord_t x2, coord_t y2, coord_t x3, coord_t y3)
{
#ifdef CHECK_BORDER
    if (x1 >= SCREEN_W ||
        y1 >= SCREEN_H ||
        x2 >= SCREEN_W ||
        y2 >= SCREEN_H ||
        x3 >= SCREEN_W ||
        y3 >= SCREEN_H) {
        // error
    } else {
#endif
    // Упорядочиваем точки p1(x1, y1),
    // p2(x2, y2), p3(x3, y3)
    if (y2 < y1) {
        __swap (&y1, &y2);
        __swap (&x1, &x2);
    } // точки p1, p2 упорядочены
    if (y3 < y1) {
        __swap (&y1, &y3);
        __swap (&x1, &x3);
    } // точки p1, p3 упорядочены
    // теперь p1 самая верхняя
    // осталось упорядочить p2 и p3
    if (y2 > y3) {
        __swap (&y2, &y3);
        __swap (&x2, &x3);
    }
    
//        Private Sub PaintTriangle(v1 As Vec2, v2 As Vec2, v3 As Vec2, c As Long)
// Dim t1 As Vec2, t2 As Vec2, t3 As Vec2
// Dim x As Long, y As Long, x1 As Long, x2 As Long
//     uint32_t x, y;
//     uint32_t x1, x2;
//   t1 = v1: t2 = v2: t3 = v3
// //   If t1.y > t3.y Then Swap t1, t3
// //   If t1.y > t2.y Then Swap t1, t2
// //   If t2.y > t3.y Then Swap t2, t3
//   For y = t1.y To t2.y - 1&
//     x1 = t1.x + (t2.x - t1.x) * (y - t1.y) / (t2.y - t1.y)
//     x2 = t1.x + (t3.x - t1.x) * (y - t1.y) / (t3.y - t1.y)
//     Line (x1, y)-(x2, y), c
//   Next y
//   For y = t2.y To t3.y - 1&
//     x1 = t2.x + (t3.x - t2.x) * (y - t2.y) / (t3.y - t2.y)
//     x2 = t1.x + (t3.x - t1.x) * (y - t1.y) / (t3.y - t1.y)
//     Line (x1, y)-(x2, y), c
//   Next y
// End Sub

// Private Sub Swap(v1 As Vec2, v2 As Vec2)
// Dim p As Vec2
//   p = v1
//   v1 = v2
//   v2 = p
// End Sub


    //paint_triangleFill_A (x1, y1, x2, y2, x3, y3);
    //paint_triangleFill_A (x3, y3, x1, y1, x2, y2);
    //paint_triangleFill_A (x3, y3, x2, y2, x1, y1);
#ifdef CHECK_BORDER
    }
#endif
}

#endif // PAINT_NEED_TRIANGLE


#ifdef PAINT_NEED_ELLIPS
void paint_ellipsFill (coord_t origin_x, coord_t origin_y, uint16_t w, uint16_t h, uint8_t corner_type)
{
    int16_t hh = h * h;
    int16_t ww = w * w;
    int16_t hhww = hh * ww;
    int16_t x0 = w;
    int16_t dx = 0;
int x, y;
    // do the horizontal diameter
    //for (int x = -w; x <= w; x++)
    paint_lineX (origin_x - w, origin_y, w * 2);

    // now do both halves at the same time, away from the diameter
    for (y = 1; y <= h; y++)
    {
        int x1 = x0 - (dx - 1);  // try slopes of dx - 1 or more
        for ( ; x1 > 0; x1--)
            if (x1*x1*hh + y*y*ww <= hhww)
                break;
        dx = x0 - x1;  // current approximation of the slope
        x0 = x1;

        for (x = -x0; x <= x0; x++)
        {
            paint_pixel (origin_x + x, origin_y - y);
            paint_pixel (origin_x + x, origin_y + y);
        }
    }
}
#endif // PAINT_NEED_ELLIPS


//---------------------------- TEXT -------------------------------
void paint_setFont (uint8_t num, PAINT_FONT_TYPE type) { // настраиваем шрифт
    switch (num) {
#if PAINT_FONT_Generic_8pt
    case PAINT_FONT_Generic_8pt:
        sPaint.font.pFontInfo = (font_info_t *)Generic_8ptFontInfo;
        break;
#endif
#if PAINT_FONT_Arial_14pt
    case PAINT_FONT_Arial_14pt:
        sPaint.font.pFontInfo = (font_info_t *)Arial_14ptFontInfo;
        break;
#endif
#if PAINT_FONT_Arial_20pt
    case PAINT_FONT_Arial_20pt:
        sPaint.font.pFontInfo = (font_info_t *)Arial_20ptFontInfo;
        break;
#endif
#if PAINT_FONT_Arial_120pt
    case PAINT_FONT_Arial_120pt:
        sPaint.font.pFontInfo = (font_info_t *)Arial_120ptFontInfo;
    break;
#endif
#if PAINT_FONT_BookmanOldStyle_120pt
    case PAINT_FONT_BookmanOldStyle_120pt:
        sPaint.font.pFontInfo = (font_info_t *)BookmanOldStyle_120ptFontInfo;
        break;
#endif
#if PAINT_FONT_OmniblackOutline_120pt
    case PAINT_FONT_OmniblackOutline_120pt:
        sPaint.font.pFontInfo = (font_info_t *)OmniblackOutline_120ptFontInfo;
        break;
#endif
        //default: while(1); break; // зависнуть, тому что такого не может быт
    }

    sPaint.font.numFont   = num;
    sPaint.font.width     = sPaint.font.pFontInfo->width; //pFont_settings[0];
    sPaint.font.height    = sPaint.font.pFontInfo->height; //pFont_settings[1];
    sPaint.font.vSpacing  = (sPaint.font.width + sPaint.font.pFontInfo->vSpacing);
    sPaint.font.hSpacing  = (sPaint.font.height + sPaint.font.pFontInfo->hSpacing);
    sPaint.font.offset    = sPaint.font.pFontInfo->offset;
    sPaint.font.maxCol    = (paint_getWidth() / sPaint.font.vSpacing)  - 0; // 1; // столбцы
    sPaint.font.maxRow    = (paint_getHeight() / sPaint.font.hSpacing) - 0; // 1; // строки
    sPaint.font.offsetX   = (paint_getWidth() - sPaint.font.maxCol * sPaint.font.vSpacing) / 2; // смещение относительное
    sPaint.font.offsetY   = (paint_getHeight() - sPaint.font.maxRow * sPaint.font.hSpacing) / 2;
    //sPaint.font.pFont     = &pFont[0];
    //sPaint.font.pFontInfo = pFontInfo;
    //sPaint.font.mashtab   = 1;
    sPaint.font.type = type;
    paint_gotoColRow (0, 0);
    
#if PAINT_FONT_x3y5
    sPaint.font.pFontInfo = (font_info_t *)console_font_4x6;
    
    sPaint.font.numFont   = num;
    sPaint.font.width     = 4; //sPaint.font.pFontInfo->width;
    sPaint.font.height    = 6; //sPaint.font.pFontInfo->height;
    sPaint.font.vSpacing  = 4; //(sPaint.font.width + sPaint.font.pFontInfo->vSpacing);
    sPaint.font.hSpacing  = 6; //(sPaint.font.height + sPaint.font.pFontInfo->hSpacing);
    sPaint.font.offset    = 6; //sPaint.font.pFontInfo->offset;
    sPaint.font.maxCol    = (paint_getWidth() / sPaint.font.vSpacing)  - 0; // столбцы
    sPaint.font.maxRow    = (paint_getHeight() / sPaint.font.hSpacing) - 0; // строки
    sPaint.font.offsetX   = (paint_getWidth() - sPaint.font.maxCol * sPaint.font.vSpacing) / 2; // смещение относительное
    sPaint.font.offsetY   = (paint_getHeight() - sPaint.font.maxRow * sPaint.font.hSpacing) / 2;
    //sPaint.font.pFont     = &pFont[0];
    //sPaint.font.pFontInfo = pFontInfo;
    //sPaint.font.mashtab   = 1;
    //paint_gotoColRow (0, 0);
#endif
}


coord_t paint_getMaxCol (void)
{
    return sPaint.font.maxCol;
}


coord_t paint_getMaxRow (void)
{
    return sPaint.font.maxRow;
}


msg_t paint_gotoColRow (coord_t col, coord_t row)
{
#ifdef CHECK_BORDER
    if ((col >= paint_getMaxCol()) ||
         (row >= paint_getMaxRow())) {
        return FUNCTION_RETURN_ERROR;
    } else {
#endif
        sPaint.font.charX = sPaint.font.offsetX + (sPaint.font.vSpacing * col);
        sPaint.font.charY = sPaint.font.offsetY + (sPaint.font.hSpacing * row);
        return FUNCTION_RETURN_OK;
#ifdef CHECK_BORDER
    }
#endif
}


coord_t paint_getCursorCol (void) {
    return (sPaint.font.charX - sPaint.font.offsetX) / ( sPaint.font.vSpacing);
}


coord_t paint_getCursorRow (void) {
    return (sPaint.font.charY - sPaint.font.offsetY) / (sPaint.font.hSpacing);
}


void paint_putChar (char c) {
    uint8_t ch;
    uint32_t smesh;
    const uint8_t *bitmap_ptr;
    const uint8_t *__bitmap_ptr;
    uint8_t tmp;
    uint16_t i, j, k, font_height_bytes, current_char_width;
#if PAINT_FONT_Generic_8pt
    if (PAINT_FONT_Generic_8pt == sPaint.font.numFont)
    {
        ch = (uint8_t)c;
        if (192 <= (uint8_t)ch) // корректировка 'А'
            ch = (uint8_t)(ch - (96 - 6)); //(6 - псевдографика)
        else
            ch = (uint8_t)ch - 32;
        smesh = (uint32_t)ch * sPaint.font.offset;
        for (j = 0; j < sPaint.font.width; j++)
        { // every column of the character
            ch = sPaint.font.pFontInfo->font_bitmap_array[smesh + j];
            for (i = 0; i < sPaint.font.height; i++)
            { // i = y
                if (ch & 0x01) {
                    paintBuf[i] = sPaint.fColor;
                } else {
                    paintBuf[i] = sPaint.bColor;
                }
                ch = ch >> 1; // @todo добавить прорисовку между символами
            }
            hal_paint_fillBlock (sPaint.font.charX + j,
                sPaint.font.charY,
                1,
                sPaint.font.height,
                &paintBuf[0]);
        }
    }
    else
    {
        // Get height in bytes
        font_height_bytes = (sPaint.font.height) / 8;
        if (((sPaint.font.height) % 8)) 
            font_height_bytes++;
        
        current_char_width = 0;
        if (' ' != (uint8_t)c)
        {
            if ((uint8_t)'А' <= (uint8_t)c) // корректировка 'А' - руССкая
                c = (uint8_t)(c - (98)); //(6 - псевдографика)
            else
                c = (uint8_t)c - sPaint.font.pFontInfo->start_char;
            
            // Get width of current character
            current_char_width = sPaint.font.pFontInfo->descr_array[c].char_width;
            // Get pointer to data
            bitmap_ptr = (const uint8_t *)&(sPaint.font.pFontInfo->font_bitmap_array[(sPaint.font.pFontInfo->descr_array[c].char_offset)]);
            __bitmap_ptr = bitmap_ptr; // Copy to temporary

            for (k = 0; k < current_char_width; k++)
            { // Width of character
                for (i = 0; i < font_height_bytes; i++)
                { // Height of character
                    tmp = *__bitmap_ptr; // Get data to "shift"
                    __bitmap_ptr += current_char_width;
                    for (j = 0; j < 8; j++)
                    { // Push byte
                        if ((tmp & 0x01))
                        {
                        //    if (vram_put_point(__p, mode) == GRAPH_ERROR) err = GRAPH_ERROR;};
                            paintBuf[i * 8 + j] = sPaint.fColor;
                        } else {
                            paintBuf[i * 8 + j] = sPaint.bColor;
                        }
                        tmp >>= 1;
                    }
                }
                hal_paint_fillBlock (sPaint.font.charX + k,
                    sPaint.font.charY,
                    1,
                    sPaint.font.height,
                    &paintBuf[0]);
                __bitmap_ptr = ++bitmap_ptr;
            }
        }
        //paint_setBackgroundColor (sPaint.bgColor);
        hal_paint_fillBlockColor (sPaint.font.charX + current_char_width, // clear other
            sPaint.font.charY,
            sPaint.font.width - current_char_width,
            sPaint.font.height,
            sPaint.bColor);
    }
#endif
}


void  paint_putCharColRow (coord_t col, coord_t row, char c)
{
    if (FUNCTION_RETURN_OK == paint_gotoColRow (col, row))
    {
        paint_putChar (c);
    }
}


void paint_putStr (const char *str)
{
    uint16_t i = 0;
    coord_t x, y;

    x = paint_getCursorCol ();
    y = paint_getCursorRow ();
    while ('\0' != str[i]) {
        if ('\r' == str[i]) {
            x = 0;
            paint_gotoColRow (x, y);
        } else if ('\n' == str[i]) {
            if (++y >= paint_getMaxRow ())
                break;
            else
                paint_gotoColRow (x, y);
        } else { // symbol
            if (FUNCTION_RETURN_OK == paint_gotoColRow (x, y)) {
                paint_putChar (str[i]);
            }
            if (++x >= paint_getMaxCol ()) {
                x = 0;
                if (++y >= paint_getMaxRow ())
                    break;
            }
            if (FUNCTION_RETURN_OK != paint_gotoColRow (x, y)) {
                break;
            }
        }
        i++;
    }
}


void paint_putStrColRow (coord_t x, coord_t y, const char *str) {
    paint_gotoColRow (x, y);
    paint_putStr (str);
}


void paint_strClearRow (coord_t row) {
    coord_t i = 0;

    for (i = 0; i < paint_getMaxCol(); i++) {
        if (FUNCTION_RETURN_OK == paint_gotoColRow (i, row)) {
            paint_putChar (' ');
        }
    }
}


coord_t paint_putCharXY  (coord_t x, coord_t y, char c) {
    uint8_t ch;
    uint32_t smesh;
    const uint8_t *bitmap_ptr;
    const uint8_t *__bitmap_ptr;
    unsigned char tmp;
    uint16_t i, j, k, font_height_bytes, current_char_width;

#if PAINT_FONT_x3y5
    current_char_width = sPaint.font.width;
    if (1)//PAINT_FONT_Generic_8pt == sPaint.font.numFont)
    {
        ch = (uint8_t)c;
        //if (192 <= (uint8_t)ch) // корректировка 'А'
        //    ch = (uint8_t)(ch - (98 - 6)); //(6 - псевдографика)
        //else
        //    ch = (uint8_t)ch - 31;
        smesh = (uint32_t)ch * sPaint.font.offset;
        for (j = 0; j < sPaint.font.height; j++)
        { // every column of the character
            //ch = sPaint.font.pFontInfo->font_bitmap_array[smesh + j];
            ch = console_font_4x6[smesh + j];
            for (i = 0; i < sPaint.font.width; i++) 
            { // i = y
                if (ch & 0x80)
                {
                    //paintBuf[i] = sPaint.fColor;
                    paint_pixel_set (x + i, y + j);
                } else {
                    //paintBuf[i] = sPaint.bColor;
                    paint_pixel_clr (x + i, y + j);
                }
                ch = ch << 1; // @todo добавить прорисовку между символами
            }
            //halLCD_fillBlock (x + j, y,
            //    1, sPaint.font.height,
            //    &paintBuf[0]);
        }
    }
    
    return x + current_char_width;// + sPaint.font.pFontInfo->hSpacing;
#endif
#if PAINT_FONT_Generic_8pt
    current_char_width = 0;
    if (PAINT_FONT_Generic_8pt == sPaint.font.numFont)
    {
        ch = (uint8_t)c;

        if (192 <= (uint8_t)ch) // корректировка 'А'
            ch = (uint8_t)(ch - (96 - 6)); //(6 - псевдографика)
        else
            ch = (uint8_t)ch - 32;

        smesh = (uint32_t)ch * sPaint.font.offset;

        current_char_width = sPaint.font.width;
        for (j = 0; j < current_char_width; j++)
        { // every column of the character
            ch = sPaint.font.pFontInfo->font_bitmap_array[smesh + j];
            for (i = 0; i < sPaint.font.height; i++)
            { // i = y
                if (ch & 0x01)
                {
                    paintBuf[i] = sPaint.fColor;
                } else {
                    paintBuf[i] = sPaint.bColor;
                }
                ch = ch >> 1; // @todo добавить прорисовку между символами
            }
            hal_paint_fillBlock (x + j, y,
                1, sPaint.font.height,
                &paintBuf[0]);
        }
    }
    else
    {
        // Get height in bytes
        font_height_bytes = (sPaint.font.height) / 8;
        if (((sPaint.font.height) % 8))
            font_height_bytes++;
        current_char_width = 0;
        if (' ' != (uint8_t)c)
        {
            if (192 <= (uint8_t)c) // корректировка 'А' - руССкая (uint8_t)'А'
                c = (uint8_t)(c - (98));//98
            else
                c = (uint8_t)c - sPaint.font.pFontInfo->start_char;

            // Get width of current character width
            current_char_width = sPaint.font.pFontInfo->descr_array[c].char_width;
            
            // Get pointer to data
            bitmap_ptr = (const uint8_t *)&(sPaint.font.pFontInfo->font_bitmap_array[(sPaint.font.pFontInfo->descr_array[c].char_offset)]);
            __bitmap_ptr = bitmap_ptr; // Copy to temporary ptr
            for (k = 0; k < current_char_width; k++)
            { // Width of character
                for (i = 0; i < font_height_bytes; i++)
                { // Height of character
                    tmp = *__bitmap_ptr; // Get data to "shift"
                    __bitmap_ptr += current_char_width;
                    for (j = 0; j < 8; j++) { // Push byte
                        if ((tmp & 0x01))
                        {
                        //    if (vram_put_point(__p, mode) == GRAPH_ERROR) err = GRAPH_ERROR;};
                            paintBuf[i * 8 + j] = sPaint.fColor;
                        } else {
                            paintBuf[i * 8 + j] = sPaint.bColor;
                        }
                        tmp >>= 1;
                    }
                }
                hal_paint_fillBlock (x + k, y,
                    1, sPaint.font.height,
                    &paintBuf[0]);
                __bitmap_ptr = ++bitmap_ptr;
            }
        }
    }
    if (PAINT_FONT_MS == sPaint.font.type) // return coord for next symbol
        return (x + sPaint.font.width + sPaint.font.pFontInfo->hSpacing);
    else
        return (x + current_char_width + sPaint.font.pFontInfo->hSpacing);
#endif
}


coord_t paint_putStrXY (coord_t x, coord_t y, const char *str)
{
    while ('\0' != *str)
    {
        x = paint_putCharXY (x, y, *str++);
    }
    return x;
}
