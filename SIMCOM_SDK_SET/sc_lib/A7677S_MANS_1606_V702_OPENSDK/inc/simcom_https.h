/**
 * @file
 * http headfile *
 */

 /*
  * Copyright (c) simcom
  * All rights reserved.
  */

#ifndef SIMCOM_HTTPS_H
#define SIMCOM_HTTPS_H

#include "simcom_os.h"

typedef struct
{
    INT32 status_code;  /*process result for request msg*/
    INT32 method;      /*for sAPI_HttpAction*/
    INT32 action_content_len;    /*the recv lenth of sAPI_HttpAction and sAPI_HttpPostfile*/
    UINT8 *data;     /*the data trans from API*/
    INT32 dataLen;   /*the datalen of data*/
} SChttpApiTrans;

typedef enum
{
    SC_HTTPS_SUCCESS,
    SC_HTTPS_FAIL,
    SC_HTTPS_SERVICE_NOT_AVAILABLE,
    SC_HTTPS_INVALID_PARAMETER,
    SC_HTTPS_FILE_NOT_EXIST,
    SC_HTTPS_WRITE_FILE_FAIL,
    SC_HTTPS_READ_FILE_FAIL,
    SC_HTTPS_DNS_PARSE_FAIL,
    SC_HTTPS_CONNECT_FAIL,
    SC_HTTPS_HANDSHAKE_FAILED,
    SC_HTTPS_TRANSFER_ERROR,      //http receive data or send fail.
    SC_HTTPS_ERROR_END
} SC_HTTP_RETURNCODE;


#define CHUNK  -0x100
/*****************************************************************************
* FUNCTION
*  HTTP_miHttpInit
*
* DESCRIPTION
*  sAPI_HttpInit is used to start HTTP service by activating PDP context.
*
* PARAMETERS
*  channel: data transmission channel
*       0 - serial port
*       1 - usb
*  magQ_urc: the message queue for urc report, it is valid duiring whole http service.
*
* RETURNS
*  SC_HTTP_RETURNCODE
*
* NOTE
*  it should be called before anyother http command.
*****************************************************************************/
SC_HTTP_RETURNCODE sAPI_HttpInit(int channel, sMsgQRef magQ_urc);



/*****************************************************************************
* FUNCTION
*  sAPI_HttpTerm
*
* DESCRIPTION
*  HTTP_miHttpTerm is used to stop HTTP service.
*
* PARAMETERS
*  none
*
* RETURNS
*  SC_HTTP_RETURNCODE
*
* NOTE
*  it should be called after anyother http command.
*****************************************************************************/
SC_HTTP_RETURNCODE sAPI_HttpTerm(void);


/*****************************************************************************
* FUNCTION
*  sAPI_HttpPara
*
* DESCRIPTION
*  HTTP_miHttpPara is used to set HTTP parameters value.
*       When you want to access to a HTTP server,
*       you should input <value> like http://’server’/’path’:’tcpPort’.
*       In addition, https://’server’/’path’:’tcpPort’ is used to access to a HTTPS server.
*
* PARAMETERS
*  command_type: the diff type of commands, such as: ”URL”, ”CONNECTTO”, ”RECVTO”, ”CONTENT” ,”ACCEPT”,”SSLCFG”,”USERDATA”,”READMODE”
*  parameters: the parameters value matched with its command
*
* RETURNS
*  SC_HTTP_RETURNCODE
*
* NOTE
*  The "READMODE" can only set once before every HTTP operation.
*  The "URL" should set before HTTP_miHttpAction,and other command_type will be default if don't set(include "READMODE").
*****************************************************************************/
SC_HTTP_RETURNCODE sAPI_HttpPara(char *command_type, char *parameters);


/*****************************************************************************
* FUNCTION
*  sAPI_HttpAction
*
* DESCRIPTION
*  HTTP_miHttpAction is used to perform a HTTP Method. You can use this api to send a get/post request to a HTTP/HTTPS server.
*
* PARAMETERS
*  method:
*       0: GET
*       1: POST
*       2: HEAD
*       3: DELETE
* RETURNS
*  SC_HTTP_RETURNCODE
*
* NOTE
*
*****************************************************************************/
SC_HTTP_RETURNCODE sAPI_HttpAction(int methods);


/*****************************************************************************
* FUNCTION
*  sAPI_HttpData
*
* DESCRIPTION
*  HTTP_miHttpData is used to pass the filename to post when you send a HTTP/HTTPS POST request.
*
* PARAMETERS
* buffer: HTTP post data
* len: Length of data
* RETURNS
*  SC_HTTP_RETURNCODE
*
* NOTE
*
*****************************************************************************/
SC_HTTP_RETURNCODE sAPI_HttpData(char *buffer, int len);


/*****************************************************************************
* FUNCTION
*  sAPI_HttpHead
*
* DESCRIPTION
*  HTTP_miHttpHead is used to read the HTTP header information of server response when module receives the response data from server.
*
* PARAMETERS
*  none
*
* RETURNS
*  SC_HTTP_RETURNCODE
*
* NOTE
* The read data would given by the msgQ
*****************************************************************************/
SC_HTTP_RETURNCODE sAPI_HttpHead(void);//(char *head_data_buff, int *head_data_len);



/*****************************************************************************
* FUNCTION
*  sAPI_HttpRead
*
* DESCRIPTION
*  HTTP_miHttpRead,can read the data which from server.
*
* PARAMETERS
*  commd_type:
*       0: get total size of data saved in buffer,  get the size by para: byte_size
*       1: set the read data offset and size
*  start_offset: The start position of reading
*  byte_size: The length of data to read
*
* RETURNS
*  SC_HTTP_RETURNCODE
*
* NOTE
* The read data would given by the msgQ
*****************************************************************************/
SC_HTTP_RETURNCODE sAPI_HttpRead(int commd_type, int start_offset, int byte_size);


/*****************************************************************************
* FUNCTION
*  sAPI_HttpPostfile
*
* DESCRIPTION
*  The API can post file from client to server.The URL must be set by HTTP_miHttpPara before executing HTTP_miHttpPostfile API.
*
* PARAMETERS
*  filename:the file name will send to the server
*  dir:the path
*     1: C:/
*     2: D:/
* RETURNS
*  SC_HTTP_RETURNCODE
*
* NOTE
* the file should exist under the directory.
* the data from server would given by the msgQ
*****************************************************************************/
SC_HTTP_RETURNCODE  sAPI_HttpPostfile(char *filename, int dir);

#endif
