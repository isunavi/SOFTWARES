/**
 * @file    halUART.h
 * @author  Ht3h5793
 * @date    03.03.2014
 * @version V11.5.2
 * @brief   Просто  выдачей данных через и посредством
            кольцевых буферов
            
    //заменить x на число        
    halUSARTx_init ();
    halUSARTx_flush ();
    uint8_t tmp_char = 0;
    while (1)
    {
        if (FUNCTION_RETURN_OK == halUSARTx_rcvS (&tmp_char))
        {
            halUSARTx_sndS (tmp_char);
        }
    }
*/

#ifndef HALUSART_H
#define	HALUSART_H 20150810

#include "board.h"


/**
 * Код скорости обмена по UART
 * Коды скоростей не должны меняться! При необходимости их изменения (исключения, добавления, изменения порядка кодов) - 
 * требуется изменить инициализацию массива hal_UART_SpeedTune, определённого в файле hal_UART_Protected
      * Speed must be one of 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800, 9600,
     * 19200, 38400, 57600, 115200, 230400, 460800, 500000, 576000, 921600, 1000000, 1152000,
     * 1500000, 2000000, 2500000, 3000000, 3500000, 4000000
     
     */
typedef enum {
    USART_BAUD_110,
    USART_BAUD_300,
    USART_BAUD_600,
    USART_BAUD_1200,
    USART_BAUD_2400,
    USART_BAUD_4800,
    USART_BAUD_9600,
    USART_BAUD_14400,
    USART_BAUD_19200,
    USART_BAUD_28800,
    USART_BAUD_38400,
    //USART_BAUD_56000,
    USART_BAUD_57600,
    USART_BAUD_115200, /** 115200 бит/с */
    USART_BAUD_230400,
    USART_BAUD_460800,
    USART_BAUD_921600,
    UART_SPEED_TOTAL_NUMBER, /** Общее число поддерживаемых скоростей */
} USART_BAUD_type;


/**
 * Код типа контроля чётности при обмене по UART
 */
typedef enum {
    _USART_PARITY_NONE = 0, /** Нет контроля чётности */
    _USART_PARITY_EVEN, /** Контроль на чётность */
    _USART_PARITY_ODD, /** Контроль на нечётность */
} UART_PARITY_type;

/**
 * Код числа стоп бит при обмене по UART
 */
typedef enum {
    USART_STOP_BITS_NUMBER_1 = 0, /** 1 стоп бит */
    USART_STOP_BITS_NUMBER_2, /** 2 стоп бита */
    USART_STOP_BITS_NUMBER_1_5, /** 1,5 стоп бит */
} UART_STOP_BITS_NUMBER_type;

/**
 * Длина слова данных при обмене по UART
 */
typedef enum {
    USART_WORD_LENGTH_5 = 0, /** 5 бит */
    USART_WORD_LENGTH_6, /** 6 бит */
    USART_WORD_LENGTH_7, /** 7 бит */
    USART_WORD_LENGTH_8, /** 8 бит */
    
} USART_WORD_LENGHT_type;


#ifndef msg_t
//#error "dsfds"
#define msg_t   uint8_t
#endif

#ifdef	__cplusplus
extern "C" {
#endif

void      halUSART1_init (uint16_t baud);
void      halUSART1_setBaud (uint16_t baud);
void      halUSART1_flush (void); //@todo сделать очистку всех буферов!
// Отправка байта
msg_t     halUSART1_sndS (uint8_t byte);
msg_t     halUSART1_sndM (uint8_t *p_buf, uint16_t size); // Функция отправляет блок в UART
// Прием байта
msg_t     halUSART1_rcvS (uint8_t *p_byte);
msg_t     halUSART1_rcvM (uint8_t *p_buf, uint16_t size);

// Возвратщики количеств
uint32_t  halUSART1_getRecBytes (void);
uint32_t  halUSART1_getSendBytes (void);
uint32_t  halUSART1_getErrors (void);
    
//-------- USART2 -------------
//void      halUSART2_IRQ (void);
void      halUSART2_init (uint16_t baud);
void      halUSART2_setBaud (uint16_t baud);
void      halUSART2_flush (void); //@todo сделать очистку всех буферов!
// Отправка байта
msg_t     halUSART2_sndS (uint8_t byte);
msg_t     halUSART2_sndM (uint8_t *p_buf, uint16_t size); // Функция отправляет блок в UART
// Прием байта
msg_t     halUSART2_rcvS (uint8_t *p_byte);
msg_t     halUSART2_rcvM (uint8_t *p_buf, uint16_t size);

// Возвратщики количеств
uint32_t  halUSART2_getRecBytes (void);
uint32_t  halUSART2_getSendBytes (void);
uint32_t  halUSART2_getErrors (void);

//------- USART3 -------------
//void      halUSART3_IRQ (void);
void      halUSART3_init (uint16_t baud);
void      halUSART3_setBaud (uint16_t baud);
void      halUSART3_flush (void); //@todo сделать очистку всех буферов!
// Отправка байта
msg_t     halUSART3_sndS (uint8_t byte);
msg_t     halUSART3_sndM (uint8_t *p_buf, uint16_t size); // Функция отправляет блок в UART
// Прием байта
msg_t     halUSART3_rcvS (uint8_t *p_byte);
msg_t     halUSART3_rcvM (uint8_t *p_buf, uint16_t size);

// Возвратщики количеств
uint32_t  halUSART3_getRecBytes (void);
uint32_t  halUSART3_getSendBytes (void);
uint32_t  halUSART3_getErrors (void);


//------- USART4 -------------
//void      halUSART3_IRQ (void);
void      halUSART4_init (uint16_t baud);
void      halUSART4_setBaud (uint16_t baud);
void      halUSART4_flush (void); //@todo сделать очистку всех буферов!
// Отправка байта
msg_t     halUSART4_sndS (uint8_t byte);
msg_t     halUSART4_sndM (uint8_t *p_buf, uint16_t size); // Функция отправляет блок в UART
// Прием байта
msg_t     halUSART4_rcvS (uint8_t *p_byte);
msg_t     halUSART4_rcvM (uint8_t *p_buf, uint16_t size);

// Возвратщики количеств
uint32_t  halUSART4_getRecBytes (void);
uint32_t  halUSART4_getSendBytes (void);
uint32_t  halUSART4_getErrors (void);


#ifdef	__cplusplus
}
#endif

#endif	/* HALUSART_H */
