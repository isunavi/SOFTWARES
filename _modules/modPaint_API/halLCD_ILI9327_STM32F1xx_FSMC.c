#include "halLCD.h"
#include "defines.h"
#include "board.h"
#include "modPaint.h"
#include "modPaint_local.h"

//#include "modRandom.h"

#define LCD_FSMC        1

#if LCD_FSMC
#define GPIO_LCD_CS         GPIOD
#define LCD_CS              GPIO_Pin_7 //

#define LCD_CS_L            //GPIO_LCD_CS->BRR  = LCD_CS
#define LCD_CS_H            //GPIO_LCD_CS->BSRR = LCD_CS

// ������ ������
#define LCD_REG                 (*((volatile uint16_t *) 0x60000000)) // RS = 0 
#define LCD_RAM                 (*((volatile uint16_t *) 0x60020000)) // RS = 1 
//register
#define R34                     0x22

#else
#define GPIO_LCD_RS         GPIOD
#define LCD_RS              GPIO_Pin_11 //
#define GPIO_LCD_CS         GPIOD
#define LCD_CS              GPIO_Pin_7 //
#define GPIO_LCD_WR         GPIOD
#define LCD_WR              GPIO_Pin_5 //
#define GPIO_LCD_RD         GPIOD
#define LCD_RD              GPIO_Pin_4

#define LCD_RS_L            GPIO_LCD_RS->BRR  = LCD_RS
#define LCD_RS_H            GPIO_LCD_RS->BSRR = LCD_RS
#define LCD_CS_L            GPIO_LCD_CS->BRR  = LCD_CS
#define LCD_CS_H            GPIO_LCD_CS->BSRR = LCD_CS
#define LCD_WR_L            GPIO_LCD_WR->BRR  = LCD_WR
#define LCD_WR_H            GPIO_LCD_WR->BSRR = LCD_WR
#define LCD_RD_L            GPIO_LCD_RD->BRR  = LCD_RD
#define LCD_RD_H            GPIO_LCD_RD->BSRR = LCD_RD

#endif


#define LCD_RES                 GPIO_Pin_4 //
#define GPIO_LCD_RES            GPIOB
#define LCD_RES_L               GPIO_LCD_RES->BRR  = LCD_RES
#define LCD_RES_H               GPIO_LCD_RES->BSRR = LCD_RES


#if (LCD_ILI9327)

// ��������� ���������� �������
struct halLCD_lcdStruct_t {
	uint8_t     orientation; // ����������
    color_t     color;
    coord_t     width;
    coord_t     height;
} halLCD_lcdStruct;


// ��������������� �������� ��� ��������� ������������� LCD
void halLCD_delay (uint32_t delay)
{
    delay = delay * 1000UL;
    while (delay){ delay--; __NOP();}
}


void halLCD_setColor (color_t color)
{
    halLCD_lcdStruct.color = color;
}




#if LCD_FSMC
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
    volatile uint16_t tmp16;;
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

#else

void    _LCD_PORT_MODE_OUT (void)
{
    GPIOD->CRL |=  (GPIO_CRL_MODE1  //0,1
        | GPIO_CRL_MODE0);
    GPIOD->CRL &= ~(GPIO_CRL_CNF1
        | GPIO_CRL_CNF0);
    
    GPIOD->CRH |=  (GPIO_CRH_MODE8  //8,9,10,11,14,15
        | GPIO_CRH_MODE9
        | GPIO_CRH_MODE10
        | GPIO_CRH_MODE11
        | GPIO_CRH_MODE14
        | GPIO_CRH_MODE15);
    GPIOD->CRH &= ~(GPIO_CRH_CNF8
        | GPIO_CRH_CNF9
        | GPIO_CRH_CNF10
        | GPIO_CRH_CNF11
        | GPIO_CRH_CNF14
        | GPIO_CRH_CNF15);
    
    GPIOE->CRL |=  (GPIO_CRL_MODE7); //7
    GPIOE->CRL &= ~(GPIO_CRL_CNF7);
        
    GPIOE->CRH |=  (GPIO_CRH_MODE8  //8,9,10,11,12,13,14,15
        | GPIO_CRH_MODE9
        | GPIO_CRH_MODE10
        | GPIO_CRH_MODE11
        | GPIO_CRH_MODE12
        | GPIO_CRH_MODE13
        | GPIO_CRH_MODE14
        | GPIO_CRH_MODE15);
    GPIOE->CRH &= ~(GPIO_CRH_CNF8
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
    GPIOD->CRL &=  ~(GPIO_CRL_MODE1  //0,1
        | GPIO_CRL_MODE0);
    GPIOD->CRL |= (GPIO_CRL_CNF1_0
        | GPIO_CRL_CNF0_0);
    
    GPIOD->CRH &=  ~(GPIO_CRH_MODE8  //8,9,10,11,14,15
        | GPIO_CRH_MODE9
        | GPIO_CRH_MODE10
        | GPIO_CRH_MODE11
        | GPIO_CRH_MODE14
        | GPIO_CRH_MODE15);
    GPIOD->CRH |= (GPIO_CRH_CNF8_0
        | GPIO_CRH_CNF9_0
        | GPIO_CRH_CNF10_0
        | GPIO_CRH_CNF11_0
        | GPIO_CRH_CNF14_0
        | GPIO_CRH_CNF15_0);
    
    GPIOE->CRL &=  ~(GPIO_CRL_MODE7); //7
    GPIOE->CRL |= (GPIO_CRL_CNF7_0);
        
    GPIOE->CRH &=  ~(GPIO_CRH_MODE8  //8,9,10,11,12,13,14,15
        | GPIO_CRH_MODE9
        | GPIO_CRH_MODE10
        | GPIO_CRH_MODE11
        | GPIO_CRH_MODE12
        | GPIO_CRH_MODE13
        | GPIO_CRH_MODE14
        | GPIO_CRH_MODE15);
    GPIOE->CRH |= (GPIO_CRH_CNF8_0
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

void LCD_PORT_DATA_WR (uint16_t data)
{
    GPIOD->ODR &= ~((0x0003) << 14);
    GPIOD->ODR |= ((data & 0x0003) << 14);
    GPIOD->ODR &= ~((0x000C) >> 2);
    GPIOD->ODR |= ((data & 0x000C) >> 2);
    GPIOE->ODR &= ~((0x1FF0) << 3);
    GPIOE->ODR |= ((data & 0x1FF0) << 3);
    GPIOD->ODR &= ~((0xE000) >> 5);
    GPIOD->ODR |= ((data & 0xE000) >> 5);
}


uint16_t LCD_PORT_DATA_RD (void)
{
    uint16_t tmp16 = 0;
    
    __NOP();
    __NOP();
    tmp16 |= ((GPIOD->IDR & 0xC000) >> 14);
    tmp16 |= ((GPIOD->IDR & 0x0003) << 2);
    tmp16 |= ((GPIOE->IDR & 0xFF80) >> 3);
    tmp16 |= ((GPIOD->IDR & 0x0700) << 5);
    return tmp16;
}


void  halLCD_wr (void)
{
    __NOP();
	LCD_WR_L;
    __NOP();
    LCD_WR_H;
    __NOP();
}


uint16_t  halLCD_readCommand (void)
{
    uint16_t data;
    LCD_PORT_MODE_IN;
	LCD_RD_L;
    data = LCD_PORT_DATA_RD(); //LCD_PORT_DATA->IDR >> 8;
    LCD_RD_H;
    LCD_PORT_MODE_OUT;
    return data;
}


void     halLCD_writeCommand (uint8_t com)
{
    LCD_RS_L; // for command
    LCD_PORT_DATA_WR(com); //LCD_PORT_DATA->ODR = (uint16_t)(com << 8);
    halLCD_wr ();
    LCD_RS_H;
}


void     halLCD_writeData8bit (uint8_t data)
{
	LCD_PORT_DATA_WR ((uint16_t)data); //LCD_PORT_DATA->ODR = (uint16_t)(VL << 8); //halLCD_writeBusVL (VL);
    halLCD_wr ();
}


void     halLCD_writeData16bit (uint16_t data)
{
    LCD_PORT_DATA_WR (data); //LCD_PORT_DATA->ODR = (uint16_t)data;
    halLCD_wr ();
}

#endif


#if LCD_FSMC
void halFSMC_GPIO_init (void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPDEN | RCC_APB2ENR_IOPEEN;
    
    /* PD.00(D2), PD.01(D3), PD.04(RD), PD.5(WR), PD.7(CS), PD.8(D13), PD.9(D14),
    PD.10(D15), PD.11(RS) PD.14(D0) PD.15(D1) */
    GPIO_StructInit (&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
         GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_14 | GPIO_Pin_15; //
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* PE.07(D4), PE.08(D5), PE.09(D6), PE.10(D7), PE.11(D8), PE.12(D9),
    PE.13(D10), PE.14(D11), PE.15(D12) */
    GPIO_StructInit (&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
        GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
//     GPIO_StructInit (&GPIO_InitStructure);
//     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
//     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//     GPIO_Init(GPIOB, &GPIO_InitStructure);
//      
//     GPIOD->CRL |=  (GPIO_CRL_MODE7); //PB4 - LCD_CS
//     GPIOD->CRL &= ~(GPIO_CRL_CNF7);
    
    GPIOB->CRL |=  (GPIO_CRL_MODE4); //PB4 - LCD_RESET
    GPIOB->CRL &= ~(GPIO_CRL_CNF4);
}


void halFSMC_init (void)
{
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef FSMC_NORSRAMTimingInitStructure;
    
    RCC->AHBENR |= RCC_AHBENR_FSMCEN;
    
/*  
    FSMC_NORSRAMTimingInitStructure.FSMC_AddressSetupTime = 0;  //0
    FSMC_NORSRAMTimingInitStructure.FSMC_AddressHoldTime = 0;   //0
    FSMC_NORSRAMTimingInitStructure.FSMC_DataSetupTime = 2;     //3
    FSMC_NORSRAMTimingInitStructure.FSMC_BusTurnAroundDuration = 0;
    FSMC_NORSRAMTimingInitStructure.FSMC_CLKDivision = 1;//1
    FSMC_NORSRAMTimingInitStructure.FSMC_DataLatency = 0;
    FSMC_NORSRAMTimingInitStructure.FSMC_AccessMode = FSMC_AccessMode_A;
    
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
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Enable;//disable
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;
    
    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);
    FSMC_NORSRAMTimingInitStructure.FSMC_AddressSetupTime = 0;    //0
    FSMC_NORSRAMTimingInitStructure.FSMC_AddressHoldTime = 0;   //0
    FSMC_NORSRAMTimingInitStructure.FSMC_DataSetupTime = 4;   //3
    FSMC_NORSRAMTimingInitStructure.FSMC_BusTurnAroundDuration = 0;
    FSMC_NORSRAMTimingInitStructure.FSMC_CLKDivision = 1;//1
    FSMC_NORSRAMTimingInitStructure.FSMC_DataLatency = 0;
    FSMC_NORSRAMTimingInitStructure.FSMC_AccessMode = FSMC_AccessMode_A;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;
    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);
    
    //Enable FSMC Bank1_SRAM Bank 
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);
   */
   
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
    
}

#else
void halLCD_GPIO_init (void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPDEN | RCC_APB2ENR_IOPEEN;
    
    GPIOD->CRL |=  (GPIO_CRL_MODE7 | GPIO_CRL_MODE5 | GPIO_CRL_MODE4);
    GPIOD->CRL &= ~(GPIO_CRL_CNF7 | GPIO_CRL_CNF5 | GPIO_CRL_CNF4);
    
    GPIOD->CRH |=  (GPIO_CRH_MODE11);
    GPIOD->CRH &= ~(GPIO_CRH_CNF11);
    
    GPIOB->CRL |=  (GPIO_CRL_MODE4); //PB4 - LCD_RESET
    GPIOB->CRL &= ~(GPIO_CRL_CNF4);
    
    LCD_CS_H;
    LCD_RS_H;
    LCD_WR_H;
    LCD_RD_H;
    LCD_RES_H;
}

#endif

void halLCD_init (uint8_t orientation)
{
    uint16_t tmp16;
    halLCD_lcdStruct.orientation = orientation;
    
#if LCD_FSMC
    halFSMC_GPIO_init ();
    halFSMC_init ();
#else
    halLCD_GPIO_init ();
    LCD_PORT_MODE_OUT;
#endif
    
    LCD_RES_H;
	halLCD_delay (10); 
	LCD_RES_L;
	halLCD_delay (10);
	LCD_RES_H;
	halLCD_delay (500);
    
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
        //LED_GREEN_ON;
    //#define MCUFRIEND_35_TFTLCD_FOR_ARDUINO_2560_INIT_LCD2
        halLCD_writeCommand(0x01); // soft reset

        halLCD_writeCommand(0xE9);
        halLCD_writeData8bit(0x20);
        
        halLCD_writeCommand(0x11); //Exit Sleep
        halLCD_delay (100);
        
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
        switch(halLCD_lcdStruct.orientation) // ����������
        {
            case LCD_ORIENTATION_0:
                halLCD_lcdStruct.width  = SCREEN_W;
                halLCD_lcdStruct.height = SCREEN_H;
                halLCD_writeData8bit(0x0A); //0x02 - ����������
                break;
            
            case LCD_ORIENTATION_90:
                halLCD_lcdStruct.width  = SCREEN_H;
                halLCD_lcdStruct.height = SCREEN_W;
                halLCD_writeData8bit(0xF8);
            
                break;

            case LCD_ORIENTATION_180:
                halLCD_lcdStruct.width  = SCREEN_W;
                halLCD_lcdStruct.height = SCREEN_H;
                halLCD_writeData8bit(0x88);
            
                halLCD_writeCommand(0x37);//scroll
        halLCD_writeData16bit((432 - SCREEN_H));
        //halLCD_writeData8bit((432 - LCD_CTRL_HEIGHT) & 0xFF);
                break;
            
            case LCD_ORIENTATION_270:
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
    //     tmp8 = halLCD_readCommand ()(); // 0xXX
    //     tmp8 = halLCD_readCommand ()(); // 0x9C
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


void halLCD_setPixel (coord_t x, coord_t y)
{
	LCD_CS_L;
	setXY (x, y, 0, 0); //not matter? its only one
    halLCD_writeData16bit(halLCD_lcdStruct.color);
	LCD_CS_H;
}


void     halLCD_setPixelColor (coord_t x, coord_t y, color_t color)
{
	LCD_CS_L;
	setXY (x, y, 0, 0); //not matter? its only one
    halLCD_writeData16bit(color);
	LCD_CS_H;
}


color_t halLCD_getPixel (coord_t x, coord_t y)
{
    color_t tmp;
  	LCD_CS_L;
	setXY(x, y, 0, 0); //not matter? its only one
    halLCD_writeCommand(0x2E); //@todo
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

void halLCD_repaint (void)
{
    // not implemented
}


void halLCD_cls (color_t color)
{
    uint32_t i;
    
    LCD_CS_L;
    setXY (0, 0, SCREEN_W -1, SCREEN_H -1);
    i = (uint32_t)(SCREEN_W * SCREEN_H);
    while (i--)
    {
        halLCD_writeData16bit (color);
    }
    LCD_CS_H;
}


// ������ ������ ����������� ����� �������� � � ����� � ���
void     halLCD_fillBlock (coord_t x, coord_t y, uint16_t w, uint16_t h, color_t *buf)
{
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


void     halLCD_fillBlockColor (coord_t x, coord_t y, uint16_t w, uint16_t h, color_t color)
{
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


vu32 DMAComplete;
extern color_t paintBuf [LCD_BUF_SIZE]; // ����� ��� ��������, ����� ����������
 
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


#endif
