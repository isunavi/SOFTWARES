/*
 * @file    hal LCD
 * @author  AKsenov, Ht3h5793
 * @date    05.03.2014
 * @version V6.5.2
 * @brief  
    @todo �������� �������� �� ������ ����������
    
modKey_init ();
...
while (1) {
    modKey_run ();
    if (modKey_getState (KEY_UP) == MODKEY_STATE_PRESSED) {
        
    }
}
    

*/
#ifndef MODKEY_H
#define MODKEY_H 20150506

#include <defines.h>
#include <modKey_local.h.>

// ��������� ������
typedef enum {
    MODKEY_STATE_NOT_PRESSED  = 0, /** ������ �� ������ */
    MODKEY_STATE_PRESSED      = 1, /** ������ ���� ������ */
    MODKEY_STATE_RELEASED     = 2, /** ������ ���� ������ � �������� */
    MODKEY_STATE_HOLD         = 3, /** ������ ���� ������ � �������� */
    MODKEY_STATE_HOLD_TIME    = 4  /** ������ ���� �������� � ������� ��������� ������� ��������� */
} KEY_STATE_t;


#ifdef	__cplusplus
extern "C" {
#endif

/**
 * ������������� ������.
 */
void modKey_init (void);

/**
 * �������� ��������� ������ �����������
 * @param keyNumber - ����� ������
 * @return          - ��������� ������
 */
uint8_t modKey_getState (uint8_t keyNumber);

/**
 * ����������� ��������� ������
 * @param keyNumber - ����� ������.
 * @param pinState - ���������� ��������� ������(������).
 */
void modKey_run (void);

/**
 * ���������� ����� ��������� ������
 * @param holdTimePeriod ����������� ������ ��� ��������� ��������� ���������
 */
void modKey_setHoldTimePeriod (uint8_t keyNumber, systime_t holdTimePeriod);

/**
 * ���������� ����� ���������� ������
 * @param holdTimePeriod ����������� ������ ��� ��������� ��������� ���������
 */
void modKey_setReleaseTimeout (uint8_t keyNumber, systime_t releaseTimeout);


#ifdef	__cplusplus
}
#endif

#endif /* MODKEY_H */
