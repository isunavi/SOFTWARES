/**
 * @file
 * @author  Ht3h5793
 * @date
 * @version V0.5.1
 * @brief
 */

#ifndef BOARD_H
#define	BOARD_H                     20160603

#define BOARD_RS485_STTM            1
#define STM32                       1 //for debug

/**
 *  Раздел для "include"
 */
#include "stm32f0xx_hal.h"



/**
 *  Раздел для "define"
 */

#define systime_t                   uint32_t
#define TIC_PERIOD                  (1UL) //us
#define BOOL_T                      uint8_t
#define bool_t                      uint8_t

#define msg_t                   uint8_t
#define FUNCTION_RETURN_OK      0 //- успешная отработка
#define FUNCTION_RETURN_ERROR   1
#define FUNCTION_RETURN_FAIL    1
#define TRUE                    1
#define FALSE                   0


#define NEED_CRC32_CCITT        1
#define NEED_CRC16              1
#define NEED_CRC8               1
#define NEED_CRC8_DS            1

//#define __NOP()
#define __DI()                  __disable_irq() // do { #asm("cli") } while(0) // Global interrupt disable
#define __EI()                  __enable_irq() //do { #asm("sei") } while(0) // Global interrupt enable
#define __clrwdt()              do { IWDG->KR = 0x0000AAAA; } while (0) // перезагрузка вачдоге

#define USART1_DIR_GPIO_Port    GPIOB
#define USART1_DIR_Pin          GPIO_PIN_1
#define GPIO_USART1_DIR_L       USART1_DIR_GPIO_Port->BSRR = (uint32_t)USART1_DIR_Pin << 16U
#define GPIO_USART1_DIR_H       USART1_DIR_GPIO_Port->BSRR = USART1_DIR_Pin
//for test
#define GPIO_USART1_DIR_INV       USART1_DIR_GPIO_Port->ODR ^= USART1_DIR_Pin


#define GPIO_POWER_ON_Port      GPIOA
#define GPIO_POWER_ON_Pin       GPIO_PIN_0
#define GPIO_POWER_ON_L         GPIO_POWER_ON_Port->BSRR = (uint32_t)GPIO_POWER_ON_Pin << 16U
#define GPIO_POWER_ON_H         GPIO_POWER_ON_Port->BSRR = GPIO_POWER_ON_Pin


// inline static int _iDisGetPrimask(void)
// {
//     int result;
//     __ASM volatile ("MRS %0, primask" : "=r" (result) );
//     __ASM volatile ("cpsid i" : : : "memory");
//     return result;
// }

// inline static int _iSetPrimask(int priMask)
// {
//     __ASM volatile ("MSR primask, %0" : : "r" (priMask) : "memory");
//     return 0;
// }

// #define ATOMIC_BLOCK_RESTORATE \
//      for(int mask = _iDisGetPrimask(), flag = 1;\
//          flag;\
//          flag = _iSetPrimask(mask))

//#define ENTER_CRITICAL_SECTION()  //do {uint32_t sreg_temp = get_interrupt_state(); __disable_irq()
//#define LEAVE_CRITICAL_SECTION()  //set_interrupt_state(sreg_temp);} while (0)
#define ATOMIC_BLOCK_RESTORATE \
     for (uint32_t mask = get_interrupt_state(), flag = 1;\
         flag;\
         flag = set_interrupt_state(mask))

#define __enter_critical()      {uint32_t flag; flag = __get_PRIMASK(); //???? где __DI();
#define __exit_critical()       __set_PRIMASK(flag);}



//----------------- I2C --------------------------------------------------------
#define HAL_I2C1            1
#if (1 == HAL_I2C1)
    #define GPIO_I2C1           GPIOA
    #define GPIO_Pin_SDA1       GPIO_PIN_10 // (1 << 5)
    #define GPIO_Pin_SCL1       GPIO_PIN_9 // (1 << 4)

    #define SCL1_L              GPIO_I2C1->BSRR = (uint32_t)GPIO_Pin_SCL1 << 16U //I2C_GPIO->ODR &= ~GPIO_Pin_SCL
    #define SCL1_H              GPIO_I2C1->BSRR = GPIO_Pin_SCL1 //I2C_GPIO->ODR |=  GPIO_Pin_SCL

    #define SDA1_L              GPIO_I2C1->BSRR = (uint32_t)GPIO_Pin_SDA1 << 16U
    #define SDA1_H              GPIO_I2C1->BSRR = GPIO_Pin_SDA1

    #define SCL1_IN             (GPIO_I2C1->IDR & GPIO_Pin_SCL1)
    #define SDA1_IN             (GPIO_I2C1->IDR & GPIO_Pin_SDA1)

    // GPIO_Init(I2C_GPIO, (GPIO_Pin_TypeDef)GPIO_Pin_SDA, GPIO_MODE_OUT_OD_HIZ_FAST); \
    // GPIO_Init(I2C_GPIO, (GPIO_Pin_TypeDef)GPIO_Pin_SCL, GPIO_MODE_OUT_OD_HIZ_FAST);
    #define I2C1_INIT        \
        RCC->AHBENR |= (RCC_AHBENR_GPIOAEN); \
        GPIO_I2C1->MODER &=  ~(GPIO_MODER_MODER10 | GPIO_MODER_MODER9); \
        GPIO_I2C1->MODER |=  (GPIO_MODER_MODER10_0 | GPIO_MODER_MODER9_0); \
        GPIO_I2C1->OTYPER |=  (GPIO_OTYPER_OT_10 | GPIO_OTYPER_OT_9);

    #define I2C1_DEINIT
    #define I2C1_DELAY          HAL_Delay (1) //do {__NOP(); __NOP(); __NOP(); __NOP(); } while (0) // HAL_Delay (1)

    #define I2C_LM75            1
    #define I2C_LM75_ADDRESS        (0x9E >> 1) //(0x90 | ((0x7 << 1) & 0x0E))

    #define halI2C_transmit(a,b,c,d)        soft_halI2C_transmit(a,b,c,d)
    #define halI2C_receive(a,b,c,d)         soft_halI2C_receive(a,b,c,d)
    #define halI2C_init()                   soft_halI2C_init()

#endif
//---------------- end I2C -----------------------------------------------------




// EEPROM ----------------------------------------------------------------------
//#define FLASH_PAGE_SIZE         ((uint32_t)0x00000400)   /* FLASH Page Size */
#define FLASH_USER_START_ADDR   ((uint32_t)0x08003C00)  //0x08002000 /* Start @ of user Flash area */
#define DATA_TO_PROG            ((uint32_t)12) //0xAA55CC33)   /* 32-bits value to be programmed */


// LEDS ------------------------------------------------------------------------

// светодиод ошибки
#define ERROR_LED_ON			LED_RED_ON
#define ERROR_LED_OFF			LED_RED_OFF

// флаг выполнения функции декодирования
#define DECODE_TEST_HIGH		LED_BLUE_ON
#define DECODE_TEST_LOW			LED_BLUE_OFF


#define USART1_LED_INV

// 1Wire
#define OW_PORT                 GPIOA		//Указать порт ow
#define OW_PIN                  GPIO_PIN_1	//Указать номер пина ow

#define MOD_WIRE_MASTER         1
#define MOD_WIRE_SLAVES_MAX     1

//#define modSysClock_getTime     HAL_GetTick
#define MOD_WIRE_TYPE_DS18B20   1



#endif	/* BOARD_H */
