/* Martin Thomas 4/2009 */
#include "fattime.h"
#include "integer.h"

//#include "rtc.h"

DWORD get_fattime (void)
{
// 	DWORD res;
// 	RTC_t rtc;

// 	rtc_gettime( &rtc );
// 	
// 	res =  (((DWORD)rtc.year - 1980) << 25)
// 			| ((DWORD)rtc.month << 21)
// 			| ((DWORD)rtc.mday << 16)
// 			| (WORD)(rtc.hour << 11)
// 			| (WORD)(rtc.min << 5)
// 			| (WORD)(rtc.sec >> 1);

// 	return res;
        // ���������� ����������� �����
    return ((DWORD)(2013 - 1980) << 25)   // ���
            | ((DWORD)9 << 21)            // �����
            | ((DWORD)4 << 16)            // ����
            | ((DWORD)12 << 11)           // ���
            | ((DWORD)0 << 5)             // ������
            | ((DWORD)0 >> 1);            // �������
}

