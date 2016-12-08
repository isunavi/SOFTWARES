/**
 * @file    
 * @author  Ht3h5793, CD45
 * @date    21.11.2015  17.45
 * @version V1.1.0
 * @brief 

 */
 

#ifndef DRVPS2_KEYBOARD_H
#define	DRVPS2_KEYBOARD_H 20151121

#include "defines.h"

// #define CLOCK   2
// #define DATAPIN 3
//  
#define ISC00 0
#define ISC01 1


#ifdef	__cplusplus
extern "C" {
#endif

void drvPS2_keyboard_init (void);
char drvPS2_keyboard_getchar (void);
void drvPS2_keyboard_interrupt (void);
    
    
#ifdef	__cplusplus
}
#endif

#endif	/** DRVPS2_KEYBOARD_H */
