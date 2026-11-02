#############################
#  create aboot package
############################

include env.mak
include ${KERNEL_DIR}/${SC_MODULE_FULL}/scinfo.mak
include make_image_16xx_settings.mak


ABOOT_DIR := $(TOOL_DIR)/$(PLATFORM)/aboot
ARELEASE := $(ABOOT_DIR)/arelease
ARELEASE := $(subst $(BAD_SLASH),$(GOOD_SLASH),$(ARELEASE))

OUT ?= out
ASR_PLAT ?= 16xx

SC_USR_OPT_LIST := $(subst _, ,${SC_USR_OPT})
SC_HD_OPT_LIST := $(subst _, ,${SC_HD_OPT})



OUT_DIR := ${ROOT_DIR}/${OUT}/${SC_MODULE_FULL}
ifeq (TRUE,${FACTORY})
ABOOT_OUT_DIR := ${OUT_DIR}/${SC_MODULE_FULL}_aboot_factory
else
ABOOT_OUT_DIR := ${OUT_DIR}/${SC_MODULE_FULL}_aboot
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
ABOOT_SRC_NAME := ${SC_MODULE_FULL}_aboot_factory
else
ABOOT_SRC_NAME := ${SC_MODULE_FULL}_aboot
endif
ABOOT_SRC_PATH := ${KERNEL_DIR}/${SC_MODULE_FULL}

else  # ifneq (,${SIMCOM_RELEASE_SDK})

ifneq (,$(findstring _1606_,_${SC_MODULE_BASE}_))
ASR_MODEL := CRANEL
NET_MODEL := C1
else ifneq (,$(findstring _1603_,_${SC_MODULE_BASE}_))
ASR_MODEL := CRANEM
NET_MODEL := C1G
else ifneq (,$(findstring _1601_,_${SC_MODULE_BASE}_))
ASR_MODEL := CRANE
NET_MODEL := C1G
endif







##########################################
#     aboot config PATH
########################################
ABOOT_CONFIG_SRC_NAME := config
ABOOT_CONFIG_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/aboot/${BASE_LINE}/${ASR_MODEL}/config

ifeq (path.txt,$(notdir $(wildcard ${ABOOT_CONFIG_SRC_PATH}/path.txt)))
ABOOT_CONFIG_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/$(shell ${CAT} $(subst ${BAD_SLASH},${GOOD_SLASH},${ABOOT_CONFIG_SRC_PATH}/path.txt))
endif

ABOOT_IMAGES_SRC_NAME := images
ABOOT_IMAGES_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/aboot/${BASE_LINE}/${ASR_MODEL}/images

ifeq (path.txt,$(notdir $(wildcard ${ABOOT_IMAGES_SRC_PATH}/path.txt)))
ABOOT_IMAGES_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/$(shell ${CAT} $(subst ${BAD_SLASH},${GOOD_SLASH},${ABOOT_IMAGES_SRC_PATH}/path.txt))
endif


##########################################
#     NVM PATH
########################################
ifneq (,$(SC_HD_CFG))
ifneq ($(SC_HD_CFG)_$(SC_HD_CFG),$(patsubst _F%,,$(SC_HD_CFG))_$(patsubst _M%,,$(SC_HD_CFG)))
ifneq (,$(findstring A7672S,${SC_MODULE}))
NVM_PRESET := GPS
endif
endif
endif
NVM_PRESET ?= NULL

NVM_OUT_NAME := nvm.bin
NVM_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/NVM/${NVM_PRESET}

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
FOTA_PARAM_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/FOTA_PARAM/${BASE_LINE}/${ASR_MODEL}/FACTORY
else
FOTA_PARAM_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/FOTA_PARAM/${BASE_LINE}/${ASR_MODEL}
endif
endif
endif
##########################################
#     GPS ASR5311 PATH
########################################
GPS_PRESET ?= ASR5311

GPS_OUT_NAME := jacana_fw.bin
GPS_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/GPS/${GPS_PRESET}


##########################################
#     DSP PATH
########################################
DSP_SRC_NAME := dsp.bin
DSP_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/DSP/${BASE_LINE}/${ASR_MODEL}/${NET_MODEL}

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
DSP_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/$(shell ${CAT} $(subst ${BAD_SLASH},${GOOD_SLASH},${DSP_SRC_PATH}/path.txt))
endif


##########################################
#     BT PATH
########################################
BT_SRC_NAME1 := btbin.bin
BT_SRC_NAME2 := btlst.bin
BT_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/BT/${BASE_LINE}

ifeq (path.txt,$(notdir $(wildcard ${BT_SRC_PATH}/path.txt)))
BT_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/$(shell ${CAT} $(subst ${BAD_SLASH},${GOOD_SLASH},${BT_SRC_PATH}/path.txt))
endif


##########################################
#     boot33 PATH
########################################
BOOT33_SRC_NAME := boot33.bin
BOOT33_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/boot33/${BASE_LINE}/${ASR_MODEL}

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
BOOT33_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/$(shell ${CAT} $(subst ${BAD_SLASH},${GOOD_SLASH},${BOOT33_SRC_PATH}/path.txt))
endif


##########################################
#     updater PATH
########################################
UPDATER_SRC_NAME := updater.bin
UPDATER_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/updater/${BASE_LINE}/${ASR_MODEL}

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
UPDATER_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/$(shell ${CAT} $(subst ${BAD_SLASH},${GOOD_SLASH},${UPDATER_SRC_PATH}/path.txt))
endif


##########################################
#     logo PATH
########################################
LOGO_SRC_NAME := logo.bin
LOGO_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/logo/${BASE_LINE}/${ASR_MODEL}

ifeq (path.txt,$(notdir $(wildcard ${LOGO_SRC_PATH}/path.txt)))
LOGO_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/$(shell ${CAT} $(subst ${BAD_SLASH},${GOOD_SLASH},${LOGO_SRC_PATH}/path.txt))
endif


##########################################
#     RF PATH
########################################
RF_SRC_NAME := rf.bin
RF_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/RF/${BASE_LINE}/${SC_MODULE_BASE}

ifeq (path.txt,$(notdir $(wildcard ${RF_SRC_PATH}/path.txt)))
RF_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/$(shell ${CAT} $(subst ${BAD_SLASH},${GOOD_SLASH},${RF_SRC_PATH}/path.txt))
endif


##########################################
#     RD PATH
########################################
RD_SRC_NAME := ReliableData.bin
ifeq (1.011.069,${BASE_LINE})
RD_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/RD/${BASE_LINE}/${SC_MODULE_BASE}
define rd_usr_opt_process
ifeq ($(1),$(notdir $(wildcard ${RD_SRC_PATH}/$(1))))
RD_SRC_PATH := ${RD_SRC_PATH}/$(1)
endif
endef
$(foreach opt,${SC_USR_OPT_LIST},$(eval $(call rd_usr_opt_process,${opt})))

ifeq (path.txt,$(notdir $(wildcard ${RD_SRC_PATH}/path.txt)))
RD_SRC_PATH := ${PREBUILD_DIR}/${ASR_PLAT}/$(shell ${CAT} $(subst ${BAD_SLASH},${GOOD_SLASH},${RD_SRC_PATH}/path.txt))
endif
else
RD_SRC_PATH := $(KERNEL_SRC_PATH)
endif #ifeq (1.011.069,${BASE_LINE})
endif  # ifneq (,${SIMCOM_RELEASE_SDK})





ifeq (TRUE,${FACTORY})
${OUT_DIR}/${SC_MODULE_FULL}_factory.zip:source
else
${OUT_DIR}/${SC_MODULE_FULL}.zip:source
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

ifneq (,$(findstring A7680C_M,${SC_MODULE_BASE})$(findstring M5780C,${SC_MODULE_BASE}))
source: ${ABOOT_IMAGES_OUT_DIR}/${GPS_OUT_NAME}
endif

ifneq (,$(SC_HD_CFG))
ifneq ($(SC_HD_CFG)_$(SC_HD_CFG),$(patsubst _F%,,$(SC_HD_CFG))_$(patsubst _B%,,$(SC_HD_CFG)))
source: ${ABOOT_IMAGES_OUT_DIR}/${BT_SRC_NAME1}
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


