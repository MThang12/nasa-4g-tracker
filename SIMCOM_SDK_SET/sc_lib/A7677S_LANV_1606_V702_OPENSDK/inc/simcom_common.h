#ifndef __SIMCOM_COMMON_H__
#define __SIMCOM_COMMON_H__

#include "simcom_os.h"


/* msg.arg1 */
#define SC_MODULE_NONE               (0)
#define SC_URC_PB_MASK               (0x01 << 0)
#define SC_URC_SMS_MASK              (0x01 << 1)
#define SC_URC_SIM_MASK              (0x01 << 2)
#define SC_URC_NETSTATUE_MASK        (0x01 << 3)
#define SC_URC_INTERNAL_AT_RESP_MASK (0x01 << 4)
#define SC_URC_WIFI_SCAN_MASK		 (0x01 << 5)
#define SC_URC_GNSS_MASK	    	 (0x01 << 6)
#define SC_URC_CALL_MASK	    	 (0x01 << 7)
#define SC_URC_AUDIO_MASK            (0x01 << 8)
#define SC_URC_TTS_MASK              (0x01 << 9)
#define SC_URC_SIM2_MASK             (0x01 << 10)
#define SC_URC_SMS2_MASK              (0x01 << 11)
#define SC_URC_CALL2_MASK	    	 (0x01 << 12)


#define SC_MODULE_ALL              (0XFFFFFFFF)

/* msg.arg2 */
enum sim_urc_code
{
    /*URC_PB_MASK*/
    SC_URC_PBDOWN          = 0,

    /*URC_SMS_MASK*/
    SC_URC_SMSDWON,
    SC_URC_SMSFULL,
    SC_URC_STATUS_REPORT,
    SC_URC_NEW_MSG_IND,
    SC_URC_FLASH_MSG,
    SC_URC_LIST_MSG,

    /*URC_CPIN_MASK*/
    SC_URC_CPIN_READY,
    SC_URC_CPIN_REMOVED,
    SC_URC_SIM_IS_LOCK,
    SC_URC_SIM_IS_BLOCKD,
    SC_URC_SIM_IS_CRASH,

    /*URC_NETSTATUE_MASE*/
    SC_URC_NETACTED,
    SC_URC_NETDIS,
    SC_URC_PDP_ACTIVE,
    SC_URC_PDP_DEACT,
    SC_URC_USSD,
	/*URC_WIFI_SCAN*/
	SC_URC_WIFI_SCAN,

    /*URC_GNSS_MASK*/
    SC_URC_GPS_INFO,
    SC_URC_GNSS_INFO,
    SC_URC_NMEA_DATA,
    #ifdef SC_DRIVER_GNSS_UNIC
    SC_URC_APFLASH_DATA_GET,
    SC_URC_APFLASH_DATA_INSTALL,
    #endif
    #ifndef SC_DRIVER_GNSS_ZKW
    SC_URC_AGNSS_DATA_CHK,
    #endif

    /*URC_CALL_MASK*/
    SC_URC_RING_IND,
    SC_URC_DTMF_IND,
    SC_URC_CALL_LIS_IND,

    /*URC_AUDIO_MASK*/
    SC_URC_AUDIO_START,
    SC_URC_AUDIO_STOP,
    SC_URC_CREC_START,
    SC_URC_CREC_STOP,
    SC_URC_CREC_FIFULL,

    /*URC_TTS_MASK*/
    SC_URC_TTS_IND,
};

enum appChannelType{
    SC_SRV_NONE = 50,
    SC_SRV_FTPS,
    SC_SRV_HTTPS,
    SC_SRV_TCPIP,
    SC_SRV_MQTT,
    SC_SRV_HTP,
    SC_SRV_NTP,
    SC_SRV_SSL,
    SC_SRV_CCH,
    SC_SRV_FOTA,
    SC_SRV_FS,
	SC_SRV_LBS,
    SC_SRV_MAX
};


enum sim_msg_id
{
    SIM_MSG_INIT = 0,
    SRV_CREATER,
    SRV_SELF,
    SRV_NET_CONTROL,
    SRV_URC,
    SRV_UART,
    SRV_UART3,
    SRV_USB_VCOM,
    SRV_SMS,
    SIM_MSG_MAX,
};

#define MAKE_MSG_ID_BY_SRV(srv) (UINT32)(SRV_CREATER & ((UINT32)srv << 8))
#define GET_SRV_FROM_MSG_ID(id) (UINT8)(0xff & ((UINT32)id >> 8))
#define IS_SRV_CREATER(id) (SRV_CREATER == (SRV_CREATER & id))


/**
 * Typically, The urc is a char array or a string constant.
 * If use the malloc space, please malloc and free the space by yourself.
 *
 * @method  sAPI_SendUrc
 *
 * @param   urc  The urc string.
 *
 * @return  0 is OK.
 */
int sAPI_SendUrc(UINT32 msgid,int urc_mask,int urccode,void *arg);


/**
 * The registerd message reference will be receive the URC.
 *
 * @method  sAPI_UrcRefRegister
 *
 * @param   ref  message reference that will be receive the URC.
 *
 * @return  0 is OK.
 */
int sAPI_UrcRefRegister(sMsgQRef ref, UINT32 mask);

/**
 * If no longer handle the URC, please release the message reference.
 *
 * @method  sAPI_UrcRefRelease
 *
 * @param   ref  message reference that no longer handle the URC.
 *
 * @return  0 is OK.
 */
int sAPI_UrcRefRelease(sMsgQRef ref);



#endif

