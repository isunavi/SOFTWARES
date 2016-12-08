/* 
 * File:   CString.h
 * Author: Moy
 *
 * Created on 13 јпрель 2012 г., 21:08
 */

#ifndef CSTRING_H
#define	CSTRING_H

/** ѕрототип метода перевода цисла в строку дл€ ulong **/
unsigned char ConvertToString(unsigned char *ptr, unsigned long  number, bool toHex = false);
/** ѕрототип метода перевода цисла в строку дл€ short **/
unsigned char ConvertToString(unsigned char *ptr, unsigned short number, bool toHex = false);
/** ѕрототип метода перевода цисла в строку дл€ char **/
unsigned char ConvertToString(unsigned char *ptr, unsigned char  number, bool toHex = false);
/** ѕрототип метода перевода цисла в строку дл€ long **/
unsigned char ConvertToString(unsigned char *ptr, signed long  number);
/** ѕрототип метода перевода цисла в строку дл€ short **/
unsigned char ConvertToString(unsigned char *ptr, signed short number);
/** ѕрототип метода перевода цисла в строку дл€ char **/
unsigned char ConvertToString(unsigned char *ptr, signed char  number);
/** ѕрототип метода перевода цисла в строку дл€ float **/
unsigned char ConvertToString(unsigned char *ptr, float number, unsigned char zeros);


#endif	/* CSTRING_H */

