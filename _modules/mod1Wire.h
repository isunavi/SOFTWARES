/**
 * @file    mod1Wire.h
 * @author  ����� �������, Ht3h5793, CD45
 * @date    15.04.2013  11:01
 * @version V1.0.2
 * @brief   ������ A, ������
 * @todo    ��������� ���������� �� ��-����!
 
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
 *  ������ ��� "include"
 */
#include "board.h"
//#include "pt.h"


/**
 *  ������ ��� "define"
 */

//--------------- ����� ������� ��� ��������� 1-Wire ---------------------------
/* ��������� 64-��������� ��� 
 * ����������� ���������, ��� ���������� �� ���� ������ ����� ��������. */
#define DS_COMMAND_SEARCH_ROM                   0xF0   
#define DS_COMMAND_READ_ROM                     0x33 //��������� 64-��������� ��� ������������ ����������
/* �������  match rom �����������, ����� ����� ���������� � �����������
 * 1-Wire ����������. ��������������� ������ �� ���� ������� match rom, 
 * � ����� 64 ��������� ��� ����������, �  �������� �� ����������. ��������
 * �� �������������� ������� ����� ������ ����������, ������������� ���� �����.
 *  ��������� ���������� ����� �������, ���� �� ���� �� ����� ����������� �������� ������. */
#define DS_COMMAND_MATCH_ROM                    0x55 
#define DS_COMMAND_SKIP_ROM                     0xCC // ������� �������� ���
 
//----------------------- ������� ��� DS18B20 ----------------------------------
#define DS18B20_COMMAND_READ_SCRATCHPAD         0xBE // ������� ������ ���������� ������, ��� 8 ������ + crc
#define DS18B20_COMMAND_WRITE_SCRATCHPAD        0x4E // �����m � ���������� ������, 3 �����
#define DS18B20_COMMAND_CONVERT_TEMP            0x44 // ������ ��������� ����������� 
/*  ����� �������.
�������� ���� ������� ��������� �������� ������� ������ ��� �� ����������� ����, ��� ������ 
DS18B20 (������� �����������) � ������������� ������� ������ ������� */
#define DS18B20_COMMAND_ALARM_SEARCH            0xEC
/* ��� ������� ��������� ���������� ���������� ���������� 3 ����� ������ � ������ DS18B20. 
������ ���� ������ ������������ � ������� (TH), ������ ���� ������������ � ������� (TL), � ������ 
���� ������������ � ������� ������������. 
��� �������� ����������� � ���������� �������� ��� ��� ����� ������ ���� �������� �������� 
����������� ��� � ��� [48h] ������, ��� ����������� ���������� ����� ������������ ������� ������, 
����� ������ ����� ��������. */
#define DS18B20_COMMAND_READ_ROM                0x33 
#define DS18B20_COMMAND_COPY_RAM2ROM            0x48

//----------------------- ������� ��� DS1821 ---------------------------------------
#define DS1821_COMMAND_CONVERT_TEMP_READ        0xAA // ������
#define DS1821_COMMAND_CONVERT_TEMP_START       0xEE // ������ ��������� �����������
#define DS1821_COMMAND_CONVERT_TEMP_STOP        0x21 // ������� ��������� �����������
// @todo �������� ������������������� ��������!

//----------------------- ���� �������� ---------------------------------------         
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
 * ����������� ������� �������
 */


/**
 *  ������ ��� "typedef"
 */


#ifdef	__cplusplus
extern "C" {
#endif

/**
 * ������� ������������� ������, ������ ���������������� etc
 */   
void mod1Wire_initMaster (void);

/**
 * ������ � ���������� ����������� 1-Wire ����, � ������ ������ - ������ � ������������� DS18B20
 */
msg_t mod1Wire_runMaster (void);


msg_t mod1Wire_Master_getTemperature (uint8_t number, int8_t *tI, uint8_t *tF);


//����� ����
msg_t mod1Wire_resetWire (void);

// �����\�������� ����
msg_t mod1Wire_xBit (uint8_t wrBit, uint8_t *rdBit);

// ���������� ������� ����� ������
msg_t mod1Wire_xByte (uint8_t wrByte, uint8_t *rdByte);


#ifdef	__cplusplus
}
#endif

#endif	/* MOD1WIRE_H */
