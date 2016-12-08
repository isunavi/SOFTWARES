#include "drvFPGA.h"
#include "defines.h"
#include "board.h"

#include "debug.h"

// __delay_us(10);
//#define SPI

#ifdef BOARD_STM32F4DISCOVERY

#define GPIO_FPGA_nCONFIG           GPIOE
#define GPIO_Pin_FPGA_nCONFIG       GPIO_Pin_3
#define GPIO_FPGA_DCLK              GPIOE
#define GPIO_Pin_FPGA_DCLK          GPIO_Pin_1
#define GPIO_FPGA_DATA0             GPIOE
#define GPIO_Pin_FPGA_DATA0         GPIO_Pin_5

// FPGA programming intercom
#define FPGA_nCONFIG_H         GPIO_FPGA_nCONFIG->BSRRL = GPIO_Pin_FPGA_nCONFIG // (output)
#define FPGA_nCONFIG_L         GPIO_FPGA_nCONFIG->BSRRH = GPIO_Pin_FPGA_nCONFIG


//#define FPGA_nSTATUS    //
#define FPGA_CONF_DONE      1 //(GPIOA->IDR & GPIO_Pin_8)  //!

#define FPGA_DCLK_H         GPIO_FPGA_nCONFIG->BSRRL = GPIO_Pin_FPGA_DCLK //
#define FPGA_DCLK_L         GPIO_FPGA_nCONFIG->BSRRH = GPIO_Pin_FPGA_DCLK

#define FPGA_DATA0_H        GPIO_FPGA_DATA0->BSRRL = GPIO_Pin_FPGA_DATA0 //
#define FPGA_DATA0_L        GPIO_FPGA_DATA0->BSRRH = GPIO_Pin_FPGA_DATA0

//#define INIT_FPGA_CONF 
//(GPIOA->CRL |= (GPIO_CRL_MODE0  | GPIO_CRL_MODE2 | GPIO_CRL_MODE3))  \
//    (GPIOA->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_CNF2 | GPIO_CRL_CNF3)) 
// #define FPGA_RESET_H          (GPIOB->BSRR = GPIO_BSRR_BR11)
// #define FPGA_RESET_L          (GPIOB->BSRR = GPIO_BSRR_BR11)
// // soft reset
// #define FPGA_SOFT_RESET_H          (GPIOB->BSRR = GPIO_BSRR_BR11)
// #define FPGA_SOFT_RESET_L          (GPIOB->BSRR = GPIO_BSRR_BR11)
#else


#endif

void modFPGA_Init(void) {


    //----------- SPI -----------------------------------
    /*
#ifdef SPI	
    //вывод управлени€ SS: выход двухтактный, общего назначени€,50MHz                             /
    GPIOB->CRH |=  (GPIO_CRH_MODE15 | GPIO_CRH_MODE13 | GPIO_CRH_MODE12); 
    GPIOB->CRH &= ~(GPIO_CRH_CNF15 |  GPIO_CRH_CNF13 | GPIO_CRH_CNF12); 
    GPIOB->CRH |=  (GPIO_CRH_CNF15_1 | GPIO_CRH_CNF13_1);
    //GPIOB->ODR |= ((1<<15)|(1<<13)|(1<<12));
  
     
    //вывод MISO: вход цифровой с подт€гивающим резистором, подт€жка к плюсу
    GPIOB->CRH   &= ~GPIO_CRH_MODE14;    //
    GPIOB->CRH   &= ~GPIO_CRH_CNF14;     //
    GPIOB->CRH   |=  GPIO_CRH_CNF14_1;   //
    GPIOB->BSRR   =  GPIO_BSRR_BS14;     //
     

    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN; //подать тактирование 
    
    SPI2->CR1     = 0x0000;             //очистить первый управл€ющий регистр
    SPI2->CR2     = 0x0000;             //очистить второй управл€ющий регистр
    SPI2->CR1    |= SPI_CR1_MSTR;       //контроллер должен быть мастером    
    //SPI2->CR1    |= SPI_CR1_BR;         //задаем скорость
    
    SPI2->CR1    |= SPI_CR1_SSI;        //обеспечить высокий уровень программного NSS
    SPI2->CR1    |= SPI_CR1_SSM;        //разрешить программное формирование NSS
    
    //SPI2->CR1    |= SPI_CR1_LSBFIRST;
    
    SPI2->CR1    |= SPI_CR1_SPE;        //разрешить работу модул€ SPI
    
    
    SPI2->CR1    |= SPI_CR1_CPHA; // ¬ыборка по заднему спадающему фронту
    //SPI2->CR1    |= SPI_CR1_CPOL;
    

#else
  
    
    GPIOB->CRH |=  (GPIO_CRH_MODE15 | GPIO_CRH_MODE13 | GPIO_CRH_MODE12); 
    GPIOB->CRH &= ~(GPIO_CRH_CNF15 | GPIO_CRH_CNF13 | GPIO_CRH_CNF12);
    
    GPIOB->CRH &= ~GPIO_CRH_MODE14;
	GPIOB->CRH &= ~GPIO_CRH_CNF14_1;
	GPIOB->CRH |= GPIO_CRH_CNF14_0;
	GPIOB->ODR |= GPIO_ODR_ODR14;   //резистором к VDD


#endif
*/ 
    //SPI_FPGA_CS_H;

}
		
uint8_t modFPGA_xspi (uint8_t *pPutChar) {
    unsigned char  Char = 0;
    /**
#ifdef SPI	
    SPI2->DR = *pPutChar;  
    while (!(SPI2->SR & SPI_SR_TXE)){};      //ожидание окончани€ передачи  
    while (!(SPI2->SR & SPI_SR_RXNE)){};      
    Char = SPI2->DR;
#else  
    
    for (i=0; i < 8; i++)
    {
            if(*pPutChar & 0x80)
            {
                SPI_FPGA_MOSI_H;
									   //tr('1');
            }
            else
            {
                SPI_FPGA_MOSI_L;
									   //tr('0');
            }
            
            //_delay_us(2);
            __NOP();
            //__NOP();
            //__NOP();
            //__NOP();
            SPI_FPGA_SCK_H;
            *pPutChar = *pPutChar << 1; 
            
            //_delay_us(2);
            __NOP();
            //__NOP();
            //__NOP();
            //__NOP();
            

            SPI_FPGA_SCK_L; //
            Char = Char << 1;
            //_delay_us(2);
            __NOP();
            //__NOP();
            //__NOP();   
            if(SPI_FPGA_MISO)
            { Char = Char | 0x01; }  //—читать бит данных
                       
    }
    
    //_delay_ms(1); //

#endif
    */
	return Char;
}


uint8_t modFPGA_setRegister (uint8_t adress, uint8_t byte) {
    uint8_t respond;
    /**
    SPI_FPGA_CS_L; 
    
    //_delay_us(20);
    modFPGA_spiPutChar( &adress);
    respond = modFPGA_spiPutChar( &pChar);
    //_delay_us(20);
    SPI_FPGA_CS_H;
    //_delay_us(30);
    */
    return respond;
    
}



void modFPGA_LoadFPGA_StartLoad (void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    
#if BOARD_STM32F4DISCOVERY 
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_FPGA_nCONFIG |
                                    GPIO_Pin_FPGA_DCLK |
                                    GPIO_Pin_FPGA_DATA0;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
#endif
#ifdef BOARD_FPGA_comlink_V2
    
    RCC->APB2ENR |=  RCC_APB2ENR_AFIOEN | //включить тактирование альтернативных функций        /
        RCC_APB2ENR_IOPAEN |     //включить тактирование порта ј
        RCC_APB2ENR_IOPBEN;     //включить тактирование порта B
    
    // FPGA programming interface -----------------------
    GPIOB->CRL |= (GPIO_CRL_MODE6);
    GPIOB->CRL &= ~(GPIO_CRL_CNF6);
    GPIOB->ODR |= (1 << 6);
    
    GPIOB->CRL |= (GPIO_CRL_MODE1  | GPIO_CRL_MODE0);
    GPIOB->CRL &= ~(GPIO_CRL_CNF1 | GPIO_CRL_CNF0);
    GPIOB->ODR |= ((1 << 1) | (1 << 0));
    
    GPIOA->CRH &= ~GPIO_CRH_MODE14;
	GPIOA->CRH &= ~GPIO_CRH_CNF14_1;
	GPIOA->CRH |= GPIO_CRH_CNF14_0;
	GPIOA->ODR |= GPIO_ODR_ODR14;   //резистором к VDD
    
    //----------- FPGA_RESET -------------------------------
    //GPIOB->CRH |=  (GPIO_CRH_MODE11); // 
    //GPIOB->CRH &= ~(GPIO_CRH_CNF11);
    //GPIOB->ODR |= (1<<11);
#endif
    FPGA_nCONFIG_L;
    _delay_ms(10); // 100
    
    FPGA_nCONFIG_H;
    _delay_ms(10); // 100
}


msg_t modFPGA_LoadFPGA_block (uint8_t *sendBuf, uint32_t size) {
    msg_t      respond = FUNCTION_RETURN_OK;
    uint8_t    data;
    uint8_t    m;
    uint32_t   i;

    for (i = 0; i < size; i++) {
        data = *sendBuf++;
        for (m = 0; m < 8; m++) {
            if (data & 0x01) {
                FPGA_DATA0_H;
            } else {
                FPGA_DATA0_L;
            }
            //_delay_us(10);
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            FPGA_DCLK_H;
            //_delay_us(10);
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            data = data >> 1;
            FPGA_DCLK_L;
        }
    }

    return respond;
}
				

msg_t modFPGA_LoadFPGA_complite(void) {
    msg_t  respond = FUNCTION_RETURN_OK;
	
    if (0 != FPGA_CONF_DONE) {
        //LED_STATUS_4_ON;
        FPGA_nCONFIG_L;
        _delay_ms(10); // 100
        FPGA_nCONFIG_H;
        _delay_ms(10); // 100
    
    //_delay_ms(100); // типа задержка включени€
        respond = FUNCTION_RETURN_OK;
    }
    
	//ex=1; //отключаем дальнеюшую инициацию плис

    return respond;
}
