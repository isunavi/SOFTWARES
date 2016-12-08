#include "mod1Wire.h"
#include "board.h"
//#include "_crc.h"
#include "../pt-1.4/pt.h"
#include "modSysClock.h"


#if MOD_WIRE_MASTER

/** �������� ��������� ������ ��� ������� ������������ ���������� (������) */
typedef struct {
    uint8_t     ROM [8];   // "��������" ����� ������
    char        sign;      // ���� �����������
    int8_t      tempInt;   // �����
    uint8_t     tempFract; // �������
    //float     tempC;
    uint8_t     status;
} modWire_slave_data_t;


typedef struct _modWire_master_s {
    // all
    modWire_slave_data_t    wireS [MOD_WIRE_SLAVES_MAX];
    uint32_t    timerA; // ������ ��� ���������
    uint32_t    timerB; // ������ ��� ���������
    uint8_t     tmp8;
    uint8_t     tmp8M [8];
    uint8_t     tMode; // ���� ������?
    
    // for search
    uint8_t     crc8; // ���������� ��� ��c���� ����������� ����� 
#if (1 < MOD_WIRE_SLAVES_MAX)    
    uint8_t     numFound; // ����� ��������� ���������
    uint8_t     IDBit; // ������ ����������� ��� � ������� ������������������ ������.
    uint8_t     cmpIDBit; // ���������� id_bit.
    uint8_t     lastDeviceFlag; // ����, �����������, ��� � ���������� ������ ������� ��������� ����������
    uint8_t     numBit; // ����� �������� ������������ ���� ROM (1 � 64)
    uint8_t     lastCollision;
    uint8_t     currentCollision;
    uint8_t     currentSelection; // ����������� ������, ������ ��� ������� ���� �� ��������� ������
#endif

    // temp
    uint32_t    delayRef; // �������� ���������� � ����������
    uint8_t     tmp_L, tmp_H;
    uint8_t     in, jm; // ��� getTemp

    // ������ �������
    //uint32_t erAttempt;
    //uint32_t erSearch;
    //uint32_t erSearchAptt;
    uint32_t    errors;

    // threats
    struct pt   search_pt; // threat
    struct pt   term_pt;

} modWire_master_s;


modWire_master_s m1Wire;


//-----------------------------------------------------------------------------
#if BOARD_RS485_STTM
static const uint8_t crc8DS_table[256] = {
    0x00, 0x5E, 0xBC, 0xE2, 0x61, 0x3F, 0xDD, 0x83, 
    0xC2, 0x9C, 0x7E, 0x20, 0xA3, 0xFD, 0x1F, 0x41, 
    0x9D, 0xC3, 0x21, 0x7F, 0xFC, 0xA2, 0x40, 0x1E, 
    0x5F, 0x01, 0xE3, 0xBD, 0x3E, 0x60, 0x82, 0xDC, 
    0x23, 0x7D, 0x9F, 0xC1, 0x42, 0x1C, 0xFE, 0xA0, 
    0xE1, 0xBF, 0x5D, 0x03, 0x80, 0xDE, 0x3C, 0x62, 
    0xBE, 0xE0, 0x02, 0x5C, 0xDF, 0x81, 0x63, 0x3D, 
    0x7C, 0x22, 0xC0, 0x9E, 0x1D, 0x43, 0xA1, 0xFF, 
    0x46, 0x18, 0xFA, 0xA4, 0x27, 0x79, 0x9B, 0xC5, 
    0x84, 0xDA, 0x38, 0x66, 0xE5, 0xBB, 0x59, 0x07, 
    0xDB, 0x85, 0x67, 0x39, 0xBA, 0xE4, 0x06, 0x58, 
    0x19, 0x47, 0xA5, 0xFB, 0x78, 0x26, 0xC4, 0x9A, 
    0x65, 0x3B, 0xD9, 0x87, 0x04, 0x5A, 0xB8, 0xE6, 
    0xA7, 0xF9, 0x1B, 0x45, 0xC6, 0x98, 0x7A, 0x24, 
    0xF8, 0xA6, 0x44, 0x1A, 0x99, 0xC7, 0x25, 0x7B, 
    0x3A, 0x64, 0x86, 0xD8, 0x5B, 0x05, 0xE7, 0xB9, 
    0x8C, 0xD2, 0x30, 0x6E, 0xED, 0xB3, 0x51, 0x0F, 
    0x4E, 0x10, 0xF2, 0xAC, 0x2F, 0x71, 0x93, 0xCD, 
    0x11, 0x4F, 0xAD, 0xF3, 0x70, 0x2E, 0xCC, 0x92, 
    0xD3, 0x8D, 0x6F, 0x31, 0xB2, 0xEC, 0x0E, 0x50, 
    0xAF, 0xF1, 0x13, 0x4D, 0xCE, 0x90, 0x72, 0x2C, 
    0x6D, 0x33, 0xD1, 0x8F, 0x0C, 0x52, 0xB0, 0xEE, 
    0x32, 0x6C, 0x8E, 0xD0, 0x53, 0x0D, 0xEF, 0xB1, 
    0xF0, 0xAE, 0x4C, 0x12, 0x91, 0xCF, 0x2D, 0x73, 
    0xCA, 0x94, 0x76, 0x28, 0xAB, 0xF5, 0x17, 0x49, 
    0x08, 0x56, 0xB4, 0xEA, 0x69, 0x37, 0xD5, 0x8B, 
    0x57, 0x09, 0xEB, 0xB5, 0x36, 0x68, 0x8A, 0xD4, 
    0x95, 0xCB, 0x29, 0x77, 0xF4, 0xAA, 0x48, 0x16, 
    0xE9, 0xB7, 0x55, 0x0B, 0x88, 0xD6, 0x34, 0x6A, 
    0x2B, 0x75, 0x97, 0xC9, 0x4A, 0x14, 0xF6, 0xA8, 
    0x74, 0x2A, 0xC8, 0x96, 0x15, 0x4B, 0xA9, 0xF7, 
    0xB6, 0xE8, 0x0A, 0x54, 0xD7, 0x89, 0x6B, 0x35
};


void crc8_DS_s (uint8_t *crc, uint8_t *inData)
{
    *crc = crc8DS_table[*inData ^ *crc];
}


#ifndef OW_PORT 
#error "not recognized PORT!"
#endif

#ifndef OW_PIN
#error "not recognized PIN!"
#endif


#define OW_L                do { OW_PORT->BSRR = (uint32_t)OW_PIN << 16U; } while (0) 
#define OW_H                do { OW_PORT->BSRR = OW_PIN; } while (0) 
#define OW_I                (OW_PORT->IDR & OW_PIN)


void OW_init (void)
{
    //���������� ������ ����������� ������ ��� ��������
    //clock source set to the internal instruction cycle
//     T2CON = 0x0000;           //Stops the Timer1 and reset control reg.
//     TMR2 = 0x0000;            //Clear contents of the timer register
//     T2CONbits.TCKPS = 0x0; // div 1
//     PR2 = 0xFFFF;
//     T2CONbits.TCS = 0; //Fcy
//     T2CONbits.TON = 1; //Start Timer1 with prescaler settings at 1:1 and 
//     
    /*
    RCC->APB2ENR |= (RCC_APB2ENR_TIM17EN);
    TIM17->CR1 = 0;
    TIM17->CR1 |= TIM_CR1_ARPE;//������� ����� ��������������� ������ �������� ����������������
    TIM17->PSC = 48 -1;
    TIM17->ARR = 0xFFFF;
    TIM17->CR1 |= TIM_CR1_CEN; //��������� ������!
    */
    
//#if OW_PORT == PORTA
    RCC->AHBENR |= (RCC_AHBENR_GPIOAEN); //TODO! add check condition
//#endif
//#if OW_PORT == PORTB
    //RCC->AHBENR |= (RCC_AHBENR_GPIOBEN);
//#endif
    OW_PORT->MODER &=  ~GPIO_MODER_MODER1; //Out
    OW_PORT->MODER |=  GPIO_MODER_MODER1_0;
    OW_PORT->OTYPER |=  GPIO_OTYPER_OT_1; //OD
    //TODO OSPEEDER PUPDR
}

//40.000.000MHz
/*
void OW_delay_us (volatile uint16_t us)
{
    TIM17->CNT = 0;
    while (TIM17->CNT < us){};
}
*/
#include "debug.h"
#define OW_delay_us(a) _delay_us(a)

//40MHz ~= 0.025 us 4 - delay for function entering/execution
#define OW_MULTIPLER    10000UL
#define SYS_MHZ         (10000UL / 48000000UL)
#define OW_TIC          1250 //250
#define OW_TIC_CORRECT  1//4 //��������� ��-�� ������� �������

#define OW_TIC_4        4//((4 * 10000UL) / OW_TIC - OW_TIC_CORRECT) //160) //4us / 0.025 = 160
#define OW_TIC_75       75//((75 * 10000UL) / OW_TIC - OW_TIC_CORRECT) //3000
   
#define OW_TIC_2        2//((2 * 10000UL) / OW_TIC - OW_TIC_CORRECT) //80
#define OW_TIC_7        7//((7 * 10000UL) / OW_TIC - OW_TIC_CORRECT) //280
#define OW_TIC_46       46//((46 * 10000UL) / OW_TIC - OW_TIC_CORRECT) //1840

#define OW_TIC_50       50//((50 * 10000UL) / OW_TIC - OW_TIC_CORRECT) //2000
#define OW_TIC_450      450//((450 * 10000UL) / OW_TIC - OW_TIC_CORRECT) //18000
#define OW_TIC_500      500//((500 * 10000UL) / OW_TIC - OW_TIC_CORRECT) //20000


uint8_t OW_itt = 0;
msg_t mod1Wire_resetWire (void) //����� ����
{
    uint8_t tmp_bit;

    if (0 == OW_itt)
    {
        OW_itt = 1;
        return FUNCTION_RETURN_ERROR;
    }
    else
    {
        OW_itt = 1;
        OW_H;
        OW_delay_us (OW_TIC_4); //��-�� �������������� �� ����
        if (0 == OW_I)              //��������� ����� �� ���������� ���������
            return FUNCTION_RETURN_ERROR;
        OW_L;					    //�������� ���� � �����
        OW_delay_us (OW_TIC_500);     //����� 480..960 ��� ����� � ������, ��� ��������� ����������
        __DI(); //ATOMIC_BLOCK_FORSEON (
            OW_H;                   //��������� ����
            // ����� ����������� �������������� ���������, ����� ������� ������� ������� �� ����
            OW_delay_us (OW_TIC_50);  //����� 50 �����������
            tmp_bit = OW_I;         //��������� ����
        __EI(); //)
        OW_delay_us (OW_TIC_450);     //��������� ��������� ������������� 
        if (0 != tmp_bit) 
            return FUNCTION_RETURN_ERROR; //������ �� ��������� 
        else
            return FUNCTION_RETURN_OK; //������ ��������� 
    }
}


msg_t mod1Wire_xBit (uint8_t wrBit, uint8_t *rdBit) // �����\�������� ����
{
    uint8_t val;

    if (0 == OW_itt)
    {
        OW_itt = 1;
        return FUNCTION_RETURN_ERROR;
    }
    else
    {
        OW_itt = 1;
    	__DI(); //ATOMIC_BLOCK_FORSEON (
        if (0 != wrBit)
        {
            // ��� ��� �� ������
            OW_L;		//�������� ���� � �����
            OW_delay_us (OW_TIC_2);   //������������ ������� ������, ������� 1 ���
            OW_H;			        //��������� ����	 
            OW_delay_us (OW_TIC_7);   // ����� �� ������� �������������, ����� �� ����� 15 ���
            val = OW_I;		        //��������� ����
            OW_delay_us (OW_TIC_46);  // �������� �� ���������� ����-�����, ������� 60 ��� � ������ ������� ������
            if (0 != val)
    	        *rdBit = 0xFF;
            else
                *rdBit = 0x00;
        }
        else 
        {
            OW_L;    //�������� ���� � �����
            OW_delay_us (OW_TIC_75); //delay_us(90);   // ������ ������� �� ���� �������� (�� ����� 60 ���, �� ����� 120 ���)
            OW_H;
            OW_delay_us (OW_TIC_4); //delay_us(5);    // ����� �������������� �������� ������� �� ���� + 1 ��� (�������)
            *rdBit = 0x00;
        }
        __EI();
    }
    return FUNCTION_RETURN_OK;
}


msg_t mod1Wire_xByte (uint8_t wrByte, uint8_t *rdByte) // ���������� ������� ����� ������
{
    uint8_t  i, tmp_bit, tmp8;

    tmp8 = 0;
    for (i = 0; i < 8; i++) // ������ "�����"
    {
        if (wrByte & 0x01)
        {
            mod1Wire_xBit (1, &tmp_bit);
        }
        else
        {
            mod1Wire_xBit (0, &tmp_bit);
        }
        wrByte = wrByte >> 1;
        tmp8 = tmp8 >> 1;
        if (0 != tmp_bit)
            tmp8 |= 0x80;
    }
    *rdByte = tmp8;
    return FUNCTION_RETURN_OK;
}

#endif //BOARD_RS485_DTERM



#if BOARD_STM32EV103


//#include <delay.h>
//#define OW_PORT             //GPIOA		//������� ���� ow
//#define OW_N_PIN            7		//������� ����� ���� ow

    #define Pin_IN_TERMODATA            (1 << 1) // PORTC
    #define OUT_TERMODATA_L             do { GPIOE->ODR &= ~Pin_IN_TERMODATA; } while (0) // PORTC
    #define OUT_TERMODATA_H             do { GPIOE->ODR |= Pin_IN_TERMODATA; } while (0) // PORTC
    #define IN_TERMODATA                (GPIOE->IDR & Pin_IN_TERMODATA)

//#define OW_PIN_MASK         (1 << OW_N_PIN)
#define OW_L                OUT_TERMODATA_L //DDRD |=  OW_PIN_MASK; } while (0)	//�������� ���� � �����
#define OW_H                OUT_TERMODATA_H //do { DDRD &= ~OW_PIN_MASK; } while (0)		//��������� ����
#define OW_I                IN_TERMODATA

/*
#include "halLCD.h"
#include "modPaint_local.h"
#include "modPaint.h"
#include "xprintf.h"
static char __str[LCD_H / 6];
extern void halLCD_layer_copy (uint8_t layerA, uint8_t layerB);
*/

void OW_init (void)
{
    //���������� ������ ����������� ������ ��� ��������
    //clock source set to the internal instruction cycle
//     T2CON = 0x0000;           //Stops the Timer1 and reset control reg.
//     TMR2 = 0x0000;            //Clear contents of the timer register
//     T2CONbits.TCKPS = 0x0; // div 1
//     PR2 = 0xFFFF;
//     T2CONbits.TCS = 0; //Fcy
//     T2CONbits.TON = 1; //Start Timer1 with prescaler settings at 1:1 and 
//     
    
    RCC->APB1ENR |= (RCC_APB1ENR_TIM2EN);
    TIM2->CR1 = 0;
    TIM2->CR1 |= TIM_CR1_ARPE;//������� ����� ��������������� ������ �������� ����������������
    TIM2->PSC = 1 -1;
    TIM2->ARR = 0xFFFF;
    TIM2->CR1 |= TIM_CR1_CEN; //��������� ������!
    
    RCC->APB2ENR |= (RCC_APB2ENR_IOPEEN);
    GPIOE->CRL |=  (GPIO_CRL_MODE1);
    GPIOE->CRL &= ~(GPIO_CRL_CNF1_1);
    GPIOE->CRL |= (GPIO_CRL_CNF1_0);
}

//40.000.000MHz
void OW_delay_us (volatile uint16_t us)
{
    TIM2->CNT = 0;
    while (TIM2->CNT < us){};
}


//40MHz ~= 0.025 us 4 - delay for function entering/execution
#define OW_TIC          14 //25
#define OW_TIC_CORRECT  4//

#define OW_TIC_4        ((4 * 1000UL) / OW_TIC - 4) //160) //4us / 0.025 = 160
#define OW_TIC_75       ((75 * 1000UL) / OW_TIC - 4) //3000
   
#define OW_TIC_2        ((2 * 1000UL) / OW_TIC - 4) //80
#define OW_TIC_7        ((7 * 1000UL) / OW_TIC - 4) //280
#define OW_TIC_46       ((46 * 1000UL) / OW_TIC - 4) //1840

#define OW_TIC_50       ((50 * 1000UL) / OW_TIC - 4) //2000
#define OW_TIC_450      ((450 * 1000UL) / OW_TIC - 4) //18000
#define OW_TIC_500      ((500 * 1000UL) / OW_TIC - 4) //20000




uint8_t OW_itt = 0;
msg_t mod1Wire_resetWire (void)
{ //����� ����
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t tmp_bit;

    if (0 == OW_itt)
    {
        OW_itt = 1;
        return FUNCTION_RETURN_ERROR;
    }
    else
    {
        OW_itt = 1;
        OW_H;
        OW_delay_us (OW_TIC_4); //��-�� �������������� �� ����
        if (0 == OW_I)              //��������� ����� �� ���������� ���������
            return respond;
        OW_L;					    //�������� ���� � �����
        OW_delay_us (OW_TIC_500);     //����� 480..960 ��� ����� � ������, ��� ��������� ����������
        __DI(); //ATOMIC_BLOCK_FORSEON (
            OW_H;                   //��������� ����
            // ����� ����������� �������������� ���������, ����� ������� ������� ������� �� ����
            OW_delay_us (OW_TIC_50);  //����� 50 �����������
            tmp_bit = OW_I;         //��������� ����
        __EI(); //)
        OW_delay_us (OW_TIC_450);     //��������� ��������� ������������� 
    
        if (0 != tmp_bit) 
            return FUNCTION_RETURN_ERROR; //������ �� ��������� 
        else
            return FUNCTION_RETURN_OK; //������ ��������� 
    }
}


msg_t mod1Wire_xBit (uint8_t wrBit, uint8_t *rdBit)
{ // �����\�������� ����
//    uint8_t  tmpBit;
    uint8_t val;

    if (0 == OW_itt)
    {
        OW_itt = 1;
        return FUNCTION_RETURN_ERROR;
    }
    else
    {
        OW_itt = 1;
    	__DI(); //ATOMIC_BLOCK_FORSEON (
        if (0 != wrBit)
        {
            // ��� ��� �� ������
            OW_L;		//�������� ���� � �����
            OW_delay_us (OW_TIC_2);   //������������ ������� ������, ������� 1 ���
            OW_H;			        //��������� ����	 
            OW_delay_us (OW_TIC_7);   // ����� �� ������� �������������, ����� �� ����� 15 ���
            val = OW_I;		        //��������� ����
            OW_delay_us (OW_TIC_46);  // �������� �� ���������� ����-�����, ������� 60 ��� � ������ ������� ������
            if (0 != val)
    	        *rdBit = 0xFF;
            else
                *rdBit = 0x00;
        }
        else 
        {
            OW_L;    //�������� ���� � �����
            OW_delay_us (OW_TIC_75); //delay_us(90);   // ������ ������� �� ���� �������� (�� ����� 60 ���, �� ����� 120 ���)
            OW_H;
            OW_delay_us (OW_TIC_4); //delay_us(5);    // ����� �������������� �������� ������� �� ���� + 1 ��� (�������)
            *rdBit = 0x00;
        }
        __EI();
    }
    return FUNCTION_RETURN_OK;
}


msg_t mod1Wire_xByte (uint8_t wrByte, uint8_t *rdByte)
{ // ���������� ������� ����� ������
//    msg_t    respond = FUNCTION_RETURN_ERROR;
    uint8_t  i, tmp_bit, tmp8;

    tmp8 = 0;
    for (i = 0; i < 8; i++)
    { // ������ "�����"
        if (wrByte & 0x01)
        {
            mod1Wire_xBit (1, &tmp_bit);
        }
        else
        {
            mod1Wire_xBit (0, &tmp_bit);
        }
        wrByte = wrByte >> 1;
        tmp8 = tmp8 >> 1;
        if (0 != tmp_bit)
            tmp8 |= 0x80;
    }
    *rdByte = tmp8;
    return FUNCTION_RETURN_OK;
}

#endif //BOARD_STM32EV103


//------------------------------------------------------------------------------
#if BOARD_PULT_VAGON 

static const uint8_t crc8DS_table[256] = {
    0x00, 0x5E, 0xBC, 0xE2, 0x61, 0x3F, 0xDD, 0x83, 
    0xC2, 0x9C, 0x7E, 0x20, 0xA3, 0xFD, 0x1F, 0x41, 
    0x9D, 0xC3, 0x21, 0x7F, 0xFC, 0xA2, 0x40, 0x1E, 
    0x5F, 0x01, 0xE3, 0xBD, 0x3E, 0x60, 0x82, 0xDC, 
    0x23, 0x7D, 0x9F, 0xC1, 0x42, 0x1C, 0xFE, 0xA0, 
    0xE1, 0xBF, 0x5D, 0x03, 0x80, 0xDE, 0x3C, 0x62, 
    0xBE, 0xE0, 0x02, 0x5C, 0xDF, 0x81, 0x63, 0x3D, 
    0x7C, 0x22, 0xC0, 0x9E, 0x1D, 0x43, 0xA1, 0xFF, 
    0x46, 0x18, 0xFA, 0xA4, 0x27, 0x79, 0x9B, 0xC5, 
    0x84, 0xDA, 0x38, 0x66, 0xE5, 0xBB, 0x59, 0x07, 
    0xDB, 0x85, 0x67, 0x39, 0xBA, 0xE4, 0x06, 0x58, 
    0x19, 0x47, 0xA5, 0xFB, 0x78, 0x26, 0xC4, 0x9A, 
    0x65, 0x3B, 0xD9, 0x87, 0x04, 0x5A, 0xB8, 0xE6, 
    0xA7, 0xF9, 0x1B, 0x45, 0xC6, 0x98, 0x7A, 0x24, 
    0xF8, 0xA6, 0x44, 0x1A, 0x99, 0xC7, 0x25, 0x7B, 
    0x3A, 0x64, 0x86, 0xD8, 0x5B, 0x05, 0xE7, 0xB9, 
    0x8C, 0xD2, 0x30, 0x6E, 0xED, 0xB3, 0x51, 0x0F, 
    0x4E, 0x10, 0xF2, 0xAC, 0x2F, 0x71, 0x93, 0xCD, 
    0x11, 0x4F, 0xAD, 0xF3, 0x70, 0x2E, 0xCC, 0x92, 
    0xD3, 0x8D, 0x6F, 0x31, 0xB2, 0xEC, 0x0E, 0x50, 
    0xAF, 0xF1, 0x13, 0x4D, 0xCE, 0x90, 0x72, 0x2C, 
    0x6D, 0x33, 0xD1, 0x8F, 0x0C, 0x52, 0xB0, 0xEE, 
    0x32, 0x6C, 0x8E, 0xD0, 0x53, 0x0D, 0xEF, 0xB1, 
    0xF0, 0xAE, 0x4C, 0x12, 0x91, 0xCF, 0x2D, 0x73, 
    0xCA, 0x94, 0x76, 0x28, 0xAB, 0xF5, 0x17, 0x49, 
    0x08, 0x56, 0xB4, 0xEA, 0x69, 0x37, 0xD5, 0x8B, 
    0x57, 0x09, 0xEB, 0xB5, 0x36, 0x68, 0x8A, 0xD4, 
    0x95, 0xCB, 0x29, 0x77, 0xF4, 0xAA, 0x48, 0x16, 
    0xE9, 0xB7, 0x55, 0x0B, 0x88, 0xD6, 0x34, 0x6A, 
    0x2B, 0x75, 0x97, 0xC9, 0x4A, 0x14, 0xF6, 0xA8, 
    0x74, 0x2A, 0xC8, 0x96, 0x15, 0x4B, 0xA9, 0xF7, 
    0xB6, 0xE8, 0x0A, 0x54, 0xD7, 0x89, 0x6B, 0x35
};


void crc8_DS_s (uint8_t *crc, uint8_t *inData)
{
    *crc = crc8DS_table[*inData ^ *crc];
}


//#include <delay.h>
//#define OW_PORT             //GPIOA		//������� ���� ow
//#define OW_N_PIN            7		//������� ����� ���� ow

    #define Pin_IN_TERMODATA            (1 << 1) // PORTC
    #define OUT_TERMODATA_L             do { LATCbits.LATC1 = 0; TRISCbits.TRISC1 = 0; } while (0) // PORTC
    #define OUT_TERMODATA_H             do { LATCbits.LATC1 = 1; TRISCbits.TRISC1 = 1; } while (0) // PORTC
    #define IN_TERMODATA                (PORTCbits.RC1)

//#define OW_PIN_MASK         (1 << OW_N_PIN)
#define OW_L                OUT_TERMODATA_L //DDRD |=  OW_PIN_MASK; } while (0)	//�������� ���� � �����
#define OW_H                OUT_TERMODATA_H //do { DDRD &= ~OW_PIN_MASK; } while (0)		//��������� ����
#define OW_I                IN_TERMODATA


/*
#include "halLCD.h"
#include "modPaint_local.h"
#include "modPaint.h"
#include "xprintf.h"
static char __str[LCD_H / 6];
extern void halLCD_layer_copy (uint8_t layerA, uint8_t layerB);
*/

void OW_init (void)
{
    //���������� ������ ����������� ������ ��� ��������
    //clock source set to the internal instruction cycle
    T2CON = 0x0000;           //Stops the Timer1 and reset control reg.
    TMR2 = 0x0000;            //Clear contents of the timer register
    T2CONbits.TCKPS = 0x0; // div 1
    PR2 = 0xFFFF;
    T2CONbits.TCS = 0; //Fcy
    T2CONbits.TON = 1; //Start Timer1 with prescaler settings at 1:1 and 
}

//40.000.000MHz
void _delay_us (volatile uint16_t us)
{
    TMR2 = 0;
    while (TMR2 < us){};
}

//40MHz ~= 0.025 us 4 - delay for function entering/execution
#define OW_TIC_4        ((4 * 1000UL) / 25 - 4) //160) //4us / 0.025 = 160
#define OW_TIC_75       ((75 * 1000UL) / 25 - 4) //3000
   
#define OW_TIC_2        ((2 * 1000UL) / 25 - 4) //80
#define OW_TIC_7        ((7 * 1000UL) / 25 - 4) //280
#define OW_TIC_46       ((46 * 1000UL) / 25 - 4) //1840

#define OW_TIC_50       ((50 * 1000UL) / 25 - 4) //2000
#define OW_TIC_450      ((450 * 1000UL) / 25 - 4) //18000
#define OW_TIC_500      ((500 * 1000UL) / 25 - 4) //20000


//#define ONE_WIRE_CLEAR


uint8_t OW_itt = 0;
msg_t mod1Wire_resetWire (void)
{ //����� ����
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t tmp_bit;

    if (0 == OW_itt)
    {
        OW_itt = 1;
        return FUNCTION_RETURN_ERROR;
    }
    else
    {
        OW_itt = 1;
        if (0 == OW_I)              //��������� ����� �� ���������� ���������
            return respond;
        OW_L;					    //�������� ���� � �����
        _delay_us (OW_TIC_500);     //����� 480..960 ���
        __DI(); //ATOMIC_BLOCK_FORSEON (
            OW_H;                   //��������� ����
            // ����� ����������� �������������� ���������, ����� ������� ������� ������� �� ����
            _delay_us (OW_TIC_50);  //����� 50 �����������
            tmp_bit = OW_I;         //��������� ����
        __EI(); //)
        _delay_us (OW_TIC_450);     //��������� ��������� ������������� 
    
        if (0 != tmp_bit) 
            return FUNCTION_RETURN_ERROR; //������ �� ��������� 
        else
            return FUNCTION_RETURN_OK; //������ ��������� 
    }
}


msg_t mod1Wire_xBit (uint8_t wrBit, uint8_t *rdBit)
{ // �����\�������� ����
//    uint8_t  tmpBit;
    uint8_t val;

    if (0 == OW_itt)
    {
        OW_itt = 1;
        return FUNCTION_RETURN_ERROR;
    }
    else
    {
        OW_itt = 1;
    	__DI(); //ATOMIC_BLOCK_FORSEON (
        if (0 != wrBit)
        {
            // ��� ��� �� ������
            OW_L;		//�������� ���� � �����
            _delay_us (OW_TIC_2);   //������������ ������� ������, ������� 1 ���
            OW_H;			        //��������� ����	 
            _delay_us (OW_TIC_7);   // ����� �� ������� �������������, ����� �� ����� 15 ���
            val = OW_I;		        //��������� ����
            _delay_us (OW_TIC_46);  // �������� �� ���������� ����-�����, ������� 60 ��� � ������ ������� ������
            if (0 != val)
    	        *rdBit = 0xFF;
            else
                *rdBit = 0x00;
        }
        else 
        {
            OW_L;    //�������� ���� � �����
            _delay_us (OW_TIC_75); //delay_us(90);   // ������ ������� �� ���� �������� (�� ����� 60 ���, �� ����� 120 ���)
            OW_H;
            _delay_us (OW_TIC_4); //delay_us(5);    // ����� �������������� �������� ������� �� ���� + 1 ��� (�������)
            *rdBit = 0x00;
        }
        __EI();
    }
    return FUNCTION_RETURN_OK;
}


msg_t mod1Wire_xByte (uint8_t wrByte, uint8_t *rdByte)
{ // ���������� ������� ����� ������
//    msg_t    respond = FUNCTION_RETURN_ERROR;
    uint8_t  i, tmp_bit, tmp8;

    tmp8 = 0;
    for (i = 0; i < 8; i++)
    { // ������ "�����"
        if (wrByte & 0x01)
        {
            mod1Wire_xBit (1, &tmp_bit);
        }
        else
        {
            mod1Wire_xBit (0, &tmp_bit);
        }
        wrByte = wrByte >> 1;
        tmp8 = tmp8 >> 1;
        if (0 != tmp_bit)
            tmp8 |= 0x80;
    }
    *rdByte = tmp8;
    return FUNCTION_RETURN_OK;
}

#endif //BOARD_PULT_VAGON


//------------------------------------------------------------------------------
#if BOARD_STM8L051

#include "_crc.h"

#define Pin_IN_TERMODATA            (1 << 0)
#define OUT_TERMODATA_L             do { GPIOB->ODR &= ~Pin_IN_TERMODATA; } while (0)
#define OUT_TERMODATA_H             do { GPIOB->ODR |= Pin_IN_TERMODATA; } while (0)
#define IN_TERMODATA                (GPIOB->IDR & Pin_IN_TERMODATA)
#define OW_L                        OUT_TERMODATA_L //�������� ���� � �����
#define OW_H                        OUT_TERMODATA_H //��������� ����
#define OW_I                        IN_TERMODATA

void OW_init (void)
{
    CLK->PCKENR1 |= CLK_PCKENR1_TIM2;
    #if CLK_HSI // 16.000.000MHz / 128 / 125 = 1000 
        TIM2->PSCR = 0x04; //TIM4_PRESCALER_16
    #endif
        
    #if CLK_LSI // 128.000MHz / 1 / 128 = 1000
        TIM2->PSCR = 0x07; //TIM4_PRESCALER_128
    #endif
        
    #if CLK_HSE // 4.000.000MHz / 32 / 125 = 1000
        TIM2->PSCR = 0x02; //TIM4_PRESCALER_4
    #endif
    
    GPIOB->ODR |= Pin_IN_TERMODATA;
    GPIOB->DDR |= Pin_IN_TERMODATA;
    GPIOB->CR1 &= ~Pin_IN_TERMODATA;
    GPIOB->CR2 |= Pin_IN_TERMODATA;
}

//40.000.000MHz
void OW_delay_us ( uint16_t us)
{
    TIM2->ARRH = (uint8_t)(us >> 8);
    TIM2->ARRL = (uint8_t)(us);
    TIM2->EGR |= TIM_EGR_UG; //���������� ������� ���������� ��� ������ ������ � �������� PSC � ARR
    TIM2->CR1 |= (TIM_CR1_CEN | TIM_CR1_OPM); //��������� ������ ������� ���� CEN � ������������� ����� ������ ������� ���������� ���� OPM
    while ((TIM2->CR1 & TIM_CR1_CEN) != 0) {};
}


//40MHz ~= 0.025 us 4 - delay for function entering/execution
//#define OW_TIC          1000 //25
#define OW_TIC_CORRECT  1 //

#define OW_TIC_4        (4 - OW_TIC_CORRECT) //160) //4us / 0.025 = 160
#define OW_TIC_75       (75 - OW_TIC_CORRECT) //3000
   
#define OW_TIC_2        (2 - OW_TIC_CORRECT) //80
#define OW_TIC_7        (7 - OW_TIC_CORRECT) //280
#define OW_TIC_46       (46 - OW_TIC_CORRECT) //1840

#define OW_TIC_50       (50 - OW_TIC_CORRECT) //2000
#define OW_TIC_450      (450  - OW_TIC_CORRECT) //18000
#define OW_TIC_500      (500  - OW_TIC_CORRECT) //20000


//#define ONE_WIRE_CLEAR


uint8_t OW_itt = 0;
msg_t mod1Wire_resetWire (void)
{ //����� ����
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t tmp_bit;

    if (0 == OW_itt)
    {
        OW_itt = 1;
        return FUNCTION_RETURN_ERROR;
    }
    else
    {
        OW_itt = 1;
        OW_H;
        OW_delay_us (OW_TIC_4); //��-�� �������������� �� ����
        if (0 == OW_I)              //��������� ����� �� ���������� ���������
            return respond;
        OW_L;					    //�������� ���� � �����
        OW_delay_us (OW_TIC_500);     //����� 480..960 ��� ����� � ������, ��� ��������� ����������
        __DI(); //ATOMIC_BLOCK_FORSEON (
            OW_H;                   //��������� ����
            // ����� ����������� �������������� ���������, ����� ������� ������� ������� �� ����
            OW_delay_us (OW_TIC_50);  //����� 50 �����������
            tmp_bit = OW_I;         //��������� ����
        __EI(); //)
        OW_delay_us (OW_TIC_450);     //��������� ��������� ������������� 
    
        if (0 != tmp_bit) 
            return FUNCTION_RETURN_ERROR; //������ �� ��������� 
        else
            return FUNCTION_RETURN_OK; //������ ��������� 
    }
}


msg_t mod1Wire_xBit (uint8_t wrBit, uint8_t *rdBit)
{ // �����\�������� ����
//    uint8_t  tmpBit;
    uint8_t val;

    if (0 == OW_itt)
    {
        OW_itt = 1;
        return FUNCTION_RETURN_ERROR;
    }
    else
    {
        OW_itt = 1;
    	__DI(); //ATOMIC_BLOCK_FORSEON (
        if (0 != wrBit)
        {
            // ��� ��� �� ������
            OW_L;		//�������� ���� � �����
            OW_delay_us (OW_TIC_2);   //������������ ������� ������, ������� 1 ���
            OW_H;			        //��������� ����	 
            OW_delay_us (OW_TIC_7);   // ����� �� ������� �������������, ����� �� ����� 15 ���
            val = OW_I;		        //��������� ����
            OW_delay_us (OW_TIC_46);  // �������� �� ���������� ����-�����, ������� 60 ��� � ������ ������� ������
            if (0 != val)
    	        *rdBit = 0xFF;
            else
                *rdBit = 0x00;
        }
        else 
        {
            OW_L;    //�������� ���� � �����
            OW_delay_us (OW_TIC_75); //delay_us(90);   // ������ ������� �� ���� �������� (�� ����� 60 ���, �� ����� 120 ���)
            OW_H;
            OW_delay_us (OW_TIC_4); //delay_us(5);    // ����� �������������� �������� ������� �� ���� + 1 ��� (�������)
            *rdBit = 0x00;
        }
        __EI();
    }
    return FUNCTION_RETURN_OK;
}


msg_t mod1Wire_xByte (uint8_t wrByte, uint8_t *rdByte)
{ // ���������� ������� ����� ������
//    msg_t    respond = FUNCTION_RETURN_ERROR;
    uint8_t  i, tmp_bit, tmp8;

    tmp8 = 0;
    for (i = 0; i < 8; i++)
    { // ������ "�����"
        if (wrByte & 0x01)
        {
            mod1Wire_xBit (1, &tmp_bit);
        }
        else
        {
            mod1Wire_xBit (0, &tmp_bit);
        }
        wrByte = wrByte >> 1;
        tmp8 = tmp8 >> 1;
        if (0 != tmp_bit)
            tmp8 |= 0x80;
    }
    *rdByte = tmp8;
    return FUNCTION_RETURN_OK;
}

#endif


//------------------------------------------------------------------------------
#if BOARD_STM8_ESP8266

#include "_crc.h"

#define Pin_IN_TERMODATA            (1 << 5)
#define OUT_TERMODATA_L             do { GPIOC->ODR &= ~Pin_IN_TERMODATA; } while (0)
#define OUT_TERMODATA_H             do { GPIOC->ODR |= Pin_IN_TERMODATA; } while (0)
#define IN_TERMODATA                (GPIOC->IDR & Pin_IN_TERMODATA)
#define OW_L                        OUT_TERMODATA_L //�������� ���� � �����
#define OW_H                        OUT_TERMODATA_H //��������� ����
#define OW_I                        IN_TERMODATA

void OW_init (void)
{
    #if CLK_HSI // 16.000.000MHz / 128 / 125 = 1000 
        TIM2->PSCR = 0x04; //TIM4_PRESCALER_16
    #endif
        
    #if CLK_HSE // 4.000.000MHz / 32 / 125 = 1000
        TIM2->PSCR = 0x02; //TIM4_PRESCALER_4
    #endif
    
    GPIOC->ODR |= Pin_IN_TERMODATA;
    GPIOC->DDR |= Pin_IN_TERMODATA;
    GPIOC->CR1 &= ~Pin_IN_TERMODATA;
    GPIOC->CR2 |= Pin_IN_TERMODATA;
}

//40.000.000MHz
void OW_delay_us (uint16_t us)
{
    TIM2->ARRH = (uint8_t)(us >> 8);
    TIM2->ARRL = (uint8_t)(us);
    TIM2->EGR |= TIM2_EGR_UG; //���������� ������� ���������� ��� ������ ������ � �������� PSC � ARR
    TIM2->CR1 |= (TIM2_CR1_CEN | TIM2_CR1_OPM); //��������� ������ ������� ���� CEN � ������������� ����� ������ ������� ���������� ���� OPM
    while ((TIM2->CR1 & TIM2_CR1_CEN) != 0) {};
}


//40MHz ~= 0.025 us 4 - delay for function entering/execution
//#define OW_TIC          1000 //25
#define OW_TIC_CORRECT  1 //

#define OW_TIC_4        (4 - OW_TIC_CORRECT) //160) //4us / 0.025 = 160
#define OW_TIC_75       (75 - OW_TIC_CORRECT) //3000
   
#define OW_TIC_2        (2 - OW_TIC_CORRECT) //80
#define OW_TIC_7        (7 - OW_TIC_CORRECT) //280
#define OW_TIC_46       (46 - OW_TIC_CORRECT) //1840

#define OW_TIC_50       (50 - OW_TIC_CORRECT) //2000
#define OW_TIC_450      (450  - OW_TIC_CORRECT) //18000
#define OW_TIC_500      (500  - OW_TIC_CORRECT) //20000


//#define ONE_WIRE_CLEAR


uint8_t OW_itt = 0;
msg_t mod1Wire_resetWire (void)
{ //����� ����
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t tmp_bit;

    if (0 == OW_itt)
    {
        OW_itt = 1;
        return FUNCTION_RETURN_ERROR;
    }
    else
    {
        OW_itt = 1;
        OW_H;
        OW_delay_us (OW_TIC_4); //��-�� �������������� �� ����
        if (0 == OW_I)              //��������� ����� �� ���������� ���������
            return respond;
        OW_L;					    //�������� ���� � �����
        OW_delay_us (OW_TIC_500);     //����� 480..960 ��� ����� � ������, ��� ��������� ����������
        __DI(); //ATOMIC_BLOCK_FORSEON (
            OW_H;                   //��������� ����
            // ����� ����������� �������������� ���������, ����� ������� ������� ������� �� ����
            OW_delay_us (OW_TIC_50);  //����� 50 �����������
            tmp_bit = OW_I;         //��������� ����
        __EI(); //)
        OW_delay_us (OW_TIC_450);     //��������� ��������� ������������� 
    
        if (0 != tmp_bit) 
            return FUNCTION_RETURN_ERROR; //������ �� ��������� 
        else
            return FUNCTION_RETURN_OK; //������ ��������� 
    }
}


msg_t mod1Wire_xBit (uint8_t wrBit, uint8_t *rdBit)
{ // �����\�������� ����
//    uint8_t  tmpBit;
    uint8_t val;

    if (0 == OW_itt)
    {
        OW_itt = 1;
        return FUNCTION_RETURN_ERROR;
    }
    else
    {
        OW_itt = 1;
    	__DI(); //ATOMIC_BLOCK_FORSEON (
        if (0 != wrBit)
        {
            // ��� ��� �� ������
            OW_L;		//�������� ���� � �����
            OW_delay_us (OW_TIC_2);   //������������ ������� ������, ������� 1 ���
            OW_H;			        //��������� ����	 
            OW_delay_us (OW_TIC_7);   // ����� �� ������� �������������, ����� �� ����� 15 ���
            val = OW_I;		        //��������� ����
            OW_delay_us (OW_TIC_46);  // �������� �� ���������� ����-�����, ������� 60 ��� � ������ ������� ������
            if (0 != val)
    	        *rdBit = 0xFF;
            else
                *rdBit = 0x00;
        }
        else 
        {
            OW_L;    //�������� ���� � �����
            OW_delay_us (OW_TIC_75); //delay_us(90);   // ������ ������� �� ���� �������� (�� ����� 60 ���, �� ����� 120 ���)
            OW_H;
            OW_delay_us (OW_TIC_4); //delay_us(5);    // ����� �������������� �������� ������� �� ���� + 1 ��� (�������)
            *rdBit = 0x00;
        }
        __EI();
    }
    return FUNCTION_RETURN_OK;
}


msg_t mod1Wire_xByte (uint8_t wrByte, uint8_t *rdByte)
{ // ���������� ������� ����� ������
//    msg_t    respond = FUNCTION_RETURN_ERROR;
    uint8_t  i, tmp_bit, tmp8;

    tmp8 = 0;
    for (i = 0; i < 8; i++)
    { // ������ "�����"
        if (wrByte & 0x01)
        {
            mod1Wire_xBit (1, &tmp_bit);
        }
        else
        {
            mod1Wire_xBit (0, &tmp_bit);
        }
        wrByte = wrByte >> 1;
        tmp8 = tmp8 >> 1;
        if (0 != tmp_bit)
            tmp8 |= 0x80;
    }
    *rdByte = tmp8;
    return FUNCTION_RETURN_OK;
}

#endif


//------------------------------------------------------------------------------
#if (1 < MOD_WIRE_SLAVES_MAX)
static int search(struct pt *pt)
{
//        msg_t respond = FUNCTION_RETURN_ERROR; // ��� ������� �������� ����
    
    PT_BEGIN(pt); // ��� ������� �������� ����
    if (FALSE == m1Wire.lastDeviceFlag)
    { // ��� ������� ���� ������ � �������� ���, ��� ��� �������
        //m1Wire.erSearchAptt++;
        m1Wire.currentCollision = 0;
        m1Wire.crc8 = 0; // ��������, ����������� ����� ������ �������
        // ������� �� ������ ������ ����������
        PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte(DS_COMMAND_SEARCH_ROM, &m1Wire.tmp8));
        // �������� ���� ������, �� ���� ������ ������ ��������� ���� �����
        for (m1Wire.numBit = 1; m1Wire.numBit <= 64; m1Wire.numBit++) {
            PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xBit(1, &m1Wire.IDBit)); // ������ ������ ���
            // ������ ������ ��� (���������������)
            PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xBit(1, &m1Wire.cmpIDBit));
            if ((0 != m1Wire.IDBit) && (0 != m1Wire.cmpIDBit))
            { // ��� �������, ���-�� ���������� � ����������� �����
//                    respond = FUNCTION_RETURN_ERROR; // ���� ���, �� �� ���� ������ ���, � �������� ��� �� ������ ���� ������, ��� ������ ������� �� ����� ����� ����
                break;
            }
            else if (m1Wire.IDBit != m1Wire.cmpIDBit) // ���� ���� ������, �� ���� ������
            { 
                m1Wire.currentSelection = m1Wire.IDBit;
            }
            else
            {
                // ��������, ���� ���, �� ������ ���� ����, ����� ������, ���� ���� � ��� ������...
                if (m1Wire.numBit < m1Wire.lastCollision) 
                {
                    if ((m1Wire.wireS[m1Wire.numFound].ROM[(m1Wire.numBit - 1) >> 3] & 1 << ((m1Wire.numBit - 1) & 0x07)) > 0) // ���� ������� ��� ����� 1
                    {
                        m1Wire.currentSelection = 1;

                        // ���� ����� �� ������ �����, ���������� ����� ����
                        if (m1Wire.currentCollision < m1Wire.numBit) 
                        {
                            m1Wire.currentCollision = m1Wire.numBit;
                        }
                    }
                    else
                    {
                        m1Wire.currentSelection = 0;
                    }
                }
                else
                {
                    if (m1Wire.numBit == m1Wire.lastCollision) // ���� �����, �� ���  ���� �������� ������
                    {
                        m1Wire.currentSelection = 0;
                    }
                    else
                    {
                        m1Wire.currentSelection = 1; // ���� �� ������ �����
                        if (m1Wire.currentCollision < m1Wire.numBit) // � ���������� ����� ����
                        {
                            m1Wire.currentCollision = m1Wire.numBit;
                        }
                    }                   
                }
            }
            if (m1Wire.currentSelection == 0) // ��������� ��� ����� ���� ROM ����� �� ������ rom_byte_number
            {
                m1Wire.wireS[m1Wire.numFound].ROM[(m1Wire.numBit - 1) >> 3] &= ~(1 << ((m1Wire.numBit - 1) & 0x07));
            }
            else
            {
                m1Wire.wireS[m1Wire.numFound].ROM[(m1Wire.numBit - 1) >> 3] |= 1 << ((m1Wire.numBit - 1) & 0x07);
            }
            // ���������� ��� ����������� ������
            PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xBit(m1Wire.currentSelection, &m1Wire.tmp8));
            if ((m1Wire.numBit & 0x07) == 0)
            { // ��� �� � ����� ������� CRC
                crc8_DS_s(&m1Wire.crc8, &m1Wire.wireS[m1Wire.numFound].ROM[(m1Wire.numBit - 1) >> 3]);
            }
        }
        // ���� ����� �������, �� CRC ������ ���� ����� 0
        if ((m1Wire.numBit > 64) && (m1Wire.crc8 == 0))
        { //  �������� �� ��� ����, ��� ����� �� ���� ����, ��� ���� ������� �������, �� ������ ���� 64
            m1Wire.errors++;
            if (m1Wire.currentCollision == 0) // ��� �� ���� ��������, ������ ���,
            {
                m1Wire.lastDeviceFlag = TRUE; // �� ����� ��������� ����������
            }
            else
            {
                m1Wire.numFound++;
                m1Wire.lastCollision = m1Wire.currentCollision;
            }
            
            if (MOD_WIRE_SLAVES_MAX <= m1Wire.numFound)
            {
                m1Wire.lastDeviceFlag = TRUE; // ������, ��������� �� �������
            }
//                respond = FUNCTION_RETURN_OK; // ����� �������� ��������
        }
        else
        {//if (!respond || !ROM_NO[0]) // ���� �� ����� ���������� then reset counters so next 'search' will be like a first
            m1Wire.lastDeviceFlag = FALSE; 
            m1Wire.wireS[m1Wire.numFound].status = FALSE;
//                respond = FUNCTION_RETURN_ERROR;
        }
    }
    
    PT_END(pt);
}
#endif //(1 < MOD_WIRE_SLAVES_MAX)


static int getTemp (struct pt *pt)
{
    uint16_t tmp16;
    uint8_t tmp8;
    
    PT_BEGIN(pt);
#if (1 == MOD_WIRE_TYPE_DS1821)     
    PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == mod1Wire_resetWire ()); // ����� ����
    PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == mod1Wire_xByte (DS1821_COMMAND_CONVERT_TEMP_START, &m1Wire.tmp8)); // �������� ������� ������ ������������� ��������������
    
    m1Wire.timerA = modSysClock_getTime(); // ���� ���� DS18B20 ���������� ������
    PT_WAIT_UNTIL (pt, (HAL_GetTick() - m1Wire.timerA) >= 1000);
    
    PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == mod1Wire_resetWire()); // ����� ����
    PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == mod1Wire_xByte (DS1821_COMMAND_CONVERT_TEMP_READ, &m1Wire.tmp8)); // ������ ������ �� �������
    PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == mod1Wire_xByte (0xFF, &m1Wire.tmp_L));

    if ((m1Wire.tmp_L & 0x80) == 0)
    { //��������� ���� 
        m1Wire.wireS[0].sign = '+';
    } else {
        m1Wire.wireS[0].sign = '-'; 
    }
    m1Wire.wireS[0].tempInt = m1Wire.tmp_L;
    m1Wire.wireS[0].tempFract = 0;
    m1Wire.wireS[0].status = TRUE;
#endif
    
#if MOD_WIRE_TYPE_DS18B20
    PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == mod1Wire_resetWire ()); // ����� ����
#if (1 < MOD_WIRE_SLAVES_MAX)
    PT_WAIT_THREAD(pt, search(&m1Wire.search_pt)); // �����
#else
    // ���� ���������� ����, �� ����� ������ ��� ID, ��� ������
    PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == mod1Wire_xByte (DS_COMMAND_READ_ROM, &m1Wire.tmp8));
    m1Wire.crc8 = 0x00;
    for (m1Wire.jm = 0; m1Wire.jm < 8; m1Wire.jm++)
    { // ������ �����
        PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == mod1Wire_xByte (0xFF, &m1Wire.wireS[0].ROM[m1Wire.jm]));
        crc8_DS_s(&m1Wire.crc8, &m1Wire.wireS[0].ROM[m1Wire.jm]);
    }
#endif // (1 < MOD_WIRE_SLAVES_MAX)
    // @todo ������� �������� �� ������� ����������
    PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == mod1Wire_resetWire ()); // ����� ����
    PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == mod1Wire_xByte (DS_COMMAND_SKIP_ROM, &m1Wire.tmp8)); // ������� �������� ���
    PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == mod1Wire_xByte (DS18B20_COMMAND_CONVERT_TEMP, &m1Wire.tmp8)); // �������� ������� ������ ������������� ��������������
    m1Wire.timerA = modSysClock_getTime(); // ���� ���� DS18B20 ���������� ������
    PT_WAIT_UNTIL (pt, (HAL_GetTick() - m1Wire.timerA) >= 800);
    for (m1Wire.in = 0; m1Wire.in < MOD_WIRE_SLAVES_MAX; m1Wire.in++)
    { // ������ ����������� �� ���������
#if (1 < MOD_WIRE_SLAVES_MAX)
        if (OW_DS18B20_FAMILY_CODE == m1Wire.wireS[m1Wire.in].ROM[0])
        {
            PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == mod1Wire_resetWire());
            PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == mod1Wire_xByte(DS_COMMAND_MATCH_ROM, &m1Wire.tmp8)); // �������� ����� ����������
            for (m1Wire.jm = 0; m1Wire.jm < 8; m1Wire.jm++)  // �������� � ���� 64 ���� ������
                PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte(m1Wire.wireS[m1Wire.in].ROM[m1Wire.jm], &m1Wire.tmp8));
        }
#else
        PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == mod1Wire_resetWire ()); // ����� ����
        PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == mod1Wire_xByte (DS_COMMAND_SKIP_ROM, &tmp8)); // ������� �������� ���
#endif // (1 < MOD_WIRE_SLAVES_MAX)
        PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == mod1Wire_xByte (DS18B20_COMMAND_READ_SCRATCHPAD, &m1Wire.tmp8)); // ������ ������ �� �������
        PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == mod1Wire_xByte (0xFF, &m1Wire.tmp_L));
        PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == mod1Wire_xByte (0xFF, &m1Wire.tmp_H));

        tmp16 = (uint16_t)(((uint16_t)m1Wire.tmp_H << 8) | (uint16_t)m1Wire.tmp_L); // ���������� ������� � ������ ����� ��� �������

        if ((m1Wire.tmp_H & 0x80) == 0) //��������� ������� ������
        {
            m1Wire.wireS[m1Wire.in].sign = '+';
        }
        else
        {
            tmp16 = ~tmp16 + 1;
            m1Wire.wireS[m1Wire.in].sign = '-'; 
        }
        m1Wire.tmp_H = tmp16 >> 4; //(unsigned short)((tmp_L & 0x07) << 4) | (tmp_H >> 4);
        m1Wire.wireS[m1Wire.in].tempInt = m1Wire.tmp_H;
        m1Wire.tmp_L = (uint16_t)(tmp16 & 0x0F); //����������� ������� � ����� �����
        m1Wire.tmp_L = (m1Wire.tmp_L << 1) + (m1Wire.tmp_L << 3); // �������� �� 10
        m1Wire.tmp_L = (m1Wire.tmp_L >> 4); //����� �� 16 ��� �������� �� 0.0625
        m1Wire.wireS[m1Wire.in].tempFract = m1Wire.tmp_L;
        // wireS[in].temp_Float = tmp_H * 1.0 + (tmp_L * 0.01); // ��������� ����� � ��������� ������

        m1Wire.wireS[m1Wire.in].status = TRUE;
    }
#endif // MOD_WIRE_TYPE_DS18B20
    PT_END(pt);
}


msg_t mod1Wire_Master_getTemperature (uint8_t number, int8_t *tI, uint8_t *tF)
{
    msg_t respond = FUNCTION_RETURN_ERROR;
    // @todo
#if (1 == MOD_WIRE_TYPE_DS18B20)
    //if (((OW_DS18B20_FAMILY_CODE == m1Wire.wireS[number].ROM[0]) || 
    //    (OW_DS18S20_FAMILY_CODE == m1Wire.wireS[number].ROM[0])) && (MOD_WIRE_SLAVES_MAX > number))
    //{ // && 
    if (TRUE == m1Wire.wireS[number].status)
    {
        *tI = m1Wire.wireS[number].tempInt;
        *tF = m1Wire.wireS[number].tempFract;
        m1Wire.wireS[number].status = FALSE;
        respond = FUNCTION_RETURN_OK;
    }
#elif  (1 == MOD_WIRE_TYPE_DS1821)
    if (MOD_WIRE_SLAVES_MAX > number)
    {
        *tI = m1Wire.wireS[number].tempInt;
        *tF = m1Wire.wireS[number].tempFract;
        respond = FUNCTION_RETURN_OK;
    }
#endif

    return respond;
}

  
void mod1Wire_initMaster (void)
{
    uint8_t i;

    OW_init (); // hardware init
//    ONE_WIRE_CLEAR;
    for (i = 0; i < MOD_WIRE_SLAVES_MAX; i++)
    {
        m1Wire.wireS[i].status = FALSE; // clear properties & status
        m1Wire.wireS[i].ROM[0] = 0;
    }
    PT_INIT(&m1Wire.search_pt);
    PT_INIT(&m1Wire.term_pt);
    m1Wire.delayRef = modSysClock_getTime();
    m1Wire.tMode = FALSE;
    m1Wire.errors = 0;
    m1Wire.timerB = modSysClock_getTime();
#if (MOD_WIRE_SLAVES_MAX > 1)
    m1Wire.numFound = 0;
    // for search
    m1Wire.lastDeviceFlag = FALSE;
    m1Wire.numFound = 0;
    m1Wire.lastCollision = 0;
#endif
}


msg_t mod1Wire_runMaster (void)
{
    msg_t respond = FUNCTION_RETURN_OK;
    
    getTemp (&m1Wire.term_pt); // �������� ������ � DS18B20
    // every 10 seconds restart
    if ((HAL_GetTick() - m1Wire.timerB) >= 10000)
    {
        m1Wire.timerB = HAL_GetTick();
        if (m1Wire.errors >= 10)
        {
            mod1Wire_initMaster ();
        }
    }
    return respond;
}

#endif //MOD_WIRE_MASTER
