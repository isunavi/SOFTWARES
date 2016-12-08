#include "drvESP8266.h"
#include "board.h"
#ifdef STM8
    #include "modSysClockMicro.h"
#else
    #include "modSysClock.h"
#endif
#include "halUSART.h"
#include "conv.h"

#if (AT_VERSION == 21) // http://esp8266.ru/esp8266-at-commands-v021/
#warning AT_VERSION == 21


#if BOARD_STM32F4DISCOVERY
    #include "modPaint_local.h"
    #include "modPaint.h"
    #include "halLCD.h"
    #include "bitmap_logo.h"

    //#include "halUSART.h"
#endif

#if BOARD_STM32F4DISCOVERYff
    #include "modPaint_local.h"
    #include "modPaint.h"
    #include "halLCD.h"

    #include "usb_lib.h"
    #include "usb_desc.h"
    #include "hw_config.h"
    #include "usb_pwr.h"
#endif

#if BOARD_STM8_ESP8266

#endif

#include "debug.h"
#include "conv.h"
#include "xprintf.h"


/*
https://github.com/erwinholzhauser/parser-code_generator/blob/master/Parser-Code_Generator.c
http://toster.ru/q/120213?utm_source=tm_habrahabr&utm_medium=tm_block&utm_campaign=tm_promo
http://habrahabr.ru/post/219107/

Escape-������������������:
\033[A = ����������� ������ �� ���� ������ �����
\033[B = ����������� ������ �� ���� ������ ����
\033[C = �������� ������ �� ���� ������� ������
\033[D = �������� ������ �� ���� ������� �����
\033[H = ����������� ������ � ����� ������� ���� � ����� (������� 0,0)
\033[J = �������� ��, �� ���������� ������ �����!
\033[K = ������� �� ����� ������, �� ���������� ������ �����!
\033[M = ����� ����� �������� � �� �����������
\033[Y = �������, ��������� Y-X
\033[X = �������, ��������� X-Y
\033[R = CGRAM ����� ������ ������ � �� �����������, �. �. ��� CGRAM
\033[V = ��������� �������� � �� �����������
\033[W = ��������� �������� � �� �����������
\033[b = ��������� ��������-��������� � �� �����������


 ������ �������� ����:
\r = ������� ������� (���������� ������ � ������� 0 �� ������� �����!)
\n = ����� �����
\t = ��������� (�� ��������� 3 �������)

*/


// http://playground.arduino.cc/Code/NTPclient
// http://playground.arduino.cc/Main/DS1307OfTheLogshieldByMeansOfNTP

//https://github.com/itead/ITEADLIB_Arduino_ESP8266/blob/master/uartWIFI.cpp

#include "pt.h"
static struct pt pt1, pt2;
#include "pt-sem.h"
static struct pt_sem USART_mutex;


uint8_t wifi_number;
//bool_t init_exit = FALSE;
extern char _str[128];

extern uint32_t delayA, delayB;
extern uint8_t size8;
extern uint16_t size16;
extern uint8_t tmp8;

uint8_t esp_init_cnt = 1; //if = 0 -> immidiatly start


 //known AP
static const char WIFI_SSID [][18] = {
    "MaxHome",
    "Putin_Huilo",
    "wifi_126_network"
    //4
 };

static const char WIFI_PASS [][18] = {
    "0956963966",
    "la-la-la-la",
    "network_pass"
    //4
};

struct s_parser_t {
    uint8_t buf[ESP_PARSER_SIZE]; // Buffer
    //uint32_t size;
    uint16_t cnt; //��������� �������� ����
};

static struct s_parser_t s_parser;





void DEBUG_OUT (const char *str) //@todo ����������� �� �����
{
#if BOARD_STM32F4DISCOVERY
    xsprintf (_str, "%s\n", str);
    paint_strClearRow (4);
    paint_putStrColRow (0, 4, (const char *)_str);
#endif
#if dfgddgdfgfd
    uint32_t i;
    i = xsprintf (_str, "%s\n", str);
    USB_sendM ((uint8_t *)&_str, i);
#endif
#if BOARD_STM8_ESP8266
#endif
}


enum ESP_RESPOND {
    //ESP_RESPOND_OK, //not implements
    //ESP_RESPOND_ERROR,
    
    ESP_RESPOND_NOT_READY       = 1,
    ESP_RESPOND_ERROR_OVF       = 2,
    ESP_RESPOND_ERROR_TIMEOUT   = 3,
    
    ESP_RESPOND_READY           = 5,
    
};


msg_t at_respond = 0;
systime_t at_delay;

msg_t at_send (const char *str)
{
    msg_t respond = FUNCTION_RETURN_ERROR;
    uint32_t i;
    
    halUSART1_flush (); //- ����� ���������� �������� ������?
    for (i = 0; i < ESP_PARSER_SIZE; i++) //������ ����� �� ��������� ���������� �� ���������x �������� ������
    {
        s_parser.buf[i] = 0;
    }
    //size16 = xsprintf (_str, "%s\n", str); // @todo �� ��������� \n!!!!!
    s_parser.cnt = 0; //� ����� ������� ����� ��������� ��� � ������ ������
    respond = halUSART1_sndM ((uint8_t *)&str[0], _strlen ((char *)&str[0]));
    //if (respond != FUNCTION_RETURN_OK)
    //    LED_GREEN_INV;
    at_delay = modSysClock_getTime ();
    return respond;
}


msg_t at_get (void) // ������ �� ����, ������ ������ �������� �����, ���������� ������ � ����
{
    uint32_t i;
    
    halUSART1_flush ();
    for (i = 0; i < ESP_PARSER_SIZE; i++)
    {
        s_parser.buf[i] = 0;
    }
    s_parser.cnt = 0; //� ����� ������� ����� ��������� ��� � ������ ������
    at_delay = modSysClock_getTime ();

    return at_respond;
}


msg_t at_check (const char *str, systime_t timeout)
{
    at_respond = ESP_RESPOND_NOT_READY;

    if (FUNCTION_RETURN_OK == halUSART1_rcvS ((uint8_t *)&s_parser.buf[s_parser.cnt]))
    {
#ifdef BOARD_STM32F4DISCOVERY
        //halUSART2_sndS (s_parser.buf[s_parser.cnt]);
#endif
#if dsfsdfdsfdddd
        USB_sendM ((uint8_t *)&s_parser.buf[s_parser.cnt], 1);
#endif
#if BOARD_STM8_ESP8266
#endif
        if (++s_parser.cnt >= (ESP_PARSER_SIZE -1)) // ������ ������ ���� ������� ������ � �����
        {
            at_respond = ESP_RESPOND_ERROR_OVF;
            return at_respond;
        }
    }
    if (NULL != _strncmp ((char *)&s_parser.buf[0], (char *)&str[0])) //���� ������ ���������
    {
        at_respond = ESP_RESPOND_READY;
        return at_respond;
    }
    if (FUNCTION_RETURN_OK == modSysClock_timeout (&at_delay, timeout, SYSCLOCK_GET_TIME_MS_1))
    {
        at_respond = ESP_RESPOND_ERROR_TIMEOUT; //����� ������� ��������
    }
    return at_respond;
}

    


uint16_t wifi_level;
uint32_t wifi_i;
static int protothread1 (struct pt *pt) {
    PT_BEGIN(pt);

    esp_init_cnt = 1;
    while (1)
    {
        do
        {
            // reset ESP8266
            //PT_WAIT_UNTIL (pt, 0 == at_check ("ready"));
            if (FUNCTION_RETURN_OK != at_send (AT_RESET))
            {
                while(1){}; //�� �������� UART
            }
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("ready", 4000));
            if (ESP_RESPOND_READY == at_respond)
            {
                DEBUG_OUT("AT_READY");
            }
            else if (ESP_RESPOND_ERROR_OVF == at_respond)
            {
                //�������� ����� ������ �����
                DEBUG_OUT("AT_ERROR_OVF");
                break;
            }
            else if (ESP_RESPOND_ERROR_TIMEOUT == at_respond)
            {
                //@todo ������������� ������, �������� ��� ������ ���
                DEBUG_OUT("AT_ERROR_TIMEOUT");
                break;
            }
            
            at_send ("AT+GMR\r\n"); //checking version AT version:0.21.0.0
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 3000));
            if (ESP_RESPOND_READY != at_respond)
            {
                DEBUG_OUT("GMR_ERROR");
                break;
            }
            
            // @todo ���� �� ����� ���������, �� ��������� ����������������� � �������� ������
            at_send ("AT+CWMODE?\r\n"); // mode get
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 100));
            if (ESP_RESPOND_READY != at_respond)
            {
                DEBUG_OUT("CWMODE?_ERROR");
                break;
            }
            
#if ESP_TCP_SERVER
            at_send ("AT+CWMODE=3\r\n"); // mode set
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 100));
            if (ESP_RESPOND_READY != at_respond)
            {
                DEBUG_OUT("CWMODE=3_NOT_OK");
                PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("no change", 1));
                if (ESP_RESPOND_READY != at_respond)
                {
                    DEBUG_OUT("CWMODE=3_NC_ERROR");
                    break;
                }
            }
            
            // settings for server
            xsprintf (_str, "AT+CWSAP=\"%s\",\"%s\",5,0\r\n", SERVER_NAME, SERVER_PASS);
            at_send (_str);
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 5000));
            if (ESP_RESPOND_READY != at_respond)
            {
                DEBUG_OUT("CWSAP_ERROR");
                break;
            }
            //��� ������� ������� ������ ������ ���� � ������ ������������� ����������� AT+CIPMUX=1.
            at_send ("AT+CIPMUX=1\r\n");
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 1000));
            if (ESP_RESPOND_READY != at_respond)
            {
                DEBUG_OUT("CIPMUX_ERROR");
                break;
            }
                  //  ������ ������ ��������� ������. ?��� ������ ��������� ������� 1.
        //AT+CIPMODE=0
            at_send ("AT+CIPSERVER=1,80\r\n");
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 5000));
            if (ESP_RESPOND_READY != at_respond)
            {
                DEBUG_OUT("CIPSERVER_ERROR");
                break;
            }
            
            at_send ("AT+CIPSTO=10\r\n"); // Timeout in sec
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 5000));
            if (ESP_RESPOND_READY != at_respond)
            {
                DEBUG_OUT("CIPSERVER_ERROR");
                break;
            }
#endif      
#if ESP_UDP_SERVER //ESP_TCP_SERVER
            at_send ("AT+CWMODE=3\r\n"); // mode set
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 100));
            if (ESP_RESPOND_READY != at_respond)
            {
                DEBUG_OUT("CWMODE=3_NOT_OK");
                PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("no change", 1));
                if (ESP_RESPOND_READY != at_respond)
                {
                    DEBUG_OUT("CWMODE=3_NC_ERROR");
                    break;
                }
            }
//             // settings for server
//             xsprintf (_str, "AT+CWSAP=\"%s\",\"%s\",5,0\r\n", SERVER_NAME, SERVER_PASS);
//             at_send (_str);
//             PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 5000));
//             if (ESP_RESPOND_READY != at_respond)
//             {
//                 DEBUG_OUT("CWSAP_ERROR");
//                 break;
//             }
            //��� ������� ������� ������ ������ ���� � ������ ������������� ����������� AT+CIPMUX=1.
            at_send ("AT+CIPMUX=1\r\n");
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 1000));
            if (ESP_RESPOND_READY != at_respond)
            {
                DEBUG_OUT("CIPMUX_ERROR");
                break;
            }
            
            at_send ("AT+CIPSERVER=1,8080\r\n");
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 5000));
            if (ESP_RESPOND_READY != at_respond)
            {
                DEBUG_OUT("CIPSERVER_ERROR");
                break;
            }
            
            at_send ("AT+CIPSTART=1,\"UDP\",\"192.168.4.1\",8080,8080,2\r\n");
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 5000));
            if (ESP_RESPOND_READY != at_respond)
            {
                DEBUG_OUT("CIPSERVER_ERROR");
                break;
            }
#endif  // client
#if ESP_TERM_RS
            // @todo ������� �������, ���� �� �������������, �� ������������ � �������� 8266
            at_send ("AT+CWMODE=1\r\n"); // mode set - ������
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 100));
            if (ESP_RESPOND_READY != at_respond)
            {
                DEBUG_OUT("CWMODE=1_ERROR");
                PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("no change", 1));
                if (ESP_RESPOND_READY != at_respond)
                {
                    DEBUG_OUT("CWMODE=1_ERROR");
                    break;
                }
            }
            
            at_send ("AT+CWLAP\r\n"); // search networks
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 15000));
            if (ESP_RESPOND_READY != at_respond)
            {
                DEBUG_OUT("CWLAP_ERROR");
                break;
            }
            for (wifi_i = 0; wifi_i < 3; wifi_i++)
            {
                char *tmp_p_str = _strncmp ((char *)&s_parser.buf[0], (char *)&WIFI_SSID[wifi_i][0]);
                if (NULL != tmp_p_str)
                {
                        
                    xsprintf (_str, "AT+CWJAP=\"%s\",\"%s\"\r\n", &WIFI_SSID[wifi_i][0], &WIFI_PASS[wifi_i][0]);
                    at_send (_str); // search networks
                    PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 30000));
                    if (ESP_RESPOND_READY == at_respond)
                    {
#ifdef BOARD_STM32F4DISCOVERY
                        LED_RED_ON;
                        xsprintf (_str, "con2:%s", &WIFI_SSID[wifi_i][0]);
                        paint_putStrColRow (0, 5, (const char *)_str);
#endif
                    }
                }
            }
            
            at_send ("AT+CIPMODE?\r\n"); //
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 100));
            if (ESP_RESPOND_READY != at_respond)
            {
                DEBUG_OUT("CIPMODE?_ERROR");
                break;
            }
            
            at_send ("AT+CIPMODE=0\r\n"); // 1=�������� �����
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 1000));
            if (ESP_RESPOND_READY != at_respond)
            {
                DEBUG_OUT("CIPMODE=2_ERROR");
                break;
            }
            

#endif //ESP_TCP_SERVER 
            at_send ("AT+CIFSR\r\n"); //�������� IP-����� ������. ��� �������� ����� ����������� � WiFi �����.
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 1000));
            if (ESP_RESPOND_READY != at_respond)
            {
                DEBUG_OUT("CIFSR_ERROR");
                break;
            }
            if (ESP_RESPOND_READY == at_respond)
            {
#ifdef BOARD_STM32F4DISCOVERY
                xsprintf (_str, "IP:%s", &s_parser.buf[0]); //printing IP
                paint_putStrColRow (0, 8, (const char *)_str);
#endif
            } 
            //������������� ������ �������
            esp_init_cnt = 0;
        } while(0);
        
        if (0 == esp_init_cnt)
            break;
        
        if (++esp_init_cnt > 2)
        {
            __DI();
            ESP8266_CH_PD_L;
            _delay_ms (1); //!
            ESP8266_CH_PD_H;
            _delay_ms (1); //!
            __EI();
            esp_init_cnt = 1;
        }
        
        if (esp_init_cnt > 20)
        {
            while (1) {};
        }
        //PT_YIELD (pt);
    }
    /* Initialize the protothread state variables with PT_INIT(). */
    PT_INIT (&pt2);
    PT_SEM_INIT(&USART_mutex, 1); // take
    
    PT_END(pt);
}





extern uint16_t wifi_level;

extern uint8_t tempI, tempF;
extern uint8_t _tempI, _tempF;
extern systime_t delayTemperature;
systime_t espDelay_A;


#if ESP_TERM_RS
static const char str_GET_TMP1[] = { //&button=4
    "GET /index.php?temperature=%d.%1u" 
};
static const char str_GET_TMP2[] = {
    " HTTP/1.1\r\nHost: ht3h5793.site88.net\r\nConnection: keep-alive\r\nAccept: */*\r\n\r\n" 
};

#endif

#if ESP_TCP_SERVER 
static const char my_site_title[] = {"HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n"}; //{"HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<H1>HELL<H1/>\r\n"}; //
                
static const char my_site_page[] = {
    "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<html><head><title>Wi-Fi ESP8266 DEMO</title></head><body><H1><a href=\"/on\"><button>on</button></a> <a href=\"/off\"><button>off</button></a></body></html>\r\n"}; //length: 117
#endif


systime_t delay_temperature;
static int protothread2 (struct pt *pt) {
    uint32_t i;
    uint16_t sizeOut, _sizeOut;
    PT_BEGIN(pt);
    wifi_i = 0;
    while (1)
    {
        
//----- WIFI LEVEL -----------------------------------------------------------------------
#if ESP_WIFI_LEVEL

        at_send ("AT+CWLAP\r");
        espDelay_A = modSysClock_getTime();
        PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == modSysClock_timeout (&espDelay_A, 2000, SYSCLOCK_GET_TIME_MS_1));
        while (1)
        {
            at_respond = at_check ("+CWLAP", 4000);
            if (ESP_RESPOND_READY == at_respond)
            {
                //for (i = 0; i < 32; i++)
                char *tmp_p_str = _strncmp ((char *)&s_parser.buf[0], "\",-");
                if (NULL != tmp_p_str)
                {
                    uint8_t tmp8 = 0;
                    wifi_level = 0;
                    tmp8 = (uint8_t)tmp_p_str[4] - (uint8_t)'0';
                    wifi_level  = (uint16_t)(tmp8);
                    tmp8 = (uint8_t)tmp_p_str[3] - (uint8_t)'0';
                    wifi_level += (uint16_t)(tmp8) * 10;
                    //wifi_level = tmp_p_str[3] * 10 + tmp_p_str[4];
                    
                    //LED_BLUE_INV;
                }
                break;
            }
            if (ESP_RESPOND_ERROR_OVF == at_respond)
            {
                break;
            }
            if (ESP_RESPOND_ERROR_TIMEOUT == at_respond)
            {
                break;
            }
            PT_YIELD (pt);
        }
        //������ ��� ����?
        at_send ("AT+CWLAP\r\n");
        PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 1000)); //15000
        if (ESP_RESPOND_READY == at_respond)
        {
            for (i = 0; i < 3; i++)
            {
                char *tmp_p_str = NULL;
                tmp_p_str = _strncmp ((char *)&s_parser.buf[0], (char *)&WIFI_SSID[i][0]);
                if (NULL != tmp_p_str)
                {
                    tmp_p_str = _strncmp (tmp_p_str, "-");
                    if (NULL != tmp_p_str)
                    {
                        uint8_t tmp8 = 0;
                        wifi_level = 0;
                        tmp8 = (uint8_t)tmp_p_str[2] - (uint8_t)'0';
                        wifi_level  = (uint16_t)(tmp8);
                        tmp8 = (uint8_t)tmp_p_str[1] - (uint8_t)'0';
                        wifi_level += (uint16_t)(tmp8) * 10;
                        
                        LED_ORANGE_INV;
                    }
                }
            }
        }
#endif

#if ESP_UDP_SERVER
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("+IPD,1,1:", 1000));
            if (ESP_RESPOND_READY == at_respond)
            {
                uint8_t tmp_byte;
                PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == halUSART1_rcvS ((uint8_t *)&s_parser.buf[s_parser.cnt]));
                tmp_byte = s_parser.buf[s_parser.cnt];
                switch (tmp_byte)
                {
                    case '1':
                        LED_RED_ON;
                        break;
                    case '2':
                        LED_RED_OFF;
                        break;
                    
                    default: break;
                }
            }
#endif
            
//-- +IPD --------------------------------------------------------------------------------
#if ESP_TCP_SERVER 
            //� ��������������� �����!!!!
            //http://allaboutee.com/2015/01/20/esp8266-android-application-for-arduino-pin-control/
            at_get();
            
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("+IPD,", 1000));
            if (ESP_RESPOND_READY == at_respond)
            {
                uint8_t num_ID = 0;
                char *tmp_p_str = NULL;
                uint32_t size = 0;
                
                //_delay_ms(100);
                //������������ ID
                PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check (",", 1000));
                //PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == halUSART1_rcvS ((uint8_t *)&s_parser.buf[s_parser.cnt]));
                num_ID  = 0;//s_parser.buf[s_parser.cnt - 1];// - '0';
//                 tmp_p_str = _strncmp ((char *)&s_parser.buf[0], "+"); // ���� ����� ���������
//                 if (NULL != tmp_p_str)
//                 {
//                     
//                 }                
                xsprintf (_str, "num_ID:%u", num_ID);
                paint_putStrColRow (0, 15, (const char *)_str);

                
                //���������� ���������� �������� ����
                PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check (":", 1000));
                i = 1;
                size = 0;
                while (',' != s_parser.buf[s_parser.cnt - i])
                {
                    size *= 10;
                    size += (s_parser.buf[s_parser.cnt - i] - '0');
                    i++;
                }
                //�������� ������������� �� ���������� ������
                
                xsprintf (_str, "size:%u", size);
                paint_putStrColRow (0, 16, (const char *)_str);
                
                //������ ������
                PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("GET /on", 300));
                if (ESP_RESPOND_READY == at_respond)
                {
                    LED_RED_ON;
                }
                PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("GET /off", 1));
                if (ESP_RESPOND_READY == at_respond)
                {
                    LED_RED_OFF;
                }    
                //_delay_ms(500);

                //PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK", 3000));
                //���������� ��������-������, ��� ����� ��������� �� ������ �������
                size =_strlen((char *)&my_site_page[0]);// _strlen((char *)&my_site_title[0]) + 
                num_ID = 0;
                xsprintf (_str, "AT+CIPSEND=%u,%u\r\n", num_ID, size);
                at_send ((const char *)&_str[0]);
                PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check (">", 3000));
                if (ESP_RESPOND_READY == at_respond)
                {
                    //at_send ((const char *)&my_site_title[0]); // ���� ��������� ��������
                    //if (FUNCTION_RETURN_OK == 
                    at_send ((const char *)&my_site_page[0]);//) // ������ � �����?
                    {
                        PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK\r\n", 1000));
                        if (ESP_RESPOND_READY == at_respond)
                        {
                            LED_ORANGE_INV;
                            //at_get();
                        }
                    }
                }
            }
#endif



//------ TERMOMETR+thingspeak ---------------------------------------------------------------------
/*      
        // http://esp8266.ru/esp8266-at-commands-v019/
        delay_temperature = modSysClock_getTime ();
        PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == modSysClock_timeout (&delay_temperature, 15000, SYSCLOCK_GET_TIME_MS_1));

        //static const char str1[] = {"AT+CIPSTART=\"TCP\",\"thingspeak.com\",80\r\n"};
        xsprintf(_str, "AT+CIPSTART=\"TCP\",\"thingspeak.com\",80\r\n");
        at_send (_str);
        PT_WAIT_UNTIL(pt, AT_NOT_READY != at_check ("OK", 5000));
        if (ESP_RESPOND_READY == at_respond)
        {
            tempI++;
            
            wifi_i = xsprintf(_str, "GET /update?key=U8KDSS6RRTECTMY2&field1=%u.%u\r\n", tempI, tempF);
            //static const char str3[] = {"AT+CIPSEND=%u\n"};
            xsprintf(_str, "AT+CIPSEND=%u", wifi_i);
            at_send (_str);
            PT_WAIT_UNTIL(pt, AT_NOT_READY != at_check (">", 500));
            if (ESP_RESPOND_READY == at_respond)
            {
                
                //static const char str4[] = {"GET /update?key=U8KDSS6RRTECTMY2&field1=%u.%u\r\n"};
                xsprintf(_str, "GET /update?key=U8KDSS6RRTECTMY2&field1=%u.%u", tempI, tempF);
                at_send (_str);
                PT_WAIT_UNTIL(pt, AT_NOT_READY != at_check ("SEND OK", 500));
                if (AT_READY == at_respond)
                {
                    LED_GREEN_INV;
                }
                // timeout server - ����� ���� �� ������� (
                delay_temperature = modSysClock_getTime ();
                PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == modSysClock_timeout (&delay_temperature, 1500, SYSCLOCK_GET_TIME_MS_1));
                
            }
            
        }
        xsprintf (_str, "AT+CIPCLOSE\r\n"); // close connection (�� ����������, ������ ��� ����������)
        at_send (_str);
        PT_WAIT_UNTIL(pt, AT_NOT_READY != at_check ("OK", 500));
        if (ESP_RESPOND_READY == at_respond)
        {
        }
*/

//-- TERMOMETR+MY site ------------------------------------------------------------------------------
        //http://we.easyelectronics.ru/smarthouse/web-termometr-na-wifi-esp8266-c-otsylkoy-na-e-mail-ot-batareyki.html
#if ESP_TERM_RS
        delay_temperature = modSysClock_getTime ();
        PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == modSysClock_timeout (&delay_temperature, 5000, SYSCLOCK_GET_TIME_MS_1));
        xsprintf (_str, "AT+CIPSTART=\"TCP\",\"ht3h5793.site88.net\",80\r\n");
        at_send (_str);
        PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("CONNECT\r\n", 5000));
        if (ESP_RESPOND_READY == at_respond)
        {
            PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK\r\n", 5000));
            if (ESP_RESPOND_READY == at_respond)
            {
                tempI++;
                
                wifi_i = xsprintf(_str, str_GET_TMP1, tempI, tempF);
                xsprintf(_str, "AT+CIPSEND=%u\r\n", wifi_i);
                at_send (_str);
                PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check (">", 500));
                if (ESP_RESPOND_READY == at_respond)
                {
                    xsprintf(_str, str_GET_TMP1, tempI, tempF);
                    at_send (_str);
                    PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("SEND OK\r\n", 1000));
                    if (ESP_RESPOND_READY == at_respond)
                    {
                    }
                }
                wifi_i = xsprintf(_str, str_GET_TMP2);
                xsprintf(_str, "AT+CIPSEND=%u\r\n", wifi_i);
                at_send (_str);
                PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check (">", 500));
                if (ESP_RESPOND_READY == at_respond)
                {
                    xsprintf(_str, str_GET_TMP2);
                    at_send (_str);
                    PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("SEND OK\r\n", 1000));
                    if (ESP_RESPOND_READY == at_respond)
                    {
#if BOARD_STM32F4DISCOVERY
                        //LED_BLUE_INV;
                        xsprintf (_str, "SEND OK:%d.%u", tempI, tempF); //printing
                        paint_putStrColRow (0, 2, (const char *)_str);
#endif
                    }
                    // timeout server - ����� ���� �� ������� (
                    //+IPD,905:HTTP/1.1 200 OK
                    //PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("HTTP/1.1 200 OK", 5000));
                    if (ESP_RESPOND_READY == at_respond)
                    {
                        
                    }
                    delay_temperature = modSysClock_getTime ();
                    PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == modSysClock_timeout (&delay_temperature, 1500, SYSCLOCK_GET_TIME_MS_1));
                }
            }
        }
        xsprintf (_str, "AT+CIPCLOSE\r\n"); // close connection (�� ����������, ������ ��� ����������)
        at_send (_str);
        PT_WAIT_UNTIL(pt, ESP_RESPOND_NOT_READY != at_check ("OK\r\n", 500));
        if (ESP_RESPOND_READY == at_respond)
        {
#if BOARD_STM32F4DISCOVERY
            LED_ORANGE_INV;
#endif
        }
        

#endif
        
//--------------------------------------------------------------------------------------
//         if (0 == TRUE) // send twitt 
//         {

// #define tsData  "Sending twitt is:" //#life_is_hell HELL FROM ESP8266 "

//             //static const char str6[] = {"AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\n"};
//             sizeOut = xsprintf (_str, "AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\n");
//             halUSART1_sndM ((uint8_t *)_str, sizeOut);
//             PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == modSysClock_timeout (&delayA, 500, SYSCLOCK_GET_TIME_MS_1));
//             //DEBUG_OUT;
//             
//             // compute sizes strings
//             _sizeOut = xsprintf (_str, "%s%s%s%s%u",
//                                  "api_key=",
//                                  thingtweetAPIKey,
//                                  "&status=",
//                                  tsData,
//                                  tempI); //num);
//             // Create HTTP POST Data
//             sizeOut = xsprintf (_str, "%s%s%s%s%s%u%s",
//                                 "POST /apps/thingtweet/1/statuses/update HTTP/1.1\n",
//                                 "Host: api.thingspeak.com\n",
//                                 "Connection: close\n",
//                                 "Content-Type: application/x-www-form-urlencoded\n",
//                                 "Content-Length: ",
//                                 _sizeOut,
//                                 "\n\n");

//             sizeOut = xsprintf (_str, AT_SEND, sizeOut + _sizeOut);
//             halUSART1_sndM ((uint8_t *)_str, sizeOut);
//             PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == modSysClock_timeout (&delayA, 500, SYSCLOCK_GET_TIME_MS_1));
//             //DEBUG_OUT;
//             
//             
//             // Create HTTP POST Data
//             sizeOut = xsprintf (_str, "%s%s%s%s%s%u%s",
//                                 "POST /apps/thingtweet/1/statuses/update HTTP/1.1\n",
//                                 "Host: api.thingspeak.com\n",
//                                 "Connection: close\n",
//                                 "Content-Type: application/x-www-form-urlencoded\n",
//                                 "Content-Length: ",
//                                 _sizeOut,
//                                 "\n\n");
//             halUSART1_sndM ((uint8_t *)_str, sizeOut); // send HTTP POST
//             PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == modSysClock_timeout (&delayA, 500, SYSCLOCK_GET_TIME_MS_1));
//             //DEBUG_OUT;
//             
//             _sizeOut = xsprintf (_str, "%s%s%s%s%u",
//                                  "api_key=",
//                                  thingtweetAPIKey,
//                                  "&status=",
//                                  tsData,
//                                  tempI); //num);
//             halUSART1_sndM ((uint8_t *)_str, _sizeOut); // SEND twitt
//             //DEBUG_OUT;
//             
//             sizeOut = xsprintf (_str, AT_CLOSE); // close connection
//             halUSART1_sndM ((uint8_t *)_str, sizeOut);
//             PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == modSysClock_timeout (&delayA, 500, SYSCLOCK_GET_TIME_MS_1));
//             //DEBUG_OUT;
//             
//             //num++;
//             //key = FALSE;
//         }
    


//--------------------------------------------------------------------------------------
//         if (0 == 1)// reading command
//         {
//             halUSART1_flush ();
//             delayA = modSysClock_getTime();
//             
//             sizeOut = xsprintf (_str, "AT+CIPSTART=\"TCP\",\"thingspeak.com\",80\n");
//             halUSART1_sndM ((uint8_t *)_str, sizeOut);
//             PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == modSysClock_timeout (&delayA, 3000, SYSCLOCK_GET_TIME_MS_1));
//             DEBUG_OUT;
//             
//             // Create HTTP POST Data
//             //static const char gggg[] = {"GET https://api.thingspeak.com/talkbacks/%s/commands/execute?api_key=%s\n"};
//             sizeOut = xsprintf (_str, "GET https://api.thingspeak.com/talkbacks/%s/commands/execute?api_key=%s\n",
//                                 talkBackID,
//                                 talkBackAPIKey);

//             sizeOut = xsprintf (_str, "AT+CIPSEND=%u\n", sizeOut);
//             halUSART1_sndM ((uint8_t *)_str, sizeOut);
//             PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == modSysClock_timeout (&delayA, 1500, SYSCLOCK_GET_TIME_MS_1));
//             DEBUG_OUT;

//             
//             
//             // Create HTTP POST Data
//             sizeOut = xsprintf (_str, "GET https://api.thingspeak.com/talkbacks/%s/commands/execute?api_key=%s\n",
//                                 talkBackID,
//                                 talkBackAPIKey);
//             halUSART1_sndM ((uint8_t *)_str, sizeOut);
//             PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == modSysClock_timeout (&delayA, 1500, SYSCLOCK_GET_TIME_MS_1));
//             //DEBUG_OUT;
//             i = 0;
//             while (i++ < 200) {
//                 if (FUNCTION_RETURN_OK == halUSART1_rcvS (&tmp8)) {
//                     //USB_sendM ((uint8_t *)&tmp8, 1);
//                     switch (tmp8) {
//                         case '(':
//                             LED_GREEN_OFF; break;
//                         case ')':
//                             LED_GREEN_ON; break;
//                         default: break;
//                     }
//                 } else { break; }
//                 //_delay_ms(1);
//                 //PT_YIELD (pt);
//             }
//             
//             sizeOut = xsprintf (_str, AT_CLOSE); // close connection
//             halUSART1_sndM ((uint8_t *)_str, sizeOut);
//             PT_WAIT_UNTIL (pt, FUNCTION_RETURN_OK == modSysClock_timeout (&delayA, 1500, SYSCLOCK_GET_TIME_MS_1));
//             DEBUG_OUT;
//         }
        
        //} // 15sec timeout
        PT_YIELD (pt);
    } // while (1)
    PT_END(pt);
}


void    drvESP8266_init (uint8_t num)
{
    s_parser.cnt = 0;
    /* Initialize the protothread state variables with PT_INIT(). */
    PT_INIT (&pt1);
    
    ESP8266_CH_PD_H;
    ESP8266_RESET_H;
    while (1)
    {
        protothread1 (&pt1);
        if (0 == esp_init_cnt)
            break;
    }
    
}



void    drvESP8266_run (void)
{
    protothread2 (&pt2);
}


#endif
