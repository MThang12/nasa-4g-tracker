#ifndef  SIMCOM_HTP_CLIENT_H
#define  SIMCOM_HTP_CLIENT_H


#include "simcom_os.h"




typedef enum
{
  SC_HTP_OK = 0,
  SC_HTP_ERROR,
  SC_HTP_UNKNOWN_ERROR, 
  SC_HTP_INVALID_PARAM,  
  SC_HTP_BAD_DATETIME_GOT,  
  SC_HTP_NETWORK_ERROR,  
  SC_HTP_END
}SChtpResultType;

typedef SChtpResultType SChtpReturnCode;

typedef enum {
    SC_HTP_OP_SET = 0,
    SC_HTP_OP_GET,
}SChtpOperationType;



/*****************************************************************************
* FUNCTION
*  sAPI_HtpSrvConfig
*
* DESCRIPTION
*  sAPI_HtpSrvConfig is used to updata system time with http server . 
* PARAMETERS
*   commad_type: 
*       SC_HTP_OP_SET, set parameters, add or remove addr
*       SC_HTP_OP_GET, get current addrs in the list
*   return_string: get the addr list, availiable only for SC_HTP_OP_GET
*   cmd: This command to operate the HTP server list.
*       “ADD”: add a HTP server item to the list
*       “DEL”: delete a HTP server item from the list
*   host_or_idx: If the <cmd> is “ADD”, this field is the same as <host>, length is 0-255, needs quotation marks; 
*       If the <cmd> is “DEL”, this field is the index of the HTP server item to be deleted from the list, does not need quotation marks.
*   port: 
*       The HTP server port, the range is (1-65535) .
*   http_version: The HTTP version of the HTP server:
*       0-	HTTP 1.0
*       1-	HTTP 1.1
*   proxy: The proxy address, length is 1-255.
*   proxy_port: The port of the proxy, the range is (1-65535).
*
* RETURNS
*  SChtpReturnCode  
*
* NOTE
*  Ignoring part of parameters by set as NULL or 0.
*****************************************************************************/

SChtpReturnCode sAPI_HtpSrvConfig(SChtpOperationType commad_type, char *return_string, char *cmd, char* host_or_idx, int host_port, int http_version, char *proxy, int proxy_port);

/*****************************************************************************
* FUNCTION
*  sAPI_HtpUpdate
*
* DESCRIPTION
*   sAPI_HtpUpdate is used to trigger the the  thread of time update, and return after the trigger is successful.
* PARAMETERS
*   magQ_urc: the massage queue for urc msg of time updata. The final result send to this msgQ
*       msg.arg1 is the result code, follow SChtpReturnCode type;
*
* RETURNS
*  SChtpReturnCode  
*
* NOTE
*  The massage queue should be valid, the result will send to this queue.
*****************************************************************************/
SChtpReturnCode sAPI_HtpUpdate( sMsgQRef magQ_urc );




#endif

