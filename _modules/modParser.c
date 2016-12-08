#include "modParser.h"
#include "board.h"
#if CRC_CHECK
#include "_crc.h"
#endif

enum PARSER_STATE {
    PARSER_STATE_RESET = 0,
    PARSER_STATE_IDLE,
    PARSER_STATE_CH_ADR,
    PARSER_STATE_CH_LN,    
    PARSER_STATE_REC_DATA,
    PARSER_STATE_CRC_A,
    PARSER_STATE_CRC_B,
    PARSER_STATE_CH_END,

};


void modParser_init (parser_t *pp, uint8_t adress) {
    pp->adress = adress;
    modParser_reset (pp);
}


void modParser_reset (parser_t *pp) {
    pp->pos = PARSER_STATE_IDLE;
}


msg_t modParser_reciv (parser_t *pp, uint8_t rchar, uint8_t *bufIN, uint8_t *size_bufIN) {
    msg_t respond = FUNCTION_RETURN_ERROR;

    switch (pp->pos) { 
        case PARSER_STATE_IDLE: // START байт
            if (PIK_START == rchar) { // Ожидаем символ преамбулы
            
                pp->pos = PARSER_STATE_CH_ADR;
            }
            break;
            
        case PARSER_STATE_CH_ADR:
            if (pp->adress == rchar) { // адрес устройства
            
                pp->pos = PARSER_STATE_CH_LN;
            } else {
                if (PIK_START != rchar) { // Если это все еще преамбула, то остаемся
                    pp->pos = PARSER_STATE_IDLE; // ошибка
                }
            }
            break;
						
        case PARSER_STATE_CH_LN:
            if (0 != rchar) { // @todo проверка на границы (pp->message_size > rchar) && 
                pp->reciv_cnt = 0;	                
                *size_bufIN    = rchar;
                pp->crcA = 0xFFFF; // см. описание почему
                pp->pos = PARSER_STATE_REC_DATA;
            } else { // ошибка
            
                pp->pos = PARSER_STATE_IDLE;
            }
            break;

        case PARSER_STATE_REC_DATA: // тут принимаем байты
            bufIN[pp->reciv_cnt] = rchar; // заполняем
            crc16_CITT_s(&pp->crcA, &rchar);
            pp->reciv_cnt++;
            if (pp->reciv_cnt >= *size_bufIN) {
                pp->crcB = 0;
                pp->pos = PARSER_STATE_CRC_A;
            }
            break;
						
       case PARSER_STATE_CRC_A: // Собираем КС
            pp->crcB = (uint16_t)rchar;
            pp->pos = PARSER_STATE_CRC_B;
            break;
            
       case PARSER_STATE_CRC_B: // Проверяем КС
            pp->crcB |= (uint16_t)(rchar << 8);
#if CRC_CHECK
            if (pp->crcA == pp->crcB)
#endif
            {
                pp->pos = PARSER_STATE_CH_END;
            }
#if CRC_CHECK
            else { // ошибка CRC
            
                ParsPos = PARSER_STATE_IDLE;
            }
#endif
            break;
            
        case PARSER_STATE_CH_END:
            if (PIK_FIN == rchar) { 
                respond = FUNCTION_RETURN_OK; // пакет сформирован успешно!
            }
            // отправляемся на начало в любом случае
            pp->pos = PARSER_STATE_IDLE;
            break;
            
        default:
            pp->pos = PARSER_STATE_IDLE;
            break;
    }
		
    return respond;  
}


msg_t modParser_transmit (parser_t *pp, uint8_t *bufIN, uint8_t size_bufIN, uint8_t *bufOUT, uint8_t *size_bufOUT) {
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint32_t i;
    
    if (0 != size_bufIN) { // ((pp->message_size >= size) && 
        bufOUT[0] = PIK_START;
        bufOUT[1] = PIK_START;
        bufOUT[2] = PIK_START;
        bufOUT[3] = pp->adress;
        bufOUT[4] = size_bufIN; // LEN
        pp->crcA = 0xFFFF; // см. описание почему
        
        for (i = 0; i < size_bufIN; i++) {
            bufOUT[i + 5] = bufIN[i];
            crc16_CITT_s(&pp->crcA, &bufIN[i]);
        }
        bufOUT[5 + size_bufIN]  = (uint8_t)(0x00FF & pp->crcA); // crc
        bufOUT[6 + size_bufIN] |= (uint8_t)(pp->crcA >> 8);
        bufOUT[7 + size_bufIN]  = PIK_FIN;
        *size_bufOUT = PARSER_STRUCT_SIZE + size_bufIN;
        
        respond = FUNCTION_RETURN_OK; 
    }
    
    return respond;  
}

