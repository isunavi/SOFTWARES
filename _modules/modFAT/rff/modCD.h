/*    
 * @file    
 * @author  Ht3h5793
 * @date    08.6.2014
 * @version V0.4.4
 * @brief   MMC/CD + FAT16 редуцированная, очень старая и весьма живучая
  
http://habrahabr.ru/post/213803/

*/

#ifndef MODCD_H
#define	MODCD_H 44 /* Revision ID */

#include "defines.h"
#include "board.h"

//Номера выводов
//     //_delay_ms(100);
//     if(FUNCTION_RETURN_OK == mmc_reset())
//     {
//         LED_RED_ON;
//         //modFPGA_LoadFPGA_StartLoad();
//         fat_init( &Pbuf[0]);
//         scan_root_base(0, &frame, &Pbuf[0]);
//         //frame = fatadr;

//         // loading bloks
//         for (uint32_t i = 0; i < 1; i++)
//         {
//             read_block( frame, &Pbuf[0]);
//             frame += 2;
//             //modFPGA_LoadFPGA_block(&Pbuf[0], 512);
//         }
//         
//         //modFPGA_LoadFPGA_comlpite();
//          // LED_STATUS_1_INV;
//     }



typedef enum file_type_ {
    FILE_TYPE_RBF,
    FILE_TYPE_WAVE,
    FILE_TYPE_BMP,
    FILE_TYPE_TXT,
}file_type;



#ifdef	__cplusplus
extern "C" {
#endif

msg_t     modMMC_init(void); //Инициализация карты MMC | SD

msg_t     modMMC_read_block(uint32_t adr, uint8_t *buf); // чтение блока 512 байт

msg_t     modMMC_write_block(uint32_t adr, uint8_t *buf); // запись блока 512 байт

////////////////////////////FAT 16//////////////////////////////
msg_t     modMMC_fat16_init(uint8_t *); // инициализация файловой системы

// поиск файла
msg_t     modMMC_scan_root_base(file_type type, uint32_t *fatadr, uint8_t *buf);

#ifdef	__cplusplus
}
#endif

#endif /* MODCD_H*/ 
