#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "demo_poc.h"
#include "simcom_os.h"
#include "simcom_poc.h"
#include "simcom_demo_init.h"
#include "simcom_file.h"

#include "sal_os.h"
#include "sal_log.h"

#define POC_LOG(...)          sal_log(__VA_ARGS__)
#define POC_LOG_ERROR(...)    sal_log_error("[DEMO-POC] " __VA_ARGS__)
#define POC_LOG_INFO(...)     sal_log_info("[DEMO-POC] " __VA_ARGS__)
#define POC_LOG_TRACE(...)    sal_log_trace("[DEMO-POC] " __VA_ARGS__)

#define POC_PLAY_BUFFER_FRAMS_MAX   20

static ret_msg_t poc_init(void);
static ret_msg_t poc_uninit(void);
static int poc_play_sound_need_size(void);
static ret_msg_t poc_play_sound_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms));
static ret_msg_t poc_play_sound_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t poc_stop_sound_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t poc_start_record_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t poc_stop_record_cmd_proc(op_t op, arg_t *argv, int argn);

arg_t poc_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGPOC_SIZE,
        .msg = "The poc data size must be less than 6.4K and a multiple of 320"
    }
};

const menu_list_t func_poc_play_sound =
{
    .menu_name = "poc play sound",
    .menu_type = TYPE_FUNC,
    .parent = &poc_menu,
    .function = {
        .argv = poc_args,
        .argn = sizeof(poc_args) / sizeof(arg_t),
        .RAWSizeLink = ARGPOC_SIZE,
        .methods.__init = poc_init,
        .methods.__uninit = poc_uninit,
        .methods.cmd_handler = poc_play_sound_cmd_proc,
        .methods.raw_handler = poc_play_sound_raw_proc,
        .methods.needRAWData = poc_play_sound_need_size,
    }
};

const menu_list_t func_poc_stop_sound =
{
    .menu_name = "poc stop sound",
    .menu_type = TYPE_FUNC,
    .parent = &poc_menu,
    .function = {
        .methods.cmd_handler = poc_stop_sound_cmd_proc,
    }
};

const value_t poc_mode_param[2] = 
{
    {.val = 1},
    {.val = 2}
};

const value_list_t poc_record_mode_list =
{
    .isrange = false,
    .list_head = poc_mode_param,
    .count = sizeof(poc_mode_param) / sizeof(value_t),
};

arg_t poc_record_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGPOC_MODE,
        .value_range = &poc_record_mode_list,
        .msg = "record mode: 1(FORWORDLY), 2(PASSIVELY)"
    },
};

const menu_list_t func_poc_start_record =
{
    .menu_name = "poc record",
    .menu_type = TYPE_FUNC,
    .parent = &poc_menu,
    .function = {
        .argv = poc_record_args,
        .argn = sizeof(poc_record_args) / sizeof(arg_t),
        .methods.cmd_handler = poc_start_record_cmd_proc,
    }
};

const menu_list_t func_poc_stop_record =
{
    .menu_name = "poc stop record",
    .menu_type = TYPE_FUNC,
    .parent = &poc_menu,
    .function = {
        .methods.cmd_handler = poc_stop_record_cmd_proc,
    }
};

const menu_list_t *poc_menu_list[] = 
{
    &func_poc_play_sound,
    &func_poc_stop_sound,
    &func_poc_start_record,
    &func_poc_stop_record,
    NULL
};

const menu_list_t poc_menu =
{
    .menu_name = "POC",
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = poc_menu_list,
    .parent = &root_menu,
};

char *g_poc_data_buf = NULL;
int g_poc_data_size;
sTaskRef g_poc_task_ref;
sal_sema_ref poc_syn_ref;
SCFILE * g_record_fd = NULL;

static ret_msg_t poc_uninit(void)
{
    POC_LOG("poc_uninit enter!");
    ret_msg_t ret = { 0 };

    if (poc_syn_ref)
    {
        ret.errcode = sal_semaphore_delete(&poc_syn_ref);
    }

    return ret;
}

static ret_msg_t poc_init(void)
{
    POC_LOG("poc_init enter!");
    ret_msg_t ret = { 0 };
    poc_uninit();

    if (0 == sal_semaphore_create(&poc_syn_ref, 0))
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

static ret_msg_t poc_play_sound_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms))
{
    ret_msg_t ret = { 0 };
    int read_size = 0;
    int remain_size = size;
    int copy_size = 0;
    char buf[POC_PLAY_BUFFER_FRAMS_MAX *320 + 1] = { 0 };

    if (g_poc_data_size != size)
    {
        POC_LOG_ERROR("raw data size invalid");
        POC_LOG_ERROR("size: %d, need: %d", size, g_poc_data_size);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "raw data size invalid";
        goto EXIT;
    }

    g_poc_data_buf = sal_malloc(g_poc_data_size + 1);
    if (!g_poc_data_buf)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "malloc fail";
        goto EXIT;
    }

    while (remain_size > 0)
    {
        read_size = remain_size <= POC_PLAY_BUFFER_FRAMS_MAX *320 ? remain_size : POC_PLAY_BUFFER_FRAMS_MAX *320;
        read_size = raw_read(buf, read_size, 10000);
        if (read_size < 0)
        {
            POC_LOG_ERROR("raw data read fail");
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "raw data read fail";
            goto EXIT;
        }
        else if (0 == read_size)
        {
            POC_LOG_ERROR("raw data read timeout");
            ret.errcode = ERRCODE_RAW_RECV_TIMEOUT;
            ret.msg = "raw data read timeout";
            goto EXIT;
        }

        memcpy(g_poc_data_buf + copy_size, buf, read_size);

        copy_size += read_size;
        remain_size -= read_size;
        POC_LOG_INFO("copy_size:[%d], remain_size:[%d], read_size:[%d]", copy_size, remain_size, read_size);
    }

    sal_semaphore_release(poc_syn_ref);
    return ret;

EXIT:
    sal_free(g_poc_data_buf);
    sal_semaphore_release(poc_syn_ref);
    return ret;
}

static void poc_play_sound_task(void *args)
{
    POC_LOG("poc_play_sound_task enter!");
    ret_msg_t ret = { 0 };

    sal_semaphore_acquire(poc_syn_ref, SAL_OS_TIMEOUT_SUSPEND);

    if (!g_poc_data_buf)
    {
        POC_LOG_ERROR("g_poc_data_buf error! poc_data_size = %d", g_poc_data_size);
        goto EXIT;
    }

    ret.errcode = sAPI_PocPlaySound(g_poc_data_buf, g_poc_data_size);
    if (ret.errcode)
    {
        POC_LOG_ERROR("Poc fail to Play Sound, ret.errcode = %d", ret.errcode);
    }

EXIT:
    sal_free(g_poc_data_buf);
    g_poc_data_size = 0;
}

static int poc_play_sound_need_size(void)
{
    return g_poc_data_size;
}

static ret_msg_t poc_play_sound_cmd_proc(op_t op, arg_t *argv, int argn)
{
    POC_LOG("poc_play_sound_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    int poc_data_size = 0;

    pl_demo_get_value(&poc_data_size, argv, argn, ARGPOC_SIZE);
    if (poc_data_size < 0 || (poc_data_size % 320 != 0))
    {
        POC_LOG_ERROR("poc_data_size param invalid!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "poc_data_size param invalid";
        return ret;
    }

    g_poc_data_size = poc_data_size;

    ret.errcode = sal_task_create(&g_poc_task_ref, NULL, SAL_2K, sal_task_priority_low_1, "poc play sound", poc_play_sound_task, NULL);
    if (ret.errcode != SC_SUCCESS)
    {
        ret.msg = "task creat fail";
    }
    sal_task_detach(&g_poc_task_ref);

    return ret;
}

static ret_msg_t poc_stop_sound_cmd_proc(op_t op, arg_t *argv, int argn)
{
    POC_LOG("poc_stop_sound_cmd_proc enter!");
    ret_msg_t ret = { 0 };

    if (!sAPI_PocStopSound())
    {
        ret.errcode = ERRCODE_OK;
        ret.msg = "audio success to stop sound";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "audio fail to stop sound";
    }

    return ret;
}

int GetRecordCb(char *buffer, int len)
{
    POC_LOG("GetRecordCb enter!");
    int write_size = 0;

    if (NULL == buffer || 0 == len)
    {
        POC_LOG_ERROR("data is NULL!");
        return -1;
    }

    write_size = sAPI_fwrite(buffer, 1, len, g_record_fd);
    if (write_size != len)
    {
        POC_LOG_ERROR("write buf failed! write_size = %d, need_size = %d", write_size, len);
        return -2;
    }

    return 0;
}

static ret_msg_t poc_start_record_cmd_proc(op_t op, arg_t *argv, int argn)
{
    POC_LOG("poc_start_record_cmd_proc enter!");
    ret_msg_t ret = { 0 };
    int mode = 1;

    pl_demo_get_value(&mode, argv, argn, ARGPOC_MODE);
    if (mode < 1 || mode > 2)
    {
        POC_LOG_ERROR("mode param invalid!");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "mode param invalid";
        return ret;
    }
    POC_LOG_INFO("mode = %d", mode);

    g_record_fd = sAPI_fopen("c:/poc_record_test.wav", "ab+");
    if (mode == 2)
    {
        ret.errcode = sAPI_PocStartRecord(NULL, 2);
        ret.msg = "Get data passively";
    }
    else if (mode == 1)
    {
        ret.errcode = sAPI_PocStartRecord(GetRecordCb, 1);
        ret.msg = "Get data forwordly";
    }

    return ret;
}

static ret_msg_t poc_stop_record_cmd_proc(op_t op, arg_t *argv, int argn)
{
    POC_LOG("poc_stop_record_cmd_proc enter!");
    ret_msg_t ret = { 0 };

    if (!sAPI_PocStopRecord())
    {
        sAPI_fclose(g_record_fd);
        g_record_fd = NULL;
        ret.errcode = ERRCODE_OK;
        ret.msg = "audio success to stop record";
    }
    else
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "audio fail to stop record";
    }

    return ret;
}