/**
 * @file    mod1Wire.h
 * @author  Антон Логинов, Ht3h5793, CD45
 * @date    15.04.2013  11:01
 * @version V1.0.2
 * @brief   Версия A, мастер
 * @todo    проверить устройства на ик-лучи!
 
http://aterlux.ru/index.php?page=article&art=1wire
http://we.easyelectronics.ru/STM32/esche-raz-o-stm32-i-ds18b20-podpravleno.html
http://www.maximintegrated.com/app-notes/index.mvp/id/187   
http://we.easyelectronics.ru/STM32/stm32-1-wire-poisk-ustroystv.html
http://cluster-d.livejournal.com/484069.html
 //#define DS_COMMAND_SKdddd_ROM            0xC4 // http://electromost.com/news/protokol_dlja_ehlektronnykh_kljuchej_rw1990/2011-04-24-35
 // http://electromost.com/news/universalnyj_kljuch_vezdekhod_mify_i_realnost/2011-02-05-25


 *
 */


#ifndef MOD1WIRE_H
#define	MOD1WIRE_H 20150713 /* Revision ID */
/**
 *  Раздел для "include"
 */
#include "board.h"
//#include "pt.h"


/**
 *  Раздел для "define"
 */

//--------------- общие команды для устройств 1-Wire ---------------------------
/* считывать 64-разрядный код 
 * подчиненных устройств, все устройства на шине должны будут ответить. */
#define DS_COMMAND_SEARCH_ROM                   0xF0   
#define DS_COMMAND_READ_ROM                     0x33 //считывать 64-разрядный код подчиненного устройства
/* Команда  match rom применяется, когда нужно обратиться к конкретному
 * 1-Wire устройству. Микроконтроллер выдает на шину команду match rom, 
 * а затем 64 разрядный код устройства, к  которому он обращается. Отвечать
 * на функциональную команду будет только устройство, «распознавшее» свой адрес.
 *  Остальные устройства будут молчать, пока на шине не будет сформирован импульса сброса. */
#define DS_COMMAND_MATCH_ROM                    0x55 
#define DS_COMMAND_SKIP_ROM                     0xCC // команда пропуска пзу
 
//----------------------- команды для DS18B20 ----------------------------------
#define DS18B20_COMMAND_READ_SCRATCHPAD         0xBE // команда читает блокнотную память, все 8 байтов + crc
#define DS18B20_COMMAND_WRITE_SCRATCHPAD        0x4E // записm в блокнотную память, 3 байта
#define DS18B20_COMMAND_CONVERT_TEMP            0x44 // запуск конверсии температуры 
/*  Поиск Тревоги.
Операция этой команды идентична операции команды Поиска пзу за исключением того, что только 
DS18B20 (датчики температуры) с установленным флажком аварии ответят */
#define DS18B20_COMMAND_ALARM_SEARCH            0xEC
/* Эта команда позволяет устройству управления записывать 3 байта данных в память DS18B20. 
Первый байт данных записывается в регистр (TH), второй байт записывается в регистр (TL), и третий 
байт записывается в регистр конфигурации. 
Для датчиков температуры с паразитным питанием все три байта ДОЛЖНЫ быть записаны командой 
КОПИРОВАНИЕ ОЗУ В ПЗУ [48h] прежде, чем устройством управления будет сгенерирован импульс сброса, 
иначе данные будут потеряны. */
#define DS18B20_COMMAND_READ_ROM                0x33 
#define DS18B20_COMMAND_COPY_RAM2ROM            0x48

//----------------------- команды для DS1821 ---------------------------------------
#define DS1821_COMMAND_CONVERT_TEMP_READ        0xAA // чтение
#define DS1821_COMMAND_CONVERT_TEMP_START       0xEE // запуск конверсии температуры
#define DS1821_COMMAND_CONVERT_TEMP_STOP        0x21 // останов конверсии температуры
// @todo доделать недокументированные комманды!

//----------------------- коды семейств ---------------------------------------         
#define OW_DS1990_FAMILY_CODE                   0x01 // - need check
#define OW_DS2405_FAMILY_CODE	                0x05
#define OW_DS2413_FAMILY_CODE	                0x3A
#define OW_DS1822_FAMILY_CODE	                0x22
#define OW_DS2430_FAMILY_CODE	                0x14
#define OW_DS1990A_FAMILY_CODE	                0x01 // - need check
#define OW_DS2431_FAMILY_CODE	                0x2D
#define OW_DS18S20_FAMILY_CODE	                0x10
#define OW_DS18B20_FAMILY_CODE	                0x28
#define OW_DS2433_FAMILY_CODE	                0x23


/*
 * Определения ответов функций
 */


/**
 *  Раздел для "typedef"
 */


#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Функция инициализации модуля, портов микроконтроллера etc
 */   
void mod1Wire_initMaster (void);

/**
 * работы с конкретным устройством 1-Wire сети, в данном случае - работа с термодатчиком DS18B20
 */
msg_t mod1Wire_runMaster (void);


msg_t mod1Wire_Master_getTemperature (uint8_t number, int8_t *tI, uint8_t *tF);


//сброс шины
msg_t mod1Wire_resetWire (void);

// прием\отправка бита
msg_t mod1Wire_xBit (uint8_t wrBit, uint8_t *rdBit);

// передаются младшим битом вперед
msg_t mod1Wire_xByte (uint8_t wrByte, uint8_t *rdByte);


#ifdef	__cplusplus
}
#endif

#endif	/* MOD1WIRE_H */
