/* sysClock - ������ ���������� �������. 
 * ������������ ��� ������� ��������� ���������� � ���������
 * ������ ������� ������� � �������������
 * �����: ������� ������ e-mail: Aksonov.Sergei@gmail.com
 
 * @file    modSysClock.h
 * @author  ������� ������ (e-mail: Aksonov.Sergei@gmail.com), Ht3h5793
 * @date    08-October-2013
 * @version V6.6.1
 * @brief  
 * @todo  ������� �� ���� ����������� ��������
 
 // ������:
    modSysClock_setRunPeriod (1000); // ����� ������� � �������
    systime_t _delay = modSysClock_getTime();
    while (1)
    {
        if (modSysClock_getPastTime (_delay, SYSCLOCK_GET_TIME_MS_1) > 1000))
        {  
            _delay = modSysClock_getTime();
            ...
        }
...
or
        if (FUNCTION_RETURN_OK == modSysClock_timeout (&_delay, 1000, SYSCLOCK_GET_TIME_MS_1))
        {
            ...
        }
    };
    
 !!! ������������ �������� ������ �����?
 !!! ����� ������ ��������
http://piclist.ru/S-ZERO-ERR-TIMER-RUS/S-ZERO-ERR-TIMER-RUS.html

 * ��������� (2012.03.13):
 * - ������ ������� ������ � ��������� ������������ �������� (������� ��� ������ modSysClock_getPastTime, ������ ����������)
 * - ������� ����������� ���������� �������� ������ ������������� ������ ������� modSysClock_run (��. modSysClock_setRunPeriod)
 * - "�� ���������" modSysClock_run ������ ���������� � �������� 1 ��
 * - � ���������/�����������/������������� �� ����� ����������: Aksonov.Sergei@gmail.com
*/

#ifndef MODSYSCLOCK_H
#define MODSYSCLOCK_H 20150507

#include "board.h"


//������� ��������� ����� � 
typedef enum {
    SYSCLOCK_GET_TIME_MKS_1     = 0, // �������� �����������
    SYSCLOCK_GET_TIME_MKS_10       , // �������� �����������
    SYSCLOCK_GET_TIME_MKS_100      , // ������ �����������
    SYSCLOCK_GET_TIME_MS_1         , // �������� �����������
    SYSCLOCK_GET_TIME_MS_10        , // �������� �����������
    SYSCLOCK_GET_TIME_MS_100       , // ������ �����������
    SYSCLOCK_GET_TIME_S_1          , // �������� ������
    SYSCLOCK_GET_TIME_S_10           // �������� ������
} SYSCLOCK_STRUCT;


#ifdef	__cplusplus
extern "C" {
#endif
    
/**
 * ������� ��������� ������� ������ 
 * @param Period - ������������� ������ ������� modSysClock_run, ���
 */
void modSysClock_setRunPeriod (systime_t period);

/**
 * ���� �������. 
 * ������ ������� ������ ���������� � ��������, �������� �������� modSysClock_setRunPeriod.
 * �� ��������� ������ ���������� � �������� 1 ��.
 */
void  modSysClock_run (void);

/**
 * ������� ���������� �������� �������� �������
 * @return ������� ����� � �������������
 */
systime_t modSysClock_getTime (void);

/**
 * ���������� �����, ��������� � ������� startTime � ����������� �������
 * @param startTime - ��������� �������� �������� �������
 * @param retFormat - ������ ������������� �������:
 *              SYSCLOCK_GET_TIME_MKS_10         - � �������� �����������
 *              SYSCLOCK_GET_TIME_MKS_100        - � ������ �����������
 *              SYSCLOCK_GET_TIME_MS_1           - � �������������
 *              SYSCLOCK_GET_TIME_MS_10          - � �������� �����������
 *              SYSCLOCK_GET_TIME_MS_100         - � ������ �����������
 *              SYSCLOCK_GET_TIME_S_1            - � ��������
 *              SYSCLOCK_GET_TIME_S_10           - � �������� ������
 * @return �����, ��������� �� startTime, � ��������� ��������
 */
systime_t modSysClock_getPastTime (systime_t startTime, uint8_t retFormat);


/**
 * ���������� �����, ��������� � ������� startTime � ����������� �������
 * @param pdelay -  �������� �������� �������
 * @param delay - ��������
 * @param retFormat - ������ ������������� �������:
 *              SYSCLOCK_GET_TIME_MKS_10         - � �������� �����������
 *              SYSCLOCK_GET_TIME_MKS_100        - � ������ �����������
 *              SYSCLOCK_GET_TIME_MS_1           - � �������������
 *              SYSCLOCK_GET_TIME_MS_10          - � �������� �����������
 *              SYSCLOCK_GET_TIME_MS_100         - � ������ �����������
 *              SYSCLOCK_GET_TIME_S_1            - � ��������
 *              SYSCLOCK_GET_TIME_S_10           - � �������� ������
 * @return FUNCTION_RETURN_OK or FUNCTION_RETURN_ERROR
 */
msg_t modSysClock_timeout (systime_t *pdelay, systime_t delay, uint8_t retFormat);


#ifdef	__cplusplus
}
#endif

#endif /* MODSYSCLOCK_H */
