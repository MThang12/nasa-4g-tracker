#ifndef SIMCOM_SIMCARD_H
#define SIMCOM_SIMCARD_H
#include "simcom_os.h"

#define SC_SIM_MAX_CMD_DATA_SIZE    261
#define SC_NULL_TERMINATOR_LENGTH   1

#define SC_GET_CPIN_MASK                (0x01<<SC_GET_CPIN)
#define SC_GET_IMSI_MASK                (0x01<<SC_GET_IMSI)
#define SC_GET_HPLMN_MASK               (0x01<<SC_GET_HPLMN)
#define SC_SET_CPIN_MASK                (0x01<<SC_SET_CPIN)
#define SC_SET_CRSM_MASK                (0x01<<SC_SET_CRSM)
#define SC_SET_CLCK_MASK                (0x01<<SC_SET_CLCK)
#define SC_SET_CSIM_MASK                (0x01<<SC_SET_CSIM)
#define SC_GET_CNUM_MASK                (0x01<<SC_GET_CNUM)

enum SCsimcardid
{
    SC_GET_CPIN=0,
    SC_GET_IMSI,
    SC_GET_HPLMN,
    SC_SET_CPIN,
    SC_SET_CRSM,
    SC_SET_CLCK,
    SC_SET_CSIM,
    SC_GET_CNUM
};
typedef enum {
    SC_SIM_RETURN_SUCCESS,
    SC_SIM_RETURN_FAIL,
    SC_SIM_RTEURN_UNKNOW
}SC_simcard_err_e;

typedef enum{
    SC_SIMCARD_INVALID,
    SC_SIMCARD_SWITCHCARD,
    SC_SIMCARD_HOTPLUG,
    SC_SIMCARD_URCIND,
    SC_SIMCARD_HOTPLUG_EX,
    SC_SIMCARD_TYPE_MAX
}SC_SIMCARD_CMD_TYPE;

typedef enum{
    SC_HOTSWAP_INVALID,
    SC_HOTSWAP_QUERY_STATE,
    SC_HOTSWAP_QUERY_LEVEL,
    SC_HOTSWAP_SET_SWITCH,
    SC_HOTSWAP_SET_LEVEL,
    SC_HOTSWAP_QUERY_OUTSTATE,
    SC_HOTSWAP_SET_OUTSWITCH,
    SC_HOTSWAP_TYPE_MAX
}SC_HotSwapCmdType_e;

typedef struct
{
  SC_HotSwapCmdType_e option;
  int SetParam;
}HotSwapMsg_st;

typedef struct
{
  UINT32 status;
  UINT32 mask;
}UrcIndMsg_st;

typedef struct
{
  BOOL PlugInEnable;       /*SIM1 1:plug in is enabled 0: plug in is disabled*/
  BOOL trigger;            /*SIM1 1:high level trigger 0:Low level trigger*/
  BOOL PlugOutEnable;      /*SIM1 1:plug out is enabled 0: plug out is disable*/
  BOOL PlugInEnable_1;     /*SIM2 1:plug in is enabled 0: plug in is disabled*/
  BOOL trigger_1;          /*SIM2 1:high level trigger 0:Low level trigger*/
  BOOL PlugOutEnable_1;    /*SIM2 1:plug out is enabled 0: plug out is disable*/
}HotSwapFileType_st;

enum scSimcardState
{
  SC_SIM_READY = 0,
  SC_SIM_PIN,
  SC_SIM_PUK,
  SC_SIM_BLOCKED,
  SC_SIM_REMOVED,
  SC_SIM_CRASH,
  SC_SIM_NOINSRETED,
  SC_SIM_UNKNOW
};

typedef struct
{
  char spn[64];/*Service provider name from SIM*/
  char mcc[4];/*Mobile Country Code*/
  char mnc[4];/*Mobile Network Code*/
}Hplmn_st;

typedef struct
{
  int sw1;
  int sw2;
  char data[2 * 261 + 50];
}CrsmResponse_st;

typedef struct
{
  int length;
  char data[2 * 261 + 50];
}CsimResponse_st;

typedef struct StkResultTag
{
    int v_state;/*0:succeed , -1: faild*/
    UINT8 a_buf[SC_SIM_MAX_CMD_DATA_SIZE];
}StkResult;

typedef enum scStkOptTag{
    SC_STK_CMD_ENABLE_SIMAT              = 0,
    SC_STK_CMD_DOWNLOAD_PROFILE          = 1,
    SC_STK_CMD_GET_CAP_INFO              = 2,
    SC_STK_CMD_GET_PROFILE               = 3,
    SC_STK_CMD_SEND_ENVELOPE             = 4,
    SC_STK_CMD_PROACTIVE                 = 11,
    SC_STK_CMD_END_SESSION               = 14,
    SC_STK_CMD_ENABLE_AUTORESP_PROACTIVE = 15,
    SC_STK_TYPE_INVALID
}_scStkOpt;

typedef INT32 scStkOpt; /*refer _scStkOpt*/

typedef struct
{
    scStkOpt opt;
    char StkData[SC_SIM_MAX_CMD_DATA_SIZE * 2 + SC_NULL_TERMINATOR_LENGTH];
    INT32 len;
}StkMsg_st;

typedef SC_simcard_err_e SC_SIM_RETURN_ERRCODE;

SC_simcard_err_e sAPI_SimcardHotSwapMsg(SC_HotSwapCmdType_e opt, UINT8 param, sMsgQRef msgQ);
SC_simcard_err_e sAPI_SimcardSwitchMsg(UINT8 status, sMsgQRef msgQ);
SC_simcard_err_e sAPI_SimcardPinSet(char * oldpin, char * newpin,int new);
SC_simcard_err_e sAPI_SimcardPinGet(UINT8 *gcpin);
SC_simcard_err_e sAPI_SysGetIccid(char *IccidValue);
SC_simcard_err_e sAPI_SysGetImsi(char *ImsiValue);
SC_simcard_err_e sAPI_SysGetHplmn(Hplmn_st *HplmnValue);
SC_simcard_err_e sAPI_SysGetBindSim(UINT8 *bindsim);
SC_simcard_err_e sAPI_SysSetBindSim(UINT8 bindsim);
SC_simcard_err_e sAPI_SysGetDualSimType(UINT8 *type);
SC_simcard_err_e sAPI_SysSetDualSimType(UINT8 type);
#if defined (LWIP_IPNETBUF_SUPPORT) && defined (FEATURE_SIMCOM_DUALSIM)
/*
 *@API: sAPI_SysGetDualSimURC
 *@brief: get DUALSIM_URC flag value
 *@param: type---recieve flag value
 *@retval: 0:SC_SIM_RETURN_SUCCESS
           1:SC_SIM_RETURN_FAIL
 *@note: only retval = 0, type can get value
 *  type value: 0---only SIM1 report
                1---only SIM2 report
                2---SIM1&SIM2 report
 */
SC_simcard_err_e sAPI_SysGetDualSimURC(UINT8 *type);
/*
 *API: sAPI_SysSetDualSimURC
 *@brief: set DUALSIM_URC flag value
 *@param: type---flag value will be set
 *@retval: 0:SC_SIM_RETURN_SUCCESS
           1:SC_SIM_RETURN_FAIL
 *@note: type can be set please refer to sAPI_SysGetDualSimURC note
 */
SC_simcard_err_e sAPI_SysSetDualSimURC(UINT8 type);
#endif
SC_simcard_err_e sAPI_SimRestrictedAccess(int cmd, int fileId, int p1, int p2, int p3, char *data, char *pathid, CrsmResponse_st *response);
SC_simcard_err_e sAPI_SimGenericAccess(int cmdLen, char *cmdStr, CsimResponse_st *response);
SC_simcard_err_e sAPI_SimLockSet(int mode, char *pin);
SC_simcard_err_e sAPI_SysGetSubscriber(char *SubscriberValue);
SC_simcard_err_e sAPI_StkService(scStkOpt opt,char *data,StkResult *output);

void sc_Set_URCtask_flag(UINT8 value);
/**
  * @brief  query SIM status
  * @param  simId: 0 query SIM1
  *                1 query SIM2
  *         gcpin: receive sim status, if simID=0/1, the value of gcpin is SIM1/2.
  * @note   none
  * @retval 0: SC_SIM_RETURN_SUCCESS
  *         1: SC_SIM_RETURN_FAIL
  *         2: SC_SIM_RTEURN_UNKNOW
  */
SC_simcard_err_e sAPI_SimcardPinGet_Ex(BOOL simId, UINT8 *gcpin);
/**
  * @brief  config simcard hot plug funtion
  * @param  simId: 0 config SIM1
  *                1 config SIM2
  *         opt:   please see SC_HotSwapCmdType_e
  *         param: If opt is a query, param will be used to receive query values
  *                If opt is set, param is used to transfer the set value
  * @note   none
  * @retval 0: SC_SIM_RETURN_SUCCESS
  *         1: SC_SIM_RETURN_FAIL
  *         2: SC_SIM_RTEURN_UNKNOW
  */
SC_simcard_err_e sAPI_SimcardHotSwap_Ex(BOOL simId, SC_HotSwapCmdType_e opt, UINT8 *param);

extern BOOL gStkApiTag;
extern void scStkReturn(StkResult *result);
#endif
