/**
 * @file    drvADNS2610.h
 * @author  Ht3h5793, CD45
 * @date    25.11.2014  8.19
 * @version V1.0.0
 * @brief 
 */
 

#ifndef DRVADNS2610_H
#define	DRVADNS2610_H 2

/**
 *  Раздел для "include"
 */
#include <defines.h>

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
    
void    drvADNS2610_init (void);
void    drvADNS2610_run (void);

#ifdef	__cplusplus
}
#endif

#endif	/** DRVADNS2610_H */
