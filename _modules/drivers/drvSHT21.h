/**
 * @file    halI2C.h
 * @author  Ht3h5793, CD45
 * @date    25.11.2014  8.19
 * @version V1.1.0
 * @brief 
 */
 

#ifndef DRVSHT21_H
#define	DRVSHT21_H 20151217

/**
 *  Раздел для "include"
 */
#include "board.h"

/**
 *  Раздел для "define"
 */


/**
 *  Раздел для "typedef"
 */

#ifdef	__cplusplus
extern "C" {
#endif

/**
 *  Раздел для прототипов функций
 */
    
msg_t    drvSHT21_getTemperature (int8_t *tempI, uint8_t *tempF);
msg_t    drvSHT21_getHumidity (uint8_t *humidity);


#ifdef	__cplusplus
}
#endif

#endif	/** DRVSHT21_H */
