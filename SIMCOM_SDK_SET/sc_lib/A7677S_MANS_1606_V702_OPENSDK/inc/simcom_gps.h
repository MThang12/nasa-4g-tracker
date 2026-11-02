/** 
* @file         simcom_gps.h 
* @brief        SIMCom GNSS API
* @author       HT
* @date         2019/9/13
* @version      V1.0.0 
* @par Copyright (c):  
*               SIMCom Co.Ltd 2003-2019
* @par History: 1:Create
*
*/

#ifndef __SIMCOM_GPS_H__
#define __SIMCOM_GPS_H__

#include "simcom_os.h"
/****************************************************************************
    Define enum  
*****************************************************************************/
typedef enum {
    SC_GNSS_STATE_INITIAL_SUCCESS             = 0,
    SC_GNSS_STATE_INITIAL_FAILED              = 1,
    SC_GNSS_STATE_INITIALED                   = 2,
    SC_GNSS_STATE_DOWNLOAD_SUCCESS            = 3,
    SC_GNSS_STATE_DOWNLOAD_FAIL               = 4,
    SC_GNSS_STATE_SEND_DATA_SUCCESS           = 5,
    SC_GNSS_STATE_SEND_DATA_FAIL              = 6,
    SC_GNSS_STATE_DEINIT_SUCCESS,
    SC_GNSS_STATE_DEINIT_FAIL,
}SC_Gnss_State_Type;

typedef enum
{
    SC_GNSS_RETURN_CODE_OK                =  0,
    SC_GNSS_RETURN_CODE_ERROR             = -1,

    SC_GNSS_CODE_SOCKRT_ERROR             = 101,
    SC_GNSS_CODE_GET_SERVER_ERROR         = 102,
    SC_GNSS_CODE_CONNECT_ERROR            = 103,
    SC_GNSS_CODE_WRITE_SOCKET_ERROR       = 104,
    SC_GNSS_CODE_READ_SOCKET_ERROR        = 105,
    SC_GNSS_CODE_GET_DATA_ERROR           = 106,
    SC_GNSS_CODE_SEND_DATA_ERROR          = 107,
    SC_GNSS_CODE_STORE_AGPSDATA_ERROR     = 108,
    SC_GNSS_CODE_AIDTIME_ERROR            = 109,
    SC_GNSS_CODE_AIDPOS_ERROR             = 110,

    SC_GNSS_CODE_MAX                      =0xFF
}SC_Gnss_Return_Code;

typedef enum
{
    SC_GNSS_POWER_OFF = 0,
    SC_GNSS_POWER_ON =  1
}SC_Gnss_Power_Status;

typedef enum
{
    SC_GNSS_AP_FLASH_OFF = 0,
    SC_GNSS_AP_FLASH_ON =  1
}SC_Gnss_Ap_Flash_Status;

typedef enum
{
    SC_GNSS_AP_FLASH_DATA_GET_FAIL                  = 0,
    SC_GNSS_AP_FLASH_DATA_GET_SUCCESS               = 1
}SC_Gnss_Ap_Flash_Data_Get_Status;

typedef enum
{
    SC_GNSS_AP_FLASH_DATA_INSTALL_FAIL             = 0,
    SC_GNSS_AP_FLASH_DATA_INSTALL_SUCCESS          = 1,
    SC_GNSS_AP_FLASH_DATA_INSTALL_DATA_NOT_EXIST   = 2
}SC_Gnss_Ap_Flash_Data_Install_Status;

typedef enum
{
    SC_GNSS_STOP_OUTPUT_NMEA_DATA  = 0,
    SC_GNSS_START_OUTPUT_NMEA_DATA = 1
}SC_Gnss_Output_Control;

typedef enum
{
    SC_GNSS_NMEA_DATA_GET_BY_PORT = 0,   //get data by NMEA port
    SC_GNSS_NMEA_DATA_GET_BY_URC  = 1   //get data by URC report
}SC_Gnss_Nmea_Data_Get;

typedef enum
{
    SC_GNSS_START_HOT        = 0,  /*0:hot 1:warm 2:cold*/
    SC_GNSS_START_WARM       = 1,
    SC_GNSS_START_COLD       = 2,
    SC_GNSS_START_MAX
}SC_Gnss_Start_Mode;

typedef enum
{
#ifdef JACANA_GPS_SUPPORT
    SC_GNSS_BAUD_RATE_115200       = 115200,
#elif SC_DRIVER_GNSS_ZKW
    SC_GNSS_BAUD_RATE_4800         = 4800,
    SC_GNSS_BAUD_RATE_9600         = 9600,
    SC_GNSS_BAUD_RATE_19200        = 19200,
    SC_GNSS_BAUD_RATE_38400        = 38400,
    SC_GNSS_BAUD_RATE_57600        = 57600,
    SC_GNSS_BAUD_RATE_115200       = 115200,
#else //SIMCOM_UNICORECOMM_GNSS_SUPPORT
    SC_GNSS_BAUD_RATE_9600         = 9600,
    SC_GNSS_BAUD_RATE_115200       = 115200,
    SC_GNSS_BAUD_RATE_230400       = 230400,
#endif
    SC_GNSS_BAUD_RATE_MAX
}SC_Gnss_Baud_Rate;

typedef enum
{
#if defined(JACANA_GPS_SUPPORT) || defined(SC_DRIVER_GNSS_ZKW)
    SC_GNSS_MODE_GPS                               = 1,
    SC_GNSS_MODE_BDS                               = 2,
    SC_GNSS_MODE_GPS_BDS                           = 3,
    SC_GNSS_MODE_GLONASS                           = 4,
    SC_GNSS_MODE_GPS_GLONASS                       = 5,
    SC_GNSS_MODE_BDS_GLONASS                       = 6,
    SC_GNSS_MODE_GPS_BDS_GLONASS                   = 7,
#else //SIMCOM_UNICORECOMM_GNSS_SUPPORT
#ifdef FEATURE_SIMCOM_GPS_OVERSEAS
    SC_GNSS_MODE_GPS_L1_SBAS_QZSS                  = 1,
    SC_GNSS_MODE_BDS                               = 2,
    SC_GNSS_MODE_GPS_GLONASS_GALILEO_SBAS_QZSS     = 3,
    SC_GNSS_MODE_GPS_BDS_GALILEO_SBAS_QZSS         = 4,
#else
    SC_GNSS_MODE_GPS_BDS_QZSS                      = 1,
    SC_GNSS_MODE_BDS                               = 2,
    SC_GNSS_MODE_GPS_QZSS                          = 3,
#endif
#endif
    SC_GNSS_MODE_MAX
}SC_Gnss_Mode;

typedef enum
{
#ifdef JACANA_GPS_SUPPORT
    SC_GNSS_NMEA_UPDATE_RATE_1HZ        = 1,
    SC_GNSS_NMEA_UPDATE_RATE_2HZ        = 2,
    SC_GNSS_NMEA_UPDATE_RATE_3HZ        = 3,
    SC_GNSS_NMEA_UPDATE_RATE_4HZ        = 4,
    SC_GNSS_NMEA_UPDATE_RATE_5HZ        = 5,
#elif SC_DRIVER_GNSS_ZKW
    SC_GNSS_NMEA_UPDATE_RATE_1HZ        = 1,
    SC_GNSS_NMEA_UPDATE_RATE_2HZ        = 2,
    SC_GNSS_NMEA_UPDATE_RATE_4HZ        = 4,
    SC_GNSS_NMEA_UPDATE_RATE_5HZ        = 5,
    SC_GNSS_NMEA_UPDATE_RATE_10HZ       = 10,
#else //SIMCOM_UNICORECOMM_GNSS_SUPPORT
    SC_GNSS_NMEA_UPDATE_RATE_1HZ        = 1,
    SC_GNSS_NMEA_UPDATE_RATE_2HZ        = 2,
    SC_GNSS_NMEA_UPDATE_RATE_5HZ        = 5,
#endif
    SC_GNSS_NMEA_RATE_MAX
}SC_Gnss_Nmea_Rate;

typedef enum
{
    SC_GNSS_DYNAMIC_LOAD_FILE_COMMON_TYPE = 0,
    SC_GNSS_DYNAMIC_LOAD_FILE_CUSTOM_TYPE = 1,
    SC_GNSS_DYNAMIC_LOAD_FILE_ERROR_TYPE  = -1
}SC_Gnss_Dynamic_Load_File_Type;

typedef enum
{
    SC_AGNSS_DATA_SAVE_TO_GPSCHIP          = 0,
    SC_AGNSS_DATA_SAVE_TO_RAM              = 1,
    SC_AGNSS_DATA_SAVE_TO_FLASH            = 2,
    SC_AGNSS_DATA_SAVE_DELETE              = 3,

    SC_AGNSS_DATA_SAVE_MAX
}SC_Agnss_Data_Save_Method;

typedef enum
{
    SC_AGNSS_DATA_SEND_METHOD_RAM          = 1,
    SC_AGNSS_DATA_SEND_METHOD_FLASH        = 2,

    SC_AGNSS_DATA_SEND_METHOD_MAX
}SC_Agnss_Data_Send_Method;

typedef enum
{
    SC_PREDICTING_EPH_DATA_DOWNLOAD_AND_UPDATE  = 1,
    SC_PREDICTING_EPH_DATA_UPDATE               = 2,

    SC_PREDICTING_EPH_DATA_OPT_MAX
}SC_Predicting_Eph_Data_Opt;

/****************************************************************************
    Define struct 
*****************************************************************************/
#define MAX_SV_COUNT11              30
#define GPS_FTM_PRN_LEN             5
#define GPS_FTM_CN_LEN              5

#define GPS_INFO_LAT_LEN            15
#define GPS_INFO_LON_LEN            15
#define GPS_INFO_DATE_LEN           8
#define GPS_INFO_TIME_LEN           13
#define GPS_INFO_ALT_LEN            8
#define GPS_INFO_SPEED_LEN          8
#define GPS_INFO_COURSE_LEN         8
#define GPS_INFO_NS_LEN             3
#define GPS_INFO_WE_LEN             3
#define GPS_INFO_PDOP_LEN           7
#define GPS_INFO_HDOP_LEN           7
#define GPS_INFO_VDOP_LEN           7
#define GPS_INFO_FIXMODE_LEN        3
#define GPS_INFO_PROD_LEN           20
#define GPS_INFO_VERS_LEN           30
#define GPS_INFO_STAENUM_LEN        4
#define GPS_INFO_SATE_LEN           5
#define GSV_ARRY                    4
#define AGPS_AID_INFO_LEN           14

#ifdef SC_DRIVER_GNSS_ZKW
#define GPS_INFO_ANT_LEN            15
#endif

typedef struct {
    char prn[GPS_FTM_PRN_LEN];
    char cn[GPS_FTM_CN_LEN];
}SC_SATE;

typedef struct {
    int sv_num;
    SC_SATE  gps_sv[MAX_SV_COUNT11];
}SC_SATE_T;

typedef struct {
    char sate_svs[GPS_INFO_STAENUM_LEN];
}SC_SATE_NUM_T;

typedef struct {
    char time[GPS_INFO_TIME_LEN];
    char lat[GPS_INFO_LAT_LEN];
    char lat_ns[GPS_INFO_NS_LEN];
    char lon[GPS_INFO_LON_LEN];
    char lon_we[GPS_INFO_WE_LEN];
    char date[GPS_INFO_DATE_LEN];
    char alt[GPS_INFO_ALT_LEN];
    char speed[GPS_INFO_SPEED_LEN];
    char course[GPS_INFO_COURSE_LEN];
    char pdop[GPS_INFO_PDOP_LEN];
    char hdop[GPS_INFO_HDOP_LEN];
    char vdop[GPS_INFO_VDOP_LEN];
    char fix_mode[GPS_INFO_FIXMODE_LEN];
    char prod[GPS_INFO_PROD_LEN];
    char model[GPS_INFO_PROD_LEN];
    char vers[GPS_INFO_VERS_LEN];
    char valid_sate_num[GPS_INFO_SATE_LEN];
    char gpsrs[AGPS_AID_INFO_LEN];
    char gpsus[AGPS_AID_INFO_LEN];
    char bdsrs[AGPS_AID_INFO_LEN];
    char bdsus[AGPS_AID_INFO_LEN];
    char galrs[AGPS_AID_INFO_LEN];
    char galus[AGPS_AID_INFO_LEN];
    char glors[AGPS_AID_INFO_LEN];
    char glous[AGPS_AID_INFO_LEN];
    char atype[AGPS_AID_INFO_LEN - 6];
    SC_SATE_NUM_T sate_num[GSV_ARRY];  /*0:GPS  1:BDS  2:GLN  3:GAL*/
    SC_SATE_T svarray[GSV_ARRY];       /*0:GPS  1:BDS  2:GLN  3:GAL*/
#ifdef SC_DRIVER_GNSS_ZKW
    char ant[GPS_INFO_ANT_LEN];
#endif

    BOOL fixValid;
}SC_GNSSINFO;

typedef struct{
    SC_Agnss_Data_Send_Method sendMethod;
    char aidTimeString[64];
    char aidPosString[64];
}SC_Agnss_Data_Send_Option;

typedef void (*SC_Gnss_User_CallBack)(SC_Gnss_State_Type event);

/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssStateBindCB
 *
 * DESCRIPTION
 *  GNSS STATE event callback
 *
 * PARAMETERS
 *  callback function
 *
 * RETURNS
 *  0: set done        -1: set fail
 *
 * NOTE
 *
 *****************************************************************************/
SC_Gnss_Return_Code sAPI_GnssStateBindCB(SC_Gnss_User_CallBack cb);

/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssPowerStatusSet
 *
 * DESCRIPTION
 *  GNSS power control
 *
 * PARAMETERS
 *  [power] power on/off
 *
 * RETURNS
 *  0: set done        -1: set fail
 *
 * NOTE
 *
 *****************************************************************************/
SC_Gnss_Return_Code sAPI_GnssPowerStatusSet(SC_Gnss_Power_Status power);
/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssPowerStatusGet
 *
 * DESCRIPTION
 *  Get the status of GNSS power
 *
 * PARAMETERS
 *  None
 *
 * RETURNS
 *  SC_GNSS_POWER_OFF: power off        SC_GNSS_POWER_ON: power on
 *
 * NOTE
 *
 *****************************************************************************/
SC_Gnss_Power_Status sAPI_GnssPowerStatusGet(void);

/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssApFlashSet
 *
 * DESCRIPTION
 *  Set whether to open AP_Flash hot start flag.
 *
 * PARAMETERS
 *  ctl: control whether to open AP_Flash hot start.
 *  SC_GNSS_AP_FLASH_OFF: turn off AP_Flash flag
 *  SC_GNSS_AP_FLASH_ON: turn on AP_Flash hot start flag
 * RETURNS
 *  0: set ok       -1: set fail
 *
 * NOTE
 *
 *****************************************************************************/
SC_Gnss_Return_Code sAPI_GnssApFlashSet(SC_Gnss_Ap_Flash_Status ctl);

/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssApFlashGet
 *
 * DESCRIPTION
 *  Get AP_Flash hot start flag.
 *
 * PARAMETERS
 *  None
 *
 * RETURNS
 *  SC_GNSS_AP_FLASH_OFF: Ap_flash flag off
 *  SC_GNSS_AP_FLASH_ON: Ap_flash flag on
 *
 * NOTE
 *
 *****************************************************************************/
SC_Gnss_Ap_Flash_Status sAPI_GnssApFlashGet(void);
/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssApFlashDataGet
 *
 * DESCRIPTION
 *  Retrieve the data from the ap_flash and store it in the module nvm.
 *
 * PARAMETERS
 *  None
 *
 * RETURNS
 *  0: get ok       -1: get fail
 *
 * NOTE
 *
 *****************************************************************************/
SC_Gnss_Return_Code sAPI_GnssApFlashDataGet(void);
/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssNmeaDataGet
 *
 * DESCRIPTION
 *  Get NMEA data by NMEA port or URC report.
 *
 * PARAMETERS
 *  ctl: control whether the data is output.
 *  mode: Get NMEA data by NMEA port or URC report.
 *
 * RETURNS
 *  0: set ok       -1: set fail
 *
 * NOTE
 *
 *****************************************************************************/
SC_Gnss_Return_Code sAPI_GnssNmeaDataGet(SC_Gnss_Output_Control ctl,SC_Gnss_Nmea_Data_Get mode);
/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssStartMode
 *
 * DESCRIPTION
 *  GNSS start mode
 *
 * PARAMETERS
 *  [mode] hot start/cold start
 *
 * RETURNS
 *  0: set ok       -1: set fail
 *
 * NOTE
 *
 *****************************************************************************/
SC_Gnss_Return_Code sAPI_GnssStartMode(SC_Gnss_Start_Mode mode);
/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssBaudRateSet
 *
 * DESCRIPTION
 *  GNSS set baud rate, Make sure the baud rate between GNSS module and uart3 
 *are consistent  
 *
 * PARAMETERS
 *  [baudrate]
 *
 * RETURNS
 *  0: set ok       -1: set fail
 *
 * NOTE
 *
 *****************************************************************************/
SC_Gnss_Return_Code sAPI_GnssBaudRateSet(SC_Gnss_Baud_Rate baudrate);
/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssBaudRateGet
 *
 * DESCRIPTION
 *  GNSS get baud rate, Make sure the baud rate between GNSS module and uart3 
 *are consistent  
 *
 * PARAMETERS
 *  None
 *
 * RETURNS
 *  SC_Gnss_Baud_Rate
 *
 * NOTE
 *
 *****************************************************************************/
SC_Gnss_Baud_Rate sAPI_GnssBaudRateGet(void);
/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssModeSet
 *
 * DESCRIPTION
 *  GNSS set support mode
 *
 * PARAMETERS
 *  [baudrate]
 *
 * RETURNS
 *  0: set ok       -1: set fail
 *
 * NOTE
 *
 *****************************************************************************/
SC_Gnss_Return_Code sAPI_GnssModeSet(SC_Gnss_Mode mode);
/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssModeGet
 *
 * DESCRIPTION
 *  get current GNSS mode 
 *
 * PARAMETERS
 *  None
 *
 * RETURNS
 *  SC_Gnss_Mode
 *
 * NOTE
 *
 *****************************************************************************/
SC_Gnss_Mode sAPI_GnssModeGet(void);
/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssNmeaRateSet
 *
 * DESCRIPTION
 *  set nmea update rate
 *
 * PARAMETERS
 *  [rate]
 *
 * RETURNS
 *  0: set ok       -1: set fail
 *
 * NOTE
 *
 *****************************************************************************/
SC_Gnss_Return_Code sAPI_GnssNmeaRateSet(SC_Gnss_Nmea_Rate rate);
/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssNmeaRateGet
 *
 * DESCRIPTION
 *  get current nmea rate 
 *
 * PARAMETERS
 *  None
 *
 * RETURNS
 *  SC_Gnss_Nmea_Rate
 *
 * NOTE
 *
 *****************************************************************************/
SC_Gnss_Nmea_Rate sAPI_GnssNmeaRateGet(void);
/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssNmeaSentenceSet
 *
 * DESCRIPTION
 *  set nmea sentence type
 *
 * PARAMETERS
 *  [mask]   bit0-GGA,default is 1
 *           bit1-GLL,default is 1
 *           bit2-GSA,default is 1
 *           bit3-GSV,default is 1
 *           bit4-RMC,default is 1
 *           bit5-VTG,default is 1
 *           bit6-ZDA,default is 1
 *           bit7-ANT,default is 1
 *           bit8-DHV,default is 1
 *           bit9-LPS,default is 1
 *           bit10-res1,default is 0
 *           bit11-res2,default is 0
 *           bit12-UTC,default is 1
 *           bit13-GST,default is 1
 *  default mask is 13311(11001111111111)
 * RETURNS
 *  0: set ok       -1: set fail
 *
 * NOTE
 *
 *****************************************************************************/
SC_Gnss_Return_Code sAPI_GnssNmeaSentenceSet(unsigned short mask);

/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssNmeaSentenceSet
 *
 * DESCRIPTION
 *  get nmea sentence type
 *
 * PARAMETERS
 *  None
 *
 * RETURNS
 *  array pointer,
 *           arr[0]-GGA
 *           arr[1]-GLL
 *           arr[2]-GSA
 *           arr[3]-GSV
 *           arr[4]-RMC
 *           arr[5]-VTG
 *           arr[6]-ZDA
 *           arr[7]-ANT
 *           arr[8]-DHV
 *           arr[9]-LPS
 *           arr[10]-res1
 *           arr[11]-res2
 *           arr[12]-UTC
 *           arr[13]-GST
 * NOTE
 *
 *****************************************************************************/
UINT8* sAPI_GnssNmeaSentenceGet(void);
/*****************************************************************************
 * FUNCTION
 *  sAPI_GpsInfoGet
 *
 * DESCRIPTION
 *  get gps information
 *  
 * PARAMETERS
 *  [period] The rang is 0-255, unit is second. after set <period> will report 
 *the GPS information every the seconds.
 *
 * RETURNS
 *  0: set done        -1: set fail  
 *
 * NOTE
 *  
 *****************************************************************************/
SC_Gnss_Return_Code sAPI_GpsInfoGet(UINT8 period);
/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssInfoGet
 *
 * DESCRIPTION
 *  get gnss information
 *  
 * PARAMETERS
 *  [period] The rang is 0-255, unit is second. after set <period> will report 
 *the GNSS information every the seconds.
 *
 * RETURNS
 *  0: set done        -1: set fail  
 *
 * NOTE
 *  
 *****************************************************************************/
SC_Gnss_Return_Code sAPI_GnssInfoGet(UINT8 period);

/*****************************************************************************
 * FUNCTION
 *  sAPI_SendCmd2Gnss
 *
 * DESCRIPTION
 *  send  nmea command directly to GNSS.
 *
 * PARAMETERS
 *  [string]   string in NMEA format
 *
 * RETURNS
 *  0: send ok       -1: send fail
 *
 * NOTE
 *
 *****************************************************************************/
SC_Gnss_Return_Code sAPI_SendCmd2Gnss(char *string);

/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssAgpsSeviceOpen
 *
 * DESCRIPTION
 *  Retrieve AGPS data from the AGNSS server and download it to the GPS chip.
 *
 * PARAMETERS
 *  None
 *
 * RETURNS
 *   0 : AGPS data save ok
 *   -1: GNSS power off or method is invaild
 *  101: open socket unsuccessfully.
 *  102: get the AGNSS server unsuccessfully.
 *  103: connect to AGNSS server unsuccessfully.
 *  104: write information to socket unsuccessfully.
 *  105: read AGPS data from socket unsuccessfully.
 *  106: get to AGNSS data unsuccessfully.
 *  107: send agnss data to gps chip unsuccessfully.
 *  108: store AGPS data unsuccessfully.
 * NOTE
 *
 *****************************************************************************/

 SC_Gnss_Return_Code sAPI_GnssAgpsSeviceOpen(void);

 /*****************************************************************************
 * FUNCTION
 *  sAPI_GnssAgpsDataSavetoLocal
 *
 * DESCRIPTION
 *  Obtain real-time AGPS assisted positioning data into the module
 *
 * PARAMETERS
 *  [method]   Choose the method for storing AGPS data.
 *
 * RETURNS
 *   0 : AGPS data save ok
 *   -1: GNSS power off or method is invaild
 *  101: open socket unsuccessfully.
 *  102: get the AGNSS server unsuccessfully.
 *  103: connect to AGNSS server unsuccessfully.
 *  104: write information to socket unsuccessfully.
 *  105: read AGPS data from socket unsuccessfully.
 *  106: get to AGNSS data unsuccessfully.
 *  107: send agnss data to gps chip unsuccessfully.
 *  108: store AGPS data unsuccessfully.
 * NOTE
 *  ASR5311 chip does not support AGNSS data store
 *****************************************************************************/
 SC_Gnss_Return_Code sAPI_GnssAgpsDataSavetoLocal(SC_Agnss_Data_Save_Method method);

 /*****************************************************************************
* FUNCTION
*  sAPI_GnssAgpsDatatoGpsChip
*
* DESCRIPTION
*  inject local AGPS data into the GPS chip, with optional injection assistance for time and location.
*
* PARAMETERS
*  [opt]   Choose which method to send AGPS data stored locally to the GPS chip.
*          And choose whether to inject auxiliary time and auxiliary position.
*   aidTimeString: indicates the auxiliary time string to be injected, which is an optional parameter.
*                  The input format is: year, month, day, hour, minute, second, milliseconds, for example: "2023,8,18,15,13,30,15".
*   aidPosString: indicates the auxiliary position string to be injected, which is an optional parameter.
*                 The input format is latitude, longitude, with latitude units in degrees, ddmm.mmmmm,
*                 and longitude units in degrees, dddmm, mmmmmm. For example, "2929.461620,10638.075350".
*
* RETURNS
*   0 : AGPS data send ok
*   -1: GNSS power off or sending method is invaild
*  107: send agnss data to gps chip unsuccessfully.
*  109: send aidtime unsuccessfully.
*  110: send aidpos unsuccessfully.
* NOTE
*   ASR5311 chip does not support AGNSS data store.
*   If the auxiliary time and auxiliary position to be injected are effective for GPS acceleration positioning,
*   the accuracy of the auxiliary time needs to ensure that the input error is within 1ms, and the accuracy of
*   the auxiliary position is effective within 5KM. If the input auxiliary time and position are invalid,
*   it will have a negative effect, making the positioning slower. The auxiliary time and auxiliary position
*   are not inputted, and being empty does not affect the injection of AGPS data into the GPS chip.
*****************************************************************************/
SC_Gnss_Return_Code sAPI_GnssAgpsDatatoGpsChip(SC_Agnss_Data_Send_Option *opt);

/*****************************************************************************
* FUNCTION
*  sAPI_GnssAgpsDataCheck
*
* DESCRIPTION
*  Verify if the injection of AGPS ephemeris data is successful.
*
* PARAMETERS
*
*
* RETURNS
*   0 : AGPS data is valid
*   -1: GNSS power off
* NOTE
*   ZKW chip does not support AGNSS data check.
*   After the instruction responds, the check results will be reported through URC as:
*   GPSRS, GPSUS, BDSRS, BDSUS, GALRS, GALUS, GLORS, GLOUS
*****************************************************************************/
SC_Gnss_Return_Code sAPI_GnssAgpsDataCheck(void);

/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssNavdataGet
 *
 * DESCRIPTION
 *  Obtain the structure pointer of navdata
 *
 * PARAMETERS
 *  none
 *
 * RETURNS
 *  SC_GNSSINFO *
 *
 * NOTE
 *
 *****************************************************************************/
SC_GNSSINFO *sAPI_GnssNavdataGet(void);
/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssDynamicLoadFileTypeSet
 *
 * DESCRIPTION
 *  Set the GNSS dynamic load file type.
 *
 * PARAMETERS
 *  type: Set the GNSS dynamic load file type.
 *  SC_GNSS_DYNAMIC_LOAD_FILE_COMMON_TYPE: common type
 *  SC_GNSS_DYNAMIC_LOAD_FILE_CUSTOM_TYPE: custom type
 *  SC_GNSS_DYNAMIC_LOAD_FILE_ERROR_TYPE: error type
 * RETURNS
 *  0: set ok       -1: set fail
 *
 * NOTE
 *
 *****************************************************************************/
SC_Gnss_Return_Code sAPI_GnssDynamicLoadFileTypeSet(SC_Gnss_Dynamic_Load_File_Type type);
/*****************************************************************************
 * FUNCTION
 *  sAPI_GnssDynamicLoadFileTypeGet
 *
 * DESCRIPTION
 *  Get the GNSS dynamic load file type.
 *
 * PARAMETERS
 *  None
 *
 * RETURNS
 *  SC_GNSS_DYNAMIC_LOAD_FILE_COMMON_TYPE: common type
 *  SC_GNSS_DYNAMIC_LOAD_FILE_CUSTOM_TYPE: custom type
 *  SC_GNSS_DYNAMIC_LOAD_FILE_ERROR_TYPE: error type
 *
 * NOTE
 *
 *****************************************************************************/
SC_Gnss_Dynamic_Load_File_Type sAPI_GnssDynamicLoadFileTypeGet(void);

/*****************************************************************************
* FUNCTION
*  sAPI_GnssPreEphDataLoadAndUpdate
*
* DESCRIPTION
*  Update the predicted ephemeris seed file and obtain the predicted ephemeris at the current time.
*
* PARAMETERS
*   SC_PREDICTING_EPH_DATA_DOWNLOAD_AND_UPDATE: update the predicted ephemeris seed file and obtain the predicted ephemeris at the current time.
*   SC_PREDICTING_EPH_DATA_UPDATE: obtain the predicted ephemeris at the current time.
* RETURNS
*   0 : Successfully obtained predicted ephemeris
*   106: Failed to obtain predicted ephemeris from server.
*   107: Failed to send predicted ephemeris to gps chip.
* NOTE
*   ZKW and UC6228 chip does not support Predicting ephemeris data.
*****************************************************************************/
SC_Gnss_Return_Code sAPI_GnssPreEphDataLoadAndUpdate(SC_Predicting_Eph_Data_Opt opt);

/*****************************************************************************
* FUNCTION
*  sAPI_GnssPvtClean
*
* DESCRIPTION
*  Clear the jacana_pvt partition.
*
* PARAMETERS
*   none
*
* RETURNS
*   0 : Clear successful.
*   -1: Clearing failed.
* NOTE
*   ZKW and UC6228 chip does not support PVT partition cleanup.
*****************************************************************************/
SC_Gnss_Return_Code sAPI_GnssPvtClean(void);

int gnssStartModeIsValid(SC_Gnss_Start_Mode mode);

void sc_gps_timer_callbake(    UINT32 val);
void sc_gnss_timer_callbake(    UINT32 val);


#endif


