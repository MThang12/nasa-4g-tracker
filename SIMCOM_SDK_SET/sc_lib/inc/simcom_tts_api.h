#ifndef __SIMCOM_TTS_API_H__
#define __SIMCOM_TTS_API_H__
#if (defined FEATURE_SIMCOM_TTS_IFLY) || (defined FEATURE_SIMCOM_TTS_YOUNGTONE)
#include "simcom_os.h"
#include "simcom_tts.h"

BOOL sAPI_TTSSetParameters(UINT8 volume,UINT8 sysVolume,UINT8 digitMode,UINT8 pitch,UINT8 speed);
#endif
#endif
