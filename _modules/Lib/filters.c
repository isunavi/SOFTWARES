#include "filters.h"


void filter_Kalman_init (filter_Kalman_16bit_t *pf,
    int16_t f,
    int16_t h,
    int16_t q,
    int16_t r) {
    pf->F = f;
    pf->H = h;
    pf->Q = q;
    pf->R = r;
}


void filter_Kalman_init_default (filter_Kalman_16bit_t *pf) {
    pf->F = 1;
    pf->H = 1;
    pf->Q = 2;
    pf->R = 15;
    pf->state = 0;
    pf->covariance = 0.5;
}



void filter_Kalman_set_state (filter_Kalman_16bit_t *pf, int16_t  _state, float _covariance) {
    pf->state = _state;
    pf->covariance = _covariance;
}


int16_t filter_Kalman_correct (filter_Kalman_16bit_t *pf, int16_t value) {
    float K;
    //time update - prediction
    pf->X0 = pf->F * pf->state;
    pf->P0 = pf->F * pf->covariance * pf->F + pf->Q;
    //measurement update - correction
    K = pf->H * pf->P0 / (pf->H * pf->P0 * pf->H + pf->R);
    pf->state = pf->X0 + K * (value - pf->H * pf->X0);
    pf->covariance = (1 - K * pf->H) * pf->P0;
    return pf->state;
}


uint16_t I_filter (uint16_t x)
{	//Öèôğîâîé ğåêóğñèâíûé ôèëüòğ
    static uint32_t y;
	y = ((7 * y) + x) >> 3;
	return y;
}
