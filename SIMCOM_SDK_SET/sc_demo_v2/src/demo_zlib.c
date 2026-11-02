#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "demo_zlib.h"
#include "simcom_os.h"
#include "simcom_demo_init.h"

#include "zip.h"
#include "unzip.h"
#include "simcom_file.h"

#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"


#define LOG_ERROR(...) sal_log_error("[DEMO-ZLIB] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-ZLIB] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-ZLIB] " __VA_ARGS__)

static ret_msg_t unzip_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t zip_cmd_proc(op_t op, arg_t *argv, int argn);

value_t replaceList[2] =
{
    {
        .bufp = ARGZLIB_REPLACE_FALSE,
        .size = sizeof(ARGZLIB_REPLACE_FALSE)
    },
    {
        .bufp = ARGZLIB_REPLACE_TRUE,
        .size = sizeof(ARGZLIB_REPLACE_TRUE)
    }
};

value_list_t replaceRange =
{
    .isrange = false,
    .list_head = replaceList,
    .count = sizeof(replaceList) / sizeof(value_t)
};

value_t passwordDefault =
{
    .bufp = "0",
    .size = 2
};

value_t savepathDefault =
{
    .bufp = "C:/",
    .size = 4
};

arg_t unzip_args[4] =
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARGZLIB_ZIPNAME,
        .msg = "Please input unzip path:like c:/test.zip"
    },
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARGZLIB_SAVEPATH,
        .value = &savepathDefault,
        .msg = "Please input save path:like c:/"
    },
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARGZLIB_PASSWORD,
        .value = &passwordDefault,
        .msg = "If no password, please input \"0\"."
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGZLIB_REPLACE,
        .value_range = &replaceRange
    }
};

value_t modeList[2] =
{
    {
        .bufp = ARGZLIB_ZIPNEW,
        .size = sizeof(ARGZLIB_ZIPNEW)
    },
    {
        .bufp = ARGZLIB_ZIPAPPEND,
        .size = sizeof(ARGZLIB_ZIPAPPEND)
    }
};

value_list_t modeRange =
{
    .isrange = false,
    .list_head = modeList,
    .count = sizeof(modeList) / sizeof(value_t)
};

arg_t zip_args[3] =
{
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARGZLIB_SRCNAME,
        .msg = "File/dir path you want to zip. e.g. C:/123.bin or C:/onedir"
    },
    {
        .type = TYPE_RAW_GBK,
        .arg_name = ARGZLIB_ZIPNAME,
        .msg = "Zip save path. e.g. c:/test.zip"
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGZLIB_ZIPMODE,
        .value_range = &modeRange,
    }
};

const menu_list_t func_unzip =
{
    .menu_name = MENU_NAME_UNZIP,
    .menu_type = TYPE_FUNC,
    .parent = &zlib_menu,
    .function = {
        .argv = unzip_args,
        .argn = sizeof(unzip_args) / sizeof(arg_t),
        .methods.cmd_handler = unzip_cmd_proc,
    },
};

const menu_list_t func_zip =
{
    .menu_name = MUNU_NAME_ZIP,
    .menu_type = TYPE_FUNC,
    .parent = &zlib_menu,
    .function = {
        .argv = zip_args,
        .argn = sizeof(zip_args) / sizeof(arg_t),
        .methods.cmd_handler = zip_cmd_proc,
    },
};

const menu_list_t *zlib_menu_sub[] =
{
    &func_unzip,
    &func_zip,
    NULL  // MUST end by NULL
};

const menu_list_t zlib_menu =
{
    .menu_name = MENU_NAME_ZLIB,
    .menu_type = TYPE_MENU,
    .__init = NULL,
    .__uninit = NULL,
    .submenu_list_head = zlib_menu_sub,
    .parent = &root_menu,
};

typedef void (*process_notify)(const menu_list_t *menu, int files, int curIdx, const char *curName,
                               int processing_rate);
#define NOTIFY(menu, cb, files, curIdx, curname, rate) \
    do { \
        if (cb) \
        { \
            cb(menu, files, curIdx, curname, rate); \
        } \
    } while(0);
#define ZIP_NOTIFY(cb, files, curIdx, curname, rate) \
    NOTIFY(&func_zip, cb, files, curIdx, curname, rate)
#define UNZIP_NOTIFY(cb, files, curIdx, curname, rate) \
    NOTIFY(&func_unzip, cb, files, curIdx, curname, rate)



/**
 * @brief
 *
 * @param files Count of files in zip. -1 indicate no care this arg.
 * @param curIdx Current unzip file idx in count. -1 indicate no care this arg.
 * @param curName Current unzip file name in zip. NULL indicate no care this arg.
 * @param processing_rate Current file unzip rate. -1 indicate no care this arg.
 */
static void processing_notify(const menu_list_t *menu, int files, int curIdx, const char *curName, int processing_rate)
{
    arg_t out_arg[4] = {0};
    int out_count = 0;

    if (files > 0)
    {
        out_arg[out_count].arg_name = URCNAME_TOTAL_FILES;
        out_arg[out_count].type = TYPE_INT;
        out_arg[out_count].value = pl_demo_make_value(files, NULL, 0);
        out_count++;
    }

    if (curIdx > 0)
    {
        out_arg[out_count].arg_name = URCNAME_HANDLE_FILES;
        out_arg[out_count].type = TYPE_INT;
        out_arg[out_count].value = pl_demo_make_value(curIdx, NULL, 0);
        out_count++;
    }

    if (curName)
    {
        out_arg[out_count].arg_name = URCNAME_HANDLE_FILENAME;
        out_arg[out_count].type = TYPE_RAW_GBK;
        out_arg[out_count].value = pl_demo_make_value(0, curName, strlen(curName));
        out_count++;
    }

    if (processing_rate >= 0)
    {
        out_arg[out_count].arg_name = URCNAME_HANDLE_RATE;
        out_arg[out_count].type = TYPE_INT;
        out_arg[out_count].value = pl_demo_make_value(processing_rate, NULL, 0);
        out_count++;
    }

    pl_demo_post_urc(menu, TYPE_URC_STATUS, out_arg, out_count, NULL, 0);

    pl_demo_release_value(out_arg[0].value);
    pl_demo_release_value(out_arg[1].value);
    pl_demo_release_value(out_arg[2].value);
    pl_demo_release_value(out_arg[3].value);
}

static inline unzFile unzip_open(char *path)
{
    return unzOpen64(path);
}

static inline int unzip_close(unzFile uf)
{
    return unzClose(uf);
}

ret_msg_t unzip_extract_curfile(unzFile uf, bool overwrite, const char *password, char *savepath, process_notify notify)
{
    int err = 0;
    ret_msg_t ret = {0};
    char *pathinzip = NULL;
    char *savepathname = NULL;
    char *needpath = NULL;
    const char *filename = NULL;
    unz_file_info64 zip_file_info = {0};
    SCFILE *fp = NULL;
    struct dirent_t file_info = {0};
    char *buf = NULL;
    char *p = NULL;
    int total_unziped_size = 0;
    int lastPercent = 0;
    int curPercent = 0;

    pathinzip = sal_malloc(SAL_256);
    savepathname = sal_malloc(SAL_256);
    needpath = sal_malloc(SAL_256);
    buf = sal_malloc(SAL_4K);
    if (!pathinzip || !savepathname || !needpath || !buf)
    {
        LOG_ERROR("malloc fail");
        ret.errcode = Z_MEM_ERROR;
        ret.msg = "malloc fail";
        goto EXIT;
    }

    ret.errcode = unzGetCurrentFileInfo64(uf, &zip_file_info, pathinzip, SAL_256, NULL, 0, NULL, 0);
    if (UNZ_OK != ret.errcode)
    {
        LOG_ERROR("get file info fail");
        ret.msg = "get file info fail";
        goto EXIT;
    }

    for (p = pathinzip + strlen(pathinzip); p > pathinzip && '/' != *p && '\\' != *p; p--);
    if ('/' == *p || '\\' == *p) filename = p + 1;
    else filename = p;

    memset(savepathname, 0, SAL_256);
    if (savepath[strlen(savepath)-1] == '/')
        snprintf(savepathname, SAL_256, "%s%s", savepath, pathinzip);
    else
        snprintf(savepathname, SAL_256, "%s/%s", savepath, pathinzip);

    memset(needpath, 0, SAL_256);
    strcpy(needpath, savepathname);
    for (p = needpath + strlen(needpath); p > needpath && '/' != *p && '\\' != *p; p--);
    while ('/' == *p || '\\' == *p) *p-- = 0;
    if (':' == *p) *needpath = 0;

    UNZIP_NOTIFY(notify, -1, -1, pathinzip, 0);

    err = sAPI_stat(savepathname, &file_info);
    if (strlen(filename) <= 0) // type is dir
    {
        if (0 != err)
        {
            LOG_TRACE("mkdir:[%s]", savepathname);
            sAPI_mkdir(savepathname, 666);
        }
    }
    else
    {
        if (0 == err)
        {
            LOG_TRACE("File [%s] exist!", savepathname);
            if (false == overwrite)
            {
                LOG_TRACE("Skip It!");
                goto EXIT;
            }
            else
            {
                LOG_TRACE("Replace It!");
            }
        }

        ret.errcode = unzOpenCurrentFilePassword(uf, password);
        if (UNZ_OK != ret.errcode)
        {
            LOG_ERROR("File in zip [%s] open fail", pathinzip);
            ret.msg = "File in zip open fail";
            goto EXIT;
        }

        if (0 != *needpath)
        {
            LOG_TRACE("Create dir: [%s]", needpath);
            sAPI_mkdir(needpath, 666);
        }

        fp = sAPI_fopen(savepathname, "wb");
        if (NULL == fp)
        {
            LOG_ERROR("File [%s] open fail", savepathname);
            ret.msg = "File open fail";
            goto EXIT;
        }

        do
        {
            err = unzReadCurrentFile(uf, buf, SAL_4K);
            if (err < 0)
            {
                LOG_ERROR("Read from zip fail. err=%d", err);
                ret.errcode = err;
                ret.msg = "Read from zip fail.";
                goto EXIT;
            }
            if (err > 0)
            {
                err = sAPI_fwrite(buf, 1, err, fp);
                if (err <= 0)
                {
                    LOG_ERROR("Write file fail");
                    ret.errcode = err;
                    ret.msg = "Write file fail";
                    goto EXIT;
                }
                total_unziped_size += err;
                curPercent = total_unziped_size * 100 / zip_file_info.uncompressed_size;
                if (curPercent - lastPercent >= 5)
                {
                    lastPercent = curPercent;
                    UNZIP_NOTIFY(notify, -1, -1, NULL, curPercent);
                }
            }
        } while (err > 0);

        sAPI_fclose(fp);
        ret.errcode = unzCloseCurrentFile(uf);
        if (ret.errcode != UNZ_OK)
        {
            LOG_ERROR("Close file in zip fail. err:%d", ret.errcode);
            ret.msg = "Close file in zip fail";
            goto EXIT;
        }
    }

    if (lastPercent < 100)
    {
        UNZIP_NOTIFY(notify, -1, -1, NULL, 100);
    }

EXIT:
    sal_free(pathinzip);
    sal_free(savepathname);
    sal_free(needpath);
    sal_free(buf);

    return ret;
}

ret_msg_t unzip_extract(unzFile uf, bool overwrite, const char *password, char *savepath, process_notify notify)
{
    ret_msg_t ret = {0};
    int i = 0;
    unz_global_info64 gi;

    // LOG_TRACE("extract file:");
    // LOG_TRACE("overwite: %d", overwrite);
    // LOG_TRACE("password: %s", password ? password : "NULL");
    // LOG_TRACE("savepath: %s", savepath);

    ret.errcode = unzGetGlobalInfo64(uf, &gi);
    if (ret.errcode != UNZ_OK)
    {
        LOG_ERROR("Zip info get fail. err:%d", ret.errcode);
        ret.msg = "Zip info get fail";
        goto EXIT;
    }
    UNZIP_NOTIFY(notify, gi.number_entry, -1, NULL, -1);

    LOG_TRACE("Files in zip:%d", gi.number_entry);
    do
    {
        i++;
        UNZIP_NOTIFY(notify, -1, i, NULL, -1);
        ret = unzip_extract_curfile(uf, overwrite, password, savepath, notify);
        if (ret.errcode != UNZ_OK)
        {
            goto EXIT;
        }

        ret.errcode = unzGoToNextFile(uf);
    } while (UNZ_OK == ret.errcode);

    if (ret.errcode != UNZ_OK && UNZ_END_OF_LIST_OF_FILE != ret.errcode)
    {
        LOG_ERROR("Go to next in zip fail. err:%d", ret.errcode);
        ret.msg = "Go to next in zip fail";
    }
    else
    {
        ret.errcode = 0;
    }

EXIT:
    return ret;
}

static ret_msg_t unzip_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *zippath = NULL;
    const char *savepath = NULL;
    const char *password = NULL;
    const char *replace = NULL;
    unzFile uf = NULL;

    pl_demo_get_data(&zippath, argv, argn, ARGZLIB_ZIPNAME);
    pl_demo_get_data(&savepath, argv, argn, ARGZLIB_SAVEPATH);
    pl_demo_get_data(&password, argv, argn, ARGZLIB_PASSWORD);
    pl_demo_get_data(&replace, argv, argn, ARGZLIB_REPLACE);

    if (true == pub_strcmpwithlen("0", password))
    {
        password = NULL;
    }

    uf = unzip_open((char *)zippath);
    if (uf != NULL)
    {
        ret = unzip_extract(uf, (strcmp(replace, ARGZLIB_REPLACE_TRUE) == 0), password, (char *)savepath, processing_notify);

        unzip_close(uf);
    }
    else
    {
        LOG_ERROR("zipfile open fail");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "zipfile open fail";
    }

    return ret;
}

static ret_msg_t zipWriteFile(zipFile zf, const char *pathName, const char *zipPathName, process_notify notify)
{
    ret_msg_t ret = {0};
    char *readBuf = NULL;
    SCFILE *fp = NULL;
    int readSize = 0;
    int fileSize = 0;
    int total_ziped_size = 0;
    int lastPercent = 0;
    int curPercent = 0;

    if (!zf || !pathName || !zipPathName)
    {
        ret.errcode = -1;
        goto EXIT;
    }

    LOG_INFO("zip file %s to %s", pathName, zipPathName);

    readBuf = sal_malloc(SAL_512);
    if (!readBuf)
    {
        LOG_ERROR("malloc fail");
        ret.errcode = -1;
        ret.msg = "malloc fail";
        goto EXIT;
    }

    fp = sAPI_fopen(pathName, "rb");
    if (!fp)
    {
        LOG_ERROR("file open fail");
        ret.errcode = -1;
        ret.msg = "file open fail";
        goto EXIT;
    }
    fileSize = sAPI_fsize(fp);

    ret.errcode = zipOpenNewFileInZip(zf, zipPathName, NULL, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_BEST_COMPRESSION);
    if (ZIP_OK != ret.errcode)
    {
        LOG_ERROR("open file in zip fail");
        ret.msg = "open file in zip fail";
        goto EXIT;
    }

    ZIP_NOTIFY(notify, -1, -1, pathName, 0);

    while ((readSize = sAPI_fread(readBuf, 1, SAL_512, fp)) > 0)
    {
        ret.errcode = zipWriteInFileInZip(zf, readBuf, readSize);
        if (ZIP_OK != ret.errcode)
        {
            LOG_ERROR("zip write fail, ret=%d", ret);
            ret.msg = "zip write fail";
            break;
        }
        total_ziped_size += readSize;
        curPercent = total_ziped_size * 100 / fileSize;
        if (curPercent - lastPercent >= 5)
        {
            lastPercent = curPercent;
            ZIP_NOTIFY(notify, -1, -1, NULL, curPercent);
        }
    }

    sAPI_fclose(fp);
    fp = NULL;
    if (zipCloseFileInZip(zf) != ZIP_OK)
    {
        LOG_ERROR("close file in zip fail");
        ret.errcode = -1;
        ret.msg = "close file in zip fail";
    }

    if (readSize < 0)
    {
        LOG_ERROR("file:%s read fail, ret=%d", pathName, readSize);
        ret.errcode = -1;
        ret.msg = "file read fail";
    }

    if (lastPercent < 100)
    {
        ZIP_NOTIFY(notify, -1, -1, NULL, 100);
    }

EXIT:
    sal_free(readBuf);
    if (fp)
    {
        sAPI_fclose(fp);
        fp = NULL;
    }

    return ret;
}

static ret_msg_t zipProcessFiles(zipFile zf, const char *dirPath, const char *basepath, process_notify notify)
{
    ret_msg_t ret = {0};
    SCDIR *dir_hp = NULL;
    struct dirent_t *file_info = NULL, info = {0};
    char *filePath = sal_malloc(SAL_256);
    char *basePathCur = sal_malloc(SAL_256);
    int count = 0;

    if (!zf || !dirPath || !basepath)
    {
        ret.errcode = -1;
        LOG_ERROR("arg invalid");
        goto EXIT;
    }

    LOG_INFO("zip %s to %s", dirPath, basepath);

    if (!filePath || !basePathCur)
    {
        LOG_ERROR("malloc fail");
        ret.errcode = -1;
        ret.msg = "malloc fail";
        goto EXIT;
    }

    if (':' != dirPath[strlen(dirPath) - 1])
    {
        ret.errcode = sAPI_stat(dirPath, &info);
        if (0 != ret.errcode)
        {
            LOG_ERROR("stat fail, ret=%d", ret);
            ret.msg = "stat fail";
            goto EXIT;
        }

        snprintf(basePathCur, SAL_256, "%s/%s", basepath, info.name);
    }
    else
    {
        info.type = TYPE_DIR;
        snprintf(basePathCur, SAL_256, "%s", basepath);
    }
    if (TYPE_REG == info.type)
    {
        ret = zipWriteFile(zf, dirPath, basePathCur, notify);
    }
    else
    {
        dir_hp = sAPI_opendir(dirPath);
        if (!dir_hp)
        {
            LOG_ERROR("dir open fail");
            ret.errcode = -1;
            ret.msg = "dir open fail";
            goto EXIT;
        }

        count = 0;
        while ((file_info = sAPI_readdir(dir_hp)) != NULL)
        {
            memset(filePath, 0, SAL_256);
            memset(basePathCur, 0, SAL_256);
            snprintf(filePath, SAL_256, "%s/%s", dirPath, file_info->name);
            if (strlen(basepath) > 0)
            {
                snprintf(basePathCur, SAL_256, "%s/%s", basepath, file_info->name);
            }
            else
            {
                snprintf(basePathCur, SAL_256, "%s", file_info->name);
            }

            if (TYPE_REG == file_info->type)
            {
                count++;
                ret = zipWriteFile(zf, filePath, basePathCur, notify);
            }
            else if (TYPE_DIR == file_info->type)
            {
                ret = zipProcessFiles(zf, filePath, basePathCur, notify);
            }

            if (0 != ret.errcode)
            {
                break;
            }
        }
        if (0 == count)
        {
            zipOpenNewFileInZip(zf, basePathCur, NULL, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_BEST_COMPRESSION);
            zipCloseFileInZip(zf);
        }

        sAPI_closedir(dir_hp);
    }

EXIT:
    sal_free(filePath);
    sal_free(basePathCur);

    return ret;
}

static ret_msg_t zip_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int mode = 0;
    const char *zip_name = NULL;
    const char *pathname = NULL;
    const char *mode_str = NULL;
    const char *p = NULL;
    struct dirent_t info = {0};
    char *ep = NULL;
    char diskPath[2] = {0};

    pl_demo_get_data(&zip_name, argv, argn, ARGZLIB_ZIPNAME);
    pl_demo_get_data(&mode_str, argv, argn, ARGZLIB_ZIPMODE);
    pl_demo_get_data(&pathname, argv, argn, ARGZLIB_SRCNAME);
    if (!zip_name)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }

    LOG_INFO("zip mode:%s", mode_str);
    if (0 == strcmp(mode_str, ARGZLIB_ZIPNEW))
    {
        mode = 0;
    }
    else
    {
        mode = 2;
    }

    ep = (char *)pathname + strlen(pathname) - 1;
    for (; '/' == *ep || '\\' == *ep; ep--) *ep = 0;

    if (!(strlen(pathname) == 2 && pathname[1] == ':'))
    {
        ret.errcode = sAPI_stat(pathname, &info);
        if (0 != ret.errcode)
        {
            LOG_ERROR("stat fail, ret=%d", ret);
            ret.msg = "stat fail";
            goto EXIT;
        }
        if (TYPE_DIR == info.type)
        {
            for (p = pathname + strlen(pathname);
                 p > pathname && *p != '/' && *p != '\\';
                 p--);
            if ('/' == *p || '\\' == *p) p++;
        }
        else
        {
            p = "";
        }
    }
    else
    {
        diskPath[0] = pathname[0];
        diskPath[1] = 0;
        p = diskPath;
    }

    zipFile zf = zipOpen(zip_name, mode);
    if (zf != NULL)
    {
        ret = zipProcessFiles(zf, pathname, p, processing_notify);
        zipClose(zf, NULL);
    }
    else
    {
        LOG_ERROR("create zip fail");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "create zip fail";
    }

EXIT:
    return ret;
}
