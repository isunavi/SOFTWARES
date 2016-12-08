/**
 * @file    drvBMP180.h
 * @author  Ht3h5793, CD45
 * @date    14.09.2015  8.25
 * @version V1.0.0
 * @brief 


давление в паскалях

 */
 

#ifndef DRVBMP180_H
#define	DRVBMP180_H 20150914

#include "board.h"


#ifdef	__cplusplus
extern "C" {
#endif
    

msg_t drvBMP180_Calibration (int16_t *BMP180_calibration_int16_t, int16_t *BMP180_calibration_uint16_t);

//uint16_t bmp180ReadShort(uint8_t address, uint8_t* errorcode);
//int32_t bmp180ReadTemp(uint8_t* error_code);
//int32_t bmp180ReadPressure(uint8_t* errorcode);

//msg_t       drvBMP180_getTemp (int16_t *);
//msg_t       drvBMP180_getPressure (int32_t *);

msg_t drvBMP180_Convert (int16_t *BMP180_calibration_int16_t, int16_t *BMP180_calibration_uint16_t, int32_t *temperature, int32_t *pressure);
int32_t drvBMP180_CalcAltitude (int32_t pressure);


#ifdef	__cplusplus
}
#endif

#endif	/** DRVBMP180_H */
