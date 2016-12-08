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



char rleInBuf[128] = {"teexxxxth"}; //20 g
char rleOutBuf[128];


//RLE_coderA
//RLE_coderB
//RLE_coderC - оперирует 16-ти битными значениями, старший бит - значение, а младшие - количество


uint32_t RLE_coderA (char *inBuf, uint32_t size, char *outBuf)
{
    uint32_t i = 0;
    uint8_t m = 0;
    char tmpA = *inBuf; // необходимо ли?
    char tmpB = tmpA;

    if ((0 != size) && (NULL != inBuf) && (NULL != outBuf))
    {
        do {
            tmpA = *inBuf++;
            outBuf[i] = tmpB; // записываем каждый раз
            outBuf[i + 1] = m;
            if (tmpB != tmpA)
            {
                tmpB = tmpA;
                i += 2;
                m = 0;
            }
            if (255 <= m)
            { // если этот символ новый
                i += 2;
                m = 0;
            }
            m++;
        } while (0 != size--);
    }
    return i;
}


uint32_t RLE_decoderA (char *inBuf, uint32_t size, char *outBuf)
{
    uint32_t i = 0, s = 0;
    uint16_t j;

    if ((0 != size) && (NULL != inBuf) && (NULL != outBuf))
    {
        do {
            if (0 != inBuf[i + 1])
            {
                for (j = 0; j < inBuf[i + 1]; j++)
                {
                    *outBuf++ = inBuf[i];
                    s++;
                }
                i += 2;
            }// else { break; }
        } while (0 != size--);
    }
    return s;
}


uint32_t RLE_coderB (char *inBuf, uint32_t size, char *outBuf) {
    uint32_t i = 0;
    uint8_t m = 0;
    char tmpA = *inBuf; // необходимо ли?
    char tmpB = tmpA;

    if ((0 != size) && (NULL != inBuf) && (NULL != outBuf)) {
        do {
            tmpA = *inBuf++;
            outBuf[i] = m;
            if (tmpB) { // записываем каждый раз
                outBuf[i] |= 0x80;
            } else {
                outBuf[i] &= ~0x80;
            }
            if (tmpB != tmpA) {
                tmpB = tmpA;
                i++;
                m = 0;
            }
            if (120 <= m) { // если этот символ новый
                i++;
                m = 0;
            }
            m++;
        } while (0 != size--);
    }
    return i;
}


uint32_t RLE_decoderB (char *inBuf, uint32_t size, char *outBuf) {
    uint32_t i = 0, s = 0;
    uint16_t j;

    if ((0 != size) && (NULL != inBuf) && (NULL != outBuf)) {
        do {
            for (j = 0; j < (inBuf[i] & 0x7F); j++) {
                *outBuf++ = inBuf[i] & 0x80 ? 1 : 0;
                s++;
            }
            i++;
        } while (0 != size--);
    }
    return s;
}




// на вход только 0 или !0
uint16_t coder_RLE_C (uint16_t *inBuf, uint32_t size, uint16_t *outBuf)
{
    uint32_t i = 0;
    uint16_t m = 1;
    uint16_t tmpA = *inBuf; // необходимо ли?
    uint16_t tmpB = tmpA;

    if ((0 != size) && (NULL != inBuf) && (NULL != outBuf))
    {
        do
        {
            tmpA = *inBuf++;
            outBuf[i] = m; //БАГ! если size==1
            if (0 != tmpB) // записываем каждый раз
            {
                outBuf[i] |= 0x8000;
            } else {
                outBuf[i] &= ~0x8000;
            }
            if (tmpB != tmpA) //если символ новый,
            {
                tmpB = tmpA; //запоминаем его
                i++; // переходим к следующей выходной ячейке
                m = 0; //обнуляем счетчик
            }
            if (32767 <= m)
            { // если этот символ новый
                i++;
                m = 0;
            }
            m++;
            size--;
        } while (0 != size);
    }
    return i;
}


uint32_t decoder_RLE_C (uint16_t *inBuf, uint32_t size, uint16_t *outBuf)
{
    uint32_t i = 0, s = 0;
    uint16_t j;
    uint16_t val;

    if ((0 != size) && (NULL != inBuf) && (NULL != outBuf))
    {
        do
        {
            val = inBuf[i] & 0x8000 ? 1 : 0;
            for (j = 0; j < (inBuf[i] & 0x7FFF); j++)
            {
                *outBuf++ = val;
                s++;
            }
            i++;
            size--;
        } while (0 != size);
    }
    return s;
}


uint8_t bcd2dec (uint8_t val)
{
    return ( (val/16*10) + (val%16) );
}


uint8_t dec2bcd (uint8_t val)
{
    return ( (val/10*16) + (val%10) );
}


/* Standard iterative function to convert 16-bit integer to BCD */
uint32_t _dec2bcd (uint16_t dec) 
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
        ms[i] = 0; // обнуляем предварительно
        temp = (value % 10);
        ms[i] = temp;
        value /= 10;
    }
    size = 0;
    // незначущие нули удаляем?
    for (i = 0; i < MAX_SIZE_STRING; i++)
    {
         //if (0 != ms[i])
         {
             // Формирование строки 
             pstring[size] = harDEC[ms[i]];
             size++;
         }
    }
    pstring[size] = '\0'; // Завершаем строку на всякий случай

    return size;
}


int32_t float2int32 (float number, uint8_t zeros)
{
    uint8_t i;
    //BOOL sign = FALSE; @todo
    float fvalue;
    
    if (number < 0) /** Признак отрицательного числа **/
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
            ((uint8_t)(ch) <= (uint8_t)('я'))) 
          ch -= ('А' - 'а');
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


// функция проверяет, встречается ли в строке символ @todo
char *_strchr (const void *aStr, char Chr)
{
    char *str = (char*)aStr;
    for (;;) {
        char a = *str++;
        if(a == Chr)
            return --str; // декремент адреса для вывода искомого
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


/*
//skip leading space or tab chars in string
char *skip_space (char *ptr)
{
    char c = *ptr;
    
    while(1)
    {        
        if ((c == ' ') || (c == 9)) 
        {
            ptr++;
            continue;
        }
        else
            break;
    }
    return ptr;
}




//skip from string word which we expect to see
unsigned char *skip_expected_word(unsigned char* ptr, unsigned char* pword, unsigned int* presult)
{
	//assume error result
	*presult=0;
	ptr = skip_space(ptr);
	while(1)
	{
		unsigned char c1,c2;
		c2 = *pword;
		if(c2==0)
		{
			//end of comparing word
			*presult=1;
			return ptr;
		}
		c1 = *ptr;
		if(c1==c2)
		{
			//at this point strings are equal, but compare next chars
			ptr++;
			pword++;
			continue;
		}
		else
			break; //string has no expected word
	}
	return ptr;
}



unsigned char *get_and_skip_word(unsigned char* ptr,
 unsigned char* pword, unsigned int* presult)//get word from string
{
	int i;
	unsigned char c;
	*presult=0;

	//assume error result
	*presult=0;
	ptr = skip_space(ptr);
	for(i=0; i<8; i++)
	{
		c = *ptr;
		if(c==' ' || c==9 || c==0 || c==0xd || c==0xa)
		{
			//end of getting word
			*pword=0;
			*presult=1;
			return ptr;
		}
		*pword++ = c;
		ptr++;
	}
	*pword=0;
	return ptr;
}


char *get_and_skip_wordhhh (char *ptr, char *pword, uint32_t *presult) //get word from string
{
	int i;
	unsigned char c;
	*presult=0;

	//assume error result
	*presult=0;
	ptr = skip_space(ptr);
	for (i=0; i<8; i++)
	{
		c = *ptr;
		if (c==' ' || c==9 || c==0 || c=='\r' || c=='\n')
		{
			//end of getting word
			*pword=0;
			*presult=1;
			return ptr;
		}
		*pword++ = c;
		ptr++;
	}
	*pword=0;
	return ptr;
}
*/


//read decimal integer from string
//returned 0xffffffff mean error
/*
unsigned char* get_dec_integer(unsigned char* ptr, unsigned int* presult)
{
	unsigned int r=0;
	ptr=skip_space(ptr);
	while(1)
	{
		unsigned char c;
		c= *ptr;
		if(c>='0' && c<='9')
		{
			r=r*10+(c-'0');
			ptr++;
			continue;
		}
		else
		if(c==' ' || c==9)
		{
			*presult=r;
			break;
		}
		else
		{
			//unexpected char
			*presult=0xffffffff;
			break;
		}
	}
	return ptr;
}
*/