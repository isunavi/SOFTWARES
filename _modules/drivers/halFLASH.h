/* 

 */

#ifndef HALFLASH_STM32F10X_V1_H
#define	HALFLASH_STM32F10X_V1_H 20150716
/**
 *  ������ ��� "include"
 */

//===== ��������� ������� include ==============================================


#include "board.h"

/**
 *  ������ ��� "define"
 */





/*
 * ����������� ������� �������
 */

/**
 *  ������ ��� "typedef"
 */

/**
 *  ������ ��� ���������� �������
 */
#ifdef	__cplusplus
extern "C" {
#endif
    
void      halFLASH_unlock (void);
void      halFLASH_lock (void);


//������� ������� ��� ��������. 
// ��� � ������ �������� ����������������. ������!
void      halFLASH_eraseAllPages (void);

//������� ������� ���� ��������. � �������� ������ ����� ������������ �����
//������������� ��������� ������� ��� �������� ������� ����� ��������.
void      halFLASH_erasePage (uint32_t address);


//data - ��������� �� ������������ ������
//address - ����� �� flash
//count - ���������� ������������ ����, ������ ���� ������ 2
void      halFLASH_flashWrite (uint32_t address, uint8_t *data, uint32_t cnt);

/**
 * 
 * @param adress - ������ � ������, �������� 65565 �����
 * @param pBuf   - ��������� �� �����
 * @param numBuf - ����� ����, �������� 32
 * @return I2C_FUNCTION_RETURN_READY         - ������, ������ ����������
 *         I2C_FUNCTION_RETURN_ERROR_TIMEOUT - ���-�� ������, ��� ��������� ��������� ����� ����������� ��������� ������
 */
uint8_t  halFLASH_read (uint32_t address);
void      halFLASH_flashRead (uint32_t address, uint8_t *data, uint32_t cnt);


#ifdef	__cplusplus
}
#endif

#endif	/* HALFLASH_STM32F10X_V1_H */

