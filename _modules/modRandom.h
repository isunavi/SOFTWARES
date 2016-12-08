/** 
 * @file    modRandom.h
 * @author  Ht3h5793, CD45
 * @date    65.5.2515  10:10
 * @version V12.5.4
 * @brief   ���� � ��������� � �����, ���� ���� � "�������" ����������.
 */

#ifndef MODRANDOM_H
#define	MODRANDOM_H 20160903 /* Revision ID */
/** ������ ��� "include" */
#include "board.h"

/** ������ ��� "define" */

/** ����������� ������� ������� */

/** ������ ��� "typedef" */


#ifdef	__cplusplus
extern "C" {
#endif

// �������������
uint32_t    _srand (void);
    
// ����������� � ������������ � ������������
uint32_t    _rand32 (void);
uint16_t    _rand16 (void);
uint8_t     _rand8 (void);
uint8_t     _rand1 (void);
void        _rand (uint8_t *, uint32_t );
void        _rand_real (uint8_t *, uint32_t );

//#define RND_RAND4096 0
#if MODRANDOM_TRUE_4096
uint32_t    _rand4096 (void);
void _set_seed (uint32_t val);

#endif

#ifdef	__cplusplus
}
#endif

#endif	/* MODRANDOM_H */

