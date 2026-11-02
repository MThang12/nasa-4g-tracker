/**
 * simcom_sjdr.h
 */
#ifndef _SIMCOM_SJDR_H_
#define _SIMCOM_SJDR_H_

#include "simcom_os.h"
typedef unsigned char    uint8_t;


unsigned int sAPI_JDSet(char enable);
void sAPI_JDConfig(UINT16 period, UINT16 mnl, UINT16 minch, UINT8 detecstat, char (*callBackFunc)(UINT32 id));
void sAPI_JDTopFlyConfig(     uint8_t period, uint8_t mnl, uint8_t minsinr, uint8_t minch, uint8_t detecstat, char (*callBackFunc)(UINT32 id));


#endif
