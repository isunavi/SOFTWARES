/**
 * @file    drvLM75.h
 * @author  Ht3h5793, CD45
 * @date    25.11.2014  8.19
 * @version V1.1.0
 * @brief 

    For (drvLM75_get) check init I2C -> halI2C_init
 */
 

#ifndef DRVLM75_H
#define	DRVLM75_H 20150818

#include "board.h"


#ifdef	__cplusplus
extern "C" {
#endif

msg_t       drvLM75_get (int8_t *tempI, uint8_t *tempF);

msg_t       drvLM75_mode (uint8_t newstate);

#ifdef	__cplusplus
}
#endif

#endif	/** DRVLM75_H */
