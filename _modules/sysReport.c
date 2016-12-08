#include "sysReport.h"
#include "board.h"

#if BOARD_STM32F4DISCOVERY

#ifdef HT3_PAINT
    #include "halPaint.h"
    #include "modPaint.h"
#else
    #include "gfx.h"
#endif
    
#ifdef HT3_PAINT
#else
extern font_t my_font;
#endif
#include "xprintf.h"
extern char _str [4096];

#endif

int printf(const char *_Restrict, ...);

//http://catethysis.ru/predefined-macros/
void fERROR_ACTION (uint16_t code, char *module, uint32_t position)
{
#if BOARD_STM32F4DISCOVERY
    xsprintf (_str, "E:%X,%u", code, position);
    #ifdef HT3_PAINT
        paint_setColor (COLOR_ERROR);
        paint_putStrXY (PAINT_POSX_1, PAINT_POSY_3T, _str);
    #else
        gdispDrawString (PAINT_POSX_1, PAINT_POSY_3T, _str, my_font, COLOR_ERROR);
    #endif
        printf("\n\nError:%d,%s,%u", code, module, position);
#endif
    while(1)
    {
        //HAL_LED_ERROR_Toggle;
        HAL_Delay (1000);
    }
}

void fWARNING_ACTION (uint16_t code, char *module, uint32_t position)
{
#if BOARD_STM32F4DISCOVERY
    xsprintf (_str, "W:%X,%u", code, position);
    #ifdef HT3_PAINT
        paint_setColor (COLOR_WARNING);
        paint_putStrXY (PAINT_POSX_1, PAINT_POSY_3T, _str);
    #else
        gdispDrawString (PAINT_POSX_1, PAINT_POSY_3T, _str, my_font, COLOR_WARNING);
    //    printf("\n\nWarning:%d,%s,%u", code, module, position);
    #endif
#endif
    //HAL_Delay (300);
}


