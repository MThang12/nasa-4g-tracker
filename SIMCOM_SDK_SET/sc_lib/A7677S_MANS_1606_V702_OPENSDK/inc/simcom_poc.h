#ifndef __SIMCOM_POC_H__
#define __SIMCOM_POC_H__

#include "simcom_os.h"

enum
{
    POC_PCM_IDLE       = 0x0,
    POC_PCM_PLAYING    = 0x1 ,
    POC_PCM_RECORDING  = 0x2,
    POC_PCM_MAX,
};

enum RecrodDataType
{
    AUD_RECORD_NOTIFY_DATA_FORWORDLY = 1,
    AUD_RECORD_NOTIFY_DATA_PASSIVELY,
    AUD_RECORD_NOTIFY_DATA_MAX
};

typedef void (*PaConfigCallback)(int onoff, int mode);
#if (defined CUS_GWSD) || (defined CUS_CORGET) || (defined CUS_BND) || (defined CUS_CY)
typedef void (* fn_type_report_play_buffer_rest_data_cb)(int rest_data);
typedef void (* fn_type_report_record_data_cb)(const char* data, int length);

int sAPI_PocInitLib(fn_type_report_play_buffer_rest_data_cb callback);
int sAPI_PocStartRecord(fn_type_report_record_data_cb callback,int mode);
#else
typedef int(*sAPI_ProcessRecordCb)(char *buffer,int len);
typedef int(*sAPI_NotifyBufStateCb)(UINT32 DataLen);

int sAPI_PocInitLib(sAPI_NotifyBufStateCb callback);
int sAPI_PocStartRecord(sAPI_ProcessRecordCb callback,int mode);
#endif
#ifdef CUS_QZXC
int sAPI_GetVailableSize(int *size);
#endif

int sAPI_PocPlaySound(const char *data, int length);
int sAPI_PocStopSound(void);
int sAPI_PocGetPcmAvail(void);
int sAPI_PocCleanBufferData(void);
int sAPI_PocStopRecord(void);
int sAPI_PocPcmRead(char *buffer,int dataLen);
int sAPI_TonePlay(int Type,int time);
void sAPI_ToneStop(void);
int sAPI_FreqPlay(int freq1,int freq2,int time);
void sAPI_SetToneVolume(int volume);
int sAPI_GetToneVolume(void);

#endif