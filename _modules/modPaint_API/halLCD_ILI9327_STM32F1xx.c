#include "halLCD.h"
#include "defines.h"
#include "board.h"
#include "modPaint.h"
#include "modPaint_local.h"

#include "modRandom.h"


#define LCD_RS              GPIO_Pin_4 //
#define GPIO_LCD_RS         GPIOA
#define LCD_CS              GPIO_Pin_2 //
#define GPIO_LCD_CS         GPIOA
#define LCD_WR              GPIO_Pin_5 //
#define GPIO_LCD_WR         GPIOA
#define LCD_RD              GPIO_Pin_6
#define GPIO_LCD_RD         GPIOA
#define LCD_RES             GPIO_Pin_3 //
#define GPIO_LCD_RES        GPIOA

#define LCD_RS_L            GPIO_LCD_RS->BRR  = LCD_RS
#define LCD_RS_H            GPIO_LCD_RS->BSRR = LCD_RS
#define LCD_CS_L            GPIO_LCD_CS->BRR  = LCD_CS
#define LCD_CS_H            GPIO_LCD_CS->BSRR = LCD_CS
#define LCD_WR_L            GPIO_LCD_WR->BRR  = LCD_WR
#define LCD_WR_H            GPIO_LCD_WR->BSRR = LCD_WR
#define LCD_RD_L            GPIO_LCD_RD->BRR  = LCD_RD
#define LCD_RD_H            GPIO_LCD_RD->BSRR = LCD_RD
#define LCD_RES_L           GPIO_LCD_RES->BRR  = LCD_RES
#define LCD_RES_H           GPIO_LCD_RES->BSRR = LCD_RES


void    _LCD_PORT_MODE_OUT (void)
{
    GPIOB->CRH |=  (GPIO_CRH_MODE8  
        | GPIO_CRH_MODE9
        | GPIO_CRH_MODE10
        | GPIO_CRH_MODE11
        | GPIO_CRH_MODE12
        | GPIO_CRH_MODE13
        | GPIO_CRH_MODE14
        | GPIO_CRH_MODE15);
    GPIOB->CRH &= ~(GPIO_CRH_CNF8
        | GPIO_CRH_CNF9
        | GPIO_CRH_CNF10
        | GPIO_CRH_CNF11
        | GPIO_CRH_CNF12
        | GPIO_CRH_CNF13
        | GPIO_CRH_CNF14
        | GPIO_CRH_CNF15);
}


void    _LCD_PORT_MODE_IN (void)
{
    //GPIOB->CRH = 0;
    
    GPIOB->CRH &=  ~(GPIO_CRH_MODE8  
        | GPIO_CRH_MODE9
        | GPIO_CRH_MODE10
        | GPIO_CRH_MODE11
        | GPIO_CRH_MODE12
        | GPIO_CRH_MODE13
        | GPIO_CRH_MODE14
        | GPIO_CRH_MODE15);
    
    GPIOB->CRH |= (GPIO_CRH_CNF8_0
        | GPIO_CRH_CNF9_0
        | GPIO_CRH_CNF10_0
        | GPIO_CRH_CNF11_0
        | GPIO_CRH_CNF12_0
        | GPIO_CRH_CNF13_0
        | GPIO_CRH_CNF14_0
        | GPIO_CRH_CNF15_0);
}
     
#define LCD_PORT_MODE_OUT   _LCD_PORT_MODE_OUT()
#define LCD_PORT_MODE_IN    _LCD_PORT_MODE_IN()

#define LCD_PORT_DATA       GPIOB



#if (LCD_ILI9327)

// структура параметров дисплея
struct halLCD_lcdStruct_t {
	uint8_t     orientation; // Ориентация
    color_t     color;
    coord_t     width;
    coord_t     height;
} halLCD_lcdStruct;


// некалиброванная задержка для процедуры инициализации LCD
void halLCD_delay (uint32_t delay)
{
    delay = delay * 1000UL;
    while (delay){ delay--; __NOP();}
}


void halLCD_setColor (color_t color)
{
    halLCD_lcdStruct.color = color;
}


#if LCD_SPI_MODE
#else // LCD_SPI_MODE
void  halLCD_wr (void)
{
    //LCD_PORT_MODE_OUT;
//     __NOP();
    __NOP();
	LCD_WR_L;
    __NOP();
    LCD_WR_H;
    __NOP();
    //LCD_PORT_MODE_IN;
}


void     halLCD_writeBusVL (uint8_t data)
{
	LCD_PORT_DATA->ODR = (uint16_t)(data << 8);
        
//     LCD_PORT_MODE_OUT;
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();LCD_WR_L;
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
// 	
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();    __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();LCD_WR_H;
//     __NOP();
//     __NOP();
//     __NOP();    __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//     __NOP();
//    // LCD_PORT_MODE_IN;
}

void     halLCD_writeBusVH (uint8_t data)
{
    GPIOA->BRR  = GPIO_Pin_0;
    GPIOA->BRR  = GPIO_Pin_1;
    GPIOA->BRR  = GPIO_Pin_9;
    GPIOA->BRR  = GPIO_Pin_10;
    GPIOA->BRR  = GPIO_Pin_11;
    GPIOA->BRR  = GPIO_Pin_12;
    
    GPIOB->BRR  = GPIO_Pin_6;
    GPIOB->BRR  = GPIO_Pin_7;
    

    if (data & 0x01)
        GPIOA->BSRR  = GPIO_Pin_0;    
    if (data & 0x02)
        GPIOA->BSRR  = GPIO_Pin_1;
    if (data & 0x04)
        GPIOA->BSRR  = GPIO_Pin_10;
    if (data & 0x08)
        GPIOA->BSRR  = GPIO_Pin_9;
    if (data & 0x10)
        GPIOA->BSRR  = GPIO_Pin_12;
    if (data & 0x20)
        GPIOA->BSRR  = GPIO_Pin_11;    
        
    if (data & 0x40)
        GPIOB->BSRR  = GPIO_Pin_7;
    if (data & 0x80)
        GPIOB->BSRR  = GPIO_Pin_6;  

}



uint8_t  halLCD_readBus (void)
{
    uint16_t data;
    LCD_PORT_MODE_IN;
	LCD_RD_L;
    __NOP();
    __NOP();
    data = LCD_PORT_DATA->IDR >> 8;
    LCD_RD_H;
    __NOP();
    __NOP();
    LCD_PORT_MODE_OUT;
    return data;
}


void     halLCD_writeCommand (uint8_t com)
{
    LCD_RS_L; // for command
    
    //halLCD_writeBusVL(0);
    //halLCD_wr ();
    //halLCD_writeBusVL(com);
    LCD_PORT_DATA->ODR = (uint16_t)(com << 8);
    halLCD_wr ();
    LCD_RS_H;
}


void     halLCD_writeData8bit (uint8_t VL)
{
	LCD_PORT_DATA->ODR = (uint16_t)(VL << 8); //halLCD_writeBusVL (VL);
    halLCD_wr ();
}


void     halLCD_writeData16bit (uint16_t data)
{
    //LCD_CS_L;
    LCD_PORT_DATA->ODR = (uint16_t)data;
    halLCD_wr ();
	LCD_PORT_DATA->ODR = (uint16_t)(data << 8); //halLCD_writeBusVL (VL);
    halLCD_wr ();
    
    //LCD_CS_H;
}


// void halLCD_writeCommand_DATA (uint8_t com, uint16_t data)
// {
//     halLCD_writeCommand(com);
//     halLCD_writeData16bit (data);
// }


// uint8_t  halLCD_readStatus (void)
// {
//     uint8_t data; // 8 bit only
//     LCD_RS_L;
//     data = halLCD_readData();
//     return data;
// }


// void     halLCD_writeReg (uint8_t reg, uint8_t val)
// {
//     LCD_RS_H; // for data
// 	halLCD_writeBus (reg);
//     halLCD_writeData (val);
// }


// uint8_t  halLCD_readReg (uint8_t reg)
// {
//     LCD_RS_H; // for data
// 	halLCD_writeBus (reg);
//     return halLCD_readStatus();
// }

#endif // LCD_SPI_MODE


//#define halLCD_writeCommand(a) halLCD_writeCommand(a)
//#define halLCD_writeData8bit(b) halLCD_writeData8bit(b)
//#define halLCD_writeData8bit(a,b) halLCD_writeData(a,b)
//#define delay(a) halLCD_delay(a)





void halLCD_init (uint8_t orientation)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    volatile uint8_t tmp8;
    uint16_t i,j,x1,x2,y1,y2;
    uint16_t tmp16;

    halLCD_lcdStruct.orientation = orientation;

    // init gpio-clocks
    
    // to default
    LCD_CS_H;
    LCD_RS_H;
    LCD_WR_H;
    LCD_RD_H;
    LCD_RES_H;
    
    GPIO_StructInit (&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = (LCD_RS | LCD_CS | LCD_WR | LCD_RD | LCD_RES); // 
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init (GPIOA, &GPIO_InitStructure);
    
    LCD_PORT_MODE_IN;
    LCD_PORT_MODE_OUT;
//     
//     GPIO_StructInit (&GPIO_InitStructure);
//     GPIO_InitStructure.GPIO_Pin   = (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12);
//     GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
//     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//     GPIO_Init (GPIOA, &GPIO_InitStructure);
//     
//     GPIO_StructInit (&GPIO_InitStructure);
//     GPIO_InitStructure.GPIO_Pin   = (GPIO_Pin_6 |GPIO_Pin_7);
//     GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
//     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//     GPIO_Init (GPIOB, &GPIO_InitStructure);
//     
//     GPIOA->BRR  = GPIO_Pin_0;
//     GPIOA->BRR  = GPIO_Pin_1;
//     GPIOA->BRR  = GPIO_Pin_9;
//     GPIOA->BRR  = GPIO_Pin_10;
//     GPIOA->BRR  = GPIO_Pin_11;
//     GPIOA->BRR  = GPIO_Pin_12;
//     
//     GPIOB->BRR  = GPIO_Pin_6;
//     GPIOB->BRR  = GPIO_Pin_7;
    

    LCD_RES_H;
	halLCD_delay(1); 
	LCD_RES_L;
	halLCD_delay(10);
	LCD_RES_H;
	halLCD_delay(50);

	LCD_CS_L;
    
    // check ID
    halLCD_writeCommand(0xEF);
    tmp8 = halLCD_readBus(); // 0xXX
    tmp8 = halLCD_readBus(); // 0x02
    tmp8 = halLCD_readBus(); // 0x04
    tmp16 = halLCD_readBus(); // 0x93
    tmp16 = tmp16 << 8;
    tmp16 |= halLCD_readBus(); // 0x27
    tmp8 = halLCD_readBus(); // 0xFF
    
    if (0x9327 == tmp16)
    {
    //     while(1)
    //     {
    //     
    //     }
        //@todo get diagnostic result
        
    //#define MCUFRIEND_35_TFTLCD_FOR_ARDUINO_2560_INIT_LCD2
        halLCD_writeCommand(0x01); // sw reset

        halLCD_writeCommand(0xE9);
        halLCD_writeData8bit(0x20);
        
        halLCD_writeCommand(0x11); //Exit Sleep
        halLCD_delay(100);
        
        halLCD_writeCommand(0xD1);//VCOM Control
        halLCD_writeData8bit(0x00);
        halLCD_writeData8bit(0x71);
        halLCD_writeData8bit(0x19);

        halLCD_writeCommand(0xD0);//Power_Setting
        halLCD_writeData8bit(0x07);
        halLCD_writeData8bit(0x01);
        halLCD_writeData8bit(0x08);

        halLCD_writeCommand(0x36);//set_address_mode
        
        //http://easyelectronics.ru/rabota-s-graficheskim-displeem-ssd1298.html
        switch(halLCD_lcdStruct.orientation) // Ориентация
        {
            case LCD_ORIENTATION_0:
                halLCD_lcdStruct.width  = LCD_CTRL_WIDTH;
                halLCD_lcdStruct.height = LCD_CTRL_HEIGHT;
                halLCD_writeData8bit(0x0A); //0x02 - правильнее
                break;
            
            case LCD_ORIENTATION_90:
                halLCD_lcdStruct.width  = LCD_CTRL_HEIGHT;
                halLCD_lcdStruct.height = LCD_CTRL_WIDTH;
                halLCD_writeData8bit(0xF8);
            
                break;

            case LCD_ORIENTATION_180:
                halLCD_lcdStruct.width  = LCD_CTRL_WIDTH;
                halLCD_lcdStruct.height = LCD_CTRL_HEIGHT;
                halLCD_writeData8bit(0x88);
            
                halLCD_writeCommand(0x37);//scroll
        halLCD_writeData16bit((432 - LCD_CTRL_HEIGHT));
        //halLCD_writeData8bit((432 - LCD_CTRL_HEIGHT) & 0xFF);
                break;
            
            case LCD_ORIENTATION_270:
                halLCD_lcdStruct.width  = LCD_CTRL_HEIGHT;
                halLCD_lcdStruct.height = LCD_CTRL_WIDTH;
                halLCD_writeData8bit(0xE8);
            
                halLCD_writeCommand(0x37);//scroll
        halLCD_writeData8bit((432 - LCD_CTRL_HEIGHT) >> 8);
        halLCD_writeData8bit((432 - LCD_CTRL_HEIGHT) & 0xFF);
                break;
            
            default: while(1){} break;
        }

        
        //halLCD_writeData8bit(0x48);//Page Address Order:Top to Bottom,Column Address Order:Right to Left,Page/Column Order:Normal Mode,Line Address Order:LCD Refresh Top to Bottom
                                        //RGB/BGR Order:Pixels sent in BGR order, Display Data Latch Data Order:Not supported, Horizontal Flip:Normal display, Vertical Flip: Normal display


        halLCD_writeCommand(0x3A);//set_pixel_format
        halLCD_writeData8bit(0x05);//DBI:16bit/pixel (65,536 colors)

        halLCD_writeCommand(0xC1);//Display_Timing_Setting for Normal/Partial Mode
        halLCD_writeData8bit(0x10);
        halLCD_writeData8bit(0x10);
        halLCD_writeData8bit(0x02);
        halLCD_writeData8bit(0x02);

        halLCD_writeCommand(0xC0); //Panel Driving Setting / Set Default Gamma
        halLCD_writeData8bit(0x00);
        halLCD_writeData8bit(0x35);
        halLCD_writeData8bit(0x00);
        halLCD_writeData8bit(0x00);
        halLCD_writeData8bit(0x01);
        halLCD_writeData8bit(0x02);

        halLCD_writeCommand(0xC5);  //Frame Rate Control / Set frame rate
        halLCD_writeData8bit(0x04);//72Hz

        halLCD_writeCommand(0xD2); //Power_Setting for Normal Mode / power setting
        halLCD_writeData8bit(0x01);//Gamma Driver Amplifier:1.00, Source Driver Amplifier: 1.00
        halLCD_writeData8bit(0x44);

        halLCD_writeCommand(0xC8); //Gamma Setting / Set Gamma
        halLCD_writeData8bit(0x04);
        halLCD_writeData8bit(0x67);
        halLCD_writeData8bit(0x35);
        halLCD_writeData8bit(0x04);
        halLCD_writeData8bit(0x08);
        halLCD_writeData8bit(0x06);
        halLCD_writeData8bit(0x24);
        halLCD_writeData8bit(0x01);
        halLCD_writeData8bit(0x37);
        halLCD_writeData8bit(0x40);
        halLCD_writeData8bit(0x03);
        halLCD_writeData8bit(0x10);
        halLCD_writeData8bit(0x08);
        halLCD_writeData8bit(0x80);
        halLCD_writeData8bit(0x00);

        
        
        
        halLCD_writeCommand(0x2A); //set_column_address
        halLCD_writeData8bit(0x00);
        halLCD_writeData8bit(0x00);
        halLCD_writeData8bit((halLCD_lcdStruct.width-1) >> 8);
        halLCD_writeData8bit((halLCD_lcdStruct.width-1));

        halLCD_writeCommand(0x2B); //set_page_address
        halLCD_writeData8bit(0x00);
        halLCD_writeData8bit(0x00);
        halLCD_writeData8bit((halLCD_lcdStruct.height-1) >> 8);
        halLCD_writeData8bit((halLCD_lcdStruct.height-1));
        
/*
        
        */
    //     halLCD_writeCommand(0x0A);
    //     tmp8 = halLCD_readBus(); // 0xXX
    //     tmp8 = halLCD_readBus(); // 0x9C
    /*
        halLCD_writeCommand(0x30);
        halLCD_writeData8bit(0x00);
        halLCD_writeData8bit(0x00);
        halLCD_writeData8bit((400-1) >> 8);
        halLCD_writeData8bit((400-1) & 0xFF);
        */
        halLCD_writeCommand(0x29); //set_display_on / display on
        halLCD_writeCommand(0x2C); //write_memory_start / display on
    }
    LCD_CS_H;
}




void setXY (coord_t x1, coord_t y1, coord_t x2, coord_t y2)
{
    x2 += x1;
    y2 += y1;
    
    halLCD_writeCommand (0x2A);//Set_column_address
  	halLCD_writeData8bit (x1 >> 8);
  	halLCD_writeData8bit (x1 & 0xFF);
  	halLCD_writeData8bit (x2 >> 8);
  	halLCD_writeData8bit (x2 & 0xFF);
  	halLCD_writeCommand (0x2B);//Set_page_address
  	halLCD_writeData8bit (y1 >> 8);
  	halLCD_writeData8bit (y1 & 0xFF);
  	halLCD_writeData8bit (y2 >> 8);
  	halLCD_writeData8bit (y2 & 0xFF);
    /*
    halLCD_writeCommand(0x2A);//Set_column_address
  	halLCD_writeData16bit(x1);//
  	halLCD_writeData16bit(x2);
  	halLCD_writeCommand(0x2B);//Set_page_address
  	halLCD_writeData16bit(y1);
  	halLCD_writeData16bit(y2);
    */
    halLCD_writeCommand (0x2C); //Write_memory_start @todo
}


void halLCD_setPixel (coord_t x, coord_t y)
{
	LCD_CS_L;
	setXY (x, y, x, y); //not matter? its only one
    //halLCD_writeData8bit(halLCD_lcdStruct.color >> 8);
    halLCD_writeData16bit(halLCD_lcdStruct.color);
	LCD_CS_H;
}


void     halLCD_setPixelColor (coord_t x, coord_t y, color_t color)
{
	LCD_CS_L;
	setXY (x, y, x, y); //not matter? its only one
	//halLCD_writeData8bit(color >> 8);
    halLCD_writeData16bit(color);
	LCD_CS_H;
}


color_t halLCD_getPixel (coord_t x, coord_t y)
{
    color_t tmp;
  	LCD_CS_L;
	setXY(x, y, x, y); //not matter? its only one
    halLCD_writeCommand(0x2E); //@todo
    halLCD_readBus(); // dummy read
    tmp = (uint16_t)(halLCD_readBus() << 8);
	tmp |= (uint16_t)(halLCD_readBus());
	LCD_CS_H;
    return tmp;
}


//#define enablePartialUpdate

// #ifdef enablePartialUpdate
// static uint8_t xUpdateMin, xUpdateMax, yUpdateMin, yUpdateMax;
// #endif
#if LCD_ENABLE_PARTIAL_UPDATE
uint8_t halLCD_cnt = 0;
#endif

void halLCD_repaint (void)
{
    // not implemented
}



// clear everything
void halLCD_cls (color_t color)
{
    uint32_t i;
    
  	LCD_CS_L;
	setXY (0, 0, LCD_CTRL_WIDTH -1, LCD_CTRL_HEIGHT -1);
    i = (uint32_t)(LCD_CTRL_WIDTH * LCD_CTRL_HEIGHT);
    
    
    
    LCD_PORT_DATA->ODR = 0;
    // @todo
    while (i--)
    {
        //halLCD_writeData8bit(color >> 8);
        //halLCD_writeData16bit(color);
        //halLCD_delay(10);
        /*
            LCD_PORT_DATA->ODR = (uint16_t)data;
    halLCD_wr ();
	LCD_PORT_DATA->ODR = (uint16_t)(data << 8); //halLCD_writeBusVL (VL);
    halLCD_wr ();
    */
        __NOP();
        LCD_WR_L;
        __NOP();
        LCD_WR_H;
        __NOP();
        
        __NOP();
        LCD_WR_L;
        __NOP();
        LCD_WR_H;
        __NOP();
    }
	LCD_CS_H;
}


// массив всегда заполняется слева направао и с верху в низ
void     halLCD_fillBlock (coord_t x, coord_t y, uint16_t w, uint16_t h, color_t *buf)
{
    uint32_t i;
    
  	LCD_CS_L;
	setXY (x, y, w -1, h -1);
    i = (uint32_t)(w * h);
    while (i--)
    {
        //halLCD_writeData8bit(*buf >> 8);
        halLCD_writeData16bit(*buf++);
    }
	LCD_CS_H;
}


void     halLCD_fillBlockColor (coord_t x, coord_t y, uint16_t w, uint16_t h, color_t color)
{
    uint32_t i;
    
  	LCD_CS_L;
	setXY (x, y, w -1, h -1);
    i = (uint32_t)(w * h);
    // @todo 
    while (i--)
    {
        //halLCD_wr();
        //halLCD_wr();
        //halLCD_writeData8bit(color >> 8);
        halLCD_writeData16bit(color);
    }
	LCD_CS_H;
}


void halLCD_setOrientation (uint8_t orient)
{

}


uint16_t halLCD_getWidth(void)
{
    return halLCD_lcdStruct.width;
}


uint16_t halLCD_getHeight(void)
{
    return halLCD_lcdStruct.height;
}


#endif
