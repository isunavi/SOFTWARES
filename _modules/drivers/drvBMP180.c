#include "board.h"
#include "drvBMP180.h"
#include "halI2C.h"

#include "debug.h"


#if I2C_BMP180

//extern I2C_HandleTypeDef hi2c1;


// http://www.nxp.com/documents/data_sheet/LM75B.pdf
//#define I2C_TMP75_ADRESS        0x9E //(0x90 | ((0x7 << 1) & 0x0E))
//#define I2C_TMP75_REG_CONF              0x01

#define OSS 3 //точность - наивысшая


msg_t drvBMP180_ReadShort (uint8_t address, uint16_t *data16)
{
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t I2C_buf[2];

    if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_BMP180_ADDRESS, &address, 1, 100))
    {
        if (FUNCTION_RETURN_OK == halI2C_receive (I2C_BMP180_ADDRESS, &I2C_buf[0], 2, 100))
        {
            *data16 = (((uint16_t)I2C_buf[0] << 8) | I2C_buf[1]); //combine world
            //*data16 = tmp16;
            respond = FUNCTION_RETURN_OK;
        }
    }
    return respond;
}
//----------------------------------------

msg_t drvBMP180_ReadLong (uint8_t address, uint32_t *data32)
{
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t I2C_buf[3];

    if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_BMP180_ADDRESS, &address, 1, 100)) {
        if (FUNCTION_RETURN_OK == halI2C_receive (I2C_BMP180_ADDRESS, &I2C_buf[0], 3, 100)) {
            *data32 = (((uint32_t)I2C_buf[0] << 16) +
                ((uint32_t)I2C_buf[1] << 8) +
                (uint32_t)I2C_buf[2]) >> (8 - OSS);
            //*data32 = tmp32;
            respond = FUNCTION_RETURN_OK;
        }
    }
	return respond;
}
//----------------------------------------


msg_t drvBMP180_getTemp (int32_t *temperature)
{
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t I2C_buf[2];
	int16_t tmp16 = 0;
    
    *temperature = 0;
    I2C_buf[0] = 0xF4; //CMD - start measure temperature
    I2C_buf[1] = 0x2E;
    if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_BMP180_ADDRESS, &I2C_buf[0], 2, 100)) {
        HAL_Delay (5); // min time is 4.5ms
        if (FUNCTION_RETURN_OK == drvBMP180_ReadShort (0xF6, (uint16_t *)&tmp16)) {
            *temperature = tmp16;
            respond = FUNCTION_RETURN_OK;
        }
    }
	return respond;
}


msg_t drvBMP180_getPressure (int32_t *pressure)
{
	//int32_t temperature = 0;
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t I2C_buf[2];

    *pressure = 0;
    I2C_buf[0] = 0xF4; //CMD - start measure temperature
    I2C_buf[1] = 0x34 + (OSS << 6);	// write 0x34+(OSS<<6)
    if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_BMP180_ADDRESS, &I2C_buf[0], 2, 100)) {
        HAL_Delay (25); // min time is 25ms
        if (FUNCTION_RETURN_OK == drvBMP180_ReadLong (0xF6, (uint32_t *)pressure)) {
            respond = FUNCTION_RETURN_OK;
        }
    }
	return respond;
}


msg_t drvBMP180_Convert (int16_t *BMP180_calibration_int16_t,
    int16_t *BMP180_calibration_uint16_t,
    int32_t *temperature,
    int32_t *pressure) 
{
	int8_t i;
	int32_t ut = 0;
	int32_t up = 0;
	int32_t x1, x2, b5, b6, x3, b3, p;
	uint32_t b4, b7;
	int16_t ac1 = BMP180_calibration_int16_t[0];
	int16_t ac2 = BMP180_calibration_int16_t[1]; 
	int16_t ac3 = BMP180_calibration_int16_t[2]; 
	uint16_t ac4 = BMP180_calibration_uint16_t[0];
	uint16_t ac5 = BMP180_calibration_uint16_t[1];
	uint16_t ac6 = BMP180_calibration_uint16_t[2];
	int16_t b1 = BMP180_calibration_int16_t[3]; 
	int16_t b2 = BMP180_calibration_int16_t[4];
	//int16_t mb=BMP180_calibration_int16_t[5];
	int16_t mc = BMP180_calibration_int16_t[6];
	int16_t md = BMP180_calibration_int16_t[7];

	if (FUNCTION_RETURN_OK != drvBMP180_getTemp (&ut))
    {
        return FUNCTION_RETURN_ERROR;
	}
	if (FUNCTION_RETURN_OK != drvBMP180_getPressure (&up))
    {
        return FUNCTION_RETURN_ERROR;
	}
	
	x1 = ((int32_t)ut - (int32_t)ac6) * (int32_t)ac5 >> 15;
	x2 = ((int32_t)mc << 11) / (x1 + md);
	b5 = x1 + x2;
	*temperature = (b5 + 8) >> 4;
	
	b6 = b5 - 4000;
	x1 = (b2 * ((b6 * b6) >> 12)) >> 11;
	x2 = (ac2 * b6) >> 11;
	x3 = x1 + x2;
	b3 = (((((int32_t) ac1) * 4 + x3)<<OSS) + 2)>> 2;
	x1 = (ac3 * b6) >> 13;
	x2 = (b1 * ((b6 * b6) >> 12)) >> 16;
	x3 = ((x1 + x2) + 2) >> 2;
	b4 = (ac4 * (uint32_t) (x3 + 32768)) >> 15;
	b7 = ((uint32_t) (up - b3) * (50000 >> OSS));
	//p = b7 < 0x80000000 ? (b7 * 2) / b4 : (b7 / b4) * 2;
	
	if (b7 < 0x80000000)
	{
		p = (b7 << 1) / b4;
	}
	else
	{ 
		p = (b7 / b4) << 1;
	}

	x1 = (p >> 8) * (p >> 8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * p) >> 16;
	*pressure = p + ((x1 + x2 + 3791) >> 4);
    return FUNCTION_RETURN_OK;
}



msg_t drvBMP180_Calibration (int16_t *BMP180_calibration_int16_t, int16_t *BMP180_calibration_uint16_t)
{
    uint8_t errorcode = 0;
    
    if (FUNCTION_RETURN_OK != drvBMP180_ReadShort (0xAA, (uint16_t *)&BMP180_calibration_int16_t[0]))//ac1
    {
        errorcode += 1;
	}	
	if (FUNCTION_RETURN_OK != drvBMP180_ReadShort (0xAC, (uint16_t *)&BMP180_calibration_int16_t[1]))//ac2
    {
        errorcode += 1;
	}	
	if (FUNCTION_RETURN_OK != drvBMP180_ReadShort (0xAE, (uint16_t *)&BMP180_calibration_int16_t[2])) //ac3
    {
        errorcode += 1;
	}	
	if (FUNCTION_RETURN_OK != drvBMP180_ReadShort (0xB0, (uint16_t *)&BMP180_calibration_uint16_t[0])) //ac4
    {
        errorcode += 1;
	}	
	if (FUNCTION_RETURN_OK != drvBMP180_ReadShort (0xB2, (uint16_t *)&BMP180_calibration_uint16_t[1])) //ac5
    {
        errorcode += 1;
	}	
	if (FUNCTION_RETURN_OK != drvBMP180_ReadShort (0xB4, (uint16_t *)&BMP180_calibration_uint16_t[2])) //ac6
    {
        errorcode += 1;
	}	
	if (FUNCTION_RETURN_OK != drvBMP180_ReadShort (0xB6, (uint16_t *)&BMP180_calibration_int16_t[3])) //b1
    {
        errorcode += 1;
	}	
	if (FUNCTION_RETURN_OK != drvBMP180_ReadShort (0xB8, (uint16_t *)&BMP180_calibration_int16_t[4])) ///b2
    {
        errorcode += 1;
	}	
	if (FUNCTION_RETURN_OK != drvBMP180_ReadShort (0xBA, (uint16_t *)&BMP180_calibration_int16_t[5])) //mb
    {
        errorcode += 1;
	}	
	if (FUNCTION_RETURN_OK != drvBMP180_ReadShort (0xBC, (uint16_t *)&BMP180_calibration_int16_t[6])) //mc
    {
        errorcode += 1;
	}	
	if (FUNCTION_RETURN_OK != drvBMP180_ReadShort (0xBE, (uint16_t *)&BMP180_calibration_int16_t[7])) //md
    {
        errorcode += 1;
	}
    if (0 == errorcode)
        return FUNCTION_RETURN_OK;
    else
        return FUNCTION_RETURN_ERROR;
}
//----------------------------------------

#include "math.h"
//http://www.avislab.com/blog/bmp085/
int32_t drvBMP180_CalcAltitude (int32_t pressure)
{
	float temp;
	int32_t altitude;
	temp = (float) pressure / 101325;

	temp = 1 - pow ((float)temp, (float)0.190294957);

	//altitude = round(44330*temp*10);
	altitude = 44330 * temp * 100;
	
	return altitude; //get altitude in dm
}

#endif
