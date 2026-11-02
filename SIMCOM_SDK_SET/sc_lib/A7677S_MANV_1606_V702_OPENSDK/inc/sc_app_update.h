#ifndef __SC_APP_UPDATE_H__
#define __SC_APP_UPDATE_H__

#include <stdint.h>

typedef struct{
    int hasPackage;
    unsigned int binSize;
    unsigned int crcValue;
}SCAppPackageInfo;

int sAPI_AppPackageOpen(char *mode);
int sAPI_AppPackageWrite(char * data, unsigned int size);
int sAPI_AppPackageRead(char *data, unsigned int size);
int sAPI_AppPackageClose(void);
int sAPI_AppPackageCrc(SCAppPackageInfo *pInfo);
int sAPI_AppPackageSizeValied(int size);

#ifdef CUS_TOPFLY
unsigned int sAPI_AppFlashCrcGet(void);
#endif

#endif