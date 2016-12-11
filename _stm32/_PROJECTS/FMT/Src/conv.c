#include "conv.h"
#include "board.h"


#define error(a, b)
//extern void error(const char a, uint32_t b);

uint16_t _strlen (const char *str)
{
    uint32_t i = 0;

    if (NULL != str)
        while ('\0' != str[i])
        {
            i++; 
            if (i >= STR_MAX_SIZE)
            { //error("strlen>", i); 
                return STR_MAX_SIZE; 
            }
        }
        
    return i;
}


uint8_t bcd2dec (uint8_t val)
{
    return ( (val/16*10) + (val%16) );
}


uint8_t dec2bcd (uint8_t val)
{
    return ( (val/10*16) + (val%10) );
}


uint32_t _dec2bcd (uint16_t dec) /* Standard iterative function to convert 16-bit integer to BCD */
{
    uint32_t result = 0;
    int shift = 0;

    while (dec)
    {
        result +=  (dec % 10) << shift;
        dec = dec / 10;
        shift += 4;
    }
    return result;
}


uint32_t dec2bcd_r(uint16_t dec) /* Recursive one liner because that's fun */
{
    return (dec) ? ((dec2bcd_r( dec / 10 ) << 4) + (dec % 10)) : 0;
}


#define  MAX_SIZE_STRING  16

/** массив с цифрами HEX системы в ASCII **/
const static char harDEC[] = "0123456789";
//const static unsigned char harHEX[] = "0123456789ABCDEF";
uint8_t long2str (char *pstring, uint32_t value)
{ // @todo
    uint8_t ms[MAX_SIZE_STRING];
    uint8_t size, i;
    uint32_t temp;

    i = MAX_SIZE_STRING;
    while(i--)
    {
        ms[i] = 0; // обнул€ем предварительно
        temp = (value % 10);
        ms[i] = temp;
        value /= 10;
    }
    size = 0;
    // незначущие нули удал€ем?
    for (i = 0; i < MAX_SIZE_STRING; i++)
    {
         //if (0 != ms[i])
         {
             // ‘ормирование строки 
             pstring[size] = harDEC[ms[i]];
             size++;
         }
    }
    pstring[size] = '\0'; // «авершаем строку на вс€кий случай

    return size;
}


int32_t float2int32 (float number, uint8_t zeros)
{
    uint8_t i;
    //BOOL sign = FALSE; @todo
    float fvalue;
    
    if (number < 0) /** ѕризнак отрицательного числа **/
    {
        //sign = TRUE;
        number *= -1;
    }
    fvalue = number;
    for (i = 0; i < zeros; i++)
    {
        fvalue *= 10;
    }
    // 0.5 - округление в большую сторону
    return (int32_t)(fvalue + 0.5);
}


char char2upper (char ch)// в верхний регистр @todo
{
    if ((ch >= 'a') && (ch <= 'z')) 
        ch &= 0x5F;
    else
    {//conversion Windows-1251
        if (((uint8_t)ch >= (uint8_t)('а')) &&
            ((uint8_t)(ch) <= (uint8_t)('€'))) 
          ch -= ('ј' - 'а');
    }
    return ch;
}


void _memcpy (char *src, uint32_t n, char *dest) 
{
    if (NULL == src) // pointer checking
        return;
    if (NULL == dest) // pointer checking
        return;
    for (; 0 < n; n--)
        *src++ = *dest++;
}


char *_strchr (const void *aStr, char Chr) // функци€ провер€ет, встречаетс€ ли в строке символ @todo
{
    char *str = (char*)aStr;
    for (;;) {
        char a = *str++;
        if(a == Chr)
            return --str; // декремент адреса дл€ вывода искомого
        if(!a)
            break; // выходим если нулевой симовл, конец строки
    }
    return 0; //NULL;
}


char *_strncmp (char *strA, char *strB)
{
    msg_t resp = FUNCTION_RETURN_ERROR;
    uint32_t i, j;
    uint32_t m, n;
    char *p_str = (char *)NULL;
    
    if (NULL == strA) // pointer checking
        return (char *)NULL;
    m = _strlen (strA); // проверка на максимальную длину
    if (m >= STR_MAX_SIZE)
        return (char *)NULL;
    if (NULL == strB)
        return (char *)NULL;
    m = _strlen (strB);
    if (m >= STR_MAX_SIZE)
        return (char *)NULL;
    n = m;
    j = 0;
    while ('\0' != strA[j]) // выходим если нулевой символ, конец строки
    {
        if (strA[j] == strB[0])
        {
            m = n; //_strlen (strB);
            i = 0;
            while (('\0' != strB[i]) || ('\0' != strA[j + i]))
            {
                if (strA[j + i] == strB[i]) //
                    m--;
                else
                    break;
                i++;
            }
            if (0 == m)  // если нулевой символ, конец строки
                return &strA[j];
        }
        j++;
    }
    
    return (char *)NULL;
}


uint8_t char2hex (char c)
{
	uint8_t v;
    
	if(c >= '0' && c <= '9')
		v = c - '0';
	else
	if(c >= 'a' && c <= 'f')
		v = c - 'a' + 10;
	else
	if(c >= 'A' && c <= 'F')
		v = c - 'A' + 10;
	else
		v = 0xFF; //not a hex
	return v;
}

