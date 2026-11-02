include ${ROOT_DIR}/old_fw_Makefile
include ${ROOT_DIR}/config/ToolChain.mak

BUILD_PATH:=$(ROOT_DIR)/out/${SCMODULE}/obj_cus
LIB_PATH:=$(ROOT_DIR)/out/${SCMODULE}/lib
BIN_PATH:=$(ROOT_DIR)/out/${SCMODULE}

PACKAGE_DIRS:=$(subst \,/,$(PACKAGE_DIRS))
PACKAGE_BUILD_FLOW:=$(patsubst %,%/package_build,${PACKAGE_DIRS})


all: mkDir preInfo ${PACKAGE_BUILD_FLOW} defaultInfo postInfo

mkDir:
	-${MKDIR} ${MKDIRARG} $(subst ${BAD_SLASH},${GOOD_SLASH},$(dir ${OLD_FW_LIB_LIST_CACHE}))

preInfo:
ifeq (win32,${PLATFORM})
	@echo SET(OLD_FW_LIB_LIST> ${OLD_FW_LIB_LIST_CACHE}
	@echo SET(OLD_FW_DFLAGS> ${OLD_FW_DFLAGS_CACHE}
else
	@echo "SET(OLD_FW_LIB_LIST"> ${OLD_FW_LIB_LIST_CACHE}
	@echo "SET(OLD_FW_DFLAGS"> ${OLD_FW_DFLAGS_CACHE}
endif

%/package_build:% preInfo
	@echo ----------------------------------
	@echo ----------- build [$<] -----------
	@echo ----------------------------------
	@${MAKE} -C $< -f Makefile SCMODULE=${SCMODULE} TOOL_DIR=${TOOL_DIR} ROOT_DIR=${ROOT_DIR} TOP_DIR=${ROOT_DIR} KERNEL_DIR=${KERNEL_DIR} BUILD_PATH=${BUILD_PATH} LIB_PATH=${LIB_PATH} -j4
	@echo $(subst \,/,$(strip ${LIB_PATH}/cuslib$(notdir $<).a))>> ${OLD_FW_LIB_LIST_CACHE}

defaultInfo: ${PACKAGE_BUILD_FLOW}
ifneq (,${DEFAULT_LIBS})
	@echo ${DEFAULT_LIBS}>> ${OLD_FW_LIB_LIST_CACHE}
endif
ifneq (,${DFLAG})
	@echo ${DFLAG}>> ${OLD_FW_DFLAGS_CACHE}
endif

postInfo: defaultInfo
ifeq (win32,${PLATFORM})
	@echo )>> ${OLD_FW_LIB_LIST_CACHE}
	@echo )>> ${OLD_FW_DFLAGS_CACHE}
else
	@echo ")">> ${OLD_FW_LIB_LIST_CACHE}
	@echo ")">> ${OLD_FW_DFLAGS_CACHE}
endif
