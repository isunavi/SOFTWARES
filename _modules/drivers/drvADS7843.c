/*
 * ads7843.c
 *
 *  Created on: 11.01.2013
 *      Author: Andrey Fursov
 
 http://we.easyelectronics.ru/lcd_gfx/osvoenie-stm32f103ve-tft-lcd-touch-screen-chast-4.html
 
 
 */

#include "drvADS7843.h"
#include "board.h"


#define TP_GPIO_BUSY_PIN    PB1

#define TP_GPIO_CS_PIN      GPIO_PIN_0 //PB0

// SPI for TouchPanel (ADS7843)
#define GPIO_Pin_TP_SCK		GPIO_Pin_13
#define GPIO_Pin_TP_MISO	GPIO_Pin_14
#define GPIO_Pin_TP_MOSI	GPIO_Pin_15
#define GPIO_Pin_TP_CS		GPIO_Pin_0
#define PORT_TP_SPI			GPIOB
#define PORT_TP_CS			GPIOB

#define TP_Select()			do {GPIOB->BRR  = TP_GPIO_CS_PIN; } while (0) //L
#define TP_Deselect()		do {GPIOB->BSRR = TP_GPIO_CS_PIN; } while (0) //H

#define TP_PENIRQ_I         (GPIOB->IDR & GPIO_PIN_12)


 float ax, bx, ay, by; //int16_t

void TouchSetCoef(int16_t _ax, int16_t _bx, int16_t _ay, int16_t _by)
{
	ax = _ax;
	bx = _bx;
	ay = _ay;
	by = _by;
}

void _TouchSetCoef(float _ax, float _bx, float _ay, float _by)
{
	ax = _ax;
	bx = _bx;
	ay = _ay;
	by = _by;
}


void touchGetCoef(int16_t *_ax, int16_t *_bx, int16_t *_ay, int16_t *_by)
{
	*_ax = ax;
	*_bx = bx;
	*_ay = ay;
	*_by = by;
}


void drvADS7843_init (void)
{

	TP_Deselect();
}


extern SPI_HandleTypeDef hspi2;
uint16_t drv_spi (uint16_t data) {// Передать байт карте, программный спи
    uint16_t res = 0;
    /*
    uint16_t i, res = 0;
    for (i = 0; i < 16; i++)
    {
        if (data & 0x8000)//Выставить бит данных
        {
            TP_MOSI_H;
        } else {
            TP_MOSI_L;
        }
        data = data << 1;
        TP_SCK_H; //Импульс
	    res = res << 1;
        _delay_us(10);
	    if (0 != TP_MISO_I)
        { res = res | 0x0001; }  //Считать бит данных
        TP_SCK_L;
        _delay_us(10);
    }
    */
    if (HAL_OK != HAL_SPI_TransmitReceive (&hspi2,
        (uint8_t *)&data,
        (uint8_t *)&res,
        2,
        1000))
    {
        while (1) {};
    }
	return res;
}


uint16_t touchGetX(void) {
	uint16_t LSB, MSB;
	int16_t ret = 4095;

	if (TP_PENIRQ_I == 0) {
		TP_Select();
		ret = 0x2F;
		while(--ret);
// 		// x
// 		SPI_I2S_SendData(SPI2, 0x9400);
// 		/* Wait to receive a byte */
// 		while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET) { ; }
// 		MSB=SPI_I2S_ReceiveData(SPI2);
// 		SPI_I2S_SendData(SPI2, 0);
// 		while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET) { ; }
// 		/* Return the byte read from the SPI bus */
// 		LSB=SPI_I2S_ReceiveData(SPI2);
        
		// x
		MSB = drv_spi(0xD000);
		LSB = drv_spi (0);
        
		ret=0x0F;
		while(--ret);
		TP_Deselect();

		ret = ( ((MSB<<4) & 0x0FF0) | ((LSB>>12) & 0x000F) ) << 1;
	}

	return ret;
}

uint16_t touchGetY(void)
{
	uint16_t LSB, MSB;
	int16_t ret = 4095;

	if (TP_PENIRQ_I == 0) {
		TP_Select();
		ret = 0x2F;
		while(--ret);
// 		// y
// 		SPI_I2S_SendData(SPI2, 0xD400);
// 		/* Wait to receive a byte */
// 		while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET) { ; }
// 		MSB=SPI_I2S_ReceiveData(SPI2);
// 		SPI_I2S_SendData(SPI2, 0);
// 		while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET) { ; }
// 		/* Return the byte read from the SPI bus */
// 		LSB=SPI_I2S_ReceiveData(SPI2);
        
        // y
        MSB = drv_spi(0x9000);
		LSB = drv_spi (0);
        
        
		ret=0x0F;
		while(--ret);
		TP_Deselect();

		ret = ( ((MSB<<4) & 0x0FF0) | ((LSB>>12) & 0x000F) )<<1;
	}
	return ret;
}


uint16_t getTouchState(void)
{
	if (TP_PENIRQ_I == 0)
		return TRUE;
	return FALSE;
}



#include "modPaint.h"

static bool_t TouchVerifyCoef ( void )
{
	if ( ax == 0 || ax == 0xFFFF || bx == 0xFFFF || ay == 0 || ay == 0xFFFF || by == 0xFFFF )
		return TRUE;
    else
        return FALSE;
}

// http://we.easyelectronics.ru/lcd_gfx/osvoenie-stm32f103ve-tft-lcd-touch-screen-chast-4.html
//#define CALIB_VAR_1     1


#define TOUCH_X_MAX SCREEN_W
#define TOUCH_Y_MAX SCREEN_H
#define TOUCH_MIN_X 0
#define TOUCH_MIN_Y 0

#define RESCALE_FACTOR 1000000
#define DIV_TOUCH 1

#define ADC_X_MIN 6000
#define ADC_X_MAX 31000
#define ADC_Y_MIN 5500
#define ADC_Y_MAX 31500
#define ADC_X_APP_MAX (ADC_X_MAX / DIV_TOUCH)    //approach X max
#define ADC_Y_APP_MAX (ADC_Y_MAX / DIV_TOUCH)    //approach Y max
 
#define GET_X 0x98              //TSC2043 param
#define GET_Y 0xD8              //TSC2043 param

 int16_t d_in_x, d_in_y;
 int32_t cali_A, cali_B, cali_C, cali_D, cali_E, cali_F;



bool_t drvADS7843_touchGetXY (uint16_t *px, uint16_t *py, bool_t isReadCorrected)
{
    bool_t flag;
//	uint16_t t, t1, temp;
	// 0 - x, 1 - y
//	uint16_t databuffer[2][10];
//	int i, count = 10;
	uint16_t x, y;
    
#if CALIB_VAR_1
// 	*x = (touchGetX()/ax)+bx;
// 	*y = (touchGetY()/ay)+by;
//     *x = touchGetX() % paint_getWidth ();
// 	*y = touchGetY() % paint_getHeight ();
//     


	flag = getTouchState ();
	if (flag) // касание обнаружено
	{
		y = 4095 -  touchGetX (); // 
		x = touchGetY ();
		if ( isReadCorrected && !TouchVerifyCoef () )
		{
			*px = ( x / ax ) + bx;
			*py = ( y / ay ) + by;
		} // if
		else
		{ // без коррекции
			*px = x;
			*py = y;
		} // else
	} // if (касание)
#else
    flag = getTouchState ();
    if (flag) // касание обнаружено
	{
        y = 4095 - touchGetX (); // 
		x = touchGetY ();
        if ( isReadCorrected && !TouchVerifyCoef () )
		{
			*px = ( x / ax ) + bx;
			*py = ( y / ay ) + by;
            
            d_in_x = (cali_A * x + cali_B * x + cali_C) / RESCALE_FACTOR;
            d_in_y = (cali_D * y + cali_E * y + cali_F) / RESCALE_FACTOR;
            if (d_in_x >= TOUCH_X_MAX || d_in_y >= TOUCH_Y_MAX)
            {
                d_in_x = 0;
                d_in_y = 0;
            }   
		} // if
		else
		{ // без коррекции
			*px = x;
			*py = y;
		} // else
    }
#endif
	return flag;
}



static const float xCenter[] = { 35, SCREEN_W-35, 35, SCREEN_W-35 }; //int16_t
static const float yCenter[] = { 35, 35, SCREEN_H-35, SCREEN_H-35 }; //int16_t
static float xPos[5], yPos[5]; //int16_t

//variables
static float axc[2], ayc[2], bxc[2], byc[2]; //int16_t


#include "halFLASH.h"

#define CALIB_VAR_1 1
// Калибровка
void dvrASD7843_calibrate (void)
{
#if CALIB_VAR_1
	uint16_t x, y;
    uint32_t _x, _y;
    uint32_t n;
    
    _TouchSetCoef (13.44118, -36.34792, 8.875757, -31.1352); //
    return;
	// Если калибровочные коэффициенты уже установлены - выход
	if ( !TouchVerifyCoef () )
		return;

	// left top corner draw (левый верхний)
    paint_setColor (COLOR_WHITE);
    paint_setBackgroundColor(COLOR_BLACK);
	paint_clearScreen ();
	paint_putStrXY ( 50, 100, "Калибровка"); //50, 100,
	paint_line ( 10, 10+25, 10+50, 10+25 );
	paint_line ( 10+25, 10, 10+25, 10+50 );
	paint_putStrXY (50, 120, "Нажимайте ");
    n = 10; _x = 0; _y = 0;
	while (n != 0)
	{
		if (TRUE == drvADS7843_touchGetXY ( &x, &y, FALSE)) // ждать нажатия
            if (x < 4090 && y < 4090)
            {
                _x += x;
                _y += y;
                n--;//break;
            }
	}
    _x = _x / 10;
    _y = _y / 10;
    xPos[0] = _x;
    yPos[0] = _y;
    _x = _x * SCREEN_H / 4095;
    _y = _y * SCREEN_H / 4095;
    paint_circFill (_x, _y, 2, PAINT_QUARTERS_ALL);
	paint_putStrXY (50, 120, "Отпускайте");
    _delay_ms (200);
	// ждать отпускания
	while ( getTouchState () );
	paint_putStrXY (50, 120, "          ");


	// right top corner draw (правый верхний)
	paint_clearScreen ();
	paint_putStrXY (50, 100, "Калибровка");
	paint_line (SCREEN_W-10-50, 10+25, SCREEN_W -10-50+50, 10+25);
	paint_line (SCREEN_W-10-25, 10, SCREEN_W -10-25, 10+50);
	paint_putStrXY (50, 100,  "Нажимайте ");
    n = 10; _x = 0; _y = 0;
	while (n != 0)
	{
		if (TRUE == drvADS7843_touchGetXY ( &x, &y, FALSE)) // ждать нажатия
            if (x < 4090 && y < 4090)
            {
                _x += x;
                _y += y;
                n--;//break;
            }
	} // while
    _x = _x / 10;
    _y = _y / 10;
    xPos[1] = _x;
    yPos[1] = _y;
    _x = _x * SCREEN_H / 4095;
    _y = _y * SCREEN_H / 4095;
    paint_circFill (_x, _y, 2, PAINT_QUARTERS_ALL);
	paint_putStrXY (50, 100, "Отпускайте");
    _delay_ms (200);
	// ждать отпускания
	while ( getTouchState () );
	paint_putStrXY (50, 120, "          ");

	// левый нижний
	paint_clearScreen ();
	paint_putStrXY (50, 110, "Калибровка");
	paint_line ( 10, SCREEN_H-10-25, 10+50, SCREEN_H-10-25);	// hor
	paint_line ( 10+25, SCREEN_H-10-50, 10+25, SCREEN_H-10-50+50);	// vert
	paint_putStrXY (50, 120, "Нажимайте ");
    n = 10; _x = 0; _y = 0;
	while (n != 0)
	{
		if (TRUE == drvADS7843_touchGetXY ( &x, &y, FALSE)) // ждать нажатия
            if (x < 4090 && y < 4090)
            {
                _x += x;
                _y += y;
                n--;//break;
            }
	} // while
    _x = _x / 10;
    _y = _y / 10;
    xPos[2] = _x;
    yPos[2] = _y;
    _x = _x * SCREEN_H / 4095;
    _y = _y * SCREEN_H / 4095;
    paint_circFill (_x, _y, 2, PAINT_QUARTERS_ALL);
	paint_putStrXY (50, 120, "Отпускайте");
    _delay_ms (200);
	// ждать отпускания
	while ( getTouchState () );
	paint_putStrXY (50, 120, "          ");

	// Правый нижний
	paint_clearScreen ();
	paint_putStrXY (50, 110, "Калибровка");
	paint_line ( SCREEN_W-10-50, SCREEN_H-10-25, SCREEN_W-10-50+50, SCREEN_H-10-25);	// hor
	paint_line ( SCREEN_W-10-25, SCREEN_H-10-50, SCREEN_W-10-25, SCREEN_H-10-50+50);	// vert
	paint_putStrXY (50, 120, "Нажимайте ");
    n = 10; _x = 0; _y = 0;
	while (n != 0)
	{
		if (TRUE == drvADS7843_touchGetXY (&x, &y, FALSE)) // ждать нажатия
            if (x < 4090 && y < 4090)
            {
                _x += x;
                _y += y;
                n--;//break;
            }
	} // while
    _x = _x / 10;
    _y = _y / 10;
    xPos[3] = _x;
    yPos[3] = _y;
    _x = _x * SCREEN_H / 4095;
    _y = _y * SCREEN_H / 4095;
    paint_circFill (_x, _y, 2, PAINT_QUARTERS_ALL);
	paint_putStrXY (50, 120, "Отпускайте");
    _delay_ms (200);
	// ждать отпускания
	while ( getTouchState () );
	paint_putStrXY (50, 120, "          ");

	// Расчёт коэффициентов
	axc[0] = (xPos[3] - xPos[0])/(xCenter[3] - xCenter[0]);
	bxc[0] = xCenter[0] - xPos[0]/axc[0];
	ayc[0] = (yPos[3] - yPos[0])/(yCenter[3] - yCenter[0]);
	byc[0] = yCenter[0] - yPos[0]/ayc[0];

	axc[1] = (xPos[2] - xPos[1])/(xCenter[2] - xCenter[1]);
	bxc[1] = xCenter[1] - xPos[1]/axc[1];
	ayc[1] = (yPos[2] - yPos[1])/(yCenter[2] - yCenter[1]);
	byc[1] = yCenter[1] - yPos[1]/ayc[1];

	// Сохранить коэффициенты
	_TouchSetCoef ( axc[0], bxc[0], ayc[0], byc[0] );
    
    paint_clearScreen ();
	paint_putStrXY (20, 110, "Калибровка завершена");
	_delay_ms (500);	// 1 sec
    
    
#else
    uint32_t xd1 = (50 * TOUCH_X_MAX) / 100, xd2 = (90 * TOUCH_X_MAX) / 100, xd3 = (10 * TOUCH_X_MAX) / 100;
    uint32_t yd1 = (10 * TOUCH_Y_MAX) / 100, yd2 = (50 * TOUCH_Y_MAX) / 100, yd3 = (90 * TOUCH_Y_MAX) / 100;
    uint32_t xt1, xt2, xt3;
    uint32_t yt1, yt2, yt3;
    double temp1, temp2;
    double cal_A = 0.0, cal_B = 0.0, cal_C = 0.0, cal_D = 0.0, cal_E = 0.0, cal_F = 0.0;

    //get all coordinates to d_cal_X[] and d_cal_Y[]

    paint_setColor (COLOR_WHITE);
    paint_setBackgroundColor(COLOR_BLACK);
	paint_clearScreen ();
	paint_putStrXY (20, 2, "Calibrate Touch Screen");
    //POINT 1
    paint_circFill (xd1, yd1, 5, PAINT_QUARTERS_ALL); //point 1
    while (1)
            {
		if (TRUE == drvADS7843_touchGetXY ( &d_in_x, &d_in_y, FALSE)) // ждать нажатия
            if (d_in_x < 4090 && d_in_y < 4090)
            {
                    
                    xt1 = d_in_x;                           //save point here
                    yt1 = d_in_y;
                    break;
                    }
            }
    _delay_ms (200);
	// ждать отпускания
	while ( getTouchState () );
    //POINT 2
    paint_circFill (xd2, yd2, 5, PAINT_QUARTERS_ALL); //point 2
    while (1)
            {
		if (TRUE == drvADS7843_touchGetXY ( &d_in_x, &d_in_y, FALSE)) // ждать нажатия
            if (d_in_x < 4090 && d_in_y < 4090)
            {
                    
                    xt2 = d_in_x;                           //save point here
                    yt2 = d_in_y;                           //save point here
                    break;
                    }
            }
    _delay_ms (200);
	// ждать отпускания
	while ( getTouchState () );
    //POINT 3
    paint_circFill (xd3, yd3, 5, PAINT_QUARTERS_ALL); //point 3
    while (1)
            {
		if (TRUE == drvADS7843_touchGetXY ( &d_in_x, &d_in_y, FALSE)) // ждать нажатия
            if (d_in_x < 4090 && d_in_y < 4090)
            {
                    
                    xt3 = d_in_x;                           //save point here
                    yt3 = d_in_y;                                                   //save point here
                    break;
                    
            }
        }
    _delay_ms (200);
	// ждать отпускания
	while ( getTouchState () );
        
        
            //finish get coordinates 
    paint_clearScreen ();
    paint_putStrXY (20, 20, "Thanks. Wait...\n");

    //A
    temp1 = ((double) xd1 * ((double) yt2 - (double) yt3)) + ((double) xd2 * ((double) yt3 - (double) yt1)) + ((double) xd3 * ((double) yt1 - (double) yt2));
    temp2 = ((double) xt1 * ((double) yt2 - (double) yt3)) + ((double) xt2 * ((double) yt3 - (double) yt1)) + ((double) xt3 * ((double) yt1 - (double) yt2));
    cal_A = temp1 / temp2;
    cali_A = (int32_t) ((double)cal_A * RESCALE_FACTOR);

    //B
    temp1 = (cal_A * ((double) xt3 - (double) xt2)) + (double) xd2 - (double) xd3;
    temp2 = (double) yt2 - (double) yt3;
    cal_B = temp1 / temp2;
    cali_B = (int32_t) ((double)cal_B * RESCALE_FACTOR);

    //C
    cal_C = (double) xd3 - (cal_A * (double) xt3) - (cal_B * (double) yt3);
    cali_C = (int32_t) (cal_C * RESCALE_FACTOR);

    //D
    temp1 = ((double) yd1 * ((double) yt2 - (double) yt3)) + ((double) yd2 * ((double) yt3 - (double) yt1)) + ((double) yd3 * ((double) yt1 - (double) yt2));
    temp2 = ((double) xt1 * ((double) yt2 - (double) yt3)) + ((double) xt2 * ((double) yt3 - (double) yt1)) + ((double) xt3 * ((double) yt1 - (double) yt2));
    cal_D = (double)temp1 / (double)temp2;
    cali_D = (int32_t) (cal_D * RESCALE_FACTOR);

    //E
    temp1 = (cal_D * ((double) xt3 - (double) xt2)) + (double) yd2 - (double) yd3;
    temp2 = (double) yt2 - (double) yt3;
    cal_E = (double)temp1 / (double)temp2;
    cali_E = (int32_t) (cal_E * RESCALE_FACTOR);

    //F
    cal_F = (double) yd3 - cal_D * (double) xt3 - cal_E * (double) yt3;
    cali_F = (int32_t) (cal_F * RESCALE_FACTOR);
#endif

} // TouchCalibrate



