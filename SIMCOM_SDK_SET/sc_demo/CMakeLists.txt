cmake_minimum_required(VERSION 3.10)

# AUX_SOURCE_DIRECTORY(./src sc_demo_src)
AUX_SOURCE_DIRECTORY(./src/token sc_demo_src)
AUX_SOURCE_DIRECTORY(./src/utils sc_demo_src)
list(APPEND sc_demo_src ./src/simcom_demo.c)
list(APPEND sc_demo_src ./src/demo_helloworld.c)

#########################
#       general config
#########################
if(HAS_UART)
    list(APPEND sc_demo_src ./src/uart_api.c)
endif()

if(HAS_URC)
    list(APPEND sc_demo_src ./src/cus_urc.c)
endif()

if(HAS_USB)
    list(APPEND sc_demo_src ./src/cus_usb_vcom.c)
endif()

#########################
#       driver
#########################
if(HAS_DEMO_WTD)
    list(APPEND sc_demo_src ./src/demo_wtd.c)
endif()

if(HAS_DEMO_PMU)
    list(APPEND sc_demo_src ./src/demo_pm.c)
endif()

if(HAS_DEMO_GPIO)
    list(APPEND sc_demo_src ./src/demo_gpio.c)
endif()

if(HAS_DEMO_PWM)
    list(APPEND sc_demo_src ./src/demo_pwm.c)
endif()

if(HAS_DEMO_UART)
    list(APPEND sc_demo_src ./src/demo_uart.c)
endif()

if(HAS_DEMO_USB)
endif()

if(HAS_DEMO_I2C)
    list(APPEND sc_demo_src ./src/demo_i2c.c)
endif()

if(HAS_DEMO_SPI)
    list(APPEND sc_demo_src ./src/demo_spi.c)
endif()

if(HAS_DEMO_FLASH)
    list(APPEND sc_demo_src ./src/demo_flash.c)
endif()

if(HAS_DEMO_OTA)
    list(APPEND sc_demo_src ./src/demo_fota.c)
endif()

if(HAS_DEMO_GNSS)
    list(APPEND sc_demo_src ./src/demo_gps.c)
endif()

if(HAS_DEMO_LCD)
    list(APPEND sc_demo_src ./src/demo_lcd.c)
endif()

if(HAS_DEMO_CAM)
    list(APPEND sc_demo_src ./src/demo_cam.c ./src/demo_cam_dirver.c)
endif()
if(HAS_DEMO_SYS)
    list(APPEND sc_demo_src ./src/demo_cam.c ./src/demo_system.c)
endif()
if(HAS_DEMO_EXFLASHMOUNTFS)
    list(APPEND sc_demo_src ./src/demo_exfs.c)
endif()

#########################
#         modem
#########################
if(HAS_DEMO_NETWORK)
    list(APPEND sc_demo_src ./src/demo_network.c ./src/demo_txrx_power.c)
endif()

if(HAS_DEMO_SIMCARD)
    list(APPEND sc_demo_src ./src/demo_simcard.c)
endif()

if(HAS_DEMO_CALL)
    list(APPEND sc_demo_src ./src/demo_call.c)
endif()

if(HAS_DEMO_SMS)
    list(APPEND sc_demo_src ./src/demo_sms.c)
endif()

if(HAS_DEMO_LBS)
    list(APPEND sc_demo_src ./src/demo_loc.c ./src/demo_loc_test.c)
endif()

if(HAS_DEMO_SJDR)
    list(APPEND sc_demo_src ./src/demo_sjdr.c)
endif()

if(HAS_DEMO_INTERNET_SERVICE)
endif()

#########################
#         app
#########################
if(HAS_DEMO_SSL)
    list(APPEND sc_demo_src ./src/demo_ssl.c ./src/demo_ssl_test.c)
endif()

if(HAS_DEMO_TCPIP)
    list(APPEND sc_demo_src ./src/demo_tcpip.c)
endif()

if(HAS_DEMO_HTTPS)
    list(APPEND sc_demo_src ./src/demo_https.c)
endif()

if(HAS_DEMO_FTPS)
    list(APPEND sc_demo_src ./src/demo_ftps.c ./src/demo_ftps_test.c)
endif()

if(HAS_DEMO_MQTTS)
    list(APPEND sc_demo_src ./src/demo_mqtt.c ./src/mqtt_OneNET.c ./src/mqtt_tencent.c)
endif()

if(HAS_DEMO_NTP)
    list(APPEND sc_demo_src ./src/demo_ntp.c)
endif()

if(HAS_DEMO_HTP)
    list(APPEND sc_demo_src ./src/demo_htp.c)
endif()

if(HAS_DEMO_AUDIO)
    list(APPEND sc_demo_src ./src/demo_audio.c)
endif()

if(HAS_DEMO_TTS)
    list(APPEND sc_demo_src ./src/demo_tts.c)
endif()

if(HAS_DEMO_POC)
    list(APPEND sc_demo_src ./src/demo_poc.c)
endif()

if(HAS_DEMO_WIFI)
    list(APPEND sc_demo_src ./src/demo_wifi.c)
endif()

if(HAS_DEMO_RTC)
    list(APPEND sc_demo_src ./src/demo_rtc.c)
endif()

if(HAS_DEMO_BLE)
    list(APPEND sc_demo_src ./src/demo_ble.c)
endif()

if(HAS_DEMO_BT)
    list(APPEND sc_demo_src ./src/demo_bt.c)
endif()
if(HAS_DEMO_BT_STACK)
    list(APPEND sc_demo_src ./src/demo_bt_stack.c)
endif()

if(HAS_DEMO_FS)
    list(APPEND sc_demo_src ./src/demo_file_system.c)
endif()

if(HAS_DEMO_FS_OLD)
endif()

if(HAS_DEMO_APP_DOWNLOAD)
    list(APPEND sc_demo_src ./src/demo_app_download.c)
endif()

if(HAS_DEMO_APP_UPDATE)
    list(APPEND sc_demo_src ./src/demo_app_updater.c)
endif()

if(HAS_DEMO_PING)
    list(APPEND sc_demo_src ./src/demo_ping.c)
endif()

#########################
#       other libs
#########################
if(HAS_ZLIB)
    list(APPEND sc_demo_src ./src/demo_zlib.c)
endif()

if(HAS_SM2)
    list(APPEND sc_demo_src ./src/demo_sm2.c)
endif()

if(HAS_CJSON)
    list(APPEND sc_demo_src ./src/demo_cjson.c)
endif(HAS_CJSON)

if(HAS_MBEDTLS)
    list(APPEND sc_demo_src ./src/demo_mbedtls.c)
endif(HAS_MBEDTLS)

# Add the static library
add_library(sc_demo STATIC ${sc_demo_src})
target_compile_options(sc_demo PUBLIC -Werror)
target_include_directories(sc_demo PUBLIC
    ./inc
    ${CMAKE_SOURCE_DIR}/sc_lib/inc
    ${CMAKE_SOURCE_DIR}/sc_lib/${SCMODULE}/inc
    ${CMAKE_SOURCE_DIR}/sc_lib/inc/GPIO
    ${CMAKE_SOURCE_DIR}/sc_lib/inc/token
    ${CMAKE_SOURCE_DIR}/sc_lib/inc/utils
    ${CMAKE_SOURCE_DIR}/sc_lib/inc/mbedtls
    ${CMAKE_SOURCE_DIR}/sc_demo/inc
    ${CMAKE_SOURCE_DIR}/config
)

if(HAS_SM2)
    target_include_directories(sc_demo PUBLIC
        ${CMAKE_SOURCE_DIR}/public_libs/sm2
        ${CMAKE_SOURCE_DIR}/public_libs/miracl
    )
endif(HAS_SM2)

if(HAS_ZLIB)
    target_include_directories(sc_demo PUBLIC
        ${CMAKE_SOURCE_DIR}/public_libs/zlib
        ${CMAKE_SOURCE_DIR}/public_libs/zlib/minizip
    )
endif(HAS_ZLIB)

if(HAS_CJSON)
    target_include_directories(sc_demo PUBLIC
        ${CMAKE_SOURCE_DIR}/public_libs/cjson
    )
endif(HAS_CJSON)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
