/**
 * CDDA ROGIE ST-WARRIOR
 */

#include "game_CDDA.h"
#include "modPaint.h"
#include "modKey_local.h"
#include "modKey.h"
#include "modSysClock.h"
#include "modRandom.h"

#include "conv.h"
#include "debug.h" // тут задержки

#include <stdlib.h>
#include <stdio.h>
#include "xprintf.h"

#include "ff.h"
FIL file_map;
FRESULT result;
FATFS FATFS_obj;
uint32_t str_size;
static const char file_path[] = "test.map";

uint32_t test_core_ticks_new, test_core_ticks_old = 0;
uint32_t test_total_time_remain = 0;

UINT frcnt, fwcnt;         // счетчик записи файла
extern char _str [4096]; //_str[LCD_H / 6]; // буфер для строк, общий буфер/ 6 - ширина минимального шрифта


typedef enum {
    DIR_RIGHT = 1,
    DIR_LEFT,
    DIR_DOWN,
    DIR_UP
} GAME_DIRECTION;



coord_t old_x, old_y;
systime_t game_repaint_delay;

int32_t wiev_offset_x, wiev_offset_y;
//uint16_t wiev_max_h, wiev_max_v;
//#define show_message_c(a)

uint8_t message_mode  = 0;
void show_message_debug (const char *str)
{
    paint_setColor (COLOR_WHITE);
    paint_setBackgroundColor (COLOR_BLACK);
    paint_gotoColRow (0, 0); 
    paint_putStr (str);
    vTaskDelay (500);
}

//for debug
uint32_t test_core_ticks_new, test_core_ticks_old;
static uint32_t *UID = (uint32_t *)0x1FFF7A10; // 0x1FFFF7E8


#pragma pack (push, 1)
typedef struct {
    coord_t  x;
    coord_t  y;
    uint8_t  dir;

    //systime_t tic_timeout;
    //pos_t food;

} mob_t; //
#pragma pack (pop)

mob_t mobs [4]; // 0 - plaer


#define GAME_MAP_BUF_SIZE_X         (16UL)
#define GAME_MAP_BUF_SIZE_Y         (16UL)
#define GAME_MAP_BUF_SIZE           (GAME_MAP_BUF_SIZE_Y * GAME_MAP_BUF_SIZE_X)
//#define GAME_MAP_BUF_SIZE_HALF      (GAME_MAP_BUF_SIZE / 2)

#define GAME_MAP_SIZE_X             (GAME_MAP_BUF_SIZE_X * 32UL)
#define GAME_MAP_SIZE_Y             (GAME_MAP_BUF_SIZE_Y * 32UL)
              
//TODO malloc
#pragma pack (push, 1)
typedef struct {
    uint8_t  c;
    uint32_t  *p;
} map_t; //
#pragma pack (pop)

map_t map [GAME_MAP_BUF_SIZE_Y * GAME_MAP_BUF_SIZE_X];

char      wiew_buf_chars [40][40]; //TODO

color_t  wiew_buf_colors [40][40];



// ------ BMP -------------------------------------------

// http://c-site.h1.ru/infa/bmp_struct.htm

// Заголовок файла suzeof - 14
#pragma pack (push, 1) // из-за того, что читаем прямо в структуру
typedef struct //__attribute__ ((aligned (1), packed)) 
{ 
    uint16_t  bfType;      // 'BM' = 0x4D42
    uint32_t  bfSize;      // Размер файла в байтах
    uint16_t  bfReserved1; // должны быть равны 0
    uint16_t  bfReserved2;
    uint32_t  bfOffBits;   // Смещение к растру
} BITMAPFILEHEADER_t ;

// Заголовок Bitmap syzeof - 40
typedef struct //__attribute__ ((aligned (1), packed)) 
{
    uint32_t  biSize;          // Размер заголовка BITMAP (в байтах) равно 40
    int32_t   biWidth;         // Ширина изображения в пикселях
    int32_t   biHeight;        // Высота изображения в пикселях
    uint16_t  biPlanes;        // Число плоскостей, должно быть 1
    uint16_t  biBitCount;      // Бит/пиксел: 1, 4, 8 или 24
    uint32_t  biCompression;   // Тип сжатия 0 - сжатие не используется
    uint32_t  biSizeImage;     // 0 или размер сжатого изображения в байтах
    int32_t   biXPelsPerMeter; // Горизонтальное разрешение, пиксел/м
    int32_t   biYPelsPerMeter; // Вертикальное разрешение, пиксел/м
    uint32_t  biClrUsed;       // Количество используемых цветов
    uint32_t  biClrImportant;  // Количество "важных" цветов.
} BITMAPINFOHEADER_t; //

// Cтруктура цвета
typedef struct //__attribute__ ((aligned (1), packed)) 
RGBQUAD {
    int8_t   rgbBlue;
    int8_t   rgbGreen;
    int8_t   rgbRed;
    int8_t   rgbReserved;
} RGBQUAD_t;
#pragma pack (pop)

//выделяем ресурсы
BITMAPFILEHEADER_t      bfh;
BITMAPINFOHEADER_t      bih;
//color_t bmpColorTable [256];

extern color_t paintBuf [LCD_BUF_SIZE];

/**
 * Декодер и вывод *.bmp файлов с карточки на LCD
 * @todo только кратные двойке размеры картинок!
 */
void paint_BMP (FIL *pf, coord_t x, coord_t y)
{
    uint16_t i, j;
    
    color_t *bmpColorTable = (color_t *) malloc (256 * sizeof (color_t)); // динамическое выделение памяти под символьную строку
    if (bmpColorTable == NULL) return;                 // если выделение памяти не выполнилось, завершить программу
    
    if (0 != pf->fs) //проверяем, что файл был открыт успешно
    {
        f_lseek (pf, 0); //ставим указатель чтения на начало
        // читаем структуры
        if (FR_OK != f_read (pf, (BYTE *)&bfh, sizeof (bfh), &frcnt))
        { 
            return;
        }
        if (FR_OK != f_read (pf, (BYTE *)&bih, sizeof (bih), &frcnt))
        { 
            return;
        }
        
        if (0x4D42 != bfh.bfType){ return; } // Это не BMP
        //if (0 == bfh.bfSize) { SDCARD_READ_ERROR; } // размер файла нулевой

        if (8 == bih.biBitCount) //256 colors
        {
            f_lseek (pf, sizeof (bih) + sizeof (bfh));
            
            for (j = 0; j < 4; j++) //вычитываем таблицу цветов
            {
                if (FR_OK != f_read (pf, (BYTE *)&_str [0], 256, &frcnt))
                {
                    return; //TODO
                }
                for (i = 0; i < 64; i++) //пакуем из четырех байт в один
                {
                    bmpColorTable [j*64 + i] = paint_RGB888_RGB16 (_str [i*4 +2], _str [i*4 +1], _str [i*4 +0]);
                }
            }

            /*
            if (bih.biWidth == 128) //некоторая оптимизация
            {

                for (j = 0; j < bih.biHeight /4; j++) //в четыре этапа
                {
                    if (FR_OK != f_read (pf, (BYTE *)&MMC_Buf[0], 512, (UINT *)&read_size)) //bih.biWidth
                    {
                    }
                    for (i = 0; i < 512; i++) // конвертруем через таблицу
                    {
                        MMC_Buf[i] = bmpColorTable[MMC_Buf[i]];
                    }
                    halLCD_fillBlockDMA (x, y + bih.biHeight - (j*4 +0), bih.biWidth, 1, &MMC_Buf[0]);
                    halLCD_fillBlockDMA (x, y + bih.biHeight - (j*4 +1), bih.biWidth, 1, &MMC_Buf[128]);
                    halLCD_fillBlockDMA (x, y + bih.biHeight - (j*4 +2), bih.biWidth, 1, &MMC_Buf[256]);
                    halLCD_fillBlockDMA (x, y + bih.biHeight - (j*4 +3), bih.biWidth, 1, &MMC_Buf[384]);
                }
            }
            else
            */
            {
                for (j = 0; j < bih.biHeight; j++)
                {
                    if (FR_OK != f_read (pf, (BYTE *)&_str [0], bih.biWidth, &frcnt)) //bih.biWidth
                    {
                    }

                    for (i = 0; i < bih.biWidth; i++) // конвертруем через таблицу
                    {
                        paintBuf [i] = bmpColorTable [_str [i]];
                    }
                    hal_paint_fillBlock (x, y + bih.biHeight - j, bih.biWidth, 1, &paintBuf [0]);
                }
            }    
        }
        else if (2 == bih.biBitCount) //черно-белый BMP
        {
            
        }
    }
    else
    {
        xsprintf (_str, "bmp draw error");
        paint_putStrXY (x, y, _str);
    }
    
    free (bmpColorTable);
}


//------------------------------------------------------------------------------
uint32_t rng (uint32_t val1, uint32_t val2)
{
    uint32_t minVal = ( val1 < val2 ) ? val1 : val2;
    uint32_t maxVal = ( val1 < val2 ) ? val2 : val1;
    /*
    return minVal + 
      (uint32_t)
        ((uint32_t)
         (maxVal - minVal + 1) * 
           _rand32 () ) / 0xFFFFFFFF;
    */
    return minVal + _rand32 () % (maxVal - minVal);
}


bool_t one_in (uint32_t chance )
{
    return (chance <= 1 || rng (0, chance - 1) == 0);
}


int dice (int number, int sides)
{
    int ret = 0;
    for (int i = 0; i < number; i++)
    {
        ret += rng( 1, sides );
    }
    return ret;
}


uint32_t map_old_pos_x, map_old_pos_y;

bool_t game_set (uint32_t posX, uint32_t posY, char ch, uint8_t valA)
{
    if ((0 <= posX) && (0 <= posY) && (GAME_MAP_BUF_SIZE_X > posX) && (GAME_MAP_BUF_SIZE_Y > posY))
    {
        /*
        if (((posX / GAME_MAP_BUF_SIZE_X) != map_old_pos_x) ||
            ((posX / GAME_MAP_BUF_SIZE_Y) != map_old_pos_y))
        {
            map_old_pos_x = posX;
            map_old_pos_x = posX;
            result = f_lseek (&file_map, GAME_MAP_BUF_SIZE_Y * GAME_MAP_BUF_SIZE_X * 2 * posX * posY);
            result = f_read (&file_map, &map[0][0], 
                GAME_MAP_BUF_SIZE_X * GAME_MAP_BUF_SIZE_X * 2, &frcnt);
        }
      */
        map [(GAME_MAP_SIZE_X * posY) + posX].c = ch;// | (uint16_t)(valA << 8);// | (uint32_t)(valB << 16);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


map_t  *game_get (uint32_t posX, uint32_t posY)
{
    if ((0 <= posX) && (0 <= posY) && (GAME_MAP_SIZE_X > posX) && (GAME_MAP_SIZE_Y > posY))
    {
        /*
        if (((posX / GAME_MAP_BUF_SIZE_X) != map_old_pos_x) ||
            ((posX / GAME_MAP_BUF_SIZE_Y) != map_old_pos_y))
        {
            map_old_pos_x = posX;
            map_old_pos_x = posX;
            result = f_lseek (&file_map, GAME_MAP_BUF_SIZE_Y * GAME_MAP_BUF_SIZE_X * 2 * posX * posY);
            result = f_read (&file_map, &map[0][0], 
                GAME_MAP_BUF_SIZE_X * GAME_MAP_BUF_SIZE_X * 2, &frcnt);
        }
      */
        return &map [(GAME_MAP_SIZE_X * posY) + posX];
    }
    else
    {
        return NULL;
    }
}


void game_create_bilding (uint32_t posX, uint32_t posY, uint16_t sizeX, uint16_t sizeY, uint8_t type)
{
    uint32_t i;
    
    sizeX += 3; //house minimal size
    sizeY += 3;
    
    //corners
    game_set (posX, posY, 130, 100);
    game_set (posX, posY + sizeY, 131, 100);
    game_set (posX + sizeX, posY, 132, 100);
    game_set (posX + sizeX, posY + sizeY, 133, 100);
    
    for (i = 1; i < sizeX; i++)
    {
        game_set (posX + i, posY, 128, 100);
    }
    for (i = 1; i < sizeX; i++)
    {
        game_set (posX + i, posY + sizeY, 128, 100);
    }
    for (i = 1; i < sizeY; i++)
    {
        game_set (posX, posY + i, 129, 100);
    }
    for (i = 1; i < sizeY; i++)
    {
        game_set (posX + sizeX, posY + i, 129, 100);
    }
    
    //create window
    //...
    //create doors
    game_set (posX + 2, posY, '+', 100);
    game_set (posX + 2, posY + sizeY, '-', 100);
}


void game_create_likes (uint32_t posX, uint32_t posY, uint16_t sizeX, uint16_t sizeY, uint8_t type)
{
    uint32_t i,j, v;
    
    sizeX += 2; //house minimal size
    sizeY += 2;
    
    //corners
    game_set (posX, posY, '~', 100);
    for (i = 1; i < sizeX; i++)
    {
        for (i = 1; i < sizeX; i++)
        {
            v = rng (0, i);
            if (0 == v)
                game_set (posX + i, posY, '~', 100);
        }
    }
}


bool_t game_seek (uint32_t x, uint32_t y, uint32_t post)
{
    uint32_t pos;
    
    pos = (GAME_MAP_BUF_SIZE * y * GAME_MAP_SIZE_Y / GAME_MAP_BUF_SIZE_Y) + (GAME_MAP_BUF_SIZE * x);
    pos += (GAME_MAP_BUF_SIZE * post / 2);
    pos *= 2;
    return f_lseek (&file_map, pos);
}


void draw_battary (uint8_t value) //darw battery
{
    coord_t x,y;
    x = 400;
    y = 300;
#define BATT_W 20
#define BATT_H 10

    
    paint_setColor (COLOR_BLUE);
    paint_rect (x, y, BATT_W, BATT_H);
    paint_rect (x + BATT_W + 1, y + 4, 4, BATT_H - 4);
    
    if (value < 20)
        paint_setColor (COLOR_RED);
    else if (value < 50)
        paint_setColor (COLOR_WHITE);
    else if (value < 80)
        paint_setColor (COLOR_GREEN);
    
    //value = (uint16_t)((value * BATT_W) % 100);

    if (value >= BATT_W) value = BATT_W;
    if (value == 0) value = 1;

    paint_rectFill (x + 2, y + 2, value, BATT_H - 2);
    paint_rectFill (x + 2 + value, y + 2,  BATT_W - value, BATT_H - 2);

    xsprintf (_str, "%03u\n%03u\n%03u", value,
                            value * BATT_W,
                            (value * BATT_W) / 100);

    paint_putStrColRow (0, 45, _str);
}


void game_CDDA_init (void)
{
    uint32_t tmp32, i, j, a, b, c;
    uint32_t ii, jj, ix, iy;
    char tmpc;
    
    //init random
    _srand ();
    //_set_seed (23444);
    
    modKey_init ();
    
    game_repaint_delay = modSysClock_getTime ();
    
    paint_init (LCD_ORIENTATION_GLOBAL);
    paint_setBackgroundColor (COLOR_BLACK);
    paint_clearScreen ();
    paint_setColor (COLOR_WHITE);
    paint_setFont (PAINT_FONT_Generic_8pt, PAINT_FONT_MS);
    
    paint_putStrColRow (0, 0, "set coord");
    mobs[0].x = GAME_MAP_SIZE_X / 2;
    mobs[0].y = GAME_MAP_SIZE_Y / 2;
    
    /*
    if (0 != paint_getMaxCol () & (paint_getMaxCol () - 1)) //if nechotnoe TODO
        wiev_max_v = paint_getMaxCol ();
    else
        wiev_max_v = paint_getMaxCol () - 0;
    
    if (0 != paint_getMaxRow () & (paint_getMaxRow () - 1))
        wiev_max_h = paint_getMaxRow () - 2; //8+ 1 + 8; // - 2; //for status
    else
        wiev_max_h = (paint_getMaxRow () - 1) - 2;
    */
    
    
    // init FAT
    result = f_mount (&FATFS_obj, "0", 1);
    if ((FR_NOT_READY == result) ||
        (FR_DISK_ERR == result) ||
        (FR_INVALID_DRIVE == result))
    {
        show_message_debug ("   SD/FAT init ERROR");
    }

    if (FR_NO_FILESYSTEM == result)
    {
        result = f_mkfs ("0", 0, SPIFLASH_BLOCK_SIZE);
        if (FR_OK != result)
        {
            show_message_debug ("FAT reinit");
        }
    }
    
    RESET_CORE_COUNT;
    test_core_ticks_old = modSysClock_getTime();
    // Повторное открытие файла
    result = f_open (&file_map, "256.bmp", FA_OPEN_EXISTING | FA_READ);
    if (FR_OK == result)
    {
        paint_BMP (&file_map, 0, 0);
        
        /*
        uint32_t full_size = 0;
        do
        {
            result = f_read (&file_map, (BYTE *)_str, 4096, &frcnt);
            full_size += frcnt;
        }
        while ((result == FR_OK) && (frcnt == 4096));

        test_core_ticks_new = GET_CORE_COUNT;
        if (test_core_ticks_new > test_core_ticks_old)
        {
            //test_core_ticks_old = test_core_ticks_new;
        }
        
        xsprintf (_str, "Rd MP3 4:%u ms", (modSysClock_getTime() - test_core_ticks_old) / 1000);
        show_message_debug (_str);
        paint_putStrColRow (0, 6, _str);
        */
    }
     
    result = f_close (&file_map);
    paint_clearScreen ();
    
    xsprintf (_str, "ID0:0x%08X ", UID [0]);
    paint_putStrColRow (0, 7, _str); 
    xsprintf (_str, "ID1:0x%08X ", UID [1]);
    paint_putStrColRow (0, 8, _str); 
    xsprintf (_str, "ID2:0x%08X ", UID [2]);
    paint_putStrColRow (0, 9, _str); 
    
    
    uint32_t test_cnt = 0;
    if (FR_OK == result)
    {
        show_message_debug ("   SD/FAT init OK");
        result = f_open (&file_map, "map1.txt", FA_CREATE_ALWAYS | FA_WRITE);
        if (FR_OK != result)
        {

        } else
        {
            show_message_debug ("   FILE open OK");
            result = f_lseek (&file_map, 0);
            if (FR_OK != result)
            {
                show_message_debug ("   FILE seek ERROR");
                while (1) {};
            }
            else
            {
                show_message_debug ("   FILE seek OK");
                test_total_time_remain = HAL_GetTick ();

 
                for (jj = 0; jj < GAME_MAP_SIZE_Y / GAME_MAP_BUF_SIZE_Y; jj++)
                {
                    for (ii = 0; ii < GAME_MAP_SIZE_X / GAME_MAP_BUF_SIZE_X; ii++)
                    {
                        
                        ix = GAME_MAP_BUF_SIZE_X * ii;
                        iy = GAME_MAP_BUF_SIZE_Y * jj;
                        
                        
                        for (j = 0; j < GAME_MAP_BUF_SIZE_Y; j++)
                        {
                            for (i = 0; i < GAME_MAP_BUF_SIZE_X; i++)
                            {
                                //clear
                                if (one_in (25)) //25 > (_rand32 () % 1000))
                                {
                                    tmp32 = 2;
                                }
                                else
                                {
                                    tmp32 = 0;
                                }
                                game_set (ix + i, iy + j, '.', tmp32);
                            }
                        }

                        //--------------------------------------------------------------------------
                        /*
                        paint_putStrColRow (0, 2, "creating map objects");
                        
                        for (j = 0; j < GAME_MAP_BUF_SIZE_Y; j++)
                        {
                            for (i = 0; i < GAME_MAP_BUF_SIZE_X; i++)
                            {
                                //wall

                                //flowers
                                if (5 > (_rand32 () % 1000))
                                {
                                    game_set (i, j, 'f', _rand32 () % 4);
                                }
                                
                                //trees
                                if (35 > (_rand32 () % 1000))
                                {
                                    tmp32 = _rand32 () % 3;
                                    
                                    switch (tmp32)
                                    {
                                    case 0: game_set (i, j, '1', _rand32 () % 20); break;
                                    case 1: game_set (i, j, '7', _rand32 () % 40); break;
                                    default: break;
                                    }
                                }
                            }
                        }
                        */
                        //str_size = xsprintf (_str, "test writings");
                        //result = game_seek (ii, jj, 0);
                        
                        
                        result = f_write (&file_map, &map[0].c, 
                            GAME_MAP_BUF_SIZE * sizeof (map_t), &fwcnt);
                        if ((FR_OK != result) || (fwcnt < GAME_MAP_BUF_SIZE)) //ошибка, если диск переполнен
                        {
                            show_message_debug ("FILE wr ERROR");
                            while (1) {};
                        }
                        else
                        {
                            // wr block ok!
                        }
                        
                        
                        result = f_write (&file_map, "test test test", 
                            15, &fwcnt);
                        if ((FR_OK != result) || (fwcnt < 15)) //ошибка, если диск переполнен
                        {
                            show_message_debug ("FILE wr ERROR");
                            while (1) {};
                        }
                        else
                        {
                            // wr block ok!
                        }
                        
                        test_cnt++;
                        xsprintf (_str, "test_cnt:%u ", test_cnt);
                        paint_putStrColRow (0, 10, _str); 
                        
                        
                    } //
                } //
                                            
                result = f_close (&file_map); //Update file size in here
                if (result != FR_OK)
                {
                    show_message_debug ("FILE close ERROR");
                }
                
            }
        }
    }
  

    
    //--------------------------------------------------------------------------
    paint_putStrColRow (0, 3, "create bildings");
    
    a = rng (1, 5);
    for (i = 0; i < a; i++)
    {
        game_create_bilding ( rng(0, GAME_MAP_BUF_SIZE_X), rng (0, GAME_MAP_BUF_SIZE_Y), rng (1, 7), rng (1, 7), 10);
    }
    
    //--------------------------------------------------------------------------
    paint_putStrColRow (0, 4, "create likes");
    
    a = rng (1, 3);
    for (i = 0; i < a; i++)
    {
        game_create_likes ( rng(0, GAME_MAP_BUF_SIZE_X), rng (0, GAME_MAP_BUF_SIZE_Y), rng (1, 7), rng (1, 17), 10);
    }
    
    //create borders
    //game_create_bilding ( 0, 0, GAME_MAP_BUF_SIZE_X - 2, GAME_MAP_SIZE_Y - 2, 128);

    vTaskDelay (500); //for debug
    paint_clearScreen ();
}


void game_CDDA_run (void)
{
    int32_t i,j;
    
    while (1)
    {
        
        //AI
      
        // keys
        modKey_run ();
        
        
        // compute video buffer 
        for (j = 0; j < GAME_MAP_BUF_SIZE_Y; j++)
        {
            for (i = 0; i < GAME_MAP_BUF_SIZE_X; i++)
            {
                uint16_t tmp16 = map [(GAME_MAP_BUF_SIZE_X * (j + wiev_offset_y)) + i + wiev_offset_x].c;
                uint8_t ch = 0x00FF & tmp16;
                uint8_t ch_H = (0xFF00 & tmp16) >> 8;
                color_t color;
                switch (ch)
                {
                //case '@': color = COLOR_WHITE; break;
                //character
                case 'Z': color = COLOR_GREEN; break;
                
                //wals
                case 128: case 129: case 130: case 131: case 132: case 133:
                case '-':
                case '=':
                case '+': color = COLOR_GRAY; break;
                
                //water
                case '~': color = COLOR_BLUE; break;
                
                //flora --------------------------------------------------------
                case 'f': //flowers 
                    if (ch_H)
                        color = COLOR_RED;
                    else
                        color = COLOR_ORANGE; break;
                        
                case '#': // cust
                case '1': // trees
                case '7': color = COLOR_GREEN; break;
                
                case '.':
                    switch (ch_H)
                    {
                    case 0: color = COLOR_GREEN; break;
                    case 1: color = COLOR_GRAY; break;
                    case 2: color = COLOR_BROWN; break;
                    case 3: color = COLOR_YELLOW; break;
                    default: color = COLOR_GREEN; break;
                    }
                    break;
                        
                
                default: color = COLOR_VIOLET; break; //error color
                }
                
                wiew_buf_colors [j][i] = color;
                wiew_buf_chars [j][i] = ch;
            }
        }
        /*
        paint_setColor (COLOR_RED);
        paint_gotoColRow (0, wiev_max_h - 1); 
        xprintf ((wiew_buf_chars [wiev_max_h - 1][0],"HP: 23 AC:4 G:4");
        paint_gotoColRow (0, wiev_max_h - 0); 
        paint_putStr ("Lvl:22 Xp:15");
            //"
        //>You have a exp!
        
        */
        
        // our character
        wiew_buf_colors [GAME_MAP_BUF_SIZE_Y / 2][GAME_MAP_BUF_SIZE_X / 2] = COLOR_WHITE;
        wiew_buf_chars [GAME_MAP_BUF_SIZE_Y / 2][GAME_MAP_BUF_SIZE_X / 2] = '@';
        

        if (FUNCTION_RETURN_OK == modSysClock_timeout (&game_repaint_delay, 100, SYSCLOCK_GET_TIME_MS_1))
        {
            //keys
            old_x = mobs[0].x;
            old_y = mobs[0].y;
            
            if ((MODKEY_STATE_PRESSED == modKey_getState (KEY_UP)) || (MODKEY_STATE_HOLD == modKey_getState (KEY_UP)))
            {
                if (GAME_MAP_SIZE_Y <= ++mobs[0].y)
                    mobs[0].y = GAME_MAP_SIZE_Y - 1;
            }
            if ((MODKEY_STATE_PRESSED == modKey_getState (KEY_DOWN)) || (MODKEY_STATE_HOLD == modKey_getState (KEY_DOWN)))
            {
                if (0 > --mobs[0].y)
                    mobs[0].y = 0;
            }
            if ((MODKEY_STATE_PRESSED == modKey_getState (KEY_RIGHT)) || (MODKEY_STATE_HOLD == modKey_getState (KEY_RIGHT)))
            {
                if (0 > --mobs[0].x)
                    mobs[0].x = 0;
            }
            if ((MODKEY_STATE_PRESSED == modKey_getState (KEY_LEFT)) || (MODKEY_STATE_HOLD == modKey_getState (KEY_LEFT)))
            {
                if (GAME_MAP_SIZE_X <= ++mobs[0].x)
                    mobs[0].x = GAME_MAP_SIZE_X - 1;
            }
            if ((MODKEY_STATE_PRESSED == modKey_getState (KEY_OK)) || (MODKEY_STATE_HOLD == modKey_getState (KEY_OK)))
            {
                
            }
            
            if (mobs[0].x > GAME_MAP_BUF_SIZE_X / 2)
                wiev_offset_x = mobs[0].x - GAME_MAP_BUF_SIZE_X / 2;
            else
                wiev_offset_x = mobs[0].x;
            
            if (mobs[0].y > GAME_MAP_BUF_SIZE_Y / 2)
                wiev_offset_y = mobs[0].y - GAME_MAP_BUF_SIZE_Y / 2;
            else
                wiev_offset_y = mobs[0].y;
            
            if (mobs[0].x <= (GAME_MAP_SIZE_X - GAME_MAP_BUF_SIZE_X / 2))
                wiev_offset_x = mobs[0].x - GAME_MAP_BUF_SIZE_X / 2;
            else
                wiev_offset_x = mobs[0].x;
            
            if (mobs[0].y <= (GAME_MAP_SIZE_Y - GAME_MAP_BUF_SIZE_Y / 2))
                wiev_offset_y = mobs[0].y - GAME_MAP_BUF_SIZE_Y / 2;
            else
                wiev_offset_y = mobs[0].y;
        
            // redraw display
            for (j = 0; j < GAME_MAP_BUF_SIZE_Y; j++)
            {
                for (i = 0; i < GAME_MAP_BUF_SIZE_X; i++)
                {
                    paint_setColor (wiew_buf_colors [j][i]);
                    paint_putCharColRow (i, j, wiew_buf_chars [j][i]);
                }
            }
        }

    }
}


void game_CDDA_paint (void)
{

    
}
