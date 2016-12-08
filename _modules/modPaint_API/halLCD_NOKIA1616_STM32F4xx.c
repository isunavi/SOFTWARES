// даташит - http://www.mdfly.com/newmdfly/products/LPC2148TFTGameboard/SPFD54124B.pdf

/// https://github.com/gresolio/N3310Lib/blob/master/n3310.c
// http://we.easyelectronics.ru/lcd_gfx/vyvod-teksta-na-displey-nokia-1616.html
// http://we.easyelectronics.ru/lcd_gfx/vyvod-kartinok-na-displey-nokia-1616.html
// http://we.easyelectronics.ru/lcd_gfx/shrifty-bolshe-horoshih-i-raznyh.html
// http://we.easyelectronics.ru/lcd_gfx/podklyuchenie-displeya-nokia-1616-na-primere-lpc1343.html
// !!!!!!!!!
// http://radiokot.ru/articles/53/
// http://we.easyelectronics.ru/OlegG/ispolzovanie-usart-stm32-dlya-upravleniya-lcd-nokia-1202.html
//!!!!!!!!!!!


#include "halPaint.h"
#include "board.h"
#include "modPaint.h"

#if (LCD_NOKIA1616)

#define SPFD54124B_SEND_CMD		0x0000
#define SPFD54124B_SEND_DATA	0x0100

#define SPFD54124B_CMD_NOP		rv(0x00)
#define SPFD54124B_CMD_SLPOUT	rv(0x11)
#define SPFD54124B_CMD_NORON	rv(0x13)

#define SPFD54124B_CMD_INVOFF	rv(0x20)
#define SPFD54124B_CMD_INVON	rv(0x21)
#define SPFD54124B_CMD_DISPON	rv(0x29)
#define SPFD54124B_CMD_CASET	rv(0x2A)
#define SPFD54124B_CMD_RASET	rv(0x2B)
#define SPFD54124B_CMD_RAMWR	rv(0x2C)
#define SPFD54124B_CMD_RGBSET	rv(0x2D)

#define SPFD54124B_CMD_MADCTR	rv(0x36)
#define SPFD54124B_CMD_VSCSAD	rv(0x37)
#define SPFD54124B_CMD_COLMOD	rv(0x3A)

#define SPFD54124B_CMD_COLMOD_MCU12bit	rv(3)			// MCU interface 12bit
#define SPFD54124B_CMD_COLMOD_MCU16bit	rv(5)			// MCU interface 16bit
#define SPFD54124B_CMD_COLMOD_MCU18bit	rv(6)			// MCU interface 18bit
#define SPFD54124B_CMD_COLMOD_RGB12bit	rv(30)          // RGB interface 16bit
#define SPFD54124B_CMD_COLMOD_RGB16bit	rv(50)          // RGB interface 16bit
#define SPFD54124B_CMD_COLMOD_RGB18bit	rv(60)			// RGB interface 18bit

#define SPFD54124B_CMD_MADCTR_MY	rv(1 << 7)		// Row Address Order
#define SPFD54124B_CMD_MADCTR_MX	rv(1 << 6)		// Column Address Order
#define SPFD54124B_CMD_MADCTR_MV	rv(1 << 5)		// Row/Column Exchange
#define SPFD54124B_CMD_MADCTR_ML	rv(1 << 4)		// Vertical Refresh Order
#define SPFD54124B_CMD_MADCTR_RGB	rv(1 << 3)		// RGB-BGR ORDER

// MY
#define SPFD54124B_CMD_PTLAR	rv(0x30)
#define SPFD54124B_CMD_SCRLAR	rv(0x33)
#define SPFD54124B_CMD_RAMHD	rv(0x2E) // Прочитать память
#define SPFD54124B_CMD_RDDSDR	rv(0x0F) // прочитать два байта самотеста


const uint16_t init_lcd1616ph [] = {
	SPFD54124B_CMD_SLPOUT,
	SPFD54124B_CMD_COLMOD,
    SPFD54124B_SEND_DATA | SPFD54124B_CMD_COLMOD_MCU16bit,
	SPFD54124B_CMD_DISPON,
	SPFD54124B_CMD_INVOFF,
	SPFD54124B_CMD_NORON,
    //SPFD54124B_CMD_VSCSAD, 0, 1,
};



// структура параметров дисплея
struct halLCD_lcdStruct_t {
	uint8_t     orientation; // Ориентация
    color_t     color;
} halLCD_lcdStruct;


// некалиброванная задержка для процедуры инициализации LCD

void halLCD_setColor (color_t color)
{
    halLCD_lcdStruct.color = color;
}

//static __inline 
void halLCD_xspi_rev (uint16_t data)
{
#if LCD_UART 
    data = __RBIT(data) >> 23;
    while ((USART6->SR & USART_SR_TXE) == 0){}; /* Transmitter register is not empty */
    USART6->DR = data;
#else
    uint8_t i;
    for (i = 0; i < 9; i++)
    {
        if (data & 0x0100)
        {
            LCD_DAT_H;
        } else {
            LCD_DAT_L;
        }
        data = data << 1;
        LCD_CLK_H;
        __NOP();
	    //if(0 != MMC_MISO) { res = res | 0x01; }  //Считать бит данных
        LCD_CLK_L;
	    __NOP();
    }
#endif
	//return res;
}


//static __inline 
void halLCD_xspi (uint16_t data)
{
#if LCD_UART 
    //data = __RBIT(data) >> 23;
    while ((USART6->SR & USART_SR_TXE) == 0){}; /* Transmitter register is not empty */
    USART6->DR = data;
#else
    uint8_t i;
    for (i = 0; i < 9; i++)
    {
        if (data & 0x0100)
        {
            LCD_DAT_H;
        } else {
            LCD_DAT_L;
        }
        data = data << 1;
        LCD_CLK_H;
        __NOP();
	    //if(0 != MMC_MISO) { res = res | 0x01; }  //Считать бит данных
        LCD_CLK_L;
	    __NOP();
    }
#endif
	//return res;
}



void halLCD_sendCmd (uint8_t data) {
    LCD_CS_L;
    halLCD_xspi_rev (SPFD54124B_SEND_CMD | data);
    LCD_CS_H;
}


void halLCD_sendData (uint8_t data) {
    LCD_CS_L;
    halLCD_xspi_rev (SPFD54124B_SEND_DATA | data);
    LCD_CS_H;
}


void halLCD_cmd1616 (uint8_t cmd, uint16_t a, uint16_t b) {
    //uint8_t *pt = (uint8_t *)a;
	halLCD_sendCmd (cmd);
    halLCD_sendData (a >> 8);  // сначала старший
	halLCD_sendData (a);
    //pt = (uint8_t *)b;
	halLCD_sendData (b >> 8);    
    halLCD_sendData (b);
}


void halLCD_setWindow (coord_t x, coord_t y, uint16_t w, uint16_t h) {
    x += LCD_CTRL_X_OFFSET;
    y += LCD_CTRL_Y_OFFSET;
    halLCD_cmd1616 (SPFD54124B_CMD_CASET, x, x + (w - 1)); // column start/end
    halLCD_cmd1616 (SPFD54124B_CMD_RASET, y, y + (h - 1)); // page start/end
	halLCD_sendCmd (SPFD54124B_CMD_RAMWR);
}


//static __inline 
void halLCD_sendPixel (color_t *color) {
    uint8_t *pt = (uint8_t *)color;
 	halLCD_xspi_rev (SPFD54124B_SEND_DATA | pt[1]); // сначала старший
 	halLCD_xspi_rev (SPFD54124B_SEND_DATA | pt[0]); //
//     
  //  halLCD_xspi ((pt[0] <<1) | 0x0001); // сначала младший
//	halLCD_xspi ((pt[1] <<1) | 0x0001); //
}


uint32_t halLCD_rspi (void)
{
    uint32_t res = 0;
#if LCD_UART 
    
#else
    uint8_t i;
    for (i = 0; i< (1+ 8*3); i++)
    {
        LCD_CLK_H;//Импульс
	    res = res << 1;
        __NOP();
	    if (0 != LCD_DAT_IN) { res |= 0x0001; }  //Считать бит данных
        LCD_CLK_L;
	    __NOP();
    }
#endif
	return res;// & 0x00FF;
}


// Доделать, вполне возможно!
color_t halLCD_getPixel (coord_t x , coord_t y)
{
/**
    volatile uint16_t respond = 0;
    GPIO_InitTypeDef GPIO_InitStructure;
    
    x += 1;
    y += 2;
    
    halLCD_cmd1616(SPFD54124B_CMD_CASET, x, x);
	halLCD_cmd1616(SPFD54124B_CMD_RASET, y, y);	

    //halLCD_sendCmd(SPFD54124B_CMD_RAMHD); 
    uint8_t i, cod = 0x04;//SPFD54124B_CMD_RAMHD;
    
    // activate lcd by low on CS pin
    LCD_CS_L;
    
    for (i = 0; i< 9 ; i++)
    {
        if (cod & 0x0100)//Выставить бит данных
        {
            LCD_DAT_H; //tr('1');
        } else {
            LCD_DAT_L; //tr('0');
        }
        cod = cod << 1;
        LCD_CLK_H;//Импульс
	    //res = res << 1;
        //__NOP();
        __NOP();//_delay_ms(1);
	    //if(0 != MMC_MISO) { res = res | 0x01; }  //Считать бит данных
        LCD_CLK_L;
	    __NOP();//_delay_ms(1);
    }
	

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_LCD_DAT;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    respond += halLCD_rspi();
//     respond += halLCD_rspi();
//     respond += halLCD_rspi();
//     respond += halLCD_rspi();
//     respond += halLCD_rspi();
//     respond += halLCD_rspi();
//     respond += halLCD_rspi();
//     respond  = halLCD_rspi() << 8;
//     respond |= halLCD_rspi();


    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_LCD_DAT;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    LCD_CS_H;
    halLCD_sendCmd(SPFD54124B_CMD_NOP);
*/
    return COLOR_BLACK; //respond;
}


void   hal_paint_init (uint8_t mode)
{
    const uint16_t *data = &init_lcd1616ph[0];
    uint16_t size;
    
#ifdef BOARD_STM32F4DISCOVERY
    //TIM_TimeBaseInitTypeDef Base_Timer;
    //TIM_OCInitTypeDef OC_Init;
    
    GPIO_InitTypeDef GPIO_InitStruct;

    /* GPIO Ports Clock Enable */
    __GPIOC_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();

  
#if LCD_UART 
    __UART6_CLK_ENABLE();
    
    /*Configure GPIO pin : PA7 */
    GPIO_InitStruct.Pin = GPIO_Pin_LCD_CS; // GPIO_Pin_LCD_RESET
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init (GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_LCD_CLK | GPIO_PIN_LCD_DAT;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init (GPIOC, &GPIO_InitStruct);
    
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_USART6);
    
    USART6->BRR = 0x0040; //0x0020 /* MAX speed */
    /* 0.5 stop bit, pib CLK ena, polatity and phaze, clock fpr last bit */
    USART6->CR2 =   USART_CR2_STOP_0 | 
                    USART_CR2_CLKEN |
                    USART_CR2_CPOL |
                    USART_CR2_CPHA |
                    USART_CR2_LBCL; 
    /* oversampling 8, 9 bit, transmitter ena, usart ena */   
    USART6->CR1 =   USART_CR1_OVER8 |
                    USART_CR1_M |
                    USART_CR1_TE |
                    USART_CR1_UE;
                    
#else
    GPIO_InitStruct.Pin = GPIO_PIN_LCD_CS;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_LCD_CLK | GPIO_PIN_LCD_DAT;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
#endif

    // Init PWM
    {
        /*
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
     
        GPIO_StructInit(&GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
     
        GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_TIM3);
        
        TIM_TimeBaseStructInit(&Base_Timer);

        Base_Timer.TIM_Prescaler = 8 - 1;   // делитель частоты
        Base_Timer.TIM_Period = TIMER_DIV;  // период
        Base_Timer.TIM_CounterMode = TIM_CounterMode_Up; // счёт вверх
        TIM_TimeBaseInit(TIM3, &Base_Timer);
        
        TIM_OCStructInit(&OC_Init);
        OC_Init.TIM_OCMode = TIM_OCMode_PWM1;   // работаем в режиме ШИМ ( PWM )
        OC_Init.TIM_OutputState = TIM_OutputState_Enable;
        OC_Init.TIM_OCPolarity = TIM_OCPolarity_High;  // положительная полярность
        OC_Init.TIM_Pulse = TIMER_DIV / 2; // всегда меандр

        TIM_OC2Init(TIM3, &OC_Init);  // настраиваем второй канал PC7
        TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);

        TIM_Cmd(TIM3, ENABLE); // запускаем подсветку
        */
    }
#endif
    LCD_CS_H;
	LCD_RESET_L;
	HAL_Delay (100);
	LCD_RESET_H;
	HAL_Delay (100);
    
    size = sizeof(init_lcd1616ph) / sizeof(init_lcd1616ph[0]);
    while (size--)
    {
        LCD_CS_L;
        halLCD_xspi_rev (*data++);
        LCD_CS_H;
    }
    HAL_Delay (100);

    hal_paint_setOrientation (mode);
}


void hal_paint_setPixelColor (coord_t x, coord_t y, color_t color) {
    x += LCD_CTRL_X_OFFSET;
    y += LCD_CTRL_Y_OFFSET;
    halLCD_cmd1616 (SPFD54124B_CMD_CASET, x, 1);
	halLCD_cmd1616 (SPFD54124B_CMD_RASET, y, 1);
	halLCD_sendCmd (SPFD54124B_CMD_RAMWR);
    LCD_CS_L;    
	halLCD_sendPixel (&color);
    LCD_CS_H;
}


void hal_paint_setPixel (coord_t x, coord_t y) {
    x += LCD_CTRL_X_OFFSET;
    y += LCD_CTRL_Y_OFFSET;
    halLCD_cmd1616 (SPFD54124B_CMD_CASET, x, 1);
	halLCD_cmd1616 (SPFD54124B_CMD_RASET, y, 1);
	halLCD_sendCmd (SPFD54124B_CMD_RAMWR);
    LCD_CS_L;    
	halLCD_sendPixel (&halLCD_lcdStruct.color);
    LCD_CS_H;
}


// массив всегда заполняется слева направао и с верху в низ
void     hal_paint_fillBlock (coord_t x, coord_t y, uint16_t w, uint16_t h, color_t *buf) {
    uint32_t n;
    
    n = (w * h);
   	halLCD_setWindow (x, y, w, h);
    LCD_CS_L;
    while(n--) halLCD_sendPixel (buf++);
    LCD_CS_H;
	halLCD_sendCmd (SPFD54124B_CMD_NOP);
}


void     hal_paint_fillBlockColor (coord_t x, coord_t y, uint16_t w, uint16_t h, color_t color) {
    uint32_t n;
    
    n = (w * h);
   	halLCD_setWindow (x, y, w, h);
	LCD_CS_L;
    while (n--) halLCD_sendPixel (&color);
    LCD_CS_H;
	halLCD_sendCmd (SPFD54124B_CMD_NOP);
}


void     hal_paint_cls (color_t color) {
    hal_paint_fillBlockColor (0, 0, SCREEN_W, SCREEN_H, color);
}


void     hal_paint_setBackLight (uint8_t value) {

}


void hal_paint_repaint (void)
{
    // @todo
}


void hal_paint_setOrientation (uint8_t orient) {
	uint8_t data = 0;
	halLCD_lcdStruct.orientation = orient;

	switch (orient) {
		case LCD_ORIENTATION_90:
			data = SPFD54124B_CMD_MADCTR_MV | SPFD54124B_CMD_MADCTR_MX;
			break;
		case LCD_ORIENTATION_180:
			data = SPFD54124B_CMD_MADCTR_MY | SPFD54124B_CMD_MADCTR_MX;
			break;
		case LCD_ORIENTATION_270:
			data = SPFD54124B_CMD_MADCTR_MV | SPFD54124B_CMD_MADCTR_MY;
			break;
	}
	halLCD_sendCmd (SPFD54124B_CMD_MADCTR);
	halLCD_sendData (data);
}


uint16_t hal_paint_getWidth (void) {
	switch (halLCD_lcdStruct.orientation)
    {
		case LCD_ORIENTATION_0:
		case LCD_ORIENTATION_180:
			return SCREEN_W;
	}
	return SCREEN_H;
}


uint16_t hal_paint_getHeight (void)
{
	switch (halLCD_lcdStruct.orientation)
    {
		case LCD_ORIENTATION_0:
		case LCD_ORIENTATION_180:
		return SCREEN_H;
	}
	return SCREEN_W;
}

#endif
