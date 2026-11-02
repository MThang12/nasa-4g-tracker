#ifndef __SIMCOM_FOTA_DOWNLOAD_H__
#define __SIMCOM_FOTA_DOWNLOAD_H__


#if 0 
#define SC_FOTA_MAX_STRING_LEN 128


typedef int (*sc_fota_callback)(int);

struct SC_FotaApiParam
{
	char host[SC_FOTA_MAX_STRING_LEN]; /*xx.xx.xx.xx:port, or URL*/
	char username[SC_FOTA_MAX_STRING_LEN];
	char password[SC_FOTA_MAX_STRING_LEN];
	unsigned char mode; /*0: ftp, 1: http*/
    sc_fota_callback sc_fota_cb;
    int timeout;
    int ssl_index;//reverd
    int cid;
};

typedef struct
{
    unsigned char enable;
    unsigned char stage;
}SC_MiniSysStatus;

void sAPI_EnterMiniSys(void);
void sAPI_GetMiniSysStatus(SC_MiniSysStatus *ministatus);
int sAPI_FotaServiceBegin(void *pfotaParam);
int sAPI_FotaImageWrite(char *data, int len, int file_size);
int sAPI_FotaImageVerify(int file_size);
#endif
#define SC_FOTA_MAX_STRING_LEN 257

typedef int (*sc_fota_callback)(int);

typedef struct
{
    unsigned char enable;
    unsigned char stage;
}SC_MiniSysStatus;

struct SC_FotaApiParam
{
    char host[SC_FOTA_MAX_STRING_LEN]; /*xx.xx.xx.xx:port, or URL*/
    char username[SC_FOTA_MAX_STRING_LEN];
    char password[SC_FOTA_MAX_STRING_LEN];
    unsigned char mode; /*0: ftp, 1: http*/
    sc_fota_callback sc_fota_cb;
};

int sAPI_GetFotaUpgradeFlag(void);
unsigned int sAPI_eraseFotaParamFlag(void);
void sAPI_EnterMiniSys(void);
void sAPI_GetMiniSysStatus(SC_MiniSysStatus *ministatus);
int sAPI_FotaServiceBegin(void *pfotaParam);
int sAPI_FotaImageWrite(char *data, int len, int file_size);
int sAPI_FotaImageVerify(int file_size);
#ifdef FEATURE_SIMCOM_NORFLASH_MINI_LFOTA
int sAPI_LFotaServiceBegin(void *pfotaParam);
int sAPI_LFotaPreVerify(void *pfotaParam);
#endif
#endif

