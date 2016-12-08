/*
 * @file    modRTC.h
 * @author  Ht3h5793
 * @date    08-October-2013
 * @version V3.5.0
 * @brief  
 
#define RTC_NEED_CALENDAR   1 

#include "modRTC.h"
rtc_s mRTC;
rtc_t RTC_cnt;


    mRTC.year = 2015;
    mRTC.month = 1;
    mRTC.day = 30;
    mRTC.hour = 9;
    mRTC.minute = 59;
    mRTC.second = 14;
    //mRTC.millisecond = 0;
    modRTC_set (&mRTC);
        
        
const char* s[] = {
    "Воскресенье", "Понедельник", "Вторник",
    "Среда", "Четверг", "Пятница", "Суббота"
    };

zsprintf(_str, "Q:%s", s[modRTC_get_weekday (&mRTC)]);
paint_putStrColRow (0, 4, _str);
    
 */

#ifndef MODRTC_H
#define MODRTC_H 20150207

#include "defines.h"
#include "board.h"

//#define RTC_CORRECT_TIC             1
//#define RTC_NEED_CALENDAR           1
//#define RTC_NEED_ALARM              1

 // дней до 1 янв 2001 ПН от 1 января 1970 года
#define JD0 2451911

// при типе unsigned long long, будет правильно считать миллиарды лет, а так только до 2136 года (примерно)
#define rtc_t unsigned long  //uint32_t

typedef struct {
    uint16_t  year;  //1970 - 2136
    uint8_t   month;   //1-12
    uint8_t   day;   //1-31
    uint8_t   hour;   //0-?
    uint8_t   minute; //0-59
    uint8_t   second; //0-59
    uint16_t  millisecond;
#if RTC_NEED_CALENDAR
    uint8_t   weekday;
#endif
} rtc_s;

/**
static const char *RTC_str[] = {
    "Воскресенье", "Понедельник", "Вторник",
    "Среда", "Четверг", "Пятница", "Суббота"
    };
*/


#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Данную функцию вызывать в прерывании с периодичностью ровно 1ms
 */
void modRTC_run (void);


/**
 * Функция преобразования григорианской даты и времени в значение счетчика
 * @param ftime - указатель на структуру, где будет хранится время
 */
void    modRTC_set (rtc_s *);


/**
 * Функция преобразования значение счетчика в григорианскую дату и время 
 * @param ftime - указатель на структуру, где будет хранится время
 */
void    modRTC_get (rtc_s *); //, rtc_t *count);

    
/**
 * Функция возвращает день недели (начинается с воскресенья) 0 - 6
 * @param ftime - указатель на структуру, где будет хранится время
 */
#if RTC_NEED_CALENDAR
uint8_t modRTC_get_weekday (rtc_s *);
#endif


#if RTC_NEED_ALARM
msg_t RTC_alarm_check (rtc_s *ftime, rtc_s *fbud);
#endif

#ifdef	__cplusplus
}
#endif
    
#endif /* MODRTC_H */
