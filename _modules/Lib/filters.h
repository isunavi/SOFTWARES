/* 
 * File:   
 * Author:
 *
 * Created on 29 Март 2013 г., 11:00
 
 http://we.easyelectronics.ru/Soft/sigma-tochechnyy-filtr-kalmana.html
 */

#ifndef FILTERS_H
#define	FILTERS_H 20160702
/**
 *  Раздел для "include"
 */
#include "board.h"


/**
 *  Раздел для "define"
 */

typedef struct _filter_Kalman_16bit_t {
    float X0; // predicted state
    float P0; // predicted covariance
    
    int16_t F; // factor of real value to previous real value
    int16_t H; // factor of measured value to real value
    int16_t Q; // measurement noise
    int16_t R; // environment noise
    
    int16_t state;
    float covariance;
}filter_Kalman_16bit_t;



#ifdef	__cplusplus
extern "C" {
#endif

void filter_Kalman_init (filter_Kalman_16bit_t *pf,
    int16_t f,
    int16_t h,
    int16_t q,
    int16_t r);


void    filter_Kalman_init_default (filter_Kalman_16bit_t *pf);

void    filter_Kalman_set_state (filter_Kalman_16bit_t *pf, int16_t  _state, float _covariance);

int16_t filter_Kalman_correct (filter_Kalman_16bit_t *pf, int16_t value);

    
#ifdef	__cplusplus
}
#endif

#endif	/* FILTERS_H */

