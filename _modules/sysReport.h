/*   
 * @author  Ht3h5793
 * @brief   
    only for EWARM tested
    http://catethysis.ru/predefined-macros/
    
 */

#ifndef SYSREPORT_H
#define	SYSREPORT_H 20160321

#include "board.h"


typedef enum {
    //Errors
	NO_ERROR                        = 0x0,
	ERROR_CMD                       = 0x1,
	ERROR_PARAM                     = 0x10,
	ERROR_STATE                     = 0x20,
	ERROR_OVERHEAT                  = 0x30,
	ERROR_POWER                     = 0x40,
	ERROR_MALFUNCTION               = 0x50,
	ERROR_INTERNAL                  = 0x60,
    ERROR_UNKNOWN                   = 0x77,
    
    
    ERROR_HAL_COMMON                = 0x1000,
    ERROR_HAL_PLL_NOT_START         = 0x1001,
    ERROR_HAL_FLASH_LAT             = 0x1002,
    ERROR_HAL_USB_CLOCK             = 0x1004,
    
    ERROR_USART1_HAL                = 0x1200,
    ERROR_USART2_HAL                = 0x1201,
    ERROR_USART3_HAL                = 0x1202,
    ERROR_USART4_HAL                = 0x1203,
    
    ERROR_SPI_INIT                  = 0x1300,
    
    ERROR_FAT_READ_FILE             = 0x2000,
    ERROR_FAT_WRITE_FILE            = 0x2005,
    ERROR_FAT_MOUNT_DISK            = 0x2006,
    ERROR_FAT_LSEEK_FILE            = 0x2007,
    ERROR_FAT_CLOSE_FILE            = 0x2008,
    
    ERROR_MODIO_NOT_RESPOND         = 0x3000,
    
    ERROR_RTOS_TASK                 = 0x5000,
    ERROR_RTOS_TASK_NOT_CREATE      = 0x5010,
    ERROR_RTOS_START_SHEDULER       = 0x5020,
    ERROR_RTOS_QUEUE_CREATE         = 0x5100,
    ERROR_RTOS_QUEUE_SEND           = 0x5110,
    
    ERROR_EEPROM_ERASE,
    ERROR_EEPROM_WRITE_PROTECTION,
    ERROR_EEPROM_PROG,

    
    //Warnings
    WARNING_ADC_SHORT2GND           = 0x8001,
    WARNING_ADC_SHORT2VCC           = 0x8002,
    
    
    
} ERROR_StatusTypeDef; //ERROR_T


#ifdef	__cplusplus
extern "C" {
#endif
    

void fERROR_ACTION (ERROR_StatusTypeDef code, char *module, uint32_t pos);

void fWARNING_ACTION (uint16_t code, char *module, uint32_t pos);

#define ERROR_ACTION(a)                             fERROR_ACTION(a,__FILE__,__LINE__)
#define WARNING_ACTION(a)                           fWARNING_ACTION(a,__FILE__,__LINE__)
    

#ifdef	__cplusplus
}
#endif

#endif /** SYSREPORT_H */
