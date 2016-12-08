#include "drvTEA5767.h"
#include "board.h"
#include "halI2C.h"

#define I2C_TEA5767_ADRESS (0x60 << 1)

void    drvTEA5767_init (uint8_t num)
{
    halI2C_init ();
}


msg_t    drvTEA5767_set (float freq, uint8_t mode)
{
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t I2C_buf[5];
    uint16_t _freq;
	uint8_t freqH, freqL;
    
    _freq = 4 * (freq * 1000000 + 225000) / 32768; 
	freqH  = _freq >> 8;
	freqL  = _freq & 0xFF;
    
	I2C_buf[0] = freqH;
	I2C_buf[1] = freqL;
	I2C_buf[2] = 0xB0; //high side LO injection is on,.
	I2C_buf[3] = 0x10; //Xtal is 32.768 kHz
	I2C_buf[4] = 0x00;
    
    if (TEA5767_MODE_SEARCH & mode)
    {
        I2C_buf[0] |= 0x40;
    }
    
    if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_TEA5767_ADRESS, &I2C_buf[0], 5, 1000))
    {
        respond = FUNCTION_RETURN_OK;
    }
    
    return respond;
}


msg_t    drvTEA5767_get (float *freq, uint8_t *mode)
{
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t I2C_buf[5];
    uint8_t freqH, freqL;
    
    if (FUNCTION_RETURN_OK == halI2C_receive (I2C_TEA5767_ADRESS, &I2C_buf[0], 5, 1000))
    {
        respond = FUNCTION_RETURN_OK;
    } else {
        return respond;
    }
    
    freqH = I2C_buf[0] & 0x3F;
    freqL = I2C_buf[1];
    *freq = (float)(((freqH << 8) + freqL) * 32768.0 / 4.0 - 225000.0) / 1000000.0;
    
    *mode = 0;
    *mode |= I2C_buf[3] >> 4; // level 
//     if (I2C_buf[0] & 0x80) @todo долелать флаг наличия радиостанции
//         *mode |= TEA5767_MODE_SEARCH;
    if (I2C_buf[2] & 0x80) // stereo
        *mode |= TEA5767_FLAG_STEREO;
    
    return respond;
}




