/* 
 * File:   CString.h
 * Author: Moy
 *
 * Created on 13 ������ 2012 �., 21:08
 */

#ifndef CSTRING_H
#define	CSTRING_H

/** �������� ������ �������� ����� � ������ ��� ulong **/
unsigned char ConvertToString(unsigned char *ptr, unsigned long  number, bool toHex = false);
/** �������� ������ �������� ����� � ������ ��� short **/
unsigned char ConvertToString(unsigned char *ptr, unsigned short number, bool toHex = false);
/** �������� ������ �������� ����� � ������ ��� char **/
unsigned char ConvertToString(unsigned char *ptr, unsigned char  number, bool toHex = false);
/** �������� ������ �������� ����� � ������ ��� long **/
unsigned char ConvertToString(unsigned char *ptr, signed long  number);
/** �������� ������ �������� ����� � ������ ��� short **/
unsigned char ConvertToString(unsigned char *ptr, signed short number);
/** �������� ������ �������� ����� � ������ ��� char **/
unsigned char ConvertToString(unsigned char *ptr, signed char  number);
/** �������� ������ �������� ����� � ������ ��� float **/
unsigned char ConvertToString(unsigned char *ptr, float number, unsigned char zeros);


#endif	/* CSTRING_H */

