#include "mod1Wire.h"

#include "defines.h"
#include "board.h"

#include "_crc.h"
#include "pt.h"
//#include "hal1Wire.h"

#include "modSysClock.h"


#ifdef MOD_WIRE_MASTER

/** �������� ��������� ������ ��� ������� ������������ ���������� (������) */
typedef struct {
    uint8_t   ROM [8];   // "��������" ����� ������
    char      sign;      // ���� �����������
    uint8_t   tempInt;   // �����
    uint8_t   tempFract; // �������
    //float     tempC;
    uint8_t   status;
} modWire_slave_data_t;

typedef struct _modWire_master_s{
    // all
    modWire_slave_data_t wireS [MOD_WIRE_SLAVES_MAX];
    uint32_t  timerA; // ������ ��� ���������
    uint32_t  timerB; // ������ ��� ���������
    uint8_t   tmp8;
    uint8_t   tmp8M [8];
    uint8_t   tMode; // ���� ������?

    // for search
    uint8_t   crc8; // ���������� ��� ��c���� ����������� ����� 
    uint8_t   numFound; // ����� ��������� ���������
    uint8_t   IDBit; // ������ ����������� ��� � ������� ������������������ ������.
    uint8_t   cmpIDBit; // ���������� id_bit.
    uint8_t   lastDeviceFlag; // ����, �����������, ��� � ���������� ������ ������� ��������� ����������

    uint8_t   numBit; // ����� �������� ������������ ���� ROM (1 � 64)
    uint8_t   lastCollision;
    uint8_t   currentCollision;
    uint8_t   currentSelection; // ����������� ������, ������ ��� ������� ���� �� ��������� ������

    // temp
    uint32_t delayRef; // �������� ���������� � DS18B20
    uint8_t tmp_L, tmp_H;
    uint8_t   in, jm; // ��� getTemp

    // ������ �������
    //uint32_t erAttempt;
    //uint32_t erSearch;
    //uint32_t erSearchAptt;
    uint32_t errors;

    // threats
    struct pt search_pt; // threat
    struct pt term_pt;


} modWire_master_s;




modWire_master_s m1Wire; // ������� ���������


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
#define OW_PORT             //GPIOA		//������� ���� ow
#define OW_N_PIN            7		//������� ����� ���� ow

#define OW_PIN_MASK         (1 << OW_N_PIN)
#define low_ow_port         do { DDRD |=  OW_PIN_MASK; } while (0)	//�������� ���� � �����
#define hi_ow_port          do { DDRD &= ~OW_PIN_MASK; } while (0)		//��������� ����
#define ow_pin              (PIND & OW_PIN_MASK)

void ONE_WIRE_USART_INIT (void)
{
    PORTD &= ~OW_N_PIN;
}



/**
  * @brief  �������� ����
  * @param  Bit
  * @retval None
  */
msg_t ONE_WIRE_USART_SEND_S (uint8_t dat)
{
	ATOMIC_BLOCK_FORSEON (
        if (0x00 != dat)
        {
            low_ow_port;    //�������� ���� � �����
            __delay_1us2(4); //delay_us(5);    // ������ �������, �� 1 �� 15 ��� (� ������ ������� �������������� ������)
            hi_ow_port;
            __delay_1us2(75); //delay_us(90);   // �������� �� ���������� ��������� (�� ����� 60 ���)
        } else {
            low_ow_port;    //�������� ���� � �����
            __delay_1us2(75); //delay_us(90);   // ������ ������� �� ���� �������� (�� ����� 60 ���, �� ����� 120 ���)
            hi_ow_port;
            __delay_1us2(4); //delay_us(5);    // ����� �������������� �������� ������� �� ���� + 1 ��� (�������)
        }
    )
    return FUNCTION_RETURN_OK;
}



/**
  * @brief  ������ �����
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
  * @brief  ������ ����
  * @param  None
  * @retval Bit
  */
msg_t ONE_WIRE_USART_RECV_S (uint8_t *pDat)
{
    uint8_t val;
    ATOMIC_BLOCK_FORSEON (
        low_ow_port;		//�������� ���� � �����
        __delay_1us2(2); //delay_us(2);		//������������ ������� ������, ������� 1 ���
        hi_ow_port;			//��������� ����	 
        __delay_1us2(7); //delay_us(8);		// ����� �� ������� �������������, ����� �� ����� 15 ���
        val = ow_pin;		//��������� ����
        __delay_1us2(46); //delay_us(55);		// �������� �� ���������� ����-�����, ������� 60 ��� � ������ ������� ������
    )
    if (0 != val)
	    *pDat = 0xFF;
    else
        *pDat = 0x00;
    return FUNCTION_RETURN_OK;
}


/**
  * @brief  ������ �����
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
{ //����� ����
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
    
    if (ow_pin == 0) //��������� ����� �� ���������� ���������
        return respond;
    low_ow_port;					//�������� ���� � �����
    delay_us(500);				//����� 480..960 ���
    ATOMIC_BLOCK_FORSEON (
        hi_ow_port;						//��������� ����
        //delay_us(2); // ����� ����������� �������������� ���������, ����� ������� ������� ������� �� ����
        delay_us(50);					//����� 50 �����������
        respond = ow_pin;			//��������� ����
    )
    delay_us(450);				//��������� ��������� ������������� 
    //__exit_critical(); //
    
    
    if(respond) 
        return respond;	//������ �� ��������� 
        
    LED_TEST_ON;
    return respond = FUNCTION_RETURN_OK;							//������ ��������� 
      
  
    
/*
    if (FALSE == m1Wire.tMode)
    {
        ONE_WIRE_USART_SET_BAUD ((uint16_t)BAUD_9600); // �������������� UART �� ����� ��������
        ONE_WIRE_USART_CLEAR;
        ONE_WIRE_USART_SEND_S (0xF0);
        m1Wire.tMode = TRUE;
    }
    if (FUNCTION_RETURN_OK == ONE_WIRE_USART_RECV_S (&m1Wire.tmp8))
    { // @todo �������� �� ����� ���, ���� ��� ���� USARTy
    // @todo �������� ��������, ���� ���������� � �������� ����������� ds1994 � ds2404
        if (m1Wire.tmp8 == 0x00)
        { // ���������, ���������� �� �����  
            respond = FUNCTION_RETURN_ERROR; // � ��� ��
            m1Wire.errors++;
        } else if (m1Wire.tmp8 == 0xF0) 
        {
            respond = FUNCTION_RETURN_ERROR; // � ��� �����
            m1Wire.errors++;
        }
        else if ((m1Wire.tmp8 == 0xE0) || (m1Wire.tmp8 == 0xC0))
        {
            respond = FUNCTION_RETURN_OK; // ���� ���������� �� ����
        }
        ONE_WIRE_USART_SET_BAUD ((uint16_t)BAUD_115200); // ���������� ����������� ��������
        ONE_WIRE_USART_CLEAR;
        m1Wire.tMode = FALSE;
    }
    */
    return respond;
}


msg_t mod1Wire_xBit (uint8_t wrBit, uint8_t *rdBit)
{ // �����\�������� ����
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
{ // ���������� ������� ����� ������
    msg_t    respond = FUNCTION_RETURN_ERROR;
    uint8_t  i;

    if (FALSE == m1Wire.tMode) {
        for (i = 0; i < 8; i++) { // ������ "�����"
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
    *rdByte = 0; // �������������
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
//        msg_t respond = FUNCTION_RETURN_ERROR; // ��� ������� �������� ����
    
    PT_BEGIN(pt); // ��� ������� �������� ����
    if (FALSE == m1Wire.lastDeviceFlag) { // ��� ������� ���� ������ � �������� ���, ��� ��� �������
        //m1Wire.erSearchAptt++;
        m1Wire.currentCollision = 0;
        m1Wire.crc8 = 0; // ��������, ����������� ����� ������ �������
        // ������� �� ������ ������ ����������
        PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte(DS_COMMAND_SEARCH_ROM, &m1Wire.tmp8));
        // �������� ���� ������, �� ���� ������ ������ ��������� ���� �����
        for (m1Wire.numBit = 1; m1Wire.numBit <= 64; m1Wire.numBit++) {
            PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xBit(1, &m1Wire.IDBit)); // ������ ������ ���
            // ������ ������ ��� (���������������)
            PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xBit(1, &m1Wire.cmpIDBit));
            if ((0 != m1Wire.IDBit) && (0 != m1Wire.cmpIDBit)) { // ��� �������, ���-�� ���������� � ����������� �����
//                    respond = FUNCTION_RETURN_ERROR; // ���� ���, �� �� ���� ������ ���, � �������� ��� �� ������ ���� ������, ��� ������ ������� �� ����� ����� ����
                break;
            }
            else if (m1Wire.IDBit != m1Wire.cmpIDBit) // ���� ���� ������, �� ���� ������
            { 
                m1Wire.currentSelection = m1Wire.IDBit;
            } else {
                // ��������, ���� ���, �� ������ ���� ����, ����� ������, ���� ���� � ��� ������...
                if (m1Wire.numBit < m1Wire.lastCollision) 
                {
                    if ((m1Wire.wireS[m1Wire.numFound].ROM[(m1Wire.numBit - 1) >> 3] & 1 << ((m1Wire.numBit - 1) & 0x07)) > 0) // ���� ������� ��� ����� 1
                    {
                        m1Wire.currentSelection = 1;

                        // ���� ����� �� ������ �����, ���������� ����� ����
                        if (m1Wire.currentCollision < m1Wire.numBit) 
                        {
                            m1Wire.currentCollision = m1Wire.numBit;
                        }
                    } else {
                        m1Wire.currentSelection = 0;
                    }
                } else {
                    if (m1Wire.numBit == m1Wire.lastCollision) // ���� �����, �� ���  ���� �������� ������
                    {
                        m1Wire.currentSelection = 0;
                    } else {
                        m1Wire.currentSelection = 1; // ���� �� ������ �����
                        if (m1Wire.currentCollision < m1Wire.numBit) // � ���������� ����� ����
                        {
                            m1Wire.currentCollision = m1Wire.numBit;
                        }
                    }                   
                }
            }
            if (m1Wire.currentSelection == 0) // ��������� ��� ����� ���� ROM ����� �� ������ rom_byte_number
            {
                m1Wire.wireS[m1Wire.numFound].ROM[(m1Wire.numBit - 1) >> 3] &= ~(1 << ((m1Wire.numBit - 1) & 0x07));
            } else {
                m1Wire.wireS[m1Wire.numFound].ROM[(m1Wire.numBit - 1) >> 3] |= 1 << ((m1Wire.numBit - 1) & 0x07);
            }
            // ���������� ��� ����������� ������
            PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xBit(m1Wire.currentSelection, &m1Wire.tmp8));
            if ((m1Wire.numBit & 0x07) == 0) { // ��� �� � ����� ������� CRC
                crc8_DS_s(&m1Wire.crc8, &m1Wire.wireS[m1Wire.numFound].ROM[(m1Wire.numBit - 1) >> 3]);
            }
        }
        // ���� ����� �������, �� CRC ������ ���� ����� 0
        if ((m1Wire.numBit > 64) && (m1Wire.crc8 == 0)) { //  �������� �� ��� ����, ��� ����� �� ���� ����, ��� ���� ������� �������, �� ������ ���� 64
            m1Wire.errors++;
            if (m1Wire.currentCollision == 0) // ��� �� ���� ��������, ������ ���,
            {
                m1Wire.lastDeviceFlag = TRUE; // �� ����� ��������� ����������
            }
            else
            {
                m1Wire.numFound++;
                m1Wire.lastCollision = m1Wire.currentCollision;
            }
            
            if (MOD_WIRE_SLAVES_MAX <= m1Wire.numFound)
            {
                m1Wire.lastDeviceFlag = TRUE; // ������, ��������� �� �������
            }
//                respond = FUNCTION_RETURN_OK; // ����� �������� ��������
        } else {//if (!respond || !ROM_NO[0]) // ���� �� ����� ���������� then reset counters so next 'search' will be like a first
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
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_resetWire()); // ����� ����
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte (DS1821_COMMAND_CONVERT_TEMP_START, &m1Wire.tmp8)); // �������� ������� ������ ������������� ��������������
    
    m1Wire.timerA = modSysClock_getTime(); // ���� ���� DS18B20 ���������� ������
    PT_WAIT_UNTIL(pt, modSysClock_getPastTime (m1Wire.timerA, SYSCLOCK_GET_TIME_MS_1) >= 10);
    
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_resetWire()); // ����� ����
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte (DS1821_COMMAND_CONVERT_TEMP_READ, &m1Wire.tmp8)); // ������ ������ �� �������
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte (0xFF, &m1Wire.tmp_L));

    if ((m1Wire.tmp_L & 0x80) == 0) { //��������� ���� 
        m1Wire.wireS[0].sign = '+';
    } else {
        m1Wire.wireS[0].sign = '-'; 
    }
    m1Wire.wireS[0].tempInt = m1Wire.tmp_L;
    m1Wire.wireS[0].tempFract = 0;
    m1Wire.wireS[0].status = TRUE;
#endif

    
#if (1 == MOD_WIRE_TYPE_DS18B20)
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_resetWire ()); // ����� ����
#if (1 == MOD_WIRE_SLAVES_MAX)
    // ���� ���������� ����, �� ����� ������ ��� ID, ��� ������
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte (DS_COMMAND_READ_ROM, &m1Wire.tmp8));
    m1Wire.crc8 = 0x00;
    for (m1Wire.jm = 0; m1Wire.jm < 8; m1Wire.jm++)
    { // ������ �����
        PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte (0xFF, &m1Wire.wireS[0].ROM[m1Wire.jm]));
        crc8_DS_s(&m1Wire.crc8, &m1Wire.wireS[0].ROM[m1Wire.jm]);
    }
#else
    PT_WAIT_THREAD(pt, search(&m1Wire.search_pt)); // �����
#endif
    // @todo ������� �������� �� ������� ����������
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_resetWire()); // ����� ����
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte(DS_COMMAND_SKIP_ROM, &m1Wire.tmp8)); // ������� �������� ���
    PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte(DS18B20_COMMAND_CONVERT_TEMP, &m1Wire.tmp8)); // �������� ������� ������ ������������� ��������������
    
    m1Wire.timerA = modSysClock_getTime(); // ���� ���� DS18B20 ���������� ������
    PT_WAIT_UNTIL(pt, modSysClock_getPastTime(m1Wire.timerA, SYSCLOCK_GET_TIME_MS_1) >= 1000);
    
    for (m1Wire.in = 0; m1Wire.in < MOD_WIRE_SLAVES_MAX; m1Wire.in++)
    { // ������ ����������� �� ���������
        if (OW_DS18B20_FAMILY_CODE == m1Wire.wireS[m1Wire.in].ROM[0])
        {
            PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_resetWire());
            PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte(DS_COMMAND_MATCH_ROM, &m1Wire.tmp8)); // �������� ����� ����������
            for (m1Wire.jm = 0; m1Wire.jm < 8; m1Wire.jm++)  // �������� � ���� 64 ���� ������
                PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte(m1Wire.wireS[m1Wire.in].ROM[m1Wire.jm], &m1Wire.tmp8));
        }
        //PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == xByte(DS_COMMAND_SKIP_ROM, &tmp8)); // ������� �������� ���
        PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte(DS18B20_COMMAND_READ_SCRATCHPAD, &m1Wire.tmp8)); // ������ ������ �� �������
        PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte(0xFF, &m1Wire.tmp_L));
        PT_WAIT_UNTIL(pt, FUNCTION_RETURN_OK == mod1Wire_xByte(0xFF, &m1Wire.tmp_H));
        tmp = ((uint16_t)m1Wire.tmp_H << 8 ) | m1Wire.tmp_L; // ���������� ������� � ������ ����� ��� �������
        if ((m1Wire.tmp_H & 0x80) == 0) { //��������� ������� ������ 
            m1Wire.wireS[m1Wire.in].sign = '+';
        } else {
            tmp = ~tmp + 1;
            m1Wire.wireS[m1Wire.in].sign = '-'; 
        }
        m1Wire.tmp_H = tmp >> 4; //(unsigned short)((tmp_L & 0x07) << 4) | (tmp_H >> 4);
        m1Wire.wireS[m1Wire.in].tempInt = m1Wire.tmp_H;
        m1Wire.tmp_L = (uint16_t)(tmp & 0x0F); //����������� ������� � ����� �����
        m1Wire.tmp_L = (m1Wire.tmp_L << 1) + (m1Wire.tmp_L << 3); // �������� �� 10
        m1Wire.tmp_L = (m1Wire.tmp_L >> 4); //����� �� 16 ��� �������� �� 0.0625
        m1Wire.wireS[m1Wire.in].tempFract = m1Wire.tmp_L;
        // wireS[in].temp_Float = tmp_H * 1.0 + (tmp_L * 0.01); // ��������� ����� � ��������� ������
        m1Wire.wireS[m1Wire.in].status = TRUE;

    }
#endif // MOD_WIRE_TYPE_DS18B20
    PT_END(pt);
}

#endif
   


msg_t mod1Wire_Master_getTemperature (uint8_t number, uint8_t *tI, uint8_t *tF){ //����� ����
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
 * ������� ������������� ������, �����, ������ ����������������,
 */   
void mod1Wire_initMaster (void) {
    uint8_t i;
    ONE_WIRE_USART_INIT; // �������������� UART ( @todo GPIO �������� ��� )
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
 * ������ � ���������� ����������� 1-Wire ����, � ������ ������ - ������ � ������������� DS18B20
 */
msg_t mod1Wire_runMaster (void) {
    msg_t respond = FUNCTION_RETURN_OK;
    
    getTemp(&m1Wire.term_pt); // �������� ������ � DS18B20
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
    0xF4, // CRC, ������ ���������� ��� ��������� �����������!
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
static uint8_t    bitS; // ��� �� ������
uint8_t           slaveRW_count;
uint8_t          *pData; //  ��������� ������

struct pt slaveState_pt; // ����������




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

//  ����� �����, ������� �����
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
    // �������������� ������� ������
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
    //if (EXTI->PR & (1 << 10)) // ����� � ������
    {
        __disable_irq(); // off interrupt for noise
        MOD_WIRE_OPERATE(bitS, &bitRd);
        TCK_CLR; // ���������� ������� ��������� RESET-�
        while(1)
        {
            if (0 == MOD_WIRE_IN) // ���� �������, ������
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
        
        slaveState(&slaveState_pt); // ��� �����
        
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
    
    MOD_WIRE_H; // �� ��������� ������� � �����
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource10);
    
    EXTI_InitStructure.EXTI_Line = EXTI_Line10;  //!
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; // ���������� (� �� �������)
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // ����������� �� ������� ������ ��������
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; // ���
    EXTI_Init(&EXTI_InitStructure);
    
    NVIC_EnableIRQ(EXTI15_10_IRQn);
    NVIC_SetPriority(EXTI15_10_IRQn, 2);
    
//     ROM[1] = _rand8();
//     ROM[2] = _rand8();
//     ROM[3] = _rand8();
//     ROM[4] = _rand8();
//     ROM[5] = _rand8();
//     ROM[6] = _rand8();

    // ��� ����, ����� ��������� CRC, �����������������
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
//     RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; //������ �������� �������� �� ������ 2
//     TIM2->PSC = (12 - 1); //����������� ������� 2096 (1 msec)
//     TIM2->ARR = 10000;
//     TIM2->CR1 &= ~TIM_CR1_DIR;
//     TIM2->CR1 |= TIM_CR1_CEN; //������ �����  
//     
    // // // void delay_init (void)
// // // {
// // //     RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
// // //     TIM2->CR1 = 0;                                    
// // //     TIM2->CR2 = 0;
// // //     TIM2->PSC = 0;//0;//0;//(12 - 1); // ����������� �������
// // //     TIM2->ARR = 0;//10000;
// // // //     TIM2->CR1 &= ~TIM_CR1_DIR;
// // // //     TIM2->CR1 |= TIM_CR1_CEN; // ������ �����  
// // //     TIM2->CR1 |= TIM_CR1_DIR | TIM_CR1_OPM;// | TIM_CR1_ARPE;
// // //     
// // // }


// // // inline void _T_delay_us (uint16_t us)
// // // {
// // //     TIM2->ARR = us;
// // //     TIM2->CR1 |= TIM_CR1_CEN; // ������ ����� 
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
