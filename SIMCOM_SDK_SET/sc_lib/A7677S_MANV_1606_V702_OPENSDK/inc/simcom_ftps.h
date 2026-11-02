/**
* @file         simcom_ftps.h
* @brief        SIMCom FTPS API
* @author       huyujie
* @date         2020/3/25
* @version      V1.0.0
* @par Copyright (c):
*               SIMCom Co.Ltd 2003-2020
* @par History: 1:Create
*
*/

#ifndef __SIMCOM_FTPS_H__
#define __SIMCOM_FTPS_H__

#include "simcom_os.h"
/*
    SIMCOM_FTPS_ERROR_CODE
*/
typedef enum {
    SC_FTPS_RESULT_OK,// 0
    SC_FTPS_RESULT_SSL_ALERT,// 1
    SC_FTPS_RESULT_UNKNOWN_ERROR, // 2
    SC_FTPS_RESULT_ERROR_BUSY,    // 3
    SC_FTPS_RESULT_SERVER_CLOSED,  // 4
    SC_FTPS_RESULT_ERROR_TIMEOUT,// 5
    SC_FTPS_RESULT_ERROR_TRANSFER_FAILED,   // 6  /*receive/send socket data failed*/
    SC_FTPS_RESULT_ERROR_MEMORY_ERROR,// 7
    SC_FTPS_RESULT_ERROR_INVALID_PARAM,  // 8
    SC_FTPS_RESULT_ERROR_REJ_BY_SERVER,  // 9
    SC_FTPS_RESULT_NETWORK_ERROR,  //10
    SC_FTPS_RESULT_STATE_ERROR,// 11
    SC_FTPS_RESULT_FAILED_TO_PARSE_SERVER_NAME, // 12
    SC_FTPS_RESULT_FAILED_TO_CREATE_SOCKET, // 13
    SC_FTPS_RESULT_FAILED_TO_CONNECT_SOCKET, //14
    SC_FTPS_RESULT_FAILED_TO_CLOSE_SOCKET, // 15
    SC_FTPS_RESULT_FAILED_TO_PEER_CLOSED,// 16
    SC_FTPS_RESULT_ERROR_FILE_ERROR,// 17
    SC_FTPS_DO_NOTHING,
    SC_FTPS_RESULT_ERR_SESSION_NULL,
    SC_FTPS_RESULT_ERR_CLOSE_SSL,
    SC_FTPS_RESULT_SERVER_CONN_TIMEOUT_ERROR = 421,
    SC_END_OF_MFTP_RESULT
}SC_FTPS_ERROR_CODE;
/*
    STRUCT DEFINE
*/
typedef struct{
    char host[256]; /*xx.xx.xx.xx:port, or URL*/
    char username[256]; /*note: qijun require 256, need turn*/
    char password[256];
    INT32 port;
    UINT8 serverType; /*0: FTP client, 1:FTPS SSL client 2:FTPS TLS client 3:implicit FTPS client*/
}SCftpsLoginMsg;
/*
    PDP ACTIVE TYPE
*/
typedef enum {
    SC_FTPS_USB,
    SC_FTPS_UART
}SC_PDP_ACTIVE_TYPE;
/*
    FILE LOCATION
*/
typedef enum{
    SC_FTPS_FILE_FLASH = 1,
    SC_FTPS_FILE_SDCARD,
    SC_FTPS_FILE_EXT_FLASH_FS
}SC_FTPS_FILE_LOCATION;
/*
    DATA SOCKET IP TYPE
*/
typedef enum{
    SC_PORT_RESPONSE_BY_SERVER,
    SC_SAME_AS_CONTROLSOCKET_IP
}SC_FTPS_DATASOCKET_IP_TYPE;

typedef enum{
    SC_FTPS_INIT,
    SC_FTPS_DEINIT,
    SC_FTPS_LOGIN,
    SC_FTPS_LOGOUT,
    SC_FTPS_DOWNLOADFILE,
    SC_FTPS_DOWNLOADTOBUFFER,
    SC_FTPS_UPLOADFILE,
    SC_FTPS_DELETEFILE,
    SC_FTPS_CREATDIR,
    SC_FTPS_DELETEDIR,
    SC_FTPS_CHANGEDIR,
    SC_FTPS_GETDIR,
    SC_FTPS_LIST,
    SC_FTPS_SIZE,
    SC_FTPS_SETTYPE,
    SC_FTPS_GETTYPE,
    SC_FTPS_SSLCONFIG,
    SC_FTPS_SETMODE
}SC_FTPS_API_TYPE;

typedef enum{
    SC_DATA_COMPLETE,
    SC_DATA_RESUME
}SC_FTPS_DATA_FLAG;

typedef struct{
    SC_FTPS_DATA_FLAG flag;
    char *data;
    UINT32 len;
}SCapiFtpsData;

/***********************************API_FTP(s)***************************************************/
typedef SC_FTPS_ERROR_CODE SC_FTPS_RETURNCODE;

/*****************************************************************************
 * FUNCTION
 *  sAPI_FTPSInit
 *
 * DESCRIPTION
 *  sAPI_FTPSInit() is used to initialize FTPS client and PDP activation.
 *
 * PARAMETERS
 *  type: PDP activation currently has two channels: USB and UART.
 *  msgQRef: All API results will be returned by msgQRef.
 *
 * RETURNS
 *  @SIM_MSG_T returned by msgQRef.
 *
 * NOTE
 *  You must execute sAPI_FTPSInit before any other FTP(S) related API.
 *****************************************************************************/
void sAPI_FtpsInit(SC_PDP_ACTIVE_TYPE type,sMsgQRef msgQRef);


/*****************************************************************************
 * FUNCTION
 *  sAPI_FTPSDeInit
 *
 * DESCRIPTION
 *  sAPI_FTPSDeInit() is used to stop FTPS client and PDP deactivation.
 *
 * PARAMETERS
 *  type: PDP deactivation currently has two channels: USB and UART.
 *
 * RETURNS
 *  @SIMCOM_FTPS_ERROR_CODE
 *
 * NOTE
 *  When you are no longer using the FTP(S) APIs, use this API.
 *****************************************************************************/
void sAPI_FtpsDeInit(SC_PDP_ACTIVE_TYPE TYPE);


/*****************************************************************************
 * FUNCTION
 *  sAPI_FTPSLogin
 *
 * DESCRIPTION
 *  sAPI_FTPSLogin() is used to login to a FTP(S) server.
 *
 * PARAMETERS
 *  ftpsLoginMsg:Contains information needed to log on to the FTP server.
 *  Such as server IP address,login account,password,port number,server type.
 *
 * RETURNS
 *  @SIMCOM_FTPS_ERROR_CODE
 *
 * NOTE
 *  serverType: 0:FTP client 1:FTPS SSL client 2:FTPS TLS client 3:implicit FTPS client
 *****************************************************************************/
SC_FTPS_RETURNCODE sAPI_FtpsLogin(SCftpsLoginMsg msg);


/*****************************************************************************
 * FUNCTION
 *  sAPI_FTPSLogout
 *
 * DESCRIPTION
 *  sAPI_FTPSLogout is used to logout a FTP(S) sever.
 *
 * PARAMETERS
 *  None
 *
 *
 * RETURNS
 *  @SIMCOM_FTPS_ERROR_CODE
 *
 * NOTE
 *  Make sure you login a FTP(S) sever before you use sAPI_FTPSLogout().
 *****************************************************************************/
SC_FTPS_RETURNCODE sAPI_FtpsLogout(void);


/*****************************************************************************
 * FUNCTION
 *  sAPI_FTPSDownloadFile
 *
 * DESCRIPTION
 *  sAPI_FTPSDownloadFile() is used to download a whole file into flash or sd card.
 *
 * PARAMETERS
 *  fileName:The name of the file you want to download from the server.
 *  loc:Where to save the file.
 *
 * RETURNS
 *  @SIMCOM_FTPS_ERROR_CODE
 *
 * NOTE
 *
 *****************************************************************************/
SC_FTPS_RETURNCODE sAPI_FtpsDownloadFile(char* fileName,SC_FTPS_FILE_LOCATION loc);
/*****************************************************************************
 * FUNCTION
 *  sAPI_FTPSDownloadFileToBuffer
 *
 * DESCRIPTION
 *  sAPI_FTPSDownloadFileToBuffer is used to download a file to buffer.
 *
 * PARAMETERS
 *  fileName:The name of the file you want to download from the server.
 *  startPos:The value for FTP “REST” command which is used for broken transfer.
 *
 * RETURNS
 *  @SIMCOM_FTPS_ERROR_CODE
 *
 * NOTE
 *  The buffer will be return by ftps_data_process_cb.
 *****************************************************************************/
SC_FTPS_RETURNCODE sAPI_FtpsDownloadFileToBuffer(char* fileName,int startPos);

/*****************************************************************************
 * FUNCTION
 *  sAPI_FTPSUploadFile
 *
 * DESCRIPTION
 *  sAPI_FTPSUploadFile is used to upload a file to FTP(S) server.
 *
 * PARAMETERS
 *  fileName:The name of the file you want to upload to server.
 *  loc:File location
 *  startPos:The value for FTP “REST” command which is used for broken transfer.
 *
 * RETURNS
 *  @SIMCOM_FTPS_ERROR_CODE
 *
 * NOTE
 *
 *****************************************************************************/
SC_FTPS_RETURNCODE sAPI_FtpsUploadFile(char* fileName,SC_FTPS_FILE_LOCATION loc,int startPos);

/*****************************************************************************
 * FUNCTION
 *  sAPI_FTPSDeleteFile
 *
 * DESCRIPTION
 *  sAPI_FTPSDeleteFile is used to delete file in FTP(S) server.
 *
 * PARAMETERS
 *  fileName:The name of the file you want to delete in FTP(S) server.
 *
 * RETURNS
 *  @SIMCOM_FTPS_ERROR_CODE
 *
 * NOTE
 *
 *****************************************************************************/
SC_FTPS_RETURNCODE sAPI_FtpsDeleteFile(char* fileName);
/*****************************************************************************
 * FUNCTION
 *  sAPI_FTPSCreateDirectory
 *
 * DESCRIPTION
 *  sAPI_FTPSCreateDirectory is used to creat a directory in FTP(S) server.
 *
 * PARAMETERS
 *  dir:directory name.
 *
 * RETURNS
 *  @SIMCOM_FTPS_ERROR_CODE
 *
 * NOTE
 *
 *****************************************************************************/
SC_FTPS_RETURNCODE sAPI_FtpsCreateDirectory(char* dir);
/*****************************************************************************
 * FUNCTION
 *  sAPI_FTPSDeleteDirectory
 *
 * DESCRIPTION
 *  sAPI_FTPSDeleteDirectory is used to delete a directory in FTP(S) server.
 *
 * PARAMETERS
 *  dir:directory name.
 *
 * RETURNS
 *  @SIMCOM_FTPS_ERROR_CODE
 *
 * NOTE
 *
 *****************************************************************************/
SC_FTPS_RETURNCODE sAPI_FtpsDeleteDirectory(char* dir);
/*****************************************************************************
 * FUNCTION
 *  FTPS_changeDirectory
 *
 * DESCRIPTION
 *  FTPS_changeDirectory is used to change the current directory in FTP(S) server.
 *
 * PARAMETERS
 *  dir:directory name.
 *
 * RETURNS
 *  @SIMCOM_FTPS_ERROR_CODE
 *
 * NOTE
 *
 *****************************************************************************/
SC_FTPS_RETURNCODE sAPI_FtpsChangeDirectory(char* dir);

/*****************************************************************************
 * FUNCTION
 *  FTPS_getCurrentDirectory
 *
 * DESCRIPTION
 *  FTPS_getCurrentDirectory is used to get the current directory on FTP(S) server.
 *
 * PARAMETERS
 *  None.
 *
 * RETURNS
 *  @SIMCOM_FTPS_ERROR_CODE
 *
 * NOTE
 *  The current directory will be return by ftps_data_process_cb.
 *****************************************************************************/
SC_FTPS_RETURNCODE sAPI_FtpsGetCurrentDirectory(void);

/*****************************************************************************
 * FUNCTION
 *  FTPS_list
 *
 * DESCRIPTION
 *  FTPS_list
 *
 * PARAMETERS
 *  dir:directory name.
 *
 * RETURNS
 *  @SIMCOM_FTPS_ERROR_CODE
 *
 * NOTE
 *  The list data will be return by ftps_data_process_cb.
 *****************************************************************************/
SC_FTPS_RETURNCODE sAPI_FtpsList(char* dir);

/*****************************************************************************
 * FUNCTION
 *  FTPS_getFileSize
 *
 * DESCRIPTION
 *  FTPS_getFileSize is used to get file size in FTP(S) server.
 *
 * PARAMETERS
 *  fileName:file name.
 *
 * RETURNS
 *  @SIMCOM_FTPS_ERROR_CODE
 *
 * NOTE
 *  The file size will be return by ftps_data_process_cb.
 *****************************************************************************/
SC_FTPS_RETURNCODE sAPI_FtpsGetFileSize(char* fileName);

/*****************************************************************************
 * FUNCTION
 *  FTPS_transferType
 *
 * DESCRIPTION
 *  FTPS_transferType() is used to set the transfer type on FTP(S) server.
 *
 * PARAMETERS
 *  type:  A:Ascii I:Binary
 *
 * RETURNS
 *  @SIMCOM_FTPS_ERROR_CODE
 *
 * NOTE
 *
 *****************************************************************************/
SC_FTPS_RETURNCODE sAPI_FtpsTransferType(char* type);
/*****************************************************************************
 * FUNCTION
 *  FTPS_getTransferType
 *
 * DESCRIPTION
 *  FTPS_getTransferType() is used to get the current transfer type.
 *
 * PARAMETERS
 *  None.
 *
 * RETURNS
 *  @SIMCOM_FTPS_ERROR_CODE
 *
 * NOTE
 *  The transfer type will be return by ftps_data_process_cb.
 *****************************************************************************/
SC_FTPS_RETURNCODE sAPI_FtpsGetTransferType(void);


/*****************************************************************************
 * FUNCTION
 *  FTPS_sslConfig
 *
 * DESCRIPTION
 *  FTPS_sslConfig is used to set the SSL context id for FTPS session.
 *
 * PARAMETERS
 *  session:0 for control session, 1 for data session.
 *  sslId:SSL context ID during 0-9.
 *
 * RETURNS
 *  @SIMCOM_FTPS_ERROR_CODE
 *
 * NOTE
 *
 *****************************************************************************/
SC_FTPS_RETURNCODE sAPI_FtpsSslConfig(int session,int sslId);
/*****************************************************************************
 * FUNCTION
 *  FTPS_setMode
 *
 * DESCRIPTION
 *  FTPS_setMode() is used to set the mode on FTP(S) server.
 *
 * PARAMETERS
 *  mode:  0:PORT 1:PASV
 *
 * RETURNS
 *  @SIMCOM_FTPS_ERROR_CODE
 *
 * NOTE
 *
 *****************************************************************************/
SC_FTPS_RETURNCODE sAPI_FtpsSetMode(int mode);
#endif
