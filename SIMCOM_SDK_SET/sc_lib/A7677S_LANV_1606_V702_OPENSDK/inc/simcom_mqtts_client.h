#ifndef _SIMCOM_MQTTS_CLIENT_H_
#define _SIMCOM_MQTTS_CLIENT_H_

#include "simcom_os.h"


typedef enum
{
    SC_MQTT_RESULT_SUCCESS = 0,   
    SC_MQTT_RESULT_FAIL,
    SC_MQTT_RESULT_BAD_UTF8_STR,
    SC_MQTT_RESULT_SOCK_CONN_FAIL,
    SC_MQTT_RESULT_SOCK_CREATE_FAIL,
    SC_MQTT_RESULT_SOCK_CLOSE_FAIL, //5
    SC_MQTT_RESULT_RCV_FAIL,
    SC_MQTT_RESULT_NETWORK_OPEN_FAIL,
    SC_MQTT_RESULT_NETWORK_CLOSE_FAIL,
    SC_MQTT_RESULT_NETWORK_NO_OPEN,
    SC_MQTT_RESULT_CLINET_INDEX_ERR, //10
    SC_MQTT_RESULT_NO_CONNECTION,
    SC_MQTT_RESULT_INVALID_PARAMETER,    
    SC_MQTT_RESULT_OPERATION_NOT_SUPPORT,
    SC_MQTT_RESULT_BUSY,
    SC_MQTT_RESULT_REQ_CONNECTION_FAIL, //15
    SC_MQTT_RESULT_SOCK_SENDING_FAIL,
    SC_MQTT_RESULT_TIMOUT,
    SC_MQTT_RESULT_TOPIC_EMPTY,
    SC_MQTT_RESULT_CLIENT_IN_USE,
    SC_MQTT_RESULT_CLIENT_NOT_ACCQ, //20
    SC_MQTT_RESULT_CLIENT_NOT_REL,
    SC_MQTT_RESULT_EXCEED_MAX_VAL,
    SC_MQTT_RESULT_NETWORK_HAVE_OPENED,
    SC_MQTT_RESULT_PACKET_FAIL,
    SC_MQTT_RESULT_DNS_ERROR,
    SC_MQTT_RESULT_SOCK_CLOSE,// 26  socket closed by server
    SC_MQTT_RESULT_UNACCEPTED_PROTOCOL_VER, //27 - connection refused: unaccepted protocol version
    SC_MQTT_RESULT_ID_REJECTED, //28  - connection refused: identifier rejected
    SC_MQTT_RESULT_SER_UNVAILBLE, //29  - connection refused: server unavailable
    SC_MQTT_RESULT_BAD_USRNAME_PWD, //30 - connection refused: bad user name or password
    SC_MQTT_RESULT_NOT_AUTHORIZED, // 31 - connection refused: not authorized
    SC_MQTT_RESULT_SSL_HANDSHAKE_ERR, //32 for ssl
    SC_MQTT_RESULT_NOT_SET_CERTS,//33 for ssl
    SC_MQTT_RESULT_OPEN_SESSION_ERR,//34 for ssl
    SC_MQTT_RESULT_DISCONN_FAIL,
    SC_MQTT_RESULT_MAX
}SCmqttResultType;
    
typedef enum {
    SC_MQTT_OP_SET = 0,
    SC_MQTT_OP_GET,
}SCmqttOperationType;


/**************************************************************************************************************************/
/*************************************************parameters defination********************************************************/
typedef SCmqttResultType  SCmqttReturnCode;

typedef void (*mqtt_connlost_cb)(int client_index, int cause);

/*********************************************************
* discription: 
*       this structure prepared for message sending 
* 
* SC_STATUS simMsgSend( OSMsgQRef   msgQRef, SIM_MSG_T   *msgPtr )
*                                                           ^
*
* SC_STATUS simMsgRecv( OSMsgQRef   msgQRef, SIM_MSG_T   *recvMsg, UINT32      timeout)
*                                                           ^
*
*********************************************************/
typedef struct{
    int client_index;
    unsigned int topic_len;
    char *topic_P;
    unsigned int payload_len;
    char *payload_P;
}SCmqttData;



/******************************************************************************************************************************/
/*************************************************API defination***************************************************************/



/*****************************************************************************
* FUNCTION
*  sAPI_MqttStart
*
* DESCRIPTION
*  start the mqtt service;
*  
* PARAMETERS
*  channel: urc transmission channel(to Serial port or other channel)
*       0 - serial port
*       1 - usb
*      -1 - ignor
*
* RETURNS
*  SCmqttReturnCode  
*
* NOTE
*  should be called before anyother command
*****************************************************************************/
SCmqttReturnCode sAPI_MqttStart(int channel);

/*****************************************************************************
* FUNCTION
*  sAPI_MqttStop
*
* DESCRIPTION
*  sAPI_MqttStop is used stop the mqtt service;
*  
* PARAMETERS
*  void 
*
* RETURNS
*  SCmqttReturnCode  
*
* NOTE
*  it should be called after anyother MQTT command.
*****************************************************************************/
SCmqttReturnCode sAPI_MqttStop(void);



/*****************************************************************************
* FUNCTION
*  sAPI_MqttAccq
*
* DESCRIPTION
*  sAPI_MqttAccq is used to acquire a MQTT client. 
* PARAMETERS
*  commad_type: 
*        SC_MQTT_OP_SET -  set parameters by this command
*        SC_MQTT_OP_GET -  get the parameters seted by this command
*  string_return: the return string for commad_type=1;
*  client_index: A numeric parameter that identifies a client. The range of permitted values is 0 to 1.
*  clientID:The UTF-encoded string. It specifies a unique identifier for the client. The string length is from 1 to 128 bytes.
*  server_type: A numeric parameter that identifies the server type. The default value is 0.
*        0 - MQTT server with TCP
*        1 - MQTT server with SSL/TLS
*  msgQ_urc: the massage queue for urc msg of mqtt service.
* RETURNS
*  SCmqttReturnCode  
*
* NOTE
*  It must be called before all commands about MQTT connect and after sAPI_MqttStart.
*****************************************************************************/
SCmqttReturnCode sAPI_MqttAccq(SCmqttOperationType commad_type, char *string_return, int client_index, char *clientID, int server_type, sMsgQRef msgQ_urc);


/*****************************************************************************
* FUNCTION
*  sAPI_MqttRel
*
* DESCRIPTION
*  sAPI_MqttRel is used to release a MQTT client. 
* PARAMETERS
*  client_index: A numeric parameter that identifies a client. The range of permitted values is 0 to 1.
*
* RETURNS
*  SCmqttReturnCode  
*
* NOTE
*  It must be called after miCmqttDisConnect and before sAPI_MqttStop.
*****************************************************************************/
SCmqttReturnCode sAPI_MqttRel(int client_index);



/*****************************************************************************
* FUNCTION
*  sAPI_MqttConnect
*
* DESCRIPTION
*  sAPI_MqttConnect is used to connect to a MQTT server.
* PARAMETERS
*    commad_type: 
*        SC_MQTT_OP_SET -  set parameters by this command
*        SC_MQTT_OP_GET -  get the parameters seted by this command
*    string_return: the return string for commad_type=1;
*    client_index: A numeric parameter that identifies a client. The range of permitted values is 0 to 1.
*    server_addr: The string that described the server address and port. The range of the string length is 9 to 256 bytes. 
*           The string should be like this ��tcp://116.247.119.165:5141��, must begin with ��tcp://��. If the <server_addr> not include the port, the default port is 1883.
*    keepalive_time: The time interval between two messages received from a client. 
*           The client will send a keep-alive packet when there is no message sent to server after song long time. The range is from 1s to 64800s (18 hours).
*    clean_session: The clean session flag. The value range is from 0 to 1, and default value is 0.
*   
*    Optional user identification parameters:  
*      user_name = NULL and password = NULL for Non-essential conditions;
*        user_name: The user name identifies the name of the user which can be used for authentication when connecting to server. The string length is from 1 to 256 bytes.
*        pass_word: The password corresponding to the user which can be used for authentication when connecting to server. The string length is from 1 to 256 bytes.
*
*
* RETURNS
*  SCmqttReturnCode  
*
* NOTE
*  It must be called after miCmqttDisConnect and before sAPI_MqttStop.
*****************************************************************************/
SCmqttReturnCode sAPI_MqttConnect(SCmqttOperationType commad_type, char *string_return, int client_index, char *server_addr, int keepalive_time, int clean_session, char *user_name, char *pass_word);

/*****************************************************************************
* FUNCTION
*  sAPI_MqttDisConnect
*
* DESCRIPTION
*  sAPI_MqttDisConnect is used to disconnect from the server. 
* PARAMETERS
*    commad_type: 
*        SC_MQTT_OP_SET -  set parameters by this command
*        SC_MQTT_OP_GET -  get the parameters seted by this command
*    string_return: the return string for commad_type=1;
*    client_index: A numeric parameter that identifies a client. The range of permitted values is 0 to 1.
*    timeout: The timeout value for disconnection. The unit is second. The range is 60s to 180s. The default value is 0s (not set the timeout value).
* RETURNS
*  SCmqttReturnCode  
*
* NOTE
*  It must be called after miCmqttDisConnect and before sAPI_MqttRel.
*****************************************************************************/
SCmqttReturnCode sAPI_MqttDisConnect(SCmqttOperationType commad_type,char *string_return, int client_index, int timeout);



/*****************************************************************************
* FUNCTION
*  sAPI_MqttWillTopic
*
* DESCRIPTION
*  sAPI_MqttWillTopic  is used to input the topic of will message.
* PARAMETERS
*    client_index: A numeric parameter that identifies a client. The range of permitted values is 0 to 1.
*    topic_data: a topic string of publish message, it should be UTF-encoded string. The range is from 1 to 1024 bytes.
*    topic_length: The length of input topic data. 
* RETURNS
*  SCmqttReturnCode  
*
* NOTE
*  
*****************************************************************************/
SCmqttReturnCode sAPI_MqttWillTopic(int client_index, char *topic_data, int topic_length);


/*****************************************************************************
* FUNCTION
*  sAPI_MqttWillMsg
*
* DESCRIPTION
*  sAPI_MqttWillMsg is used to input the message body of will message.
* PARAMETERS
*    client_index: A numeric parameter that identifies a client. The range of permitted values is 0 to 1.
*    msg_data: a topic string of will message, it should be UTF-encoded string. The range is from 1 to 1024 bytes.
*    msg_length: The length of will message data. 
* RETURNS
*  SCmqttReturnCode  
*
* NOTE
*  
*****************************************************************************/
SCmqttReturnCode sAPI_MqttWillMsg(int client_index, char *msg_data, int msg_length);




/*****************************************************************************
* FUNCTION
*  sAPI_MqttTopic
*
* DESCRIPTION
*  sAPI_MqttTopic is used to input the topic of a publish message.
* PARAMETERS
*    client_index: A numeric parameter that identifies a client. The range of permitted values is 0 to 1.
*    topic_data: a topic string of publish message
*    topic_length: The length of input topic data. The publish message topic should be UTF-encoded string. The range is from 1 to 1024 bytes.
* RETURNS
*  SCmqttReturnCode  
*
* NOTE
*  The topic will be clean after execute sAPI_MqttPub.
*****************************************************************************/
SCmqttReturnCode sAPI_MqttTopic(int client_index, char *topic_data, int topic_length);


/*****************************************************************************
* FUNCTION
*  sAPI_MqttPayload
*
* DESCRIPTION
*  sAPI_MqttPayload is used to input the message body of a publish message.
* PARAMETERS
*    client_index: A numeric parameter that identifies a client. The range of permitted values is 0 to 1.
*    payload_data: a buffer for payload data 
*    payload_length: The length of input message data. The publish message should be UTF-encoded string. The range is from 1 to 10240 bytes.
* RETURNS
*  SCmqttReturnCode  
*
* NOTE
*  The payload will be clean after execute sAPI_MqttPub.
*****************************************************************************/
SCmqttReturnCode sAPI_MqttPayload(int client_index, char *payload_data, int payload_length);

/*****************************************************************************
* FUNCTION
*  sAPI_MqttPub
*
* DESCRIPTION
*  sAPI_MqttPub is used to publish a message to MQTT server.
* PARAMETERS
*    client_index: A numeric parameter that identifies a client. The range of permitted values is 0 to 1.
*    qos: The publish message��s qos. The range is from 0 to 2.
*    pub_timeout: The timeout value for disconnection. The unit is second. The range is 60s to 180s. The default value is 0s (not set the timeout value).
*    ratained: The retain flag of the publish message. The value is 0 or 1. The default value is 0. 
*    dup: The dup flag to the message. The value is 0 or 1. The default value is 0. The flag is set when the client or server attempts to re-deliver a message.
* RETURNS
*  SCmqttReturnCode  
*
* NOTE
*  The payload will be clean after execute AT+CMQTTPUB.
*****************************************************************************/
SCmqttReturnCode sAPI_MqttPub(int client_index, int qos, int pub_timeout, int ratained, int dup);


/*****************************************************************************
* FUNCTION
*  sAPI_MqttSubTopic
*
* DESCRIPTION
*  sAPI_MqttSubTopic is used to input the topic of a subscribe message.
* PARAMETERS
*    client_index: A numeric parameter that identifies a client. The range of permitted values is 0 to 1.
*    sub_topic_data: the topic of a subscribe message.
*    sub_topic_length: The length of input topic data. The publish message topic should be UTF-encoded string.  The range is from 1 to 10240 bytes.
*    qos: The publish message��s qos. The range is from 0 to 2.
* RETURNS
*  SCmqttReturnCode  
*
* NOTE
*  The topic will be clean after execute sAPI_MqttSub.
*****************************************************************************/
SCmqttReturnCode sAPI_MqttSubTopic(int client_index, char *sub_topic_data, int sub_topic_length, int qos);

/*****************************************************************************
* FUNCTION
*  sAPI_MqttSub
*
* DESCRIPTION
*  sAPI_MqttSub is used to  subscribe a message to MQTT server.
* PARAMETERS
*    client_index: A numeric parameter that identifies a client. The range of permitted values is 0 to 1.
*    topic_data: a topic string of publish message
*    topic_length: The length of input topic data. The publish message topic should be UTF-encoded string. The range is from 1 to 1024 bytes.
*    dup: The dup flag to the message. The value is 0 or 1. The default value is 0. The flag is set when the client or server attempts to re-deliver a message.
*    qos: The publish message��s qos. The range is from 0 to 2.
* RETURNS
*  SCmqttReturnCode  
*
* NOTE
*  The urc callback func will be called if any subscribed topic data;
*  sub more than one topic: topic_data=NULL, topic_length=0, qos =0;
*  The topic will be clean after execute sAPI_MqttSub.
*****************************************************************************/
SCmqttReturnCode sAPI_MqttSub(int client_index, char *topic_data, int topic_length, int qos, int dup);



/*****************************************************************************
* FUNCTION
*  sAPI_MqttUNSubTopic
*
* DESCRIPTION
*  sAPI_MqttUNSubTopic is used to input the topic of a unsubscribe message.
* PARAMETERS
*    client_index: A numeric parameter that identifies a client. The range of permitted values is 0 to 1.
*    unsub_topic_data: the topic of a unsubscribe message.
*    unsub_topic_length: The length of input topic data. The publish message topic should be UTF-encoded string. The range is from 1 to 1024 bytes.
* RETURNS
*  SCmqttReturnCode  
*
* NOTE
*  The topic will be clean after execute sAPI_MqttUnsub
*****************************************************************************/
SCmqttReturnCode sAPI_MqttUNSubTopic(int client_index, char *unsub_topic_data, int unsub_topic_length);



/*****************************************************************************
* FUNCTION
*  sAPI_MqttUnsub
*
* DESCRIPTION
*  sAPI_MqttUnsub is used to  subscribe a message to MQTT server.
* PARAMETERS
*    client_index: A numeric parameter that identifies a client. The range of permitted values is 0 to 1.
*    topic_data: A topic string of Unsub message
*    topic_length: The length of input topic data. The publish message topic should be UTF-encoded string. The range is from 1 to 1024 bytes.
*    dup: The dup flag to the message. The value is 0 or 1. The default value is 0. The flag is set when the client or server attempts to re-deliver a message.
* RETURNS
*  SCmqttReturnCode  
*
* NOTE
*  The urc callback func will be called if any subscribed topic data;
*  sub more than one topic: topic_data=NULL, topic_length=0;
*  The topic will be clean after execute sAPI_MqttUnsub.
*****************************************************************************/
SCmqttReturnCode sAPI_MqttUnsub(int client_index, char *topic_data, int topic_length, int dup);







/*****************************************************************************
* FUNCTION
*  sAPI_MqttSslCfg
*
* DESCRIPTION
*  sAPI_MqttSslCfg is used to set the SSL context which to be used in the SSL connection when it will connect to a SSL/TLS MQTT server. 
* PARAMETERS
*  commad_type: 
*        SC_MQTT_OP_SET -  set parameters by this command
*        SC_MQTT_OP_GET -  get the parameters seted by this command
*  string_return: the return string for commad_type=1;
*  client_index: A numeric parameter that identifies a client. The range of permitted values is 0 to 1.
*  ssl_ctx_index: The SSL context ID which will be used in the SSL connection. Refer to the <ssl_ctx_index> of AT+CSSLCFG
* RETURNS
*  SCmqttReturnCode  
*
* NOTE
*  If you don��t set the SSL context by this command before connecting to server by sAPI_MqttConnect, 
*    the mqtt connect operation will use the SSL context as same as index <client_index> 
*    (the second parameter of sAPI_MqttConnect) when connecting to the server.
*****************************************************************************/
SCmqttReturnCode sAPI_MqttSslCfg(SCmqttOperationType commad_type, char *string_return, int client_index, int ssl_ctx_index);


/*****************************************************************************
* FUNCTION
*  sAPI_MqttCfg
*
* DESCRIPTION
*  sAPI_MqttCfg is used to configure the MQTT context. It must be called before MQTT_miCMQTTConnect and after MQTT_miCMqttAccq. The setting will be cleared after MQTT_miCMqttRel.
* PARAMETERS
*  commad_type: 
*        SC_MQTT_OP_SET -  set parameters by this command
*        SC_MQTT_OP_GET -  get the parameters seted by this command
*  string_return: the return string for commad_type=1.
*  client_index: A numeric parameter that identifies a client. The range of permitted values is 0 to 1.
*  config_type: choose one of the configuration type 
*        0 -  "checkUTF8"
*        1 -  "optimeout"
*  related_value:
*       (1) if config_type = 0
*               related_value set as the flag to indicate whether to check the string is UTF8 coding or not, the default value is 1.
*               related_value = 0 �C Not check UTF8 coding.
*               related_value = 1 �C Check UTF8 coding.
*       (2) if config_type = 1
*               related_value = time_out, it is used to set max timeout interval of sending or receiving data operation. 
*               The range is from 20 seconds to 120 seconds, the default value is 120 seconds.
*
*         
* RETURNS
*  SCmqttReturnCode  
*
* NOTE
*  It must be called before sAPI_MqttConnect and after sAPI_MqttAccq. The setting will be cleared after sAPI_MqttRel.
*****************************************************************************/
SCmqttReturnCode sAPI_MqttCfg(SCmqttOperationType commad_type, char *string_return, int client_index, int config_type, int related_value);

/*****************************************************************************
* FUNCTION
*  sAPI_MqttConnlostCb
*
* DESCRIPTION
*  sAPI_MqttCfg is used to specify the callback function when the network is abnormally interrupted.
* PARAMETERS
*  cb: The function that will be called when the network is interrupted abnormally 
*         
* RETURNS
*  NONE
*
* NOTE
*  NONE
*****************************************************************************/
void sAPI_MqttConnLostCb(mqtt_connlost_cb cb);
#endif
