/**
 * @file    halI2C.h
 * @author  Ht3h5793, CD45
 * @date    13.6.2013  2:12
 * @version V6.0.9
 * @brief 


// http://vk.com/id30002634?w=wall30002634_12852%2Fall
// http://easyelectronics.ru/interface-bus-iic-i2c.html
http://we.easyelectronics.ru/STM32/stm32-i2c-eeprom-24sxx.html

#define I2C_SEARCH_SIZE 4
volatile uint8_t I2C_search_buf[I2C_SEARCH_SIZE];
volatile uint8_t I2C_search_cnt = 0;
volatile uint8_t I2C_search_adress = 0;
volatile uint8_t I2C_search_f = FALSE;

    halI2C_init ();
    if (FUNCTION_RETURN_OK == halI2C_re_init ())
    {
        xsprintf (_str, "REINIT");
        paint_putStrColRow (0, 15, _str);
        halLCD_layer_copy (1, 0);
    }

    while (1)
    {
        if (FALSE == I2C_search_f)
        {
            if (FUNCTION_RETURN_OK == halI2C_transmit(I2C_search_adress, &I2C_buf[0], 0, 0))
            {
                I2C_search_buf[I2C_search_cnt] = I2C_search_adress;
                if (I2C_SEARCH_SIZE <= ++I2C_search_cnt)
                {
                    I2C_search_f = TRUE;
                    break;  
                }
            }
            I2C_search_adress += 2;
        }
        xsprintf (_str, "%u.%02X %02X %02X %02X %02X", 
            I2C_search_f,
            I2C_search_adress,            
            I2C_search_buf[0], 
            I2C_search_buf[1],
            I2C_search_buf[2],
            I2C_search_buf[3]);
        paint_putStrColRow (0, 5, _str);
        // перерисовываем
        halLCD_layer_copy (1, 0);
        _delay_ms (10);
    }


    // read small EEPROM
    I2C_buf[0] = 0;
    if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_ADRESS_24Cxx, &I2C_buf[0], 1, 0))
    {
        if (FUNCTION_RETURN_OK == halI2C_receive (I2C_ADRESS_24Cxx, &I2C_buf[0], 1, 0))
        {
            xsprintf (_str, "r %02X",            
                I2C_buf[0]);
            paint_putStrColRow (0, 8, _str);
        }
    }

    // write small EEPROM
    I2C_buf[1] = I2C_buf[0];
    I2C_buf[0] = 0;
    I2C_buf[1]++;
    if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_ADRESS_24xx, &I2C_buf[0], 2, 0))
    {
        xsprintf (_str, "w %02X %02X",            
            I2C_buf[0], 
            I2C_buf[1]);
        paint_putStrColRow (0, 9, _str);
    }
    _delay_ms (1200);


    // read large EEPROM
    I2C_buf[0] = 1;
    I2C_buf[1] = 0;
    if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_ADRESS_24Cxx, &I2C_buf[0], 2, 0))
    {
        if (FUNCTION_RETURN_OK == halI2C_receive (I2C_ADRESS_24Cxx, &I2C_buf[0], 1, 0))
        {
            xsprintf (_str, "r %02X",            
                I2C_buf[0]);
            paint_putStrColRow (0, 8, _str);
        }  
    }

    // write large EEPROM
    I2C_buf[2] = I2C_buf[0];
    I2C_buf[0] = 1;
    I2C_buf[1] = 0;
    I2C_buf[2]++;
    if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_ADRESS_24Cxx, &I2C_buf[0], 3, 0))
    {
        xsprintf (_str, "w %02X %02X %02X",            
            I2C_buf[0], 
            I2C_buf[1],
            I2C_buf[2]);
        paint_putStrColRow (0, 9, _str);
    }
    _delay_ms (4200);


 */

#ifndef HALI2C_H
#define	HALI2C_H 20161031

/**
 *  Раздел для "include"
 */
#include "board.h"

/**
 *  Раздел для "define"
 */
#define I2C_ADRESS_24Cxx        0xA0
#define I2C_ADRESS_PCF8523      0xD0


#define I2C_BIT_RW    0x01

enum {
    _I2C1 = 0,
    _I2C2,
    _I2C3
};


/**
 *  Раздел для "typedef"
 */

#ifdef	__cplusplus
extern "C" {
#endif

/**
 *  Раздел для прототипов функций
 */
    
void    soft_halI2C_init ();

msg_t   soft_halI2C_re_init ();
    
void    soft_halI2C_deInit ();
    


//Функция отправки в шину
msg_t   soft_halI2C_transmit (uint8_t adr, uint8_t *txbuf, uint8_t txbytes, systime_t timeout);

//Чтение из шины
msg_t   soft_halI2C_receive (uint8_t adr, uint8_t *rxbuf, uint8_t rxbytes, systime_t timeout);


msg_t   soft_halI2C_get_errors (void);


#ifdef	__cplusplus
}
#endif

#endif	/* HALI2C_H */
