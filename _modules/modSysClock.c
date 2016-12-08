#include "modSysClock.h"
#include "board.h"

/**
 * ����������� ��������� �����, ���
 */
#define SYSCLOCK_MAX_TIME          0xFFFFFFFF

/**
 * �������� "�� ���������" ��� ������������ SysClock = 1 ��
 */
#define SYSCLOCK_DEFAULT_PERIOD    TIC_PERIOD   


//====================================================================================================================================================
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
systime_t modSysClock_formatTime (systime_t pastTime, uint8_t retFormat);
//====================================================================================================================================================


static systime_t currentTime;
static systime_t modSysClockPeriod = SYSCLOCK_DEFAULT_PERIOD;
//====================================================================================================================================================


/**
 * ������� ��������� ������� ������ 
 * @param Period - ������������� ������ ������� modSysClock_run, ���
 */
void modSysClock_setRunPeriod (systime_t period)
{
    modSysClockPeriod = period;
}


/**
 * ���� �������. 
 * ������ ������� ������ ���������� � �������� 1��. 
 */
void  modSysClock_run (void)
{
    currentTime += modSysClockPeriod;
}


/**
 * ������� ���������� ������� �������� �������� �������
 * @return ������� ����� � �������������
 */
systime_t modSysClock_getTime (void)
{
    return currentTime;
}


/**
 * ���������� �����, ��������� � ������� startTime � ����������� �������
 * ������������ ������������ �������� currentTime
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
systime_t modSysClock_getPastTime (systime_t startTime, uint8_t retFormat)
{
    systime_t pastTime = 0;

    if (startTime <= currentTime)
    {
        pastTime = currentTime - startTime;
    } else {
        pastTime = (systime_t)SYSCLOCK_MAX_TIME - startTime + currentTime + 1;
    }
    
    pastTime = modSysClock_formatTime (pastTime, retFormat);
    
    return pastTime;
}


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
systime_t modSysClock_formatTime (systime_t pastTime, uint8_t retFormat)
{
    uint32_t retVal = 0;
    uint32_t divider = 1;
    
    switch(retFormat)
    {
        case SYSCLOCK_GET_TIME_MKS_10:
            divider = (TIC_PERIOD / 100);
            break;
        
        case SYSCLOCK_GET_TIME_MKS_100:
            divider = (TIC_PERIOD / 10);
            break;
        
        case SYSCLOCK_GET_TIME_MS_1:
            divider = (TIC_PERIOD);
            break;
        
        case SYSCLOCK_GET_TIME_MS_10:
            divider = (TIC_PERIOD * 10);
            break;
        
        case SYSCLOCK_GET_TIME_MS_100:
            divider = (TIC_PERIOD * 100);
            break;
        
        case SYSCLOCK_GET_TIME_S_1:
            divider = (TIC_PERIOD * 1000);
            break;
        
        case SYSCLOCK_GET_TIME_S_10:
            divider = (TIC_PERIOD * 10000);
            break;
        
        default: break;
    }
            
    retVal = pastTime / divider;
    
    return retVal;
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
