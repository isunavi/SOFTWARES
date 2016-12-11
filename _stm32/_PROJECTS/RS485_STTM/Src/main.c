/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f0xx_hal.h"

/* USER CODE BEGIN Includes */
#include "board.h"
#include "mb.h"
#include "mbport.h"
//#include <string.h> //for memcpy
#include "sysReport.h"
#include "modSysClock.h"
#include "drvStorage.h"
#include "debug.h"
#include "mod1Wire.h"
//#include "halI2C.h"
//#include "drvLM75.h"
//#include <math.h>       /* sqrt */


/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
static void MX_GPIO_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
uint32_t error;
volatile static uint16_t eeprom_buf [1024 / 2]; // EEPROM size - 1024 bytes
uint32_t test_to_be_performed_twice;

//static volatile uint32_t ggg = 0;

/* ----------------------- Static variables ---------------------------------*/
volatile static const uint16_t compile_date[]    = { 2016, 12, 10 };
volatile static const uint16_t compile_version[] = { 14 };
volatile static const uint16_t firmware_date[]   = { 2016, 12, 10 };
//volatile static const char compile_date[12] = __DATE__;
//volatile static const char compile_time[10] = __TIME__;

int16_t  tmpS16;
uint16_t  tmpU16;
int32_t SDADC_v1, SDADC_v2;
float SDADC_vref_current;
volatile int32_t tmpS32;
uint32_t tmpU32;
float tmpF;
uint32_t delay_blink = 0;
uint32_t delay_set_out = 0;
uint32_t delay_get_in = 0;
uint32_t delay_get_temp = 0;
extern uint32_t USART1_errCount;

//double sum;

uint32_t delay_set_zero;
uint8_t v_flag = 0;
int32_t v_zero = 0;
uint32_t cnt_test = 0;


static uint32_t *UID = (uint32_t *)0x1FFFF7E8; //0x1FFF7A10
volatile uint32_t ID [3];

//extern
//eMBErrorCode
//eMBRTUSend( UCHAR ucSlaveAddress, const UCHAR * pucFrame, USHORT usLength );
//const UCHAR * Stb[] = {"434342342342342342343"};



#define REG_INPUT_START         1
#define REG_INPUT_NREGS         32

#define REG_HOLDING_START       1
#define REG_HOLDING_NREGS       16

// SYS register emulation
typedef enum {
    HOLDING_REG_DIGITAL_OUT     = 0, //1

    HOLDING_REG_PASS            = 2,

    HOLDING_REG_RS485_ADDRESS   = 4,
    HOLDING_REG_RS485_BAUD      = 5,
    HOLDING_REG_RS485_PARITY    = 6,
    //HOLDING_REG_RS485_WORDLENGTH = 6

    HOLDING_REG_RS485_DELAY     = 7,

    /*
    HOLDING_REG_ANALOG_OUT_1 = 1, //2
    // дальше регистры дл¤ отладки
    HOLDING_REG_ADC_1_CORRECT, //6
    HOLDING_REG_ADC_1_DIV, //8
    HOLDING_REG_SDADC_1_CORRECT, //11
    HOLDING_REG_RTD_1_COEFFICIENT, //15
    HOLDING_REG_DAC, //19
    */

} HOLDING_REGs;

//typedef enum {
//    RTD_TYPE_PT100 = 0,
//    RTD_TYPE_100M,
//
//} RTD_TYPEs;


typedef enum {
    INPUT_REG_DIGITAL_IN        = 0, //1

    INPUT_REG_TEMPERATURE_1     = 1, //2
    INPUT_REG_TEMPERATURE_2     = 2, //3
    INPUT_REG_TEMPERATURE_3     = 3, //4
    INPUT_REG_TEMPERATURE_4     = 4, //5

    INPUT_REG_ADC_RAW           = 5,
    INPUT_REG_ADC_V             = 6,

    TEST                        = 7,

    INPUT_REG_RS485_ERRORS      = 10, //11


    INPUT_REG_ADC_VREF, //12

    INPUT_REG_COMPILE_DATE_1  = 25, //26
    INPUT_REG_COMPILE_DATE_2  = 26, //27
    INPUT_REG_COMPILE_DATE_3  = 27, //28
    INPUT_REG_COMPILE_VERSION = 28, //29
    INPUT_REG_FIRMWARE_DATE_1 = 29, //30
    INPUT_REG_FIRMWARE_DATE_2 = 30, //31
    INPUT_REG_FIRMWARE_DATE_3 = 31, //32

} INPUT_REGs;


// modbus regs, etc
eMBErrorCode    eStatus;
const UCHAR     ucSlaveID[] = {0xAA, 0xBB, 0xCC};

static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT usRegInputBuf [REG_INPUT_NREGS];

static USHORT   usRegHoldingStart = REG_HOLDING_START;
static USHORT usRegHoldingBuf [REG_HOLDING_NREGS];

uint16_t *modbus_delay = &usRegHoldingBuf [HOLDING_REG_RS485_DELAY];


uint16_t  crc16_CITT (uint8_t *inData, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    uint8_t i;

    while (len--)
    {
        crc ^= *inData++ << 8;
        for (i = 0; i < 8; i++)
            crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
    }

    return crc;
}


/*
typedef enum {
    EEPROM_REG_RS485_ADDRESS = 0,
    EEPROM_REG_RS485_BAUD,
    EEPROM_REG_RS485_PARITY,

    EEPROM_REG_RS485_DELAY,

    EEPROM_REG_ALL //must bee <= FLASH_PAGE_SIZE / 2

} EEPROM_REGs;
*/

uint16_t halEEPROM_read16 (uint32_t adress)
{
    return eeprom_buf [adress];
}


bool_t halEEPROM_write16 (uint32_t adress, uint16_t value)
{
    eeprom_buf [adress] = value;
    return TRUE;
}


void eeprom2regs (void)
{
    uint32_t i;

    for (i = 0; i < REG_HOLDING_NREGS; i++)
        usRegHoldingBuf [i] = halEEPROM_read16 (i);
}


void regs2eeprom (void)
{
    uint32_t i;

    for (i = 0; i < REG_HOLDING_NREGS; i++)
        halEEPROM_write16 (i, usRegHoldingBuf [i]);
}


//void sys_settings_set2default (void);
void sys_settings_restore2flash (void);

systime_t  delay_eeprom_write;



//----------------------------------------запись настроек в EEPROM----------------------------------------

void halEEPROM_commit (void)
{
    uint16_t crc;
    uint16_t tmp16 = 0;
    size_t sz = FLASH_PAGE_SIZE / 2; //sizeof (eeprom_buf);
    uint32_t i;

    while (1)
    {
        // only 1 sec writing!
        if (FUNCTION_RETURN_OK == modSysClock_timeout (&delay_eeprom_write, 1000, SYSCLOCK_GET_TIME_MS_1))
        {
            halFLASH_unlock ();
            halFLASH_erase_page (FLASH_USER_START_ADDR);
            halFLASH_erase_check (FLASH_USER_START_ADDR); //+8
            crc = crc16_CITT ((uint8_t *)&eeprom_buf [0], sz * 2 - 2);
            eeprom_buf [sz - 1] = crc;
            for (i = 0; i < sz; i++)
            {
                tmp16 = eeprom_buf [i];
                halFLASH_FlashWord16Prog (FLASH_USER_START_ADDR + i * 2, eeprom_buf [i]);
            }
            halFLASH_lock ();
            break;
        }
    }
}


// reading config
void halEEPROM_init (void)
{
    uint16_t crc;
    uint16_t tmp16 = 0;
    size_t sz = FLASH_PAGE_SIZE / 2; //sizeof(sConfig);
    uint32_t i;

    delay_eeprom_write = modSysClock_getTime ();
    halFLASH_unlock ();
    for (i = 0; i < sz; i++) //read flash
    {
        eeprom_buf [i] = halFLASH_read16 (FLASH_USER_START_ADDR + i * 2); //init ram eeprom buf data
    }
    halFLASH_lock ();
    crc = crc16_CITT ((uint8_t *)&eeprom_buf [0], sz * 2 - 2);
    if (eeprom_buf [sz - 1] != crc) // flash clear or danaged!
    {
        for (uint32_t i = 0; i < sz; i++) //clear all EEPROM buf
        {
            eeprom_buf [i] = 0xFFFF;
        }
        // set 2 default
        usRegHoldingBuf [HOLDING_REG_RS485_ADDRESS] = 10;
        usRegHoldingBuf [HOLDING_REG_RS485_BAUD] = 9600;
        usRegHoldingBuf [HOLDING_REG_RS485_PARITY] = MB_PAR_NONE;
        usRegHoldingBuf [HOLDING_REG_RS485_DELAY] = 10;
        regs2eeprom ();
        halEEPROM_commit ();
    }
    eeprom2regs ();
}


eMBErrorCode
eMBRegInputCB ( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs ) {
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if( ( usAddress >= REG_INPUT_START )
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegInputStart );
        while( usNRegs > 0 )
        {
            *pucRegBuffer++ =
                ( unsigned char )( usRegInputBuf[iRegIndex] >> 8 );
            *pucRegBuffer++ =
                ( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}


eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs,
                 eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if( ( usAddress >= REG_HOLDING_START ) &&
        ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegHoldingStart );
        switch ( eMode )
        {
            /* Pass current register values to the protocol stack. */
        case MB_REG_READ:
            /*
            if (usAddress >= 1001)
            {
                while( usNRegs > 0 )
                {
                    *pucRegBuffer++ = ( UCHAR ) ( usRegHoldingBuf[iRegIndex] >> 8 );
                    *pucRegBuffer++ = ( UCHAR ) ( usRegHoldingBuf[iRegIndex] & 0xFF );
                    iRegIndex++;
                    usNRegs--;
                }
            } else {
            */
                while( usNRegs > 0 )
                {
                    *pucRegBuffer++ = ( UCHAR ) ( usRegHoldingBuf[iRegIndex] >> 8 );
                    *pucRegBuffer++ = ( UCHAR ) ( usRegHoldingBuf[iRegIndex] & 0xFF );
                    iRegIndex++;
                    usNRegs--;
                }
            /*} */
            break;

            /* Update current register values with new values from the
             * protocol stack. */
        case MB_REG_WRITE:
            while( usNRegs > 0 )
            {
                usRegHoldingBuf [iRegIndex] = *pucRegBuffer++ << 8;
                usRegHoldingBuf [iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;

                //------------
                if (666 == usRegHoldingBuf [HOLDING_REG_PASS]) //need set settings?
                {
                    usRegHoldingBuf [HOLDING_REG_PASS] = 0; //clear pass
                    regs2eeprom ();
                    halEEPROM_commit ();
                    eeprom2regs ();  //TODO need?
                }
                //------------

            }
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}


eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,
               eMBRegisterMode eMode )
{
    return MB_ENOREG;
}

eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    return MB_ENOREG;
}


void halRCC_init (void)
{
    /* (1) Enable interrupt on HSE ready */
    /* (2) Enable the CSS
    Enable the HSE and set HSEBYP to use the external clock
    instead of an oscillator
    Enable HSE */
    /* Note : the clock is switched to HSE in the RCC_CRS_IRQHandler ISR */
    //RCC->CIR |= RCC_CIR_HSERDYIE; /* (1) */
    RCC->CR |= RCC_CR_CSSON | RCC_CR_HSEBYP |
        RCC_CR_HSEON; /* (2) */


    /* (1) Check the flag HSE ready */
    /* (2) Clear the flag HSE ready */
    /* (3) Switch the system clock to HSE */

    if ((RCC->CIR & RCC_CIR_HSERDYF) != 0) // Check the flag HSE ready
    {
        RCC->CIR |= RCC_CIR_HSERDYC; // Clear the flag HSE ready
         /* (3) */
    }

    /* (1)  Test if PLL is used as System clock */
    /* (2)  Select HSI as system clock */
    /* (3)  Wait for HSI switched */
    /* (4)  Disable the PLL */
    /* (5)  Wait until PLLRDY is cleared */
    /* (6)  Set the PLL multiplier to 6 */
    /* (7)  Enable the PLL */
    /* (8)  Wait until PLLRDY is set */
    /* (9)  Select PLL as system clock */
    /* (10) Wait until the PLL is switched on */
    if ((RCC->CFGR & RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL) /* (1) */
    {
        RCC->CFGR = ((RCC->CFGR & (~RCC_CFGR_SW)) | RCC_CFGR_SW_0); //Switch the system clock to HSE
        while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE) // Wait for HSE switched
        {
            /* For robust implementation, add here time-out management */
        }
    }

    RCC->CR &= (uint32_t)(~RCC_CR_PLLON); //Disable the PLL
    while((RCC->CR & RCC_CR_PLLRDY) != 0) // Wait until PLLRDY is cleared
    {
        /* For robust implementation, add here time-out management */
    }

    RCC->CFGR = (RCC->CFGR & (~RCC_CFGR_PLLMUL)) | (RCC_CFGR_PLLMUL3); // Set the PLL multiplier to 12
    RCC->CR |= RCC_CR_PLLON; // Enable the PLL
    while((RCC->CR & RCC_CR_PLLRDY) == 0) // Wait until PLLRDY is set
    {
        /* For robust implementation, add here time-out management */
    }

    RCC->CFGR = ((RCC->CFGR & (~RCC_CFGR_SW)) | RCC_CFGR_SW_PLL); // Select PLL as system clock
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) // Wait until the PLL is switched on
    {
        /* For robust implementation, add here time-out management */
    }

    HAL_SYSTICK_Config (48000000 / 1000);
    HAL_SYSTICK_CLKSourceConfig (SYSTICK_CLKSOURCE_HCLK);
    HAL_NVIC_SetPriority (SysTick_IRQn, 0, 0);
}


void halADC_init (void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= GPIO_MODER_MODER2; //  Select analog mode for PA2

	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; // Enable the peripheral clock of the ADC
	RCC->CR2 |= RCC_CR2_HSI14ON; // Start HSI14 RC oscillator
	while ((RCC->CR2 & RCC_CR2_HSI14RDY) == 0) // Wait HSI14 is ready
	{
		// For robust implementation, add here time-out management
	}
	//This function performs a self-calibration of the ADC
	if ((ADC1->CR & ADC_CR_ADEN) != 0) // Ensure that ADEN = 0
	{
		ADC1->CR &= (uint32_t)(~ADC_CR_ADEN);  //Clear ADEN
	}
	ADC1->CR |= ADC_CR_ADCAL; // Launch the calibration by setting ADCAL
	while ((ADC1->CR & ADC_CR_ADCAL) != 0) // Wait until ADCAL=0
	{
		// For robust implementation, add here time-out management
	}

	//ADC1->CFGR2 &= ~ADC_CFGR2_CKMODE; // Select HSI14 by writing 00 in CKMODE (reset value)
	ADC1->CFGR1 |= ADC_CFGR1_CONT | ADC_CFGR1_SCANDIR; //  Select the continuous mode and scanning direction
	ADC1->CHSELR = ADC_CHSELR_CHSEL2; // Select CHSEL1
	ADC1->SMPR |= ADC_SMPR_SMP_0 | ADC_SMPR_SMP_1 | ADC_SMPR_SMP_2; //Select a sampling mode of 111 i.e. 239.5 ADC clk to be greater than 17.1us
	ADC->CCR |= ADC_CCR_VREFEN; // Wake-up the VREFINT (only for VBAT, Temp sensor and VRefInt)
	do
    {
	// For robust implementation, add here time-out management
		ADC1->CR |= ADC_CR_ADEN;
	} while ((ADC1->ISR & ADC_ISR_ADRDY) == 0);

	ADC1->CR |= ADC_CR_ADSTART; // start the ADC conversions

}


#define MEDIAN_FILTER_SIZE             64 //размер буфера фильтра


uint16_t ADC_filter_buf [MEDIAN_FILTER_SIZE + 1];

// BaseType - любой перечисляемый тип
// typedef int BaseType - пример
void shell_sort (uint16_t *A, uint32_t N)
{
	uint32_t i,j,k;

	uint16_t t;

	for (k = N/2; k > 0; k /=2)
    {
        for (i = k; i < N; i+=1)
        {
            t = A[i];
            for (j = i; j>=k; j-=k)
            {
                if(t < A[j-k])
                    A[j] = A[j-k];
                else
                    break;
            }
            A[j] = t;
        }
    }
}


uint16_t adc_copy [MEDIAN_FILTER_SIZE];
//#include <string.h>
uint16_t filter_median (uint16_t *adc_buf, uint32_t size, uint16_t sample)
{
    uint32_t tmp32;
    //uint_fast16_t

    uint32_t ADC_filter_pointer = adc_buf [MEDIAN_FILTER_SIZE];

    if (++ADC_filter_pointer >= MEDIAN_FILTER_SIZE)
        ADC_filter_pointer = 0;
    adc_buf [ADC_filter_pointer] = sample;
    adc_buf[MEDIAN_FILTER_SIZE] = ADC_filter_pointer; //store pointer

    /* Copy the data */
    //memcpy (adc_copy, adc_buf, MEDIAN_FILTER_SIZE);
    for (uint32_t i = 0; i < MEDIAN_FILTER_SIZE; i++)
    {
        adc_copy [i] = adc_buf [i];
    }

    /* Сортируем */
    shell_sort (adc_copy, MEDIAN_FILTER_SIZE);

    /*  Среднее арифметическое */
    tmp32 = adc_copy [(MEDIAN_FILTER_SIZE) / 2U - 2U];
    tmp32 += adc_copy [(MEDIAN_FILTER_SIZE) / 2U - 1U];
    tmp32 += adc_copy [(MEDIAN_FILTER_SIZE) / 2U];
    tmp32 += adc_copy [(MEDIAN_FILTER_SIZE) / 2U + 1U];
    tmp32 += adc_copy [(MEDIAN_FILTER_SIZE) / 2U + 2U];

    return tmp32 / 5;
}


#define GPIO_GPIO595_Port               GPIOA

#define GPIO_GPIO595_DATA               GPIO_PIN_7
#define GPIO_GPIO595_SH                 GPIO_PIN_5
#define GPIO_GPIO595_LT                 GPIO_PIN_4
#define GPIO_GPIO595_OUT                GPIO_PIN_6

#define GPIO_595_DATA_L                 GPIO_GPIO595_Port->BSRR = (uint32_t)GPIO_GPIO595_DATA << 16U
#define GPIO_595_DATA_H                 GPIO_GPIO595_Port->BSRR = GPIO_GPIO595_DATA
#define GPIO_595_SH_L                   GPIO_GPIO595_Port->BSRR = (uint32_t)GPIO_GPIO595_SH << 16U
#define GPIO_595_SH_H                   GPIO_GPIO595_Port->BSRR = GPIO_GPIO595_SH
#define GPIO_595_LT_L                   GPIO_GPIO595_Port->BSRR = (uint32_t)GPIO_GPIO595_LT << 16U
#define GPIO_595_LT_H                   GPIO_GPIO595_Port->BSRR = GPIO_GPIO595_LT
#define GPIO_595_DATA_IN                (GPIO_GPIO595_Port->IDR & GPIO_GPIO595_OUT)


uint32_t sys_595_out_state = 0;
uint32_t sys_595_in = 0;
enum GPIO595_outs {
    GPIO595_OUT1 = 4,
    GPIO595_OUT2,
    GPIO595_OUT3,
    GPIO595_OUT4,
    GPIO595_OUT5,
    GPIO595_OUT6,
    GPIO595_OUT7,
    GPIO595_OUT8,
    GPIO595_OUT9,
    GPIO595_OUT10,
    GPIO595_OUT11,
    GPIO595_OUT12,
    GPIO595_OUT13,
    GPIO595_OUT14,
    GPIO595_OUT15,
    GPIO595_OUT16,
    //... not mounted others

};


enum GPIO595_ins {
    GPIO595_IN1 = 0,
    GPIO595_IN2,
    GPIO595_IN3,
    GPIO595_IN4,
    GPIO595_IN5,
    GPIO595_IN6,
    GPIO595_IN7,
    GPIO595_IN8,
    GPIO595_IN9,
    GPIO595_IN10,
    GPIO595_IN11,
    GPIO595_IN12,
    GPIO595_IN13,
    GPIO595_IN14,
    GPIO595_IN15,
    GPIO595_IN16,
    //... not mounted others

};


void sys_595_load (void)
{
    uint8_t i;
    uint32_t data = sys_595_out_state;

    __NOP(); __NOP(); __NOP(); //delayMicroseconds (1);
    GPIO_595_LT_L;
    for (i = 0; i < 32; i++)
    {
        if (data & 0x80000000)
        {
            GPIO_595_DATA_H;
        }
        else
        {
            GPIO_595_DATA_L;
        }
        data = data << 1;
        //HAL_Delay (1);
        __NOP(); __NOP(); __NOP();
        //delayMicroseconds (1);
        GPIO_595_SH_H;
        //HAL_Delay (1);
        __NOP(); __NOP(); __NOP(); //delayMicroseconds (1);
        GPIO_595_SH_L;
        //HAL_Delay (1);
        __NOP(); __NOP(); __NOP(); //delayMicroseconds (1);
    }
    __NOP(); __NOP(); __NOP(); //delayMicroseconds (1);
    GPIO_595_LT_H; // latching...
    __NOP(); __NOP(); __NOP(); //delayMicroseconds (1);
    GPIO_595_LT_L;
    __NOP(); __NOP(); __NOP(); //delayMicroseconds (1);
}


void sys_595_out_set (void) //uint8_t num, bool_t state)   //TODO111
{
    uint32_t tmp32 = 0;

//    switch (data)
//    {
//    case GPIO595_OUT1: tmp32 |= 1 << GPIO595_OUT1; break;
//    //...
//
//    default: return false; break;
//    }
    //tmp32 |= 1 << (GPIO595_OUT1 + num);
    sys_595_out_state = usRegHoldingBuf [HOLDING_REG_DIGITAL_OUT] << GPIO595_OUT1; //TODO!!

//    if (0 != state) //if TRUE = H, FALSE = L
//        sys_595_out_state |= tmp32;
//    else
//        sys_595_out_state &= ~tmp32;

    sys_595_load ();
}


uint32_t sys_595_in_get (uint8_t num)
{
    uint8_t tmp16 = 0;

//    switch (data)
//    {
//    case GPIO595_IN1: tmp16 |= GPIO595_IN1; break;
//    //...
//
//    default: return false; break;
//    }
    tmp16 |= (GPIO595_IN1 + num);
    //set mux in
    sys_595_out_state &= ~0x000F;
    sys_595_out_state |= tmp16;

    sys_595_load ();

    //delayMicroseconds (10);
    __NOP ();
    __NOP ();
    __NOP ();
    __NOP ();
    __NOP ();
    __NOP ();

    if (GPIO_595_DATA_IN)
        return 1; //HIGH;
    else
        return 0; //LOW;
}



/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  //MX_GPIO_Init();

  /* USER CODE BEGIN 2 */
    _delay_init ();
    _debug_init ();

    // Enable the peripheral clock of GPIOA
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN |
        RCC_AHBENR_GPIOBEN |
        RCC_AHBENR_GPIOFEN;
    // for test LED
    //GPIOB->MODER = (GPIOB->MODER & ~(GPIO_MODER_MODER1)) | (GPIO_MODER_MODER1_0); // DIR - Yelloy
    //GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER10)) | (GPIO_MODER_MODER10_0); //RX - Red
    //GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER9)) | (GPIO_MODER_MODER9_0); // TX - Blue

    //GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER7)) |
    //    (GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0 | GPIO_MODER_MODER7_0);
    //TODO add PULLUP-rezistor on board

    //for DS18B20 - all time!~
    GPIOA->MODER &=  ~GPIO_MODER_MODER0; //Out
    GPIOA->MODER |=  GPIO_MODER_MODER0_0;
    GPIOA->OTYPER |=  GPIO_OTYPER_OT_0; //OD
    GPIO_POWER_ON_L;

    //ggg = HAL_RCC_GetHCLKFreq();
    halRCC_init ();
    //ggg = HAL_RCC_GetHCLKFreq();

    modSysClock_setRunPeriod (TIC_PERIOD);

	delay_blink = modSysClock_getTime ();
	delay_set_out = modSysClock_getTime ();
	delay_get_in = modSysClock_getTime ();
	delay_get_temp = modSysClock_getTime ();
	delay_set_zero = modSysClock_getTime ();

    __enable_irq ();

#define MCU_ID (0x444)
#define MCU_ID_MASK (0x00000EFF)
    volatile uint32_t MCU_Id = 0;
    MCU_Id = DBGMCU->IDCODE; /* Read MCU Id, 32-bit access */
    if ((MCU_Id & MCU_ID_MASK) == MCU_ID) /* Check Id */
    {
        //LED_WORK_ON;
    }

    ID [0] = UID [0];
    ID [1] = UID [1];
    ID [2] = UID [2];

    //HAL_Delay (100);
    halEEPROM_init ();

    // init MODBUS registers 2 default -----------------------------
	usRegInputBuf [INPUT_REG_TEMPERATURE_1] = 10000;

	usRegInputBuf [INPUT_REG_COMPILE_DATE_1] = compile_date [0];
	usRegInputBuf [INPUT_REG_COMPILE_DATE_2] = compile_date [1];
	usRegInputBuf [INPUT_REG_COMPILE_DATE_3] = compile_date [2];
    usRegInputBuf [INPUT_REG_COMPILE_VERSION] = compile_version [0];
    usRegInputBuf [INPUT_REG_FIRMWARE_DATE_1] = firmware_date [0];
	usRegInputBuf [INPUT_REG_FIRMWARE_DATE_2] = firmware_date [1];
	usRegInputBuf [INPUT_REG_FIRMWARE_DATE_3] = firmware_date [2];


	/**/
	eStatus = eMBInit (MB_RTU,
        usRegHoldingBuf [HOLDING_REG_RS485_ADDRESS],
        0,
        usRegHoldingBuf [HOLDING_REG_RS485_BAUD], //TODO size
        MB_PAR_NONE);

	if (eStatus != MB_ENOERR)
	{
		//while (1) {};
	}

	/* */
	eStatus = eMBSetSlaveID (0x34, TRUE, &ucSlaveID [0], 3);

	/* Enable the Modbus Protocol Stack.*/
	eStatus = eMBEnable ();
    //--------------------------------------------------------------------------

	mod1Wire_initMaster ();
	//halI2C_init ();
    halADC_init ();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
        __clrwdt ();
        (void) eMBPoll (); //RS485 pull
        mod1Wire_runMaster ();

        usRegInputBuf [TEST] =(uint16_t)cnt_test++;
        usRegInputBuf [INPUT_REG_RS485_ERRORS] = USART1_errCount;

        if (USART1_errCount > 100)
        {
            BOOL eStatus = eMBInit (MB_RTU,
                10,
                0,
                9600, //TODO size
                MB_PAR_NONE);

            if (eStatus != MB_ENOERR)
            {
                while (1) {};
            }
            USART1_errCount = 0;
        }
        
        
        if (FUNCTION_RETURN_OK == modSysClock_timeout (&delay_get_temp, 1500, SYSCLOCK_GET_TIME_MS_1)) //(HAL_GetTick() - delay_set_zero) >= 1000)
        {
            int8_t tI;
            uint8_t tF;

            if (FUNCTION_RETURN_OK == mod1Wire_Master_getTemperature (0, &tI, &tF))
            {
                if (127 != tI)
                {
                    usRegInputBuf [INPUT_REG_TEMPERATURE_1] = tI * 10 + tF;
                }
                else
                {
                    usRegInputBuf [INPUT_REG_TEMPERATURE_1] = 10000;
                }
            }
            else
            {
                usRegInputBuf [INPUT_REG_TEMPERATURE_1] = 10000;
            }
        }

        if (FUNCTION_RETURN_OK == modSysClock_timeout (&delay_get_in, 10, SYSCLOCK_GET_TIME_MS_1)) //(HAL_GetTick() - delay_set_zero) >= 1000)
        {
            //sys_595_out_set (); //not needed

            uint32_t i, tmp16 = 0;
            for (i = 0; i < 16; i++)
            {
                tmp16 |= sys_595_in_get (i) << i;
                tmp16 = tmp16 << 1;
            }
            usRegInputBuf [INPUT_REG_DIGITAL_IN] = tmp16;

            // ADC conversion completed
            tmpS32 = ADC1->DR;

            //sum += (tmpS32 * tmpS32);
            //sum += (tmpS32);

            //if (++cntC >= 16)

            //cntC = 0;
            //sum = sum >> 8;
            // tmpS32 = sqrt (sum / 128); // / 256);
            //usRegInputBuf [INPUT_REG_U_BAT] = tmpS32;
            //tmpS32 =  (sum / 16); // / 256);

            //sum = 0;
            tmpS32 = filter_median (&ADC_filter_buf[0], MEDIAN_FILTER_SIZE, tmpS32);
            tmpS32 = (tmpS32 * 3300) / 4096;

            usRegInputBuf [INPUT_REG_ADC_RAW] = tmpS32;

            //tmpS32 = tmpS32 * 1200;
            //tmpS32 = tmpS32 / ADC_vfefint;
//                if (0 != v_flag)
//                {
            //tmpS32 =  v_zero - tmpS32;
            tmpS32 = (tmpS32 * 10000) / 6666;
            usRegInputBuf [INPUT_REG_ADC_V] = tmpS32;
//                }
//                else
//                {
//                    v_zero = tmpS32; //all ready V
//                }
            //}
        }

  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL3;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure the Systick interrupt time
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler */
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */

/**
  * @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
