/**
 * @file    halI2C.h
 * @author  Ht3h5793, CD45
 * @date    25.11.2014  8.19
 * @version V1.1.0
 * @brief 
 */
 

#ifndef DRVTEA5767_H
#define	DRVTEA5767_H 2

/**
 *  Раздел для "include"
 */
#include <defines.h>

/**
 *  Раздел для "define"
 */
#define TEA5767_MODE_MANUAL  0x00
#define TEA5767_MODE_SEARCH  0x80

#define TEA5767_FLAG_STEREO  0x40


/**
 *  Раздел для "typedef"
 */

#ifdef	__cplusplus
extern "C" {
#endif

/**
 *  Раздел для прототипов функций
 */
    
msg_t    drvTEA5767_set (float freq, uint8_t mode);
msg_t    drvTEA5767_get (float *freq, uint8_t *mode);

#ifdef	__cplusplus
}
#endif

#endif	/** DRVTEA5767_H */
