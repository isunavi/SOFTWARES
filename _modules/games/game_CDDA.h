/**
 * @file    
 * @author  Ht3h5793, CD45
 * @date    02.11.2016  8.19
 * @version V1.0.0
 * @brief 


    game_CDDA_init ();
    game_CDDA_run ();


 */
 

#ifndef GAME_CDDA_H
#define	GAME_CDDA_H 20161102

/**
 *  ������ ��� "include"
 */
#include "board.h"

/**
 *  ������ ��� "define"
 */

/**
 *  ������ ��� "typedef"
 */

#ifdef	__cplusplus
extern "C" {
#endif

/**
 *  ������ ��� ���������� �������
 */
void game_CDDA_init (void);
void game_CDDA_run (void);
void game_CDDA_paint (void);

#ifdef	__cplusplus
}
#endif

#endif	/** GAME_CDDA_H */
