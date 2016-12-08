/**
 * @file    drvHMC5883L.h
 * @author  ht3v
 * @date    21.12.2015  10.44.42
 * @version V1.0.0
 * @brief 


давление в паскалях

 */
 

#ifndef DRVHMC5883L_H
#define	DRVHMC5883L_H 20151221

#include "board.h"


#ifdef	__cplusplus
extern "C" {
#endif
    

msg_t drvHMC5883L_init (uint8_t mode);
    
msg_t drvHMC5883L_get (int16_t *X, int16_t *Y, int16_t *Z);


#ifdef	__cplusplus
}
#endif

#endif	/** DRVHMC5883L_H */
