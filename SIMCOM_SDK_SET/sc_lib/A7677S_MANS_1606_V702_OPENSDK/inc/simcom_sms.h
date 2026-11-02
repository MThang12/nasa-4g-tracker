#ifndef _SIMCOM_SMS_H_
#define _SIMCOM_SMS_H_


#include "simcom_common.h"
#include "simcom_os.h"
#define SC_SMS_MAX_ADDRESS_LENGTH               40
#define SC_SMS_MAX_CI_MSG_PDU_SIZE             400
#define SC_SMS_MAX_MULTIPLE_MSG_LENGTH         820


typedef enum{
    SC_CI_SMS_PRIM_READ_MESAGE_REQ,
    SC_CI_SMS_PRIM_READ_MESAGE_CNF,
    SC_CI_SMS_PRIM_READ_PDU_MESSAGE_REQ,
    SC_CI_SMS_PRIM_READ_PDU_MESSAGE_CNF,
    SC_CI_SMS_PRIM_READ_TXT_MESSAGE_REQ,
    SC_CI_SMS_PRIM_READ_TXT_MESSAGE_CNF,//5
    SC_CI_SMS_PRIM_SET_PREFSTROGE_REQ,
    SC_CI_SMS_PRIM_SET_PREFSTROGE_CNF,
    SC_CI_SMS_PRIM_GET_PREFSTROGE_REQ,
    SC_CI_SMS_PRIM_GET_PREFSTROGE_CNF,
    SC_CI_SMS_PRIM_GET_SRCADDR_REQ,//10
    SC_CI_SMS_PRIM_GET_SRCADDR_CNF,
    SC_CI_SMS_PRIM_SEND_MSG_REQ,
    SC_CI_SMS_PRIM_SEND_MSG_CNF,
    SC_CI_SMS_PRIM_SEND_STROG_MSG_REQ,
    SC_CI_SMS_PRIM_SEND_STROG_MSG_CNF,//15
    SC_CI_SMS_PRIM_WRITE_MSG_REQ,
    SC_CI_SMS_PRIM_WRITE_MSG_CNF,
    SC_CI_SMS_PRIM_SET_SMSC_ADDR_REQ,
    SC_CI_SMS_PRIM_SET_SMSC_ADDR_CNF,
    SC_CI_SMS_PRIM_GET_SMSC_ADDR_REQ,//20
    SC_CI_SMS_PRIM_GET_SMSC_ADDR_CNF,
    SC_CI_SMS_PRIM_DEL_MSG_REQ,
    SC_CI_SMS_PRIM_DEL_MSG_CNF,
    SC_CI_SMS_PRIM_CONFIG_MSG_IND_REQ,
    SC_CI_SMS_PRIM_CONFIG_MSG_IND_CNF,
    SC_CI_SMS_PRIM_GET_MSG_IND_REQ,
    SC_CI_SMS_PRIM_GET_MSG_IND_CNF,
    SC_CI_SMS_PRIM_DEL_ONE_MSG_REQ,
    SC_CI_SMS_PRIM_DEL_ONE_MSG_CNF,
    SC_CI_SMS_PRIM_SET_NEW_MSG_IND_REQ,
    SC_CI_SMS_PRIM_SET_NEW_MSG_IND_CNF,
    SC_CI_SMS_PRIM_INVALID

}SC_CI_MSG_PRIMID;

/*API cmd type*/
typedef enum{
    SC_SMS_INVALID,
    SC_SMS_WRITE,
    SC_SMS_READ,
    SC_SMS_SND_CMGS,
    SC_SMS_SND_CMSS,
    SC_SMS_SND_CMGSEX,
    SC_SMS_DELALL,
    SC_SMS_DELONE,
    SC_SMS_NEWMSGIND,
    SC_SMS_SND_CMGL,
    SC_SMS_TYPE_MAX
}SC_SMS_CMD_TYPE;



typedef enum{
    SC_SMS_READY_IND,
    SC_MSG_STATUS_REPORT_IND,//+CDS
    SC_MSG_DELIVER_REPORT_IND,//CMT CDS

}SC_SMS_EVENT_TYPE;


typedef enum{
    SC_SMS_SUCESS,
    SC_SMS_TIMEOUT,
    SC_SMS_PARAM_INVALID,
    SC_SMS_UNKNOWN_ERR,
    SC_SMS_OS_ERR,
    SC_SMS_OPERATE_NOTSUPPORT,
    SC_SMS_BUSY,
    SC_SMS_INVALID_INDEX,
    SC_SMS_MEMORY_FULL,
    SC_SMS_UNREADY,
    SC_SMS_FAIL

}SC_SMSReturnCode;

typedef enum{
    SC_SMS_FREE,
    SC_SMS_READY,
    SC_SMS_READ_CMD,
    SC_SMS_DEL_CMD
}SC_SMS_Status;


/*the response structure*/
typedef struct{
    int primId;
    int resultCode;
    INT8 *repStr;
}SC_SMS_RESP;




typedef UINT8 SCsmsFormatMode;


//CiMsgPrimReadMessageCnf
typedef struct
{
	UINT8     tsYear;
	UINT8     tsMonth;
	UINT8     tsDay;
	UINT8     tsHour;
	UINT8     tsMinute;
	UINT8     tsSecond;
	UINT8     tsTimezone;
	UINT8     tsZoneSign;
}SCsmsTimeStamp;

/* Struct used in readSmsDeliverPdu() */
typedef struct {
    UINT32 type;                            // @field DCS type
    UINT32 msgClass;                   // @field message class (Only for RIL_DCSTYPE_GENERAL and RIL_DCSTYPE_MSGCLASS)
    UINT32 alphabet;                   // @field DCS alphabet
    UINT32 indication;                 // @field indication (Only for RIL_DCSTYPE_MSGWAIT)
} SCsmsDcs;

typedef struct{
    UINT8 rc;
    UINT8 indexorref;
}SCsmsWriteSendCnf;

typedef struct {
    UINT16 resultcode;
    SCsmsFormatMode formatmode;

    UINT8  status;
    UINT16 len;
    INT8 smscBuf[SC_SMS_MAX_ADDRESS_LENGTH * 2 + 1];
    INT8 data[SC_SMS_MAX_ADDRESS_LENGTH];
}SCsmsPduReadCnf;

typedef enum{
    SMS_DELIVER,
    SMS_SUBMIT
}Smstype;

typedef struct{
  
    UINT8         Length;                   /* Address Length in digits */
    UINT8 Digits[ SC_SMS_MAX_ADDRESS_LENGTH];  /* Address Digits */
	
} SCsmsAddressInfo;


typedef struct{
    UINT16 resultcode;
    UINT8  status;	
    SCsmsFormatMode fmtmode;
    Smstype type;
    UINT8  srcAddr[ SC_SMS_MAX_ADDRESS_LENGTH + 1];
    UINT8 Pid;
    UINT8 fo;
    UINT8 Dsc;
    UINT8 vp;
    SCsmsTimeStamp   timeStamp;
    SCsmsDcs bDCS;
    UINT32 msglen;
    INT8 msgbuf[512];
    SCsmsAddressInfo srvCenterAddr;
    
}SCsmsPayload;


typedef enum{
    SC_SMS_STROGE_ME = 1,
    SC_SMS_STROGE_SM = 3,
    SC_SMS_STROGE_MAX
}SC_SMS_Stroge;

typedef struct{
    UINT8 mem1;
    UINT8 mem2;
    UINT8 mem3;
}SCsmsStroge;



typedef struct{
    UINT8 used;
    UINT8 tatal;
    UINT8 storage;
}SCsmsStrogeInfoT;
typedef SCsmsStrogeInfoT SCsmsStrogeInfo[3];

BOOL getSmsReady(void);



/****************************************************************************
* FUNCTION
*  sAPI_SmsWriteMsg
*
* DESCRIPTION
*  Write a sms msg to sim card by text/pdu mode
*
* PARAMETERS
*  [in] fmatmode: The input and output format of the short messages
*  [in] sms: Message body
*  [in] smslen: The length of the message body
*  [in] srcAddr: Destination-Address, NULL for pdumode
*  [in] stat: 
*      1: stored unsent message
*      2: stored sent message
*  [in] msgQ: Results will be returned by msgQ
*
* RETURNS
*  SC_SMSReturnCode
*
* NOTE
*  msQ needs be created before using this API
*****************************************************************************/
SC_SMSReturnCode sAPI_SmsWriteMsg(SCsmsFormatMode fmatmode,UINT8* sms,UINT16 smslen,UINT8* srcAddr,UINT8 stat,sMsgQRef msgQ);

/****************************************************************************
* FUNCTION
*  sAPI_SmsSendMsg
*
* DESCRIPTION
*  send a msg by text/pdu mode
*
* PARAMETERS
*  [in] fmatmode: The input and output format of the short messages
*  [in] sms: Message body
*  [in] smslen: The length of the message body
*  [in] addr: Destination-Address, NULL for pdumode
*  [in] msgQ: Results will be returned by msgQ
*
* RETURNS
*  SC_SMSReturnCode
*
* NOTE
*  msQ needs be created before using this API
*****************************************************************************/
SC_SMSReturnCode sAPI_SmsSendMsg(SCsmsFormatMode fmatmode,UINT8* sms,UINT16 smslen,UINT8* addr,sMsgQRef msgQ);

/****************************************************************************
* FUNCTION
*  sAPI_SmsSendLongMsg
*
* DESCRIPTION
*  send multiple long sms in text mode
*
* PARAMETERS
*  [in] sms: Message body
*  [in] smslen: The length of the message body
*  [in] addr: Destination-Address, NULL for pdumode
*  [in] msgQ: Results will be returned by msgQ

*
* RETURNS
*  SC_SMSReturnCode
*
* NOTE
*  msQ needs be created before using this API
*****************************************************************************/
SC_SMSReturnCode sAPI_SmsSendLongMsg(UINT8 * sms, UINT16 smslen, UINT8 * addr, sMsgQRef msgQ);

/****************************************************************************
* FUNCTION
*  sAPI_SmsSendStorageMsg
*
* DESCRIPTION
*  send a msg which is already stored in sim card
*
* PARAMETERS
*  [in] msgIndex: Value in the range of location numbers supported by the associated memory and start with one
*  [in] addr: Destination-Address
*  [in] msgQ: Results will be returned by msgQ
*
* RETURNS
*  SC_SMSReturnCode
*
* NOTE
*  msQ needs be created before using this API
*****************************************************************************/
SC_SMSReturnCode sAPI_SmsSendStorageMsg(INT32 msgIndex,UINT8* addr,sMsgQRef msgQ);

/****************************************************************************
* FUNCTION
*  sAPI_SmsReadMsg
*
* DESCRIPTION
*  Read a msg which is already stored in sim card
*
* PARAMETERS
*  [in] fmatmode: The input and output format of the short messages
*  [in] msgIndex: Value in the range of location numbers supported by the associated memory and start with one
*  [in] msgQ: Results will be returned by msgQ
*
* RETURNS
*  SC_SMSReturnCode
*
* NOTE
*  msQ needs be created before using this API
*  if the msg index is empty just return ok
*****************************************************************************/
SC_SMSReturnCode sAPI_SmsReadMsg(SCsmsFormatMode fmatmode,INT32 msgIndex,sMsgQRef msgQ);

/****************************************************************************
* FUNCTION
*  sAPI_SmsDelAllMsg
*
* DESCRIPTION
*  Delte all msg which is already stored in sim card
*
* PARAMETERS
*  [in] msgQ: Results will be returned by msgQ
*
* RETURNS
*  SC_SMSReturnCode
*
* NOTE
*  msQ needs be created before using this API
*****************************************************************************/
SC_SMSReturnCode sAPI_SmsDelAllMsg(sMsgQRef msgQ);

/****************************************************************************
* FUNCTION
*  sAPI_SmsDelOneMsg
*
* DESCRIPTION
*  Delte one msg which is already stored in sim card
*
* PARAMETERS
*  [in] msgQ: Results will be returned by msgQ
*
* RETURNS
*  SC_SMSReturnCode
*
* NOTE
*  msQ needs be created before using this API
*****************************************************************************/
SC_SMSReturnCode sAPI_SmsDelOneMsg(INT32 msgIndex, sMsgQRef msgQ);


/****************************************************************************
* FUNCTION
*  sAPI_SmsSetNewMsgInd
*
* DESCRIPTION
*  Set the way of new msg report
*
* PARAMETERS
*  [in] fmatmode: The input and output format of the short messages
*  [in] mode,[in] mt,[in] bm,[in] ds,[in] bfr
*  Please choose one of combination(order: mode,mt,bm,ds,bfr):
*  First : (1,2,1,0,0,0) --- stor new msg to SIM card, new msg index will report to: SC_URC_NEW_MSG_IND
*  Second: (1,1,2,0,0,0) --- Report the sms msg directly, new text msg content report to: SC_URC_FLASH_MSG
*
* RETURNS
*  SC_SMSReturnCode
*
* NOTE
*   1. Do not using any other para combinations, it may cause Unknown ERROR!!!!!!
*   2. Please using this api when system booting.
*****************************************************************************/
SC_SMSReturnCode sAPI_SmsSetNewMsgInd(INT32 fmatmode, INT32 mode, INT32 mt, INT32 bm, INT32 ds, INT32 bfr);

/****************************************************************************
* FUNCTION
*  sAPI_SmsGetNewMsgInd
*
* DESCRIPTION
*  Get the way of new msg report
*
* PARAMETERS
*  [out] mode,[out] mt,[out] bm,[out] ds,[out] bfr
*  Please choose one of combination(order: mode,mt,bm,ds,bfr):
*  First : (1,2,1,0,0,0) --- stor new msg to SIM card, new msg index will report to: SC_URC_NEW_MSG_IND
*  Second: (1,1,2,0,0,0) --- Report the sms msg directly, new text msg content report to: SC_URC_FLASH_MSG
*
* RETURNS
*  SC_SMSReturnCode
*
* NOTE
*   1. Please using this api when system booting.
*****************************************************************************/
SC_SMSReturnCode sAPI_SmsGetNewMsgInd(UINT32 *pmode, UINT32 *pmt, UINT32 *pbm, UINT32 *pds, UINT32 *pbfr);


/****************************************************************************
* FUNCTION
*  sAPI_SmsgetChset
*
* DESCRIPTION
*  Example Query the format of the current device
*
* PARAMETERS
*  [in] pchset_type: Gets set format
*
* RETURNS
*  SC_SMSReturnCode
*
*****************************************************************************/
SC_SMSReturnCode sAPI_SmsgetChset(UINT8 *pchset_type);


/****************************************************************************
* FUNCTION
*  sAPI_SmssetChset
*
* DESCRIPTION
*  Set the Chset
*
* PARAMETERS
*  [in] pchset_type: set Chset
*
* RETURNS
*  SC_SMSReturnCode
*
*****************************************************************************/
SC_SMSReturnCode sAPI_SmssetChset(UINT8 pchset_type);

/****************************************************************************
* FUNCTION
*  sAPI_SmsSetFormat
*
* DESCRIPTION
*  Set the format
*
* PARAMETERS
*  [in] sms_format: set format
*
* RETURNS
*  SC_SMSReturnCode
*
*****************************************************************************/
SC_SMSReturnCode sAPI_SmsgetFormat(UINT8      *sms_format);


/****************************************************************************
* FUNCTION
*  sAPI_SmsSetFormat
*
* DESCRIPTION
*  Get the character set format
*
* PARAMETERS
*  [in] sms_format: get format
*
* RETURNS
*  SC_SMSReturnCode
*
*****************************************************************************/

SC_SMSReturnCode sAPI_SmsSetFormat(UINT8      sms_format);


/****************************************************************************
* FUNCTION
*  sAPI_SmsListMsg
*
* DESCRIPTION
*  List the current storage space SMS
*
* PARAMETERS
*  [in] fmatmode: The input and output format of the short messages
*  [in] mode: Select a mode for listing SMS messages
*  [in] msgQ: Results will be returned by msgQ
*
* RETURNS
*  SC_SMSReturnCode
*
*****************************************************************************/
SC_SMSReturnCode sAPI_SmsListMsg(INT32 mode);

/****************************************************************************
* FUNCTION
*  sAPI_SmsDecodePdu
*
* DESCRIPTION
*  PDU to TXT
*
* PARAMETERS
* @param[in]  {char} *pduText pdu data
* @param[in]  {char *} outAddr phone number
* @param[out]  {int *} outTextLen Converted text data
* @param[out]  {char *} outText The converted text data length
*
* RETURNS
*  SC_SMSReturnCode
*
*****************************************************************************/
SC_SMSReturnCode sAPI_SmsDecodePdu(char *pduText, char * outAddr, int * outTextLen, char * outText);


/**
 * @brief text转pdu
 * @param[in]  {char} *destAddr 电话号码
 * @param[in]  {char} *contentText 短信内容
 * @param[in]  {int} contentLen 短信内容的长度
 * @param[in]  {char *} pduText pdu数据的最大长度
 * @param[out]  {int *} pduLen 转换后的pdu数据的实际长度
 * @param[in]  {SmsHeaderInfo} *header 长短信的信息结构体  
 * @return {*} 执行结果 0 成功  others 失败
 */
/****************************************************************************
* FUNCTION
*  sAPI_SmsEncodePdu
*
* DESCRIPTION
*  TXT to pdu
*
* PARAMETERS
* @param[in]  {char} *destAddr phone number
* @param[in]  {char *} *contentText 短信内容
* @param[in]  {int} contentLen 短信内容的长度
* @param[out]  {int *} outTextLen Converted text data
* @param[out]  {char *} outText The converted text data length
*
* RETURNS
*  SC_SMSReturnCode
*
*****************************************************************************/

SC_SMSReturnCode sAPI_SmsEncodePdu(char *addrStr, char *pSmsData, int smsDataLength, char * pduText, int * pduLen);



#endif







