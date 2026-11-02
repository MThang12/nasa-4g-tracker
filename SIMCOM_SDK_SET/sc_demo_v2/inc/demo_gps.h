#ifndef __DEMO_GPS_H__
#define __DEMO_GPS_H__

#define ARGGNSS_APFLASH_STATUS                  "ap_flash status option"
#define AP_FLASH_STATUS_ON                      "ap_flash on"
#define AP_FLASH_STATUS_OFF                     "ap_flash off"
#define GET_AP_FLASH_STATUS                     "get ap_flash status"

#define ARGGNSS_POWER_STATUS                    "gnss power status option"
#define GNSS_PWR_STATUS_ON                      "power on"
#define GNSS_PWR_STATUS_OFF                     "power off"
#define GET_GNSS_PWR_STATUS                     "get gnss power status"

#define ARGGNSS_NMEA_DATA_GET                   "gnss nmea data get option"
#define GNSS_NMEA_DATA_GET_START_BY_PORT        "start get NMEA data by port"
#define GNSS_NMEA_DATA_GET_STOP_BY_PORT         "stop get NMEA data by port"
#define GNSS_NMEA_DATA_GET_START_BY_URC         "start get NMEA data by URC"
#define GNSS_NMEA_DATA_GET_STOP_BY_URC          "stop get NMEA data by URC"

#define ARGGNSS_START_MODE_SET                  "gnss start mode set option"
#define GNSS_HOT_START                          "hot start"
#define GNSS_WARM_START                         "warm start"
#define GNSS_COLD_START                         "cold start"

#define ARGGNSS_BAUD_RATE                       "gnss baud rate option"
#define GNSS_DEMO_BAUD_RATE_4800                "4800"
#define GNSS_DEMO_BAUD_RATE_9600                "9600"
#define GNSS_DEMO_BAUD_RATE_19200               "19200"
#define GNSS_DEMO_BAUD_RATE_38400               "38400"
#define GNSS_DEMO_BAUD_RATE_57600               "57600"
#define GNSS_DEMO_BAUD_RATE_115200              "115200"
#define GNSS_DEMO_BAUD_RATE_230400              "230400"
#define GET_GNSS_BAUD_RATE                      "get gnss baud rate"

#define ARGGNSS_MODE                            "gnss mode option"
#define GNSS_DEMO_MODE_GPS                      "GPS"
#define GNSS_DEMO_MODE_BDS                      "BDS"
#define GNSS_DEMO_MODE_GPS_BDS                  "GPS+BDS"
#define GNSS_DEMO_MODE_GLONASS                  "GLONASS"
#define GNSS_DEMO_MODE_GPS_GLONASS              "GPS+GLONASS"
#define GNSS_DEMO_MODE_BDS_GLONASS              "BDS+GLONASS"
#define GNSS_DEMO_MODE_GPS_BDS_GLONASS          "GPS+BDS+GLONASS"
#define GNSS_DEMO_MODE_GPS_GLONASS_GALILEO      "GPS+GLONASS+GALILEO"
#define GNSS_DEMO_MODE_GPS_BDS_GALILEO          "GPS+BDS+GALILEO"
#define GET_GNSS_MODE                           "get gnss mode"

#define ARGGNSS_NMEA_RATE                       "gnss nmea rate option"
#define GNSS_DEMO_NMEA_RATE_1HZ                 "1Hz"
#define GNSS_DEMO_NMEA_RATE_2HZ                 "2Hz"
#define GNSS_DEMO_NMEA_RATE_4HZ                 "4Hz"
#define GNSS_DEMO_NMEA_RATE_5HZ                 "5Hz"
#define GNSS_DEMO_NMEA_RATE_10HZ                "10Hz"
#define GET_GNSS_NMEA_RATE                      "get nmea rate"

#define ARGGNSS_NMEA_SENTENCE_SET               "gnss nmea sentence set"
#define ARGGNSS_NMEA_SENTENCE_GET               "gnss nmea sentence get"
#define GET_NMEA_SENTENCE                       "get nmea sentence"

#define ARGGNSS_AND_GPS_INFO_GET                "gpsinfo and gnssinfo get"
#define GET_GPSINFO                             "get gpsinfo"
#define GET_GNSSINFO                            "get gnssinfo"
#define SET_INFO_TIMER                          "time"

#define ARGGNSS_CMD_SEND                        "gnss command send"

#define ARGGNSS_AGPS                            "agps option"
#define AGPS_OPEN                               "agps open"

#endif
