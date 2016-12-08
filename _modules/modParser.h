/**
 * @file    modParser.h
 * @author  Антон Логинов, Ht3h5793, CD45
 * @date    13.6.2013  10:10
 * @version V1.0.2
 * @brief   


#include "halUSART.h"
#include "modParser.h"
uint8_t respBuf[PARSER_MAX_DATA_SIZE];
uint8_t IOBuf[PARSER_MAX_DATA_SIZE];
uint8_t size8, sizeOut;
uint8_t char_tmp8;
parser_t mParser1;
...
modParser_init (&mParser1, '#'); //'#' - address
while(1)
{
    if (PIN_READ_IN == 0) //режим записи пакетов во флешь
    {
        if (FUNCTION_RETURN_OK == halUSART1_rcvS (&char_tmp8))
        {
            if (FUNCTION_RETURN_OK == modParser_reciv (&mParser1, char_tmp8, &IOBuf1[0], &size8))
            {
                save2flash (size8);
            }
        }
    }
}
*/

#ifndef MODPARSER_H
#define	MODPARSER_H 20150911

#include "board.h"

#define PIK_ADR                 '#'
// Структура пакета - xxxxx:::,P,0x0064=len,data[len],0x5821=crc,;,xxxxxx
// где ::: - преамбула, для очистки парсера
#define PARSER_MAX_DATA_SIZE    (128) //(256 +50)

#define PIK_START               '\n'
#define PIK_FIN                 '\r'
#define PARSER_STRUCT_SIZE      8

#define PARSER_MAX_PACKET_SIZE  (PARSER_MAX_DATA_SIZE + PARSER_STRUCT_SIZE)

#define CRC_CHECK               0 //1

typedef struct _parser_t {
    // Parser state
    //uint16_t  message_size;
    
    uint8_t   pos; // позиция внутреннего автомата, не трогать
    uint8_t   adress; // адрес, на который нужно ответить
    uint32_t  reciv_cnt;
    uint16_t  crcA, crcB;
    uint8_t   *pBufOUT;
} parser_t;


#ifdef	__cplusplus
extern "C" {
#endif

void    modParser_init (parser_t *pp, uint8_t adress); // PIK_ADR
void    modParser_reset (parser_t *pp);

msg_t   modParser_reciv (parser_t *pp, uint8_t c, uint8_t *bufIN, uint8_t *size_bufIN);
msg_t   modParser_transmit (parser_t *pp, uint8_t *bufIN, uint8_t size_bufIN, uint8_t *bufOUT, uint8_t *size_bufOUT);

    
#ifdef	__cplusplus
}
#endif

#endif	/* MODPARSER_H */
