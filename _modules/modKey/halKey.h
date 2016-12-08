/*
 * @file    halKey.h
 * @author  Ht3h5793
 * @date    08.12.2013
 * @version V0.5.1
 * @brief   Драйвер клавиатры
 */

#ifndef HALKEY_H
#define HALKEY_H 20150513

#include "board.h"

#ifdef	__cplusplus
extern "C" {
#endif

void     halKey_init (void);
uint8_t  halKey_getCount(uint8_t num);
void     halKey_run(void);

#ifdef	__cplusplus
}
#endif

#endif /* HALKEY_H */
