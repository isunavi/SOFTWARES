/**
 * @file    halI2C.h
 * @author  Ht3h5793, CD45
 * @date    13.6.2013  2:12
 * @version V6.0.2
 * @brief 

http://we.easyelectronics.ru/STM8/rabota-s-kontrollerom-lcd.html
 */

#ifndef HALLCD_STM8L_H
#define	HALLCD_STM8L_H 2

/**
 *  Раздел для "include"
 */
#include "defines.h"

/**
 *  Раздел для "define"
 */
#include "stdbool.h"
//определение порядка битов в символьных константах
#define A_ 6
#define B_ 2
#define C_ 5
#define D_ 4
#define E_ 0
#define F_ 7
#define G_ 3
#define H_ 15
#define J_ 14
#define K_ 10
#define M_ 1
#define N_ 12
#define P_ 8
#define Q_ 11
#define DP 13
#define COL 9

/**
 *  Раздел для "typedef"
 */

#ifdef	__cplusplus
extern "C" {
#endif

/**
 *  Раздел для прототипов функций
 */
    
//void    halI2C_init(void);



/***********************************************************************************
*   Function name :   LCD_Init
*   Returns :         нет       
*   Parameters :      нет    
*   Purpose :         инициализация контроллера LCD
************************************************************************************/
void LCD_Init(void);
/***********************************************************************************
*   Function name :   LCD_Contrast
*   Returns :         нет       
*   Parameters :      cntr - значение контрасности 0-7   
*   Purpose :         установка контрасности LCD
************************************************************************************/
void LCD_Contrast(unsigned char cntr);

/***********************************************************************************
*   Function name :   LCD_Char_Find
*   Returns :         cur_char - маскa символа
*   Parameters :      *с - код символа ASCII, 
*   Purpose :         определяет соответствующую символу маску
***********************************************************************************/
static unsigned int LCD_Char_Find(unsigned char* symb);

/***********************************************************************************
*   Function name :   LCD_Write_Char
*   Returns :         нет       
*   Parameters :      *ch - код символа, point - дес. точка, column - двоеточие,
		      position - порядковый номер знакоместа
*   Purpose :         Выводит символ на индикатор в соответствующем знакоместе
***********************************************************************************/
void LCD_Write_Char(unsigned char* ch, bool point, bool column, unsigned char position);

/***********************************************************************************
*   Function name :   LCD_Write_String
*   Returns :         нет       
*   Parameters :      * str - указатель на начало строки символов
*   Purpose :         Выводит строку
***********************************************************************************/
void LCD_Write_String(unsigned char *str);


void LCD_Scroll(unsigned char* str);


#ifdef	__cplusplus
}
#endif

#endif	/* HALLCD_STM8L_H */
