/*
 * @file    halSPI.h
 * @author  Ht3h5793
 * @date    03.03.2014
 * @version V11.5.2
 * @brief   
*/

#ifndef HALSPI_H
#define	HALSPI_H 20150923

#include "board.h"


#ifdef	__cplusplus
extern "C" {
#endif

//------- SPI1 ------------
//void      halSPI1_IRQ (void);
void      halSPI1_init (uint8_t speed);
void      halSPI2_init (uint8_t speed);
void      halSPI3_init (uint8_t speed);
    
    
//void      halSPI1_setDiv (uint16_t baud);
//void      halSPI1_flush (void); //@todo сделать очистку всех буферов!
// Отправка байта
uint8_t     halSPI1_xput (uint8_t byte);
uint8_t     halSPI2_xput (uint8_t byte);
uint8_t     halSPI3_xput (uint8_t byte);
    
// uint8_t     halSPI1_sndS (uint8_t );
// msg_t     halSPI1_sndM (uint8_t *, uint16_t);

// Прием байта
//msg_t     halSPI1_rcvS (uint8_t *);
//msg_t     halSPI1_rcvM (uint8_t *, uint16_t);


#ifdef	__cplusplus
}
#endif

#endif	/* HALSPI_H */
