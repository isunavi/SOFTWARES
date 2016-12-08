#include "game_worm.h"
#include "modPaint.h"
#include "modKey.h"
#include "modSysClock.h"
#include "modRandom.h"

#include "conv.h"
#include "debug.h" // тут задержки


typedef enum {
    DIR_RIGHT = 1,
    DIR_LEFT,
    DIR_DOWN,
    DIR_UP
} GAME_DIRECTION;

typedef struct {
    int16_t x;
    int16_t y;
} pos_t;

typedef struct {
    coord_t  x;
    coord_t  y;
    uint8_t  dir;
    uint32_t  size;
    systime_t tic_timer;
    systime_t tic_timeout;
    pos_t body[256];
    pos_t food;
    uint32_t score;
    uint16_t sound;
    systime_t sound_timer;
} worm_t; //
worm_t game;

#define WORM_SIZE 16


void  worm_food_new (void) {
    game.food.x = _rand32 () % (paint_getWidth () / WORM_SIZE) * WORM_SIZE;
    game.food.y = _rand32 () % (paint_getHeight () / WORM_SIZE) * WORM_SIZE;
}


msg_t  worm_collision (void) {
    uint32_t i;
    msg_t respond = FALSE;
    
//     for (i = 0; i < game.size; i++) {
//         if ((game.y == game.body[i].x) && (game.y == game.body[i].y)) {
//             respond = TRUE;
//             break;
//         }
//     }
    return respond;
}


void game_worm_init (void) {
    uint32_t i;
    
    game.x = WORM_SIZE * 6; //paint_getWidth() / 2;
    game.y = WORM_SIZE * 4; //paint_getHeight() / 2;
    game.size = 4;
    for (i = 0; i < 256; i++) {
        game.body[i].x = 1111;
        game.body[i].y = 1111;
    }
    for (i = 0; i < game.size; i++) {
        game.body[i].x = game.x;
        game.body[i].y = game.y;
    }
    game.tic_timer = 0;
    game.tic_timeout = 200;
    game.dir = DIR_RIGHT;
    game.score = 0;
    // 
    worm_food_new ();
    game.sound = 0;
    //
    paint_setBackgroundColor (COLOR_BLACK);
    paint_clearScreen ();
    paint_repaint ();

    paint_setBackgroundColor (COLOR_BLACK);
    paint_clearScreen ();
    paint_repaint ();
}


void game_worm_run (void) {
    uint32_t i;
    char strr[32];
    
    if (MODKEY_STATE_PRESSED == modKey_getState (KEY_RIGHT)) {
        game.dir = DIR_RIGHT;
    }
    if (MODKEY_STATE_PRESSED == modKey_getState (KEY_LEFT)) {
        game.dir = DIR_LEFT;
    }
    if (MODKEY_STATE_PRESSED == modKey_getState (KEY_DOWN)) {
        game.dir = DIR_DOWN;
    }
    if (MODKEY_STATE_PRESSED == modKey_getState (KEY_UP)) {
        game.dir = DIR_UP;
    }
    if (MODKEY_STATE_PRESSED == modKey_getState (KEY_OK)) {
        
    }

    
    if (modSysClock_getPastTime (game.tic_timer, SYSCLOCK_GET_TIME_MS_1) > game.tic_timeout) {   
        game.tic_timer = modSysClock_getTime();
//     // clear all painting
//     for (i = 0; i < game.size; i++) { // body
//         halLCD_fillBlockColor (game.body[i].x, game.body[i].y, WORM_SIZE, WORM_SIZE, COLOR_BLACK);
//     }
//     halLCD_fillBlockColor (game.x, game.y, WORM_SIZE, WORM_SIZE, COLOR_BLACK); // head
//     halLCD_fillBlockColor (game.food.x, game.food.y, WORM_SIZE, WORM_SIZE, COLOR_BLACK);
//     
        paint_setBackgroundColor (COLOR_BLACK);
        paint_clearScreen ();
        // move
        switch (game.dir) {
            case DIR_RIGHT:
                game.x = game.x + WORM_SIZE;
                break;
            
            case DIR_LEFT:
                game.x = game.x - WORM_SIZE;
                break;
            
            case DIR_DOWN:
                game.y = game.y + WORM_SIZE;
                break;
            
            case DIR_UP:
                game.y = game.y - WORM_SIZE;
                break;
        }
        if (game.x >= paint_getWidth () || 
            game.y >= paint_getHeight () || 
            game.x < 0 || 
            game.y < 0 ||
            worm_collision ()) {
            _delay_ms (1000);
            game_worm_init ();
        }
        for (i = (game.size - 1); i >= 1; i--) {
            game.body[i].x = game.body[i -1].x;
            game.body[i].y = game.body[i -1].y;
        }
        game.body[0].x = game.x;
        game.body[0].y = game.y;
        if ((game.x == game.food.x) && 
            (game.y == game.food.y)) {
            paint_setColor (COLOR_BLACK);
            paint_rectFill (game.food.x, game.food.y, WORM_SIZE, WORM_SIZE);
            worm_food_new ();
            game.score++;
            game.size++;
            if (1) {
                if (game.tic_timeout > 8) {
                    game.tic_timeout -= 4;
                }
            }
            game.sound = 4;
        }

        // repaint body
        paint_setColor (COLOR_BLUE);
        for (i = 0; i < game.size; i++) {
            paint_rectFill (game.body[i].x, game.body[i].y, WORM_SIZE, WORM_SIZE);
        }
        paint_setColor (COLOR_GREEN);
        paint_rectFill (game.x, game.y, WORM_SIZE, WORM_SIZE); // repaint head
        paint_setColor (COLOR_RED);
        paint_rectFill (game.food.x, game.food.y, WORM_SIZE, WORM_SIZE); // repaint food
        
        
        paint_setColor (COLOR_WHITE);
        paint_setFont (PAINT_FONT_Generic_8pt);
        zsprintf (strr, "Score:%08u", game.score);
        paint_putStrXY (0, 0, strr);
        
        paint_repaint ();

    }
    
    if (modSysClock_getPastTime (game.sound_timer, SYSCLOCK_GET_TIME_MS_1) > 10) {   
        game.sound_timer = modSysClock_getTime();
        // sound
        if (0 != game.sound) {
            game.sound--;
            TIM_Cmd (TIM3, ENABLE); // запускаем PWM
        } else {
            TIM_Cmd (TIM3, DISABLE);
        }
    }
}

