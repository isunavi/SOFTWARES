#include "drvSHT21.h"
#include "board.h"
#include "halI2C.h"
#include "debug.h"

#if (I2C_SHT21)

#define I2C_SHT21_ADRESS (0x80)




msg_t    drvSHT21_setMode (uint8_t mode)
{
    msg_t respond = FUNCTION_RETURN_ERROR;

    
    return respond;
}


msg_t    drvSHT21_getTemperature (int8_t *tempI, uint8_t *tempF)
{
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t I2C_buf[3];
	//int16_t tmp16;
    uint32_t tmp32;
    float tmpF;
    
    //*tempI = 20;
    //*tempF = 0;
    I2C_buf[0] = 0xF3; //CMD - start measure temperature
    if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_SHT21_ADRESS, &I2C_buf[0], 1, 0))
    {
        _delay_ms (80);
        if (FUNCTION_RETURN_OK == halI2C_receive (I2C_SHT21_ADRESS, &I2C_buf[0], 3, 0)) //+crc
        {
            tmp32 = (((uint16_t)I2C_buf[0] << 8) | I2C_buf[1]); //combine world
            //tmp32 = tmp32 >> 2;
            
            tmpF = tmp32;
            tmpF = tmpF / 65536.0;
            tmpF = tmpF * 175.72;
            tmpF = -46.85 + tmpF;
            
            /*
            tmp32 = tmp32 << 16;
            tmp32 *= 11515985; // 11515985.92 = (175.72 * 65536)
            tmp32 -= 3070361; // -3070361.6 = (-46.85 * 65536)
            tmp32 = tmp32 >> 16;
            */
            *tempI = (uint8_t)tmpF;
            
            respond = FUNCTION_RETURN_OK;
        }
    }
    return respond;
}


msg_t    drvSHT21_getHumidity (uint8_t *humidity)
{
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t I2C_buf[3];
    uint32_t tmp32;
    float tmpF;
    
    I2C_buf[0] = 0xF5; //CMD - start measure humidity
    if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_SHT21_ADRESS, &I2C_buf[0], 1, 0))
    {
        _delay_ms (30);
        if (FUNCTION_RETURN_OK == halI2C_receive (I2C_SHT21_ADRESS, &I2C_buf[0], 3, 0)) //+crc
        {
            tmp32 = (((uint16_t)I2C_buf[0] << 8) | I2C_buf[1]); //combine world
            //tmp32 = tmp32 >> 2;
            
            tmpF = tmp32;
            tmpF = tmpF / 65536.0;
            tmpF = tmpF * 125;
            tmpF = -6 + tmpF;
            
            /*
            tmp32 = tmp32 << 16;
            tmp32 *= 11515985; // 11515985.92 = (175.72 * 65536)
            tmp32 -= 3070361; // -3070361.6 = (-46.85 * 65536)
            tmp32 = tmp32 >> 16;
            */
            *humidity = (uint8_t)tmpF;
            
            
            respond = FUNCTION_RETURN_OK;
        }
    }
    return respond;
}

#endif
