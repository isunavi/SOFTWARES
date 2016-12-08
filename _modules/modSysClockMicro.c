#include "modSysClockMicro.h"
#include "board.h"


/**
 * ����������� ��������� �����, ���
 */
#define SYSCLOCK_MAX_TIME          0xFFFF

/**
 * �������� "�� ���������" ��� ������������ SysClock = 1 ��
 */
#define SYSCLOCK_DEFAULT_PERIOD    1

//=========================================================================
/**
 * ����������� �������������� ���� � ��������� �������
 * @param pastTime - ����� � �������������
 * @param retFormat - ��������� ������ �������:
 *              SYSCLOCK_GET_TIME_MKS_10         - � �������� �����������
 *              SYSCLOCK_GET_TIME_MKS_100        - � ������ �����������
 *              SYSCLOCK_GET_TIME_MS_1           - � �������������
 *              SYSCLOCK_GET_TIME_MS_10          - � �������� �����������
 *              SYSCLOCK_GET_TIME_MS_100         - � ������ �����������
 *              SYSCLOCK_GET_TIME_S_1            - � ��������
 *              SYSCLOCK_GET_TIME_S_10           - � �������� ������
 * 
 * @return ����� � �������� ���������� �������
 */
#define modSysClock_formatTime(a) a
//=========================================================================


static systime_t currentTime;
//=========================================================================

/**
 * ���� �������. 
 * ������ ������� ������ ���������� � ��������, �������� �������� modSysClock_setRunPeriod.
 * �� ��������� ������ ���������� � �������� 1 ��.
 */
void modSysClock_run (void)
{
    currentTime++;
}


/**
 * ������� ���������� �������� �������� �������
 * @return ������� ����� � �������������
 */
systime_t modSysClock_getTime(void)
{
    return currentTime;
}


/**
 * ���������� �����, ��������� � ������� startTime � ����������� �������
 * ������������ ������������ �������� currentTime
 * @param startTime - ��������� �������� �������� �������
 * @return �����, ��������� �� startTime, � ��������� ��������
 */
systime_t modSysClock_getPastTime (systime_t startTime, uint8_t retFormat)
{
    systime_t pastTime = 0;

    if (startTime <= currentTime)
    {
        pastTime = currentTime - startTime;
    } else {
        pastTime = SYSCLOCK_MAX_TIME - startTime + currentTime + 1;
    }

    pastTime = modSysClock_formatTime(pastTime);

    return pastTime;
}


msg_t modSysClock_timeout (systime_t *pdelay, systime_t delay, uint8_t retFormat)
{
    msg_t respond = FUNCTION_RETURN_ERROR;
    
    if (modSysClock_getPastTime (*pdelay, retFormat) >= delay)
    {  
        *pdelay = modSysClock_getTime();
        respond = FUNCTION_RETURN_OK;
    }
    
    return respond;
}
