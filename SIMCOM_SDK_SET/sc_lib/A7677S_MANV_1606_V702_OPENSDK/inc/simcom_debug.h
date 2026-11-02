#ifndef _SIMCOM_DEBUG_H
#define _SIMCOM_DEBUG_H

int sAPI_Debug(const char *format, ...);

void sAPI_Printf(char* log);

void sAPI_disableDUMP(void);

void sAPI_enableDUMP(void);

#endif



