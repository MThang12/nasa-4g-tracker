#############################
#  create aboot package
############################

include ${TOOL_DIR}/script/env.mak
include make_image_16xx_settings.mak


ifneq (,$(SIMCOM_RELEASE_SDK))
ABOOT_DIR := $(TOOL_DIR)/$(PLATFORM)/aboot
else
ABOOT_DIR := ${ROOT_DIR}/AbootTool
endif
ARELEASE := $(ABOOT_DIR)/arelease
ARELEASE := $(subst $(BAD_SLASH),$(GOOD_SLASH),$(ARELEASE))

OUT ?= out

SC_USR_OPT_LIST := $(subst _, ,${SC_USR_OPT})
SC_HD_OPT_LIST := $(subst _, ,${SC_HD_OPT})



OUT_DIR := ${ROOT_DIR}/${OUT}/${SC_MODULE_FULL}
ifeq (TRUE,${FACTORY})
ABOOT_OUT_DIR := ${OUT_DIR}/aboot_factory
else
ABOOT_OUT_DIR := ${OUT_DIR}/aboot
endif
ABOOT_IMAGES_OUT_DIR := ${ABOOT_OUT_DIR}/images

##########################################
#     KERNEL OUTPUT PATH
########################################
KERNEL_SRC_NAME ?= cp.bin
APN_SRC_NAME ?=apn.bin
ifneq (,$(findstring OPENSDK,${SC_MODULE_BASE}))
KERNEL_SRC_PATH := ${KERNEL_DIR}/${SC_MODULE_FULL}
else
KERNEL_SRC_PATH := ${ROOT_DIR}/tavor/Arbel/bin
endif

ifneq (,$(findstring OPENSDK,${SC_MODULE_BASE}))
##########################################
#     USERSPACE OUTPUT PATH
########################################
USERSPACE_SRC_NAME := ${APP_NAME}.bin
USERSPACE_SRC_PATH := ${APP_DIR}/${OUT}/${SC_MODULE_FULL}
endif

##########################################
#     aboot config PATH
########################################
ifneq (,${SIMCOM_RELEASE_SDK})

ifeq (TRUE,${FACTORY})
ABOOT_SRC_NAME := aboot_factory
else
ABOOT_SRC_NAME := aboot
endif
ABOOT_SRC_PATH := ${KERNEL_DIR}/${SC_MODULE_FULL}

else  # ifneq (,${SIMCOM_RELEASE_SDK})

ifneq (,$(findstring _1606_,_${SC_MODULE_BASE}_))
ABOOT_CFG := releasepack-ASR1606-source
ASR_MODEL := CRANEL
NET_MODEL := C1
else ifneq (,$(findstring _1602_,_${SC_MODULE_BASE}_))
ABOOT_CFG := releasepack-ASR1602-source
ASR_MODEL := CRANELR
NET_MODEL := C1
else ifneq (,$(findstring _1603_,_${SC_MODULE_BASE}_))
ABOOT_CFG := releasepack-ASR1603-source
ASR_MODEL := CRANEM
NET_MODEL := C1G
else ifneq (,$(findstring _1601_,_${SC_MODULE_BASE}_))
ABOOT_CFG := releasepack-ASR1601-source
ASR_MODEL := CRANE
NET_MODEL := C1G
endif


##########################################
#     aboot config PATH
########################################
ABOOT_CONFIG_SRC_NAME := config
ABOOT_CONFIG_SRC_PATH := ${ROOT_DIR}/AbootTool/configurations/${ABOOT_CFG}

ABOOT_IMAGES_SRC_NAME := images
ABOOT_IMAGES_SRC_PATH := ${ROOT_DIR}/AbootTool/configurations/${ABOOT_CFG}

##########################################
#     NVM PATH
########################################

ifneq (,$(SC_HD_CFG))
ifeq (,$(findstring _A7672E_FASE_1603_V201_,_$(SC_MODULE_FULL)_)$(findstring _A7672SA_FASE_1603_V201_,_$(SC_MODULE_FULL)_))
ifneq ($(SC_HD_CFG)_$(SC_HD_CFG),$(patsubst _F%,,$(SC_HD_CFG))_$(patsubst _M%,,$(SC_HD_CFG)))
ifneq (__,$(findstring A7670,${SC_MODULE})_$(findstring A7672,${SC_MODULE})_$(findstring A7677,${SC_MODULE}))
#CHIP_PLATFORM START
	ifneq (,$(findstring _1603_,_${SC_MODULE_BASE}_))
		NVM_PLATFORM := GPS_CRANEGM
	endif

	ifneq (,$(findstring _1606_,_${SC_MODULE_BASE}_))
		NVM_PLATFORM := GPS_CRANEL
	endif

	ifneq (,$(findstring _1602_,_${SC_MODULE_BASE}_))
		NVM_PLATFORM := GPS_CRANELR
	endif

#CHIP_PLATFORM END
#NVM_PRESET START
	ifneq (,$(SC_USR_OPT))
		ifneq ($(wildcard $(PREBUILD_DIR)/NVM/$(SC_USR_OPT)),)
			NVM_PRESET := $(SC_USR_OPT)
		endif
	endif
	NVM_PRESET ?= NULL
	ifeq (NULL,${NVM_PRESET})
		ifneq ($(SC_HD_CFG)$(SC_HD_CFG),$(patsubst %S,,$(SC_HD_CFG))$(patsubst %C,,$(SC_HD_CFG)))
			ifneq (,$(findstring C_,$(SC_MODULE_FULL)))
				NVM_PRESET := GENERAL_IN
			else
				NVM_PRESET := GENERAL_OUT
			endif
		endif

		ifeq (_16MB,$(FLASH_TARGET_SIZE))
			ifneq (,$(findstring C_,$(SC_MODULE_FULL))$(findstring C1_,$(SC_MODULE_FULL)))
				NVM_PRESET := GENERAL_IN
			else
				NVM_PRESET := GENERAL_OUT
			endif
		endif
	endif
#NVM_PRESET END

endif#(,$(SC_HD_CFG))
endif#($(SC_HD_CFG)_$(SC_HD_CFG)
endif#(_,$(findstring A7670
endif

ifneq (,$(findstring ONOMONDO,${SC_USR_OPT}))
	NVM_PLATFORM := ONOMONDO
	NVM_PRESET := GENERAL_IN
endif

ifneq (,$(findstring _A7677S_MANS_1606_V702_OPENSDK_ADSUN_,_$(SC_MODULE_FULL)_))
	NVM_PLATFORM := ADSUN
	NVM_PRESET := GENERAL_OUT
endif

ifeq (,$(findstring _A7672E_FASE_1603_V201_,_$(SC_MODULE_FULL)_))
	ifneq (,$(findstring _RU_,_${SC_USR_OPT}_))
		NVM_PLATFORM := RU
		NVM_PRESET := GENERAL_OUT
	endif
endif

ifneq (,$(findstring _A7680C_LANS_1606_V506_CQCY_,_$(SC_MODULE_FULL)_))
	NVM_PLATFORM := CQCY
	NVM_PRESET := GENERAL_IN
endif

NVM_PRESET ?= NULL
ifeq (NULL,${NVM_PRESET})
	NVM_PLATFORM := NULL
endif

NVM_OUT_NAME := nvm.bin
NVM_SRC_PATH := ${PREBUILD_DIR}/NVM/${NVM_PLATFORM}/${NVM_PRESET}

ifeq (NULL,${NVM_PRESET})
NVM_SRC_NAME := 0KiB.bin
else
NVM_SRC_NAME := $(shell python getInfo.py GET_NVM_SIZE ${ABOOT_CONFIG_SRC_PATH}/${ABOOT_SRC_NAME}/config/product/${ASR_PRODUCT_TYPE}.json ${ASR_PRODUCT}).bin
endif

##########################################
#     FOTA_PARAM PATH
########################################
ifneq (,$(SC_HD_CFG))
ifneq (,$(findstring OPENSDK,${SC_MODULE_BASE}))
FOTA_PARAM_OUT_NAME := FOTA_PARAM.bin
ifeq (TRUE,${FACTORY})
FOTA_PARAM_SRC_PATH := ${PREBUILD_DIR}/FOTA_PARAM/${ASR_MODEL}/FACTORY
else
FOTA_PARAM_SRC_PATH := ${PREBUILD_DIR}/FOTA_PARAM/${ASR_MODEL}
endif
endif
endif

ifneq (,$(SC_HD_CFG))
ifneq (,$(findstring UBIA,${SC_USR_OPT}))
FOTA_PARAM_OUT_NAME := FOTA_PARAM.bin
ifeq (TRUE,${FACTORY})
FOTA_PARAM_SRC_PATH := ${PREBUILD_DIR}/FOTA_PARAM/${ASR_MODEL}/FACTORY
else
FOTA_PARAM_SRC_PATH := ${PREBUILD_DIR}/FOTA_PARAM/${ASR_MODEL}
endif
endif
endif
##########################################
#     GPS ASR5311 PATH
########################################
GPS_PRESET ?= ASR5311

GPS_OUT_NAME := jacana_fw.bin
GPS_SRC_PATH := ${PREBUILD_DIR}/GPS/${GPS_PRESET}


##########################################
#     DSP PATH
########################################
DSP_SRC_NAME := dsp.bin
ifneq (,$(findstring _1602_,_${SC_MODULE_BASE}_))
ifeq (4M,$(patsubst %V,4M,$(SC_HD_CFG)))
DSP_SRC_PATH := ${PREBUILD_DIR}/DSP/${ASR_MODEL}/${NET_MODEL}/wifiscan
else
ifneq (,$(findstring _WIFISCAN_,_${SC_USR_OPT}_))
DSP_SRC_PATH := ${PREBUILD_DIR}/DSP/${ASR_MODEL}/${NET_MODEL}/wifiscan
else ifneq (,$(findstring _CSSMS_,_${SC_USR_OPT}_))
DSP_SRC_PATH := ${PREBUILD_DIR}/DSP/${ASR_MODEL}/${NET_MODEL}/cssms
else
DSP_SRC_PATH := ${PREBUILD_DIR}/DSP/${ASR_MODEL}/${NET_MODEL}/normal
endif
endif
else ifneq (,$(findstring _1606_,_${SC_MODULE_BASE}_))
DSP_SRC_PATH := ${PREBUILD_DIR}/DSP/${ASR_MODEL}/${NET_MODEL}
ifeq (4M,$(patsubst %V,4M,$(SC_HD_CFG)))
ifneq (,$(findstring _VOLTE_,_${SC_USR_OPT}_))
DSP_SRC_PATH := ${PREBUILD_DIR}/DSP/${ASR_MODEL}/${NET_MODEL}/VoLTE
endif
endif
else
ifneq (,$(findstring _A7670G_LABE_1603_V202_,_${SC_MODULE_BASE}_)$(findstring _A7672G_LABE_1603_V202_,_${SC_MODULE_BASE}_))
DSP_SRC_PATH := ${PREBUILD_DIR}/DSP/${ASR_MODEL}/${NET_MODEL}/Global
else
DSP_SRC_PATH := ${PREBUILD_DIR}/DSP/${ASR_MODEL}/${NET_MODEL}
endif
endif
define dsp_usr_opt_process
ifeq ($(1),$(notdir $(wildcard ${DSP_SRC_PATH}/$(1))))
DSP_SRC_PATH := ${DSP_SRC_PATH}/$(1)
endif
endef
ifeq (${SC_MODULE_BASE}${SC_USR_OPT},$(notdir $(wildcard ${DSP_SRC_PATH}/${SC_MODULE_BASE}${SC_USR_OPT})))
DSP_SRC_PATH := ${DSP_SRC_PATH}/${SC_MODULE_BASE}${SC_USR_OPT}
else
$(foreach opt,${SC_HD_OPT_LIST},$(eval $(call dsp_usr_opt_process,${opt})))
$(foreach opt,${SC_USR_OPT_LIST},$(eval $(call dsp_usr_opt_process,${opt})))
endif

ifeq (path.txt,$(notdir $(wildcard ${DSP_SRC_PATH}/path.txt)))
DSP_SRC_PATH := ${PREBUILD_DIR}/$(shell ${CAT} $(subst ${BAD_SLASH},${GOOD_SLASH},${DSP_SRC_PATH}/path.txt))
endif


##########################################
#     BT PATH
########################################
BT_SRC_NAME1 := btbin.bin
BT_SRC_NAME2 := btlst.bin
BT_SRC_PATH := ${PREBUILD_DIR}/BT

ifeq (path.txt,$(notdir $(wildcard ${BT_SRC_PATH}/path.txt)))
BT_SRC_PATH := ${PREBUILD_DIR}/$(shell ${CAT} $(subst ${BAD_SLASH},${GOOD_SLASH},${BT_SRC_PATH}/path.txt))
endif


##########################################
#     boot33 PATH
########################################
BOOT33_SRC_NAME := boot33.bin
BOOT33_SRC_PATH := ${PREBUILD_DIR}/boot33/${ASR_MODEL}

define boot33_usr_opt_process
ifeq ($(1),$(notdir $(wildcard ${BOOT33_SRC_PATH}/$(1))))
BOOT33_SRC_PATH := ${BOOT33_SRC_PATH}/$(1)
endif
endef

ifeq (${SC_MODULE_BASE}${SC_USR_OPT},$(notdir $(wildcard ${BOOT33_SRC_PATH}/${SC_MODULE_BASE}${SC_USR_OPT})))
BOOT33_SRC_PATH := ${BOOT33_SRC_PATH}/${SC_MODULE_BASE}${SC_USR_OPT}
else
$(foreach opt,${SC_HD_OPT_LIST},$(eval $(call boot33_usr_opt_process,${opt})))
$(foreach opt,${SC_USR_OPT_LIST},$(eval $(call boot33_usr_opt_process,${opt})))
endif

ifeq (path.txt,$(notdir $(wildcard ${BOOT33_SRC_PATH}/path.txt)))
BOOT33_SRC_PATH := ${PREBUILD_DIR}/$(shell ${CAT} $(subst ${BAD_SLASH},${GOOD_SLASH},${BOOT33_SRC_PATH}/path.txt))
endif


##########################################
#     updater PATH
########################################
UPDATER_SRC_NAME := updater.bin

ifneq (,$(findstring _1606_,_${SC_MODULE_BASE}_))
	ifneq (,$(findstring A7680C,${SC_MODULE}))
		ifneq (,$(findstring _V506,_$(SC_HD_OPT)_)$(findstring _V503,_$(SC_HD_OPT)_)$(findstring _V603,_$(SC_HD_OPT)_)$(findstring _V602_,_$(SC_HD_OPT)_))
			UPDATER_SRC_PATH := ${PREBUILD_DIR}/updater/${ASR_MODEL}/UART4
		else
			UPDATER_SRC_PATH := ${PREBUILD_DIR}/updater/${ASR_MODEL}/UART2
		endif
	else ifneq (,$(findstring A7670C,${SC_MODULE}))
		ifneq (,$(findstring _V701,_$(SC_HD_OPT)_)$(findstring _V702,_$(SC_HD_OPT)_)$(findstring _V703,_$(SC_HD_OPT)_))
			UPDATER_SRC_PATH := ${PREBUILD_DIR}/updater/${ASR_MODEL}/UART4
		else
			UPDATER_SRC_PATH := ${PREBUILD_DIR}/updater/${ASR_MODEL}/UART2
		endif
	else ifneq (,$(findstring A7677S,${SC_MODULE}))
		ifneq (,$(findstring _V702,_$(SC_HD_OPT)_)$(findstring _V703,_$(SC_HD_OPT)_))
			UPDATER_SRC_PATH := ${PREBUILD_DIR}/updater/${ASR_MODEL}/UART4
		else
			UPDATER_SRC_PATH := ${PREBUILD_DIR}/updater/${ASR_MODEL}/UART2
		endif
	else
		UPDATER_SRC_PATH := ${PREBUILD_DIR}/updater/${ASR_MODEL}/UART2
	endif
else
	UPDATER_SRC_PATH := ${PREBUILD_DIR}/updater/${ASR_MODEL}
endif

define updater_usr_opt_process
ifeq ($(1),$(notdir $(wildcard ${UPDATER_SRC_PATH}/$(1))))
UPDATER_SRC_PATH := ${UPDATER_SRC_PATH}/$(1)
endif
endef
ifeq (${SC_MODULE_BASE}${SC_USR_OPT},$(notdir $(wildcard ${UPDATER_SRC_PATH}/${SC_MODULE_BASE}${SC_USR_OPT})))
UPDATER_SRC_PATH := ${UPDATER_SRC_PATH}/${SC_MODULE_BASE}${SC_USR_OPT}
else
$(foreach opt,${SC_HD_OPT_LIST},$(eval $(call updater_usr_opt_process,${opt})))
$(foreach opt,${SC_USR_OPT_LIST},$(eval $(call updater_usr_opt_process,${opt})))
endif

ifeq (path.txt,$(notdir $(wildcard ${UPDATER_SRC_PATH}/path.txt)))
UPDATER_SRC_PATH := ${PREBUILD_DIR}/$(shell ${CAT} $(subst ${BAD_SLASH},${GOOD_SLASH},${UPDATER_SRC_PATH}/path.txt))
endif


##########################################
#     logo PATH
########################################
LOGO_SRC_NAME := logo.bin
LOGO_SRC_PATH := ${PREBUILD_DIR}/logo/${ASR_MODEL}

ifeq (path.txt,$(notdir $(wildcard ${LOGO_SRC_PATH}/path.txt)))
LOGO_SRC_PATH := ${PREBUILD_DIR}/$(shell ${CAT} $(subst ${BAD_SLASH},${GOOD_SLASH},${LOGO_SRC_PATH}/path.txt))
endif


##########################################
#     RF PATH
########################################
RF_SRC_NAME := rf.bin

ifeq ($(wildcard ${PREBUILD_DIR}/RF/${SC_MODULE_BASE}${SC_USR_OPT}),)
RF_SRC_PATH := ${PREBUILD_DIR}/RF/${SC_MODULE_BASE}
else
RF_SRC_PATH := ${PREBUILD_DIR}/RF/${SC_MODULE_BASE}${SC_USR_OPT}
endif

ifeq (path.txt,$(notdir $(wildcard ${RF_SRC_PATH}/path.txt)))
RF_SRC_PATH := ${PREBUILD_DIR}/$(shell ${CAT} $(subst ${BAD_SLASH},${GOOD_SLASH},${RF_SRC_PATH}/path.txt))
endif

##########################################
#     RD PATH
########################################
RD_SRC_NAME := ReliableData.bin
RD_SRC_PATH := $(KERNEL_SRC_PATH)

endif  # ifneq (,${SIMCOM_RELEASE_SDK})





ifeq (TRUE,${FACTORY})
${OUT_DIR}/burn_factory.zip:source
else
${OUT_DIR}/burn.zip:source
endif
	$(ARELEASE) -c $(ABOOT_OUT_DIR) -g -p ${ASR_PRODUCT_TYPE} -v $(ASR_PRODUCT) --release-pack $(subst ${BAD_SLASH},${GOOD_SLASH},$(patsubst %.zip,%_source.zip,$@)) $(subst ${BAD_SLASH},${GOOD_SLASH},$@)


.PHONE: source ${ABOOT_OUT_DIR}

ifneq (,${SIMCOM_RELEASE_SDK})

source: ${ABOOT_OUT_DIR} ${ABOOT_IMAGES_OUT_DIR}/${USERSPACE_SRC_NAME}

else

source: ${ABOOT_OUT_DIR} ${ABOOT_IMAGES_OUT_DIR}/${KERNEL_SRC_NAME} ${ABOOT_IMAGES_OUT_DIR}/${APN_SRC_NAME} ${ABOOT_IMAGES_OUT_DIR}/${NVM_OUT_NAME} ${ABOOT_IMAGES_OUT_DIR}/${DSP_SRC_NAME} ${ABOOT_IMAGES_OUT_DIR}/${BOOT33_SRC_NAME} ${ABOOT_IMAGES_OUT_DIR}/${UPDATER_SRC_NAME} ${ABOOT_IMAGES_OUT_DIR}/${LOGO_SRC_NAME} ${ABOOT_IMAGES_OUT_DIR}/${RF_SRC_NAME} ${ABOOT_IMAGES_OUT_DIR}/${RD_SRC_NAME}

ifneq (,$(findstring OPENSDK,${SC_MODULE_BASE}))
source: ${ABOOT_IMAGES_OUT_DIR}/${USERSPACE_SRC_NAME}
${ABOOT_IMAGES_OUT_DIR}/${USERSPACE_SRC_NAME}: ${ABOOT_OUT_DIR} ${ABOOT_IMAGES_OUT_DIR}/${FOTA_PARAM_OUT_NAME}
endif

ifneq (,$(findstring UBIA,${SC_USR_OPT}))
source: ${ABOOT_IMAGES_OUT_DIR}/${USERSPACE_SRC_NAME}
${ABOOT_IMAGES_OUT_DIR}/${USERSPACE_SRC_NAME}: ${ABOOT_OUT_DIR} ${ABOOT_IMAGES_OUT_DIR}/${FOTA_PARAM_OUT_NAME}
endif

ifneq (,$(findstring A7680C_M,${SC_MODULE_BASE})$(findstring M5780C,${SC_MODULE_BASE})$(findstring A7673,${SC_MODULE_BASE})$(findstring A7672E_FASE_1603_V201,${SC_MODULE_BASE})$(findstring A7663,${SC_MODULE_BASE})$(findstring A7666,${SC_MODULE_BASE})$(findstring A7670U_MAMS_1606_V201,${SC_MODULE_BASE})$(findstring A7670NA_MAMS_1606_V201,${SC_MODULE_BASE})$(findstring A7670G_MAMS_1606_V201,${SC_MODULE_BASE})$(findstring A7670E_MAMS_1606_V201,${SC_MODULE_BASE})$(findstring A7670SA_MAMS_1606_V201,${SC_MODULE_BASE})$(findstring A7670G_MNMY_1602_UB_V101,${SC_MODULE_BASE})$(findstring A7672SA_FASE_1603_V201,${SC_MODULE_BASE}))
source: ${ABOOT_IMAGES_OUT_DIR}/${GPS_OUT_NAME} 
endif

ifneq (,$(SC_HD_CFG))
ifneq ($(SC_HD_CFG)_$(SC_HD_CFG),$(patsubst _F%,,$(SC_HD_CFG))_$(patsubst _B%,,$(SC_HD_CFG)))
source: ${ABOOT_IMAGES_OUT_DIR}/${BT_SRC_NAME1}
endif
endif

ifneq (,$(SC_HD_CFG))
ifneq (,$(findstring _1603_,_${SC_MODULE_FULL}_))
ifneq (,$(findstring _TOPFLY_, _${SC_USR_OPT}_))
source: ${ABOOT_IMAGES_OUT_DIR}/${BT_SRC_NAME1}
endif
endif
endif

endif


ifneq (,${SIMCOM_RELEASE_SDK})
${ABOOT_OUT_DIR}:${ABOOT_SRC_PATH}
else
${ABOOT_OUT_DIR}:${ABOOT_CONFIG_SRC_PATH} ${ABOOT_IMAGES_SRC_PATH}
endif
	-${RMDIR} ${RMDIRARG} $(subst ${BAD_SLASH},${GOOD_SLASH},${ABOOT_OUT_DIR})
	${MKDIR} ${MKDIRARG} $(subst ${BAD_SLASH},${GOOD_SLASH},${ABOOT_OUT_DIR})
ifneq (,${SIMCOM_RELEASE_SDK})
ifeq (win32,${PLATFORM})
	${COPY} ${COPYARG} $(subst ${BAD_SLASH},${GOOD_SLASH},$</${ABOOT_SRC_NAME}) $(subst ${BAD_SLASH},${GOOD_SLASH},$@/)
else
	${COPY} ${COPYARG} $(subst ${BAD_SLASH},${GOOD_SLASH},$</${ABOOT_SRC_NAME}) $(subst ${BAD_SLASH},${GOOD_SLASH},$(dir $@))
endif
else
	${MKDIR} ${MKDIRARG} $(subst ${BAD_SLASH},${GOOD_SLASH},$@/${ABOOT_CONFIG_SRC_NAME})
	${COPY} ${COPYARG} $(subst ${BAD_SLASH},${GOOD_SLASH},${ABOOT_CONFIG_SRC_PATH}/${ABOOT_CONFIG_SRC_NAME}) $(subst ${BAD_SLASH},${GOOD_SLASH},$@/${ABOOT_CONFIG_SRC_NAME}/)
	${MKDIR} ${MKDIRARG} $(subst ${BAD_SLASH},${GOOD_SLASH},$@/${ABOOT_IMAGES_SRC_NAME})
	${COPY} ${COPYARG} $(subst ${BAD_SLASH},${GOOD_SLASH},${ABOOT_IMAGES_SRC_PATH}/${ABOOT_IMAGES_SRC_NAME}) $(subst ${BAD_SLASH},${GOOD_SLASH},$@/${ABOOT_IMAGES_SRC_NAME}/)
endif

ifneq (,$(findstring OPENSDK,${SC_MODULE_BASE}))
${ABOOT_IMAGES_OUT_DIR}/${USERSPACE_SRC_NAME}:${USERSPACE_SRC_PATH}/${USERSPACE_SRC_NAME}
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},$<) $(subst ${BAD_SLASH},${GOOD_SLASH},$(dir $@))

${ABOOT_IMAGES_OUT_DIR}/${FOTA_PARAM_OUT_NAME}:${FOTA_PARAM_SRC_PATH}
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},$</${FOTA_PARAM_OUT_NAME}) $(subst ${BAD_SLASH},${GOOD_SLASH},$(dir $@))
endif

ifneq (,$(findstring UBIA,${SC_USR_OPT}))
${ABOOT_IMAGES_OUT_DIR}/${USERSPACE_SRC_NAME}:${USERSPACE_SRC_PATH}/${USERSPACE_SRC_NAME}
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},$<) $(subst ${BAD_SLASH},${GOOD_SLASH},$(dir $@))

${ABOOT_IMAGES_OUT_DIR}/${FOTA_PARAM_OUT_NAME}:${FOTA_PARAM_SRC_PATH}
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},$</${FOTA_PARAM_OUT_NAME}) $(subst ${BAD_SLASH},${GOOD_SLASH},$(dir $@))
endif

ifeq (,${SIMCOM_RELEASE_SDK})

${ABOOT_IMAGES_OUT_DIR}/${KERNEL_SRC_NAME}:${KERNEL_SRC_PATH}/${KERNEL_SRC_NAME}
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},$<) $(subst ${BAD_SLASH},${GOOD_SLASH},$(dir $@))

${ABOOT_IMAGES_OUT_DIR}/${APN_SRC_NAME}:${KERNEL_SRC_PATH}/${APN_SRC_NAME}
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},$<) $(subst ${BAD_SLASH},${GOOD_SLASH},$(dir $@))


${ABOOT_IMAGES_OUT_DIR}/${NVM_OUT_NAME}:${NVM_SRC_PATH} ${ABOOT_OUT_DIR}
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},$</${NVM_SRC_NAME}) $(subst ${BAD_SLASH},${GOOD_SLASH},$@)


${ABOOT_IMAGES_OUT_DIR}/${DSP_SRC_NAME}:${DSP_SRC_PATH}
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},$</${DSP_SRC_NAME}) $(subst ${BAD_SLASH},${GOOD_SLASH},$(dir $@))


${ABOOT_IMAGES_OUT_DIR}/${GPS_OUT_NAME}:${GPS_SRC_PATH}
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},$</${GPS_OUT_NAME}) $(subst ${BAD_SLASH},${GOOD_SLASH},$(dir $@))


${ABOOT_IMAGES_OUT_DIR}/${BOOT33_SRC_NAME}:${BOOT33_SRC_PATH}
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},$</${BOOT33_SRC_NAME}) $(subst ${BAD_SLASH},${GOOD_SLASH},$(dir $@))


${ABOOT_IMAGES_OUT_DIR}/${UPDATER_SRC_NAME}:${UPDATER_SRC_PATH}
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},$</${UPDATER_SRC_NAME}) $(subst ${BAD_SLASH},${GOOD_SLASH},$(dir $@))


${ABOOT_IMAGES_OUT_DIR}/${BT_SRC_NAME1}:${BT_SRC_PATH}
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},$</${BT_SRC_NAME1}) $(subst ${BAD_SLASH},${GOOD_SLASH},$(dir $@))
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},$</${BT_SRC_NAME2}) $(subst ${BAD_SLASH},${GOOD_SLASH},$(dir $@))


${ABOOT_IMAGES_OUT_DIR}/${LOGO_SRC_NAME}:${LOGO_SRC_PATH}
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},$</${LOGO_SRC_NAME}) $(subst ${BAD_SLASH},${GOOD_SLASH},$(dir $@))


${ABOOT_IMAGES_OUT_DIR}/${RF_SRC_NAME}:${RF_SRC_PATH}
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},$</${RF_SRC_NAME}) $(subst ${BAD_SLASH},${GOOD_SLASH},$(dir $@))


${ABOOT_IMAGES_OUT_DIR}/${RD_SRC_NAME}:${RD_SRC_PATH}
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},$</${RD_SRC_NAME}) $(subst ${BAD_SLASH},${GOOD_SLASH},$(dir $@))

endif  # ifeq (,${SIMCOM_RELEASE_SDK})


