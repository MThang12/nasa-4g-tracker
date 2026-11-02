/**
  ******************************************************************************
  * @file    demo_file_system.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of file system operation.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "stdarg.h"

#include "simcom_file.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "simcom_os.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "uart_api.h"

extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);


static inline void ui_print(const char *format, ...)
{
    char tmpstr[200];
    va_list args;

    memset(tmpstr, 0, sizeof(tmpstr));
    va_start(args, format);
    vsnprintf(tmpstr, sizeof(tmpstr), format, args);
    va_end(args);
    PrintfResp("\r\n");
    PrintfResp(tmpstr);
    PrintfResp("\r\n");
}

typedef enum
{
    SC_FS_DEMO_WRITEFILE = 1,
    SC_FS_DEMO_READFILE,
    SC_FS_DEMO_DELETEFILE,
    SC_FS_DEMO_RENAMEFILE,
    SC_FS_DEMO_GETDISKSIZE,
    SC_FS_DEMO_MAKEDIR,
    SC_FS_DEMO_REMOVEDIR,
    SC_FS_DEMO_OPENDIR,
    SC_FS_DEMO_FORANT_DIR,
    SC_FS_DEMO_RXFILE,
    SC_FS_DEMO_TXFILE,
    SC_FS_DEMO_MAX
} SC_FS_DEMO_TYPE;

typedef enum
{
    SC_DATA_READ,
    SC_DATA_WRITE
} SC_RX_DATA_OP;

#define FALSE 0
#define TRUE 1

#define BUFF_LEN 100
#define MAX_OUTPUT_LEN 512
#define MAX_VALID_USER_NAME_LENGTH  255-7

#define RECV_LEN 50
#define FLASH_PAGE_SIZE (2048) //2k
#define FLASH_BLOCK_SIZE (FLASH_PAGE_SIZE * 64) //128k
#define READ_FILE_PACKET_SIZE FLASH_BLOCK_SIZE //128k
#define SIM_FILE_OUTPUT_SIZE 512
#define RECEIVE_FLAG_MASK_SYNC (0x01 << 0)
#define RECEIVE_FLAG_MASK_SUCCESS (0x01 << 1)
#define RECEIVE_FLAG_MASK_FAIL (0x01 << 2)
#define RECV_DATA_BUFFER_SIZE (1024 * 20)  // 20k
#define SC_RECV_FILE_TASK_PRIORITY (150)

char cur_path_in[MAX_VALID_USER_NAME_LENGTH] = "C:/";  // default path
static int gfile_len_total = 0;
static BOOL gbFileTransmitting = FALSE;
static char gpReceiveFileDes[MAX_VALID_USER_NAME_LENGTH + 1] = {0};
//static UINT8 gFsRecvTaskStack[SC_DEFAULT_THREAD_STACKSIZE];
static sTaskRef gFsHandleTask = NULL;
static char *gFsHandleTaskStack;
static char *pRXDataBuffer = NULL;
static char *pDataR = NULL, *pDataW = NULL;
static unsigned int rxDataFreeSize = 0;
static sMutexRef gReceiveDataMutexRef = NULL;
static sFlagRef gReceiveDataPreparedFlagRef = NULL;
static BOOL bFail = FALSE;



static int sReceiveDataRW(SC_RX_DATA_OP op, char *pData, int len)
{
    char *p = pData;
    int lessLen = len;
    int opLen = len;
    int tailSize = 0;

    sAPI_Debug("FILE SYSTEM op: %d", op);

    if ((NULL == pRXDataBuffer) || (NULL == pData) || (len <= 0))
    {
        sAPI_Debug("FILE SYSTEM parameter invalid!! pRXDataBuffer: %p, pData: %p, len: %d", pRXDataBuffer, pData, len);
        return -1;
    }

    sAPI_MutexLock(gReceiveDataMutexRef, SC_SUSPEND);
    switch (op)
    {
        case SC_DATA_READ:
        {
            opLen = (len < (RECV_DATA_BUFFER_SIZE - rxDataFreeSize)) ? len : (RECV_DATA_BUFFER_SIZE - rxDataFreeSize);
            lessLen = opLen;
            rxDataFreeSize += opLen;
            sAPI_Debug("FILE SYSTEM test buff free size[%d]", rxDataFreeSize);

            if ((opLen == 0) && (pDataR == pDataW))
            {
                sAPI_Debug("FILE SYSTEM the buffer is empty!! opLen[%d]", opLen);
                opLen = 0;
                break;
            }

            if ((pDataR < pDataW)
                || ((RECV_DATA_BUFFER_SIZE == opLen) && (pDataR == pRXDataBuffer)))
            {
                memcpy(pData, pDataR, opLen);
                if (RECV_DATA_BUFFER_SIZE != opLen)
                {
                    pDataR += opLen;
                }
            }
            else if ((pDataR > pDataW)
                     || ((opLen == RECV_DATA_BUFFER_SIZE) && (pDataR != pRXDataBuffer)))
            {
                tailSize = pRXDataBuffer + RECV_DATA_BUFFER_SIZE - pDataR;
                memcpy(p, pDataR, tailSize);

                p += tailSize;
                lessLen -= tailSize;
                memcpy(p, pRXDataBuffer, lessLen);
                if (RECV_DATA_BUFFER_SIZE != opLen)
                {
                    pDataR = pRXDataBuffer + lessLen;
                }
            }
            else
            {
                opLen = -1;

                sAPI_Debug("FILE SYSTEM buffer operation err!!");
                break;  // necessary
            }

            if (rxDataFreeSize < RECV_DATA_BUFFER_SIZE)
            {
                opLen += 1;
            }
        }
        break;

        case SC_DATA_WRITE:
        {
            /*free buff size less than this len*/
            if (rxDataFreeSize < len)
            {
                opLen = -1;

                sAPI_Debug("FILE SYSTEM buffer is full!!");
                break;
            }
            else
            {
                rxDataFreeSize -= len; /*free buff size minus this length*/
            }

            tailSize = pRXDataBuffer + RECV_DATA_BUFFER_SIZE - pDataW;
            if (tailSize >= len)
            {
                memcpy(pDataW, pData, len);

                pDataW += len;
                if ((pRXDataBuffer + RECV_DATA_BUFFER_SIZE) == pDataW)
                {
                    pDataW = pRXDataBuffer;
                }
            }
            else
            {
                memcpy(pDataW, p, tailSize);

                p += tailSize;
                lessLen -= tailSize;
                memcpy(pRXDataBuffer, p, lessLen);
                pDataW = pRXDataBuffer + lessLen;
            }
            //sAPI_Free(p);
        }
        break;

        default:
        {
            opLen = -1;
            sAPI_Debug("FILE SYSTEM invalid operation!!");
        }
        break;
    }

    sAPI_Debug("FILE SYSTEM cacked data: %d", (RECV_DATA_BUFFER_SIZE - rxDataFreeSize));

    if ((RECV_DATA_BUFFER_SIZE - rxDataFreeSize) > (1024))
    {
        // if buffer has had data more than 1K
        sAPI_Debug("FILE SYSTEM SET flag!!!!");
        sAPI_FlagSet(gReceiveDataPreparedFlagRef, RECEIVE_FLAG_MASK_SYNC, SC_FLAG_OR);
    }
    sAPI_MutexUnLock(gReceiveDataMutexRef);

    return opLen;
}

/**
  * @brief  File system receiving process
  * @param  void
  * @note   This demo show how to process files sent to module
  * @retval void
  */
void sRecvProcesser(void)
{
    int single_len;
    unsigned int receivedLength = 0;

    int ret = -1;
    char buff[RECV_LEN] = {0};
    int datalen = 0;

    ui_print("\r\nPlease input full data:\r\n");

    while (1)
    {
        datalen = gfile_len_total - receivedLength > RECV_LEN ? RECV_LEN : gfile_len_total - receivedLength;
        if(datalen > 0)
        {
            //buff = (char *)sAPI_Malloc(datalen);
            memset(buff, 0, datalen);
            single_len = UartRead(buff, datalen,1);
            receivedLength += single_len;
            sAPI_Debug("FILE SYSTEM single_recv_len[%d],total_recv_len[%d]", single_len, receivedLength);

            if (!bFail)
            {
                ret = sReceiveDataRW(SC_DATA_WRITE, buff, single_len);
                if ((ret < 0) || (ret != single_len) || (receivedLength > gfile_len_total))
                {
                    bFail = TRUE;
                    sAPI_Debug("FILE SYSTEM write data fail!!");
                }
            }

            if (receivedLength >= gfile_len_total)
            {
                if (receivedLength > gfile_len_total)
                {
                    sAPI_Debug("FILE SYSTEM total_recv_len[%d] > gfile_len_total[%d]", receivedLength, gfile_len_total);
                    bFail = TRUE;
                }

                if (!bFail)
                {
                    sAPI_FlagSet(gReceiveDataPreparedFlagRef, RECEIVE_FLAG_MASK_SUCCESS, SC_FLAG_OR);
                }
                else
                {
                    sAPI_FlagSet(gReceiveDataPreparedFlagRef, RECEIVE_FLAG_MASK_FAIL, SC_FLAG_OR);
                }
                return;
            }
        }
        else
        {
            break;
        }
    }
}

/**
  * @brief  File system handle process
  * @param  void
  * @note   This demo show how to process files sent to module
  * @retval void
  */
void sTask_FsHandeProcesser(void *argv)
{
    unsigned int bufferLength = 0;
    unsigned int bufferLengthFree = 0;
    char *dataBuffer = NULL;
    char *p = NULL;
    unsigned int totalWriteLen = 0;
    SCFILE *handle = NULL;
    UINT32 flags = 0;
    BOOL bSuccessAndCleared = TRUE;
    int ret = -1;
    UINT32 actualWriteLen;
    //char resp[50] ={0};

    while (1)
    {
        if (sAPI_FlagWait(gReceiveDataPreparedFlagRef,
                          RECEIVE_FLAG_MASK_FAIL | RECEIVE_FLAG_MASK_SYNC | RECEIVE_FLAG_MASK_SUCCESS,
                          SC_FLAG_OR_CLEAR, &flags, SC_SUSPEND) != SC_SUCCESS)
        {
        }
        sAPI_Debug("FILE SYSTEM FLAG: %#x", flags);

        if (gbFileTransmitting != TRUE)
        {
            continue;
        }

        if (!bFail)
        {
            if (NULL == handle)
            {
                handle = sAPI_fopen(gpReceiveFileDes, "wb");
                if (handle == NULL)
                {
                    bFail = TRUE;
                    sAPI_Debug("FS creat file fail!!");
                }
            }

            if (NULL == dataBuffer)
            {
                bufferLength = ((gfile_len_total < READ_FILE_PACKET_SIZE) ? gfile_len_total : READ_FILE_PACKET_SIZE);

                if ((dataBuffer = (char *)sAPI_Malloc(bufferLength)) == NULL)
                {
                    bFail = TRUE;
                    sAPI_Debug("FILE SYSTEM creat buffer fail!!");
                }

                memset(dataBuffer, 0, bufferLength);
                p = dataBuffer;
                bufferLengthFree = bufferLength;
            }

            do
            {
                bSuccessAndCleared = TRUE;
                sAPI_Debug("FILE SYSTEM test bufferLengthFree[%d]", bufferLengthFree);

                ret = sReceiveDataRW(SC_DATA_READ, p, bufferLengthFree);
                if (ret < 0)
                {
                    bFail = TRUE;
                    sAPI_Debug("FILE SYSTEM read data err!!");
                }
                else if (ret > bufferLengthFree)
                {
                    // the cache has data not clear
                    ret -= 1;
                    if ((flags & RECEIVE_FLAG_MASK_SUCCESS) == RECEIVE_FLAG_MASK_SUCCESS)
                    {
                        bSuccessAndCleared = FALSE;
                    }
                }

                p += ret;
                bufferLengthFree -= ret;

                if ((ret >= 0)
                    && ((0 == bufferLengthFree)
                        || ((flags & RECEIVE_FLAG_MASK_SUCCESS) && (TRUE == bSuccessAndCleared)
                            && (bufferLengthFree < bufferLength))
                        || ((flags & RECEIVE_FLAG_MASK_FAIL) && (bufferLengthFree < bufferLength))))
                {
                    sAPI_Debug("FILE SYSTEM write before");
                    actualWriteLen = sAPI_fwrite(dataBuffer, 1, (bufferLength - bufferLengthFree), handle);
                    if (actualWriteLen != (bufferLength - bufferLengthFree))
                    {
                        bFail = TRUE;
                        sAPI_Debug("FILE SYSTEM write file fail!! actual write len: %ld", actualWriteLen);
                    }
                    sAPI_Debug("FILE SYSTEM write after");

                    totalWriteLen += actualWriteLen;

                    sAPI_Free(dataBuffer);
                    dataBuffer = NULL;

                    bufferLengthFree = bufferLength;
                    sAPI_Debug("FILE SYSTEM plan_file_total = %d; totalWriteLen = %d.\n", gfile_len_total, totalWriteLen);
                }
            } while (FALSE == bSuccessAndCleared);
        }

        if ((flags & RECEIVE_FLAG_MASK_SUCCESS) || (flags & RECEIVE_FLAG_MASK_FAIL))
        {
            if (totalWriteLen != gfile_len_total)
            {
                bFail = TRUE;
                sAPI_Debug("FILE SYSTEM data count err!!");
            }

            if (bFail)
            {
                sAPI_Debug("FILE SYSTEM receive err,plan_file_total = %d; totalWriteLen = %d.\n", gfile_len_total, totalWriteLen);

                if (NULL != handle)
                {
                    sAPI_fclose(handle);
                    handle = NULL;
                    sAPI_remove((const char *)gpReceiveFileDes);
                }

                ui_print("\r\nFILE SYSTEM transfer file fail\r\n");
            }
            else
            {
                int ret = 0;
                sAPI_Debug("FILE SYSTEM recv succ, plan_file_total = %d; totalWriteLen = %d.\n", gfile_len_total, totalWriteLen);

                sAPI_fclose(handle);
                handle = NULL;
                //ret = sAPI_WriteBack();
                sAPI_Debug(" sAPI_WriteBack ret = %d.\n", ret);
                ui_print("\r\nFILE SYSTEM transfer file succ\r\n");
            }
            gfile_len_total = 0;
            totalWriteLen = 0;

            sAPI_Free(pRXDataBuffer);
            pRXDataBuffer = NULL;

            pDataR = NULL;
            pDataW = NULL;
            rxDataFreeSize = 0;
            gbFileTransmitting = FALSE;

            sAPI_FlagWait(gReceiveDataPreparedFlagRef,
                          RECEIVE_FLAG_MASK_FAIL | RECEIVE_FLAG_MASK_SYNC | RECEIVE_FLAG_MASK_SUCCESS,
                          SC_FLAG_OR_CLEAR,
                          &flags,
                          SC_NO_SUSPEND);
        }
    }
}

/**
  * @brief  File system demo operation
  * @param  void
  * @note   This demo will show how to operate file system on ASR1603 based module, such as write\read\seek ,etc
  * @retval void
  */
void FsDemo(void)
{
    SCFILE *file_hdl = NULL;
    SCDIR *dir_hdl = NULL;
    UINT32 ret = 0;
    //char buff[MAX_OUTPUT_LEN] = {0};

    unsigned long buff_data_len = 0;
    int actul_write_len = 0;
    int actul_read_len = 0;
    INT64 total_size = 0;
    INT64 free_size = 0;
    INT64 used_size = 0;
    struct dirent_t *info_dir = NULL;
    struct dirent_t info_file = {0};

    char tx_file_name[MAX_VALID_USER_NAME_LENGTH] = {0};
    //    char del_name[MAX_VALID_USER_NAME_LENGTH] = {0};
    //    char operation_path[MAX_VALID_USER_NAME_LENGTH]={0};
    char  pTemBuffer[MAX_OUTPUT_LEN * 3];
    //    char *dir_name = "D:/";
    char *file_name = "D:/test_file.txt";
    UINT32 opt = 0;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] =
    {
        "1. Write file",
        "2. Read file",
        "3. Delete file",
        "4. Rename file",
        "5. Get disk size",
        "6. Make directory",
        "7. Remove directory",
        "8. List file",
        "9. Format drive D:",
        "10. Transfer file in",
        "11. Transfer file in",
        "99. Back",
    };

    while (1)
    {
        ui_print("%s", note);
        PrintfOptionMenu(options_list, sizeof(options_list) / sizeof(options_list[0]));

        opt = UartReadValue();
        sAPI_Debug("opt %d", opt);

        switch (opt)
        {
            case SC_FS_DEMO_FORANT_DIR:
            {
                char drive[10] = {0};
                ui_print("\r\nPlease input format drive\r\n");
                UartReadLine(drive, 10);
                ui_print("drive: [%s] ", drive);
                ret = sAPI_fformat(drive);
                if (0 == ret)
                {
                    ui_print("\r\nFormat success!\r\n");
                }
                else
                {
                    ui_print("\r\nFormat fail!\r\n");
                    break;
                }

                char dir_name[100];
                ui_print("\r\nPlease input dir\r\n");
                UartReadLine(dir_name, 100);
                dir_hdl = sAPI_opendir(dir_name);
                if (dir_hdl == NULL)
                {
                    ui_print("open %s fail", dir_name);
                    break;
                }
                ui_print("open %s success", dir_name);

                while ((info_dir = sAPI_readdir(dir_hdl)) != NULL)
                {
                    ui_print("name[%s]-size[%ld]-type[%d]\r\n", info_dir->name, info_dir->size, info_dir->type);
                }

                ret = sAPI_closedir(dir_hdl);
                if (ret != 0)
                {
                    ui_print("close %s fail[%d]", dir_name, ret);
                    break;
                }
                break;
            }
            case SC_FS_DEMO_WRITEFILE:
            {
                char write_str[1000] = {0};
                char fileName[100];
                ui_print("\r\nPlease input fileName\r\n");
                UartReadLine(fileName, 100);
                ui_print("\r\nPlease input data\r\n");
                UartReadLine(write_str, 1000);
                file_hdl = sAPI_fopen(fileName, "wb+");
                if (file_hdl == NULL)
                {
                    ui_print("open %s fail", fileName);
                    break;
                }
                buff_data_len = strlen(write_str);
                actul_write_len = sAPI_fwrite(write_str, 1, buff_data_len, file_hdl);
                if (actul_write_len != buff_data_len)
                {
                    sAPI_fclose(file_hdl);
                    ui_print("%s write fail [%d]", fileName, actul_write_len);
                    break;
                }
                ret = sAPI_fclose(file_hdl);
                if (ret != 0)
                {
                    ui_print("%s close fail [%d]", fileName, ret);
                    break;
                }
                else
                {
                    file_hdl = NULL;
                }
                break;
            }
            case SC_FS_DEMO_READFILE:
            {
                int position;
                char fileName[100];
                char read_buffer[MAX_OUTPUT_LEN];
                ui_print("\r\nPlease input fileName\r\n");
                UartReadLine(fileName, 100);
                file_hdl = sAPI_fopen(fileName, "rb");
                if (file_hdl == NULL)
                {
                    ui_print("open %s fail", fileName);
                    break;
                }

                buff_data_len = sAPI_fsize(file_hdl);
                ui_print("sAPI_fsize buff_data_len: %d",  buff_data_len);

                buff_data_len = (buff_data_len <= MAX_OUTPUT_LEN) ? buff_data_len : MAX_OUTPUT_LEN;
                memset(read_buffer, 0, MAX_OUTPUT_LEN);
                actul_read_len = sAPI_fread(read_buffer, 1, buff_data_len, file_hdl);
                if (actul_read_len <= 0)
                {
                    sAPI_fclose(file_hdl);
                    ui_print("%s read fail [%d]", fileName, actul_read_len);
                    break;
                }
                ui_print("read len: %d,data: %s", actul_read_len, read_buffer);
                sAPI_fseek(file_hdl, 1, 0);
                position = sAPI_ftell(file_hdl);
                ui_print("postion: %d", position);
                memset(read_buffer, 0, MAX_OUTPUT_LEN);
                actul_read_len = sAPI_fread(read_buffer, 1, buff_data_len, file_hdl);
                ui_print("read len: %d,data: %s", actul_read_len, read_buffer);
                ret = sAPI_fclose(file_hdl);
                if (ret != 0)
                {
                    ui_print("close %s fail", file_name);
                    break;
                }
                else
                {
                    file_hdl = NULL;
                }
                break;
            }
            case SC_FS_DEMO_DELETEFILE:
            {
                char fileName[100];
                ui_print("\r\nPlease input fileName\r\n");
                UartReadLine(fileName, 100);
                ret = sAPI_stat(fileName, &info_file);
                if (ret != 0)
                {
                    ui_print("sAPI_stat fail [%d]", ret);
                    break;
                }
                ui_print("[%s]-size[%d]-type[%d]", info_file.name, info_file.size, info_file.type);
                ret = sAPI_remove(fileName);
                if (ret != 0)
                {
                    ui_print("remove %s fail [%d]", fileName, ret);
                    break;
                }
                ui_print("remove %s sucess", fileName);
                ret = sAPI_stat(fileName, &info_file);
                if (ret != 0)
                {
                    ui_print("not find file");
                    break;
                }
                ui_print("[%s]-size[%d]-type[%d]", info_file.name, info_file.size, info_file.type);

                break;
            }
            case SC_FS_DEMO_RENAMEFILE:
            {
                char fileName[100];
                char new_name[100];
                ui_print("\r\nPlease input fileName\r\n");
                UartReadLine(fileName, 100);
                ui_print("\r\nPlease input new fileName\r\n");
                UartReadLine(new_name, 100);
                ret = sAPI_stat(fileName, &info_file);
                if (ret != 0)
                {
                    ui_print("sAPI_stat fail [%d]", ret);
                    break;
                }
                ui_print("[%s]-size[%d]-type[%d]", info_file.name, info_file.size, info_file.type);
                ret = sAPI_stat(new_name, &info_file);
                if (ret != 0)
                {
                    ui_print("not find file");
                }
                else
                {
                    ui_print("[%s]-size[%d]-type[%d]", info_file.name, info_file.size, info_file.type);
                    break;
                }
                ret = sAPI_rename(fileName, new_name);
                if (ret != 0)
                {
                    ui_print("rename %s fail [%d]", fileName, ret);
                    break;
                }
                ui_print("rename %s sucess", new_name);
                ret = sAPI_stat(fileName, &info_file);
                if (ret != 0)
                {
                    ui_print("not find file");
                }
                ret = sAPI_stat(new_name, &info_file);
                if (ret != 0)
                {
                    ui_print("sAPI_stat fail [%d]", ret);
                    break;
                }
                ui_print("[%s]-size[%d]-type[%d]", info_file.name, info_file.size, info_file.type);

                break;
            }
            case SC_FS_DEMO_GETDISKSIZE:
            {
                int disk = 0;
                ui_print("\r\nPlease input disk:0 is C:/ and 1 is D:/\r\n");
                disk = UartReadValue();
                if (disk)
                {
                    total_size = sAPI_GetSize("D:/");
                    free_size = sAPI_GetFreeSize("D:/");
                    used_size = sAPI_GetUsedSize("D:/");
                }
                else
                {
                    total_size = sAPI_GetSize(cur_path_in);
                    free_size = sAPI_GetFreeSize(cur_path_in);
                    used_size = sAPI_GetUsedSize(cur_path_in);
                }
                ui_print("total_size= %lld, free_size= %lld,used_size = %lld", total_size, free_size, used_size);

                break;
            }
            case SC_FS_DEMO_MAKEDIR:
            {
                char dir_name[100];
                ui_print("\r\nPlease input dir\r\n");
                UartReadLine(dir_name, 100);
                ret = sAPI_mkdir(dir_name, 0);
                if (ret != 0)
                {
                    ui_print("creat %s fail [%d]", dir_name, ret);
                    break;
                }
                ui_print("creat %s sucess", dir_name);
                ret = sAPI_stat(dir_name, &info_file);
                if (ret != 0)
                {
                    ui_print("get %s info fail[%d]", dir_name, ret);
                    break;
                }
                ui_print("[%s]-[%ld]-[%d]", info_file.name, info_file.size, info_file.type);
                break;
            }
            case SC_FS_DEMO_REMOVEDIR:
            {
                char dir_name[100];
                ui_print("\r\nPlease input dir\r\n");
                UartReadLine(dir_name, 100);
                ret = sAPI_remove(dir_name);
                if (ret != 0)
                {
                    ui_print("delte %s fail [%d]", dir_name, ret);
                    break;
                }
                ui_print("delte %s success", dir_name);
                break;
            }
            case SC_FS_DEMO_OPENDIR:
            {
                char dir_name[100];
                ui_print("\r\nPlease input dir\r\n");
                UartReadLine(dir_name, 100);
                dir_hdl = sAPI_opendir(dir_name);
                if (dir_hdl == NULL)
                {
                    ui_print("open %s fail", dir_name);
                    break;
                }
                ui_print("open %s success", dir_name);

                while ((info_dir = sAPI_readdir(dir_hdl)) != NULL)
                {
                    ui_print("name[%s]-size[%ld]-type[%d]\r\n", info_dir->name, info_dir->size, info_dir->type);
                }

                ret = sAPI_closedir(dir_hdl);
                if (ret != 0)
                {
                    ui_print("close %s fail[%d]", dir_name, ret);
                    break;
                }
                break;
            }
            case SC_FS_DEMO_RXFILE:
            {
                memset(pTemBuffer, 0, sizeof(pTemBuffer));
                memset(gpReceiveFileDes, 0, MAX_VALID_USER_NAME_LENGTH);

                ui_print("\r\nPlease input full filename:\r\n");
                UartReadLine(gpReceiveFileDes, MAX_VALID_USER_NAME_LENGTH + 1);

                if (strstr(gpReceiveFileDes, "\r\n"))
                {
                    sAPI_Debug("FILE SYSTEM File name should not have a carriage return feed");
                    sprintf(pTemBuffer, "\r\n rx file fail - filename should not have carriage return newlines\r\n");
                    ui_print(pTemBuffer);
                    break;
                }

                ui_print("\r\nPlease input data length:\r\n");
                gfile_len_total = UartReadValue();
                sAPI_enableDUMP();

                free_size = sAPI_GetFreeSize(cur_path_in);
                if (gfile_len_total >= free_size)
                {
                    sAPI_Debug("FILE SYSTEM  free size[%lld] is not enough for filesize[%d]", free_size, gfile_len_total);
                    sprintf(pTemBuffer, "\r\n rx file size is too large\r\n");
                    ui_print(pTemBuffer);
                    break;
                }

                if (gfile_len_total < 1)
                {
                    sprintf(pTemBuffer, "\r\nFILE SYSTEM file size should not be zero\r\n");
                    ui_print(pTemBuffer);
                    break;
                }
                if ((pRXDataBuffer = (char *)sAPI_Malloc(RECV_DATA_BUFFER_SIZE)) == NULL)
                {
                    sAPI_Debug("FILE SYSTEM malloc fail!!");
                    sprintf(pTemBuffer, "\r\n rx file fail -malloc fail\r\n");
                    ui_print(pTemBuffer);
                    break;
                }
                sAPI_Debug("FILE SYSTEM malloc pRXDataBuffer[%p]", pRXDataBuffer);

                pDataR = pRXDataBuffer;
                pDataW = pRXDataBuffer;
                rxDataFreeSize = RECV_DATA_BUFFER_SIZE;
                gbFileTransmitting = TRUE;
                bFail = FALSE;

                if (NULL == gReceiveDataMutexRef)
                {
                    if ((sAPI_MutexCreate(&gReceiveDataMutexRef, SC_FIFO)) != SC_SUCCESS)
                    {
                        sprintf(pTemBuffer, "\r\n rx file fail -sAPI_MutexCreate fail\r\n");
                        ui_print(pTemBuffer);
                        break;
                    }
                }

                if (NULL == gReceiveDataPreparedFlagRef)
                {
                    if (sAPI_FlagCreate(&gReceiveDataPreparedFlagRef) != SC_SUCCESS)
                    {
                        sAPI_MutexDelete(gReceiveDataMutexRef);
                        sprintf(pTemBuffer, "\r\n rx file fail -sAPI_FlagCreate1 fail\r\n");
                        ui_print(pTemBuffer);
                        break;
                    }
                }

                if (NULL == gFsHandleTask)
                {
                    gFsHandleTaskStack = sAPI_Malloc(SC_DEFAULT_THREAD_STACKSIZE);
                    if (sAPI_TaskCreate(
                            &gFsHandleTask,
                            gFsHandleTaskStack,
                            SC_DEFAULT_THREAD_STACKSIZE,
                            SC_RECV_FILE_TASK_PRIORITY,
                            (char *)"fsHandleProcesser",
                            sTask_FsHandeProcesser, (void *)0)) //init the demo task
                    {
                        sAPI_MutexDelete(gReceiveDataMutexRef);
                        sAPI_FlagDelete(gReceiveDataPreparedFlagRef);
                        sprintf(pTemBuffer, "\r\n rx file fail -sAPI_TaskCreate2 fail\r\n");
                        ui_print(pTemBuffer);
                        break;
                    }
                }
                sAPI_Debug("FILE SYSTEM Successful preparation!!");
                ui_print("FILE SYSTEM Successful preparation!!");

                sRecvProcesser();

                break;
            }
            case SC_FS_DEMO_TXFILE:
            {
                char *pReadBuffer = NULL;
                char buff[MAX_OUTPUT_LEN * 2] = {0};
                int totalReadLen = 0;
                int needReadLen = 0;
                int prepareReadLen = 0;
                memset(tx_file_name, 0, MAX_VALID_USER_NAME_LENGTH);

                ui_print("\r\nPlease input full filename:\r\n");
                UartReadLine(tx_file_name, MAX_VALID_USER_NAME_LENGTH);

                memset(pTemBuffer, 0, sizeof(pTemBuffer));
                ret = sAPI_stat(tx_file_name, &info_file);
                if (ret != 0)
                {
                    sAPI_Debug("sAPI_FsStat make err,ERROR code: %d", ret);
                    sprintf(pTemBuffer, "\r\nFILE SYSTEM file not exist\r\n");
                    ui_print(pTemBuffer);
                    break;
                }
                needReadLen = info_file.size;

                file_hdl = sAPI_fopen(tx_file_name, "rb");
                if (NULL == file_hdl)
                {
                    sAPI_Debug("sAPI_fopen make err,ERROR code: %d", ret);
                    sprintf(pTemBuffer, "\r\nFILE SYSTEM open file fail\r\n");
                    ui_print(pTemBuffer);
                    break;

                }
                totalReadLen = 0;
                while (1)
                {
                    if ((needReadLen - totalReadLen) < SIM_FILE_OUTPUT_SIZE)
                    {
                        prepareReadLen = needReadLen - totalReadLen;
                    }
                    else
                    {
                        prepareReadLen = SIM_FILE_OUTPUT_SIZE;
                    }

                    if (NULL == pReadBuffer)
                    {
                        pReadBuffer = (char *)sAPI_Malloc(SIM_FILE_OUTPUT_SIZE + 1);
                        if (NULL == pReadBuffer)
                        {
                            sAPI_Debug("sAPI_Malloc err,ERROR code: %d", ret);
                            sprintf(pTemBuffer, "\r\nFILE SYSTEM MALLOC fail\r\n");
                            ui_print(pTemBuffer);
                            break;
                        }
                    }
                    else
                    {
                        sAPI_Debug("pReadBuffer is not null");
                        ui_print("pReadBuffer is not null");
                    }

                    memset(pReadBuffer, 0, SIM_FILE_OUTPUT_SIZE + 1);
                    actul_read_len = sAPI_fread(pReadBuffer, 1, prepareReadLen, file_hdl);
                    totalReadLen += actul_read_len;

                    memset(pTemBuffer, 0, sizeof(pTemBuffer));
                    memset(buff, 0, sizeof(buff));
                    if (actul_read_len > 0)
                    {
                        sprintf(pTemBuffer, "read file size: %d\r\n", actul_read_len);
                        sprintf(buff, "read file data:%s\r\n", pReadBuffer);
                        strcat(pTemBuffer, buff);
                        ui_print(pTemBuffer);
                        sAPI_Free(pReadBuffer);
                        pReadBuffer = NULL;

                        if (totalReadLen >= needReadLen)
                        {
                            sprintf(pTemBuffer, "read file END\r\n");
                            ui_print(pTemBuffer);
                            memset(pTemBuffer, 0, sizeof(pTemBuffer));
                            break;
                        }
                    }
                    else
                    {
                        sAPI_Debug("sAPI_fread err,actual read: %d", actul_read_len);
                        sprintf(pTemBuffer, "\r\nFILE SYSTEM READ fail\r\n");
                        ui_print(pTemBuffer);
                        sAPI_Free(pReadBuffer);
                        pReadBuffer = NULL;
                        break;
                    }
                }

                sAPI_fclose(file_hdl);
                if (ret != 0)
                {
                    sAPI_Debug("sAPI_fclose err,err_code[%d]", ret);
                    sprintf(pTemBuffer, "\r\nFILE SYSTEM close file failed\r\n");
                    ui_print(pTemBuffer);
                    break;
                }
                else
                {
                    file_hdl = NULL;
                }

                break;
            }
            case 99:
            {
                return;
            }
            default:
            {
                break;
            }
        }
    }

}

