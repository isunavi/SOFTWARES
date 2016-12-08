/**
 * http://cxem.net/mc/mc221.php
 * http://cxem.net/mc/mc255.php
https://geektimes.ru/post/255436/
 */

#include "halPaint.h"
#include "board.h"
#include "modPaint.h"
#include "modPaint_local.h"
#include "colors.h"
#include <string.h> // for memset


#if (LCD_LPH9157)

// ��������� ���������� �������
struct halLCD_lcdStruct_t {
	uint8_t       orientation; // ����������
    color_t       color;
} halLCD_lcdStruct;


// ��������������� �������� ��� ��������� ������������� LCD
void halLCD_delay (uint32_t delay)
{
    delay = delay * 1000UL;
    while (delay){ delay--; }
}


void halLCD_setColor (color_t color)
{
    halLCD_lcdStruct.color = color;
}


void _LCD_setContrast (uint8_t contrast)
{

}


void _LCD_spiWrite (uint8_t byte)
{
//#ifdef TRUE //__DEBUG
    uint8_t bit;
    // for(uint8_t bit = 0x80; bit; bit >>= 1) {
    for (bit = 0x80; bit; bit = bit >> 1)
    {
        __NOP();
        //__NOP();
        LCD_CLK_L;
        __NOP();
        //__NOP();
        if (0x80 & byte) {
            LCD_DAT_H;
        } else {
            LCD_DAT_L;
        }
        byte = byte << 1;
        //halLCD_delay (1);
        __NOP();
        //__NOP();
        LCD_CLK_H;
        __NOP();
        //__NOP();
        //if (PORTGbits.RG7) byte |= 0x01;
        //halLCD_delay (1);
    }
}

//�������/������
#define CMD 0
#define DAT 1
char RS_old;


 //===============================================================
 //������� ������ �������/������ � LCD (RS==0 - �������, RS==1 - ������)
 //===============================================================
void Send_to_lcd (unsigned char RS, unsigned char data)
{
    LCD_CLK_L;
    LCD_DAT_L;
    if ((RS_old != RS) || (!RS_old && !RS)) //��������� ������ �������� RS (���� ��������� ���� ������� �� ������� CS)
    {
        LCD_CS_H;
        if(RS) 
            LCD_RS_H
        else 
            LCD_RS_L;
        LCD_CS_L;
    }
    
    _LCD_spiWrite (data);
    /*
    LCD_DAT_L;
    if ((data & 128) == 128) LCD_DAT_H;
    LCD_CLK_H;
    LCD_CLK_L;
    LCD_DAT_L;
    if ((data & 64) == 64)  LCD_DAT_H;
    LCD_CLK_H;
    LCD_CLK_L;
    LCD_DAT_L;
    if ((data & 32) == 32)  LCD_DAT_H;
    LCD_CLK_H;
    LCD_CLK_L;
    LCD_DAT_L;
    if ((data & 16) ==16)  LCD_DAT_H;
    LCD_CLK_H;
    LCD_CLK_L;
    LCD_DAT_L;
    if ((data & 8) == 8)  LCD_DAT_H;
    LCD_CLK_H;
    LCD_CLK_L;
    LCD_DAT_L;
    if ((data & 4) == 4)  LCD_DAT_H;
    LCD_CLK_H;
    LCD_CLK_L;
    LCD_DAT_L;
    if ((data & 2) == 2)  LCD_DAT_H;
    LCD_CLK_H;
    LCD_CLK_L;
    LCD_DAT_L;
    if ((data & 1) == 1)  LCD_DAT_H;
    LCD_CLK_H;
    LCD_CLK_L;
    */
    
    RS_old=RS;  //��������� �������� RS
    LCD_DAT_L;
}


void hal_paint_init (uint8_t mode)
{
    // Setup software SPI.
    GPIO_InitTypeDef GPIO_InitStructure;
    uint32_t i;

    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin : PB2 */
    GPIO_InitStructure.Pin = (GPIO_PIN_LCD_RESET |
        GPIO_PIN_LCD_RS |
        GPIO_PIN_LCD_CS |
        GPIO_PIN_LCD_CLK |
        GPIO_PIN_LCD_DAT);
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init (GPIOB, &GPIO_InitStructure);

  
    LCD_RESET_L;
  halLCD_delay (1000);
  LCD_RESET_H;
  halLCD_delay (1000);
  Send_to_lcd(0, 0x01); //����������� �����
  Send_to_lcd(0, 0x36); //Memory Access Control (����������� ���������� ������� ������� (������): 0bVHRXXXXX, V - ���������� �� ��������� (0 - ������-����, 1 - �����-�����),
                          //H - ���������� �� ����������� (0 - �����-�������, 1 - ������-������), R - �������� ������� ������ � ������� (��� ���� ���������� �������� ������-����, �����-�������))
  Send_to_lcd(1, 0x00);
  Send_to_lcd(0, 0x11); //����� �� ������� ������
  halLCD_delay (200);
  Send_to_lcd(0, 0x3a); //��������� �������� �������
  #ifdef _8_BIT_COLOR
  Send_to_lcd(1, 0x02); //���� �� ������� 256 ������
  #else
  Send_to_lcd(1, 0x05); //��� ����� �� ������� 65536 ������
  #endif
  halLCD_delay (200);
  Send_to_lcd(0, 0x29); //��������� �������
}


void SetArea (coord_t x1, coord_t x2, coord_t y1, coord_t y2)
{
    Send_to_lcd ( 0, 0x2A );  //������ ������� �� X
    Send_to_lcd ( 1, x1 );    //���������
    Send_to_lcd ( 1, x2 );    //��������

    Send_to_lcd ( 0, 0x2B );  //������ ������� �� Y
    Send_to_lcd ( 1, y1 );    //���������
    Send_to_lcd ( 1, y2 );    //��������

    Send_to_lcd ( 0, 0x2C );  //���������� ������� �� ������ ������ � ������ � �������� �������� ������
}




void     hal_paint_setPixelColor (coord_t x, coord_t y, color_t color)
{
  signed char i;
  SetArea( x, x, y, y );
  LCD_RS_H;

#ifdef _8_BIT_COLOR	//(8-�� ������� �������� ������� (256 ������))
    Send_to_lcd( 1, color ); //������ - ����� ���� �������
#else			        //(16-�� ������� �������� ������� (65536 ������))
    Send_to_lcd( 1, (color >> 8) );
    Send_to_lcd( 1, color );
#endif
}


void hal_paint_setPixel (coord_t x, coord_t y)
{

}


color_t hal_paint_getPixel (coord_t x, coord_t y)
{
    return COLOR_BLACK;   
}


#if LCD_ENABLE_PARTIAL_UPDATE
uint8_t halLCD_cnt = 0;
#endif
void hal_paint_repaint (void)
{
 
}


void hal_paint_cls (color_t color)
{
    uint32_t x;
    signed char i;
    SetArea( 0, 131, 0, 175 );   //������� ����� ������
    LCD_RS_H;

    //������ - ����� ���� �������
    for (x = 0; x < 23232; x++)  // 23232 - ��� 132 * 176
    {
        #ifdef _8_BIT_COLOR	//(8-�� ������� �������� ������� (256 ������))
        Send_to_lcd( DAT, color ); //������ - ����� ���� �������
        #else			//(16-�� ������� �������� ������� (65536 ������))
        Send_to_lcd( DAT, (color >> 8) ); Send_to_lcd( DAT, color );
        #endif
    }
}


void     hal_paint_fillBlock (coord_t x, coord_t y, uint16_t w, uint16_t h, color_t *buf)
{
    coord_t xx, yy;
    
    for (xx = 0; xx < w; xx++)
        for (yy = 0; yy < h; yy++)
            hal_paint_setPixelColor (x + xx, y + yy, *buf++);
}


void     hal_paint_fillBlockColor (coord_t x, coord_t y,
    uint16_t w, uint16_t h,
    color_t color)
{
    coord_t xx, yy;
    
    for (xx = 0; xx < w; xx++)
        for (yy = 0; yy < h; yy++)
            hal_paint_setPixelColor (x + xx, y + yy, color);
}


void halLCD_setOrientation (uint8_t orient)
{
    
}

#endif
