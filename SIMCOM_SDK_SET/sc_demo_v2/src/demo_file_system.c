/*
 * @Author: Louis_Qiu
 * @Date: 2023-11-20 11:15:41
 * @Last Modified by: Louis_Qiu
 * @Last Modified time: 2023-12-08 11:46:02
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "demo_file_system.h"
#include "simcom_file.h"
#include "simcom_os.h"
#ifdef HAS_DEMO_FS_AUTH
#include "simcom_auth_api.h"
#endif

#define LOG_ERROR(...) sal_log_error("[DEMO-FS] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-FS] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-FS] " __VA_ARGS__)


static int write_file_need_size(void);
static ret_msg_t write_file_init(void);
static ret_msg_t write_file_uninit(void);
static ret_msg_t write_file_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t write_file_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms));

static ret_msg_t read_file_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t del_file_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t rename_file_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t disk_size_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t mkdir_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t rmdir_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t list_cmd_proc(op_t op, arg_t *argv, int argn);
#if defined (EXT_FLASH_LFSV2_ENABLE) || defined (EXT_FLASH_LFSV1_ENABLE) || defined (FEATURE_SIMCOM_SD_CARD) || defined (EXT_FLASH_YAFFS2_ENABLE)
static ret_msg_t format_cmd_proc(op_t op, arg_t *argv, int argn);
#endif
static ret_msg_t switch_dir_cmd_proc(op_t op, arg_t *argv, int argn);
#ifdef HAS_DEMO_FS_AUTH
static ret_msg_t auth_connect_cmd_proc(op_t op, arg_t *argv, int argn);
#endif

arg_t write_args[] =
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARGFILE_PATHNAME,
        .msg = "Support disks: C:. e.g: C:/test.txt"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGFILE_FILESIZE,
    }
};

const menu_list_t func_write =
{
    .menu_name = MENU_NAME_WRITE_FILE,
    .menu_type = TYPE_FUNC,
    .parent = &fs_menu,
    .function = {
        .argv = write_args,
        .argn = 2,
        .RAWSizeLink = ARGFILE_FILESIZE,
        .methods.__init = write_file_init,
        .methods.__uninit = write_file_uninit,
        .methods.cmd_handler = write_file_cmd_proc,
        .methods.raw_handler = write_file_raw_proc,
        .methods.needRAWData = write_file_need_size,
    },
};

arg_t rw_args[1] =
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARGFILE_PATHNAME,
        .msg = "Support disks: C:"
    }
};

const menu_list_t func_read =
{
    .menu_name = MENU_NAME_READ_FILE,
    .menu_type = TYPE_FUNC,
    .parent = &fs_menu,
    .function = {
        .argv = rw_args,
        .argn = sizeof(rw_args) / sizeof(arg_t),
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.raw_handler = NULL,
        .methods.cmd_handler = read_file_cmd_proc,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_delete =
{
    .menu_name = MENU_NAME_DELETE_FILE,
    .menu_type = TYPE_FUNC,
    .parent = &fs_menu,
    .function = {
        .argv = rw_args,
        .argn = sizeof(rw_args) / sizeof(arg_t),
        .methods.cmd_handler = del_file_cmd_proc,
    },
};

arg_t rename_args[2] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGFILE_OLDPATH,
        .msg = "Support disks: C:"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGFILE_NEWPATH,
        .msg = "Support disks: C:"
    }
};

const menu_list_t func_rename =
{
    .menu_name = MENU_NAME_RENAME_FILE,
    .menu_type = TYPE_FUNC,
    .parent = &fs_menu,
    .function = {
        .argv = rename_args,
        .argn = sizeof(rename_args) / sizeof(arg_t),
        .methods.cmd_handler = rename_file_cmd_proc,
    },
};

const value_t disks[] =
{
    {
        .bufp = "C:",
        .size = 2,
    },
#if defined (EXT_FLASH_LFSV2_ENABLE) || defined (EXT_FLASH_LFSV1_ENABLE) || defined (FEATURE_SIMCOM_SD_CARD) || defined (EXT_FLASH_YAFFS2_ENABLE)
    {
        .bufp = "D:",
        .size = 2,
    },
#endif
#ifdef HAS_DEMO_FS_AUTH
    {
        .bufp = "S:",
        .size = 2,
    },
#endif
};
const value_list_t disk_range =
{
    .isrange = false,
    .list_head = disks,
#if (defined (EXT_FLASH_LFSV2_ENABLE) || defined (EXT_FLASH_LFSV1_ENABLE) || defined (FEATURE_SIMCOM_SD_CARD) || defined (EXT_FLASH_YAFFS2_ENABLE)) && defined (HAS_DEMO_FS_AUTH)
    .count = 3,
#elif defined (EXT_FLASH_LFSV2_ENABLE) || defined (EXT_FLASH_LFSV1_ENABLE) || defined (FEATURE_SIMCOM_SD_CARD) || defined (EXT_FLASH_YAFFS2_ENABLE)
    .count = 2,
#elif defined (HAS_DEMO_FS_AUTH)
    .count = 2,
#else
    .count = 1,
#endif
};
arg_t disk_args[1] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGFILE_DISK,
        .value_range = &disk_range,
    }
};

const menu_list_t func_get_disk_size =
{
    .menu_name = MENU_NAME_GET_DISK_SIZE,
    .menu_type = TYPE_FUNC,
    .parent = &fs_menu,
    .function = {
        .argv = disk_args,
        .argn = sizeof(disk_args) / sizeof(arg_t),
        .methods.cmd_handler = disk_size_cmd_proc,
    },
};

const menu_list_t func_mkdir =
{
    .menu_name = MENU_NAME_MKDIR,
    .menu_type = TYPE_FUNC,
    .parent = &fs_menu,
    .function = {
        .argv = rw_args,
        .argn = sizeof(rw_args) / sizeof(arg_t),
        .methods.cmd_handler = mkdir_cmd_proc,
    },
};

const menu_list_t func_rmdir =
{
    .menu_name = MENU_NAME_RMDIR,
    .menu_type = TYPE_FUNC,
    .parent = &fs_menu,
    .function = {
        .argv = rw_args,
        .argn = sizeof(rw_args) / sizeof(arg_t),
        .methods.cmd_handler = rmdir_cmd_proc,
    },
};

const menu_list_t func_ls =
{
    .menu_name = MENU_NAME_LS,
    .menu_type = TYPE_FUNC,
    .parent = &fs_menu,
    .function = {
        .argv = rw_args,
        .argn = sizeof(rw_args) / sizeof(arg_t),
        .methods.cmd_handler = list_cmd_proc,
    },
};

#if defined (EXT_FLASH_LFSV2_ENABLE) || defined (EXT_FLASH_LFSV1_ENABLE) || defined (FEATURE_SIMCOM_SD_CARD) || defined (EXT_FLASH_YAFFS2_ENABLE)
const menu_list_t func_format =
{
    .menu_name = MENU_NAME_FORMAT,
    .menu_type = TYPE_FUNC,
    .parent = &fs_menu,
    .function = {
        .argv = disk_args,
        .argn = sizeof(disk_args) / sizeof(arg_t),
        .methods.cmd_handler = format_cmd_proc,
    },
};
#endif

value_t switchmodeList[2] =
{
    {
        .bufp = ARGFILE_SWITCH_TO_ROOT,
        .size = sizeof(ARGFILE_SWITCH_TO_ROOT)
    },
    {
        .bufp = ARGFILE_SWITCH_TO_C,
        .size = sizeof(ARGFILE_SWITCH_TO_C)
    }
};

value_list_t switchmodeRange =
{
    .isrange = false,
    .list_head = switchmodeList,
    .count = sizeof(switchmodeList) / sizeof(value_t)
};

arg_t switch_args[2] =
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARGFILE_PATHNAME,
        .msg = "Support disks: C:"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGFILE_SWITCHMODE,
        .value_range = &switchmodeRange,
    }
};

const menu_list_t func_switchdir =
{
    .menu_name = MENU_NAME_SWITCHDIR,
    .menu_type = TYPE_FUNC,
    .parent = &fs_menu,
    .function = {
        .argv = switch_args,
        .argn = sizeof(switch_args) / sizeof(arg_t),
        .methods.cmd_handler = switch_dir_cmd_proc,
    },
};

#ifdef HAS_DEMO_FS_AUTH
value_list_t timerRange =
{
    .isrange = true,
    .min = 0 ,
    .max = 6000
};

arg_t authconnect_args[2] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGAUTH_PASSWORD,
        .msg = "Input password(HEX)"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGAUTH_TIMEOUT,
        .value_range = &timerRange,
    }
};

const menu_list_t func_auth =
{
    .menu_name = MENU_NAME_AUTHCONNECT,
    .menu_type = TYPE_FUNC,
    .parent = &fs_menu,
    .function = {
        .argv = authconnect_args,
        .argn = sizeof(authconnect_args) / sizeof(arg_t),
        .methods.cmd_handler = auth_connect_cmd_proc,
    },
};
#endif

const menu_list_t *fs_menu_sub[] =
{
    &func_write,
    &func_read,
    &func_delete,
    &func_rename,
    &func_get_disk_size,
    &func_mkdir,
    &func_rmdir,
    &func_ls,
#if defined (EXT_FLASH_LFSV2_ENABLE) || defined (EXT_FLASH_LFSV1_ENABLE) || defined (FEATURE_SIMCOM_SD_CARD) || defined (EXT_FLASH_YAFFS2_ENABLE)
    &func_format,
#endif
    &func_switchdir,
#ifdef HAS_DEMO_FS_AUTH
    &func_auth,
#endif
    NULL  // MUST end by NULL
};

const menu_list_t fs_menu =
{
    .menu_name = MENU_NAME_FS,
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = fs_menu_sub,
    .parent = &root_menu,
};


int g_file_size;
SCFILE *g_write_fd;
sTaskRef g_write_task_ref;
pub_cache_p g_write_cache;
ret_msg_t g_write_result;

static int write_file_need_size(void)
{
    return g_file_size;
}

static void write_file_task(void *args);

static ret_msg_t write_file_uninit(void)
{
    ret_msg_t ret = {0};

    pub_bzero(&g_write_result, sizeof(ret_msg_t));
    g_file_size = 0;
    pub_cache_set_status(g_write_cache, CACHE_STOPED);

    sAPI_TaskSleep(5);  // wait task exit.

    if (g_write_task_ref)
    {
        ret.errcode = sAPI_TaskDelete(g_write_task_ref);
        if (ret.errcode != 0)
        {
            LOG_ERROR("task delete fail, errcode=%d", ret.errcode);
            goto DONE;
        }
        else
        {
            g_write_task_ref = NULL;
        }
    }

    if (g_write_fd)
    {
        sAPI_fclose(g_write_fd);
        g_write_fd = NULL;
    }

    if (g_write_cache)
    {
        ret.errcode = pub_cache_delete(&g_write_cache);
        if (ret.errcode != 0)
        {
            LOG_ERROR("cache delete fail");
            goto DONE;
        }
    }

DONE:
    return ret;
}

static ret_msg_t write_file_init(void)
{
    ret_msg_t ret = {0};

    ret = write_file_uninit();
    if (0 != ret.errcode)
    {
        goto DONE;
    }

    g_write_cache = pub_cache_create(SAL_20K, CACHE_TYPE_BUFFER, CACHE_SYNC_R);
    if (NULL == g_write_cache)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("cache create fail");
        goto DONE;
    }

DONE:
    return ret;
}

static ret_msg_t write_file_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    INT64 free_size = 0;
    const char *filePath = NULL;
    int fileSize = 0;

    pl_demo_get_data(&filePath, argv, argn, ARGFILE_PATHNAME);
    pl_demo_get_value(&fileSize, argv, argn, ARGFILE_FILESIZE);
    if (NULL == filePath || fileSize < 0)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    free_size = sAPI_GetFreeSize((char *)filePath);
    if (fileSize >= free_size)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "file larger than free size";
        return ret;
    }

    g_write_fd = sAPI_fopen(filePath, "wb");
    if (NULL == g_write_fd)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "file open fail";
        return ret;
    }

    g_file_size = fileSize;

    if (fileSize > 0)
    {
        ret.errcode = sAPI_TaskCreate(&g_write_task_ref,
                                      NULL,
                                      SAL_2K,
                                      150,
                                      "SAVE FILE",
                                      write_file_task,
                                      NULL);
        if (ret.errcode != SC_SUCCESS)
        {
            ret.msg = "task creat fail";
        }
    }
    else
    {
        LOG_INFO("NULL file");
    }

    return ret;
}

static ret_msg_t write_file_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms))
{
    ret_msg_t ret = {0};
    int read_size = 0;
    int process_size = 0;
    int remain_size = size;
    char buf[SAL_256] = {0};

    if (g_file_size != size)
    {
        LOG_ERROR("raw data size invalid");
        LOG_ERROR("size:%d, need:%d", size, g_file_size);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "raw data size invalid";
        return ret;
    }

    while (remain_size > 0 && g_write_fd)
    {
        read_size = remain_size <= SAL_256 ? remain_size : SAL_256;
        read_size = raw_read(buf, read_size, 10000);
        if (read_size < 0)
        {
            LOG_ERROR("raw data read fail");
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "raw data read fail";
            goto EXIT;
        }
        else if (0 == read_size)
        {
            LOG_ERROR("raw data read timeout");
            ret.errcode = ERRCODE_RAW_RECV_TIMEOUT;
            ret.msg = "raw data read timeout";
            goto EXIT;
        }

        process_size = pub_cache_write(g_write_cache, buf, read_size);
        if (process_size < read_size)
        {
            LOG_ERROR("raw data cache fail");
            LOG_ERROR("read size:%d, cache size:%d", read_size, process_size);
            ret.errcode = process_size < 0 ? process_size : ERRCODE_DEFAULT;
            ret.msg = "raw data cache fail";
            goto EXIT;
        }

        remain_size -= read_size;
    }

EXIT:
    /* wait file write to fs. */
    pub_cache_set_status(g_write_cache, CACHE_STOPED);
    while (NULL != g_write_fd) sAPI_TaskSleep(20);

    if (ret.errcode != 0) return ret;
    else return g_write_result;
}

static void write_file_task(void *args)
{
    ret_msg_t ret = {0};
    char *buf = NULL;
    int read_size = 0, process_size = 0;

    buf = sal_malloc(SAL_4K);
    if (!buf)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "malloc fail";
        goto EXIT;
    }

    while (g_file_size > 0 && g_write_fd && g_write_cache)
    {
        read_size = g_file_size <= SAL_4K ? g_file_size : SAL_4K;

        read_size = pub_cache_read(g_write_cache, buf, read_size);
        if (read_size <= 0)
        {
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "cache read fail";
            goto EXIT;
        }

        process_size = sAPI_fwrite(buf, 1, read_size, g_write_fd);
        if (process_size != read_size)
        {
            LOG_ERROR("write file fail");
            LOG_ERROR("writed size:%d, need write size:%d", process_size, read_size);
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "write file fail";
            goto EXIT;
        }

        g_file_size -= process_size;
    }

EXIT:
    sal_free(buf);
    sAPI_fclose(g_write_fd);
    g_write_fd = NULL;
    g_write_result = ret;
}

static ret_msg_t read_file_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *pathName = NULL;
    SCFILE *fd = NULL;
    int fileSize = 0;
    int read_size = 0;
    char *buf = NULL;
    arg_t out_arg[1] = {0};

    buf = sal_malloc(SAL_1K);
    if (!buf)
    {
        LOG_ERROR("malloc fail");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "malloc fail";
        return ret;
    }

    pl_demo_get_data(&pathName, argv, argn, ARGFILE_PATHNAME);
    if (!pathName)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        goto EXIT;
    }

    fd = sAPI_fopen(pathName, "rb");
    if (!fd)
    {
        LOG_ERROR("file open fail");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "file open fail";
        goto EXIT;
    }

    fileSize = sAPI_fsize(fd);

    out_arg[0].arg_name = pre_arg_name[ARG_ID_DATA];
    out_arg[0].type = TYPE_RAW;
    out_arg[0].value = pl_demo_make_value(0, NULL, fileSize);

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
    if (0 != ret.errcode)
    {
        ret.msg = "post data fail";
        goto EXIT;
    }

    while (fileSize > 0)
    {
        read_size = fileSize <= SAL_1K ? fileSize : SAL_1K;
        read_size = sAPI_fread(buf, 1, read_size, fd);
        if (read_size <= 0)
        {
            LOG_ERROR("file read fail");
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "file read fail";
            pl_demo_post_raw_cancel();
            goto EXIT;
        }

        pl_demo_post_raw(buf, read_size);
        fileSize -= read_size;
    }

EXIT:
    pl_demo_release_value(out_arg[0].value);
    sal_free(buf);
    if (fd) sAPI_fclose(fd);

    return ret;
}

static ret_msg_t del_file_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *pathName = NULL;
    struct dirent_t info = {0};

    pl_demo_get_data(&pathName, argv, argn, ARGFILE_PATHNAME);
    if (!pathName)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    ret.errcode = sAPI_stat(pathName, &info);
    if (0 != ret.errcode)
    {
        LOG_ERROR("file not exist");
        ret.msg = "file not exist";
        return ret;
    }

    ret.errcode = sAPI_remove(pathName);

    return ret;
}

static ret_msg_t rename_file_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *oldpath = NULL;
    const char *newpath = NULL;
    struct dirent_t info = {0};

    pl_demo_get_data(&oldpath, argv, argn, ARGFILE_OLDPATH);
    pl_demo_get_data(&newpath, argv, argn, ARGFILE_NEWPATH);
    if (!oldpath || !newpath)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    ret.errcode = sAPI_stat(oldpath, &info);
    if (0 != ret.errcode)
    {
        LOG_ERROR("file not exist");
        ret.msg = "file not exist";
        return ret;
    }

    ret.errcode = sAPI_stat(newpath, &info);
    if (0 == ret.errcode)
    {
        LOG_ERROR("new file already exist");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "new file already exist";
        return ret;
    }

    ret.errcode = sAPI_rename(oldpath, newpath);

    return ret;
}

static ret_msg_t disk_size_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *disk = NULL;
    int total_size = 0;
    int free_size = 0;
    int used_size = 0;
    arg_t out_arg[3] = {0};

    pl_demo_get_data(&disk, argv, argn, ARGFILE_DISK);
    if (!disk)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    total_size = sAPI_GetSize((char *)disk);
    free_size = sAPI_GetFreeSize((char *)disk);
    used_size = sAPI_GetUsedSize((char *)disk);

    out_arg[0].type = TYPE_INT;
    out_arg[0].arg_name = ARGFILE_DISK_TSIZE;
    out_arg[0].value = pl_demo_make_value(total_size, NULL, 0);

    out_arg[1].type = TYPE_INT;
    out_arg[1].arg_name = ARGFILE_DISK_FSIZE;
    out_arg[1].value = pl_demo_make_value(free_size, NULL, 0);

    out_arg[2].type = TYPE_INT;
    out_arg[2].arg_name = ARGFILE_DISK_USIZE;
    out_arg[2].value = pl_demo_make_value(used_size, NULL, 0);

    ret.errcode = pl_demo_post_data(OP_POST, out_arg, 3, NULL, 0);

    pl_demo_release_value(out_arg[0].value);
    pl_demo_release_value(out_arg[1].value);
    pl_demo_release_value(out_arg[2].value);

    return ret;
}

static ret_msg_t mkdir_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *pathName = NULL;

    pl_demo_get_data(&pathName, argv, argn, ARGFILE_PATHNAME);
    if (!pathName)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    ret.errcode = sAPI_mkdir(pathName, 0);

    return ret;
}

static ret_msg_t rmdir_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *pathName = NULL;

    pl_demo_get_data(&pathName, argv, argn, ARGFILE_PATHNAME);
    if (!pathName)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    ret.errcode = sAPI_remove(pathName);

    return ret;
}

static ret_msg_t list_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *pathName = NULL;
    SCDIR *dir_fp = NULL;
    struct dirent_t *info = NULL;
    arg_t out_arg[3] = {0};
    int arg_count = 0;
    int dir_count = 0;
    int file_count = 0;

    pl_demo_get_data(&pathName, argv, argn, ARGFILE_PATHNAME);
    if (!pathName)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    dir_fp = sAPI_opendir(pathName);
    if (!dir_fp)
    {
        LOG_ERROR("dir %s open fail", pathName);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "dir open fail";
        return ret;
    }

    for (info = sAPI_readdir(dir_fp); info; info = sAPI_readdir(dir_fp))
    {
        pl_demo_release_value(out_arg[0].value);
        pl_demo_release_value(out_arg[1].value);
        pl_demo_release_value(out_arg[2].value);

        out_arg[0].arg_name = ARGFILE_LS_NAME;
        out_arg[0].type = TYPE_RAW_GBK;
        out_arg[0].value = pl_demo_make_value(0, NULL, strlen(info->name));

        out_arg[1].arg_name = ARGFILE_LS_TYPE;
        if (TYPE_DIR == info->type)
        {
            out_arg[1].value = pl_demo_make_value(0, ARGFILE_LS_TYPE_DIR, strlen(ARGFILE_LS_TYPE_DIR));

            arg_count = 2;
            dir_count++;
        }
        else
        {
            out_arg[1].value = pl_demo_make_value(0, ARGFILE_LS_TYPE_FILE, strlen(ARGFILE_LS_TYPE_FILE));

            out_arg[2].arg_name = ARGFILE_LS_SIZE;
            out_arg[2].type = TYPE_INT;
            out_arg[2].value = pl_demo_make_value(info->size, NULL, 0);

            arg_count = 3;
            file_count++;
        }

        pl_demo_post_data(OP_POST, out_arg, arg_count, info->name, strlen(info->name));
    }

    pl_demo_release_value(out_arg[0].value);
    out_arg[0].arg_name = ARGFILE_LS_DIR_COUNT;
    out_arg[0].type = TYPE_INT;
    out_arg[0].value = pl_demo_make_value(dir_count, NULL, 0);

    pl_demo_release_value(out_arg[1].value);
    out_arg[1].arg_name = ARGFILE_LS_FILE_COUNT;
    out_arg[1].type = TYPE_INT;
    out_arg[1].value = pl_demo_make_value(file_count, NULL, 0);

    pl_demo_post_data(OP_POST, out_arg, 2, NULL, 0);

    if (sAPI_closedir(dir_fp) != 0)
    {
        LOG_ERROR("dir close fail");
    }

    pl_demo_release_value(out_arg[0].value);
    pl_demo_release_value(out_arg[1].value);
    pl_demo_release_value(out_arg[2].value);

    return ret;
}

#if defined (EXT_FLASH_LFSV2_ENABLE) || defined (EXT_FLASH_LFSV1_ENABLE) || defined (FEATURE_SIMCOM_SD_CARD) || defined (EXT_FLASH_YAFFS2_ENABLE)
static ret_msg_t format_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *disk = NULL;

    pl_demo_get_data(&disk, argv, argn, ARGFILE_DISK);
    if (!disk)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    ret.errcode = sAPI_fformat((char *)disk);

    return ret;
}
#endif

static ret_msg_t switch_dir_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *pathName = NULL;
    const char *mode_str = NULL;
    int dirction = 0;

    pl_demo_get_data(&pathName, argv, argn, ARGFILE_PATHNAME);
    pl_demo_get_data(&mode_str, argv, argn, ARGFILE_SWITCHMODE);
    if (!pathName)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    if (0 == strcmp(mode_str, ARGFILE_SWITCH_TO_C))/*c:/1.txt->c:/simdir/1.txt*/
    {
        dirction = 0;
    }
    else/*c:/simdir/1.txt->c:/1.txt*/
    {
        dirction = 1;
    }

    ret.errcode = sAPI_FsSwitchDir((char *)pathName, dirction);
    if (ret.errcode)
    {
        LOG_ERROR("SwitchDir err");
        ret.msg = "SwitchDir err";
        ret.errcode = ERRCODE_DEFAULT;
        return ret;
    }

    ret.msg = "SwitchDir succ";
    return ret;
}

#ifdef HAS_DEMO_FS_AUTH
static ret_msg_t auth_connect_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *pass_word = NULL;
    unsigned long password = 0;
    int timeout = 0;
    char *buf = NULL;
    arg_t out_arg[1] = {0};

    buf = sal_malloc(SAL_128);
    if (!buf)
    {
        LOG_ERROR("malloc fail");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "malloc fail";
        return ret;
    }

    pl_demo_get_data(&pass_word, argv, argn, ARGAUTH_PASSWORD);
    pl_demo_get_value(&timeout, argv, argn, ARGAUTH_TIMEOUT);
    if (!pass_word)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    password = strtoul(pass_word,NULL,16);
    LOG_INFO("password = [%lu]",password);

    ret.errcode = sAPI_auth((unsigned int)password, timeout);
    if (-1 == ret.errcode)
    {
        LOG_ERROR("Auth err");
        ret.msg = "Auth err";
        ret.errcode = ERRCODE_DEFAULT;
    }
    else if(0 == ret.errcode)
    {
        LOG_INFO("Auth succ");
        ret.msg = "Auth succ";
    }
    else
    {
        LOG_INFO("Magic is %d",ret.errcode);
        sprintf(buf, "magic: %d", ret.errcode);
        out_arg[0].arg_name = pre_arg_name[ARG_ID_DATA];
        out_arg[0].type = TYPE_RAW;
        out_arg[0].value = pl_demo_make_value(0, NULL, strlen(buf));

        ret.errcode = pl_demo_post_data(OP_POST, out_arg, 1, NULL, 0);
        if (0 != ret.errcode)
        {
            ret.msg = "post data fail";
            ret.errcode = ERRCODE_DEFAULT;
            pl_demo_release_value(out_arg[0].value);
            return ret;
        }
        pl_demo_post_raw(buf, strlen(buf));
        pl_demo_release_value(out_arg[0].value);
    }

    sal_free(buf);
    return ret;
}
#endif
