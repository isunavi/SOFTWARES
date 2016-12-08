#include "board.h"
#include "drvLM75.h"
#include "halI2C.h"


#if I2C_LM75
// http://www.nxp.com/documents/data_sheet/LM75B.pdf
//#define I2C_TMP75_ADRESS        0x9E //(0x90 | ((0x7 << 1) & 0x0E))
#define I2C_TMP75_REG_CONF              0x01
#define I2C_TMP75_REG_TEMPERATURE       0x00

msg_t    drvLM75_get (int8_t *tempI, uint8_t *tempF)
{
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t I2C_buf[5];
    
    I2C_buf[0] = I2C_TMP75_REG_TEMPERATURE; //pointer - temp register
    if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_LM75_ADDRESS, &I2C_buf[0], 1, 0))
    {
        if (FUNCTION_RETURN_OK == halI2C_receive (I2C_LM75_ADDRESS, &I2C_buf[0], 2, 0))
        {
            uint16_t tmp16 = (((uint16_t)I2C_buf[0] << 8 ) | I2C_buf[1]) >> 5;
            *tempI = tmp16 >> 3;
            *tempF = (uint16_t)(tmp16 & 0x07); //преобразуем дробное в целое число
            *tempF = (*tempF << 1) + (*tempF << 3); // Умножаем на 10
            *tempF = (*tempF >> 3); //делим на 8
            
            respond = FUNCTION_RETURN_OK;
        }
    }
    return respond;
}

// Set LM75 shutdown mode
// newstate:
//    FALSE = put LM75 into powerdown mode
//    TRUE = wake up LM75
msg_t  drvLM75_mode (uint8_t newstate)
{
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t I2C_buf[5];
    
    I2C_buf[0] = I2C_TMP75_REG_CONF; //pointer - conf register
    //Read configuration register including pointer byte (1-byte data)
    if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_LM75_ADDRESS, &I2C_buf[0], 1, 0))
    {
        if (FUNCTION_RETURN_OK == halI2C_receive (I2C_LM75_ADDRESS, &I2C_buf[0], 1, 0))
        {
            // Write configuration register (1-byte data)
            I2C_buf[1] = (newstate == FALSE) ? I2C_buf[0] | 0x01 : I2C_buf[0] & 0xFE;
            I2C_buf[0] = I2C_TMP75_REG_CONF; //pointer - conf register
            if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_LM75_ADDRESS, &I2C_buf[0], 2, 0))
            {
                respond = FUNCTION_RETURN_OK;
            }
        }
    }
    return respond;
}

#endif
