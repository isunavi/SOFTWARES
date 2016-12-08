#include "modGPS.h"
#include "board.h"


/*  Example for test:
 *  $GPRMC,094508.30,A,4959.21828,N,03615.50791,E,0.003,221.11,210716,,,A*77\r\n
 *  $GPGGA,094508.30,4959.21828,N,03615.50791,E,1,03,5.23,220.1,M,17.0,M,,*59\r\n
 */

//char fmt[]="$GPRMC,dddtdd.ddm,A,eeae.eeee,l,eeeae.eeee,o,djdk,ddd.dc,dddy??,,,?*??";
char fmtRMC[]="$GPRMC,dddddt.dm,s,eeee.eeeee,l,eeeee.eeeee,o,djddk,ddd.dc,dddddy??,,,?*??";
             //$GPGGA,094508.30,4959.21828,N,03615.50791,E,1,03,5.23,220.1,M,17.0,M,,*59
char fmtGGA[]="$GPGGA,??????.??,????.?????,?,?????.?????,?,?,??,?.??,dja,?,??.?,?,,*??";

extern modGPS_RMC_struct gpsRMC;
extern modGPS_GGA_struct gpsGGA;

void modGPS_init (modGPS_RMC_struct *s, uint32_t baud)
{
    gpsRMC.state = 0;
    gpsGGA.state = 0;

    gpsRMC.Fix = 0;
    gpsGGA.Fix = 0;

}


void modGPS_parseRMC (char c)
{
    if (c == '$') { gpsRMC.state = 0; gpsRMC.temp = 0; }
    char mode = fmtRMC[gpsRMC.state++];
    // If received character matches format string, or format is '?' - return
    if ((mode == c) || (mode == '?')) return;
    // d=decimal digit
    char d = c - '0';
    if (mode == 'd') gpsRMC.temp = gpsRMC.temp*10 + d;
    // e=long decimal digit
    else if (mode == 'e') gpsRMC.ltmp = gpsRMC.ltmp*10 + d;
    // a=angular measure
    else if (mode == 'a') gpsRMC.ltmp = gpsRMC.ltmp*6 + d;
    // t=Time - hhmm
    else if (mode == 't') { gpsRMC.Time = gpsRMC.temp*10 + d; gpsRMC.temp = 0; }
    // m=Millisecs
    else if (mode == 'm') { gpsRMC.Msecs = gpsRMC.temp*10 + d; gpsRMC.ltmp=0; }
    // A - valid, V - not valid //for test
    else if (mode == 's') { if (c == 'A') gpsRMC.Fix = 1; if (c == 'V') gpsRMC.Fix = 0; }
    // l=Latitude - in minutes*10000
    else if (mode == 'l') { if (c == 'N') gpsRMC.Lat = gpsRMC.ltmp; else gpsRMC.Lat = -gpsRMC.ltmp; gpsRMC.ltmp = 0; }
    // o=Longitude - in minutes*10000
    else if (mode == 'o') { if (c == 'E') gpsRMC.Long = gpsRMC.ltmp; else gpsRMC.Long = -gpsRMC.ltmp; gpsRMC.temp = 0; }
    // j/k=Speed - in knots*100
    else if (mode == 'j') { if (c != '.') { gpsRMC.temp = gpsRMC.temp*10 + d; gpsRMC.state--; } }
    else if (mode == 'k') { gpsRMC.Knots = gpsRMC.temp*10 + d; gpsRMC.temp = 0; }
    // c=Course (Track) - in degrees*100
    else if (mode == 'c') { gpsRMC.Course = gpsRMC.temp*10 + d; gpsRMC.temp = 0; }
    // y=Date - ddmm
    else if (mode == 'y') { gpsRMC.Date = gpsRMC.temp*10 + d; }

    else gpsRMC.state = 0;
}


void modGPS_parseGGA (char c)
{
    if (c == '$') { gpsGGA.state = 0; gpsGGA.temp = 0; }
    char mode = fmtGGA[gpsGGA.state++];
    // If received character matches format string, or format is '?' - return
    if ((mode == c) || (mode == '?')) return;
    // d=decimal digit
    char d = c - '0';
    if (mode == 'd') gpsGGA.temp = gpsGGA.temp*10 + d;
    else if (mode == 'j') { if (c != '.') { gpsGGA.temp = gpsGGA.temp*10 + d; gpsGGA.state--; } }
    // a=Altitude
    else if (mode == 'a') { gpsGGA.Altitude = gpsGGA.temp*10 + d; gpsGGA.temp = 0; gpsGGA.Fix = 1;}

    else gpsGGA.state = 0;
}


void _modGPS_test (modGPS_RMC_struct *s, uint16_t baud)
{
    //gps.state = 0;

}


int32_t modGPS_dms2dd (int32_t val) //@todo
{
    int32_t tmps32_A, tmps32_B, tmps32_C;

    tmps32_A = val / 10000000; //отсекаем недесятичную часть
    tmps32_A = tmps32_A * 10000000;

    tmps32_B = val % 10000000;
    tmps32_B = (tmps32_B * 100) / 60;

    tmps32_C = val % 100000;
    tmps32_C = tmps32_C * 100000;
    tmps32_C = tmps32_C / 36000;

    val = tmps32_A + tmps32_B;

    return val;
}

