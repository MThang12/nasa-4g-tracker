#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "demo_tts.h"
#include "simcom_os.h"
#include "simcom_tts_api.h"
#include "simcom_demo_init.h"

#include "sal_os.h"
#include "sal_log.h"

#define CHAR_TO_INT(x) ((int)(x) - '0')
/**LOG define*/
#define LOG(...)       sal_log(__VA_ARGS__)
#define LOG_ERROR(...) sal_log_error("[DEMO-TTS] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-TTS] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-TTS] " __VA_ARGS__)

#if 0
void sAPI_TTSSetStatusCallBack(sAPI_TTSStatussCb ttsCb);
#endif

static ret_msg_t tts_play_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t tts_stop_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t tts_set_para_cmd_proc(op_t op, arg_t *argv, int argn);
// static ret_msg_t tts_text_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms));
// static int tts_text_raw_size(void);

/**parameters define*/
const value_t option[2] =
{
    {
        .bufp = "1_UNICODE",
        .size = 9,
    },
    {
        .bufp = "2_ASCII",
        .size = 7,
    }
};
const value_list_t option_list =
{
    .isrange = false,
    .list_head = option,
    .count = sizeof(option) / sizeof(value_t)
};

const value_t playMode[2] =
{
    {
        .bufp = "0_LOCAL",
        .size = 7,
    },
    {
        .bufp = "1_REMOTE",
        .size = 8,
    }
};
const value_list_t playMode_list =
{
    .isrange = false,
    .list_head =playMode,
    .count = sizeof(playMode) / sizeof(value_t)
};


arg_t paly_args[3] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGTTS_PALY_ENCODE,
        .value_range = &option_list
    },
    {
#if 1
        // .type = TYPE_RAW,
        .type = TYPE_RAW_GBK,
        .arg_name = ARGTTS_PALY_TEXT
#else
        .type = TYPE_STR,
        .arg_name = ARGTTS_PALY_TEXT
#endif
    },
    {
        .type = TYPE_STR,
        .arg_name =  ARGTTS_PLAY_PATH,
        .value_range = &playMode_list
    }
};


const value_list_t para_tts_vol =
{
    .isrange = true,
    .min = 0,
    .max = 11
};

arg_t tts_para_args[5] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGTTS_PARA_SYS_VOL,
        .value_range = &para_tts_vol,
        .msg = "TTS vol (Range: 0-11)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGTTS_PARA_ENC_VOL,
        .msg = "sys vol (YT:0-3, IFLY:0-2)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGTTS_PARA_DIGIT_MODE,
        .msg = "digit mode (YT:0-3, IFLY:0-2)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGTTS_PARA_PITCH,
        .msg = "pitch para (YT:0-2, IFLY:0-2)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGTTS_PARA_SPEED,
        .msg = "speed para (YT:0-2, IFLY:0-2)"
    }
};

const menu_list_t func_tts_play =
{
    .menu_name = "TTS play",
    .menu_type = TYPE_FUNC,
    .parent = &tts_menu,
    .function = {
        .argv = paly_args,
        .argn = sizeof(paly_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = tts_play_cmd_proc,
#if 0
        .methods.raw_handler = tts_text_raw_proc,
        .methods.needRAWData = tts_text_raw_size,
#endif
    }
};
const menu_list_t func_tts_para_set =
{
    .menu_name = "TTS parameters",
    .menu_type = TYPE_FUNC,
    .parent = &tts_menu,
    .function = {
        .argv = tts_para_args,
        .argn = sizeof(tts_para_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = tts_set_para_cmd_proc,
    }
};

const menu_list_t func_tts_stop =
{
    .menu_name = "TTS stop",
    .menu_type = TYPE_FUNC,
    .parent = &tts_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.cmd_handler = tts_stop_cmd_proc,
    }
};

const menu_list_t *tts_menu_list[] =
{
    &func_tts_play,
    &func_tts_stop,
    &func_tts_para_set,
    NULL
};

const menu_list_t tts_menu =
{
    .menu_name = "TTS",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = tts_menu_list,
    .parent = &root_menu,
};


static ret_msg_t tts_play_cmd_proc(op_t op, arg_t *argv, int argn)
{
    LOG_TRACE("tts_play_cmd_proc enter!");
    ret_msg_t ret = {0};
    const char* encodeMode = NULL;
    const char *inputText = NULL;
    const char* playPath =NULL;

#ifdef TTS_TEST_DEBUG
    int i = 0;
    for(i = 0; i < argn; i++) {
        LOG_INFO("param argv[%d].type=%d, argv[%d].value=%d", i, argv[i].type, i, argv[i].value);
    }
#endif

    pl_demo_get_data(&encodeMode, argv, argn, ARGTTS_PALY_ENCODE);
    pl_demo_get_data(&inputText, argv, argn, ARGTTS_PALY_TEXT);
    pl_demo_get_data(&playPath, argv, argn, ARGTTS_PLAY_PATH);
    if (NULL == inputText || !encodeMode || !playPath)
    {
        LOG_ERROR("param invalid! encodeMode=%s, playPath=%s", encodeMode, playPath);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    if(sAPI_TTSPlay(CHAR_TO_INT(*encodeMode), (char *)inputText, CHAR_TO_INT(*playPath)))
    {
        ret.errcode = ERRCODE_OK;
        ret.msg = "tts paly start";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "tts paly fail";
    }

    return ret;
}

static ret_msg_t tts_stop_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};

    if(sAPI_TTSStop())
    {
        ret.errcode = ERRCODE_OK;
        ret.msg = "tts stop success";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "tts stop fail";
    }

    return ret;
}

static ret_msg_t tts_set_para_cmd_proc(op_t op, arg_t *argv, int argn)
{
    LOG_TRACE("tts_set_para_cmd_proc enter!");
    ret_msg_t ret = {0};
    int ttsVolume, sysVolume, digitMode, pitchPara, speedPara;

    pl_demo_get_value(&sysVolume, argv, argn, ARGTTS_PARA_SYS_VOL);
    pl_demo_get_value(&ttsVolume, argv, argn, ARGTTS_PARA_ENC_VOL);
    pl_demo_get_value(&digitMode, argv, argn, ARGTTS_PARA_DIGIT_MODE);
    pl_demo_get_value(&pitchPara, argv, argn, ARGTTS_PARA_PITCH);
    pl_demo_get_value(&speedPara, argv, argn, ARGTTS_PARA_SPEED);

#ifdef TTS_TEST_DEBUG
    int i = 0;
    for(i = 0; i < argn; i++) {
        LOG_INFO("param argv[%d].type=%d, argv[%d].value=%d", i, argv[i].type, i, argv[i].value);
    }
#endif

    if(TTSSetParameters(sysVolume, ttsVolume, digitMode, pitchPara, speedPara))
    {
        ret.errcode = ERRCODE_OK;
        ret.msg = "tts set para success";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "tts set para fail";
    }

    return ret;
}
