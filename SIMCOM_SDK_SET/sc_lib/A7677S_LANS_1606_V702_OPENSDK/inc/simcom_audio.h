#ifndef __SIMCOM_AUDIO_H__
#define __SIMCOM_AUDIO_H__

//#include "simcom_file_system.h"
#include "simcom_os.h"
#include "simcom_gpio.h"

#define AUDIO_FILENAME_MAX_LEN          60
#define AUDIO_FILENAME_MAX_NUM          (20)
#define AUDIO_PLAY_INTERRUPT            0x80

#ifndef AMR_TX_WMF
#define AMR_TX_WMF  0
#endif
#ifndef AMR_TX_IF2
#define AMR_TX_IF2  1
#endif
#ifndef AMR_TX_ETS
#define AMR_TX_ETS  2
#endif

typedef int     Flag;
typedef short Word16;
typedef signed char Word8;
typedef unsigned char UWord8;

typedef void (*AUDIO_SpeakerPA_T) (UINT32 on);
typedef void(*AudRecCallback)(UINT8 status);
typedef void(*sAPI_GetAmrFrameCB)(const UINT8* buf, UINT32 size);
typedef void(*sAPI_GetPCMFrameCB)(const UINT8* buf, UINT32 size);

typedef  short tMsgLen;
typedef  short *tpMsgType;
typedef void(*sAPI_GetPCMFrameCB)(const UINT8* buf, UINT32 size);

typedef struct
{
    signed short resmode;
    signed short dtTresh;
    signed short gamma;
    signed short nonlinear;
    signed short noiseFloorDbov;
    signed short MaxSuppressDb;
    signed short PeakNoiseDb;
    signed short NoiseMatrix;
    signed short txBoostMode;
    signed short txBoostGainDb;
    signed short txPeakGainDb;
    signed short txBoostNoiseGainDb;
    signed short txAgcMaxGainDb;
    signed short rxBoostMode;
    signed short rxBoostGainDb;
    signed short rxPeakGainDb;
    signed short rxBoostNoiseGainDb;
    signed short rxAgcMaxGainDb;
    signed short DAC_DigGain;
    signed short DAC_Gain;
    signed short RCV_Gain;
    signed short PGA_Stage1;
    signed short PGA_Stage2;
    signed short ADC_DIgGain;
    signed short sampleDelay;
    signed short numOfTaps;
    signed short convergenceSpeed;
    signed short ref2EchoPowerRatio;
} SC_Dspconfiguration;

typedef enum
{
    SAMPLE_RATE_8K = 0,
    SAMPLE_RATE_16K,
    SAMPLE_RATE_MAX
}AUD_SampleRate;

typedef enum
{
    REC_PATH_LOCAL = 0,
    REC_PATH_MAX
}AUD_RecordSrcPath;

typedef enum
{
    AUDIO_VOLUME_MUTE = 0,
    AUDIO_VOLUME_1,
    AUDIO_VOLUME_2,
    AUDIO_VOLUME_3,
    AUDIO_VOLUME_4,
    AUDIO_VOLUME_5,
    AUDIO_VOLUME_6,
    AUDIO_VOLUME_7,
    AUDIO_VOLUME_8,
    AUDIO_VOLUME_9,
    AUDIO_VOLUME_10,
    AUDIO_VOLUME_11,
} AUD_Volume;

typedef enum
{
    AUD_PLAY_STOP,
    AUD_PLAY_PLAYING,
    AUD_PLAY_STATE_MAX
}AUD_STREAM_PLAY_STATE_E;

typedef enum AudioFileType{
    TYPE_UNKNOWN,
    TYPE_AMR,
    TYPE_MP3,
    TYPE_AAC,
    TYPE_PCM,
    TYPE_WAV,
    TYPE_MID,
    TYPE_BUFFER,
    TYPE_MP3_STREAM,
    TYPE_AMR_STREAM,
    TYPE_WAV_HIGH
    //TYPE_MAX
}AudioFileType_t;


typedef struct AudioStreamInfo
{
    char fileNameStr[AUDIO_FILENAME_MAX_LEN];
    char fileName[AUDIO_FILENAME_MAX_LEN];
    AudioFileType_t fileType;
    char *buf;
    UINT32 len;
    AUD_SampleRate rate;
    sMutexRef mutex;
    sFlagRef playStateFlag;
    AUD_STREAM_PLAY_STATE_E playState;
    BOOL isSingleFile;
    BOOL fileEndFlag;
    UINT32 replayCount;/*repeat times*/
    UINT8 playPath;/*0:local,1:remote*/
}AudioStreamInfo_t;

typedef enum
{
    AUD_RECORD_STOP,
    AUD_RECORD_RECORDING,
    AUD_RECORD_STATE_MAX
}AUD_RECORD_STATE_E;

enum
{
    AUDIO_STATUS_IDLE,
    AUDIO_STATUS_BUSY,
};

typedef struct AudioRecordInfo
{
    char fileNameStr[AUDIO_FILENAME_MAX_LEN];
    char fileName[AUDIO_FILENAME_MAX_LEN];
    AudioFileType_t fileType;
    AUD_SampleRate rate;
    BOOL isFull;
    AUD_RECORD_STATE_E recordState;
    UINT32 receiveDataLen;
    UINT32 writeDataLen;
    BOOL isPocFunc;
    int cbMode;
    sAPI_GetPCMFrameCB PCMStreamCb;
}AudioRecordInfo_t;

typedef struct
{
    void *encCtx;
    void *pidSyncCtx;
}AmrNbEncState_S;
typedef enum
{
    /*
     *    One word (2-byte) to indicate type of frame type.
     *    One word (2-byte) to indicate frame type.
     *    One word (2-byte) to indicate mode.
     *    N words (2-byte) containing N bits (bit 0 = 0xff81, bit 1 = 0x007f).
     */
    AMR_ETS = 0, /* Both AMR-Narrowband and AMR-Wideband */

    /*
     *    One word (2-byte) for sync word (good frames: 0x6b21, bad frames: 0x6b20)
     *    One word (2-byte) for frame length N.
     *    N words (2-byte) containing N bits (bit 0 = 0x007f, bit 1 = 0x0081).
     */
    AMR_ITU, /* AMR-Wideband */

    /*
     *   AMR-WB MIME/storage format, see RFC 3267 (sections 5.1 and 5.3) for details
     */
    AMR_MIME_IETF,

    AMR_WMF, /* AMR-Narrowband */

    AMR_IF2  /* AMR-Narrowband */

} bitstream_format_AMR;

enum Mode_AMR
    {
        AMR_MR475 = 0,
        AMR_MR515,
        AMR_MR59,
        AMR_MR67,
        AMR_MR74,
        AMR_MR795,
        AMR_MR102,
        AMR_MR122,
        AMR_MRDTX,
        AMR_N_MODES     /* number of (SPC) modes */
    };

enum Frame_Type_3GPP_AMR
    {
        NB_AMR_475 = 0,        /* 4.75 kbps    */
        NB_AMR_515,            /* 5.15 kbps    */
        NB_AMR_59,             /* 5.9 kbps     */
        NB_AMR_67,             /* 6.7 kbps     */
        NB_AMR_74,             /* 7.4 kbps     */
        NB_AMR_795,            /* 7.95 kbps    */
        NB_AMR_102,            /* 10.2 kbps    */
        NB_AMR_122,            /* 12.2 kbps    */
        NB_AMR_SID,            /* GSM AMR DTX  */
        NB_GSM_EFR_SID,        /* GSM EFR DTX  */
        NB_TDMA_EFR_SID,       /* TDMA EFR DTX */
        NB_PDC_EFR_SID,        /* PDC EFR DTX  */
        NB_FOR_FUTURE_USE1,    /* Unused 1     */
        NB_FOR_FUTURE_USE2,    /* Unused 2     */
        NB_FOR_FUTURE_USE3,    /* Unused 3     */
        NB_AMR_NO_DATA         /* No data      */
    };

enum RXFrameType
    {
        RX_SPEECH_GOOD = 0,
        RX_SPEECH_DEGRADED,
        RX_ONSET,
        RX_SPEECH_BAD,
        RX_SID_FIRST,
        RX_SID_UPDATE,
        RX_SID_BAD,
        RX_NO_DATA,
        RX_N_FRAMETYPES     /* number of frame types */
    };

/*AMR Encode*/
Word16 sAPI_AmrEncodeInit(void **pEncStructure, void **pSidSyncStructure, Flag dtx_enable);
void sAPI_AmrEncodeExit(void **pEncStructure, void **pSidSyncStructure);
Word16 sAPI_AmrEncode(  void *pEncState,
                        void *pSidSyncState,
                        enum Mode_AMR mode,
                        Word16 *pEncInput,
                        UWord8 *pEncOutput,
                        enum Frame_Type_3GPP_AMR *p3gpp_frame_type,
                        Word16 output_format);

/*AMR Decode*/
Word16 sAPI_AmrDecodeInit(void **state_data, Word8 *id);
void sAPI_AmrDecodeExit(void **state_data);
Word16 sAPI_AmrDecode(  void *state_data,
                        enum Frame_Type_3GPP_AMR frame_type,
                        UWord8 *speech_bits_ptr,
                        Word16 *raw_pcm_buffer,
                        bitstream_format_AMR input_format);

void AudioParametersDeinit(void);
UINT32 RecordGetMaxFileLen(void);
void RecordParametersDeinit(void);
void WavRecordTask(void *para);
void sAPP_RecordTaskInit(void);
void simcom_stop_audio_incall(void);
void simcom_stop_recording_incall(void);
AUD_STREAM_PLAY_STATE_E sc_GetAudioPlayStatus(void);
AUD_RECORD_STATE_E sc_GetRecodeStatus(void);

void sAPI_BindSpeakerPA_CB(AUDIO_SpeakerPA_T cb);
void sAPI_AudioSetAmrRateLevel(int level);
BOOL sAPI_AudioRecord(BOOL enable, AUD_RecordSrcPath path, char *file);
BOOL sAPI_AudRec(UINT8 oper,char *file,UINT8 duration,AudRecCallback callback);
BOOL sAPI_PCMStreamRec(sAPI_GetPCMFrameCB callback);
BOOL sAPI_PCMStopStreamRec(void);
BOOL sAPI_AmrStreamRecord(sAPI_GetAmrFrameCB getframe);
BOOL sAPI_AmrStopStreamRecord(void);
BOOL sAPI_PCMStreamRecord(sAPI_GetPCMFrameCB callback);
BOOL sAPI_PCMStopStreamRecord(void);

BOOL sAPI_AudioPlaySampleRate(AUD_SampleRate rate);
BOOL sAPI_AudioPlay(char *file, BOOL direct,BOOL isSingle);
BOOL sAPI_AudioStop(void);
BOOL sAPI_AudioPcmPlay(char *buffer, UINT32 len, BOOL direct);
BOOL sAPI_AudioPcmStop(void);
BOOL sAPI_AudioMp3StreamPlay(char *buffer,UINT32 len,BOOL direct);
BOOL sAPI_AudioMp3StreamStop(void);
BOOL sAPI_AudioAmrStreamPlay(char *buffer,UINT32 len,BOOL direct);
BOOL sAPI_AudioAmrStreamStop(void);
BOOL sAPI_AudioPlayAmrCont(char *file, BOOL startplay);
BOOL sAPI_AudioWavFilePlay(char *file, BOOL direct);
UINT8 sAPI_AudioStatus(void);
BOOL sAPI_AudioSetPlayPath(UINT8 path);
UINT8 sAPI_AudioGetPlayPath(void);

void sAPI_AudioSetMicGain(UINT32 micgain);
int sAPI_AudioGetMicGain(void);
SC_GPIOReturnCode sAPI_AudioSetPaCtrlConfig(unsigned int GpioNum, UINT8 ActiveLevel);

void sAPI_AudioSetVolume(AUD_Volume volume);
AUD_Volume sAPI_AudioGetVolume(void);
void sAPP_AudioTaskInit(void);
void AudioContinuouslyDeinit(void);
BOOL sAPI_AudioPlayMp3Cont(char *file, BOOL startplay,BOOL frame);
BOOL sAPI_AudioPlayWavCont(char *file, BOOL startplay);

BOOL sAPI_EchoSuppression_PARA(SC_Dspconfiguration dspconfig);
BOOL sAPI_EchoParaModeSet(unsigned char SaveMode, unsigned char Level);
void sAPI_Get_EchoSuppression_PARA(SC_Dspconfiguration *dspconfig,int mode);
#endif



