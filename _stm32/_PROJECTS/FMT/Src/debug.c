#include "debug.h"
#include "board.h"

#ifdef STM8
void _delay_init (void)
{
#if (BOARD_STM8LDISCOVERY || BOARD_STM8L051)
    CLK->PCKENR1 |= CLK_PCKENR1_TIM2;
    #if CLK_HSI // 16.000.000MHz / 128 / 125 = 1000 
        TIM2->PSCR = 0x04; //TIM4_PRESCALER_16
    #endif
        
    #if CLK_LSI // 128.000MHz / 1 / 128 = 1000
        TIM2->PSCR = 0x07; //TIM4_PRESCALER_128
    #endif
        
    #if CLK_HSE // 4.000.000MHz / 32 / 125 = 1000
        TIM2->PSCR = 0x02; //TIM4_PRESCALER_4
    #endif
#endif
        
#if (BOARD_STM8TEST || BOARD_STM8TEST_A)
    #if CLK_HSI // 8.000.000MHz / 64 / 125 = 1000 // Prescaler
        TIM2->PSCR = TIM2_PRESCALER_8;
    #endif
        
    #if CLK_LSI // 128.000MHz / 1 / 128 = 1000
        TIM2->PSCR = TIM2_PRESCALER_128;
    #endif
        
    #if CLK_HSE // 4.000.000MHz / 32 / 125 = 1000
        TIM2->PSCR = TIM2_PRESCALER_4;
    #endif 
#endif
      
}


void TIM2_Configuration (void)
{
//     TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

//     RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM2, ENABLE);
//     
//     TIM_TimeBaseStructure.TIM_Period = UINT16_MAX;
//     TIM_TimeBaseStructure.TIM_Prescaler = 59;
//     TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//     TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//         
//     TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
//     TIM_Cmd(TIM2, ENABLE);
}


void _delay_us (volatile uint16_t us)
{

#if (BOARD_STM8TEST || BOARD_STM8TEST_A)
    TIM2->ARRH = (uint8_t)(us >> 8);
    TIM2->ARRL = (uint8_t)(us);
    TIM2->EGR |= TIM2_EGR_UG; //Генерируем Событие обновления для записи данных в регистры PSC и ARR
    TIM2->CR1 |= (TIM2_CR1_CEN | TIM2_CR1_OPM); //Запускаем таймер записью бита CEN и устанавливаем режим Одного прохода установкой бита OPM
    while ((TIM2->CR1 & TIM2_CR1_CEN) != 0) {};
#endif

#if BOARD_STM8L051
    TIM2->ARRH = (uint8_t)(us >> 8);
    TIM2->ARRL = (uint8_t)(us);
    TIM2->EGR |= TIM_EGR_UG; //Генерируем Событие обновления для записи данных в регистры PSC и ARR
    TIM2->CR1 |= (TIM_CR1_CEN | TIM_CR1_OPM); //Запускаем таймер записью бита CEN и устанавливаем режим Одного прохода установкой бита OPM
    while ((TIM2->CR1 & TIM_CR1_CEN) != 0) {};
#endif
}


#else //STM8

#endif


#if STM32

void _debug_init (void)
{
    SCB_DEMCR   |= 0x01000000; // Настраиваем
    DWT_CONTROL |= 1; // включаем счётчик
    
 // эта функция всеголишь записывает в регистр SWO, тем самым настраивая дебаг
// <h> Debug MCU Configuration
//   <o1.0>    DBG_SLEEP     <i> Debug Sleep Mode
//   <o1.1>    DBG_STOP      <i> Debug Stop Mode
//   <o1.2>    DBG_STANDBY   <i> Debug Standby Mode
//   <o1.5>    TRACE_IOEN    <i> Trace I/O Enable 
//   <o1.6..7> TRACE_MODE    <i> Trace Mode
//             <0=> Asynchronous
//             <1=> Synchronous: TRACEDATA Size 1
//             <2=> Synchronous: TRACEDATA Size 2
//             <3=> Synchronous: TRACEDATA Size 4
//   <o1.8>    DBG_IWDG_STOP <i> Independant Watchdog Stopped when Core is halted
//   <o1.9>    DBG_WWDG_STOP <i> Window Watchdog Stopped when Core is halted
//   <o1.10>   DBG_TIM1_STOP <i> Timer 1 Stopped when Core is halted
//   <o1.11>   DBG_TIM2_STOP <i> Timer 2 Stopped when Core is halted
//   <o1.12>   DBG_TIM3_STOP <i> Timer 3 Stopped when Core is halted
//   <o1.13>   DBG_TIM4_STOP <i> Timer 4 Stopped when Core is halted
//   <o1.14>   DBG_CAN_STOP  <i> CAN Stopped when Core is halted
// </h>
  //_WDWORD(0xE0042004, 0x00000027);  // DBGMCU_CR
    DBGMCU_CR = 0x00000027; 
    
    //_delay_init ();
}


void _delay_init (void)
{
    // 16.000.000MHz / 128 / 125 = 1000 
#if BOARD_RS485_STTM
    RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
    TIM17->PSC = 72;
    
#else
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 72;
#endif

}


void _delay_us (uint16_t us)
{

#if BOARD_RS485_STTM  
    TIM17->ARR = us;
    TIM17->EGR |= TIM_EGR_UG; //Генерируем Событие обновления для записи данных в регистры PSC и ARR
    TIM17->CR1 |= (TIM_CR1_CEN | TIM_CR1_OPM); //Запускаем таймер записью бита CEN и устанавливаем режим Одного прохода установкой бита OPM
    while ((TIM17->CR1 & TIM_CR1_CEN) != 0) {};
        
#else
    TIM2->ARR = us;
    TIM2->EGR |= TIM_EGR_UG; //Генерируем Событие обновления для записи данных в регистры PSC и ARR
    TIM2->CR1 |= (TIM_CR1_CEN | TIM_CR1_OPM); //Запускаем таймер записью бита CEN и устанавливаем режим Одного прохода установкой бита OPM
    while ((TIM2->CR1 & TIM_CR1_CEN) != 0) {};
#endif
}

#endif


void _delay_ms (uint16_t ms)
{
    volatile uint16_t i;
    
    i = ms;
    while (i > 0)
    {
        _delay_us (1000);
        i--;
    }
}


//#define MEM_CHECK 1
#if (1 == MEM_CHECK)
static const char hexchars[] = "0123456789ABCDEF";

static char highhex(int  x)
{
   return hexchars[(x >> 4) & 0xF];
}

static char lowhex(int  x)
{
    return hexchars[x & 0xF];
}
/* convert the memory, pointed to by mem into hex, placing result in buf */
 /* return a pointer to the last char put in buf (null) */
static char *mem2hex (const char *mem, char *buf, int count)
{
    int i;
    int ch;
    for (i = 0; i < count; i++)
    {
        ch = *mem++;
        *buf++ = highhex (ch);
        *buf++ = lowhex (ch);
    }
    *buf = 0;
    return (buf);
}
#include "conv.h"
// logf already defined
void _logf(const char *pStr, uint32_t value) {//"EEPROM address: %d", address);
    char str[32];
    //zspintf (str, "%u", value);
    // concat...
    //out to log file...
    
}
#endif
