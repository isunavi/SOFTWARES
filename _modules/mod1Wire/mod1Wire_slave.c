#include "mod1Wire.h"

#include "defines.h"
#include "board.h"

#include "_crc.h"
#include "pt.h"
//#include "hal1Wire.h"

#include "modSysClock.h"


#ifdef MOD_WIRE_MASTER

/** Прототип структуры данных для каждого подчиненного устройства (слейва) */
typedef struct {
    uint8_t   ROM [8];   // "лазерный" адрес слейва
    char      sign;      // знак температуры
    uint8_t   tempInt;   // целая
    uint8_t   tempFract; // дробная
    //float     tempC;
    uint8_t   status;
} modWire_slave_data_t;

typedef struct _modWire_master_s{
    // all
    modWire_slave_data_t wireS [MOD_WIRE_SLAVES_MAX];
    uint32_t  timerA; // Таймер для таймаутов
    uint32_t  timerB; // Таймер для таймаутов
    uint8_t   tmp8;
    uint8_t   tmp8M [8];
    uint8_t   tMode; // шина занята?

    // for search
    uint8_t   crc8; // переменная для раcщета контрольной суммы 
    uint8_t   numFound; // число найденных устройств
    uint8_t   IDBit; // первый считываемый бит в битовой последовательности поиска.
    uint8_t   cmpIDBit; // Дополнение id_bit.
    uint8_t   lastDeviceFlag; // флаг, указывающий, что в предыдущем поиске найдено последнее устройство

    uint8_t   numBit; // номер текущего проверяемого бита ROM (1 – 64)
    uint8_t   lastCollision;
    uint8_t   currentCollision;
    uint8_t   currentSelection; // направление поиска, налево или направо идем по бинарному дереву

    // temp
    uint32_t delayRef; // задержка считывания с DS18B20
    uint8_t tmp_L, tmp_H;
    uint8_t   in, jm; // для getTemp

    // ошибки системы
    //uint32_t erAttempt;
    //uint32_t erSearch;
    //uint32_t erSearchAptt;
    uint32_t errors;

    // threats
    struct pt search_pt; // threat
    struct pt term_pt;


} modWire_master_s;




modWire_master_s m1Wire; // создаем экземпляр


inline void __delay_1us2 (volatile uint8_t us)
{
    while (--us > 0)
    {
        __NOP();
        __NOP();
        __NOP();
    }
}

//#include <delay.h>
#define OW_PORT             //GPIOA		//Указать порт ow
#define OW_N_PIN            7		//Указать номер пина ow

#define OW_PIN_MASK         (1 << OW_N_PIN)
#define low_ow_port         do { DDRD |=  OW_PIN_MASK; } while (0)	//Потянуть шину к земле
#define hi_ow_port          do { DDRD &= ~OW_PIN_MASK; } while (0)		//Отпустить шину
#define ow_pin              (PIND & OW_PIN_MASK)

void ONE_WIRE_USART_INIT (void)
{
    PORTD &= ~OW_N_PIN;
}



/**
  * @brief  Передача бита
  * @param  Bit
  * @retval None
  */
msg_t ONE_WIRE_USART_SEND_S (uint8_t dat)
{
	ATOMIC_BLOCK_FORSEON (
        if (0x00 != dat)
        {
            low_ow_port;    //Потянуть шину к земле
            __delay_1us2(4); //delay_us(5);    // Низкий импульс, от 1 до 15 мкс (с учётом времени восстановления уровня)
            hi_ow_port;
            __delay_1us2(75); //delay_us(90);   // Ожидание до завершения таймслота (не менее 60 мкс)
        } else {
            low_ow_port;    //Потянуть шину к земле
            __delay_1us2(75); //delay_us(90);   // Низкий уровень на весь таймслот (не менее 60 мкс, не более 120 мкс)
            hi_ow_port;
            __delay_1us2(4); //delay_us(5);    // Время восстановления высокого уровеня на шине + 1 мкс (минимум)
        }
    )
    return FUNCTION_RETURN_OK;
}



/**
  * @brief  Запись байта
  * @param  Byte: data to be transmitted
  * @retval None
  */
msg_t ONE_WIRE_USART_SEND_M (uint8_t *pDat, uint8_t size)
{
    uint8_t i;//, bita;
	for(i = 0; i < size; i++)
    {
		//bita = *pDat++;		
		ONE_WIRE_USART_SEND_S (*pDat++);
	}
    return FUNCTION_RETURN_OK;
}


/**
  * @brief  Чтение бита
  * @param  None
  * @retval Bit
  */
msg_t ONE_WIRE_USART_RECV_S (uint8_t *pDat)
{
    uint8_t val;
    ATOMIC_BLOCK_FORSEON (
        low_ow_port;		//Потянуть шину к земле
        __delay_1us2(2); //delay_us(2);		//Длительность низкого уровня, минимум 1 мкс
        hi_ow_port;			//Отпускаем шину	 
        __delay_1us2(7); //delay_us(8);		// Пауза до момента сэмплирования, всего не более 15 мкс
        val = ow_pin;		//Прочитать шину
        __delay_1us2(46); //delay_us(55);		// Ожидание до следующего тайм-слота, минимум 60 мкс с начала низкого уровня
    )
    if (0 != val)
	    *pDat = 0xFF;
    else
        *pDat = 0x00;
    return FUNCTION_RETURN_OK;
}


/**
  * @brief  Чтение байта
  * @param  None
  * @retval Result: the value of the received data
  */
msg_t ONE_WIRE_USART_RECV_M (uint8_t *pDat, uint8_t size)
{
    uint8_t i;
    uint8_t tmp;
	for(i=0; i < size; i++)
    {
		ONE_WIRE_USART_RECV_S(&tmp);
        *pDat = tmp; //result |= ((tmp) << i);
        pDat++;
	}
    
   return FUNCTION_RETURN_OK;//result;
}




#include "xprintf.h"
extern char _str[16];
msg_t mod1Wire_resetWire (void)
{ //сброс шины
    msg_t respond = FUNCTION_RETURN_ERROR;
    
//    uint16_t tmp16;
//
//    LATCbits.LATC1 = 0;
//    __DI();
//    TRISCbits.TRISC1 = 0;
//    __delay_us (480);
//    TRISCbits.TRISC1 = 1;
//    __delay_us (8);
//    tmp16 = PORTCbits.RC1;
//    __EI();
//
//    xsprintf (_str, "reset:%u", tmp16);
//    paint_putStrColRow (0, 36, _str);


    //__enter_critical(); //
    
    if (ow_pin == 0) //Проверить линию на отсутствие замыкания
        return respond;
    low_ow_port;					//Потянуть шину к земле
    delay_us(500);				//Ждать 480..960 мкс
    ATOMIC_BLOCK_FORSEON (
        hi_ow_port;						//Отпускаем шину
        //delay_us(2); // Время необходимое подтягивающему резистору, чтобы вернуть высокий уровень на шине
        delay_us(50);					//Ждать 50 микросекунд
        respond = ow_pin;			//Прочитать шину
    )
    delay_us(450);				//Дождаться окончания инициализации 
    //__exit_critical(); //
    
    
    if(respond) 
        return respond;	//Датчик не обнаружен 
        
    LED_TEST_ON;
    return respond = FUNCTION_RETURN_OK;							//Датчик обнаружен 
      
  
    
/*
    if (FALSE == m1Wire.tMode)
    {
        ONE_WIRE_USART_SET_BAUD ((uint16_t)BAUD_9600); // Инициализируем UART на малую скорость
        ONE_WIRE_USART_CLEAR;
        ONE_WIRE_USART_SEND_S (0xF0);
        m1Wire.tMode = TRUE;
    }
    if (FUNCTION_RETURN_OK == ONE_WIRE_USART_RECV_S (&m1Wire.tmp8))
    { // @todo проверку на время еще, если нет веры USARTy
    // @todo Добавить задержку, если необходимо и возможно присутствие ds1994 и ds2404
        if (m1Wire.tmp8 == 0x00)
        { // Проверяем, нормальный ли сброс  
            respond = FUNCTION_RETURN_ERROR; // у нас кз
            m1Wire.errors++;
        } else if (m1Wire.tmp8 == 0xF0) 
        {
            respond = FUNCTION_RETURN_ERROR; // у нас обрыв
            m1Wire.errors++;
        }
        else if ((m1Wire.tmp8 == 0xE0) || (m1Wire.tmp8 == 0xC0))
        {
            respond = FUNCTION_RETURN_OK; // есть устройства на шине
        }
        ONE_WIRE_USART_SET_BAUD ((uint16_t)BAUD_115200); // возвращаем стандартную скорость
        ONE_WIRE_USART_CLEAR;
        m1Wire.tMode = FALSE;
    }
    */
    return respond;
}


msg_t mod1Wire_xBit (uint8_t wrBit, uint8_t *rdBit)
{ // прием\отправка бита
    msg_t    respond = FUNCTION_RETURN_ERROR;
    uint8_t  tmpBit;
    
    if (FALSE == m1Wire.tMode)
    {
        if (0 == wrBit) {
            ONE_WIRE_USART_SEND_S (0x00);
        } else {
            ONE_WIRE_USART_SEND_S (0xFF);  
        }
        m1Wire.tMode = TRUE;
    }
    if (FUNCTION_RETURN_OK == ONE_WIRE_USART_RECV_S (&tmpBit))
    {
        if (tmpBit == 0xFF)
        {
            *rdBit = 1;
        } else {
            *rdBit = 0;
        }
        m1Wire.tMode = FALSE;
        respond = FUNCTION_RETURN_OK;
    }
    return respond;
}


msg_t mod1Wire_xByte (uint8_t wrByte, uint8_t *rdByte)
{ // передаются младшим битом вперед
    msg_t    respond = FUNCTION_RETURN_ERROR;
    uint8_t  i;

    if (FALSE == m1Wire.tMode) {
        for (i = 0; i < 8; i++) { // сборка "битов"
            if (wrByte & 0x01) {
                m1Wire.tmp8M[i] = 0xFF;
            } else {
                m1Wire.tmp8M[i] = 0x00;
            }
            wrByte = wrByte >> 1;
        } 
        ONE_WIRE_USART_SEND_M (&m1Wire.tmp8M[0], 8);  
        m1Wire.tMode = TRUE;
    }
    *rdByte = 0; // необязательно
    if (FUNCTION_RETURN_OK == ONE_WIRE_USART_RECV_M (&m1Wire.tmp8M[0], 8)) {
        for (i = 0; i < 8; i++) {
            *rdByte = *rdByte >> 1;
            if (m1Wire.tmp8M[i] == 0xFF) {
                *rdByte |= 0x80;
            }
        }
        m1Wire.tMode = FALSE;
        respond = FUNCTION_RETURN_OK;
    }
    return respond;
}


static int search(struct pt *pt)
{
//        msg_t respond = FUNCTION_RETURN_ERROR; // для отладки оставить пока
    
    PT_BEGIN(pt); // для отладки оставить пока
    if (FALSE == m1Wire.lastDeviceFlag) { // тут сделать цикл поиска и итерации тех, что уже найдены
        //m1Wire.erSearchAptt++;
        m1Wire.currentCollision = 0;
        m1Wire.crc8 = 0; // обнуляем, обязательно перед каждым поиском
        // команда на чтение адреса устройства
        PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte(DS_COMMAND_SEARCH_ROM, &m1Wire.tmp8));
        // начинаем цикл поиска, за один проход должен собраться один адрес
        for (m1Wire.numBit = 1; m1Wire.numBit <= 64; m1Wire.numBit++) {
            PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xBit(1, &m1Wire.IDBit)); // читаем первый бит
            // читаем второй бит (комплементарный)
            PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xBit(1, &m1Wire.cmpIDBit));
            if ((0 != m1Wire.IDBit) && (0 != m1Wire.cmpIDBit)) { // две единицы, где-то провтыкали и заканчиваем поиск
//                    respond = FUNCTION_RETURN_ERROR; // если тут, то на шине никого нет, в принципе тут не должны быть потому, что прошли успешно до этого ресет шины
                break;
            }
            else if (m1Wire.IDBit != m1Wire.cmpIDBit) // если биты разные, то ищем дальше
            { 
                m1Wire.currentSelection = m1Wire.IDBit;
            } else {
                // Коллизия, если тут, то кругом одни нули, нужно решать, куда идти и что делать...
                if (m1Wire.numBit < m1Wire.lastCollision) 
                {
                    if ((m1Wire.wireS[m1Wire.numFound].ROM[(m1Wire.numBit - 1) >> 3] & 1 << ((m1Wire.numBit - 1) & 0x07)) > 0) // если прошлый бит равен 1
                    {
                        m1Wire.currentSelection = 1;

                        // если пошли по правой ветке, запоминаем номер бита
                        if (m1Wire.currentCollision < m1Wire.numBit) 
                        {
                            m1Wire.currentCollision = m1Wire.numBit;
                        }
                    } else {
                        m1Wire.currentSelection = 0;
                    }
                } else {
                    if (m1Wire.numBit == m1Wire.lastCollision) // если равен, то тут  была развилка раньше
                    {
                        m1Wire.currentSelection = 0;
                    } else {
                        m1Wire.currentSelection = 1; // идем по правой ветке
                        if (m1Wire.currentCollision < m1Wire.numBit) // и запоминаем номер бита
                        {
                            m1Wire.currentCollision = m1Wire.numBit;
                        }
                    }                   
                }
            }
            if (m1Wire.currentSelection == 0) // установка или сброс бита ROM байта по номеру rom_byte_number
            {
                m1Wire.wireS[m1Wire.numFound].ROM[(m1Wire.numBit - 1) >> 3] &= ~(1 << ((m1Wire.numBit - 1) & 0x07));
            } else {
                m1Wire.wireS[m1Wire.numFound].ROM[(m1Wire.numBit - 1) >> 3] |= 1 << ((m1Wire.numBit - 1) & 0x07);
            }
            // записываем бит направления поиска
            PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xBit(m1Wire.currentSelection, &m1Wire.tmp8));
            if ((m1Wire.numBit & 0x07) == 0) { // тут же в цикле считаем CRC
                crc8_DS_s(&m1Wire.crc8, &m1Wire.wireS[m1Wire.numFound].ROM[(m1Wire.numBit - 1) >> 3]);
            }
        }
        // если поиск удачный, то CRC должно быть равно 0
        if ((m1Wire.numBit > 64) && (m1Wire.crc8 == 0)) { //  проверка на тот факт, что нигде не было сбоя, все биты приняли успешно, их должно быть 64
            m1Wire.errors++;
            if (m1Wire.currentCollision == 0) // что не было коллизий, значит все,
            {
                m1Wire.lastDeviceFlag = TRUE; // мы нашли последнее устройство
            }
            else
            {
                m1Wire.numFound++;
                m1Wire.lastCollision = m1Wire.currentCollision;
            }
            
            if (MOD_WIRE_SLAVES_MAX <= m1Wire.numFound)
            {
                m1Wire.lastDeviceFlag = TRUE; // хватит, остальным не повезло
            }
//                respond = FUNCTION_RETURN_OK; // поиск успешным оказался
        } else {//if (!respond || !ROM_NO[0]) // если не нашли устройства then reset counters so next 'search' will be like a first
            m1Wire.lastDeviceFlag = FALSE; 
            m1Wire.wireS[m1Wire.numFound].status = FALSE;
//                respond = FUNCTION_RETURN_ERROR;
        }
    }
    
    PT_END(pt);
}


static int getTemp (struct pt *pt) {
    uint16_t tmp;
    
    PT_BEGIN(pt);
#if (1 == MOD_WIRE_TYPE_DS1821)     
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_resetWire()); // сброс шины
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte (DS1821_COMMAND_CONVERT_TEMP_START, &m1Wire.tmp8)); // Передаем команду начала тмпературного преобразования
    
    m1Wire.timerA = modSysClock_getTime(); // ждем пока DS18B20 подготовит данные
    PT_WAIT_UNTIL(pt, modSysClock_getPastTime (m1Wire.timerA, SYSCLOCK_GET_TIME_MS_1) >= 10);
    
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_resetWire()); // сброс шины
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte (DS1821_COMMAND_CONVERT_TEMP_READ, &m1Wire.tmp8)); // читаем данные из датчика
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte (0xFF, &m1Wire.tmp_L));

    if ((m1Wire.tmp_L & 0x80) == 0) { //проверяем знак 
        m1Wire.wireS[0].sign = '+';
    } else {
        m1Wire.wireS[0].sign = '-'; 
    }
    m1Wire.wireS[0].tempInt = m1Wire.tmp_L;
    m1Wire.wireS[0].tempFract = 0;
    m1Wire.wireS[0].status = TRUE;
#endif

    
#if (1 == MOD_WIRE_TYPE_DS18B20)
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_resetWire ()); // сброс шины
#if (1 == MOD_WIRE_SLAVES_MAX)
    // если устройство одно, то сразу читаем его ID, без поиска
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte (DS_COMMAND_READ_ROM, &m1Wire.tmp8));
    m1Wire.crc8 = 0x00;
    for (m1Wire.jm = 0; m1Wire.jm < 8; m1Wire.jm++)
    { // читаем адрес
        PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte (0xFF, &m1Wire.wireS[0].ROM[m1Wire.jm]));
        crc8_DS_s(&m1Wire.crc8, &m1Wire.wireS[0].ROM[m1Wire.jm]);
    }
#else
    PT_WAIT_THREAD(pt, search(&m1Wire.search_pt)); // поиск
#endif
    // @todo сделать проверку на удачное сщитывание
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_resetWire()); // сброс шины
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte(DS_COMMAND_SKIP_ROM, &m1Wire.tmp8)); // команда пропуска пзу
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte(DS18B20_COMMAND_CONVERT_TEMP, &m1Wire.tmp8)); // Передаем команду начала тмпературного преобразования
    
    m1Wire.timerA = modSysClock_getTime(); // ждем пока DS18B20 подготовит данные
    PT_WAIT_UNTIL(pt, modSysClock_getPastTime(m1Wire.timerA, SYSCLOCK_GET_TIME_MS_1) >= 1000);
    
    for (m1Wire.in = 0; m1Wire.in < MOD_WIRE_SLAVES_MAX; m1Wire.in++)
    { // читаем температуру из известных
        if (OW_DS18B20_FAMILY_CODE == m1Wire.wireS[m1Wire.in].ROM[0])
        {
            PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_resetWire());
            PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte(DS_COMMAND_MATCH_ROM, &m1Wire.tmp8)); // Известен адрес устройства
            for (m1Wire.jm = 0; m1Wire.jm < 8; m1Wire.jm++)  // Передаем в шину 64 бита адреса
                PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte(m1Wire.wireS[m1Wire.in].ROM[m1Wire.jm], &m1Wire.tmp8));
        }
        //PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == xByte(DS_COMMAND_SKIP_ROM, &tmp8)); // команда пропуска пзу
        PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte(DS18B20_COMMAND_READ_SCRATCHPAD, &m1Wire.tmp8)); // читаем данные из датчика
        PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte(0xFF, &m1Wire.tmp_L));
        PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte(0xFF, &m1Wire.tmp_H));
        tmp = ((uint16_t)m1Wire.tmp_H << 8 ) | m1Wire.tmp_L; // «склеиваем» нулевой и первый байты ОЗУ датчика
        if ((m1Wire.tmp_H & 0x80) == 0) { //проверяем старший разряд 
            m1Wire.wireS[m1Wire.in].sign = '+';
        } else {
            tmp = ~tmp + 1;
            m1Wire.wireS[m1Wire.in].sign = '-'; 
        }
        m1Wire.tmp_H = tmp >> 4; //(unsigned short)((tmp_L & 0x07) << 4) | (tmp_H >> 4);
        m1Wire.wireS[m1Wire.in].tempInt = m1Wire.tmp_H;
        m1Wire.tmp_L = (uint16_t)(tmp & 0x0F); //преобразуем дробное в целое число
        m1Wire.tmp_L = (m1Wire.tmp_L << 1) + (m1Wire.tmp_L << 3); // Умножаем на 10
        m1Wire.tmp_L = (m1Wire.tmp_L >> 4); //делим на 16 или умножаем на 0.0625
        m1Wire.wireS[m1Wire.in].tempFract = m1Wire.tmp_L;
        // wireS[in].temp_Float = tmp_H * 1.0 + (tmp_L * 0.01); // формируем число с плавающей точкой
        m1Wire.wireS[m1Wire.in].status = TRUE;

    }
#endif // MOD_WIRE_TYPE_DS18B20
    PT_END(pt);
}

#endif
   


msg_t mod1Wire_Master_getTemperature (uint8_t number, uint8_t *tI, uint8_t *tF){ //сброс шины
    msg_t respond = FUNCTION_RETURN_ERROR;
    // @todo
#if (1 == MOD_WIRE_TYPE_DS18B20)
    if (((OW_DS18B20_FAMILY_CODE == m1Wire.wireS[number].ROM[0]) || 
        (OW_DS18S20_FAMILY_CODE == m1Wire.wireS[number].ROM[0])) && (MOD_WIRE_SLAVES_MAX > number))
    { // && 
   //if (FALSE != wireS[number].status) {
#elif  (1 == MOD_WIRE_TYPE_DS1821)
    if (MOD_WIRE_SLAVES_MAX > number)
    {
#endif
        *tI = m1Wire.wireS[number].tempInt;
        *tF = m1Wire.wireS[number].tempFract;
        respond = FUNCTION_RETURN_OK;
    }
    return respond;
}


    
/**
 * Функция инициализации модуля, уарта, портов микроконтроллера,
 */   
void mod1Wire_initMaster (void) {
    uint8_t i;
    ONE_WIRE_USART_INIT; // Инициализируем UART ( @todo GPIO дописать тут )
    ONE_WIRE_USART_CLEAR;
    for (i = 0; i < MOD_WIRE_SLAVES_MAX; i++) {
        m1Wire.wireS[i].status = FALSE; // clear properties & status
        m1Wire.wireS[i].ROM[0] = 0;
    }
    PT_INIT(&m1Wire.search_pt);
    PT_INIT(&m1Wire.term_pt);
    m1Wire.numFound = 0;
    m1Wire.delayRef = modSysClock_getTime();
    
    m1Wire.tMode = FALSE;
    m1Wire.errors = 0;
    m1Wire.timerB = modSysClock_getTime();
    
    // for search
    m1Wire.lastDeviceFlag = FALSE;
    m1Wire.numFound = 0;
    m1Wire.lastCollision = 0;
}


/**
 * работы с конкретным устройством 1-Wire сети, в данном случае - работа с термодатчиком DS18B20
 */
msg_t mod1Wire_runMaster (void) {
    msg_t respond = FUNCTION_RETURN_OK;
    
    getTemp(&m1Wire.term_pt); // микрокод работы с DS18B20
    // every 10 seconds restart
    if ((modSysClock_getPastTime (m1Wire.timerB, SYSCLOCK_GET_TIME_MS_1) > 10000) &&
        (m1Wire.errors >= 10)) {
        m1Wire.timerB = modSysClock_getTime();
        mod1Wire_initMaster ();
    }
    return respond;
}
    










//#include "debug.h"
//#include "modRandom.h"
// http://www.maximintegrated.com/app-notes/index.mvp/id/214
#ifdef MOD_WIRE_SLAVE

static const uint8_t ROM[8] = { 
    0x01,
    0xAA,
    0x55,
    0x0F,
    0xF0,
    0x42,
    0x00,
    0xF4, // CRC, заново персщитать при изменении вышестоящих!
};



enum slaveState_status {
    SLAVE_STATE_IDLE = 1,
    SLAVE_STATE_RESET,
    SLAVE_STATE_SEND_DATA,
    SLAVE_STATE_RECIV_DATA,
    
};


static uint8_t    modWire_slaveStatus;
uint16_t          slaveTimer;
uint8_t           slaveByteA;
static uint8_t    slaveCountBit, slaveCountByte;
uint8_t           bitA, bitB;
uint8_t           bitRd;
static uint8_t    bitS; // бит на выдачу
uint8_t           slaveRW_count;
uint8_t          *pData; //  указатель данных

struct pt slaveState_pt; // протопоток




void delay_init (void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM2, ENABLE);
    
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler = (12 - 1);
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
        
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_Cmd(TIM2, ENABLE);

}


void _T_delay_us (volatile uint16_t us)
{
    TIM2->CNT = 0;
    while((TIM2->CNT) <= us);
}


void _T_delay_ms (uint16_t ms)
{
    int i;
    for (i=0; i < ms; i++){
        _T_delay_us (1000);
    }
}

#define DELAY_US(a)         { TIM2->CNT = 0; while((TIM2->CNT) <= a){}; };
#define TCK_SET(a)          { TIM2->CNT = a; };
#define TCK_CLR             { TIM2->CNT = 0; };
#define CNT_VAL             ( TIM2->CNT )

#define MOD_WIRE_IN          (GPIOA->IDR & GPIO_Pin_10)
#define MOD_WIRE_L           { GPIOA->BRR  = GPIO_Pin_10; } //GPIO_ResetBits(GPIOA, GPIO_Pin_10)
#define MOD_WIRE_H           { GPIOA->BSRR = GPIO_Pin_10; } //GPIO_SetBits  (GPIOA, GPIO_Pin_10)

//  лучше сразу, некогда ждать
// #define MOD_WIRE_OPERATE(a, b)   { \
//                                  if (!a){ MOD_WIRE_L; } \
//                                  _T_delay_us(60); *b = MOD_WIRE_IN ? 1 : 0; \
//                                  _T_delay_us(42 - 24); MOD_WIRE_H; } 
// // // #define MOD_WIRE_OPERATE(a, b)   { _T_delay_us(2); \
// // //                                  if (!a){ MOD_WIRE_L; } \
// // //                                  _T_delay_us(47); *b = MOD_WIRE_IN ? 1 : 0; \
// // //                                  _T_delay_us(26 - 24); MOD_WIRE_H; } 

#define MOD_WIRE_OPERATE(a, b)   { \
                                 if (!a){ MOD_WIRE_L; } \
                                 _T_delay_us(25); *b = MOD_WIRE_IN ? 1 : 0; \
                                 _T_delay_us(26 - 24); MOD_WIRE_H; } 

#define SET_MODE_READ     { bitS = 1; }
#define SET_MODE_WRITE     { } //?


void slaveResetSys(void)
{
    SET_MODE_READ;
    PT_INIT(&slaveState_pt); // restart thread
    modWire_slaveStatus = SLAVE_STATE_IDLE;
}


PT_THREAD(slaveState(struct pt *pt))
{
    PT_BEGIN(pt);
    
    PT_WAIT_UNTIL(pt, SLAVE_STATE_RESET == modWire_slaveStatus);
    SET_MODE_READ; // read command
    for (slaveCountBit = 0; slaveCountBit < 8; slaveCountBit++)
    {
        PT_YIELD(pt); // wait bit first
        slaveByteA = slaveByteA >> 1;
        if (bitRd) slaveByteA |= 0x80;
    }
    if (DS_COMMAND_SEARCH_ROM == slaveByteA)
    {
        for (slaveCountByte = 0; slaveCountByte < 8; slaveCountByte++)
        {
            for (slaveCountBit = 0; slaveCountBit < 8; slaveCountBit++)
            {
                SET_MODE_WRITE;
                bitA = (ROM[slaveCountByte] & (1 << slaveCountBit)) ? 1 : 0;
                bitS = bitA;
                PT_YIELD(pt); // wait bit
                bitS = bitA ? 0 : 1;
                PT_YIELD(pt);
                SET_MODE_READ;
                PT_YIELD(pt);
                if (bitA != bitRd)
                {
                    slaveResetSys(); // if bit alien
                    PT_EXIT(pt);
                }
            }
        }
        slaveResetSys(); PT_EXIT(pt);
    }
    else if (DS_COMMAND_READ_ROM == slaveByteA)
    {
        modWire_slaveStatus = SLAVE_STATE_SEND_DATA;
        slaveRW_count = 8;
        pData = (uint8_t *)&ROM[0];
    }
    else { slaveResetSys(); PT_EXIT(pt); }
    // транзакционный уровень данных
    if (SLAVE_STATE_RECIV_DATA == modWire_slaveStatus)
    {
        SET_MODE_READ;
        for (slaveCountByte = 0; slaveCountByte < slaveRW_count; slaveCountByte++)
        {
            for (slaveCountBit = 0; slaveCountBit < 8; slaveCountBit++)
            {
                PT_YIELD(pt);
                bitS = pData[slaveCountByte] |= (bitRd << slaveCountBit);
            }
        }     
    }
    else if (SLAVE_STATE_SEND_DATA == modWire_slaveStatus)
    {   
        SET_MODE_WRITE;
        for (slaveCountByte = 0; slaveCountByte < slaveRW_count; slaveCountByte++)
        {
            for (slaveCountBit = 0; slaveCountBit < 8; slaveCountBit++)
            {
                bitS = pData[slaveCountByte] & (1 << slaveCountBit) ? 1 : 0;
                PT_YIELD(pt); // wait bit
            }
        }
    }
    
   // slave_reset_all();
    
    PT_END(pt);
}


void mod1Wire_EXTI_IRQ(void)
{
    //if (EXTI->PR & (1 << 10)) // можно и убрать
    {
        __disable_irq(); // off interrupt for noise
        MOD_WIRE_OPERATE(bitS, &bitRd);
        TCK_CLR; // сбрасываем сщетчик измерения RESET-а
        while(1)
        {
            if (0 == MOD_WIRE_IN) // пока прижаты, терпим
            {
                if (CNT_VAL >= (400 - 45)) // 300 - 400
                {
                    DELAY_US(80); //20
                    MOD_WIRE_L;
                    DELAY_US(180); //228
                    MOD_WIRE_H;
                    slaveCountBit = 0;
                    LED_A_INV;
                    slaveResetSys();
                    modWire_slaveStatus = SLAVE_STATE_RESET;
                }
            }
            else break;
        }
        
        slaveState(&slaveState_pt); // наш поток
        
        __enable_irq();
        EXTI->PR |= (1 << 10); // clear pending interrupt
    }
//     if(EXTI_GetITStatus(EXTI_Line10) != RESET)
// 	{
// 		EXTI_ClearITPendingBit(EXTI_Line10);
// 	}
}


void initSlave(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    
    MOD_WIRE_H; // во избежание бросков в линии
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource10);
    
    EXTI_InitStructure.EXTI_Line = EXTI_Line10;  //!
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; // прерывание (а не событие)
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // срабатываем по заднему фронту импульса
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; // вкл
    EXTI_Init(&EXTI_InitStructure);
    
    NVIC_EnableIRQ(EXTI15_10_IRQn);
    NVIC_SetPriority(EXTI15_10_IRQn, 2);
    
//     ROM[1] = _rand8();
//     ROM[2] = _rand8();
//     ROM[3] = _rand8();
//     ROM[4] = _rand8();
//     ROM[5] = _rand8();
//     ROM[6] = _rand8();

    // для того, чтобы посщитать CRC, раскомментировать
//     uint8_t tmp8 = 0x00;
//     for (uint8_t i = 0; i < 7; i++)
//     {
//         crc8S_DS(&tmp8, &ROM[i]);
//     }
//     ROM[7] = tmp8;

    slaveResetSys();
    delay_init();
    slaveTimer  = 2000UL;
    
//     EXTI->IMR |= EXTI_IMR_MR10; // Line 9            (GPIOA)
//     EXTI->FTSR |= EXTI_FTSR_TR10; // falling edge
//     RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; //Подаем тактовые импульсы на таймер 2
//     TIM2->PSC = (12 - 1); //Коэффициент деления 2096 (1 msec)
//     TIM2->ARR = 10000;
//     TIM2->CR1 &= ~TIM_CR1_DIR;
//     TIM2->CR1 |= TIM_CR1_CEN; //Запуск счета  
//     
    // // // void delay_init (void)
// // // {
// // //     RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
// // //     TIM2->CR1 = 0;                                    
// // //     TIM2->CR2 = 0;
// // //     TIM2->PSC = 0;//0;//0;//(12 - 1); // коэффициент деления
// // //     TIM2->ARR = 0;//10000;
// // // //     TIM2->CR1 &= ~TIM_CR1_DIR;
// // // //     TIM2->CR1 |= TIM_CR1_CEN; // запуск счета  
// // //     TIM2->CR1 |= TIM_CR1_DIR | TIM_CR1_OPM;// | TIM_CR1_ARPE;
// // //     
// // // }


// // // inline void _T_delay_us (uint16_t us)
// // // {
// // //     TIM2->ARR = us;
// // //     TIM2->CR1 |= TIM_CR1_CEN; // запуск счета 
// // //     while (TIM2->CR1 & TIM_CR1_CEN);
// // // }

//         RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;                     // enable clock for Alternate Function
//         AFIO->EXTICR[2] &= 0xF0FF;                              // clear used pin
//         AFIO->EXTICR[2] |= (0x0F00 & __AFIO_EXTICR3);           // set pin to use

//         EXTI->IMR       |= ((1 << 10) & __EXTI_IMR);            // unmask interrupt
//         EXTI->EMR       |= ((1 << 10) & __EXTI_EMR);            // unmask event
//         EXTI->RTSR      |= ((1 << 10) & __EXTI_RTSR);           // set rising edge
//         EXTI->FTSR      |= ((1 << 10) & __EXTI_FTSR);           // set falling edge

//         NVIC->ISER[1] |= (1 << (EXTI15_10_IRQChannel & 0x1F)); // enable interrupt EXTI 10..15






   // modWire_slaveState = SLAVE_STATE_RESET;


    
//     while(1)
//     {
//         _T_delay_ms(1000);
//         LED_B_INV;
//     }
    __enable_irq();


}


msg_t runSlave(void)
{
    msg_t respond = FUNCTION_RETURN_OK;
    return respond;
}

#endif
