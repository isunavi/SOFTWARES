/**
 * @file    halUART.h
 * @author  Ht3h5793
 * @date    19.07.2016
 * @version V1.0.0

*/

#ifndef MODGPS_H
#define	MODGPS_H 20160719

#include "board.h"


typedef struct modGPS_RMC_struct_ {

	int16_t state; // = 0;
	int32_t temp;
	int32_t ltmp;

	int32_t Time, Msecs, Knots, Course, Date;
	int32_t Lat, Long;
	bool_t Fix;
	bool_t Valide;
} modGPS_RMC_struct;

typedef struct modGPS_GGA_struct_ {

	int16_t state; // = 0;
	int32_t temp;
	int32_t ltmp;

	bool_t Fix;
	int32_t Altitude;
} modGPS_GGA_struct;


#ifdef	__cplusplus
extern "C" {
#endif

/**
 *
 */
void modGPS_init (modGPS_RMC_struct *s, uint32_t baud);

/**
 *
 */
void      modGPS_parseRMC (char c);
void      modGPS_parseGGA (char c);


/**
 * Convert GPA data format from dd.mm.sssss to dd.dd.ddddd
 * All data is fixed point
 */
int32_t modGPS_dms2dd (int32_t val);


#ifdef	__cplusplus
}
#endif

#endif	/* MODGPS_H */
