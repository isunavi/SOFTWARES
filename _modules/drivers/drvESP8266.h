/**
 * @file    halI2C.h
 * @author  Ht3h5793, CD45
 * @date    13.6.2013  2:12
 * @version V6.0.9
 * @brief 

https://github.com/bafeigum/ESP8266-Library/blob/master/test/ARDUINO/ESP8266_at_commands/ESP8266_at_commands.ino
http://geektimes.ru/post/241054/
http://esp8266.ru/esp8266-at-commands-v021/
http://pcus.ru/1897-esp8266-nachalo.html
https://github.com/esp8266/esp8266-wiki/wiki
http://www.electrodragon.com/w/Wi07c
http://digitrode.ru/computing-devices/mcu_cpu/186-pogodnaya-stanciya-na-osnove-arduino-i-wi-fi-modulya-esp8266.html
https://github.com/itead/ITEADLIB_Arduino_ESP8266/blob/master/uartWIFI.cpp
http://www.instructables.com/files/orig/FWL/65I4/H9T4SEG4/FWL65I4H9T4SEG4.cpp


http://oioki.ru/2011/09/proveryaem-rabotu-smtp-auth-login-cherez-telnet/
http://rusua.org.ua/2013/01/28/avtorizaciya-smtp-auth-plain-i-auth-login-chtokak-i-kak-proverit-vruchnuyu/
http://we.easyelectronics.ru/electro-and-pc/podklyuchenie-mikrokontrollera-k-lokalnoy-seti-tcp-klient.html
http://secundomer.ru/index.php?r=base64/encode

https://github.com/itead/ITEADLIB_Arduino_ESP8266/blob/master/examples/chatServer/chatServer.ino

// arduino - temboo
http://www.instructables.com/id/Arduino-Etheret-Project-Hey-You-Get-away-from-my-h/step7/How-to-send-google-email-from-Arduino-through-TEMB/


http://habrahabr.ru/post/224137/
https://www.pushingbox.com/services.php


риали воркинг! - 
https://github.com/iobridge/ThingSpeak-Arduino-Examples/blob/master/Ethernet/Arduino_to_ThingTweet.ino
http://www.instructables.com/id/ESP8266-Wifi-Temperature-Logger/3/?lang=ja
//SWG
http://www.w3schools.com/svg/svg_polygon.asp


*/


 


#ifndef DRVESP8266_H
#define	DRVESP8266_H 20150805

/**
 *  –аздел дл€ "include"
 */
#include "board.h"


/**
 *  –аздел дл€ "define"
 */
#define ESP8266_USART_BAUD USART_BAUD_115200 // or 9600

/*
nslookup google.ru
209.85.229.104,
216.239.59.104,
74.125.77.104
74.125.77.147

api.openweathermap.org
APPID (API key) 27fcf4e931cce36026b81880da7baf28

//twitter
381458838-5RdaAj2kDcf0Uf4wpVCe1fmB4tkhYUsSpkBp3rh2
*/



/**
#define WIFI_encryption

#endif // AT_COMMANDS_H //
*/

/**
 *  –аздел дл€ "typedef"
 */



enum ESP_MODE {
    OPEN = 0,
    WEP = 1,
    WAP_PSK = 2,
    WAP2_PSK = 3,
    WAP_WAP2_PSK = 4
};


//AT + CWMODE = <режим> Ц задает режим работы модул€ mode: 1 Ц клиент, 2 Ц точка доступа, 3 Ц смешанный режим

static const char   AT_RESET[]      = {"AT+RST\r\n"};      //сброс 
//ѕереключение режима wifi (1 - клиент, 2 - точка доступа, 3 - и то и то.). “ребуетс€ перезапуск AT+RST 
#define AT_WIFI_MODE 		"AT+CWMODE"     //
#define AT_JOIN_AP 			"AT+CWJAP"      //ѕодключение к AP
#define AT_LIST_AP 			"AT+CWLAP\r\n"    //ќтобразить список доступных AP - +CWLAP:(3,"Putin_Huilo",-53,"74:d0:2b:57:24:2c",6)
#define AT_QUIT_AP 			"AT+CWQAP\r\n"
#define AT_SET_AP_PARAMS 	"AT+CWSAP\r\n" 
#define AT_GET_CON_STATUS 	"AT+CIPSTATUS\r\n"
#define AT_START_CON 		"AT+CIPSTART"
#define AT_SEND 			"AT+CIPSEND=%u\r\n"
#define AT_CLOSE 			"AT+CIPCLOSE\n"
#define AT_SET_MUL_CON 		"AT+CIPUX"
#define AT_SET_SERVER 		"AT+CIPSERVER"
#define AT_RECEIVED_DATA	"+IPD"


// //known AP
// static const char WIFI_SSID [][18] = {
//     "MaxHome",
//     "Putin_Huilo",
//     "wifi_126_network"
//     //4
// };

// static const char WIFI_PASS [][18] = {
//     "0956963966",
//     "la-la-la-la",
//     "network_pass"
//     //4
// };

static const char   talkBackID[] = {"1104"}; // трек ID
static const char   talkBackAPIKey[] = {"D1DPMB8W4HF4T94N"}; // API key
static const char   thingtweetAPIKey[] = {"6PN9WO11H6OVSF6N"};


/*
Steps and note
AT+RST restart the module, received some strange data, and "ready"
AT+CWMODe=3 change the working mode to 3, AP+STA, only use the most versatile mode 3 (AT+RST may be necessary when this is done.)

Join Router
AT+CWLAP search available wifi spot
AT+CWJAP=Уyou ssidФ, УpasswordФ join my mercury router spot
AT+CWJAP=? check if connected successfully, or use AT+CWJAP?

TCP Client
AT+CIPMUX=1 turn on multiple connection
AT+CIPSTART=4,"TCP","192,168.1.104",9999 connect to remote TCP server 192.168.1.104 (the PC)
AT+CIPMODE=1 optionally enter into data transmission mode
AT+CIPSEND=4,5 send data via channel 4, 5 bytes length (see socket test result below, only "elect" received), link will be "unlink" when no data go through

TCP Server
AT+CIPSERVER=1,9999 setup TCP server, on port 9999, 1 means enable
AT+CIFSR check module IP address
PC as a TCP client connect to module using socket test, send data

UDP server


http://www.avislab.com/blog/esp8266_ru/

*/


#ifdef	__cplusplus
extern "C" {
#endif

/**
 *  –аздел дл€ прототипов функций
 */
void    drvESP8266_init (uint8_t num);
void    drvESP8266_run (void);

#ifdef	__cplusplus
}
#endif

#endif	/** DRVESP8266_H */
