/**
 * ads7843.h
 *
 *  Created on: 11.01.2013
 *      Author: Andrey Fursov
 */

#ifndef ADS7843_H_
#define ADS7843_H_

#include "board.h"

#define EE_ADR_COEF	0x10


#ifdef	__cplusplus
extern "C" {
#endif


void drvADS7843_init (void);
bool_t drvADS7843_touchGetXY (uint16_t *x, uint16_t *y, bool_t flag);

void touchSetCoef(int16_t , int16_t , int16_t , int16_t );
uint16_t touchVerifyCoef(void);

uint16_t touchGetX(void);
uint16_t touchGetY(void);
uint16_t getTouchState(void);

void dvrASD7843_calibrate (void);
    
#ifdef	__cplusplus
}
#endif

#endif /* ADS7843_H_ */
