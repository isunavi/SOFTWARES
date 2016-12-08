#include "drvTEA5767.h"
#include "board.h"

#include "debug.h"
#include "modSysClock.h"
#include "modPaint.h"


#define ADNS2610_CLK            GPIO_Pin_6
#define ADNS2610_SDIO           GPIO_Pin_4
                                
#define ADNS2610_CLK_L          GPIOE->BSRRH = ADNS2610_CLK
#define ADNS2610_CLK_H          GPIOE->BSRRL = ADNS2610_CLK
#define ADNS2610_SDIO_L         GPIOE->BSRRH = ADNS2610_SDIO
#define ADNS2610_SDIO_H         GPIOE->BSRRL = ADNS2610_SDIO
#define ADNS2610_SDIO_IN        (GPIOE->IDR & ADNS2610_SDIO)

//#define ADNS2610_SDIO_SET_OUT()
void ADNS2610_SDIO_SET_OUT (void) {
    GPIO_InitTypeDef   GPIO_InitStructureA;
    
    GPIO_StructInit (&GPIO_InitStructureA);
    GPIO_InitStructureA.GPIO_Pin   = ADNS2610_SDIO;
    GPIO_InitStructureA.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructureA.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructureA.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructureA.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init (GPIOE, &GPIO_InitStructureA);
}


//#define ADNS2610_SDIO_SET_IN()
void ADNS2610_SDIO_SET_IN (void) {
    GPIO_InitTypeDef   GPIO_InitStructureB;
    
    GPIO_StructInit (&GPIO_InitStructureB);
    GPIO_InitStructureB.GPIO_Pin   = ADNS2610_SDIO;
    GPIO_InitStructureB.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructureB.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init (GPIOE, &GPIO_InitStructureB);
}
        
uint8_t drvADNS2610_read (uint8_t address) {
    uint8_t res = 0;
    uint8_t i;
    
    ADNS2610_SDIO_SET_OUT ();
    for (i = 0x80; i > 0; i >>= 1)  {
        ADNS2610_CLK_L;
        _delay_us(1);
        if (address & i)
            ADNS2610_SDIO_H;
        else
            ADNS2610_SDIO_L;
        _delay_us(1);
        ADNS2610_CLK_H;
        _delay_us(1);
    }
    _delay_us(1); //12
    ADNS2610_SDIO_SET_IN ();
    for (i = 0x80; i > 0; i >>= 1) {
        ADNS2610_CLK_L;
        _delay_us(1);
        ADNS2610_CLK_H;
        
        if (0 != ADNS2610_SDIO_IN) {
          res |= i;
        }
        _delay_us(1);
    }
    _delay_us (10); //10
    return res;
}


void drvADNS2610_write (uint8_t address, uint8_t data) {
    uint8_t i;
    
    address |= 0x80;
    ADNS2610_SDIO_SET_OUT ();
    for (i = 0x80; i > 0; i >>= 1) {
        ADNS2610_CLK_L;
        _delay_us(1);
        if (address & i)
            ADNS2610_SDIO_H;
        else
            ADNS2610_SDIO_L;
        _delay_us(1);    
        ADNS2610_CLK_H;
        _delay_us(1);
    }
    _delay_us(1); //12
    for (i = 0x80; i > 0; i >>= 1) {
        ADNS2610_CLK_L;
        _delay_us(1);
        if (data & i)
            ADNS2610_SDIO_H;
        else
            ADNS2610_SDIO_L;
        _delay_us(1);
        ADNS2610_CLK_H;
        _delay_us(1);
    }
    _delay_us (10); //10
}


void drvADNS2610_init (void) {
    GPIO_InitTypeDef   GPIO_InitStructure;
    volatile uint8_t val;
    RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOE, ENABLE);
    
    GPIO_StructInit (&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = (ADNS2610_CLK | ADNS2610_SDIO | GPIO_Pin_2);
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init (GPIOE, &GPIO_InitStructure);
    
    GPIOE->BSRRH = GPIO_Pin_2;
    
    ADNS2610_SDIO_SET_OUT ();
    ADNS2610_CLK_H;
    _delay_us (50);
    ADNS2610_CLK_L;
    _delay_us (10);
    ADNS2610_CLK_H;
    _delay_ms (1025);
    drvADNS2610_write (0x00, 0x01);
    _delay_ms (1000); // 3000
    
//     val = drvADNS2610_read (0x00);
//     val = drvADNS2610_read (0x01);
//     val = drvADNS2610_read (0x02);
//     val = drvADNS2610_read (0x03);
//     val = drvADNS2610_read (0x04);
//     val = drvADNS2610_read (0x05);
//     val = drvADNS2610_read (0x06);
//     val = drvADNS2610_read (0x07);
//     val = drvADNS2610_read (0x08);
//     val = drvADNS2610_read (0x09);
//     val = drvADNS2610_read (0x0A);
//     val = drvADNS2610_read (0x11);

    
}


int drvADNS2610_readFrame (uint8_t *arr, int len) {
    uint8_t *pos = arr;
    uint8_t *uBound = arr + len;
    systime_t timeout = modSysClock_getTime();
    uint8_t val;
    
    drvADNS2610_write (0x08, 0x2A);
    
    while (modSysClock_getPastTime (timeout, SYSCLOCK_GET_TIME_MS_1) < 4000) {
        val = drvADNS2610_read (0x08);
        if ((val & 0x80)) {
            break;
        }
    }
    
    while ((modSysClock_getPastTime (timeout, SYSCLOCK_GET_TIME_MS_1) < 4000) && (pos < uBound)) {
        val = drvADNS2610_read (0x08);
        //timeout = modSysClock_getTime();

                
        //Only bother with the next bit if the pixel data is valid.
        if ((val & 0x40) != 0) {
          continue;
        }
        //If we encounter a start-of-field indicator, and the cursor isn't at the first pixel,
        //then stop. ('Cause the last pixel was the end of the frame.)
        if ((val & 0x80)  &&  (pos != arr)) {
            break;
        }
        *pos = val & 0x3F;
        pos++;
    }
    return (pos - arr);
}

uint8_t ADNS261_buf [324];
#define ADNS2610_PIXEL_SIZE     6
void drvADNS2610_run (void) {
    
    uint32_t x, y;
    int len;
    
    //drvADNS2610_init ();
    //while(1) {
    len = drvADNS2610_readFrame (ADNS261_buf, 324);
    
    if (len == 324) {
        //halLCD_setLayer (1);
        for (x = 0; x < 18; x++) {
            for (y = 0; y < 18; y++) {
                paint_setColor (ADNS261_buf[x * ADNS2610_PIXEL_SIZE + y]);
                paint_rectFill (x * ADNS2610_PIXEL_SIZE,
                    y * ADNS2610_PIXEL_SIZE,
                    ADNS2610_PIXEL_SIZE,
                    ADNS2610_PIXEL_SIZE); // color
            }
        }
    }
        //halLCD_setLayer (0);
        //halLCD_repaint ();
        //_delay_ms(100);
    //}
}


