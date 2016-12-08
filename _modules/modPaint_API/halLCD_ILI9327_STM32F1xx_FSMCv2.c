#include "halPaint.h"
#include "board.h"
#include "modPaint.h"
#include "modPaint_local.h"

#define LCD_FSMC        1


//register
//#define R34                     0x22
#define LCD_CS_H
#define LCD_CS_L

#define LCD_RES                 
#define GPIO_LCD_RES            
#define LCD_RES_L               GPIOB->BRR  = GPIO_PIN_4
#define LCD_RES_H               GPIOB->BSRR = GPIO_PIN_4


#if (LCD_ILI9327)

struct halLCD_lcdStruct_t {
	uint8_t     orientation; // Ориентация
    color_t     color;
    coord_t     width;
    coord_t     height;
} halLCD_lcdStruct;


void halLCD_setColor (color_t color)
{
    halLCD_lcdStruct.color = color;
}


#ifdef __KEIL__
static INLINE  
#endif
void     halLCD_writeCommand (uint8_t com)
{
    LCD_REG = com; // Write 16-bit Index, then Write Reg
}


#ifdef __KEIL__
static INLINE  
#endif
uint16_t halLCD_readCommand (void)
{
    volatile uint16_t tmp16;
    tmp16 = LCD_RAM; //LCD_REG;
    return tmp16;
}


#ifdef __KEIL__
static INLINE  
#endif
void halLCD_writeData8bit (uint8_t data)
{
    LCD_RAM = (uint16_t)data;
}


#ifdef __KEIL__
static INLINE 
#endif
void     halLCD_writeData16bit (uint16_t data)
{
    LCD_RAM = data;
}


void halFSMC_GPIO_init (void)
{
    /* PD.00(D2), PD.01(D3), PD.04(RD), PD.5(WR), PD.7(CS), PD.8(D13), PD.9(D14),
    PD.10(D15), PD.11(RS) PD.14(D0) PD.15(D1) */
    
    /* PE.07(D4), PE.08(D5), PE.09(D6), PE.10(D7), PE.11(D8), PE.12(D9),
    PE.13(D10), PE.14(D11), PE.15(D12) */
    
    //GPIOB->CRL |=  (GPIO_CRL_MODE4); //PB4 - LCD_RESET
    //GPIOB->CRL &= ~(GPIO_CRL_CNF4);
}


void halFSMC_init (void)
{
/*
    FSMC_NORSRAMTimingInitStructure.FSMC_AddressSetupTime = 2; //2
    FSMC_NORSRAMTimingInitStructure.FSMC_AddressHoldTime = 0;	   
    FSMC_NORSRAMTimingInitStructure.FSMC_DataSetupTime = 2;	//2   
    FSMC_NORSRAMTimingInitStructure.FSMC_BusTurnAroundDuration = 0x00;
    FSMC_NORSRAMTimingInitStructure.FSMC_CLKDivision = 0x00;
    FSMC_NORSRAMTimingInitStructure.FSMC_DataLatency = 0x00;
    FSMC_NORSRAMTimingInitStructure.FSMC_AccessMode = FSMC_AccessMode_A;

    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;
    FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
    FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 

    // Enable FSMC Bank1_SRAM Bank 
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE); 
    */
}


void hal_paint_init (uint8_t orientation)
{
    uint16_t tmp16;
    halLCD_lcdStruct.orientation = orientation;
    
    //halFSMC_GPIO_init ();
    //halFSMC_init ();
    
    LCD_RES_H;
	HAL_Delay (10); 
	LCD_RES_L;
	HAL_Delay (10);
	LCD_RES_H;
	HAL_Delay (50);
    
    LCD_CS_L;
    
    // check ID
    halLCD_writeCommand (0xEF); // Write 16-bit Index (then Read Reg)
    // Read 16-bit Reg
    tmp16 = halLCD_readCommand (); // 0xXX
    tmp16 = halLCD_readCommand (); // 0x02
    tmp16 = halLCD_readCommand (); // 0x04
    tmp16 = halLCD_readCommand (); // 0x93
    tmp16 = tmp16 << 8;
    tmp16 |= halLCD_readCommand (); // 0x27
    halLCD_readCommand (); // 0xFF - exit code
    
    if (0x9327 == tmp16)
    {
        //@todo get diagnostic result
    //#define MCUFRIEND_35_TFTLCD_FOR_ARDUINO_2560_INIT_LCD2
        halLCD_writeCommand (0x01); // soft reset

        halLCD_writeCommand (0xE9);
        halLCD_writeData8bit (0x20);
        
        halLCD_writeCommand (0x11); //Exit Sleep
        HAL_Delay (10);
        
        halLCD_writeCommand (0xD1);//VCOM Control
        halLCD_writeData8bit (0x00);
        halLCD_writeData8bit (0x71);
        halLCD_writeData8bit (0x19);

        halLCD_writeCommand (0xD0);//Power_Setting
        halLCD_writeData8bit (0x07);
        halLCD_writeData8bit (0x01);
        halLCD_writeData8bit (0x08);

        halLCD_writeCommand (0x36);//set_address_mode
        
        //http://easyelectronics.ru/rabota-s-graficheskim-displeem-ssd1298.html
        switch(halLCD_lcdStruct.orientation) // Ориентация
        {
        case SCREEN_ORIENTATION_0:
            halLCD_lcdStruct.width  = SCREEN_W;
            halLCD_lcdStruct.height = SCREEN_H;
            halLCD_writeData8bit (0x0A); //0x02 - правильнее
            break;
        
        case SCREEN_ORIENTATION_90:
            halLCD_lcdStruct.width  = SCREEN_H;
            halLCD_lcdStruct.height = SCREEN_W;
            halLCD_writeData8bit (0xF8);
        
            break;

        case SCREEN_ORIENTATION_180:
            halLCD_lcdStruct.width  = SCREEN_W;
            halLCD_lcdStruct.height = SCREEN_H;
            halLCD_writeData8bit (0x88);
        
            halLCD_writeCommand(0x37);//scroll
            halLCD_writeData16bit((432 - SCREEN_H));
    //halLCD_writeData8bit((432 - LCD_CTRL_HEIGHT) & 0xFF);
            break;
        
        case SCREEN_ORIENTATION_270:
            halLCD_lcdStruct.width  = SCREEN_H;
            halLCD_lcdStruct.height = SCREEN_W;
            halLCD_writeData8bit(0xE8);
        
            halLCD_writeCommand(0x37);//scroll
            halLCD_writeData8bit((432 - SCREEN_H) >> 8);
            halLCD_writeData8bit((432 - SCREEN_H) & 0xFF);
            break;
        
            //default: while(1){}; break;
        }

        
        //halLCD_writeData8bit(0x48);//Page Address Order:Top to Bottom,Column Address Order:Right to Left,Page/Column Order:Normal Mode,Line Address Order:LCD Refresh Top to Bottom
                                        //RGB/BGR Order:Pixels sent in BGR order, Display Data Latch Data Order:Not supported, Horizontal Flip:Normal display, Vertical Flip: Normal display


        halLCD_writeCommand (0x3A);//set_pixel_format
        halLCD_writeData8bit (0x05);//DBI:16bit/pixel (65,536 colors)

        halLCD_writeCommand (0xC1);//Display_Timing_Setting for Normal/Partial Mode
        halLCD_writeData8bit (0x10);
        halLCD_writeData8bit (0x10);
        halLCD_writeData8bit (0x02);
        halLCD_writeData8bit (0x02);

        halLCD_writeCommand (0xC0); //Panel Driving Setting / Set Default Gamma
        halLCD_writeData8bit (0x00);
        halLCD_writeData8bit (0x35);
        halLCD_writeData8bit (0x00);
        halLCD_writeData8bit (0x00);
        halLCD_writeData8bit (0x01);
        halLCD_writeData8bit (0x02);

        halLCD_writeCommand(0xC5);  //Frame Rate Control / Set frame rate
        halLCD_writeData8bit(0x04);//72Hz

        halLCD_writeCommand (0xD2); //Power_Setting for Normal Mode / power setting
        halLCD_writeData8bit (0x01);//Gamma Driver Amplifier:1.00, Source Driver Amplifier: 1.00
        halLCD_writeData8bit (0x44);

        halLCD_writeCommand (0xC8); //Gamma Setting / Set Gamma
        halLCD_writeData8bit (0x04);
        halLCD_writeData8bit (0x67);
        halLCD_writeData8bit (0x35);
        halLCD_writeData8bit (0x04);
        halLCD_writeData8bit (0x08);
        halLCD_writeData8bit (0x06);
        halLCD_writeData8bit (0x24);
        halLCD_writeData8bit (0x01);
        halLCD_writeData8bit (0x37);
        halLCD_writeData8bit (0x40);
        halLCD_writeData8bit (0x03);
        halLCD_writeData8bit (0x10);
        halLCD_writeData8bit (0x08);
        halLCD_writeData8bit (0x80);
        halLCD_writeData8bit (0x00);
        
        halLCD_writeCommand (0x2A); //set_column_address
        halLCD_writeData8bit (0x00);
        halLCD_writeData8bit (0x00);
        halLCD_writeData8bit ((halLCD_lcdStruct.width-1) >> 8);
        halLCD_writeData8bit ((halLCD_lcdStruct.width-1));

        halLCD_writeCommand (0x2B); //set_page_address
        halLCD_writeData8bit (0x00);
        halLCD_writeData8bit (0x00);
        halLCD_writeData8bit ((halLCD_lcdStruct.height-1) >> 8);
        halLCD_writeData8bit ((halLCD_lcdStruct.height-1));
        

    //     halLCD_writeCommand(0x0A);
    //     tmp8 = halLCD_readCommand ()(); // 0xXX
    //     tmp8 = halLCD_readCommand ()(); // 0x9C
    /*
        halLCD_writeCommand(0x30);
        halLCD_writeData8bit(0x00);
        halLCD_writeData8bit(0x00);
        halLCD_writeData8bit((400-1) >> 8);
        halLCD_writeData8bit((400-1) & 0xFF);
        */
        halLCD_writeCommand (0x29); //set_display_on / display on
        halLCD_writeCommand (0x2C); //write_memory_start / display on
    }
    LCD_CS_H;
}



void setXY (coord_t x1, coord_t y1, coord_t x2, coord_t y2)
{
    x2 += x1;
    y2 += y1;
    halLCD_writeCommand (0x2A);//Set_column_address
  	halLCD_writeData8bit (x1 >> 8); // 
  	halLCD_writeData8bit (x1 & 0xFF);
  	halLCD_writeData8bit (x2 >> 8);
  	halLCD_writeData8bit (x2 & 0xFF);
  	halLCD_writeCommand (0x2B);//Set_page_address
  	halLCD_writeData8bit (y1 >> 8);
  	halLCD_writeData8bit (y1 & 0xFF);
  	halLCD_writeData8bit (y2 >> 8);
  	halLCD_writeData8bit (y2 & 0xFF);
    halLCD_writeCommand (0x2C); //Write_memory_start @todo
}


void hal_paint_setPixel (coord_t x, coord_t y)
{
	LCD_CS_L;
	setXY (x, y, 0, 0); //not matter? its only one
    halLCD_writeData16bit (halLCD_lcdStruct.color);
	LCD_CS_H;
}


void     hal_paint_setPixelColor (coord_t x, coord_t y, color_t color)
{
	LCD_CS_L;
	setXY (x, y, 0, 0); //not matter? its only one
    halLCD_writeData16bit (color);
	LCD_CS_H;
}


color_t hal_paint_getPixel (coord_t x, coord_t y)
{
    color_t tmp;
  	LCD_CS_L;
	setXY (x, y, 0, 0); //not matter? its only one
    halLCD_writeCommand (0x2E); //@todo
    halLCD_readCommand (); // dummy read
	tmp = halLCD_readCommand ();
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

void hal_paint_repaint (void) {
    // not implemented
}


void hal_paint_cls (color_t color) {
    uint32_t i;
    
    LCD_CS_L;
    setXY (0, 0, SCREEN_W -1, SCREEN_H -1);
    i = (uint32_t)(SCREEN_W * SCREEN_H);
    while (i--) {
        halLCD_writeData16bit (color);
    }
    LCD_CS_H;
}


// массив всегда заполняется слева направао и с верху в низ
void     hal_paint_fillBlock (coord_t x, coord_t y, uint16_t w, uint16_t h, color_t *buf) {
    uint32_t i;
    
  	LCD_CS_L;
	setXY (x, y, w -1, h -1);
    i = (uint32_t)(w * h);
    while (i--)
    {
        halLCD_writeData16bit(*buf++);
    }
	LCD_CS_H;
}


void     hal_paint_fillBlockColor (coord_t x, coord_t y, uint16_t w, uint16_t h, color_t color) {
    uint32_t i;
    
  	LCD_CS_L;
	setXY (x, y, w -1, h -1);
    i = (uint32_t)(w * h);
    while (i--)
    {
        halLCD_writeData16bit(color);
    }
	LCD_CS_H;
}


void hal_paint_setOrientation (uint8_t orient)
{

}


uint16_t hal_paint_getWidth(void)
{
    return halLCD_lcdStruct.width;
}


uint16_t hal_paint_getHeight(void)
{
    return halLCD_lcdStruct.height;
}


/*
vu32 DMAComplete;
extern color_t paintBuf [LCD_BUF_SIZE]; // буфер для пикселей, вывод построчный
 
void halLCD_WriteDMA (coord_t x, coord_t y, uint16_t w, uint16_t h)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	
	NVIC_InitStructure.NVIC_IRQChannel =  DMA1_Channel2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	DMA_DeInit(DMA1_Channel2);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) paintBuf;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(&LCD_RAM);
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = w * h;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;
	DMA_Init(DMA1_Channel2, &DMA_InitStructure);
    
    LCD_CS_L;
	setXY (x, y, w, h);//LCD_SetWindow
	//LCD_SetCursor(x, y);
	//LCD_WriteRAM_Prepare();
    
	DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);
	DMA_SetCurrDataCounter(DMA1_Channel2, w * h);
	DMAComplete = 0;
	DMA_Cmd(DMA1_Channel2, ENABLE);
	while(DMAComplete == 0){};
    LCD_CS_H;
}

void DMA1_Channel2_IRQHandler(void)
{
	DMA_Cmd(DMA1_Channel2, DISABLE);
	DMAComplete = 1;
	DMA_ClearITPendingBit(DMA1_IT_GL2);
}
*/

#endif
