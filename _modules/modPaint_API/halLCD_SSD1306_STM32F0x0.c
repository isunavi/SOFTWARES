/*********************************************************************
This is a library for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

These displays use SPI to communicate, 4 or 5 pins are required to
interface

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen below must be included in any redistribution
*********************************************************************/

#include "board.h"
//#include <stdlib.h>
#include <string.h>

#include "halPaint.h"




#if defined SSD1306_64_32       

//#define width()      SSD1306_LCDWIDTH
//#define height()      SSD1306_LCDHEIGHT

//#define WIDTH   SSD1306_LCDWIDTH
//#define HEIGHT   SSD1306_LCDHEIGHT
#endif


#define SSD1306_SETCONTRAST                     0x81
#define SSD1306_DISPLAYALLON_RESUME             0xA4
#define SSD1306_DISPLAYALLON                    0xA5
#define SSD1306_NORMALDISPLAY                   0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10
#define SSD1306_SETSTARTLINE 0x40
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR 0x22
#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_SEGREMAP 0xA0
#define SSD1306_CHARGEPUMP 0x8D
#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2
// Scrolling #defines
#define SSD1306_ACTIVATE_SCROLL 0x2F
#define SSD1306_DEACTIVATE_SCROLL 0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A


//#define ssd1306_swap(a, b) { int16_t t = a; a = b; b = t; }


// the memory buffer for the LCD

//static uint8_t buffer[SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8];

// структура параметров дисплея
struct halLCD_lcdStruct_t {
	uint8_t       orientation; // Ориентация
    color_t       color;
    uint8_t     update_pos;
} halLCD_lcdStruct;


void hal_paint_setColor (color_t color) {
    halLCD_lcdStruct.color = color;
}

uint8_t screen_pixel_mode = GDI_ROP_COPY;
uint8_t screen_buf[SCREEN_H / 8][SCREEN_W];
uint8_t screen_update_pos = 0;

//uint8_t x, y;
uint8_t contrast;
extern I2C_HandleTypeDef hi2c1;

uint8_t getRotation (void)
{
    return 1;
}

/*
// the most basic function, set a single pixel
void drawPixel (int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
    return;

  // check rotation, move pixel around if necessary
  switch (getRotation()) {
  case 1:
    ssd1306_swap(x, y);
    x = WIDTH - x - 1;
    break;
  case 2:
    x = WIDTH - x - 1;
    y = HEIGHT - y - 1;
    break;
  case 3:
    ssd1306_swap(x, y);
    y = HEIGHT - y - 1;
    break;
  }

  // x is which column
    switch (color)
    {
      case WHITE:   buffer[x+ (y/8)*SSD1306_LCDWIDTH] |=  (1 << (y&7)); break;
      case BLACK:   buffer[x+ (y/8)*SSD1306_LCDWIDTH] &= ~(1 << (y&7)); break;
      case INVERSE: buffer[x+ (y/8)*SSD1306_LCDWIDTH] ^=  (1 << (y&7)); break;
    }

}
*/

void halLCD_send_command (uint8_t c) {
    uint8_t respond;
    uint8_t buf[2] = {0x00, c};// Co = 0, D/C = 0
    //if (HAL_OK != HAL_I2C_Master_Transmit (&hi2c1, SSD1306_I2C_ADDRESS, buf, 2, 1000)) {
repeat_1:
    //Wire.beginTransmission(_i2caddr);
    respond = HAL_I2C_Master_Transmit (&hi2c1, SSD1306_I2C_ADDRESS, buf, 2, 1000);
    //Wire.endTransmission();
    switch (respond)
    {
    case HAL_OK: break;
    case HAL_BUSY:
        goto repeat_1;
        break;
    case HAL_ERROR:
    case HAL_TIMEOUT:
    default: while (1) {}; break;
    }
}


unsigned char  pic[]=
{
/*--64x32  --*/
0x0F,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x08,0xFF,0x08,0xF8,0x00,0xFE,0x92,
0x92,0x92,0xFE,0x00,0x00,0x00,0x00,0xFF,0x11,0x11,0x11,0x11,0x11,0x11,0xFF,0x00,
0x00,0x02,0x22,0x32,0xAA,0x26,0xFB,0x22,0x62,0xA2,0x22,0x02,0x00,0x04,0x04,0x04,
0x84,0x7C,0x15,0x16,0x14,0xF4,0x06,0x04,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x0F,
0x00,0x00,0x00,0xF0,0xF0,0x90,0x90,0x90,0x00,0x86,0x41,0x40,0xC7,0x84,0x05,0x84,
0xC4,0x44,0x45,0xC6,0x80,0x40,0xE0,0xF7,0x42,0x02,0x02,0x02,0x02,0xF2,0xF7,0x10,
0x10,0xF2,0xE2,0x01,0xD0,0xD4,0x07,0x80,0xC0,0x40,0x41,0xC2,0x80,0x04,0xD4,0xD2,
0x01,0xC0,0xC4,0x44,0x44,0xC3,0x80,0x00,0x80,0xC0,0x40,0x40,0xC0,0x80,0x00,0x00,
0xC0,0xC0,0xC0,0xCF,0xCF,0xC8,0xC8,0xC8,0xC0,0xC6,0xCF,0xC9,0xCF,0xCF,0xC0,0xC5,
0xCD,0xCB,0xCB,0xCE,0xC6,0xC0,0xC7,0xCF,0xC8,0xC0,0xC0,0xC0,0xC0,0xCF,0xCF,0xC1,
0xC3,0xC7,0xCC,0xC8,0xCF,0xCF,0xC0,0xC5,0xCD,0xCB,0xCB,0xCE,0xC6,0xC0,0xCF,0xCF,
0xC0,0xCF,0xCF,0xC0,0xC0,0xCF,0xCF,0xC0,0xD7,0xEF,0xE8,0xE8,0xFF,0xDF,0xC0,0xC0,
0xF0,0x80,0x80,0x00,0xFE,0x92,0x92,0x92,0x00,0xFE,0x12,0x32,0x6C,0x80,0x00,0x20,
0x20,0x00,0x00,0x7C,0x82,0x82,0x7C,0x00,0xFE,0x80,0x80,0x80,0x00,0xFE,0x92,0x92,
0x92,0x00,0xFE,0x82,0x82,0x7C,0x00,0x7C,0x82,0x82,0x7C,0x00,0x60,0x58,0xFE,0x40,
0x00,0x9C,0x92,0xD2,0x7C,0x00,0x10,0x10,0x00,0x82,0xFE,0x80,0x00,0x80,0x80,0xF0,


};



void Set_window (void)
{	

    //uint8_t respond;
    //uint8_t buf[] = {0x80, SSD1306_COLUMNADDR, 0, 128-1};
    
//repeat_11:
    //Wire.beginTransmission(_i2caddr);
    //respond = HAL_I2C_Master_Transmit (&hi2c1, SSD1306_I2C_ADDRESS, buf, 4, 1000);
    //Wire.endTransmission();
    
    halLCD_send_command (SSD1306_COLUMNADDR);
    halLCD_send_command (0);   // Column start address (0 = reset)
    halLCD_send_command (55 - 1); // SCREEN_W Column end address (127 = reset)

    halLCD_send_command (SSD1306_PAGEADDR);
    halLCD_send_command (0); // Page start address (0 = reset)
#if (SCREEN_H == 64)
    halLCD_send_command (7); // Page end address
#endif
#if (SCREEN_H == 32)
    halLCD_send_command (3 << 2); // Page end address
#endif
#if (SCREEN_H == 16)
    halLCD_send_command (1); // Page end address
#endif
}


// Set page address 0~15
void Set_Page_Address (unsigned char add) {	
    add += 32;
	//Start();
	//SentByte(Write_Address);
	//SentByte(0x80);
    add = 0xB0 | add;
	//SentByte(add);
    //_nop_();

    
    uint8_t respond;
    uint8_t buf[2] = {0x80, add};
    
repeat_11:
    //Wire.beginTransmission(_i2caddr);
    respond = HAL_I2C_Master_Transmit (&hi2c1, SSD1306_I2C_ADDRESS, buf, 2, 1000);
    //Wire.endTransmission();
    switch (respond)
    {
    case HAL_OK: break;
    case HAL_BUSY:
        goto repeat_11;
        break;
    case HAL_ERROR:
    case HAL_TIMEOUT:
    default: while (1) {}; break;
    }
}


void Set_Column_Address(unsigned char add) {	
    add += 32;
    //Start();
    //SentByte(Write_Address);
    //SentByte(0x80);
    //SentByte( (0x10 | (add >> 4)));
    //SentByte(0x80);
    //SentByte((0x0f&add));
    //Stop();

    uint8_t respond;
    uint8_t buf[4] = {0x80, (0x10 | (add >> 4)), 0x80, (0x0F & add)};
    
repeat_12:
    //Wire.beginTransmission(_i2caddr);
    respond = HAL_I2C_Master_Transmit (&hi2c1, SSD1306_I2C_ADDRESS, buf, 4, 1000);
    //Wire.endTransmission();
    switch (respond)
    {
    case HAL_OK: break;
    case HAL_BUSY:
        goto repeat_12;
        break;
    case HAL_ERROR:
    case HAL_TIMEOUT:
    default: while (1) {}; break;
    }
}


void Display_Picture(unsigned char pic[])
{
    unsigned char i,j,num=0;
    uint8_t respond;
    uint8_t buf[1 + SCREEN_W];
    
	for(i=0;i<4;i++)
	{
        Set_Page_Address(i);
        Set_Column_Address(0x00);

        buf[0] = 0x40;
        for (uint8_t j = 0; j < SCREEN_W; j++) { //16
            buf[1 + j] = pic[i*SCREEN_W+j];
        }
        
repeat_3:
    //Wire.beginTransmission(_i2caddr);
    //HAL_Delay (4);
        respond = HAL_I2C_Master_Transmit (&hi2c1, SSD1306_I2C_ADDRESS, buf, (1 + SCREEN_W), 1000);
        
        //Wire.endTransmission();
        switch (respond)
        {
        case HAL_OK: break;
        case HAL_BUSY:
            goto repeat_3;
            break;
        case HAL_ERROR:
        case HAL_TIMEOUT:
        default: while (1) {}; break;
        }
	}
    return;
}






void halLCD_inverse_mode (uint8_t i)
{
    if (i)
    {
        halLCD_send_command (SSD1306_INVERTDISPLAY);
    } else {
        halLCD_send_command (SSD1306_NORMALDISPLAY);
    }
}


void stopscroll(void)
{
    halLCD_send_command (SSD1306_DEACTIVATE_SCROLL);
}


void halLCD_update_partial (uint8_t pos) {
    uint8_t respond;
    uint8_t buf[1 + SCREEN_W];


    

//Set_window ();

            Set_Page_Address (pos);
        Set_Column_Address (0x00);
        
    //for (uint8_t i = 0; i < (SCREEN_H / 8); i++)
    {
        // send a bunch of data in one xmission
        //Set_Page_Address(i);
        //Set_Column_Address(0x00);
        
        buf[0] = 0x40;
        //WIRE_WRITE(0x40);
        for (uint8_t j = 0; j < SCREEN_W; j++) { //16
            //WIRE_WRITE(buffer[i]);
            buf[1 + j] = screen_buf[pos][j];
            //i++;
        }
repeat_2:
    //HAL_Delay (15);
    //Wire.beginTransmission(_i2caddr);
        //if (HAL_OK != HAL_I2C_Master_Transmit (&hi2c1, SSD1306_I2C_ADDRESS, buf, (1 + SCREEN_W), 1000)) //16
        respond = HAL_I2C_Master_Transmit (&hi2c1, SSD1306_I2C_ADDRESS, buf, (1 + SCREEN_W), 1000);
        //Wire.endTransmission();
        switch (respond)
        {
        case HAL_OK: break;
        case HAL_BUSY:
            goto repeat_2;
            break;
        case HAL_ERROR:
        case HAL_TIMEOUT:
        default: while (1) {}; break;
        }
        //i--;
    }
}


void hal_paint_init (uint8_t mode) { //uint8_t vccstate, uint8_t i2caddr, bool_t reset)

    uint8_t vccstate = SSD1306_SWITCHCAPVCC;
 
    HAL_GPIO_WritePin (GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
    HAL_Delay (1);
    HAL_GPIO_WritePin (GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
    HAL_Delay (2);
    
    // Init sequence
    halLCD_send_command (SSD1306_DISPLAYOFF);                    // 0xAE
    
    halLCD_send_command (0x00); 
    halLCD_send_command (0x12); 
    halLCD_send_command (0x00); 
    halLCD_send_command (0xB0); 
    

    halLCD_send_command (SSD1306_SETCONTRAST);                  // 0x81
#if SSD1306_64_32
    if (vccstate == SSD1306_EXTERNALVCC)
    { halLCD_send_command (0x10); }
    else
    { halLCD_send_command (0x07); } //0x1F //0x4F
#endif
    
    halLCD_send_command (0xA1); 

    halLCD_send_command (SSD1306_NORMALDISPLAY);                // 0xA6

    halLCD_send_command (SSD1306_SETMULTIPLEX);                 // 0xA8
    halLCD_send_command (SCREEN_H - 1);

    halLCD_send_command (SSD1306_COMSCANDEC);                   //0xC8
    
    halLCD_send_command (SSD1306_SETDISPLAYOFFSET);             // 0xD3
    halLCD_send_command (0x0);                                  // no offset
    
    halLCD_send_command (SSD1306_SETDISPLAYCLOCKDIV);           // 0xD5
    halLCD_send_command (0x80);                                 // the suggested ratio 0x80

    
    halLCD_send_command (SSD1306_SETPRECHARGE);                  // 0xd9
    if (vccstate == SSD1306_EXTERNALVCC)
    { halLCD_send_command (0x22); }
    else
    { halLCD_send_command (0xF1); }


    halLCD_send_command (SSD1306_SETCOMPINS);                   // 0xDA
#if SSD1306_64_32
    halLCD_send_command (0x12); //0x2
#endif
#if SSD1306_128_32
    halLCD_send_command (0x2); //0x2
#endif
    
    halLCD_send_command (SSD1306_SETVCOMDETECT);                // 0xDB
    halLCD_send_command (0x40);
    
    
    halLCD_send_command (SSD1306_CHARGEPUMP);                   // 0x8D
    if (vccstate == SSD1306_EXTERNALVCC)
    { halLCD_send_command (0x10); }
    else
    { halLCD_send_command (0x14); } //0x14

    halLCD_send_command (SSD1306_DISPLAYON); //--turn on oled panel
    

   // Set_window ();
    
    HAL_Delay (100);
    Display_Picture (pic);
    //HAL_Delay (2000);
    return;
    
    halLCD_update ();
    uint8_t x = 0;
    while (1)
    {
        HAL_Delay (100);
        if (++x >= 6)
        {
            x = 0;
            halLCD_inverse_mode (0);
        } else {
            halLCD_inverse_mode (1);
        }
        
        drawPixel (x, 6, x);
    }
    
    
    halLCD_send_command (SSD1306_SETSTARTLINE | 0x0);            // line #0

    halLCD_send_command (SSD1306_MEMORYMODE);                    // 0x20
    halLCD_send_command (0x00);                                  // 0x0 act like ks0108
    halLCD_send_command (SSD1306_SEGREMAP | 0x1);
    halLCD_send_command (SSD1306_COMSCANDEC);

    halLCD_send_command (SSD1306_DISPLAYALLON_RESUME);           // 0xA4
    
    halLCD_send_command (SSD1306_DEACTIVATE_SCROLL);
}



uint8_t _vccstate;
// Dim the display
// dim = true: display is dimmed
// dim = false: display is normal
void dim (bool_t dim) {
    uint8_t contrast;

    if (dim) {
        contrast = 0; // Dimmed display
    } else {
        if (_vccstate == SSD1306_EXTERNALVCC) {
            contrast = 0x9F;
        } else {
            contrast = 0xCF;
        }
    }
    // the range of contrast to too small to be really useful
    // it is useful to dim the display
    ssd1306_command(SSD1306_SETCONTRAST);
    ssd1306_command(contrast);
}

                             

void paint_pixel_ (uint8_t x, uint8_t y, uint8_t val) {
    uint8_t tmp8;
    uint8_t  y0;
        
    if ((x < SCREEN_W) && (y < SCREEN_H)) {
        y0 = y & 0x07;
        y = y >> 3;
        tmp8 = screen_buf[y][x];
        switch (screen_pixel_mode) {
            case GDI_ROP_COPY:	tmp8 = val;  break;
            case GDI_ROP_XOR:	tmp8 ^=  (val << y0); break; //inv
            case GDI_ROP_AND:	tmp8 &= ~(val << y0); break; //clr
            case GDI_ROP_OR:	tmp8 |=  (val << y0); break; //set
        }
        screen_buf[y][x] = tmp8;
    }
}


void hal_paint_setPixel (coord_t x, coord_t y)
{
    paint_pixel_ ((uint8_t)x, (uint8_t)y, 0x01);
}


void     hal_paint_setPixelColor (coord_t x, coord_t y, color_t color)
{
    uint8_t tmp8;
    uint8_t  y0;
        
    if ((x < SCREEN_W) && (y < SCREEN_H)) {
        y0 = y & 0x07;
        y = y >> 3;
        tmp8 = screen_buf[y][x];
        if (COLOR_BLACK == color)
            tmp8 &= ~(1 << y0); //clr
        else
            tmp8 |=  (1 << y0); //set
        screen_buf[y][x] = tmp8;
    }
}

void paint_pixel_set (uint16_t x, uint16_t y)
{
    halLCD_setPixelColor ((uint8_t)x, (uint8_t)y, COLOR_WHITE);
}

void paint_pixel_clr (uint16_t x, uint16_t y)
{
    halLCD_setPixelColor ((uint8_t)x, (uint8_t)y, COLOR_BLACK);
}


color_t hal_paint_getPixel (coord_t x, coord_t y)
{
    uint8_t tmp8;
    uint8_t pix = 0x00;
    char  y0;
        
    if ((x < SCREEN_W) && (y < SCREEN_H)) {
        y0 = y & 0x07;//((SCREEN_H / 8) - 1);
        //y = y / (SCREEN_H / 8); //
        y = y >> 3;
        tmp8 = screen_buf[y][x];
        pix = tmp8 & (1 << y0);
        if (COLOR_BLACK == pix)
            return COLOR_BLACK;
        else
            return COLOR_WHITE;
    }
    return COLOR_WHITE;    
}


void hal_paint_repaint (void)
{
#if LCD_ENABLE_PARTIAL_UPDATE //update only 1/8
    if ++update_pos >= 4) update_pos = 0;
    halLCD_update_partial (update_pos);
#else
    halLCD_update_partial (0);
    halLCD_update_partial (1);
    halLCD_update_partial (2);
    halLCD_update_partial (3);
#endif
}


// clear everything
void hal_paint_cls (color_t color)
{
    /*
    uint8_t i, j;
    
    for (j = 0; j < 8; j++)
    {
        for (i = 0; i < 101; i++)
        {
            screen_buf[i][j] = 0x00;
        }
    }
    */
    memset (screen_buf, color, (SCREEN_W * (SCREEN_H / 8) ));
}


// массив всегда заполняется слева направао и с верху в низ
void     hal_paint_fillBlock (coord_t x, 
    coord_t y,
    uint16_t w, 
    uint16_t h, 
    color_t *buf) {
    coord_t xx, yy;
    
    for (xx = 0; xx < w; xx++)
        for (yy = 0; yy < h; yy++)
            hal_paint_setPixelColor (x + xx, y + yy, *buf++);
}


void     hal_paint_fillBlockColor (coord_t x,
    coord_t y,
    uint16_t w,
    uint16_t h,
    color_t color) {
    coord_t xx, yy;
    
    for (xx = 0; xx < w; xx++)
        for (yy = 0; yy < h; yy++)
            hal_paint_setPixelColor (x + xx, y + yy, color);
}


void hal_paint_setOrientation (uint8_t orient) {
}


                             
