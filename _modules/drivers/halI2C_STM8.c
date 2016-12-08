
#include "halI2C.h"
#include "../modSysClock/modSysClock.h"     // для формирования задержек ожидания и таймаутов

#define I2C_ // разрешить аппаратный I2C

#define    DWT_CYCCNT    *(volatile unsigned long *)0xE0001004
#define    DWT_CONTROL   *(volatile unsigned long *)0xE0001000
#define    SCB_DEMCR     *(volatile unsigned long *)0xE000EDFC



#ifdef  I2C_
#include "../_HAL/MCU_include/stm32f10x.h"  // I2C
#else
// Для дебага! тут будут хранится данные 
unsigned char       ROM[32*2 + 32*30];
#endif

#define DEBUG_
#ifdef DEBUG_
    #include <stdio.h>
    #define dprintf(x) printf x

//static unsigned long  EventBuf[32]; // дебаг
//unsigned char EvPosition = 0; // дебаг
#else
    #define dprintf(x)
#endif


//------------------------------I2C--------------------------------------------
//#define I2C_GPIO     GPIOB
//#define I2C_RCC_APB2Periph_GPIO  RCC_APB2Periph_GPIOB



//==============================================================================
enum I2C_POSITION {
	I2C_IDLE                 =  0,
    I2C_LOAD_ADRESS          =  1, // EV5 было сгенерированно прерывание по старт биту, отсылаем тут адрес слейва
    
    I2C_WRITE_ADRESS_H       =  2, // засылаем старший байт адреса данных в ЕЕПРОМ
    I2C_WRITE_ADRESS_L       =  3,
    
    I2C_RESTART              =  4, //

    I2C_REC_SELECT           =  5, // выбор направления
	
    I2C_SELECT_NEXT          =  6, // повторный старт байт
    I2C_CHECK_ADDRESS_DEVICE =  7,
    
    
    I2C_READ_BUFFER          =  8,
    I2C_READ_BUFFER_LAST     =  9,
    I2C_READ_STOP            = 15, // дошли до конца чтения
    
    
    I2C_WRITE_BUFFER         = 17,
    I2C_WRITE_BUFFER_LAST    = 18,
    I2C_WRITE_STOP           = 31,
    
    I2C_ERROR                = 63,                // ошибочное состояние КА
    
};
/** позиция автомата I2C в перрывании */
unsigned long I2C_PosMode;

enum interrupt_flag_
{
    INTFLAG_WAIT,
    INTFLAG_NOT_READY,           // запрос еще не готов
    INTFLAG_READY,               // запрос выполнен
    
};
/** флаг выполнения */
unsigned char InteruptFlag;

unsigned char I2C_Size; // размер данных, которые нужно записать\считать
unsigned char I2C_Count; // счетчик байтов, которые прошли по шине 

static unsigned short event; // флаги прерывания читаем сюда
unsigned long regSR;

//unsigned char tmpChar;

unsigned long I2C_Timer; // счетчик таймаута, при превышении срабатывает ошибка

/** адрес данных в 24C64 */
unsigned char I2C_Adress_L; // младший байт
unsigned char I2C_Adress_H; // старший байт

unsigned char *I2C_Pbuf; // запоминаем указатель на внешний буфер данных

enum STATUS_PROCESS
{
    I2C_NOT_OPERATION,
    I2C_NEED_READ,
    I2C_NEED_WRITE,
};

unsigned char Operation   = I2C_NOT_OPERATION; // флаг для прерывания, определяем по нему что нужно сделать

unsigned long I2C_ErrorCount = 0;


//============================== Функции ===================================
unsigned char modI2C_Init(void)
{
	unsigned char respond = I2C_FUNCTION_RETURN_OK;
#ifdef  I2C_

    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO , ENABLE); // разрешаем тактирование порта B и ремапа
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_I2C1, ENABLE); // разрешение тактирования И2Ц
    
    //RCC->APB2ENR |= (RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN); // подаем тактирование на порт
    //RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; // на И2Ц

    
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_SDA | GPIO_Pin_SCL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; // конфигурируем как открытый коллектор
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    //GPIOB->ODR |= 0x00ff;
    
    GPIOB->ODR &= ~GPIO_Pin_SDA;
    GPIOB->ODR &= ~GPIO_Pin_SCL;
    
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    //__NOP();
    //__NOP();
    //__NOP();
    //__NOP();
    //__NOP();
    GPIOB->ODR |= GPIO_Pin_SCL;
    
    //__NOP();
    //__NOP();
    //__NOP();
    //__NOP();
    //__NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    GPIOB->ODR |= GPIO_Pin_SDA;
    
    
    
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_SDA | GPIO_Pin_SCL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; // конфигурируем как открытый коллектор с разрешением алтернативынй функций
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    //GPIOB->ODR = 0xffff; // возможно лучше в 1 выставить все, можно и проверить
    // возможно микросхема находится в неверном состоянии и поджимает к земле. тогла старт бит не пройдет

    //GPIOB->CRH |=  (GPIO_CRH_MODE9_0 | GPIO_CRH_MODE8_0);
    //GPIOB->CRH |=  (GPIO_CRH_CNF9 | GPIO_CRH_CNF8);
    
    AFIO->MAPR |= AFIO_MAPR_I2C1_REMAP; // ремап модуля
	
    I2C1->CR1 = I2C_CR1_SWRST; // сброс модуля И2Ц
    I2C1->CR1 = 0;
    
    // текущее значение частоты PCLK1 (может быть от 2 до 32???, включительно)
    I2C1->CR2 &= ~I2C_CR2_FREQ; 
    I2C1->CR2 |= 72;//I2C_CR2_FREQ; // max 0x3F

    I2C1->CCR &= ~I2C_CCR_CCR; //установка предделителя 72MHz/100KHz
    I2C1->CCR |= 720;//I2C_CCR_CCR; // максимальное значение 0x0FFF, менять только при PE = 0

    I2C1->TRISE = 25;//I2C_TRISE_TRISE; // rise time max 0x3F
    
    I2C1->CR1 &= (unsigned short)~(I2C_CR1_SMBUS | I2C_CR1_SMBTYPE); // только И2Ц режим
	
    
    // ---------- Настраиваем прерывания -----------------------
    NVIC_InitTypeDef NVIC_InitStructure;  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // Configure the Priority Group to 1 bit 
    // Configure the I2C event priority 
    NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //приоритет
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        //приоритет субгруппы
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           //включаем канал
    NVIC_Init(&NVIC_InitStructure);                           //инициализируем
    
    
    // разрешаем работу модуля I2C
    I2C1->CR1 |= I2C_CR1_PE;
    
    // при инициализации не помешает STOP в шину загнать (мало ли когда сброс произошел).
    //I2C1->CR1   |=  I2C_CR1_STOP;  // поднимаем флаг 
    //I2C1->DR = 
    //I2C1->CR1   &= ~I2C_CR1_STOP;  // опускаем
    modI2C_ReInit(); // 
    
    // разрешаем прерывания
    I2C1->CR2 |= (I2C_CR2_ITEVTEN | I2C_CR2_ITERREN);
#ifdef DEBUG_
    // свтодиоды для дебага на плате MCBSTM32C
    //RCC->APB2ENR |=  RCC_APB2ENR_IOPEEN; // тактирование порта
    //GPIOE->CRH   |=  (GPIO_CRH_MODE15_0 | GPIO_CRH_MODE14_0 | GPIO_CRH_MODE13_0 | GPIO_CRH_MODE12_0 | GPIO_CRH_MODE11_0 | GPIO_CRH_MODE10_0 | GPIO_CRH_MODE9_0 | GPIO_CRH_MODE8_0);
    //GPIOE->CRH   &=  ~(GPIO_CRH_CNF15 | GPIO_CRH_CNF14 | GPIO_CRH_CNF13 | GPIO_CRH_CNF12 | GPIO_CRH_CNF11 | GPIO_CRH_CNF10 | GPIO_CRH_CNF9 | GPIO_CRH_CNF8);
#endif

#endif
    InteruptFlag = INTFLAG_WAIT; // флаг статуса операции неактивен
    
    return respond;
}

void Wait_ms(unsigned long t)
{
#ifdef  I2C_
    __NOP();
    __NOP();
    __NOP();
    /*
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    */
#endif
    
}

unsigned char modI2C_ReInit(void)
{
	unsigned char respond = I2C_FUNCTION_RETURN_OK;
#ifdef  I2C_

   if(I2C1->SR2 & I2C_SR2_BUSY) // проверяю, занята ли шина I2C (взведён ли флаг BUSY)
   {       
      // dprintf(("Reinit\r\n"));
#ifdef DEBUG_
	GPIOE->ODR = 1 << 14; // на светодиоды
#endif 
    // если нет - выходим
    // а если взведён RECOVERY
    //GPIOB->BSRR |= (1 << PIN7) | (1 << PIN6); // выставляю SDA и SCL в 1
    //GPIOB->CRL &= ~(0xC << CR_PIN6) & ~(0xF << CR_PIN7); // настраиваю порты: 6 - выход(SCL), 7 - вход(SDA)
    //GPIOB->CRL |= (0x3 << CR_MODE6) | (0x2 << CR_CNF7);
       
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_SDA | GPIO_Pin_SCL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; // конфигурируем как открытый коллектор
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    
    while(1) // здесь мы можем быть максимум 9 раз (крайний случай, смотри в pdf)
    {// дёргаю SCL (тактирую Slave)
        GPIOB->ODR &= ~GPIO_Pin_SCL; // SCL = 0
        Wait_ms(1);
        GPIOB->ODR |= GPIO_Pin_SCL; // SCL = 1
        Wait_ms(1);
       
        if(GPIOB->IDR & GPIO_Pin_SDA) // смотрю, отпустил ли Slave SDA (SDA == 1 ?)
        {// если да - настраиваю выводы на выход и делаю Stop состояние
            GPIOB->ODR &= ~GPIO_Pin_SCL; // SCL = 0
            Wait_ms(1);
            GPIOB->ODR &= ~GPIO_Pin_SDA; // SDA = 0
            Wait_ms(1);
         
            //GPIOB->CRL &= ~(0xC << CR_PIN7); // выход
            //GPIOB->CRL |= (0x3 << CR_MODE7);
         
            GPIOB->ODR |= GPIO_Pin_SCL; // SCL = 1
            Wait_ms(1);
            GPIOB->ODR |= GPIO_Pin_SDA; // SDA = 1
            break; // выходим из цикла
        }
    }
    // возвращаю настройки порта (аппаратный I2C)
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_SDA | GPIO_Pin_SCL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; // конфигурируем как открытый коллектор с разрешением алтернативынй функций
    GPIO_Init(GPIOB, &GPIO_InitStructure);
   
    // после этого шина I2C свободна и готова к работе
    } 
#endif
    return respond;
}


unsigned char modI2C_DeInit(void)
{
	unsigned char respond = I2C_FUNCTION_RETURN_OK;
#ifdef  I2C_
    // Reset i2c peripheral.
    I2C1->CR1 = I2C_CR1_SWRST;
    I2C1->CR1 = 0;
#endif
    return respond;
}


/**
 * Функция, которая должна вызыватья в прерывании I2C1_ER_IRQHandler
 */
void modI2C_InterruptError(void)
{
#ifdef  I2C_
    event = I2C1->SR1;
    regSR = I2C1->SR2;
    //I2C_PosMode = I2C_ERROR;
    //dprintf((" Interrupt ERError:%X |\r\n", event));
    
    if(I2C1->SR1 & I2C_SR1_BERR)
    {
       I2C1->SR1 &= ~I2C_SR1_BERR;
        modI2C_Init();
    }
	//while(1){};
#endif
}


/**
 * Функция, которая должна вызыватья в прерывании I2C1_EV_IRQHandler
 */
void modI2C_InterruptEvent(void)
{
#ifdef  I2C_
    event = I2C1->SR1; // считываем регистры флагов, тут же их как бы сбрасываем, хотя не всегда
    regSR = I2C1->SR2; // этот регистр не проверяю, тут флаги не тех событий

#ifdef DEBUG_
	//EventBuf[EvPosition] = event;
	//EvPosition = (++EvPosition) & 0x1f;
    //dprintf(("I:%X \r\n", event));
#endif

    switch(I2C_PosMode) // TODO. убрать бесконечные заглушки, все равно не работает дебаг, поменять на, возможно, флаг ошибки
    {
        case I2C_LOAD_ADRESS:
            if ((event & I2C_SR1_SB) != 0) // EV5
            {
                //dprintf(("EV5 \r\n"));
                I2C1->SR1 &= ~I2C_SR1_SB; // сбрасываю бит, хотя некоторые не делают такого
                I2C1->DR = I2C_COMMAND_WRITE; // передаем адрес микросхемы + комманду записи 
                I2C_PosMode = I2C_WRITE_ADRESS_H;
            }
            break;
            
        case I2C_WRITE_ADRESS_H:
            if ((event & I2C_SR1_ADDR) != 0) //EV6
            {
                //dprintf(("EV6h \r\n"));
                I2C1->DR = I2C_Adress_H;    // тут же загружаем старший байт адреса сектора памяти. чтобы вызвать прерывание пустого приемного буфера
                I2C_PosMode = I2C_WRITE_ADRESS_L;
            }
            break;
            
        case I2C_WRITE_ADRESS_L:
            if ((event & I2C_SR1_BTF) != 0) // EV8_1
            {
                //dprintf(("EV8_1 \r\n"));
                I2C1->DR = I2C_Adress_L; // записываем младший байт адреса
                I2C_PosMode = I2C_RESTART;
            }
            break;
						
         case I2C_RESTART:
            if ((event & I2C_SR1_BTF) != 0) // EV8, ничего не делаем, тут прерывание по причине того, что данные пришли успешно в слейв и он подтвердил
            {
                //dprintf(("EV8 \r\n"));
                I2C_PosMode = I2C_REC_SELECT;
            }
            break;
            
        case I2C_REC_SELECT:
            if ((event & I2C_SR1_BTF) != 0) // EV8 //загружаем младший байт адреса
            {
                //dprintf(("EV8s \r\n"));
                if (I2C_NEED_READ == Operation)
                {
                    I2C1->CR1  |= (I2C_CR1_START | I2C_CR1_ACK); // тут сбрасываем шину, передав комманду повторного старта + подтверждение всего, что считаем (возможно луше перенести отсюда ACK)
                    I2C1->DR    = I2C_Adress_L; // Wat?
                    I2C_PosMode = I2C_SELECT_NEXT; // возможно лучше будет вынести в отдельное case? возможно генерирует еще одно прерывание
                }
                if (I2C_NEED_WRITE == Operation)
                {
                   I2C_PosMode  = I2C_WRITE_BUFFER; // возможно лучше будет вынести в отдельное case? возможно генерирует еще одно прерывание 
                   I2C1->DR     = I2C_Pbuf[I2C_Count]; // просто записываем что-то	
                   I2C_Count++;                    
                }
            }
            break;
            
        case I2C_SELECT_NEXT:
            if ((event & I2C_SR1_SB) != 0) // EV5? но тут же как бы повторный старт
            {
                //dprintf(("EV5n \r\n"));
                I2C_PosMode = I2C_CHECK_ADDRESS_DEVICE; 
                I2C1->DR = I2C_COMMAND_READ;
            }
            break;
							
       case I2C_CHECK_ADDRESS_DEVICE:
            if ((event & I2C_SR1_ADDR) != 0) // EV6 тут мы оказались, если нужно чтение и 24C64 ответило
            {  // ничего не нужно делать
                //dprintf(("EV6 \r\n"));
                I2C_PosMode = I2C_READ_BUFFER;//I2C_CHECK_ADDRESS_; 
            }
            break;
                

        case I2C_READ_BUFFER:
            if ((event & I2C_SR1_RXNE) != 0) // EV7 к нам пришел байт
            {
                //dprintf(("EV7"));
                if ( I2C_Count >= (I2C_Size-1) )
                {
                    I2C1->CR1   &= ~I2C_CR1_ACK;   // если мы подошли к концу, то больше читать не нужно
                    //dprintf(("EV7_L \r\n"));
                }
                
                if ( I2C_Count >= I2C_Size )
                {
                    //dprintf(("EV7_S \r\n"));
                    I2C_PosMode  =  I2C_READ_STOP; // на случай если еще одно перывание возникнет, хотя не должно, по идее TODO поменять на ERROR
                    // возможно лучше будет добавить функцию принудительного отключения модуля I2C, чтобы не возникало на шине колебаний уровней
                    InteruptFlag =  INTFLAG_READY; // обмен завершен
                    
                    I2C1->CR1   |=  I2C_CR1_STOP;  // поднимаем флаг 
                }
                I2C_Pbuf[I2C_Count] = I2C1->DR; // читаем регистр, этим самым же инициируем дальнейший цикл чтения на шине
                I2C_Count++;                    // инкрементируем
            }
            break;
  
        case I2C_READ_STOP: // если тут, то данные все уже считаны, но сюда мы не должны попасть
            dprintf(("READ_STOP? \r\n"));
            break;
            
        // ---------------- записываем данные ------------------------------
        case I2C_WRITE_BUFFER:
            //dprintf(("EV write buf \r\n"));
            if ((event & I2C_SR1_BTF) != 0)
            {
                if ( I2C_Count > I2C_Size )
                {
                    I2C_PosMode  =  I2C_READ_STOP;
                    InteruptFlag =  INTFLAG_READY;
                    I2C1->CR1   |=  I2C_CR1_STOP;
                }
                
                I2C1->DR = I2C_Pbuf[I2C_Count]; 
                I2C_Count++; 
            }
            break;   

        case I2C_WRITE_STOP: // сюда не должны попасть
            dprintf(("WRITE_STOP? \r\n"));
            break;
            
        default:
            dprintf(("Interrupt EV Error:default \r\n"));
            modI2C_ReInit();
            break;
    }
		
	
    /* при поднимании флага совпадения адреса устройства. его нужно сбросить путем чтения регистров статуса */
    //if (event & (I2C_SR1_ADDR | I2C_SR1_ADD10))
    //{
        //(void)I2C1->SR2;
        //(void)I2C1->SR2;
    //}

#ifdef DEBUG_
	GPIOE->ODR = I2C_PosMode << 8; // на светодиоды
#endif 
#endif // конец проверки на разрешение аппаратного I2C 
}


/**
 * Функция запуска процесса чтения по шине I2C блока данных. Настраивает КА на старт,
 * настраивает локальные перменные (адрес чтения/записи, длина блока ( TODO добавить проверку на граничный размер блока)
 * сбрасывает флаг выполненияоперации (возможно стоит просто проверять указатель KA)
 * запоминает указатели на буферы, 
 * (сбрасывает или повторно инициализирует модуль I2C)
 *  
 * разрешает прерывания от модуля I2C путем установки в нем битов разрешения прерывания
 * инициирует последовательность чтения путем установки бита START
 * 
 * @param ReadAddr   - адрес чтения, принимает значения от 0 до 8191 для 23C64 
 * @param pBuf       - приемный буфер
 * @param Lenght     - количество считанных байт, от 1 до 32 ( не 0)
 * @return 
 */
unsigned char modI2C_StartReadBlock(unsigned short ReadAddr, unsigned char *pBuf, unsigned short Lenght)
{
    unsigned char respond = I2C_FUNCTION_RETURN_OK;
    
    I2C_Timer = modSysClock_getTime(); // запоминаем время для таймаута
#ifdef  I2C_ 
    // тут вызывать реинициализацию
    modI2C_Init();
    
    // запускаем автомат
    I2C_PosMode  = I2C_LOAD_ADRESS;
    Operation    = I2C_NEED_READ;
    InteruptFlag = INTFLAG_NOT_READY;
    
    if (ReadAddr & 0xE000) // проверка адреса
    {
        respond = I2C_FUNCTION_RETURN_ERROR_ADRESS;
    }
    
    // настраиваем адрес чтения страницы
    I2C_Adress_L = (unsigned char)( ReadAddr & 0x00FF);
    I2C_Adress_H = (unsigned char)((ReadAddr & 0xFF00) >> 8);
    
    I2C_Pbuf     = (unsigned char *) &pBuf[0];
    I2C_Size     = Lenght;
    I2C_Size--;
    I2C_Count    = 0;

    // формирование сигнала старт
    I2C1->CR1 |= I2C_CR1_START | I2C_CR1_ACK;
    
#else
    for (I2C_Count = 0; I2C_Count< Lenght; I2C_Count++)
    {
        pBuf[I2C_Count] = ROM[ ReadAddr + I2C_Count ] ;
    }
    InteruptFlag = INTFLAG_READY;
#endif
    return respond;
}


/**
 * Функция проверки флага окончания чтения по шине I2C блока данных.
 * @return 
 */
unsigned char modI2C_CheckRead(void)
{
    unsigned char respond = I2C_FUNCTION_RETURN_NOT_READY;

    if(INTFLAG_READY  == InteruptFlag)
    { 
#ifdef  I2C_

        //I2C1->CR2 &= ~(I2C_CR2_ITEVTEN | I2C_CR2_ITERREN); // int
        //while (!(I2C1->SR2 & I2C_SR2_MSL)){};
        //I2C1->CR1 = I2C_CR1_SWRSST;
        //I2C1->CR1 = 0;
        //modI2C_DeInit();
#endif
        InteruptFlag = INTFLAG_NOT_READY;
        respond = I2C_FUNCTION_RETURN_READY;
    }
    if (INTFLAG_NOT_READY == InteruptFlag)
    {
        if( modSysClock_getPastTime(I2C_Timer, SYSCLOCK_GET_TIME_MS_1) > I2C_TIMEOUT)
        {
            I2C_ErrorCount++;
            respond = I2C_FUNCTION_RETURN_ERROR_TIMEOUT;
        }
    }

    return respond;
}


unsigned char modI2C_StartWriteBlock(unsigned short WriteAddr, unsigned char *pBuf, unsigned short Lenght)
{
    unsigned char respond = I2C_FUNCTION_RETURN_OK;
    I2C_Timer = modSysClock_getTime(); // запоминаем время для таймаута
#ifdef  I2C_
    // тут вызывать реинициализацию
    modI2C_Init();
    
    // запускаем автомат
    I2C_PosMode  = I2C_LOAD_ADRESS;
    Operation    = I2C_NEED_WRITE;
    InteruptFlag = INTFLAG_NOT_READY;
    
    if (WriteAddr & 0xE000) // проверка адреса
    {
        respond = I2C_FUNCTION_RETURN_ERROR_ADRESS;
    }
    
    // настраиваем адрес чтения страницы
    I2C_Adress_L = (unsigned char)( WriteAddr & 0x00FF);
    I2C_Adress_H = (unsigned char)((WriteAddr & 0xFF00) >> 8);
    
        
    I2C_Pbuf     = (unsigned char *) &pBuf[0];
    I2C_Size     = Lenght;
    I2C_Size--;
    I2C_Count    = 0;
    
    // формирование сигнала старт
    I2C1->CR1 |= I2C_CR1_START | I2C_CR1_ACK; 
    
#else
    for (I2C_Count = 0; I2C_Count< Lenght; I2C_Count++)
    {
        ROM[ WriteAddr + I2C_Count ] = pBuf[I2C_Count];
    }
    InteruptFlag = INTFLAG_READY;
#endif
    return respond;
}


/**
 * Функция проверки флага окончания чтения
 * @return 
 */
unsigned char modI2C_CheckWrite(void)
{
    unsigned char respond = I2C_FUNCTION_RETURN_NOT_READY;

    if(INTFLAG_READY  == InteruptFlag)
    {
#ifdef  I2C_ // тут типа сбросим быты разрешения перываний или что-то еще
        
#endif
        InteruptFlag = INTFLAG_NOT_READY;
        respond = I2C_FUNCTION_RETURN_READY;
    }
    
    if (INTFLAG_NOT_READY == InteruptFlag)
    {
        if( modSysClock_getPastTime(I2C_Timer, SYSCLOCK_GET_TIME_MS_1) > I2C_TIMEOUT)
        {
            I2C_ErrorCount++;
            respond = I2C_FUNCTION_RETURN_ERROR_TIMEOUT;
        }
    }
    return respond;
}


