#include "modRTC.h"
//#include "board.h"


#if MOD_RTC
extern rtc_t RTC_cnt; // счетчик секунд


#if RTC_CORRECT_TIC
#define SEC_ERROR      ( 3650UL - 365UL )  // ошибка секунд в год
static uint8_t  correctZnak = 0; // 0 если спешат, 1 если отстают

uint32_t correct = 0;
static uint8_t needCorrect = 0; // флаг необходимости коррекции
#define CORRECT_CONSTANT ((24UL*60UL*60UL*365UL)/SEC_ERROR) 
#endif


void modRTC_set (rtc_s *ftime)
{
    int32_t  a; // uint8_t
    int32_t  y; // uint16_t
    int32_t  m; // uint8_t
    rtc_t    JDN;
    
    // Вычисление необходимых коэффициентов
    a = (14 - ftime->month) / 12;
    y = (uint32_t)ftime->year + 4800 - a;
    m = ftime->month + (12 * a) - 3;
    // Вычисляем значение текущего Юлианского дня
    JDN = ftime->day;
    JDN += (153 * m + 2) / 5;
    JDN += 365 * (uint32_t)y;
    JDN += y / 4;
    JDN += -y / 100;
    JDN += y / 400;
    JDN = JDN - 32045; // совместить с нижним
    JDN = JDN - JD0; // так как счетчик у нас нерезиновый, уберем дни которые прошли до 01 янв 2001 
    JDN *= 86400;     // переводим дни в секунды
    JDN += ((uint32_t)ftime->hour * 3600); // и дополняем его секундами текущего дня
    JDN += ((uint32_t)ftime->minute * 60);
    JDN += (ftime->second);
    // итого имеем количество секунд с 00-00 01 янв 2001
    //__DI();
    RTC_cnt = JDN;
    //__EI();
    //tmr_MillSecond = ftime->millisecond;
}


void modRTC_get (rtc_s *ftime)
{
    rtc_t    ace;
    int32_t  b; // uint8_t
    int32_t  d; // uint8_t
    int32_t  m; // uint8_t
    
    //__DI();
    ace = RTC_cnt;
    //__EI();
    ace = (ace / 86400) + 32044 + JD0;
    b   = (4 * ace + 3) / 146097; // может ли произойти потеря точности из-за переполнения ?
    ace = ace - ((146097 * b) / 4); // но и так работает
    d   = (4 * ace + 3) / 1461;
    ace = ace - ((1461 * d) / 4);
    m   = (5 * ace + 2) / 153;
    ftime->day    = (uint8_t)(ace - ((153 * m + 2) / 5) + 1);
    ftime->month  = m + 3 - (12 * (m / 10));
    ftime->year   = (uint16_t)(100 * b + d - 4800 + (m / 10));
    ftime->hour   = (RTC_cnt / 3600) % 24;
    ftime->minute = (RTC_cnt / 60) % 60;
    ftime->second = (RTC_cnt % 60);
    
    //ftime->millisecond = tmr_MillSecond;
}


#if RTC_NEED_CALENDAR
uint8_t modRTC_get_weekday (rtc_s *ftime)
{
    int16_t m, y;
    int16_t c, weekday;

    m = ftime->month;
    y = ftime->year;
    if (m > 2) m -= 2; else m += 10, y--;
    c = y / 100;
    y %= 100;
    weekday = (ftime->day + (13 * m - 1) / 5 + y + y / 4 - c * 2 + c / 4) % 7;
    if (weekday < 0) weekday += 7;
    ftime->weekday = weekday;
    
    return (uint8_t)(weekday);
}
#endif

volatile uint16_t tmr_MillSecond = 0; // счетчик миллисекунд



void  modRTC_run (void)
{
#if RTC_CORRECT_TIC
    tmr_MillSecond++;
    if (TIC_PERIOD <= tmr_MillSecond)
    {
        tmr_MillSecond = 0; 
        RTC_cnt++; 
    }
    
        // коррекция
//     correct += CORRECT_CONSTANT;
//     if ( 1.0 <= correct)
//     {
//         correct = 0.0;
//         if (0 == correctZnak) // значит вычитаем
//         {
//             TimerMillSecond--;
//         }
//         else
//         {
//             TimerMillSecond++;
//         }
//     }
    
    correct++; //
    if (CORRECT_CONSTANT <= correct)
    {
        correct = 0;
        needCorrect = 1;
    }
    if ((1 == needCorrect) && ((2 <= tmr_MillSecond) && (998 >= tmr_MillSecond))) // при таких условиях, коррекция не должна быть меньше 4-х
    {
        needCorrect = 0;
        if (0 == correctZnak) // значит вычитаем
        {
            tmr_MillSecond--;
        } else {
            tmr_MillSecond++;
        }
    } 
#else
    //tmr_MillSecond++;
    //if (TIC_PERIOD <= tmr_MillSecond)
    {
        //tmr_MillSecond = 0;
        RTC_cnt++;
    }
#endif // RTC_CORRECT_TIC
}




#if RTC_NEED_ALARM
//msg_t RTC_alarm_set (rtc_s *fbud) // & armed
//{ 
//    msg_t respond = FUNCTION_RETURN_ERROR;
//    //ftime->second = sec;
//    return respond;
//}

msg_t RTC_alarm_check (rtc_s *ftime, rtc_s *fbud)
{
    msg_t respond = FUNCTION_RETURN_ERROR;

    if (//(ftime->year   == fbud->year)   &&
        //(ftime->month  == fbud->month)  &&
        //(ftime->day    == fbud->day)    &&
        (ftime->hour   == fbud->hour)   &&
        (ftime->minute == fbud->minute)// && 
        //(ftime->second >= fbud->second) && // @todo
        //(ftime->second < (fbud->second + 2))
    )
    {
        respond = FUNCTION_RETURN_OK;
    }
    return respond;
}
#endif // RTC_NEED_ALARM

#endif //MOD_RTC
