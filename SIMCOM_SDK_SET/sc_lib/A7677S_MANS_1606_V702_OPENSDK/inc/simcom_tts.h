#ifndef _SIMCOM_TTS_H_
#define  _SIMCOM_TTS_H_
#include "simcom_os.h"

#if (defined CUS_GWSD) || (defined CUS_BND) || (defined CUS_CORGET) || (defined CUS_CY)
typedef enum
{
    SC_TTS_STOP,
    SC_TTS_START,
    SC_TTS_FAIL,
    SC_TTS_STATUS_MAX,
}TTSStatus;
#else
typedef enum
{
    SC_TTS_START,
    SC_TTS_STOP,
    SC_TTS_FAIL,
    SC_TTS_STATUS_MAX,
}TTSStatus;
#endif

typedef void (*sAPI_TTSStatussCb)(TTSStatus flag);

/*****************************************************************************
* FUNCTION
*  sAPI_TTSPlay
*
* DESCRIPTION
*  TTS play
*  
* PARAMETERS
*  option    Encoding type    1:UNICODE 2:ASCII_GBK
*  inputText Input the text to play
*  playMode  Near or Remote   0:PLAY_LOCAL 1:PLAY_REMOTE
* RETURNS
*  true    Play sucessfully
*  fail    Play failure
* NOTE
*  
*****************************************************************************/
BOOL sAPI_TTSPlay(UINT8 option,char *inputText,UINT8 playMode);

/*****************************************************************************
* FUNCTION
*  sAPI_TTSStop
*
* DESCRIPTION
*  Stop playing
*  
* PARAMETERS
*  no
*
* RETURNS
*  true    Stop sucessfully
*  fail    Stop failure
* NOTE
*  
*****************************************************************************/
BOOL sAPI_TTSStop(void);

/*****************************************************************************
* FUNCTION
*  TTSSetParameters
*
* DESCRIPTION
*
*
* PARAMETERS
*  volume    Module volume
*  sysVolume tts library volume
*  digitMode mode selection
*  pitch     pitch selection
*  speed     speed selection
* RETURNS
*  true    Set tts parameters sucessfully
*  fail    Failed to set tts parameters
* NOTE
*  
*****************************************************************************/
BOOL TTSSetParameters(UINT8 volume,UINT8 sysVolume,UINT8 digitMode,UINT8 pitch,UINT8 speed);

/*****************************************************************************
* FUNCTION
*  sAPI_TTSSetStatusCallBack
*
* DESCRIPTION
*  Set the callback function to gets the status of the current tts
*  
* PARAMETERS
*  ttsCb    State callback function
*
* RETURNS
*  no
*
* NOTE
*  
*****************************************************************************/
void sAPI_TTSSetStatusCallBack(sAPI_TTSStatussCb ttsCb);
#endif