include ${TOOL_DIR}/script/env.mak

OLD_FW_LIB_LIST_CACHE := ${ROOT_DIR}/out/${SCMODULE}/old_fw_lib_list.cmake
OLD_FW_DFLAGS_CACHE := ${ROOT_DIR}/out/${SCMODULE}/old_fw_dflags.cmake

#-------------------------------------------------------------------------------
# build tool define.
#-------------------------------------------------------------------------------
CC:=arm-none-eabi-gcc
CPP:=arm-none-eabi-g++
LD:=arm-none-eabi-ld
AR:=arm-none-eabi-ar
OBJCOPY:=arm-none-eabi-objcopy

CRC_SET:=$(TOOL_DIR)\${PLATFORM}\crc_set


