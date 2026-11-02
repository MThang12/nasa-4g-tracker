$(info -----------------env configs-------------------------)
$(info -  set tools)
$(info -  -include $(lastword $(MAKEFILE_LIST)))
$(info -----------------------------------------------------)

empty:=
space:= $(empty) $(empty)

ifeq ($(OS),Windows_NT)
	PLATFORM := win32
	GCC_ARM_TOOLCHAIN_PACKAGE := cross_tool.zip
	GZIP := $(TOOL_DIR)/$(PLATFORM)/7z/7z.exe
	GZIPARG := x
	COPY := xcopy
	COPYARG := /s /e /q
	COPY_FILE := copy
	COPYARG_FILE := /y
	CAT := type
	MKDIR := mkdir
	MKDIRARG :=
	RM := del
	RMARG := /q
	RMDIR := rmdir
	RMDIRARG := /s /q
	MOVE := ren
	BUILD_TYPE := "Ninja"
	BUILD := ninja
	MAKE := gnumake
	CMAKE := ${TOOL_DIR}/${PLATFORM}/cmake/bin/cmake.exe
	CMAKE_DIR := ${TOOL_DIR}/${PLATFORM}/cmake
	CMAKE_PACKAGE_NAME := cmake.zip
	MSYS2_DIR := ${TOOL_DIR}/${PLATFORM}/msys64
	MSYS2_PACKAGE := ${TOOL_DIR}/${PLATFORM}/msys64.zip
	SED :=  ${MSYS2_DIR}/usr/bin/sed.exe
	BAD_SLASH := $(strip /)
	GOOD_SLASH := $(strip \)
	GOOD_BREAKER := $(strip ;)
	WHERE := where

	BUILDCONFIG := ${TOOL_DIR}/win32/buildConfig
else ifeq ($(shell uname),Linux)
	PLATFORM := linux
	GCC_ARM_TOOLCHAIN_PACKAGE := cross_tool.tar.bz2
	GZIP := tar
	GZIPARG := -jxf
	COPY := cp
	COPYARG := -rf
	COPY_FILE := cp
	COPYARG_FILE := -rf
	CAT := cat
	MKDIR := mkdir
	MKDIRARG := -p
	RM := rm
	RMARG := -f
	RMDIR := rm
	RMDIRARG := -rf
	MOVE := mv
	BUILD_TYPE := "Unix Makefiles"
	BUILD := make
	MAKE := make
	CMAKE := ${TOOL_DIR}/${PLATFORM}/cmake/bin/cmake
	CMAKE_DIR := ${TOOL_DIR}/${PLATFORM}/cmake
	CMAKE_PACKAGE_NAME := cmake.tar.bz2
	SED := sed
	BAD_SLASH := $(strip \)
	GOOD_SLASH := $(strip /)
	GOOD_BREAKER := $(strip :)
	WHERE := which

	BUILDCONFIG := python ${TOOL_DIR}/script/buildConfig.py
else
  $(error The os "$(shell uname)" is not supported)
endif

Q=@

CROSS_TOOL_DIR := ${TOOL_DIR}/${PLATFORM}/cross_tool
GCC_ARM_TOOLCHAIN_DIR := ${CROSS_TOOL_DIR}/gcc-arm-none-eabi
GCC_ARM_TOOLCHAIN := ${GCC_ARM_TOOLCHAIN_DIR}/bin

API_MAP_INIT_SCRIPT := ${TOOL_DIR}/script/sapiInit.py

export PATH := $(subst $(BAD_SLASH),$(GOOD_SLASH),${TOOL_DIR}/${PLATFORM})$(GOOD_BREAKER)$(subst $(BAD_SLASH),$(GOOD_SLASH),${GCC_ARM_TOOLCHAIN})$(GOOD_BREAKER)${PATH}

export MENUCONFIG_STYLE=monochrome
