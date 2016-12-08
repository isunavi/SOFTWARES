#include "board.h"
#include "drvHMC5883L.h"
#include "halI2C.h"

#include "debug.h"


#if I2C_HMC5883L

//extern I2C_HandleTypeDef hi2c1;



/*

Для установки нуля в простейшем случае можно использовать алгоритм поворота
на 180 градусов. Т.е. выбирается направление, любое, главное, чтобы его можно
было точно установить повторно, делается измерение показаний (Xm1, Ym1), 
затем датчик поворачивается ровно на 180 градусов и делаются следующие 
измерения (Xm2, Ym2). Исходя из них рассчитывается ноль: 
(X_{m0}, Y_{m0}) = ((X_{m1}+X_{m2})/2, (Ym1+Ym2)/2).
Для 3х мерного случая требуется еще перевернуть устройство вверх-ногами, 
чтобы найти ноль для 3ей оси.
*/

/*
    uint8_t respond;
    uint8_t buf[2] = {0x00, c};// Co = 0, D/C = 0
    //if (HAL_OK != HAL_I2C_Master_Transmit (&hi2c1, SSD1306_I2C_ADDRESS, buf, 2, 1000)) {
repeat_1:
    //Wire.beginTransmission(_i2caddr);
    respond = HAL_I2C_Master_Transmit (&hi2c1, SSD1306_I2C_ADDRESS, buf, 2, 1000);
*/

// http://avrproject.ru/publ/kak_podkljuchit/rabota_s_magnitometrom_hmc5883l/2-1-0-150
msg_t drvHMC5883L_init (uint8_t mode)
{
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t I2C_buf[2];
    
    I2C_buf[0] = 0x00; // register A
    I2C_buf[1] = 0x70; // (8 - average, 15 Hz default, normal measurement
    if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_HMC5883L_ADDRESS, &I2C_buf[0], 2, 100))
    {
        I2C_buf[0] = 0x01; // register B
        I2C_buf[1] = 0x00; //0x0A// Gain=5, or any other desired gain
        if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_HMC5883L_ADDRESS, &I2C_buf[0], 2, 100))
        {
            I2C_buf[0] = 0x02; // mode register
            I2C_buf[1] = 0x80; //0x00// continuous measurement mode
            if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_HMC5883L_ADDRESS, &I2C_buf[0], 2, 100))
            {
                respond = FUNCTION_RETURN_OK;
            }
        }
    }
    return respond;
}


msg_t drvHMC5883L_get (int16_t *X, int16_t *Y, int16_t *Z)
{
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t I2C_buf[8];
	int16_t tmp16 = 0;
    
    /*
    *X = 0;
    *Y = 0;
    *Z = 0;
    */
    I2C_buf[0] = 0x03; // X_H register
    if (FUNCTION_RETURN_OK == halI2C_transmit (I2C_HMC5883L_ADDRESS, &I2C_buf[0], 1, 100))
    {
        HAL_Delay (70); //70// min time is 4.5ms
        if (FUNCTION_RETURN_OK == halI2C_receive (I2C_HMC5883L_ADDRESS, &I2C_buf[0], 6, 100))
        {
            *X = (((uint16_t)I2C_buf[0] << 8) | I2C_buf[1]); //combine world
            *Z = (((uint16_t)I2C_buf[2] << 8) | I2C_buf[3]); //combine world
            *Y = (((uint16_t)I2C_buf[4] << 8) | I2C_buf[5]); //combine world
            respond = FUNCTION_RETURN_OK;
        }
    }
	return respond;
}




// //http://www.avislab.com/blog/bmp085/
// int32_t drvBMP180_CalcAltitude (int32_t pressure)
// {
// 	float temp;
// 	int32_t altitude;
// 	temp = (float) pressure / 101325;

// 	temp = 1 - pow ((float)temp, (float)0.190294957);

// 	//altitude = round(44330*temp*10);
// 	altitude = 44330 * temp * 100;
// 	
// 	return altitude; //get altitude in dm
// }

#endif
