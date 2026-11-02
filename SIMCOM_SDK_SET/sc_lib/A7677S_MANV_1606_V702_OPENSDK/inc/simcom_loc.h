

#ifndef _SIMCOM_LOC_H_
#define _SIMCOM_LOC_H_


#include "simcom_os.h"


typedef enum {

  SC_LBS_SUCCESS,
  SC_LBS_FAIL, 
  SC_LBS_INVALID_PARAMETER,
  SC_LBS_SIMCARD_NOT_READY,
  SC_LBS_RESULT_NETWORK_ERROR,
  SC_LBS_GET_LOC_FAIL,
  SC_LBS_ERROR_END
}SC_lbs_err_e;

typedef enum{
    SC_LBS_GET_LONLAT           = 1,
    SC_LBS_GET_DETAILADDRESS    = 2,
    SC_LBS_GET_ERRNO            = 3,
    SC_LBS_GET_LONLATTIME       = 4,
    SC_LBS_TEST                 = 5,
    SC_LBS_TYPE_SET             = 6,
    SC_LBS_SERVER_SET           = 7,
    SC_LBS_DEMO_MAX             = 99
}SC_LBS_DEMO_TYPE;

typedef struct
{
    unsigned char ver[6];                     ///<version -6 byte
    unsigned long  u32SequId;                ///<sequence_id - 4 byte
    unsigned char   u8ErrorCode;              ///<ret - 1 byte
    unsigned long   u32Lng;                  ///<lng - 4 byte
    unsigned long   u32Lat;                  ///<lat - 4 byte
    unsigned short  u16Acc;                   ///<acc - 2 byte
    unsigned char   u8LocAddress[150];          ///<addr - SIMCOM_LBS_DETAILADDR_LEN
    unsigned long   u32AddrLen;                                      ///<addrlen - 4 byte
    unsigned char  u8DateAndTime[40];         ///<dataandtime - SIMCOM_LBS_DATAANDTIME_LEN
    unsigned long  u32QueryAmount;                                   ///<amout - 4 byte
}SC_lbs_info_t;



typedef  SC_lbs_err_e SC_LBS_RETURNCODE;
/*****************************************************************************
* FUNCTION
*  sAPI_LocGet
*
* DESCRIPTION
*  sAPI_LocGet is used to get current location 
*  
* PARAMETERS
*  channel: data transmission channel
*       0 - serial port
*       1 - usb
*  magQ_urc: the message queue for urc report, it is valid duiring whole http service.
*  type:A numeric parameter which specifies the location type.
*       1  use 3 cell’s information
*       2  get detail address
*       3  get access times
*       4  get longitude latitude and date time
*       9  report positioning error
*
* RETURNS
*    
*
* NOTE
*  
*****************************************************************************/
SC_LBS_RETURNCODE sAPI_LocGet(int channel, sMsgQRef magQ_urc,int type);

SC_LBS_RETURNCODE sAPI_LocTypeSet(int LocType);
SC_LBS_RETURNCODE sAPI_LocServerSet(char *server);

#endif




