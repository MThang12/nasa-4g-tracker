/*
 * @Author: Louis_Qiu
 * @Date: 2022-12-27 11:10:57
 * @Last Modified by: Louis_Qiu
 * @Last Modified time: 2022-12-28 09:42:00
 */

#include "string.h"
#include "stdlib.h"
#include "stdio.h"

#include "unzip.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "uart_api.h"
#include "zip.h"
#include "simcom_file.h"

void ui_print(const char *format, ...);
extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);
extern void PrintfRespData(char *buff, UINT32 length);
extern void PrintfRespHexData(char *buff, UINT32 length);

extern int do_extract(unzFile uf, int opt_extract_without_path, int opt_overwrite, const char *password);
extern unzFile do_open(char *path);
extern int do_close(unzFile uf);



typedef enum
{
    SC_ZLIB_UNZIP                 = 1,
    SC_ZLIB_ZIP                   = 2,
    SC_ZLIB_EXIT                  = 99
} SC_ZLIB_DEMO_TYPE;

int zipwritefile(zipFile zf,char *filename)
{
    UINT32 ret = 0;
    SCFILE *file_hdl = NULL;
    unsigned long buff_data_len = 0;
    unsigned long receive_len = 0;
    unsigned long actul_read_len = 0;
    char buff[2048] = {0};
    sAPI_Debug("zipwritefile filename is %s ", filename);
    int err = zipOpenNewFileInZip(zf, &filename[3], NULL, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_BEST_COMPRESSION);
    if(err == ZIP_OK)
    {
        file_hdl = sAPI_fopen(filename, "rb");
        buff_data_len = sAPI_fsize(file_hdl);
        while(receive_len < buff_data_len)
        {
            memset(buff, 0, 2048);
            actul_read_len = sAPI_fread(buff,2048, 1, file_hdl);
            if(actul_read_len <= 0)
            {
                sAPI_Debug("sAPI_FsFileRead err,data: %s, len: %d", buff, actul_read_len);
                ret = sAPI_fclose(file_hdl);
                if (ret != 0)
                {
                    ui_print("close %s fail", filename);
                    break;
                }
                else
                {
                    file_hdl = NULL;
                }
            }
            sAPI_Debug("read data: [%s], len: %d", buff, actul_read_len);
            receive_len += actul_read_len;
            err = zipWriteInFileInZip(zf,buff,actul_read_len);
        }
        ret = sAPI_fclose(file_hdl);
        if (ret != 0)
        {
            ui_print("close %s fail", filename);
        }
        else
        {
            file_hdl = NULL;
        }
        err = zipCloseFileInZip(zf);
    }
    return err;
}

int scAddSpritInPathTail(char *path)
{
    if(path == NULL)
        return -1;

    unsigned int len = strlen(path);

    if (path[len - 1] != '/')
    {
        path[len] = '/';
        path[len + 1] = 0;
        return 0;
    }
    else
    {
        return -1;
    }
}

char *scStrcatHead(char *dest, char *src)
{
    if (dest == NULL) return NULL;

    char *tem = NULL;

    tem = (char *)sAPI_Malloc(strlen(dest) + 1);
    if (tem == NULL) return NULL;

    strcpy(tem, dest);

    sprintf(dest, "%s%s", src, tem);

    sAPI_Free(tem);

    return dest;
}

void ZLIBDemo(void)
{
    UINT32 opt = 0;

    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] =
    {
        "1. unzip",
        "2. zip",
        "99. back"
    };

    while (1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list, sizeof(options_list) / sizeof(options_list[0]));

        opt = UartReadValue();

        switch (opt)
        {
            case SC_ZLIB_UNZIP:
            {
                char path[512] = {0};
                PrintfResp("\r\nPlease input unzip path.\r\n");
                UartReadLine(path, sizeof(path));
                sAPI_Debug("zlib unzip: path is %s ", path);

                unzFile uf = NULL;
                int err;

                uf = do_open(path);
                if (uf != NULL)
                {
                    err = do_extract(uf, 0, 0, NULL);
                    sAPI_Debug("ret_value[%d]", err);
                    do_close(uf);
                }
                else
                {
                    sAPI_Debug("unzip open fail!!\r\n");
                }
            }
            break;

            case SC_ZLIB_ZIP:
            {
                UINT32 ret = 0;
                char zip_name[100] = {0};
                char dir_name[100] = {0};
                SCDIR *dir_hdl = NULL;
                struct dirent_t *info_dir = NULL;
                int mode;
                ui_print("\r\nPlease input the name of zip :\r\n");
                UartReadLine(zip_name, 100);
                sAPI_Debug("zlib zip: zipname is %s ", zip_name);
                ui_print("\r\nPlease input zip mode(0-2):\r\n");
                mode = UartReadValue();
                sAPI_Debug("zlib zip: mode is %d ", mode);
                ui_print("\r\nPlease enter the directory you want to zip:\r\n");
                UartReadLine(dir_name, 100);
                sAPI_Debug("zlib zip dirname is %s ", dir_name);

                zipFile zf = zipOpen(zip_name,mode);
                sAPI_Debug("zip mode:%d",mode);
                if(zf != NULL)
                {
                    dir_hdl = sAPI_opendir(dir_name);
                    while ((info_dir = sAPI_readdir(dir_hdl)) != NULL)
                    {
                        scAddSpritInPathTail(dir_name);
                        scStrcatHead(info_dir->name,dir_name);
                        if(info_dir->type == 1)//file
                        {
                            sAPI_Debug("zlib zip filename is %s ,info_dir->type = %d", info_dir->name,info_dir->type);
                            int result = zipwritefile(zf,info_dir->name);
                            if(result != ZIP_OK)
                            {
                                ui_print("add file in zip fail");
                                sAPI_Debug("add file in zip fail");
                                zipClose(zf,NULL);
                                break;
                            }
                        }
                        else if((int)info_dir->type == 2)//directory
                        {
                            scAddSpritInPathTail(info_dir->name);
                            sAPI_Debug("zlib zip pathname is %s ,info_dir->type = %d", info_dir->name,info_dir->type);
                            int err = zipOpenNewFileInZip(zf, &info_dir->name[3], NULL, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_BEST_COMPRESSION);
                            if(err == ZIP_OK)
                                zipCloseFileInZip(zf);
                            else
                            {
                                ui_print("add file in zip fail");
                                sAPI_Debug("add file in zip fail");
                                zipCloseFileInZip(zf);
                                zipClose(zf,NULL);
                                break;
                            }
                        }
                    }
                    ret = sAPI_closedir(dir_hdl);
                    if (ret != 0)
                    {
                        ui_print("close %s fail[%d]", dir_name, ret);
                        break;
                    }
                    zipClose(zf,NULL);
                    ui_print("zip succ");
                    sAPI_Debug("zip succ");
                }
                else{
                    ui_print("create zip fail");
                    sAPI_Debug("create zip fail");
                }
            }
            break;

            case SC_ZLIB_EXIT:
            {
                return;
            }
            break;

            default:
                break;
        }
    }
}
