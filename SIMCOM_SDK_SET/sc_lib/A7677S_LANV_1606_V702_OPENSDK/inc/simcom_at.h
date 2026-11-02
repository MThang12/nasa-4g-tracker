#ifndef _SIMCOM_AT_H_
#define _SIMCOM_AT_H_

#include "simcom_os.h"

void ScInternalAtInit(void);
BOOL sAPI_AtSend(UINT8 *data,UINT32 length);

#endif


