/**
 * @file    _FIFO.h
 * @author  Ht3h5793, CD45
 * @date    13.6.2012  15:50
 * @version V100.30.2
 * @brief   
 * @todo 
 * 
 
 */

#ifndef _FIFO_H
#define	_FIFO_H 20120611

/** Раздел для "include" */

#include "board.h"


/** Раздел для "define" */ 

/** Определения ответов функций */

/** Раздел для "typedef" */
typedef struct FIFO_t_ //struct
{
    int32_t in; // Next In Index
    int32_t out; // Next Out Index
    uint8_t *buf; // Buffer
    int32_t size;
} FIFO_t;


#ifdef	__cplusplus
extern "C" {
#endif

void FIFO_flush ( FIFO_t *s);

bool_t FIFO_init ( FIFO_t *s, uint8_t *buf, uint32_t FIFO_size);

uint32_t FIFO_available ( FIFO_t *s);

bool_t FIFO_get ( FIFO_t *s, uint8_t *c);

bool_t FIFO_put ( FIFO_t *s, uint8_t c);

bool_t FIFO_gets ( FIFO_t *s, uint8_t *c, uint32_t data_size);

bool_t FIFO_puts ( FIFO_t *s, uint8_t *c, uint32_t data_size);


#ifdef	__cplusplus
}
#endif

#endif	/* _FIFO_H */
