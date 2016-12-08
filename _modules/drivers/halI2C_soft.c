#include "halI2C.h"
#include "board.h"

void halI2C1_delay (void)
{ 
    volatile uint16_t i = 50; //max 65535
    while (i--) {};
}


#if (HAL_I2C1)
    #ifndef SCL1_H
        #error "SCL1_H not defined!"
    #endif
    #ifndef SCL1_L
        #error "SCL1_L not defined!"
    #endif
    #ifndef SDA1_H
        #error "SDA1_H not defined!"
    #endif
    #ifndef SDA1_L
        #error "SDA1_L not defined!"
    #endif
    #ifndef SCL1_IN
        #error "SCL1_IN not defined!"
    #endif
    #ifndef SDA1_IN
        #error "SDA1_IN not defined!"
    #endif

    #ifndef I2C1_INIT
        #error "I2C1_INIT not defined!"
    #endif
    #ifndef I2C1_DEINIT
        #error "I2C1_DEINIT not defined."
    #endif
    #ifndef I2C1_DELAY
        #warning "I2C1_DELAY not defined, used default!"
        #define I2C1_DELAY halI2C1_delay()
    #endif
#endif


//�������� ����� �� ����
void    halI2C1_STOP (void);
//�������� ������ �� ����
msg_t   halI2C1_START (void);
//�������� ACK ������������ �� ����
void    halI2C1_send_ACK (void);
//��� ��������, �� ���������� ACK �� ����, � ��������� ����� ������
void    halI2C1_send_NACK (void);
//������� �������� �������������
msg_t   halI2C1_check_ACK (void);



#if HAL_I2C1


void soft_halI2C1_STOP (void)
{
    SCL1_L; //������� �������� SCL
    I2C1_DELAY; //++
    SDA1_L; //����� SDA
    I2C1_DELAY;
    SCL1_H; //����� ���� ����, �������� ������� SCL
    I2C1_DELAY;
    SDA1_H; //� ����� SDA
    I2C1_DELAY;
}


msg_t soft_halI2C1_START (void)
{
    SDA1_H;
    SCL1_H;
    I2C1_DELAY;
    if ((SDA1_IN) == 0)
        return FUNCTION_RETURN_ERROR; //���� ���� ������ ���-�� ������, ������ ��� ������. ������
    SDA1_L; //���� ����� �� ����
    I2C1_DELAY;
    if (0 != (SDA1_IN))
        return FUNCTION_RETURN_ERROR; //���� ���� �� ����������, �� ���������. ������
    SCL1_L;      
    I2C1_DELAY;
    return FUNCTION_RETURN_OK; //���������� TRUE, ���� ��� ������ ������
}


void soft_halI2C1_send_ACK (void)
{
    SCL1_L;      
    I2C1_DELAY;
    SDA1_L;       //��������� SDA � �����
    I2C1_DELAY;
    SCL1_H;       //� �� ����� ��� ��������� ����� SCL
    I2C1_DELAY;
    SCL1_L;
    I2C1_DELAY;
    SDA1_H;
}
 

void soft_halI2C1_send_NACK (void)
{
    SCL1_L;
    I2C1_DELAY;
    SDA1_H;
    I2C1_DELAY;
    SCL1_H;
    I2C1_DELAY;
    SCL1_L;
    I2C1_DELAY;
}
 

msg_t   soft_halI2C1_check_ACK (void)
{
    SCL1_L;
    I2C1_DELAY;
    SDA1_H;
    I2C1_DELAY;
    SCL1_H;  //��������� ����� SCL
    I2C1_DELAY;
    if (0 != SDA1_IN) { //� ������, ��� ������ � ������ �����������
        SCL1_L; //���� ����� �� �������, �� ������������� ������
        return FUNCTION_RETURN_ERROR;
    }
    SCL1_L;
    return FUNCTION_RETURN_OK;
}


void soft_halI2C_init (void) // init GPIO
{
    I2C1_INIT;
}


msg_t soft_halI2C_re_init (void)
{
	msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t i = 10;
    
        // http://we.easyelectronics.ru/STM32/stm32-i2c-eeprom-24sxx.html
        //http://www.analog.com/static/imported-files/application_notes/54305147357414AN686_0.pdf
        if (0 == SDA1_IN) { // ��������, ������ �� ���� I2C (������� �� � �����)
            while (i--) { // ����� �� ����� ���� �������� 9 ��� (������� ������, ������ � pdf)
            // ����� SCL (�������� Slave)
                SCL1_L; // SCL = 0
                I2C1_DELAY;
                SCL1_H; // SCL = 1
                I2C1_DELAY;
               
                if (0 != SDA1_IN) { // ������, �������� �� Slave SDA (SDA == 1 ?)
                // ���� �� - ���������� ������ �� ����� � ����� Stop ���������
                    soft_halI2C1_STOP ();
        //             SCL_L;
        //             halI2C_delay ();
        //             SDA_L;
        //             halI2C_delay ();
        //             SCL_H;
        //             halI2C_delay ();
        //             SDA_H;
                    // ����� ����� ���� I2C �������� � ������ � ������
                    respond = FUNCTION_RETURN_OK;
                    break; // ������� �� �����
                }
            }
        }
    return respond;
}


void soft_halI2C_deInit (void)
{ // init GPIO
    I2C1_DEINIT;
}


msg_t   soft_halI2C_transmit (uint8_t adr, uint8_t *txbuf, uint8_t txbytes, systime_t timeout) {
    //msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t i, data;

    soft_halI2C1_STOP ();
    adr = adr << 1;
    if (FUNCTION_RETURN_OK == soft_halI2C1_START ())
    {
        data = (adr & ~I2C_BIT_RW);
        do {
            for (i = 0; i < 8; i++)
            {
                SCL1_L;              //�������� CLK �����
                I2C1_DELAY;
                if (data & 0x80)
                {  //���� ������� ��� � ����� ����������
                    SDA1_H;
                } else {
                    SDA1_L;
                }
                data = (data << 1); //�������� ��� ���� �����. �� ����� �������� ���� ���������� ����� �������
                I2C1_DELAY;     //����� �������� ������ �� ����� �������� ���� �������� ����� ������� ���.
                SCL1_H;              //���������� ����� ��� ����
                I2C1_DELAY;
                SCL1_L;
            }
            if (FUNCTION_RETURN_OK != soft_halI2C1_check_ACK ())
            {
                return FUNCTION_RETURN_ERROR; //break;
            } 
            data = *txbuf++;
        } while (0 != txbytes--); 
    } else {
        return FUNCTION_RETURN_ERROR;
    }
    soft_halI2C1_STOP (); // @todo ������� �������� �� ���������� ����������

    return FUNCTION_RETURN_OK;
}


msg_t   soft_halI2C_receive (uint8_t adr, uint8_t *rxbuf, uint8_t rxbytes, systime_t timeout)
{
    //msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t i, data;

    soft_halI2C1_STOP ();
    adr = adr << 1;
    if (FUNCTION_RETURN_OK == soft_halI2C1_START ())
    {
        data = (adr | I2C_BIT_RW);
        for (i = 0; i < 8; i++) {
            SCL1_L;              //�������� CLK �����
            I2C1_DELAY;
            if (data & 0x80)
            {  //���� ������� ��� � ����� ����������
                SDA1_H;
            } else {
                SDA1_L;
            }
            data = (data << 1); //�������� ��� ���� �����. �� ����� �������� ���� ���������� ����� �������
            I2C1_DELAY;     //����� �������� ������ �� ����� �������� ���� �������� ����� ������� ���.
            SCL1_H;              //���������� ����� ��� ����
            I2C1_DELAY;
        }
        if (FUNCTION_RETURN_OK != soft_halI2C1_check_ACK ()) // adress good?
        {
            return FUNCTION_RETURN_ERROR; //break;
        } 
        while (0 != rxbytes--) // read bytes
        {
            data = 0;
            for (i = 0; i < 8; i++)
            {
                SCL1_L;              //�������� CLK �����
                I2C1_DELAY;
                data = (data << 1); //�������� ��� ���� �����. �� ����� �������� ���� ���������� ����� �������
                I2C1_DELAY;     //����� �������� ������ �� ����� �������� ���� �������� ����� ������� ���.
                SCL1_H;              //���������� ����� ��� ����
                I2C1_DELAY;
                if (0 != SDA1_IN)
                {   // ������ ��������� ����� SDA
                    data |= 0x01;    //��������� ������ �������� �� ����� ������ �������� ���� � �������� �����
                }
            }
            if (0 == rxbytes)
            {
                soft_halI2C1_send_NACK ();
            } else {
                soft_halI2C1_send_ACK ();
            }
            *rxbuf++ = data;
        }
    } else {
        return FUNCTION_RETURN_ERROR;
    }
    soft_halI2C1_STOP (); // @todo ������� �������� �� ���������� ����������

    return FUNCTION_RETURN_OK; //respond; //FUNCTION_RETURN_OK
}


msg_t   soft_halI2C_get_errors (void) {
    return FUNCTION_RETURN_OK;
}

#endif
