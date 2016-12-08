/*
 * @file    LCD grafic
 * @author  Ht3h5793
 * @date    04.04.2014
 * @version V0.0.1
 * @brief  
 *
*/

#ifndef MODGUI_H
#define	MODGUI_H 2 /* Revision ID */

#include "defines.h"
#include "modPaint.h"
#include "modPaint_local.h"

typedef enum {
    MODGUI_INIT_OK                = 0, // успешная отработка
    MODGUI_FUNCTION_RETURN_ERROR     , // ошибка

} MODPAINT_RESPOND;


class cGUI
{
public:

    msg_t    init(void);

    msg_t    run(uint8_t key);

private:
    struct CURSOR_ {
        uint8_t    cursor;
        uint16_t   x;
        uint16_t   y;
        uint32_t   timer;
    }CURSOR;

//    cPaint guiPaint;


};

#endif	/* MODGUI_H */
