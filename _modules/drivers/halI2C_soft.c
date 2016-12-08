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


//Отправка СТОПа на шину
void    halI2C1_STOP (void);
//Отправка СТАРТа на шину
msg_t   halI2C1_START (void);
//Отправка ACK подчиненному на шину
void    halI2C1_send_ACK (void);
//Тут наоборот, не отправляем ACK на шину, а оставляем линию висеть
void    halI2C1_send_NACK (void);
//Функция ожидания подтверждения
msg_t   halI2C1_check_ACK (void);



#if HAL_I2C1


void soft_halI2C1_STOP (void)
{
    SCL1_L; //Сначала опускаем SCL
    I2C1_DELAY; //++
    SDA1_L; //Потом SDA
    I2C1_DELAY;
    SCL1_H; //Затем даем стоп, поднимая сначала SCL
    I2C1_DELAY;
    SDA1_H; //А потом SDA
    I2C1_DELAY;
}


msg_t soft_halI2C1_START (void)
{
    SDA1_H;
    SCL1_H;
    I2C1_DELAY;
    if ((SDA1_IN) == 0)
        return FUNCTION_RETURN_ERROR; //Если шину прижал кто-то другой, значит она занята. Уходим
    SDA1_L; //Даем старт на шину
    I2C1_DELAY;
    if (0 != (SDA1_IN))
        return FUNCTION_RETURN_ERROR; //Если шина не опустилась, то неполадка. Уходим
    SCL1_L;      
    I2C1_DELAY;
    return FUNCTION_RETURN_OK; //Возвращаем TRUE, если все прошло хорошо
}


void soft_halI2C1_send_ACK (void)
{
    SCL1_L;      
    I2C1_DELAY;
    SDA1_L;       //Прижимаем SDA к земле
    I2C1_DELAY;
    SCL1_H;       //В то время как отпускаем линию SCL
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
    SCL1_H;  //Отпускаем линию SCL
    I2C1_DELAY;
    if (0 != SDA1_IN) { //И читаем, что делает с линией подчиненный
        SCL1_L; //Если линия не прижата, то подтверждения небыло
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
        if (0 == SDA1_IN) { // проверяю, занята ли шина I2C (прижата ли к земле)
            while (i--) { // здесь мы можем быть максимум 9 раз (крайний случай, смотри в pdf)
            // дёргаю SCL (тактирую Slave)
                SCL1_L; // SCL = 0
                I2C1_DELAY;
                SCL1_H; // SCL = 1
                I2C1_DELAY;
               
                if (0 != SDA1_IN) { // смотрю, отпустил ли Slave SDA (SDA == 1 ?)
                // если да - настраиваю выводы на выход и делаю Stop состояние
                    soft_halI2C1_STOP ();
        //             SCL_L;
        //             halI2C_delay ();
        //             SDA_L;
        //             halI2C_delay ();
        //             SCL_H;
        //             halI2C_delay ();
        //             SDA_H;
                    // после этого шина I2C свободна и готова к работе
                    respond = FUNCTION_RETURN_OK;
                    break; // выходим из цикла
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
                SCL1_L;              //Опускаем CLK линию
                I2C1_DELAY;
                if (data & 0x80)
                {  //Если старший бит в байте установлен
                    SDA1_H;
                } else {
                    SDA1_L;
                }
                data = (data << 1); //Сдвигаем все биты влево. На место старшего бита становится более младший
                I2C1_DELAY;     //После седьмого сдвига на месте старшего бита окажется самый младший бит.
                SCL1_H;              //Возвращаем линию как было
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
    soft_halI2C1_STOP (); // @todo сделать проверку на корректное завершение

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
            SCL1_L;              //Опускаем CLK линию
            I2C1_DELAY;
            if (data & 0x80)
            {  //Если старший бит в байте установлен
                SDA1_H;
            } else {
                SDA1_L;
            }
            data = (data << 1); //Сдвигаем все биты влево. На место старшего бита становится более младший
            I2C1_DELAY;     //После седьмого сдвига на месте старшего бита окажется самый младший бит.
            SCL1_H;              //Возвращаем линию как было
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
                SCL1_L;              //Опускаем CLK линию
                I2C1_DELAY;
                data = (data << 1); //Сдвигаем все биты влево. На место старшего бита становится более младший
                I2C1_DELAY;     //После седьмого сдвига на месте старшего бита окажется самый младший бит.
                SCL1_H;              //Возвращаем линию как было
                I2C1_DELAY;
                if (0 != SDA1_IN)
                {   // читаем состояние линии SDA
                    data |= 0x01;    //Результат чтения помещяем на место самого младшего бита в приемном байте
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
    soft_halI2C1_STOP (); // @todo сделать проверку на корректное завершение

    return FUNCTION_RETURN_OK; //respond; //FUNCTION_RETURN_OK
}


msg_t   soft_halI2C_get_errors (void) {
    return FUNCTION_RETURN_OK;
}

#endif
