#include "modGUI.h"
#include "modPaint.h"
#include "defines.h"

#include "modSysClock.h"

msg_t    cGUI::init(void)
{
    CURSOR.x = 4;
    CURSOR.y = 2;
    CURSOR.cursor = 0;
    CURSOR.timer = modSysClock_getTime();
    
    return FUNCTION_RETURN_OK;
}

msg_t    cGUI::run(uint8_t key)
{
//     guiPaint.setBackgroundColor(COLOR_BLUE);
//     guiPaint.clearScreen();
//     guiPaint.setColor(COLOR_WHITE);
//     uint32_t i;
//     for (i = 1; i < (TEXT_MAX_COL - 1); i++)
//     {
//         guiPaint.putCharXY( i, 0, 128);
//     }
//     for (i = 1; i < (TEXT_MAX_COL - 1); i++)
//     {
//         guiPaint.putCharXY( i, TEXT_MAX_ROW - 1, 128);
//     }
//     for (i = 1; i < (TEXT_MAX_ROW - 1); i++)
//     {
//         guiPaint.putCharXY( 0, i, 129);
//     }
//     for (i = 1; i < (TEXT_MAX_ROW - 1); i++)
//     {
//         guiPaint.putCharXY( TEXT_MAX_COL - 1, i, 129);
//     }
//     guiPaint.putCharXY( 0, 0, 130);
//     guiPaint.putCharXY( 0, TEXT_MAX_ROW - 1, 131);
//     guiPaint.putCharXY( TEXT_MAX_COL - 1, 0, 132);
//     guiPaint.putCharXY( TEXT_MAX_COL - 1, TEXT_MAX_ROW - 1, 133);

//     // рисуем титл
//     const char strMain[] = {"BOOT"};
//     guiPaint.setColor(0x4333);
//     uint16_t size = sizeof(strMain);
//     guiPaint.putStringXY( ((TEXT_MAX_COL - 1) / 2) - (size / 2, 0), 0, (char *)&strMain[0]);
//     
//     // пишем элементы меню
//     guiPaint.setColor(COLOR_GREEN);
//     guiPaint.putStringXY( 1, 1, (char *)"ERROR!");
//     guiPaint.putStringXY( 1, 2, (char *)"ОШИБКА!");
//     
//     guiPaint.putCharXY(1, 5, key);
//     
//     // мигаем курсором :)
//     if (modSysClock_getPastTime(CURSOR.timer, SYSCLOCK_GET_TIME_MS_1) > 500)
//     {
//         CURSOR.timer = modSysClock_getTime();
//         if (1 == CURSOR.cursor)
//         {
//             CURSOR.cursor = 0;
//             guiPaint.putCharXY( CURSOR.x, CURSOR.y, ' ');
//         }
//         else
//         {
//             CURSOR.cursor = 1;
//             guiPaint.putCharXY( CURSOR.x, CURSOR.y, '-');
//         }
//     }
}
