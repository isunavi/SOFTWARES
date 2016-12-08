/* 
 * @file    modFPGA
 * @author  Ht3h5793
 * @date    11.04.2013  16:54
 * @version V1.1.0
 * @brief  
@todo Напечатать состояние FPGA
@todo Сброс FPGA
 */

#ifndef DRVFPGA_H
#define	DRVFPGA_H 20150202

#include "defines.h"
#include "board.h"

#ifdef	__cplusplus
extern "C" {
#endif

// for initing 
void modFPGA_LoadFPGA_StartLoad (void);

msg_t modFPGA_LoadFPGA_block (uint8_t *, uint32_t);

msg_t modFPGA_LoadFPGA_complite (void);

//
uint8_t modFPGA_setRegister (uint8_t adress, uint8_t byte);


#ifdef	__cplusplus
}
#endif

#endif	/* DRVFPGA_H */

