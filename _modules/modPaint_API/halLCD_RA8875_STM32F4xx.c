#include "halLCD.h"
#include "defines.h"
#include "board.h"
#include "modPaint.h"

/**
 PD0  -> CS  (5)
 PC11 <- SDO (6)
 PC12 -> SDI (7)
 PC10 -> SCLK(8)
 
*/

// @todo
// need to short J16 and open J15 on PCB .

#ifdef LCD_RA8875

// Command/Data pins for SPI
#define RA8875_DATAWRITE        0x00
#define RA8875_DATAREAD         0x40
#define RA8875_CMDWRITE         0x80
#define RA8875_CMDREAD          0xC0

// Registers & bits
#define RA8875_PWRR             0x01
#define RA8875_PWRR_DISPON      0x80
#define RA8875_PWRR_DISPOFF     0x00
#define RA8875_PWRR_SLEEP       0x02
#define RA8875_PWRR_NORMAL      0x00
#define RA8875_PWRR_SOFTRESET   0x01

#define RA8875_MRWC             0x02

#define RA8875_GPIOX            0xC7

#define RA8875_PLLC1            0x88
#define RA8875_PLLC1_PLLDIV2    0x80
#define RA8875_PLLC1_PLLDIV1    0x00

#define RA8875_PLLC2            0x89
#define RA8875_PLLC2_DIV1       0x00
#define RA8875_PLLC2_DIV2       0x01
#define RA8875_PLLC2_DIV4       0x02
#define RA8875_PLLC2_DIV8       0x03
#define RA8875_PLLC2_DIV16      0x04
#define RA8875_PLLC2_DIV32      0x05
#define RA8875_PLLC2_DIV64      0x06
#define RA8875_PLLC2_DIV128     0x07


#define RA8875_SYSR             0x10
#define RA8875_SYSR_8BPP        0x00
#define RA8875_SYSR_16BPP       0x0C
#define RA8875_SYSR_MCU8        0x00
#define RA8875_SYSR_MCU16       0x03

#define RA8875_PCSR             0x04
#define RA8875_PCSR_PDATR       0x00
#define RA8875_PCSR_PDATL       0x80
#define RA8875_PCSR_CLK         0x00
#define RA8875_PCSR_2CLK        0x01
#define RA8875_PCSR_4CLK        0x02
#define RA8875_PCSR_8CLK        0x03

#define RA8875_HDWR             0x14

#define RA8875_HNDFTR           0x15
#define RA8875_HNDFTR_DE_HIGH   0x00
#define RA8875_HNDFTR_DE_LOW    0x80

#define RA8875_HNDR             0x16
#define RA8875_HSTR             0x17
#define RA8875_HPWR             0x18
#define RA8875_HPWR_LOW         0x00
#define RA8875_HPWR_HIGH        0x80

#define RA8875_VDHR0            0x19
#define RA8875_VDHR1            0x1A
#define RA8875_VNDR0            0x1B
#define RA8875_VNDR1            0x1C
#define RA8875_VSTR0            0x1D
#define RA8875_VSTR1            0x1E
#define RA8875_VPWR             0x1F
#define RA8875_VPWR_LOW         0x00
#define RA8875_VPWR_HIGH        0x80

#define RA8875_HSAW0            0x30
#define RA8875_HSAW1            0x31
#define RA8875_VSAW0            0x32
#define RA8875_VSAW1            0x33

#define RA8875_HEAW0            0x34
#define RA8875_HEAW1            0x35
#define RA8875_VEAW0            0x36
#define RA8875_VEAW1            0x37

#define RA8875_MCLR             0x8E
#define RA8875_MCLR_START       0x80
#define RA8875_MCLR_STOP        0x00
#define RA8875_MCLR_READSTATUS  0x80
#define RA8875_MCLR_FULL        0x00
#define RA8875_MCLR_ACTIVE      0x40

#define RA8875_DCR                    0x90
#define RA8875_DCR_LINESQUTRI_START   0x80
#define RA8875_DCR_LINESQUTRI_STOP    0x00
#define RA8875_DCR_LINESQUTRI_STATUS  0x80
#define RA8875_DCR_CIRCLE_START       0x40
#define RA8875_DCR_CIRCLE_STATUS      0x40
#define RA8875_DCR_CIRCLE_STOP        0x00
#define RA8875_DCR_FILL               0x20
#define RA8875_DCR_NOFILL             0x00
#define RA8875_DCR_DRAWLINE           0x00
#define RA8875_DCR_DRAWTRIANGLE       0x01
#define RA8875_DCR_DRAWSQUARE         0x10

#define RA8875_ELLIPSE                0xA0
#define RA8875_ELLIPSE_STATUS         0x80

#define RA8875_MWCR0            0x40
#define RA8875_MWCR0_GFXMODE    0x00
#define RA8875_MWCR0_TXTMODE    0x80

#define RA8875_MWCR1            0x41

#define RA8875_CURH0            0x46
#define RA8875_CURH1            0x47
#define RA8875_CURV0            0x48
#define RA8875_CURV1            0x49

// PWM
#define RA8875_P1CR             0x8A
#define RA8875_P1CR_ENABLE      0x80
#define RA8875_P1CR_DISABLE     0x00
#define RA8875_P1CR_CLKOUT      0x10
#define RA8875_P1CR_PWMOUT      0x00

#define RA8875_P1DCR            0x8B

#define RA8875_P2CR             0x8C
#define RA8875_P2CR_ENABLE      0x80
#define RA8875_P2CR_DISABLE     0x00
#define RA8875_P2CR_CLKOUT      0x10
#define RA8875_P2CR_PWMOUT          0x00

#define RA8875_P2DCR                    0x8D
       
#define RA8875_PWM_CLK_DIV1             0x00
#define RA8875_PWM_CLK_DIV2             0x01
#define RA8875_PWM_CLK_DIV4             0x02
#define RA8875_PWM_CLK_DIV8             0x03
#define RA8875_PWM_CLK_DIV16            0x04
#define RA8875_PWM_CLK_DIV32            0x05
#define RA8875_PWM_CLK_DIV64            0x06
#define RA8875_PWM_CLK_DIV128           0x07
#define RA8875_PWM_CLK_DIV256           0x08
#define RA8875_PWM_CLK_DIV512           0x09
#define RA8875_PWM_CLK_DIV1024          0x0A
#define RA8875_PWM_CLK_DIV2048          0x0B
#define RA8875_PWM_CLK_DIV4096          0x0C
#define RA8875_PWM_CLK_DIV8192          0x0D
#define RA8875_PWM_CLK_DIV16384         0x0E
#define RA8875_PWM_CLK_DIV32768         0x0F

// регистры тачпада (резистивного) если есть
#define RA8875_TPCR0                    0x70
#define RA8875_TPCR0_ENABLE             0x80
#define RA8875_TPCR0_DISABLE          0x00
#define RA8875_TPCR0_WAIT_512CLK      0x00
#define RA8875_TPCR0_WAIT_1024CLK     0x10
#define RA8875_TPCR0_WAIT_2048CLK     0x20
#define RA8875_TPCR0_WAIT_4096CLK     0x30
#define RA8875_TPCR0_WAIT_8192CLK     0x40
#define RA8875_TPCR0_WAIT_16384CLK    0x50
#define RA8875_TPCR0_WAIT_32768CLK    0x60
#define RA8875_TPCR0_WAIT_65536CLK    0x70
#define RA8875_TPCR0_WAKEENABLE       0x08
#define RA8875_TPCR0_WAKEDISABLE      0x00
#define RA8875_TPCR0_ADCCLK_DIV1      0x00
#define RA8875_TPCR0_ADCCLK_DIV2      0x01
#define RA8875_TPCR0_ADCCLK_DIV4      0x02
#define RA8875_TPCR0_ADCCLK_DIV8      0x03
#define RA8875_TPCR0_ADCCLK_DIV16     0x04
#define RA8875_TPCR0_ADCCLK_DIV32     0x05
#define RA8875_TPCR0_ADCCLK_DIV64     0x06
#define RA8875_TPCR0_ADCCLK_DIV128    0x07

#define RA8875_TPCR1            0x71
#define RA8875_TPCR1_AUTO       0x00
#define RA8875_TPCR1_MANUAL     0x40
#define RA8875_TPCR1_VREFINT    0x00
#define RA8875_TPCR1_VREFEXT    0x20
#define RA8875_TPCR1_DEBOUNCE   0x04
#define RA8875_TPCR1_NODEBOUNCE 0x00
#define RA8875_TPCR1_IDLE       0x00
#define RA8875_TPCR1_WAIT       0x01
#define RA8875_TPCR1_LATCHX     0x02
#define RA8875_TPCR1_LATCHY     0x03

#define RA8875_TPXH             0x72
#define RA8875_TPYH             0x73
#define RA8875_TPXYL            0x74

#define RA8875_INTC1            0xF0
#define RA8875_INTC1_KEY        0x10
#define RA8875_INTC1_DMA        0x08
#define RA8875_INTC1_TP         0x04
#define RA8875_INTC1_BTE        0x02

#define RA8875_INTC2            0xF1
#define RA8875_INTC2_KEY        0x10
#define RA8875_INTC2_DMA        0x08
#define RA8875_INTC2_TP         0x04
#define RA8875_INTC2_BTE        0x02

// регистры цвета
#define RA8875_BGCR0 0x60
#define RA8875_BGCR1 0x61
#define RA8875_BGCR2 0x62

#define RA8875_FGCR0 0x63
#define RA8875_FGCR1 0x64
#define RA8875_FGCR2 0x65


#define RA8875_STSR 0x00 // статусный регистр


#define RA8875_BECR0 0x50 // BTE Function Control Register0
#define RA8875_BECR0_BTE 0x80 
#define RA8875_BECR1 0x51 // BTE Function Control Register1
#define RA8875_BECR1_ROP 

#define RA8875_HSBE0 0x54 // Horizontal Source Point 0 of BTE 
#define RA8875_HSBE1 0x55 // Horizontal Source Point 1 of BTE
#define RA8875_VSBE0 0x56 // Vertical Source Point 0 of BTE 
#define RA8875_VSBE1 0x57 // Vertical Source Point 1 of BTE 
#define RA8875_HDBE0 0x58 // Horizontal Destination Point 0 of BTE
#define RA8875_HDBE1 0x59 // Horizontal Destination Point 1 of BTE
#define RA8875_VDBE0 0x5A // Vertical Destination Point 0 of BTE
#define RA8875_VDBE1 0x5B // Vertical Destination Point 1 of BTE

#define RA8875_BTE_LAYER 0x80

#define RA8875_BEWR0 0x5C // BTE Width Register 0
#define RA8875_BEWR1 0x5D // BTE Width Register 0
#define RA8875_BEHR0 0x5E // BTE Height Register 0
#define RA8875_BEHR1 0x5F // BTE Height Register 1

#define RA8875_DLHSR0 0x91 // Draw Line/Square Horizontal Start Address Register0
#define RA8875_DLHSR1 0x92 // Draw Line/Square Horizontal Start Address Register1
#define RA8875_DLVSR0 0x93 // Draw Line/Square Vertical Start Address Register0
#define RA8875_DLVSR1 0x94 // Draw Line/Square Vertical Start Address Register1
#define RA8875_DLHER0 0x95 // Draw Line/Square Horizontal End Address Register0
#define RA8875_DLHER1 0x96 // Draw Line/Square Horizontal End Address Register1
#define RA8875_DLVER0 0x97 // Draw Line/Square Vertical End Address Register0
#define RA8875_DLVER1 0x98 // Draw Line/Square Vertical End Address Register1

// #define RA8875_DCR_LINESQUTRI_START   0x80

// #define RA8875_DCR_LINESQUTRI_STATUS  0x80

#define RA8875_LTPR0 0x52 //
#define RA8875_LTPR1 0x53
#define RA8875_LTPR0_MODE_L1 0x00
#define RA8875_LTPR0_MODE_L2 0x01

#define RA8875_DPCR 0x20 // Display Configuration Register 


uint8_t scr_number = 0; // номер слоя

#include "debug.h"

// структура параметров дисплея
struct halLCD_lcdStruct_t {
	uint8_t       orientation; // Ориентация
    color_t       color;
} halLCD_lcdStruct;


// некалиброванная задержка для процедуры инициализации LCD
void halLCD_delay (uint32_t delay) {
    delay = delay * 1000UL;
    while (delay){ delay--; }
}




/************************* Low Level ***********************************/
#ifdef LCD_SPI_MODE

void halLCD_SPI_init (uint32_t baud) {
    volatile uint8_t tmp8;
    GPIO_InitTypeDef GPIO_InitStructure;
    
#if LCD_SPI_HAL
    SPI_InitTypeDef  SPI_InitStructure;

    //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);

    /* Configure I/O for Flash Chip select */
    GPIO_StructInit (&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_LCD_CS;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
    
	/* Configure SPI pins: SCK-GPIO_Pin_10 and MOSI-GPIO_Pin_12 with default alternate function (not re-mapped) push-pull */
	GPIO_StructInit (&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_LCD_MISO | GPIO_Pin_LCD_MOSI | GPIO_Pin_LCD_CLK;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP; //GPIO_PuPd_NOPULL
	GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    //
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SPI3);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3);
    
    /* SPI configuration */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    //       __   ___
    // Clock   ___A     Rising edge latched
    //       ___ ____
    // Data  ___X____
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High; // По минусу SCK
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge; // Положительный фронт
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; // Программный
	SPI_InitStructure.SPI_BaudRatePrescaler = baud; // 42000kHz/64=281kHz < 400kHz
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7; // Полином, все равно отключим

	SPI_Init (LCD_SPI, &SPI_InitStructure);
	SPI_CalculateCRC (LCD_SPI, DISABLE);
	SPI_Cmd (LCD_SPI, ENABLE);

	/* drain SPI */
	while (SPI_I2S_GetFlagStatus (LCD_SPI, SPI_I2S_FLAG_TXE) == RESET) {};
    tmp8 = SPI_I2S_ReceiveData (LCD_SPI);
    
#else // LCD_SPI_HAL
    SPI_InitTypeDef  SPI_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    GPIO_StructInit (&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_LCD_CS;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_StructInit (&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_LCD_MOSI |  
        GPIO_Pin_LCD_CLK;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    GPIO_StructInit (&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_LCD_MISO;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
    
#endif // LCD_SPI_HAL        
}


uint8_t halLCD_xspi (uint8_t byte) {
#if LCD_SPI_HAL
    uint8_t res;
    LCD_SPI->DR = byte; // write data to be transmitted to the SPI data register
	//while (!(LCD_SPI->SR & SPI_I2S_FLAG_TXE)){}; // wait until transmit complete
	while (!(LCD_SPI->SR & SPI_I2S_FLAG_RXNE)){}; // wait until receive complete
	//while (LCD_SPI->SR & SPI_I2S_FLAG_BSY){}; // wait until SPI is not busy anymore
//     halLCD_delay (1); // @todo
//     __NOP();
    res = LCD_SPI->DR;
    return res; // return received data from SPI data register
#else
    uint8_t i, res;
    
    res = 0;
    for (i = 0; i < 8; i++) { //@todo! неправильно, поменять активные фронты
        if (byte & 0x80) {
            LCD_MOSI_H;
        } else {
            LCD_MOSI_L;
        }
        byte = byte << 1;
        halLCD_delay (100);
        LCD_CLK_L; // синхроимпульс фронт
      
        halLCD_delay (100);
	    if (0 != LCD_MISO_IN) { res = res | 0x01; }
        res = res << 1;
        LCD_CLK_H; // синхроимпульс спад
        
    }
    return res;
#endif
}



// выбор максимальной и минимальной частоты (для инициализации)
enum    SPI_DIV {
    SPI_CLOCK_MIN = 0,
    SPI_CLOCK_MAX
};


void    halLCD_SPI_setCLK (uint8_t div) {
//     switch (div) {
//     case SPI_CLOCK_MAX: // @todo возможно придется уменьшить
//         LCD_SPI->CR1 &= ~SPI_BaudRatePrescaler_256;
//         LCD_SPI->CR1 |= SPI_BaudRatePrescaler_16;
//         break;
//     case SPI_CLOCK_MIN:
//         LCD_SPI->CR1 &= ~SPI_BaudRatePrescaler_256;
//         LCD_SPI->CR1 |= SPI_BaudRatePrescaler_64;
//         break;
//     default: break;
//     }
}

void     halLCD_writeData (uint8_t data) {
    LCD_CS_L;
    halLCD_xspi (RA8875_DATAWRITE);
    halLCD_xspi (data);
    LCD_CS_H;
}

uint8_t  halLCD_readData (void) {
    uint8_t x;
    LCD_CS_L;
    halLCD_xspi (RA8875_DATAREAD);
    x = halLCD_xspi (0x00);
    LCD_CS_H;
    return x;
}

void     halLCD_writeCommand (uint8_t data) {
    LCD_CS_L;
    halLCD_xspi (RA8875_CMDWRITE);
    halLCD_xspi (data);
    LCD_CS_H;
}

uint8_t  halLCD_readStatus (void) {
    uint8_t data;
    LCD_CS_L;
    halLCD_xspi (RA8875_CMDREAD);
    data = halLCD_xspi (0x00);
    LCD_CS_H;
    return data;
}


void     halLCD_writeReg (uint8_t reg, uint8_t val) {
    halLCD_writeCommand (reg);
    halLCD_writeData (val);
}


uint8_t  halLCD_readReg (uint8_t reg) {
    halLCD_writeCommand (reg);
    return halLCD_readData ();
}

#else // LCD_SPI_MODE

void     halLCD_writeBus (uint8_t data) {
    LCD_CS_L;	
	LCD_PORT_DATA = data;	
	LCD_WR_L;
    __NOP();
	LCD_WR_H;
    LCD_CS_H;
}


void     halLCD_writeData (uint8_t data) {
    LCD_CS_L;
    LCD_RS_L;
	halLCD_writeBus(data);
    LCD_CS_H;
}


uint8_t  halLCD_readData (void) {
    uint16_t data;
    LCD_PORT_MODE_IN;
    LCD_CS_L;
	LCD_RD_L;
    __NOP();
    data = LCD_PORT_DATA;
    LCD_RD_H;
    LCD_PORT_MODE_OUT;
    LCD_CS_H;
    return data;
}


void     halLCD_writeCommand (uint8_t data) {
    LCD_CS_L;
    LCD_RS_L; // for command
    halLCD_writeData(data);
    LCD_CS_H;
}


uint8_t  halLCD_readStatus (void) {
    uint8_t data; // 8 bit only
    LCD_RS_L;
    data = halLCD_readData();
    return data;
}


void     halLCD_writeReg (uint8_t reg, uint8_t val) {
    LCD_RS_H; // for data
	halLCD_writeBus (reg);
    halLCD_writeData (val);
}


uint8_t  halLCD_readReg (uint8_t reg) {
    LCD_RS_H; // for data
	halLCD_writeBus (reg);
    return halLCD_readStatus();
}


#endif // LCD_SPI_MODE


// Sets the display in graphics mode (as opposed to text mode)
void halLCD_graphicsMode (void) {
    uint8_t tmp8;
    halLCD_writeCommand (RA8875_MWCR0);
    tmp8 = halLCD_readData ();
    tmp8 &= ~RA8875_MWCR0_TXTMODE; // bit #7
    halLCD_writeData (tmp8);
}


void halLCD_setColor (color_t color) {
    halLCD_lcdStruct.color = color;
#ifdef COLOR_16BIT
    halLCD_writeReg (RA8875_FGCR0, halLCD_lcdStruct.color >> 11);
    halLCD_writeReg (RA8875_FGCR1, halLCD_lcdStruct.color >> 5);
    halLCD_writeReg (RA8875_FGCR2, halLCD_lcdStruct.color);
#else
    halLCD_writeReg (RA8875_FGCR0, halLCD_lcdStruct.color >> 5);
    halLCD_writeReg (RA8875_FGCR1, halLCD_lcdStruct.color >> 2);
    halLCD_writeReg (RA8875_FGCR2, halLCD_lcdStruct.color);
#endif    
}


void halLCD_softReset (void) { // Performs a SW-based reset of the RA8875
    halLCD_writeCommand (RA8875_PWRR);
    halLCD_writeData (RA8875_PWRR_SOFTRESET);
    halLCD_writeData (RA8875_PWRR_NORMAL);
    halLCD_delay (1);
}



void halLCD_activeWindow (uint16_t xs, uint16_t xe, uint16_t ys, uint16_t ye) {
    /* Set active window X */
    halLCD_writeReg (RA8875_HSAW0, (uint16_t)(xs - 1) & 0xFF); // horizontal start point
    halLCD_writeReg (RA8875_HSAW1, (uint16_t)(xs - 1) >> 8);
    halLCD_writeReg (RA8875_HEAW0, (uint16_t)(xe - 1) & 0xFF); // horizontal end point
    halLCD_writeReg (RA8875_HEAW1, (uint16_t)(xe - 1) >> 8);
    
    /* Set active window Y */
    halLCD_writeReg (RA8875_VSAW0, (uint16_t)(ys - 1) & 0xFF); // vertical start point
    halLCD_writeReg (RA8875_VSAW1, (uint16_t)(ys - 1) >> 8);  
    halLCD_writeReg (RA8875_VEAW0, (uint16_t)(ye - 1) & 0xFF); // horizontal end point
    halLCD_writeReg (RA8875_VEAW1, (uint16_t)(ye - 1) >> 8);
}


void halLCD_PLLinit (void) { // Initialise the PLL
    halLCD_writeReg (RA8875_PLLC1, RA8875_PLLC1_PLLDIV1 + 10); // 10
    halLCD_delay (100);
    halLCD_writeReg (RA8875_PLLC2, RA8875_PLLC2_DIV2); // RA8875_PLLC2_DIV4
    halLCD_delay (100);
    /* 
    halLCD_writeReg (RA8875_PLLC1, RA8875_PLLC1_PLLDIV1 + 30); // 10
    halLCD_delay (1);
    halLCD_writeReg (RA8875_PLLC2, RA8875_PLLC2_DIV1); // RA8875_PLLC2_DIV4
    halLCD_delay (1);
    */
}


void   halLCD_init (uint8_t mode) {
    /* Timing values */
    //uint8_t pixclk;
    uint8_t hsync_start;
    uint8_t hsync_pw;
    uint8_t hsync_finetune;
    uint8_t hsync_nondisp;
    uint8_t vsync_pw; 
    uint16_t vsync_nondisp;
    uint16_t vsync_start;
    color_t color;
    uint16_t i;

#ifdef LCD_SPI_MODE
    halLCD_SPI_init (SPI_BaudRatePrescaler_16); //16
    halLCD_SPI_setCLK (SPI_CLOCK_MIN);
#else
    GPIO_LCD_INIT; // init GPIO
    
    LCD_PORT_MODE_OUT; // outs
    // настройка по-умолчанию
    LCD_WR_H;
    LCD_RD_H;
    LCD_RS_H;

#endif // LCD_SPI_MODE
    LCD_CS_H;
//    LCD_CLK_H;
    
    LCD_RESET_L; // if present, but not allocated
    halLCD_delay (10); // @todo reset pulse check!
    LCD_RESET_H;
    halLCD_delay (10);
    
    halLCD_softReset ();
    while (1) {
        if (halLCD_readReg(0) == 0x75) { // читаем статусный регистр
            LED_RED_ON;
            break;
        }
    };
    
    halLCD_PLLinit(); // настройка делителей и PLL
#ifdef COLOR_16BIT
    halLCD_writeReg (RA8875_SYSR, RA8875_SYSR_16BPP | RA8875_SYSR_MCU16); //RA8875_SYSR_MCU8); ?
#else
    halLCD_writeReg (RA8875_SYSR, RA8875_SYSR_8BPP | RA8875_SYSR_MCU8);
#endif
    halLCD_delay (100); // >100uS
    

    hsync_nondisp   = 26;
    hsync_start     = 32;
    hsync_pw        = 96;
    hsync_finetune  = 0;
    vsync_nondisp   = 32;
    vsync_start     = 23;
    vsync_pw        = 2;
    
    // PDAT is fetched at PCLK falling edge. 
    // PCLK period = 2 times of System Clock period. 
    halLCD_writeReg (RA8875_PCSR, RA8875_PCSR_PDATL | RA8875_PCSR_2CLK); // Pixel Clock Setting Register 
    halLCD_delay (100); // >100uS
    
    /* Horizontal settings registers */
    halLCD_writeReg (RA8875_HDWR, (halLCD_getWidth() / 8) - 1);                 // H width: (HDWR + 1) * 8 = 480
    halLCD_writeReg (RA8875_HNDFTR, RA8875_HNDFTR_DE_HIGH + hsync_finetune);
    halLCD_writeReg (RA8875_HNDR, (hsync_nondisp - hsync_finetune - 2) / 8);    // H non-display: HNDR * 8 + HNDFTR + 2 = 10
    halLCD_writeReg (RA8875_HSTR, hsync_start / 8 - 1);                         // Hsync start: (HSTR + 1)*8 
    halLCD_writeReg (RA8875_HPWR, RA8875_HPWR_LOW + (hsync_pw / 8 - 1));        // HSync pulse width = (HPWR+1) * 8
    
    /* Vertical settings registers */
    halLCD_writeReg (RA8875_VDHR0, (uint16_t)(halLCD_getHeight() - 1) & 0xFF);
    halLCD_writeReg (RA8875_VDHR1, (uint16_t)(halLCD_getHeight() - 1) >> 8);
    halLCD_writeReg (RA8875_VNDR0, vsync_nondisp - 1);                          // V non-display period = VNDR + 1
    halLCD_writeReg (RA8875_VNDR1, vsync_nondisp >> 8);
    halLCD_writeReg (RA8875_VSTR0, vsync_start - 1);                            // Vsync start position = VSTR + 1
    halLCD_writeReg (RA8875_VSTR1, vsync_start >> 8);
    halLCD_writeReg (RA8875_VPWR, RA8875_VPWR_LOW + vsync_pw - 1);              // Vsync pulse width = VPWR + 1
    
    halLCD_activeWindow (0, halLCD_getWidth(), 0, halLCD_getHeight());
    halLCD_graphicsMode (); // включаем графический режим
    halLCD_writeReg (RA8875_PWRR, RA8875_PWRR_DISPON | RA8875_PWRR_NORMAL); //display on 

//         #define RA8875_BTCR 0x44
//     halLCD_writeReg (RA8875_MWCR0,  0xE0);
//     halLCD_writeReg (RA8875_MWCR1,  0x84);
//     halLCD_writeReg (RA8875_BTCR, 6);
//     halLCD_writeReg (RA8875_LTPR0, 0x03);
//     halLCD_writeReg (RA8875_LTPR1, 0x44);
    halLCD_setOrientation (LCD_ORIENTATION_NORMAL); 
    halLCD_setColor (COLOR_WHITE); // чистим экран от мусора
    halLCD_cls (COLOR_WHITE);
    
    halLCD_writeReg (RA8875_DPCR, 0x80); // два слоя, между ними переключаемся сами вручную
    for (i = 0; i < 256; i++) { // плавное зажигание подсветки
        halLCD_writeReg (RA8875_P1CR, 0); //PWM clear
        halLCD_writeReg (RA8875_P1CR, RA8875_P1CR_ENABLE); // | RA8875_P1CR_CLKOUT); PWM setting - open PWM
        halLCD_writeReg (RA8875_P1DCR, i); //Backlight brightness setting - parameter 0xff-0x00
        halLCD_delay (100);
    }

    
#ifdef LCD_SPI_MODE
    halLCD_SPI_init (SPI_BaudRatePrescaler_8); //8
    halLCD_SPI_setCLK (SPI_CLOCK_MAX);
#endif
}
    

void halLCD_setBackLight (uint8_t value) {
    halLCD_writeReg (RA8875_P1CR, 0); //PWM clear
    halLCD_writeReg (RA8875_P1CR, RA8875_P1CR_ENABLE); // | RA8875_P1CR_CLKOUT); PWM setting - open PWM
    halLCD_writeReg (RA8875_P1DCR, value); //Backlight brightness setting - parameter 0xff-0x00
}


void     halLCD_cls (color_t color) {
#ifdef COLOR_16BIT
    halLCD_writeReg (RA8875_BGCR0, color >> 11);
    halLCD_writeReg (RA8875_BGCR1, color >> 5);
    halLCD_writeReg (RA8875_BGCR2, color);
#else
    halLCD_writeReg (RA8875_BGCR0, color >> 5);
    halLCD_writeReg (RA8875_BGCR1, color >> 2);
    halLCD_writeReg (RA8875_BGCR2, color);
#endif
    if (0 != scr_number) { // layer 0
        //halLCD_writeReg (RA8875_VSBE1, 0x00);
        halLCD_writeReg (RA8875_MWCR1, 0x00);
    } else { // layer 1
        //halLCD_writeReg (RA8875_VSBE1, 0x80);
        halLCD_writeReg (RA8875_MWCR1, 0x01);
    }
    /* Clear the entire window */
    halLCD_writeReg (RA8875_MCLR, RA8875_MCLR_START | RA8875_MCLR_ACTIVE);
    while (halLCD_readReg (RA8875_MCLR) & RA8875_MCLR_START) {};   // wait completion.
}


void halLCD_setXY (coord_t x, coord_t y) { // Sets the current X/Y position on the display before drawing
    halLCD_writeReg (RA8875_CURH0, x);
    halLCD_writeReg (RA8875_CURH1, x >> 8);
    halLCD_writeReg (RA8875_CURV0, y);
    halLCD_writeReg (RA8875_CURV1, y >> 8);  
}


void halLCD_setPixel (coord_t x , coord_t y) { // @todo переделать под мультипоток или вывести 
    halLCD_writeReg (RA8875_CURH0, x);
    halLCD_writeReg (RA8875_CURH1, x >> 8);
    halLCD_writeReg (RA8875_CURV0, y);
    halLCD_writeReg (RA8875_CURV1, y >> 8);  
    halLCD_writeCommand (RA8875_MRWC);
    LCD_CS_L;
#ifdef LCD_SPI_MODE
    halLCD_xspi (RA8875_DATAWRITE);
#ifdef COLOR_16BIT
    halLCD_xspi (halLCD_lcdStruct.color >> 8);
#endif
    halLCD_xspi (halLCD_lcdStruct.color);
#else
    halLCD_writeCommand (RA8875_DATAWRITE);
#ifdef COLOR_16BIT
    halLCD_writeData (halLCD_lcdStruct.color >> 8);
#endif
    halLCD_writeData (halLCD_lcdStruct.color);
#endif // LCD_SPI_MODE  
    LCD_CS_H;
    
    //halLCD_fillBlockColor (x,y, 1, 1, halLCD_lcdStruct.color);
} 


void halLCD_setPixelColor (coord_t x , coord_t y, color_t color) { // @TODO переделать под мультипоток или вывести 
//     halLCD_writeReg (RA8875_CURH0, x);
//     halLCD_writeReg (RA8875_CURH1, x >> 8);
//     halLCD_writeReg (RA8875_CURV0, y);
//     halLCD_writeReg (RA8875_CURV1, y >> 8);  
//     halLCD_writeCommand (RA8875_MRWC);
// #ifdef LCD_SPI_MODE
//     LCD_CS_L;
//     halLCD_xspi (RA8875_DATAWRITE);
// #ifdef COLOR_16BIT
//     halLCD_xspi (color >> 8);
// #endif
//     halLCD_xspi (color);
//     LCD_CS_H;
// #else
//     LCD_CS_L;
//     halLCD_writeCommand (RA8875_DATAWRITE);
// #ifdef COLOR_16BIT
//     halLCD_writeData (color >> 8);
// #endif
//     halLCD_writeData (color);
//     LCD_CS_H;
// #endif // LCD_SPI_MODE   

    halLCD_fillBlockColor (x,y, 1, 1, color);
} 





/**************************************************************************/
/*!
      HW accelerated function to push a chunk of raw pixel data
      
      @args num[in] The number of pixels to push
      @args p[in]   The pixel color to use
*/
/**************************************************************************/
void halLCD_pushPixels (uint32_t num, uint16_t p) {
#ifdef LCD_SPI_MODE
    LCD_CS_L;
    halLCD_xspi (RA8875_DATAWRITE);
    while (num--) {
#ifdef COLOR_16BIT
        halLCD_xspi (p >> 8);
#endif
        halLCD_xspi (p);
    }
    LCD_CS_H;
#else
    LCD_CS_L;
    halLCD_writeCommand(RA8875_DATAWRITE);
    while (num--) {
#ifdef COLOR_16BIT
        halLCD_writeData(p >> 8);
#endif
        halLCD_writeData(p);
    }
    LCD_CS_H;
#endif // LCD_SPI_MODE
}




void halLCD_setBTE_coord_source (coord_t x, coord_t y) {
    halLCD_writeReg (RA8875_HSBE0, x);
    halLCD_writeReg (RA8875_HSBE1, x >> 8);
    halLCD_writeReg (RA8875_VSBE0, y);
    halLCD_writeReg (RA8875_VSBE1, y >> 8);
}


void halLCD_setBTE_coord_destination (coord_t x, coord_t y) {
    halLCD_writeReg (RA8875_HDBE0, x);
    halLCD_writeReg (RA8875_HDBE1, x >> 8);
    halLCD_writeReg (RA8875_VDBE0, y);
    y = y >> 8;
    if (0 != scr_number) { // layer 1
        //y &= ~RA8875_BTE_LAYER;
        halLCD_writeReg (RA8875_VDBE1, y);
    } else { // layer 1
        y |= RA8875_BTE_LAYER;
        halLCD_writeReg (RA8875_VDBE1, y);
    }
}


void halLCD_setBTE_size (uint16_t width, uint16_t height) { // BTE area size settings  
    halLCD_writeReg (RA8875_BEWR0, width); //BET area width literacy
    halLCD_writeReg (RA8875_BEWR1, width >> 8); //BET area width literacy	    
    halLCD_writeReg (RA8875_BEHR0, height); //BET area height literacy
    halLCD_writeReg (RA8875_BEHR1, height >> 8); //BET area height literacy	   
}




void     halLCD_fff (coord_t x, coord_t y) {
    uint32_t n = 1100;
    uint32_t i, j;

#ifdef CHECK_BORDER
    if (x >= LCD_CTRL_WIDTH  ||
        x >= LCD_CTRL_WIDTH) {
        // error
    } else {
#endif

        

        

        
        halLCD_writeReg (RA8875_BECR1, 0xC0); // 0x0C 0x78
        halLCD_writeReg (RA8875_BECR0, RA8875_BECR0_BTE);  
        while (halLCD_readStatus() & 0x80) {}; // wait completion.    
        halLCD_writeCommand (RA8875_MRWC); //
        while (n--) {
            LCD_CS_L;
            halLCD_xspi (RA8875_DATAWRITE);
#ifdef COLOR_16BIT
            halLCD_xspi (5555 >> 8);
#else
            halLCD_xspi (55);
#endif
            LCD_CS_H;

            //while (halLCD_readStatus() & 0x80) {}; // wait completion.
        }
        while (halLCD_readStatus() & 0x40) {};   // wait completion.
#ifdef CHECK_BORDER
    }
#endif
}



void     halLCD_fillBlock (coord_t x, coord_t y, uint16_t w, uint16_t h, color_t *buf) {
    uint32_t n;
    uint32_t i, j;
    uint8_t tmp8;

#ifdef CHECK_BORDER
    if (x >= LCD_CTRL_WIDTH  ||
        x >= LCD_CTRL_WIDTH  ||
        w == 0 ||
        h == 0) {
        // error
    } else {
#endif
        n = (w * h);
        
        halLCD_setBTE_coord_destination (x, y);
        halLCD_setBTE_size (w, h);
        
        halLCD_writeReg (RA8875_BECR1, 0xC0); // 0x0C 0x78
        halLCD_writeReg (RA8875_BECR0, RA8875_BECR0_BTE);  
        while (halLCD_readStatus() & 0x80) {}; // wait completion.    
        halLCD_writeCommand (RA8875_MRWC); //
        while (n--) {
            LCD_CS_L;
            halLCD_xspi (RA8875_DATAWRITE);
#ifdef COLOR_16BIT
            halLCD_xspi (*buf >> 8);
#else
            halLCD_xspi (*buf);
#endif
            LCD_CS_H;
            buf++;
            //while (halLCD_readStatus() & 0x80) {}; // wait completion.
        }
        while (halLCD_readStatus() & 0x40) {};   // wait completion.
#ifdef CHECK_BORDER
    }
#endif
}


void     halLCD_fillBlockColor (coord_t x, coord_t y, uint16_t w, uint16_t h, color_t color) {
    uint32_t n;
    uint32_t i, j;
    uint8_t tmp8;
//     
//     halLCD_activeWindow (x, y, w, h);
//     /* Clear the entire window */
//     halLCD_writeReg (RA8875_MCLR, RA8875_MCLR_START | RA8875_MCLR_ACTIVE);
//     while (halLCD_readReg (RA8875_MCLR) & RA8875_MCLR_START) {};   // wait completion.
//     halLCD_activeWindow (0, 0, paint_getWidth () - 1, paint_getHeight () - 1);
#ifdef CHECK_BORDER
    if (x >= LCD_CTRL_WIDTH  ||
        x >= LCD_CTRL_WIDTH  ||
        w == 0 ||
        h == 0) {
        // error
    } else {
#endif
        n = (w * h);
        
        halLCD_setBTE_coord_destination (x, y);
        halLCD_setBTE_size (w, h);
        
        halLCD_writeReg (RA8875_BECR1, 0xC0); // 0x0C 0x78
        halLCD_writeReg (RA8875_BECR0, RA8875_BECR0_BTE);  
        while (halLCD_readStatus() & 0x80) {}; // wait completion.    
        halLCD_writeCommand (RA8875_MRWC); //
        while (n--) {
            LCD_CS_L;
            halLCD_xspi (RA8875_DATAWRITE);
#ifdef COLOR_16BIT
            halLCD_xspi (color >> 8);
#else
            halLCD_xspi (color);
#endif
            LCD_CS_H;
            //while (halLCD_readStatus() & 0x80) {}; // wait completion.
        }
        while (halLCD_readStatus() & 0x40) {};   // wait completion.
#ifdef CHECK_BORDER
    }
#endif
}


void halLCD_setPixelBig (coord_t x , coord_t y, color_t color) {
    halLCD_fillBlockColor(x - 1, y - 1, x + 1, y + 1, color);
}


void     halLCD_setOrientation (uint8_t orient) {
    halLCD_lcdStruct.orientation = orient;
};


color_t halLCD_getPixel (coord_t x, coord_t y) { // Доделать, вполне возможно!
    color_t color = 0;
    
    #ifdef CHECK_BORDER
    if (x >= LCD_CTRL_WIDTH  ||
        x >= LCD_CTRL_WIDTH) {
        // error
    } else {
#endif
        halLCD_setBTE_coord_destination (x, y);
        halLCD_setBTE_size (1, 1);
        halLCD_writeReg (RA8875_BECR1, 0x01); // 0x0C 0x78
        halLCD_writeReg (RA8875_BECR0, RA8875_BECR0_BTE);  
        while (halLCD_readStatus() & 0x80) {}; // wait completion.    
        halLCD_writeCommand (RA8875_MRWC); //
        LCD_CS_L;
        halLCD_xspi (RA8875_DATAWRITE);
#ifdef COLOR_16BIT
        color = halLCD_xspi (0xFF);
        color = halLCD_xspi (0xFF);
#else
        color = halLCD_xspi (0xFF);
#endif
        LCD_CS_H;
        while (halLCD_readStatus() & 0x40) {};   // wait completion.
#ifdef CHECK_BORDER
    }
#endif
    
//     halLCD_writeReg (RA8875_CURH0, x);
//     halLCD_writeReg (RA8875_CURH1, x >> 8);
//     halLCD_writeReg (RA8875_CURV0, y);
//     halLCD_writeReg (RA8875_CURV1, y >> 8);  
//     halLCD_writeCommand (RA8875_MRWC);
// #ifdef LCD_SPI_MODE
//     LCD_CS_L;
//     halLCD_xspi (RA8875_DATAREAD);
// #ifdef COLOR_16BIT
//     color = (uint16_t)(halLCD_xspi (0xFF) << 8);
// #endif
//     color |= halLCD_xspi (0xFF);
//     LCD_CS_H;
// #else
// //     LCD_CS_L;
// //     halLCD_writeCommand (RA8875_DATAWRITE);
// // #ifdef COLOR_16BIT
// //     halLCD_writeData (color >> 8);
// // #endif
// //     halLCD_writeData (color);
// //     LCD_CS_H;
// #endif // LCD_SPI_MODE  
    return color; //respond;
}


// void paint_lineX (coord_t x0, coord_t y0, uint16_t w) {
// #ifdef CHECK_BORDER
//     if (x0 >= LCD_CTRL_WIDTH  ||
//         w >= LCD_CTRL_WIDTH  ||
//         y0 >= LCD_CTRL_HEIGHT) {
//         // error
//     } else {
// #endif
//     halLCD_fillBlockColor (x0, y0, w, 1, halLCD_lcdStruct.color);
// #ifdef CHECK_BORDER
//     }
// #endif
// }


void _paint_line (coord_t x1, coord_t y1, coord_t x2, coord_t y2) {
#ifdef CHECK_BORDER
    if (x1 >= LCD_CTRL_WIDTH  ||
        x2 >= LCD_CTRL_WIDTH  ||
        y1 >= LCD_CTRL_HEIGHT ||
        y2 >= LCD_CTRL_HEIGHT) {
        // error
    } else {
#endif
        halLCD_writeReg (RA8875_DLHSR0, x1);
        halLCD_writeReg (RA8875_DLHSR1, x1 >> 8);
        halLCD_writeReg (RA8875_DLVSR0, y1);
        halLCD_writeReg (RA8875_DLVSR1, y1 >> 8);
        halLCD_writeReg (RA8875_DLHER0, x2);
        halLCD_writeReg (RA8875_DLHER1, x2 >> 8);
        halLCD_writeReg (RA8875_DLVER0, y2);
        halLCD_writeReg (RA8875_DLVER1, y2 >> 8);
        
        
#ifdef COLOR_16BIT
        halLCD_writeReg (RA8875_FGCR0, halLCD_lcdStruct.color >> 11);
        halLCD_writeReg (RA8875_FGCR1, halLCD_lcdStruct.color >> 5);
        halLCD_writeReg (RA8875_FGCR2, halLCD_lcdStruct.color);
#else
        halLCD_writeReg (RA8875_FGCR0, 55 >> 5);
        halLCD_writeReg (RA8875_FGCR1, 55 >> 2);
        halLCD_writeReg (RA8875_FGCR2, 55);
#endif  

        halLCD_writeReg (RA8875_DCR, RA8875_DCR_DRAWLINE);
        halLCD_writeReg (RA8875_DCR, RA8875_DCR_LINESQUTRI_START | RA8875_DCR_DRAWLINE); // Start drawing.
        //_delay_ms(10);//
        //while (halLCD_readReg (RA8875_DCR) & 0x80) {};   // await completion.
        //while (halLCD_readStatus() & 0x80) {}; // wait completion.

#ifdef CHECK_BORDER
    }
#endif
}



void halLCD_layer_copy (uint8_t layerA, uint8_t layerB) {
    
/* копируем с нижнего слоя на верхний
    
    */

    //halLCD_setBTE_coord_destination (0, 0);

    
    halLCD_writeReg (RA8875_HSBE0, 0);
    halLCD_writeReg (RA8875_HSBE1, 0);
    halLCD_writeReg (RA8875_VSBE0, 0);
    if (0 != layerA) { // layer 1
        //y &= ~RA8875_BTE_LAYER;
        halLCD_writeReg (RA8875_VSBE1, 0);
    } else { // layer 1
        //y |= RA8875_BTE_LAYER;
        halLCD_writeReg (RA8875_VSBE1, 0 | RA8875_BTE_LAYER);
    }
    
    halLCD_writeReg (RA8875_HDBE0, 0);
    halLCD_writeReg (RA8875_HDBE1, 0);
    halLCD_writeReg (RA8875_VDBE0, 0);
    if (0 != layerB) { // layer 1
        //y &= ~RA8875_BTE_LAYER;
        halLCD_writeReg (RA8875_VDBE1, 0);
    } else { // layer 1
        //y |= RA8875_BTE_LAYER;
        halLCD_writeReg (RA8875_VDBE1, 0 | RA8875_BTE_LAYER);
    }
    
    halLCD_setBTE_size (LCD_CTRL_WIDTH - 0, LCD_CTRL_HEIGHT -0);
    
    halLCD_writeReg (RA8875_BECR1, 0xC2); // 0x02
    halLCD_writeReg (RA8875_BECR0, RA8875_BECR0_BTE);  
    while (halLCD_readStatus() & 0x80) {}; // wait completion.    
    halLCD_writeCommand (RA8875_MRWC); //

    while (halLCD_readStatus() & 0x40) {};   // wait completion.
            
}


void _paint_rect (coord_t x1, coord_t y1, uint16_t xw, uint16_t yh) {
    uint16_t x2, y2;
    static uint8_t tmp;
    color_t color = COLOR_RED;
    static uint32_t iii = 0;

    x2 = x1 + (xw - 1);
    y2 = y1 + (yh - 1);
    //halLCD_setLayer (0);

// #define RA8875_DLHSR0 0x91 // Draw Line/Square Horizontal Start Address Register0
// #define RA8875_DLHSR1 0x92 // Draw Line/Square Horizontal Start Address Register1
// #define RA8875_DLVSR0 0x93 // Draw Line/Square Vertical Start Address Register0
// #define RA8875_DLVSR1 0x94 // Draw Line/Square Vertical Start Address Register1
// #define RA8875_DLHER0 0x95 // Draw Line/Square Horizontal End Address Register0
// #define RA8875_DLHER1 0x96 // Draw Line/Square Horizontal End Address Register1
// #define RA8875_DLVER0 0x97 // Draw Line/Square Vertical End Address Register0
// #define RA8875_DLVER1 0x98 // Draw Line/Square Vertical End Address Register1
    /**
    halLCD_writeReg (RA8875_DLHSR0, x1 & 0xFF);
    tmp = halLCD_readReg (RA8875_DLHSR0);
    halLCD_writeReg (RA8875_DLHSR1, x1 >> 8);
    tmp = halLCD_readReg (RA8875_DLHSR1);
    halLCD_writeReg (RA8875_DLVSR0, y1 & 0xFF);
    tmp = halLCD_readReg (RA8875_DLVSR0);
    halLCD_writeReg (RA8875_DLVSR1, y1 >> 8);
    tmp = halLCD_readReg (RA8875_DLVSR1);
    halLCD_writeReg (RA8875_DLHER0, x2 & 0xFF);
    tmp = halLCD_readReg (RA8875_DLHER0);
    halLCD_writeReg (RA8875_DLHER1, x2 >> 8);
    tmp = halLCD_readReg (RA8875_DLHER1);
    halLCD_writeReg (RA8875_DLVER0, y2 & 0xFF);
    tmp = halLCD_readReg (RA8875_DLVER0);
    halLCD_writeReg (RA8875_DLVER1, y2 >> 8);
    tmp = halLCD_readReg (RA8875_DLVER1);
 
  




    halLCD_writeReg (RA8875_DLHSR0, x1 & 0xFF);
    tmp = halLCD_readReg (RA8875_DLHSR0);
    halLCD_writeReg (RA8875_DLHSR1, x1 >> 8);
    tmp = halLCD_readReg (RA8875_DLHSR1);
    halLCD_writeReg (RA8875_DLVSR0, 0);
    tmp = halLCD_readReg (RA8875_DLVSR0);
    halLCD_writeReg (RA8875_DLVSR1, 0);
    tmp = halLCD_readReg (RA8875_DLVSR1);
    halLCD_writeReg (RA8875_DLHER0, 15);
    tmp = halLCD_readReg (RA8875_DLHER0);
    halLCD_writeReg (RA8875_DLHER1, 0);
    tmp = halLCD_readReg (RA8875_DLHER1);
    halLCD_writeReg (RA8875_DLVER0, 16);
    tmp = halLCD_readReg (RA8875_DLVER0);
    halLCD_writeReg (RA8875_DLVER1, 0);
    tmp = halLCD_readReg (RA8875_DLVER1); */
    
    halLCD_writeReg (RA8875_DLHSR0, x1 & 0xFF);
    tmp = halLCD_readReg (RA8875_DLHSR0);
    halLCD_writeReg (RA8875_DLHSR1, x1 >> 8);
    tmp = halLCD_readReg (RA8875_DLHSR1);
    halLCD_writeReg (RA8875_DLVSR0, y1 & 0xFF);
    tmp = halLCD_readReg (RA8875_DLVSR0);
    halLCD_writeReg (RA8875_DLVSR1, y1 >> 8);
    tmp = halLCD_readReg (RA8875_DLVSR1);
    halLCD_writeReg (RA8875_DLHER0, x2 & 0xFF);
    tmp = halLCD_readReg (RA8875_DLHER0);
    halLCD_writeReg (RA8875_DLHER1, x2 >> 8);
    tmp = halLCD_readReg (RA8875_DLHER1);
    halLCD_writeReg (RA8875_DLVER0, y2 & 0xFF);
    tmp = halLCD_readReg (RA8875_DLVER0);
    halLCD_writeReg (RA8875_DLVER1, y2 >> 8);
    tmp = halLCD_readReg (RA8875_DLVER1);
    
    
iii = 0;

#ifdef COLOR_16BIT
        halLCD_writeReg (RA8875_FGCR0, halLCD_lcdStruct.color >> 11);
        tmp = halLCD_readReg (RA8875_FGCR0);
        halLCD_writeReg (RA8875_FGCR1, halLCD_lcdStruct.color >> 5);
        tmp = halLCD_readReg (RA8875_FGCR1);
        halLCD_writeReg (RA8875_FGCR2, halLCD_lcdStruct.color);
        tmp = halLCD_readReg (RA8875_FGCR2);
#else
/**
        halLCD_writeReg (RA8875_FGCR0, 0xFF >> 5);
        tmp = halLCD_readReg (RA8875_FGCR0);
        halLCD_writeReg (RA8875_FGCR1, 0xFF >> 2);
        tmp = halLCD_readReg (RA8875_FGCR1);
        halLCD_writeReg (RA8875_FGCR2, 0xFF);
        tmp = halLCD_readReg (RA8875_FGCR2);
        */
        halLCD_writeReg (RA8875_FGCR0, halLCD_lcdStruct.color >> 5);
        tmp = halLCD_readReg (RA8875_FGCR0);
        halLCD_writeReg (RA8875_FGCR1, halLCD_lcdStruct.color >> 2);
        tmp = halLCD_readReg (RA8875_FGCR1);
        halLCD_writeReg (RA8875_FGCR2, halLCD_lcdStruct.color);
        tmp = halLCD_readReg (RA8875_FGCR2);
        
// #define RA8875_GCC0  0x84
// #define RA8875_GCC1  0x85
//     halLCD_writeReg (RA8875_GCC0, 0x55);
//     halLCD_writeReg (RA8875_GCC1, 0x55);
//         
#endif


   /// halLCD_pushPixels (33, 66);
    tmp = halLCD_readReg (RA8875_DCR);
    halLCD_writeReg (RA8875_DCR, RA8875_DCR_DRAWSQUARE | RA8875_DCR_NOFILL);
    tmp = halLCD_readReg (RA8875_DCR);
    halLCD_writeReg (RA8875_DCR, 0x80 | RA8875_DCR_DRAWSQUARE | RA8875_DCR_NOFILL); // Start drawing.
//     halLCD_writeReg (RA8875_DCR, 0x80 | RA8875_DCR_DRAWSQUARE | RA8875_DCR_NOFILL);
//     halLCD_writeReg (RA8875_DCR, 0x80 | RA8875_DCR_DRAWSQUARE | RA8875_DCR_NOFILL);
//     halLCD_writeReg (RA8875_DCR, 0xFF);
    
    //_delay_ms (100);
    
    //tmp = halLCD_readReg (RA8875_DCR);
    
    while (halLCD_readReg (RA8875_DCR) & 0x80) {iii++;};   // await completion.
    
    _delay_ms(1);
    //halLCD_setLayer (1);

}


// void paint_rectFill (coord_t x1, coord_t y1, uint16_t xw, uint16_t yh) {
//     uint16_t x2, y2;
//     volatile uint32_t iii = 0;

//     x2 = x1 + (xw - 1);
//     y2 = y1 + (yh - 1);
// #ifdef CHECK_BORDER
//     if (x1 >= LCD_CTRL_WIDTH  ||
//         xw >= LCD_CTRL_WIDTH  ||
//         y1 >= LCD_CTRL_HEIGHT || // @todo сделать проверку нормальной!
//         yh >= LCD_CTRL_HEIGHT) {
//         // error
//     } else {
// #endif

//         //halLCD_fillBlockColor (x1, y1, xw, yh, sPaint.color);
//         halLCD_writeReg (RA8875_DLHSR0, x1);
//         halLCD_writeReg (RA8875_DLHSR1, x1 >> 8);
//         halLCD_writeReg (RA8875_DLVSR0, y1);
//         halLCD_writeReg (RA8875_DLVSR1, y1 >> 8);
//         halLCD_writeReg (RA8875_DLHER0, x2);
//         halLCD_writeReg (RA8875_DLHER1, x2 >> 8);
//         halLCD_writeReg (RA8875_DLVER0, y2);
//         halLCD_writeReg (RA8875_DLVER1, y2 >> 8);
//         
// #ifdef COLOR_16BIT
//         halLCD_writeReg (RA8875_FGCR0, halLCD_lcdStruct.color >> 11);
//         halLCD_writeReg (RA8875_FGCR1, halLCD_lcdStruct.color >> 5);
//         halLCD_writeReg (RA8875_FGCR2, halLCD_lcdStruct.color);
// #else
//         halLCD_writeReg (RA8875_FGCR0, 0xFF >> 5);
//         halLCD_writeReg (RA8875_FGCR1, 0x00 >> 2);
//         halLCD_writeReg (RA8875_FGCR2, 0x00);
//         
// #define RA8875_GCC0  0x84
// #define RA8875_GCC1  0x85
//     halLCD_writeReg (RA8875_GCC0, 0x55);
//     halLCD_writeReg (RA8875_GCC1, 0x55);
//         
// #endif
//         //halLCD_writeReg (RA8875_DCR, 0);
//         //halLCD_writeReg (RA8875_DCR, RA8875_DCR_DRAWSQUARE);
//         //halLCD_writeReg (RA8875_DCR, RA8875_DCR_DRAWSQUARE | RA8875_DCR_FILL);
//         halLCD_writeReg (RA8875_DCR, RA8875_DCR_LINESQUTRI_START | RA8875_DCR_DRAWSQUARE | RA8875_DCR_FILL); // Start drawing.
//         while (halLCD_readReg (RA8875_DCR) & 0x80){iii++;};   // await completion.

//         x1 = iii;
//         _delay_ms(10);
// #ifdef CHECK_BORDER
//     }
// #endif
// }


        
        



// физически переключает слой, отображая его
void halLCD_paintLayer (uint8_t num) {
    if (0 != num) { //LTPR1
        halLCD_writeReg (RA8875_LTPR0, RA8875_LTPR0_MODE_L1);
    } else {
        halLCD_writeReg (RA8875_LTPR0, RA8875_LTPR0_MODE_L2);
    }
}

// присваивает номер слоя
void halLCD_setLayer (uint8_t num) {
    scr_number = num;
}

void halLCD_repaint (void) { // переключает следующий слой
    if (0 != scr_number) { //LTPR1
        scr_number = 0;
        halLCD_writeReg (RA8875_LTPR0, RA8875_LTPR0_MODE_L1);
    } else {
        scr_number = 1;
        halLCD_writeReg (RA8875_LTPR0, RA8875_LTPR0_MODE_L2);
    }
}


// void paint_circ (coord_t x, coord_t y, uint16_t radius, uint8_t corner_type) {
//     uint8_t tmp;
// #ifdef CHECK_BORDER
//     if (x >= LCD_CTRL_WIDTH  ||
//         y >= LCD_CTRL_HEIGHT ||
//         radius >= LCD_CTRL_WIDTH) {
//         // error
//     } else {
// #endif
//         halLCD_writeReg (0x99, x & 0xFF);
//         tmp = halLCD_readReg (0x99);
//         halLCD_writeReg (0x9a, x >> 8);
//         tmp = halLCD_readReg (0x9a);
//         halLCD_writeReg (0x9b, y & 0xFF);
//         tmp = halLCD_readReg (0x9b);
//         halLCD_writeReg (0x9c, y >> 8);
//         tmp = halLCD_readReg (0x9c);
//         halLCD_writeReg (0x9d, radius & 0xFF);
//         tmp = halLCD_readReg (0x9d);
//             
//     #ifdef COLOR_16BIT
//         halLCD_writeReg (RA8875_FGCR0, halLCD_lcdStruct.color >> 11);
//         halLCD_writeReg (RA8875_FGCR1, halLCD_lcdStruct.color >> 5);
//         halLCD_writeReg (RA8875_FGCR2, halLCD_lcdStruct.color);
//     #else
//         halLCD_writeReg (RA8875_FGCR0, 0x03 >> 5);
//         halLCD_writeReg (RA8875_FGCR1, 0x03 >> 2);
//         halLCD_writeReg (RA8875_FGCR2, 0x03);
//     #endif

//     //     unsigned char drawCmd = 0x00;       // Circle
//     //     if (fillit == FILL)
//     //         drawCmd |= 0x20;
//         halLCD_writeReg (0x90, 0); //stop
//             tmp = halLCD_readReg (0x90);
//         halLCD_writeReg (0x90, 0x40 + 0); // Start drawing.
//             tmp = halLCD_readReg (0x90);
//         while (halLCD_readReg (0x90) & 0x40){};  // await completion.
//          //   wait_us(POLLWAITuSec);
// #ifdef CHECK_BORDER
//     }
// #endif
// }


uint16_t halLCD_getWidth(void) {
	switch(halLCD_lcdStruct.orientation) {
		case LCD_ORIENTATION_NORMAL:
		case LCD_ORIENTATION_180:
			return LCD_CTRL_WIDTH;
	}
	return LCD_CTRL_HEIGHT;
}


uint16_t halLCD_getHeight(void) {
	switch(halLCD_lcdStruct.orientation) {
		case LCD_ORIENTATION_NORMAL:
		case LCD_ORIENTATION_180:
		return LCD_CTRL_HEIGHT;
	}
	return LCD_CTRL_WIDTH;
}

#endif
