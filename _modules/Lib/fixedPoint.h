/* 
 * @file    
 * @author  ����� �������, Ht3h5793, CD45
 * @date    3.04.2013  11:01
 * @version V1.0.0
 * @brief
   http://compgraphics.info/2D/fixed.php
*/

#ifndef SYS_H
#define	SYS_H 1
/** ������ ��� "include" */
#include "defines.h"
#include <math.h>

/** ������ ��� "define" */
#define fixed int32_t
#define roundf(x) floor(x + 0.5f)
#define round(x) floor(x + 0.5)

/** ������ ��� "typedef" */

/** ����������� ������� ������� */

#ifdef __cplusplus
extern "C" {
#endif


// ������������ ����� ����� � ������� ����� � ������������� ������
inline fixed int_to_fixed(int32_t value)
{
      return (value << 16);
}

// ����� ����� ����� � ������������� ������
inline int32_t fixed_to_int(fixed  value)
{
      if (value < 0) return ((value >> 16) - 1);
      if (value >= 0) return (value >> 16);
}

// ���������� �� ���������� ������
inline int round_fixed(fixed value)
{
      return fixed_to_int(value + 5 << 15);
}

// ������������ ����� � ��������� ������ � ������� ����� � ������������� ������
// ����� ���������� ������� ������ ��������
inline fixed double_to_fixed(double value)
{
      return round(value * (65536.0));
}

inline fixed float_to_fixed(float value)
{
      return roundf(value * (65536.0f));
}

// ���������� ��������� (a / b) � ������� ����� � ������������� ������
inline fixed frac_to_fixed(int32_t a, int32_t b)
{
      return (a << 16) / b;
}



#ifdef	__cplusplus
}
#endif

#endif	/* SYS_H */
