/* 
 * File:   CString.cpp
 * Author: Moy
 * 
 * Created on 13 ������ 2012 �., 21:08
 */

#include "Tostring.h"
#define  SIZE_OF_STRING  16

/** ������ � ������� HEX ������� � ASCII **/
const static unsigned char numbersDEC[] = "0123456789";
const static unsigned char numbersHEX[] = "0123456789ABCDEF";
/** ���������� �������� ����� ������� **/
unsigned char Zeros;
/** �������� ���������������� �������� �������� ����� � ������� **/
unsigned char ValueToDECString(unsigned char *string, unsigned long value, bool sign, bool dot);
unsigned char ValueToHEXString(unsigned char *string, unsigned long value, unsigned char digits);
//***********************************************************************//

unsigned char ConvertToString(unsigned char *ptr, unsigned long number, bool toHex) {
    unsigned char length;
    if (toHex) {
        length = ValueToHEXString(ptr, (unsigned long)number, 8);
    } else {
        length = ValueToDECString(ptr, (unsigned long)number, false, false);
    }
    return length;
}
unsigned char ConvertToString(unsigned char *ptr, unsigned short number, bool toHex) {
    unsigned char length;
    if (toHex) {
        length = ValueToHEXString(ptr, (unsigned long)number, 4);
    } else {
        length = ValueToDECString(ptr, (unsigned long)number, false, false);
    }
    return length;
}
unsigned char ConvertToString(unsigned char *ptr, unsigned char number, bool toHex) {
    unsigned char length;
    if (toHex) {
        length = ValueToHEXString(ptr, (unsigned long)number, 2);
    } else {
        length = ValueToDECString(ptr, (unsigned long)number, false, false);
    }
    return length;
}

unsigned char ConvertToString(unsigned char *ptr, signed short  number) {
    return ConvertToString(ptr, (signed long) number);
}

unsigned char ConvertToString(unsigned char *ptr, signed char  number) {
    return ConvertToString(ptr, (signed long) number);
}


unsigned char ConvertToString(unsigned char *ptr, signed long number) {
    bool sign = false;
    /** ������� �������������� ����� **/
    if (number < 0) {
        sign = true;
        number *= -1;
    }
    return ValueToDECString(ptr, number, sign, false);
}

unsigned char ConvertToString(unsigned char *ptr, float number, unsigned char zeros) {
    unsigned char i;
    bool sign = false;
    float fvalue;
    Zeros = zeros;
     /** ������� �������������� ����� **/
    if (number < 0) {
        sign = true;
        number *= -1;
    }
    fvalue = number;
    for (i = 0; i < Zeros; i++) {
        fvalue *= 10;
    }
    return ValueToDECString(ptr, (unsigned long)(fvalue+0.5), sign, true); // 0.5 - ���������� � ������� �������
}

unsigned char ValueToDECString(unsigned char *string, unsigned long value, bool sign, bool dot) {
    unsigned char tetrada[SIZE_OF_STRING];
    unsigned char size, i;
    unsigned long temp;
    /** ��������� ������� **/
    for (i = 0; i < SIZE_OF_STRING; i++) {
        tetrada[i] = 0;
    }    
    
    i = SIZE_OF_STRING;
    while (i--) {
        temp = 0;
        temp = (value % 10);
        tetrada[i] = temp;
        value /= 10;
    }
    
    /** ���������� ������ ���������� ����� **/
     if (dot) {
         size = SIZE_OF_STRING - (Zeros + 1);
     } else {
         size = SIZE_OF_STRING - 1;
     }
     for (i = 0; i < size; i++) {
         if (tetrada[i] != 0)
             break;
     }

    /** ������������ ������ **/
    for (size = 0; i < SIZE_OF_STRING; size++, i++) {
        /** ���� ������ **/
        if (sign) {
            sign = false;
            string[size++] = '-';
        }
        /** ����� � ����� ���� float **/
        if (dot) {
            if (i == SIZE_OF_STRING - Zeros) {
                string[size++] = '.';
            }
        }
        string[size] = numbersDEC[tetrada[i]];
    }
    string[size] = '\0';
    return size;
}

unsigned char ValueToHEXString(unsigned char *string, unsigned long value, unsigned char digits) {
    signed char size = 0;
	unsigned char i = 0;
	unsigned char length = 0;
    unsigned char *pointerToString = string;
    /** ������������ ������ **/
    *pointerToString++ = '0';
    *pointerToString++ = 'x';
    length = digits + 2;
    for (size = digits-1; size >=0 ; size--, i+=4) {
        /** ���� ������ **/
        /** ����� � ����� ���� float **/
        pointerToString[size] = numbersHEX[(value>>i)&0xF];
    }
    string[size] = '\0';
    return length;
}

