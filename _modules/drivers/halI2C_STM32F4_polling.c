#include "halI2C.h"
#include "board.h"

// http://easyelectronics.ru/interface-bus-iic-i2c.html

#ifndef SCL_H
    #error "SCL_H not defined!"
#endif
#ifndef SCL_L
    #error "SCL_L not defined!"
#endif
#ifndef SDA_H
    #error "SDA_H not defined!"
#endif
#ifndef SDA_L
    #error "SDA_L not defined!"
#endif
#ifndef SCL_IN
    #error "SCL_IN not defined!"
#endif
#ifndef SDA_IN
    #error "SDA_IN not defined!"
#endif

#ifndef I2C_INIT
    #error "I2C_INIT not defined!"
#endif
#ifndef I2C_DEINIT
    #error "I2C_DEINIT not defined!"
#endif
#ifndef I2C_DELAY
    #error "I2C_DELAY not defined!"
#endif

void halI2C_delay (void) {
    uint8_t i = I2C_DELAY;
    while(i) {
        i--;
    }
}

void halI2C_init (void) { // init GPIO

           I2C_InitTypeDef  I2C_InitStructure;
           GPIO_InitTypeDef  GPIO_InitStructure;

           RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);

           RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB| RCC_APB2Periph_AFIO , ENABLE);//

           /* Configure I2C1 pins: PB6->SCL and PB7->SDA */
           GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
           GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
           GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
           GPIO_Init(GPIOB, &GPIO_InitStructure);

           I2C_DeInit(I2C_EE);
           I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
           I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_16_9;
           I2C_InitStructure.I2C_OwnAddress1 = 1;
           I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
           I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
           I2C_InitStructure.I2C_ClockSpeed = 100000;  /* 100kHz */

           I2C_Cmd(I2C_EE, ENABLE);
           I2C_Init(I2C_EE, &I2C_InitStructure);
           I2C_AcknowledgeConfig(I2C_EE, ENABLE);
}


msg_t halI2C_reInit (void) {
	msg_t respond = FUNCTION_RETURN_OK;

   //if(I2C1->SR2 & I2C_SR2_BUSY) // проверяю, занята ли шина I2C (взведён ли флаг BUSY)
    if (0 == SDA_IN) {
    
        while(1) { // здесь мы можем быть максимум 9 раз (крайний случай, смотри в pdf)
        // дёргаю SCL (тактирую Slave)
            SCL_L; // SCL = 0
            halI2C_delay ();
            SCL_H; // SCL = 1
            halI2C_delay ();
           
            if (0 != SDA_IN) { // смотрю, отпустил ли Slave SDA (SDA == 1 ?)
            // если да - настраиваю выводы на выход и делаю Stop состояние
                halI2C_STOP ();
    //             SCL_L;
    //             halI2C_delay ();
    //             SDA_L;
    //             halI2C_delay ();
    //             SCL_H;
    //             halI2C_delay ();
    //             SDA_H;
                break; // выходим из цикла
            }
        }
    }
    // после этого шина I2C свободна и готова к работе
    return respond;
}


void halI2C_deInit (void) { // init GPIO
    I2C_DEINIT;
}

void halI2C_STOP (void) {
    SCL_L; //Сначала опускаем SCL
    halI2C_delay();
    SDA_L; //Потом SDA
    halI2C_delay();
    SCL_H; //Затем даем стоп, поднимая сначала SCL
    halI2C_delay();
    SDA_H; //А потом SDA
    halI2C_delay();
}


msg_t halI2C_START (void) {
    SDA_H;
    SCL_H; //Опускаем SCL
    halI2C_delay();
    if ((SDA_IN) == 0)
        return FUNCTION_RETURN_ERROR; //Если шину прижал кто-то другой, значит она занята. Уходим
    SDA_L; //Даем старт на шину
    halI2C_delay();
    if (0 != (SDA_IN))
        return FUNCTION_RETURN_ERROR; //Если шина не опустилась, то неполадка. Уходим
    SDA_L;      
    halI2C_delay();
    return FUNCTION_RETURN_OK; //Возвращаем TRUE, если все прошло хорошо
}


void halI2C_ACK (void) {
    SCL_L;      
    halI2C_delay();
    SDA_L;       //Прижимаем SDA к земле
    halI2C_delay();
    SCL_H;       //В то время как отпускаем линию SCL
    halI2C_delay();
    SCL_L;
    halI2C_delay();
}
 

void halI2C_NACK (void) {
    SCL_L;
    halI2C_delay();
    SDA_H;       //Не прижимаем SDA к земле
    halI2C_delay();
    SCL_H;       //В то время как отпускаем линию SCL
    halI2C_delay();
    SCL_L;
    halI2C_delay();
}
 

msg_t   halI2C_waitACK (void) {
    SCL_L;
    halI2C_delay();
    SDA_H;
    halI2C_delay();
    SCL_H;  //Отпускаем линию SCL
    halI2C_delay();
    if (0 != SDA_IN) { //И читаем, что делает с линией подчиненный
        SCL_L; //Если линия не прижата, то подтверждения небыло
        return FUNCTION_RETURN_ERROR;
    }
    SCL_L;
    return FUNCTION_RETURN_OK;
}


msg_t   halI2C_transmitTimeout (uint8_t num, uint8_t adr, uint8_t *txbuf, uint8_t txbytes, systime_t timeout) {
    //msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t i, data;
    
    if (FUNCTION_RETURN_OK == halI2C_START ()) {
        data = (adr & ~I2C_BIT_RW);
        do {
            for (i = 0; i < 8; i++) {
                SCL_L;              //Опускаем CLK линию
                halI2C_delay();
                if (data & 0x80) {  //Если старший бит в байте установлен
                    SDA_H;
                } else {
                    SDA_L;
                }
                data = (data << 1); //Сдвигаем все биты влево. На место старшего бита становится более младший
                halI2C_delay ();     //После седьмого сдвига на месте старшего бита окажется самый младший бит.
                SCL_H;              //Возвращаем линию как было
                halI2C_delay ();
                SCL_L;
            }
            if (FUNCTION_RETURN_OK != halI2C_waitACK ()) {
                return FUNCTION_RETURN_ERROR; //break;
            } 
            data = *txbuf++; //  @todo inogda budet chitatsa error!
        } while (txbytes--); 
    } else {
        return FUNCTION_RETURN_ERROR;
    }
    halI2C_STOP (); // @todo сделать проверку на корректное завершение
    
    return FUNCTION_RETURN_OK;
}


msg_t   halI2C_receiveTimeout (uint8_t num, uint8_t adr, uint8_t *rxbuf, uint8_t rxbytes, systime_t timeout) {
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint8_t i, data;
    
    if (FUNCTION_RETURN_OK == halI2C_START ()) {
        data = (adr | I2C_BIT_RW);
        for (i = 0; i < 8; i++) {
            SCL_L;              //Опускаем CLK линию
            halI2C_delay();
            if (data & 0x80) {  //Если старший бит в байте установлен
                SDA_H;
            } else {
                SDA_L;
            }
            data = (data << 1); //Сдвигаем все биты влево. На место старшего бита становится более младший
            halI2C_delay ();     //После седьмого сдвига на месте старшего бита окажется самый младший бит.
            SCL_H;              //Возвращаем линию как было
            halI2C_delay ();
            SCL_L;
        }
        if (FUNCTION_RETURN_OK != halI2C_waitACK ()) { // adress good
            return FUNCTION_RETURN_ERROR; //break;
        } 
        do { // read bytes
            for (i = 0; i < 8; i++) {
                SCL_L;              //Опускаем CLK линию
                halI2C_delay();
                if (data & 0x80) {  //Если старший бит в байте установлен
                    SDA_H;
                } else {
                    SDA_L;
                }
                data = (data << 1); //Сдвигаем все биты влево. На место старшего бита становится более младший
                halI2C_delay ();     //После седьмого сдвига на месте старшего бита окажется самый младший бит.
                SCL_H;              //Возвращаем линию как было
                halI2C_delay ();
                SCL_L;
            }
            if (FUNCTION_RETURN_OK != halI2C_waitACK ()) {
                return FUNCTION_RETURN_ERROR; //break;
            } 
            *rxbuf++ = data; //  @todo inogda budet chitatsa error!
        } while (rxbytes--);
        halI2C_NACK ();
    } else {
        return FUNCTION_RETURN_ERROR;
    }
    halI2C_STOP (); // @todo сделать проверку на корректное завершение
    
    return FUNCTION_RETURN_OK;
    
    
//     uint8_t i, data;
//  
//     data = (adr | I2C_READ);
//     for (i = 0; i < 8; i++) {
//         SCL_L;              //Опускаем CLK линию
//         halI2C_delay();
//         if (data & 0x80) {  //Если старший бит в байте установлен
//             SDA_H;
//         } else {
//             SDA_L;
//         }
//         data = data << 1;   //Сдвигаем все биты влево. На место старшего бита становится более младший
//         halI2C_delay();     //После седьмого сдвига на месте старшего бита окажется самый младший бит.
//         SCL_H;              //Возвращаем линию как было
//         halI2C_delay();
//     }
//     //SCL_L;
//     if (FUNCTION_RETURN_OK == halI2C_waitACK ()) { // adress good
//         SDA_H; // relase data line
//         while (--rxbytes) {
//             data = 0;
//             for (i = 0; i < 8; i++) { //Пока не отправили все 8 бит
//                 data = (data << 1);   //сдвигаем последние принятые биты слево
//                 SCL_L;
//                 halI2C_delay();
//                 SCL_H;
//                 halI2C_delay();
//                 if (0 != SDA_IN) {   // читаем состояние линии SDA
//                     data |= 0x01;    //Результат чтения помещяем на место самого младшего бита в приемном байте
//                 }
//             }
//             //SCL_L;
//             *rxbuf++ = data;
//             halI2C_ACK ();
//         }
//         data = 0;
//         for (i = 0; i < 8; i++) { //Пока не отправили все 8 бит
//             data = (data << 1);   //сдвигаем последние принятые биты слево
//             SCL_L;
//             halI2C_delay();
//             SCL_H;
//             halI2C_delay();
//             if (0 != SDA_IN) {   // читаем состояние линии SDA
//                 data |= 0x01;    //Результат чтения помещяем на место самого младшего бита в приемном байте
//             }
//         }
//         //SCL_L;
//         *rxbuf++ = data;
//         halI2C_NACK ();
//         respond = FUNCTION_RETURN_OK;
//     }
//     
//     return respond;
}


msg_t   halI2C_getErrors (void) {
    return FUNCTION_RETURN_OK;
}
