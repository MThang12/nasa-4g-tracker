#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "simcom_audio.h"
#include "demo_audio.h"
#include "simcom_file.h"
#include "simcom_os.h"
#include "simcom_demo_init.h"
#include "pub_string.h"
#include "pub_cache.h"

#include "sal_os.h"
#include "sal_log.h"

#define AUDIO_LOG(...)          sal_log(__VA_ARGS__)
#define AUDIO_LOG_ERROR(...)    sal_log_error("[DEMO-AUDIO] " __VA_ARGS__)
#define AUDIO_LOG_INFO(...)     sal_log_info("[DEMO-AUDIO] " __VA_ARGS__)
#define AUDIO_LOG_TRACE(...)    sal_log_trace("[DEMO-AUDIO] " __VA_ARGS__)
#define AUDIO_FILENAME_MAX_LEN  60

static ret_msg_t audio_samplerate_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_play_file_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_stop_file_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_set_amr_encoder_rate_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_play_wav_with_high_sampling_rate_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_set_micgain_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_get_micgain_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_set_volume_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_get_volume_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_record_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_play_mp3_cont_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_play_amr_cont_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_play_wav_cont_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_pcm_record_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_stop_pcm_record_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_get_amr_record_frame_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_stop_amr_frame_cmd_proc(op_t op, arg_t *argv, int argn);
static int audio_play_pcm_need_size(void);
static ret_msg_t audio_pcm_init(void);
static ret_msg_t audio_pcm_uninit(void);
static ret_msg_t audio_play_pcm_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms));
static ret_msg_t audio_play_pcm_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_stop_pcm_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_play_mp3_buffer_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_stop_mp3_buffer_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_play_amr_buffer_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t audio_stop_amr_buffer_cmd_proc(op_t op, arg_t *argv, int argn);

const value_t audio_param[2] = 
{
    {.val = 0},
    {.val = 1}
};

const value_list_t audioparam_list = 
{
    .isrange = false,
    .list_head = audio_param,
    .count = sizeof(audio_param) / sizeof(value_t),
};

arg_t audio_args[1] = {
    {
        .type = TYPE_INT,
        .arg_name = ARGAUDIO_SAMPLERATE,
        .value_range = &audioparam_list,
        .msg = "Samplerate 0(8K), 1(16K)"
    }
};

const menu_list_t func_audio_samplerate =
{
    .menu_name = "audio samplerate",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .argv = audio_args,
        .argn = sizeof(audio_args) / sizeof(arg_t),
        .methods.cmd_handler = audio_samplerate_cmd_proc,
    }
};

arg_t audio_play_file_args[3] = 
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARGAUDIO_FILENAME,
        .msg = "audio file name"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGAUDIO_DIRECTPLAY,
        .value_range = &audioparam_list,
        .msg = "direct play or not(0.false, 1.yes)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGAUDIO_ISSINGLE,
        .value_range = &audioparam_list,
        .msg = "Single play or not(0.false, 1.yes)"
    }
};

const menu_list_t func_audio_play_file =
{
    .menu_name = "audio play file",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .argv = audio_play_file_args,
        .argn = sizeof(audio_play_file_args) / sizeof(arg_t),
        .methods.cmd_handler = audio_play_file_cmd_proc,
    }
};

const menu_list_t func_audio_stop_file =
{
    .menu_name = "audio stop to play file",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = audio_stop_file_cmd_proc,
    }
};

arg_t audio_play_pcm_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGAUDIO_PCM_SIZE,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGAUDIO_DIRECTPLAY,
        .value_range = &audioparam_list,
        .msg = "direct play or not(0.false, 1.yes)"
    }
};

const menu_list_t func_audio_play_pcm =
{
    .menu_name = "audio play pcm",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .argv = audio_play_pcm_args,
        .argn = sizeof(audio_play_pcm_args) / sizeof(arg_t),
        .RAWSizeLink = ARGAUDIO_PCM_SIZE,
        .methods.__init = audio_pcm_init,
        .methods.__uninit = audio_pcm_uninit,
        .methods.cmd_handler = audio_play_pcm_cmd_proc,
        .methods.raw_handler = audio_play_pcm_raw_proc,
        .methods.needRAWData = audio_play_pcm_need_size,
    },
};

const menu_list_t func_audio_stop_pcm =
{
    .menu_name = "audio stop to play pcm",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .methods.cmd_handler = audio_stop_pcm_cmd_proc,
    }
};

const value_list_t audio_rate_level_list = 
{
    .isrange = true,
    .min = 0,
    .max = 7
};

arg_t audio_set_amr_encoder_rate_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGAUDIO_RATELEVEL,
        .value_range = &audio_rate_level_list,
        .msg = "amr encoder rate level: (0 ~ 7)"
    }
};

const menu_list_t func_audio_set_amr_encoder_rate =
{
    .menu_name = "audio set amr encoder rate",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .argv = audio_set_amr_encoder_rate_args,
        .argn = sizeof(audio_set_amr_encoder_rate_args) / sizeof(arg_t),
        .methods.cmd_handler = audio_set_amr_encoder_rate_cmd_proc,
    }
};

arg_t audio_play_wav_with_high_sampling_rate_args[2] =
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARGAUDIO_FILENAME,
        .msg = "audio file name"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGAUDIO_DIRECTPLAY,
        .value_range = &audioparam_list,
        .msg = "direct play or not(0.false, 1.yes)"
    }
};

const menu_list_t func_audio_play_wav_with_high_sampling_rate =
{
    .menu_name = "audio play wav with high_sampling_rate",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .argv = audio_play_wav_with_high_sampling_rate_args,
        .argn = sizeof(audio_play_wav_with_high_sampling_rate_args) / sizeof(arg_t),
        .methods.cmd_handler = audio_play_wav_with_high_sampling_rate_cmd_proc,
    }
};

const value_list_t audio_micgain_list = 
{
    .isrange = true,
    .min = 0,
    .max = 7
};

arg_t audio_set_micgain_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGAUDIO_MICGAIN,
        .value_range = &audio_micgain_list,
        .msg = "audio micgain(0 ~ 7)"
    }
};

const menu_list_t func_audio_set_micgain =
{
    .menu_name = "audio set micgain",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .argv = audio_set_micgain_args,
        .argn = sizeof(audio_set_micgain_args) / sizeof(arg_t),
        .methods.cmd_handler = audio_set_micgain_cmd_proc,
    }
};

const menu_list_t func_audio_get_micgain =
{
    .menu_name = "audio get micgain",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .methods.cmd_handler = audio_get_micgain_cmd_proc,
    }
};

const value_list_t audio_volume_list = 
{
    .isrange = true,
    .min = 0,
    .max = 11
};

arg_t audio_set_volume_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGAUDIO_VOLUME,
        .value_range = &audio_volume_list,
        .msg = "audio volume(0 ~ 11)"
    }
};

const menu_list_t func_audio_set_volume =
{
    .menu_name = "audio set volume",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .argv = audio_set_volume_args,
        .argn = sizeof(audio_set_volume_args) / sizeof(arg_t),
        .methods.cmd_handler = audio_set_volume_cmd_proc,
    }
};

const menu_list_t func_audio_get_volume =
{
    .menu_name = "audio get volume",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .methods.cmd_handler = audio_get_volume_cmd_proc,
    }
};

const value_t audio_path_param[] = 
{
    {.val = 0},
};

const value_list_t audio_record_path_list =
{
    .isrange = false,
    .list_head = audio_path_param,
    .count = sizeof(audio_path_param) / sizeof(value_t),
};

arg_t audio_record_args[3] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGAUDIO_ENABLE,
        .value_range = &audioparam_list,
        .msg = "0(stop reord) 1(start record)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGAUDIO_RECORDPATH,
        .value_range = &audio_record_path_list,
        .msg = "0: local"
    },
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARGAUDIO_FILENAME,
        .msg = "audio record file name"
    }
};

const menu_list_t func_audio_record =
{
    .menu_name = "audio record",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .argv = audio_record_args,
        .argn = sizeof(audio_record_args) / sizeof(arg_t),
        .methods.cmd_handler = audio_record_cmd_proc,
    }
};

const menu_list_t func_audio_stop_record =
{
    .menu_name = "audio stop record",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .argv = audio_record_args,
        .argn = sizeof(audio_record_args) / sizeof(arg_t),
        .methods.cmd_handler = audio_record_cmd_proc,
    }
};

const value_list_t audio_mp3_frame_list =
{
    .isrange = true,
    .max = 2,
    .min = 0
};

arg_t audio_play_mp3_cont_args[3] =
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARGAUDIO_FILENAME,
        .msg = "intput mp3 file"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGAUDIO_STARTPLAY,
        .value_range = &audioparam_list,
        .msg = "0: fill the files ,1: start play"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGAUDIO_MP3_FRAME,
        .value_range = &audio_mp3_frame_list,
        .msg = "The number of frames skipped before playback(0 ~ 2)"
    }
};

const menu_list_t func_audio_play_mp3_cont =
{
    .menu_name = "audio play mp3 continuously",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .argv = audio_play_mp3_cont_args,
        .argn = sizeof(audio_play_mp3_cont_args) / sizeof(arg_t),
        .methods.cmd_handler = audio_play_mp3_cont_cmd_proc,
    }
};

arg_t audio_play_cont_args[2] =
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARGAUDIO_FILENAME,
        .msg = "intput audio file"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGAUDIO_STARTPLAY,
        .value_range = &audioparam_list,
        .msg = "0: fill the files ,1: start play"
    }
};

const menu_list_t func_audio_play_amr_cont =
{
    .menu_name = "audio play amr continuously",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .argv = audio_play_cont_args,
        .argn = sizeof(audio_play_cont_args) / sizeof(arg_t),
        .methods.cmd_handler = audio_play_amr_cont_cmd_proc,
    }
};

const menu_list_t func_audio_play_wav_cont =
{
    .menu_name = "audio play wav continuously",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .argv = audio_play_cont_args,
        .argn = sizeof(audio_play_cont_args) / sizeof(arg_t),
        .methods.cmd_handler = audio_play_wav_cont_cmd_proc,
    }
};

const menu_list_t func_audio_pcm_record =
{
    .menu_name = "audio pcm record",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .methods.cmd_handler = audio_pcm_record_cmd_proc,
    }
};

const menu_list_t func_audio_stop_pcm_record =
{
    .menu_name = "audio stop pcm record",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .methods.cmd_handler = audio_stop_pcm_record_cmd_proc,
    }
};

const menu_list_t func_audio_get_amr_record_frame =
{
    .menu_name = "audio get amr record frame",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .methods.cmd_handler = audio_get_amr_record_frame_cmd_proc,
    }
};

const menu_list_t func_audio_stop_amr_frame =
{
    .menu_name = "audio stop amr frame",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .methods.cmd_handler = audio_stop_amr_frame_cmd_proc,
    }
};

const menu_list_t func_audio_play_mp3_buffer =
{
    .menu_name = "audio play mp3 buffer",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .argv = audio_play_pcm_args,
        .argn = sizeof(audio_play_pcm_args) / sizeof(arg_t),
        .RAWSizeLink = ARGAUDIO_PCM_SIZE,
        .methods.__init = audio_pcm_init,
        .methods.__uninit = audio_pcm_uninit,
        .methods.cmd_handler = audio_play_mp3_buffer_cmd_proc,
        .methods.raw_handler = audio_play_pcm_raw_proc,
        .methods.needRAWData = audio_play_pcm_need_size,
    },
};

const menu_list_t func_audio_stop_mp3_buffer =
{
    .menu_name = "audio stop to play mp3 buffer",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = audio_stop_mp3_buffer_cmd_proc,
    }
};

const menu_list_t func_audio_play_amr_buffer =
{
    .menu_name = "audio play amr buffer",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .argv = audio_play_pcm_args,
        .argn = sizeof(audio_play_pcm_args) / sizeof(arg_t),
        .RAWSizeLink = ARGAUDIO_PCM_SIZE,
        .methods.__init = audio_pcm_init,
        .methods.__uninit = audio_pcm_uninit,
        .methods.cmd_handler = audio_play_amr_buffer_cmd_proc,
        .methods.raw_handler = audio_play_pcm_raw_proc,
        .methods.needRAWData = audio_play_pcm_need_size,
    },
};

const menu_list_t func_audio_stop_amr_buffer =
{
    .menu_name = "audio stop to play amr buffer",
    .menu_type = TYPE_FUNC,
    .parent = &audio_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = audio_stop_amr_buffer_cmd_proc,
    }
};

const menu_list_t *audio_menu_list[] = 
{
    &func_audio_samplerate,
    &func_audio_play_file,
    &func_audio_stop_file,
    &func_audio_play_pcm,
    &func_audio_stop_pcm,
    &func_audio_play_mp3_buffer,
    &func_audio_stop_mp3_buffer,
    &func_audio_play_amr_buffer,
    &func_audio_stop_amr_buffer,
    &func_audio_set_amr_encoder_rate,
    &func_audio_get_amr_record_frame,
    &func_audio_stop_amr_frame,
    &func_audio_play_mp3_cont,
    &func_audio_play_amr_cont,
    &func_audio_play_wav_cont,
    &func_audio_play_wav_with_high_sampling_rate,
    &func_audio_set_micgain,
    &func_audio_get_micgain,
    &func_audio_set_volume,
    &func_audio_get_volume,
    &func_audio_record,
    &func_audio_stop_record,
    &func_audio_pcm_record,
    &func_audio_stop_pcm_record,
    NULL
};

const menu_list_t audio_menu =
{
    .menu_name = "AUDIO",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = audio_menu_list,
    .parent = &root_menu,
};

typedef struct data_info{
    char *data_buf;
    int data_size;
    int direct;
    menu_list_t *func;
}data_info_t;

data_info_t data_info = { 0 };
data_info_t data_standby_info = { 0 };

char *g_data_buf = NULL;
SCFILE * g_stream_fd = NULL;
int g_wait_status = 0;
int g_display_stop = 1;
int g_data_size = 0;
sTaskRef g_pcm_task_ref;
sTaskRef g_pcm_status_task_ref;
sal_sema_ref pcm_sync_ref;

ret_msg_t audio_samplerate_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_samplerate_cmd_proc enter!");

    ret_msg_t ret = { 0 };
    int audioSamplerate  = 0;

    pl_demo_get_value(&audioSamplerate, argv, argn, ARGAUDIO_SAMPLERATE);
    if (audioSamplerate < 0 || audioSamplerate > 1)
    {
        AUDIO_LOG_ERROR("Para invalid! audioSamplerate = %d\r\n", audioSamplerate);
        ret.errcode =ERRCODE_DEFAULT;
        ret.msg = "Para invalid!";
        return ret;
    }

    if (sAPI_AudioPlaySampleRate(audioSamplerate))
    {
        ret.errcode = ERRCODE_OK;
        ret.msg = "Success to set audio Samplerate!";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "Fail to set audio Samplerate!";
    }

    return ret;
}

ret_msg_t audio_play_file_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_play_file_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    const char *filename = NULL;
    char file[AUDIO_FILENAME_MAX_LEN] = { 0 };
    int direct = 0;
    int issingle = 1;

    pl_demo_get_data(&filename, argv, argn, ARGAUDIO_FILENAME);
    if (!filename)
    {
        AUDIO_LOG_ERROR("filename Param invalid!");
        ret.errcode = ERRCODE_DEFAULT;
        return ret;
    }

    memcpy(file, filename, strlen(filename));

    pl_demo_get_value(&direct, argv, argn, ARGAUDIO_DIRECTPLAY);
    pl_demo_get_value(&issingle, argv, argn, ARGAUDIO_ISSINGLE);
    if ((direct < 0 || direct > 1) || (issingle < 0 || issingle > 1))
    {
        AUDIO_LOG_ERROR("param invalid! direct = %d, issingle = %d", direct, issingle);
        ret.errcode = ERRCODE_DEFAULT;
        return ret;
    }

    if (sAPI_AudioPlay(file, direct, issingle))
    {
        ret.errcode = ERRCODE_OK;
        ret.msg = "audio play start";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "audio play failed";
    }

    return ret;
}

ret_msg_t audio_stop_file_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_stop_file_cmd_proc enter!");
    ret_msg_t ret = { 0 };

    if (sAPI_AudioStop())
    {
        ret.errcode = ERRCODE_OK;
        ret.msg = "audio stop successfully";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "audio stop failed";
    }

    return ret;
}

ret_msg_t audio_set_amr_encoder_rate_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_set_amr_encoder_rate_cmd_proc enter!");

    ret_msg_t ret = { 0 };
    int rateLevel = 0;

    pl_demo_get_value(&rateLevel, argv, argn, ARGAUDIO_RATELEVEL);
    AUDIO_LOG_INFO("rate: %d", rateLevel);
    if (rateLevel < 0 || rateLevel > 7)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "audio set amr encoder rate param fail!";
        return ret;
    }

    sAPI_AudioSetAmrRateLevel(rateLevel);

    return ret;
}

ret_msg_t audio_play_wav_with_high_sampling_rate_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_play_wav_with_high_sampling_rate_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    const char *filename = NULL;
    char file[AUDIO_FILENAME_MAX_LEN] = { 0 };
    int direct = 0;

    pl_demo_get_data(&filename, argv, argn, ARGAUDIO_FILENAME);
    if (!filename)
    {
        AUDIO_LOG_ERROR("filename Param invalid!");
        ret.errcode = ERRCODE_DEFAULT;
        return ret;
    }

    pl_demo_get_value(&direct, argv, argn, ARGAUDIO_DIRECTPLAY);
    if (direct < 0 || direct > 1)
    {
        AUDIO_LOG_ERROR("param invalid! direct = %d", direct);
        ret.errcode = ERRCODE_DEFAULT;
        return ret;
    }

    memcpy(file, filename, strlen(filename));

    if (sAPI_AudioWavFilePlay(file, direct))
    {
        ret.errcode = ERRCODE_OK;
        ret.msg = "audio play wav_with_high_sampling_rate start";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "audio play wav_with_high_sampling_rate failed";
    }

    return ret;
}

ret_msg_t audio_set_micgain_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_set_micgain_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    int micgain;

    pl_demo_get_value(&micgain, argv, argn, ARGAUDIO_MICGAIN);
    AUDIO_LOG("micgain: %d", micgain);
    if (micgain < 0 || micgain >7)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    sAPI_AudioSetMicGain(micgain);

    return ret;
}

ret_msg_t audio_get_micgain_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_get_micgain_cmd_proc enter");
    ret_msg_t ret = { 0 };
    arg_t out_arg[1] = { 0 };
    int micgain;

    micgain = sAPI_AudioGetMicGain();

    out_arg[0].type = TYPE_INT;
    out_arg[0].arg_name = ARGAUDIO_MICGAIN;
    out_arg[0].value = pl_demo_make_value(micgain, NULL, 0);

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

    pl_demo_release_value(out_arg[0].value);

    return ret;
}

ret_msg_t audio_set_volume_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_set_volume_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    int volume;

    pl_demo_get_value(&volume, argv, argn, ARGAUDIO_VOLUME);
    AUDIO_LOG("volume: %d", volume);
    if (volume < 0 || volume >11)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    sAPI_AudioSetVolume(volume);

    return ret;
}

ret_msg_t audio_get_volume_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_get_volume_cmd_proc enter");
    ret_msg_t ret = { 0 };
    arg_t out_arg[1] = { 0 };
    int volume;

    volume = sAPI_AudioGetVolume();

    out_arg[0].type = TYPE_INT;
    out_arg[0].arg_name = ARGAUDIO_VOLUME;
    out_arg[0].value = pl_demo_make_value(volume, NULL, 0);

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);

    pl_demo_release_value(out_arg[0].value);

    return ret;
}

ret_msg_t audio_record_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_record_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    int enable, path;
    const char *filename = NULL;
    char file[AUDIO_FILENAME_MAX_LEN] = { 0 };

    pl_demo_get_value(&enable, argv, argn, ARGAUDIO_ENABLE);
    pl_demo_get_value(&path, argv, argn, ARGAUDIO_RECORDPATH);
    if (enable < 0 || enable > 1)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    pl_demo_get_data(&filename, argv, argn, ARGAUDIO_FILENAME);
    if (!filename)
    {
        AUDIO_LOG_ERROR("filename Param invalid!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "filename Param invalid";
        return ret;
    }

    memcpy(file, filename, strlen(filename));

    if (sAPI_AudioRecord(enable, path, file))
    {
        ret.errcode = ERRCODE_OK;
        if (!enable)
            ret.msg = "audio record stop";
        else
            ret.msg = "audio record start";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "audio record failed";
    }

    return ret;
}

ret_msg_t audio_play_mp3_cont_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_play_mp3_cont_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    const char *filename = NULL;
    char file[AUDIO_FILENAME_MAX_LEN] = { 0 };
    int startplay, mp3frame = 0;

        pl_demo_get_data(&filename, argv, argn, ARGAUDIO_FILENAME);
        if (!filename)
        {
            AUDIO_LOG_ERROR("filename Param invalid!");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "filename Param invalid";
            return ret;
        }

        pl_demo_get_value(&startplay, argv, argn, ARGAUDIO_STARTPLAY);
        pl_demo_get_value(&mp3frame, argv, argn, ARGAUDIO_MP3_FRAME);
        if (startplay < 0 || startplay > 1 || mp3frame < 0 || mp3frame > 2)
        {
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "param invalid";
            return ret;
        }
        memcpy(file, filename, strlen(filename));

        if (sAPI_AudioPlayMp3Cont(file, startplay, mp3frame))
        {
            ret.errcode = ERRCODE_OK;
            if (!startplay)
                ret.msg = "sucess to load mp3 file";
            else
                ret.msg = "sucess to play mp3 continuously";
        }
        else
        {
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "failed to play mp3 continuously";
        }

    AUDIO_LOG_INFO("ret.errcode:[%d] audio start continuous play!", ret.errcode);

    return ret;
}

ret_msg_t audio_play_amr_cont_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_play_amr_cont_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    const char *filename = NULL;
    char file[AUDIO_FILENAME_MAX_LEN] = { 0 };
    int startplay = 0;

        pl_demo_get_data(&filename, argv, argn, ARGAUDIO_FILENAME);
        if (!filename)
        {
            AUDIO_LOG_ERROR("filename Param invalid!");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "filename Param invalid";
            return ret;
        }

        pl_demo_get_value(&startplay, argv, argn, ARGAUDIO_STARTPLAY);
        if (startplay < 0 || startplay > 1)
        {
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "param invalid";
            return ret;
        }
        memcpy(file, filename, strlen(filename));

        if (sAPI_AudioPlayAmrCont(file, startplay))
        {
            ret.errcode = ERRCODE_OK;
            if (!startplay)
                ret.msg = "sucess to load amr file";
            else
                ret.msg = "sucess to play amr continuously";
        }
        else
        {
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "failed to play amr continuously";
        }

    AUDIO_LOG_INFO("ret.errcode:[%d] audio start continuous play!", ret.errcode);

    return ret;
}

ret_msg_t audio_play_wav_cont_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_play_wav_cont_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    const char *filename = NULL;
    char file[AUDIO_FILENAME_MAX_LEN] = { 0 };
    int startplay = 0;


        pl_demo_get_data(&filename, argv, argn, ARGAUDIO_FILENAME);
        if (!filename)
        {
            AUDIO_LOG_ERROR("filename Param invalid!");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "filename Param invalid";
            return ret;
        }

        pl_demo_get_value(&startplay, argv, argn, ARGAUDIO_STARTPLAY);
        if (startplay < 0 || startplay > 1)
        {
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "param invalid";
            return ret;
        }

        memcpy(file, filename, strlen(filename));

        if (sAPI_AudioPlayWavCont(file, startplay))
        {
            ret.errcode = ERRCODE_OK;
            if (!startplay)
                ret.msg = "sucess to load wav file";
            else
                ret.msg = "sucess to play wav continuously";
        }
        else
        {
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "failed to play wav continuously";
        }

    AUDIO_LOG_INFO("ret.errcode:[%d] startplay:[%d] audio start continuous play!", ret.errcode, startplay);

    return ret;
}

void GetPCMRec(const UINT8 *buf, UINT32 size)
{
    AUDIO_LOG("GetPCMRec enter!");
    int write_size = 0;

    if (NULL == buf || size == 0)
    {
        AUDIO_LOG_ERROR("Data null!");
        return;
    }

    write_size = sAPI_fwrite(buf, 1, size, g_stream_fd);
    if (write_size != size)
    {
        AUDIO_LOG_ERROR("write buf failed! write_size = %d, need_size = %d", write_size, size);
    }
}

ret_msg_t audio_pcm_record_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_pcm_record_cmd_proc enter!");
    ret_msg_t ret = { 0 };

    g_stream_fd = sAPI_fopen("c:/pcm_test.wav", "ab+");
    if (sAPI_PCMStreamRec((void *)GetPCMRec))
    {
        ret.errcode = ERRCODE_OK;
        ret.msg = "audio start pcm record";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "audio pcm record failed";
    }

    return ret;
}

ret_msg_t audio_stop_pcm_record_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_stop_pcm_record_cmd_proc enter!");
    ret_msg_t ret = { 0 };

    if (sAPI_PCMStopStreamRec())
    {
        sAPI_fclose(g_stream_fd);
        g_stream_fd = NULL;
        ret.errcode = ERRCODE_OK;
        ret.msg = "audio success to stop pcm record";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "audio fail to stop pcm record";
    }

    return ret;
}

ret_msg_t audio_get_amr_record_frame_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_get_amr_record_frame_cmd_proc enter!");
    ret_msg_t ret = { 0 };

    g_stream_fd = sAPI_fopen("c:/amr_stream_test.amr", "ab+");
    if (sAPI_AmrStreamRecord((void *)GetPCMRec))
    {
        ret.errcode = ERRCODE_OK;
        ret.msg = "audio start amr record";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "audio amr record failed";
    }

    return ret;
}

ret_msg_t audio_stop_amr_frame_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_stop_amr_frame_cmd_proc enter!");
    ret_msg_t ret = { 0 };

    if (sAPI_AmrStopStreamRecord())
    {
        sAPI_fclose(g_stream_fd);
        g_stream_fd = NULL;
        ret.errcode = ERRCODE_OK;
        ret.msg = "audio success to stop amr record";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "audio fail to stop amr record";
    }

    return ret;
}

static void clean_data_info(void *args)
{
    AUDIO_LOG("clean data info!");
    data_info_t *clean_data_info = (data_info_t *)args;

    sal_free(clean_data_info->data_buf);
    clean_data_info->data_size = 0;
    clean_data_info->direct = 0;
    clean_data_info->func = NULL;
    g_data_size = 0;
    g_display_stop = 1;
}

static ret_msg_t audio_pcm_uninit(void)
{
    ret_msg_t ret = { 0 };
    if (pcm_sync_ref)
    {
        ret.errcode = sal_semaphore_delete(&pcm_sync_ref);
    }

    return ret;
}

static ret_msg_t audio_pcm_init(void)
{
    ret_msg_t ret = { 0 };
    audio_pcm_uninit();

    if (0 == sal_semaphore_create(&pcm_sync_ref, 0))
    {
        ret.errcode = ERRCODE_OK;
        ret.msg = "audio success to create semaphore";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "audio fail to create semaphore";
    }

    return ret;
}

static void check_audio_status_task(void *args)
{
    AUDIO_LOG_INFO("check_audio_status_task enter!");
    arg_t out_arg[1] = { 0 };
    ret_msg_t ret = { 0 };
    data_info_t *audio_info = (data_info_t *)args;
    g_wait_status = 1;

    while(sAPI_AudioStatus())
    {
        AUDIO_LOG_INFO("During the play!");
        sal_task_sleep(20);
    }

    AUDIO_LOG_INFO("g_display_stop = [%d], status = [%d]", g_display_stop, sAPI_AudioStatus());
    if (g_display_stop)
    {
        out_arg[0].arg_name = ARGAUDIO_REPORT_STOP;
        out_arg[0].type = TYPE_RAW;
        out_arg[0].value = pl_demo_make_value(0, "+AUDIOSTATE: audio play stop",28);

        ret.errcode = pl_demo_post_urc(audio_info->func, TYPE_URC_DATA, out_arg, 1, NULL, 0);

        AUDIO_LOG_INFO("errcode = [%d]", ret.errcode);
        pl_demo_release_value(out_arg[0].value);
    }

    clean_data_info(audio_info);

    g_wait_status = 0;
}

static ret_msg_t audio_play_pcm_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms))
{
    ret_msg_t ret = { 0 };
    int read_size = 0;
    int remain_size = size;
    int copy_size = 0;
    char buf[SAL_5K] = { 0 };
    data_info_t *pcm_raw_info = NULL;

    data_info.data_buf == NULL ? (pcm_raw_info = &data_info) : (pcm_raw_info = &data_standby_info);

    if (pcm_raw_info->data_size != size)
    {
        AUDIO_LOG_ERROR("raw data size invalid");
        AUDIO_LOG_ERROR("size: %d, need: %d", size, g_data_size);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "raw data size invalid";
        goto EXIT;
    }

    pcm_raw_info->data_buf = sal_malloc(pcm_raw_info->data_size + 1);
    if (!pcm_raw_info->data_buf)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "malloc fail";
        goto EXIT;
    }

    while (remain_size > 0)
    {
        read_size = remain_size <= SAL_5K ? remain_size : SAL_5K;
        read_size = raw_read(buf, read_size, 10000);
        if (read_size < 0)
        {
            AUDIO_LOG_ERROR("raw data read fail");
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "raw data read fail";
            goto EXIT;
        }
        else if (0 == read_size)
        {
            AUDIO_LOG_ERROR("raw data read timeout");
            ret.errcode = ERRCODE_RAW_RECV_TIMEOUT;
            ret.msg = "raw data read timeout";
            goto EXIT;
        }

        memcpy(pcm_raw_info->data_buf + copy_size, buf, read_size);

        copy_size += read_size;
        remain_size -= read_size;
        AUDIO_LOG_INFO("copy_size:[%d], remain_size:[%d], read_size:[%d]", copy_size, remain_size, read_size);
    }

    sal_semaphore_release(pcm_sync_ref);
    return ret;

EXIT:
    sal_free(pcm_raw_info->data_buf);
    sal_semaphore_release(pcm_sync_ref);
    return ret;
}

static void audio_play_pcm_task(void *args)
{
    AUDIO_LOG("audio_play_pcm_task enter!");
    data_info_t *pcm_data_info = (data_info_t *)args;
    bool result = true;

    sal_semaphore_acquire(pcm_sync_ref, SAL_OS_TIMEOUT_SUSPEND);

    if (!pcm_data_info->data_buf)
    {
        AUDIO_LOG_ERROR("g_data_buf error! data_size = %d", g_data_size);
        clean_data_info(pcm_data_info);

        return;
    }

    if (pcm_data_info->direct && sAPI_AudioStatus())
    {
        g_display_stop = 0;
    }

    result = sAPI_AudioPcmPlay(pcm_data_info->data_buf, pcm_data_info->data_size, pcm_data_info->direct);
    if (!result)
    {
        AUDIO_LOG_ERROR("pcm play failed!");
        clean_data_info(pcm_data_info);
        return;
    }

    while (g_wait_status)
    {
        AUDIO_LOG_INFO("wait to create audio status!");
        sal_task_sleep(10);
    }
    sal_task_create(&g_pcm_status_task_ref, NULL, SAL_2K, sal_task_priority_low_1, "check status", check_audio_status_task, pcm_data_info);
    sal_task_detach(&g_pcm_status_task_ref);

}

static int audio_play_pcm_need_size(void)
{
    return g_data_size;
}

ret_msg_t audio_play_pcm_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_play_pcm_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    int pcm_size = 0;
    int direct = 0;
    data_info_t *play_data_info = NULL;

    pl_demo_get_value(&pcm_size, argv, argn, ARGAUDIO_PCM_SIZE);
    pl_demo_get_value(&direct, argv, argn, ARGAUDIO_DIRECTPLAY);
    if (pcm_size < 0 || direct < 0 || direct > 1)
    {
        AUDIO_LOG_ERROR("pcm size param invalid!");
        AUDIO_LOG_ERROR("g_direct:[%d]", direct);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "pcm size param invalid";
        return ret;
    }

    data_info.data_buf == NULL ? (play_data_info = &data_info) : (play_data_info = &data_standby_info);

    g_data_size = play_data_info->data_size = pcm_size;
    play_data_info->direct = direct;
    play_data_info->func = (menu_list_t *)&func_audio_play_pcm;

    ret.errcode = sal_task_create(&g_pcm_task_ref, NULL, SAL_2K, sal_task_priority_low_1, "play pcm", audio_play_pcm_task, play_data_info);
    if (ret.errcode != SC_SUCCESS)
    {
        ret.msg = "task creat fail";
    }
    sal_task_detach(&g_pcm_task_ref);

    return ret;
}

ret_msg_t audio_stop_pcm_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_stop_pcm_cmd_proc enter!");
    ret_msg_t ret = { 0 };

    if (sAPI_AudioPcmStop())
    {
        ret.errcode = ERRCODE_OK;
        ret.msg = "audio success to stop pcm";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "audio fail to stop pcm";
    }

    return ret;
}

static void audio_play_mp3_buffer_task(void *args)
{
    AUDIO_LOG("audio_play_mp3_buffer_task enter!");
    data_info_t *pcm_data_info = (data_info_t *)args;
    bool result = true;

    sal_semaphore_acquire(pcm_sync_ref, SAL_OS_TIMEOUT_SUSPEND);

    if (!pcm_data_info->data_buf)
    {
        AUDIO_LOG_ERROR("mp3:g_data_buf error! g_data_size = %d", g_data_size);
        return;
    }

    if (pcm_data_info->direct && sAPI_AudioStatus())
    {
        g_display_stop = 0;
    }

    result = sAPI_AudioMp3StreamPlay(pcm_data_info->data_buf, pcm_data_info->data_size, pcm_data_info->direct);
    if (!result)
    {
        AUDIO_LOG_ERROR("pcm play failed!");
        clean_data_info(pcm_data_info);
        return;
    }

    while (g_wait_status)
    {
        AUDIO_LOG_INFO("wait to create audio status!");
        sal_task_sleep(10);
    }

    sal_task_create(&g_pcm_status_task_ref, NULL, SAL_2K, sal_task_priority_low_1, "check status", check_audio_status_task, pcm_data_info);
    sal_task_detach(&g_pcm_status_task_ref);
}

ret_msg_t audio_play_mp3_buffer_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_play_mp3_buffer_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    int pcm_size = 0;
    int direct = 0;
    data_info_t *play_data_info = NULL;

    pl_demo_get_value(&pcm_size, argv, argn, ARGAUDIO_PCM_SIZE);
    pl_demo_get_value(&direct, argv, argn, ARGAUDIO_DIRECTPLAY);
    if (pcm_size < 0 || direct < 0 || direct > 1)
    {
        AUDIO_LOG_ERROR("pcm size param invalid!");
        AUDIO_LOG_ERROR("g_direct:[%d]", direct);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "pcm size param invalid";
        return ret;
    }

    data_info.data_buf == NULL ? (play_data_info = &data_info) : (play_data_info = &data_standby_info);

    g_data_size = play_data_info->data_size = pcm_size;
    play_data_info->direct = direct;
    play_data_info->func = (menu_list_t *)&func_audio_play_mp3_buffer;

    ret.errcode = sal_task_create(&g_pcm_task_ref, NULL, SAL_4K, sal_task_priority_low_1, "play mp3 buffer", audio_play_mp3_buffer_task, play_data_info);
    if (ret.errcode != SC_SUCCESS)
    {
        ret.msg = "task creat fail";
    }
    sal_task_detach(&g_pcm_task_ref);

    return ret;
}

ret_msg_t audio_stop_mp3_buffer_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_stop_mp3_buffer_cmd_proc enter!");
    ret_msg_t ret = { 0 };

    if (sAPI_AudioMp3StreamStop())
    {
        ret.errcode = ERRCODE_OK;
        ret.msg = "audio success to stop mp3 buffer";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "audio fail to stop mp3 buffer";
    }

    return ret;
}

static void audio_play_amr_buffer_task(void *args)
{
    AUDIO_LOG("audio_play_amr_buffer_task enter!");
    data_info_t *pcm_data_info = (data_info_t *)args;
    bool result = true;

    sal_semaphore_acquire(pcm_sync_ref, SAL_OS_TIMEOUT_SUSPEND);

    if (!pcm_data_info->data_buf)
    {
        AUDIO_LOG_ERROR("amr:g_data_buf error! g_data_size = %d", g_data_size);
        return;
    }

    if (pcm_data_info->direct && sAPI_AudioStatus())
    {
        g_display_stop = 0;
    }

    result = sAPI_AudioAmrStreamPlay(pcm_data_info->data_buf, pcm_data_info->data_size, pcm_data_info->direct);
    if (!result)
    {
        AUDIO_LOG_ERROR("pcm play failed!");
        clean_data_info(pcm_data_info);
        return;
    }

    while (g_wait_status)
    {
        AUDIO_LOG_INFO("wait to create audio status!");
        sal_task_sleep(10);
    }

    sal_task_create(&g_pcm_status_task_ref, NULL, SAL_2K, sal_task_priority_low_1, "check status", check_audio_status_task, pcm_data_info);
    sal_task_detach(&g_pcm_status_task_ref);
}

ret_msg_t audio_play_amr_buffer_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_play_amr_buffer_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    int pcm_size = 0;
    int direct = 0;
    data_info_t *play_data_info = NULL;

    pl_demo_get_value(&pcm_size, argv, argn, ARGAUDIO_PCM_SIZE);
    pl_demo_get_value(&direct, argv, argn, ARGAUDIO_DIRECTPLAY);
    if (pcm_size < 0 || direct < 0 || direct > 1)
    {
        AUDIO_LOG_ERROR("pcm size param invalid!");
        AUDIO_LOG_ERROR("g_direct:[%d]", direct);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "pcm size param invalid";
        return ret;
    }

    data_info.data_buf == NULL ? (play_data_info = &data_info) : (play_data_info = &data_standby_info);

    g_data_size = play_data_info->data_size = pcm_size;
    play_data_info->direct = direct;
    play_data_info->func = (menu_list_t *)&func_audio_play_amr_buffer;

    ret.errcode = sal_task_create(&g_pcm_task_ref, NULL, SAL_2K, sal_task_priority_low_1, "play amr buffer", audio_play_amr_buffer_task, play_data_info);
    if (ret.errcode != SC_SUCCESS)
    {
        ret.msg = "task creat fail";
    }
    sal_task_detach(&g_pcm_task_ref);

    return ret;
}

ret_msg_t audio_stop_amr_buffer_cmd_proc(op_t op, arg_t *argv, int argn)
{
    AUDIO_LOG("audio_stop_amr_buffer_cmd_proc enter!");
    ret_msg_t ret = { 0 };

    if (sAPI_AudioAmrStreamStop())
    {
        ret.errcode = ERRCODE_OK;
        ret.msg = "audio success to stop amr buffer";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "audio fail to stop amr buffer";
    }

    return ret;
}