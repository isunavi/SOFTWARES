/**
 * @file    
 * @author  Ht3h5793
 * @date    13.01.2012
 * @version V2.0.0
 * @brief   Прошивка ПЛИС через JTAG, 

 * @HTTP    http://marsohod.org/index.php/prodmbftdi/menusvfplayer
            http://marsohod.org/index.php/ourblog/11-blog/162-jtag

 * @todo    

0x020A, 0x10DD
0x8232, 0x2aa2, 0x4A82, 0x0c2c, 0x0000


// Сначала нужно определиь выводы
#define JTAG_TCK_H   (GPIOB->BSRR = GPIO_Pin_4)
#define JTAG_TCK_L   (GPIOB->BRR  = GPIO_Pin_4)

#define JTAG_TMS_H   (GPIOB->BSRR = GPIO_Pin_3)
#define JTAG_TMS_L   (GPIOB->BRR  = GPIO_Pin_3)

#define JTAG_TDI_H   (GPIOB->BSRR = GPIO_Pin_15)
#define JTAG_TDI_L   (GPIOB->BRR  = GPIO_Pin_15)

#define JTAG_TDO_IN  (GPIOA->IDR  & GPIO_Pin_15) //master-in

#define JTAG_DELAY   _delay_us(1)

....

            
 */

#ifndef	JTAG_H
#define	JTAG_H 20151021

#include "defines.h"


/** Раздел для "define" */

//SAMPLE/PRELOAD 00 0000 0101
//EXTEST (1) 00 0000 1111
#define CJTAG_BYPASS             0x03FF  //BYPASS 11 1111 1111
//USERCODE 00 0000 0111
#define CJTAG_IDCODE             0x0006     //IDCODE 00 0000 0110
//HIGHZ (1) 00 0000 1011
//CLAMP (1) 00 0000 1010
//USER0 00 0000 1100
//USER1 00 0000 1110

//IEEE 1532 instructions
#define CJTAG_ISC_ENABLE         0x02CC
#define CJTAG_ISC_DISABLE        0x0201
#define CJTAG_ISC_PROGRAM        0x02F4
#define CJTAG_ISC_ERASE          0x02F2
#define CJTAG_ISC_ADDRESS         0x0203
#define CJTAG_ISC_READ           0x0205
#define CJTAG_ISC_NOP            0x0210
//flash char ID_code[]={0x02,0x0A,0x10,0xDD}; // EPM240


/** Раздел для "typedef" */
typedef enum JTAG_STATE { 
    JTAG_STATE_RESET         = 1,
    JTAG_STATE_IDLE          = 2,
    
    JTAG_STATE_DR_SELECT     = 3,
    JTAG_STATE_DR_CAPTURE    = 4,
    JTAG_STATE_DR_SHIFT      = 5,
    JTAG_STATE_DR_EXIT1      = 6,
    JTAG_STATE_DR_PAUSE      = 7,
    JTAG_STATE_DR_EXIT2      = 8,
    JTAG_STATE_DR_UPDATE     = 9,
    
    JTAG_STATE_IR_SELECT     = 10,
    JTAG_STATE_IR_CAPTURE    = 11,
    JTAG_STATE_IR_SHIFT      = 12,
    JTAG_STATE_IR_EXIT1      = 13,
    JTAG_STATE_IR_PAUSE      = 14,
    JTAG_STATE_IR_EXIT2      = 15,
    JTAG_STATE_IR_UPDATE     = 16
} JTAG_STATE_;



#ifdef	__cplusplus
extern "C" {
#endif
    
void drvJTAG_init (void);

void drvJTAG_deinit (void);
    
void drvJTAG_tck(void);

void drvJTAG_res(void);

void drvJTAG_setState (uint8_t state);

void drvJTAG_exitIDLE (void);

void drvJTAG_loadComm (uint16_t com);

/** Обмен байтами через JTAG 
 * если esc == 1, то это признак того, что данные последние
 */
uint16_t drvJTAG_xputData (uint16_t data, uint8_t esc);

void drvJTAG_loadAddress (uint16_t adr);

    
#ifdef	__cplusplus
}
#endif

#endif	// JTAG_H
