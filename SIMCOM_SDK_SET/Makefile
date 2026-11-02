#
# Makefile
#

CACHE_FILE = cache.mak

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
ROOT_DIR := $(patsubst %/,%, $(dir $(mkfile_path)))

-include ${CACHE_FILE}

ifneq (,${GIT_ID})
GIT_ID_CACHE := YES
endif
ifneq (,${TOOL_DIR})
TOOL_DIR_CACHE := YES
endif
ifneq (,${APP_DIR})
APP_DIR_CACHE := YES
endif
ifneq (,${PREBUILD_DIR})
PREBUILD_DIR_CACHE := YES
endif

ifeq (kernel, $(notdir $(wildcard $(ROOT_DIR)/kernel)))
SIMCOM_RELEASE_SDK := YES
SIMCOM_SDK_MACRO := $(strip -DSIMCOM_SDK=ON)
TOOL_DIR := $(ROOT_DIR)/tools
APP_DIR := $(ROOT_DIR)
KERNEL_DIR := $(ROOT_DIR)/kernel
else
SIMCOM_RELEASE_SDK :=
SIMCOM_SDK_MACRO :=
KERNEL_DIR := $(ROOT_DIR)/tavor/Arbel/kernel
TOOL_DIR ?= $(ROOT_DIR)/tavor/tools
APP_DIR ?= $(ROOT_DIR)/simcom/userspace
PREBUILD_DIR ?= $(ROOT_DIR)/tavor/Arbel/prebuild_images
J ?= 1
endif

OUT = out
RELEASE = release
RELEASE_SET = SIMCOM_SDK_SET
MAKEFILENAME := Makefile
CRC_SET := crc_set
OBJCPY := arm-none-eabi-objcopy
API_MAP_FILE_NAME := api_map
APP_NAME := customer_app

KERNEL_LINK_DEFINE := kernel_link_define.mak

-include ${TOOL_DIR}/script/env.mak

SC_TOOLCHAIN_DIR := ${TOOL_DIR}/${PLATFORM}
CROSS_TOOL_DIR := ${TOOL_DIR}/${PLATFORM}/cross_tool
GCC_ARM_TOOLCHAIN_DIR := ${CROSS_TOOL_DIR}/gcc-arm-none-eabi
GCC_ARM_TOOLCHAIN := ${GCC_ARM_TOOLCHAIN_DIR}/bin

API_MAP_MAKEFILE := ${ROOT_DIR}/simcom/config/apiMapInit.mak
API_MAP_INIT_SCRIPT := ${TOOL_DIR}/script/sapiInit.py

ifneq (,$(SIMCOM_RELEASE_SDK))
TARGET_IMAGE_MAKEFILE := ${ROOT_DIR}/config/make_image_16xx.mak
else
TARGET_IMAGE_MAKEFILE := ${ROOT_DIR}/simcom/config/make_image_16xx.mak
endif

export PATH := ${PATH}$(GOOD_BREAKER)$(subst $(BAD_SLASH),$(GOOD_SLASH),${TOOL_DIR}/${PLATFORM})$(GOOD_BREAKER)$(subst $(BAD_SLASH),$(GOOD_SLASH),${GCC_ARM_TOOLCHAIN})


-include ${ROOT_DIR}/simcom/config/custom_list.mak
-include ${APP_DIR}/makeDepend.mak

MAKEFILE_DEPEND = $(patsubst %,${APP_DIR}/%,${MAKEFILE_DEPEND_LIST})


ifneq (,$(SIMCOM_RELEASE_SDK))
SCMODULE := $(sort $(notdir $(wildcard $(KERNEL_DIR)/*)))
else
SCMODULE := $(sort $(notdir $(wildcard ${ROOT_DIR}/simcom/config/config_*.mak)))
SCMODULE := ${patsubst config_%.mak,%,${SCMODULE}}
endif


.PHONY:info info_pre cleanAll clean clean_all_app clean_all_kernel


################################
# help info define
################################
info: info_pre info_modules info_end

all:info-all
info-all: info_pre info_modules_custom info_end

info_end:
	@echo -
	@echo -  config option list:
	@echo -      menuconfig            [do menuconfig for app]
	@echo -      guiconfig             [do guiconfig for app]
	@echo -      clean_config          [clean the userconfig]
	@echo -
	@echo -  clean option list:
	@echo -      clean                 [clean all modules.]
ifneq (,$(SIMCOM_RELEASE_SDK))
	@echo -      clean_[module]        [clean a module.]
else
	@echo -      clean_app             [clean all app, target and object files.]
	@echo -      clean_kernel          [clean all kernel, just target files.]
	@echo -      clean_[module]        [clean app and kernel, just kernel target files.]
	@echo -      clean_[module]_app    [target and object files.]
	@echo -      clean_[module]_kernel [just target files.]
endif
ifneq (,$(SIMCOM_RELEASE_SDK))
else
	@echo -
	@echo -  install option list:
	@echo -      install_[module]      [release one module to a SDK package. it will compile automatically when it not been compiled].
	@echo -      install_set           [release all modules in one SDK package, which have been compiled.]
	@echo -      uninstall_[module]    [delete the SDK package for one module.]
	@echo -      uninstall_set         [delete the SDK package, which is mix some different modules.]
	@echo -      uninstall             [delete all SDK package.]
endif
	@echo -
	@echo ---------------------------------------------

info_pre:
	@echo ---------------------------------------------
	@echo -
ifneq (,$(SIMCOM_RELEASE_SDK))
	@echo -  build method: gnumake [target]
	@echo -  target:[module],[clean option]
else
	@echo -  build method: gnumake [target] (GIT_ID=[work id]) (PREBUILD_DIR=[dir]) (TOOL_DIR=[dir]) (APP_DIR=[dir]) (J=[jobs])
	@echo -  target:[module](_[custom])(_app/_kernel/_force/_remake/_releaseSource),[clean option],[install option],upgrade
	@echo -
	@echo -  _releaseSource will make the source code package which without simcom source code and include a simcom lib.
	@echo -
	@echo -  GIT_ID is used to dowload userspace code and tools automatically.
	@echo -  PREBUILD_DIR is set the prebuild images dir, defulte is $$(ROOT_DIR)/tavor/Arbel/prebuild_images
	@echo -  TOOL_DIR is set the tools dir, defulte is $$(ROOT_DIR)/tavor/tools
	@echo -  APP_DIR is set the userspace dir, defulte is $$(ROOT_DIR)/simcom/userspace
	@echo -
	@echo -  show customs of the modules: gnumake [module]-list
	@echo -  show all modules and customs at onece: gnumake all/info-all
	@echo -
	@echo -  upgrade:
	@echo -    Do git pull --rebase for all. If the sub-warehouse has not cloned yet, it will clone it automatically.
endif
	@echo -
	@echo -  module list:

info_modules:

info_modules_custom:
# help info define end


################################
#    git pull all
################################
ifeq (,$(SIMCOM_RELEASE_SDK))

.PHONY: upgrade upgrade_base upgrade_tools upgrade_app upgrade_prebuild create_cache

upgrade:create_cache upgrade_base upgrade_tools upgrade_app upgrade_prebuild

upgrade_base:
	@echo ----------------------------
	@echo ----- uprade base code -----
	@echo ----------------------------
	-git pull --rebase

upgrade_tools:${TOOL_DIR}
	@echo ------------------------
	@echo ----- uprade tools -----
	@echo ------------------------
	-$(word 1,$(subst :, ,$<)): && cd $< && git pull --rebase

upgrade_app:${APP_DIR}
	@echo ----------------------------
	@echo ----- uprade userspace -----
	@echo ----------------------------
	-$(word 1,$(subst :, ,$<)): && cd $< && git pull --rebase

upgrade_prebuild:${PREBUILD_DIR}
	@echo ----------------------------------
#	@echo ----- uprade prebuild images -----
#	@echo ----------------------------------
#	-$(word 1,$(subst :, ,$<)): && cd $< && git pull --rebase

create_cache:
	echo ###########################> ${CACHE_FILE}
	echo #   makefile cache data   #>> ${CACHE_FILE}
	echo ###########################>> ${CACHE_FILE}
ifneq (,${GIT_ID_CACHE})
	echo GIT_ID ?= ${GIT_ID}>> ${CACHE_FILE}
endif
ifneq (,${TOOL_DIR_CACHE})
	echo TOOL_DIR ?= ${TOOL_DIR}>> ${CACHE_FILE}
endif
ifneq (,${APP_DIR_CACHE})
	echo APP_DIR ?= ${APP_DIR}>> ${CACHE_FILE}
endif
ifneq (,${PREBUILD_DIR_CACHE})
	echo PREBUILD_DIR ?= ${PREBUILD_DIR}>> ${CACHE_FILE}
endif
	echo $(strip J ?= ${J})>> ${CACHE_FILE}

else #  ifeq (,$(SIMCOM_RELEASE_SDK))
upgrade_base:
upgrade_tools:
upgrade_app:
upgrade_prebuild:
create_cache:
endif #  ifeq (,$(SIMCOM_RELEASE_SDK))


################################
# tools process
################################
ifeq (,$(SIMCOM_RELEASE_SDK))
${TOOL_DIR}:
	$(word 1,$(subst :, ,$@)): && cd $(subst $(BAD_SLASH),$(GOOD_SLASH),$(dir $@)) && git clone "ssh://${GIT_ID}@10.238.1.10:29418/OpenCpu" -b tools tools
endif

${MSYS2_PACKAGE}:${TOOL_DIR}
${TOOL_DIR}/${PLATFORM}/${CMAKE_PACKAGE_NAME}:${TOOL_DIR}
${TOOL_DIR}/${PLATFORM}/${GCC_ARM_TOOLCHAIN_PACKAGE}:${TOOL_DIR}
$(API_MAP_INIT_SCRIPT):${TOOL_DIR}

ifeq (win32,${PLATFORM})
ifneq (,${MSYS2_DIR})
${MSYS2_DIR}:${MSYS2_PACKAGE}
	$(word 1,$(subst :, ,$@)): && cd $(dir $@) && $(GZIP) $(GZIPARG) $<
	$(word 1,$(subst :, ,$@)): && cd $@ && $(MKDIR) $(MKDIRARG) fuck_windows && $(RMDIR) $(RMDIRARG) fuck_windows
endif
else
${MSYS2_DIR}:
${CMAKE_DIR}:
endif

ifneq (,${CMAKE_DIR})
${CMAKE_DIR}:${TOOL_DIR}/${PLATFORM}/${CMAKE_PACKAGE_NAME}
ifeq (win32,${PLATFORM})
	$(word 1,$(subst :, ,$@)): && cd $(dir $@) && $(GZIP) $(GZIPARG) $<
	$(word 1,$(subst :, ,$@)): && cd $@ && $(MKDIR) $(MKDIRARG) fuck_windows && $(RMDIR) $(RMDIRARG) fuck_windows
else
	cd $(dir $@) && $(GZIP) $(GZIPARG) $<
endif
endif

${CROSS_TOOL_DIR}:${TOOL_DIR}/${PLATFORM}/${GCC_ARM_TOOLCHAIN_PACKAGE}
ifeq (win32,${PLATFORM})
	$(word 1,$(subst :, ,$@)): && cd $(dir $@) && $(GZIP) $(GZIPARG) $<
	$(word 1,$(subst :, ,$@)): && cd $@ && $(MKDIR) $(MKDIRARG) fuck_windows && $(RMDIR) $(RMDIRARG) fuck_windows
else
	cd $(dir $@) && $(GZIP) $(GZIPARG) $<
endif
# tools process end

################################
# app code process
################################
ifeq (,$(SIMCOM_RELEASE_SDK))
${APP_DIR}:
	$(word 1,$(subst :, ,$@)): && cd $(subst $(BAD_SLASH),$(GOOD_SLASH),$(dir $@)) && git clone "ssh://${GIT_ID}@10.238.1.10:29418/OpenCpu" -b userspace_1.1 userspace
endif

$(APP_DIR)/config/ToolChain.cmake:$(APP_DIR)
$(APP_DIR)/config/Config_APP.cmake:$(APP_DIR)
# app code process end



################################
# clean option define
################################
clean: clean_app

ifneq (,$(SIMCOM_RELEASE_SDK))
else
clean: clean_kernel cleanAll clean_file

clean_file:

cleanAll:
	cd ./tavor/Arbel/build && cleanAll.bat

clean_kernel:
	-$(RMDIR) $(RMDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(KERNEL_DIR))
	cd $(ROOT_DIR)/tavor/Arbel/bin && $(RM) $(RMARG) *

uninstall:
	-$(RMDIR) $(RMDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(ROOT_DIR)/$(RELEASE))
endif

clean_app:
	-$(RMDIR) $(RMDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(OUT))

clean_config:
	-${RM} ${RMARG} $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/config/.config)
	-${RM} ${RMARG} $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/config/config.h)
# clean option define end

## just released SDK with a .config file can skip
$(APP_DIR)/config/.config:$(APP_DIR)/config/Kconfig
ifeq (YES_.config_,${SIMCOM_RELEASE_SDK}_$(notdir $(wildcard $(APP_DIR)/config/.config))_$(patsubst %.exe,%,$(notdir $(shell ${WHERE} genconfig))))
	@echo genconfig cmd not found. ... skip ...
else
ifeq (win32,${PLATFORM})
	$(word 1,$(subst :, ,$<)): && cd $(subst ${BAD_SLASH},${GOOD_SLASH},$(dir $@)) && genconfig --config-out .config
else
	cd $(subst ${BAD_SLASH},${GOOD_SLASH},$(dir $@)) && genconfig --config-out .config
endif
	-${RM} ${RMARG} $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/config/config.h)
endif

menuconfig: ${APP_DIR}/config/Kconfig
ifeq (win32,${PLATFORM})
	$(word 1,$(subst :, ,$<)): && cd $(subst $(BAD_SLASH),$(GOOD_SLASH),$(dir $<)) && menuconfig
else
	cd $(subst $(BAD_SLASH),$(GOOD_SLASH),$(dir $<)) && menuconfig
endif

guiconfig: ${APP_DIR}/config/Kconfig
ifeq (win32,${PLATFORM})
	$(word 1,$(subst :, ,$<)): && cd $(subst $(BAD_SLASH),$(GOOD_SLASH),$(dir $<)) && guiconfig
else
	cd $(subst $(BAD_SLASH),$(GOOD_SLASH),$(dir $<)) && guiconfig
endif

################################
# target create
################################
define user_option_process
################################
## info create
################################
ifeq (,$(4))
info_modules:$(1)$(2)$(3)$(4)_info
endif
info_modules_custom:$(1)$(2)$(3)$(4)_info

$(1)$(2)$(3)$(4)_info:
	@echo -      $(1)$(2)$(3)$(4)

ifneq (,$(4))
$(1)$(2)$(3)-list:$(1)$(2)$(3)-list_s $(patsubst _%,%,$(4))_info
else
$(1)$(2)$(3)-list:
	@echo ---------------------------------------------

$(1)$(2)$(3)-list_s:
	@echo ---------------------------------------------
	@echo -  $(1)$(2)$(3) custom list:
endif
## info create end



.PHONY: $(1)$(2)$(3)$(4) $(ROOT_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/burn.zip

ifeq (,$(SIMCOM_RELEASE_SDK))
$(1)$(2)$(3)$(4):upgrade_tools
	@echo ---------------------------
	@echo ----- build success!! -----
	@echo ---------------------------
endif

install_$(1)$(2)$(3)$(4):$(ROOT_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/burn.zip
## create the package used to burn
$(ROOT_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/burn.zip:$(TARGET_IMAGE_MAKEFILE) ${PREBUILD_DIR} upgrade_prebuild
	@echo --------------------------------
	@echo ----- create aboot package -----
	@echo --------------------------------
	-${RM} ${RMARG} $$(subst $${BAD_SLASH},$${GOOD_SLASH},$$@)
	-${RMDIR} ${RMDIRARG} $$(subst $${BAD_SLASH},$${GOOD_SLASH},$$(dir $$@)aboot)
	-${RMDIR} ${RMDIRARG} $$(subst $${BAD_SLASH},$${GOOD_SLASH},$$(dir $$@)aboot_factory)
	$(MAKE) -C $$(dir $$<) -f $$(notdir $$<) ROOT_DIR=${ROOT_DIR} KERNEL_DIR=$(KERNEL_DIR) APP_DIR=${APP_DIR} TOOL_DIR=$(TOOL_DIR) PREBUILD_DIR=${PREBUILD_DIR} SC_MODULE=$(1) SC_HD_CFG=${2} SC_HD_OPT=$(3) SC_USR_OPT=$(4) SC_MODULE_BASE=$(1)$(2)$(3) SC_MODULE_FULL=$(1)$(2)$(3)$(4) OUT=${OUT} APP_NAME=$(APP_NAME) SIMCOM_RELEASE_SDK=${SIMCOM_RELEASE_SDK}
	$(MAKE) -C $$(dir $$<) -f $$(notdir $$<) ROOT_DIR=${ROOT_DIR} KERNEL_DIR=$(KERNEL_DIR) APP_DIR=${APP_DIR} TOOL_DIR=$(TOOL_DIR) PREBUILD_DIR=${PREBUILD_DIR} SC_MODULE=$(1) SC_HD_CFG=${2} SC_HD_OPT=$(3) SC_USR_OPT=$(4) SC_MODULE_BASE=$(1)$(2)$(3) SC_MODULE_FULL=$(1)$(2)$(3)$(4) OUT=${OUT} APP_NAME=$(APP_NAME) SIMCOM_RELEASE_SDK=${SIMCOM_RELEASE_SDK} ASR_PLAT=160X FACTORY=TRUE
ifneq (,$(SIMCOM_RELEASE_SDK))
else ifneq (,$(findstring _OPENSDK_,_$(3)_))
	-${RMDIR} ${RMDIRARG} $$(subst $${BAD_SLASH},$${GOOD_SLASH},${KERNEL_DIR}/$(1)$(2)$(3)$(4)/aboot/)
	-${MKDIR} ${MKDIRARG} $$(subst $${BAD_SLASH},$${GOOD_SLASH},${KERNEL_DIR}/$(1)$(2)$(3)$(4)/aboot/)
	-${RMDIR} ${RMDIRARG} $$(subst $${BAD_SLASH},$${GOOD_SLASH},${KERNEL_DIR}/$(1)$(2)$(3)$(4)/aboot_factory/)
	-${MKDIR} ${MKDIRARG} $$(subst $${BAD_SLASH},$${GOOD_SLASH},${KERNEL_DIR}/$(1)$(2)$(3)$(4)/aboot_factory/)
	${COPY} ${COPYARG} $$(subst $${BAD_SLASH},$${GOOD_SLASH},$$(dir $$@)aboot) $$(subst $${BAD_SLASH},$${GOOD_SLASH},${KERNEL_DIR}/$(1)$(2)$(3)$(4)/aboot/)
	${COPY} ${COPYARG} $$(subst $${BAD_SLASH},$${GOOD_SLASH},$$(dir $$@)aboot_factory) $$(subst $${BAD_SLASH},$${GOOD_SLASH},${KERNEL_DIR}/$(1)$(2)$(3)$(4)/aboot_factory/)
endif

$(1)$(2)$(3)$(4)_remake:
	@$(MAKE) clean
	@$(MAKE) $(1)$(2)$(3)$(4)

################################
## openSDK source or released SDK build flow
################################
ifneq (,$(findstring _OPENSDK_,_$(3)_)$(SIMCOM_RELEASE_SDK))

$(1)$(2)$(3)$(4):create_cache $(ROOT_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/burn.zip

ifeq (,$(SIMCOM_RELEASE_SDK))
$(1)$(2)$(3)$(4)_app:
	@$(MAKE) clean_$(1)$(2)$(3)$(4)_app
	@$(MAKE) $(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(APP_NAME).bin
	@echo ---------------------------
	@echo ----- build success!! -----
	@echo ---------------------------

$(1)$(2)$(3)$(4)_kernel:
	@$(MAKE) clean_$(1)$(2)$(3)$(4)_kernel
	@$(MAKE) $(KERNEL_DIR)/$(1)$(2)$(3)$(4)/cp.bin
	@echo ---------------------------
	@echo ----- build success!! -----
	@echo ---------------------------

$(1)$(2)$(3)$(4)_force:
	@$(MAKE) clean_$(1)$(2)$(3)$(4)
	@$(MAKE) $(1)$(2)$(3)$(4)
endif

$(ROOT_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/burn.zip:$(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(APP_NAME).bin $(KERNEL_DIR)/$(1)$(2)$(3)$(4)/cp.bin

## create bin from elf
$(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(APP_NAME).bin: $(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(APP_NAME).elf
	${OBJCPY} -O binary $$< $$@
	$(CRC_SET) $$@ $$(patsubst %/,%,$$(dir $$@))/$(APP_NAME)_crc.bin
ifeq (win32,$(PLATFORM))
	$(RM) $(RMARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$@)
	$(MOVE) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(patsubst %/,%,$$(dir $$@))/$(APP_NAME)_crc.bin) $$(notdir $$@)
else
	$(RM) $(RMARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$@)
	$(MOVE) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(patsubst %/,%,$$(dir $$@))/$(APP_NAME)_crc.bin) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$@)
endif

## do make or ninja
.PHONY: $(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(APP_NAME).elf

$(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(APP_NAME).elf: $(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(MAKEFILENAME) $(APP_DIR)/config/$(1)$(2)$(3)$(4)_linkscript.ld
	@echo ---------------------------
	@echo ----- build userspace -----
	@echo ---------------------------
ifeq (win32,${PLATFORM})
	$$(word 1,$$(subst :, ,$$@)): && $(BUILD) -C $$(dir $$<)
else
	$(BUILD) -C $$(dir $$<)
endif

ifeq (,$(SIMCOM_RELEASE_SDK))
$(APP_DIR)/config/$(1)$(2)$(3)$(4)_linkscript.ld:$(KERNEL_DIR)/$(1)$(2)$(3)$(4)/${KERNEL_LINK_DEFINE}
	@echo ----------------------------------------
	@echo ----- create userspace link script -----
	@echo ----------------------------------------
	-${RM} ${RMARG} $$(subst ${BAD_SLASH},${GOOD_SLASH},$$@)
	python ${TOOL_DIR}/script/userspace_linkscript_generator.py $$< $$@
endif

# $(KERNEL_DIR)/$(1)$(2)$(3)$(4)/${KERNEL_LINK_DEFINE}:$(KERNEL_DIR)/$(1)$(2)$(3)$(4)/KernelDefinitions.cmake

##  do cmake create makefile for make or ninja
$(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(MAKEFILENAME): $(APP_DIR) $(CMAKE_DIR) ${CROSS_TOOL_DIR} $(APP_DIR)/config/ToolChain.cmake $(APP_DIR)/config/Config_APP.cmake $(KERNEL_DIR)/$(1)$(2)$(3)$(4)/KernelDefinitions.cmake $(APP_DIR)/config/.config ${MAKEFILE_DEPEND}
	-$(RMDIR) $(RMDIRARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$@))
	-$(MKDIR) $(MKDIRARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$@))
	@echo ----------------------------------------
	@echo ----- build old userspace framework -----
	@echo ----------------------------------------
	${MAKE} -C ${APP_DIR} -f ${APP_DIR}/config/buildCusLib.mak SCMODULE=$(1)$(2)$(3)$(4) TOOL_DIR=${TOOL_DIR} ROOT_DIR=${APP_DIR} TOP_DIR=${APP_DIR} APP_DIR=${APP_DIR} KERNEL_DIR=${KERNEL_DIR} -j1
ifneq (,$(SIMCOM_RELEASE_SDK))
	$$(CMAKE) -B$$(dir $$@) -DSCMODULE="$(1)" $(SIMCOM_SDK_MACRO) \
		-DKERNEL_DIR="$(KERNEL_DIR)" \
		-DAPP_DIR=${APP_DIR} \
		$(patsubst %,-D%=ON,$(subst _, ,$(1))) \
		-D$(1)=ON \
		-DGCC_ARM_TOOLCHAIN_DIR="$(GCC_ARM_TOOLCHAIN_DIR)" \
		-DCMAKE_TOOLCHAIN_FILE="$(APP_DIR)/config/ToolChain.cmake" \
		-DAPP_NAME="$(APP_NAME)" \
		$$(shell $(subst $(BAD_SLASH),$(GOOD_SLASH),$(BUILDCONFIG)) $(APP_DIR)/config/.config) \
		-G $(BUILD_TYPE) \
		$(APP_DIR)
else
ifeq (win32,${PLATFORM})
	$$(word 1,$$(subst :, ,$$@)): && cd $$(dir $$@) && $$(CMAKE) \
		-DSCMODULE="$(1)$(2)$(3)$(4)" $(SIMCOM_SDK_MACRO) \
		-DKERNEL_DIR="$(KERNEL_DIR)" \
		-DTOOL_DIR="${TOOL_DIR}" \
		-DROOT_DIR="${ROOT_DIR}" \
		-DAPP_DIR=${APP_DIR} \
		$(patsubst %,-D%=ON,$(subst _, ,$(1)$(2)$(3)$(4))) \
		-D$(1)$(2)$(3)$(4)=ON \
		-DGCC_ARM_TOOLCHAIN_DIR="$(GCC_ARM_TOOLCHAIN_DIR)" \
		-DCMAKE_TOOLCHAIN_FILE="$(APP_DIR)/config/ToolChain.cmake" \
		-DAPP_NAME="$(APP_NAME)" \
		$$(shell $(subst $(BAD_SLASH),$(GOOD_SLASH),$(BUILDCONFIG)) $(APP_DIR)/config/.config) \
		-G $(BUILD_TYPE) \
		$(APP_DIR)
else
	cd $$(dir $$@) && $$(CMAKE) \
		-DSCMODULE="$(1)$(2)$(3)$(4)" $(SIMCOM_SDK_MACRO) \
		-DKERNEL_DIR="$(KERNEL_DIR)" \
		-DTOOL_DIR="${TOOL_DIR}" \
		-DROOT_DIR="${ROOT_DIR}" \
		-DAPP_DIR=${APP_DIR} \
		$(patsubst %,-D%=ON,$(subst _, ,$(1)$(2)$(3)$(4))) \
		-D$(1)$(2)$(3)$(4)=ON \
		-DGCC_ARM_TOOLCHAIN_DIR="$(GCC_ARM_TOOLCHAIN_DIR)" \
		-DCMAKE_TOOLCHAIN_FILE="$(APP_DIR)/config/ToolChain.cmake" \
		-DAPP_NAME="$(APP_NAME)" \
		$$(shell $(subst $(BAD_SLASH),$(GOOD_SLASH),$(BUILDCONFIG)) $(APP_DIR)/config/.config) \
		-G $(BUILD_TYPE) \
		$(APP_DIR)
endif
endif


clean_$(1)$(2)$(3)$(4):clean_$(1)$(2)$(3)$(4)_app
clean_$(1)$(2)$(3)$(4)_app:
	-$(RMDIR) $(RMDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4))

endif  ## ifneq (,$(findstring _OPENSDK_,_$(3)_)$(SIMCOM_RELEASE_SDK))

ifneq (,$(findstring _OPENSDK_,_$(3)_))  ## openSDK source build flow
ifneq (,$(SIMCOM_RELEASE_SDK))
else

## create macros dependence file from cp buildlog which used by userspace
$(KERNEL_DIR)/$(1)$(2)$(3)$(4)/KernelDefinitions.cmake:$(KERNEL_DIR)/$(1)$(2)$(3)$(4)/cp.bin
	-$(RM) $(RMARG) $$(subst ${BAD_SLASH},${GOOD_SLASH},$$@)
	python ${TOOL_DIR}/script/extract_macro_from_buildlog.py $(ROOT_DIR)/tavor/Arbel/build/buildlog.txt yes $$(patsubst %.cmake,%,$$@)

$(KERNEL_DIR)/$(1)$(2)$(3)$(4)/${KERNEL_LINK_DEFINE}:$(KERNEL_DIR)/$(1)$(2)$(3)$(4)/cp.bin
	echo KERNEL_LINK_FILE := $$(word 1,$$(subst :, ,$$@)):$$(shell python $${TOOL_DIR}/script/getInfo.py GET_KERNEL_LINKSCRIPT_FILE $$(ROOT_DIR)/tavor/Arbel/build/buildlog.txt)> $$@

## copy the necessary cp target file form cp build folder to folder userspace needed
$(KERNEL_DIR)/$(1)$(2)$(3)$(4)/cp.bin: tavor/Arbel/bin/$(1)$(2)$(3)$(4)_cp.bin
	-${RMDIR} ${RMDIRARG} $$(dir $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$@))
	-$(MKDIR) $(MKDIRARG) $$(dir $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$@))
	$(COPY_FILE) $(COPYARG_FILE) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(patsubst %/,%,$$(dir $$<))/*.bin) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$@))
	$(COPY_FILE) $(COPYARG_FILE) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(patsubst %/,%,$$(dir $$<))/*.axf) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$@))
	$(COPY_FILE) $(COPYARG_FILE) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(patsubst %/,%,$$(dir $$<))/*.map) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$@))
	$(COPY_FILE) $(COPYARG_FILE) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(patsubst %/,%,$$(dir $$<))/*.txt) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$@))
	-${RM} ${RMARG} $$(subst ${BAD_SLASH},${GOOD_SLASH},$$(dir $$@)$$(notdir $$<))

ifneq (,$(findstring _QL_, _$(4)_))
./tavor/Arbel/bin/$(1)$(2)$(3)$(4)_cp.bin:$(ROOT_DIR)/simcom/open_api_ql/$(1)$(2)$(3)$(4)/src/$(API_MAP_FILE_NAME).c $(ROOT_DIR)/simcom/open_api_ql/$(1)$(2)$(3)$(4)/inc/$(API_MAP_FILE_NAME).h
$(ROOT_DIR)/simcom/open_api_ql/$(1)$(2)$(3)$(4)/inc/$(API_MAP_FILE_NAME).h: $(ROOT_DIR)/simcom/open_api_ql/$(1)$(2)$(3)$(4)/src/$(API_MAP_FILE_NAME).c
else
./tavor/Arbel/bin/$(1)$(2)$(3)$(4)_cp.bin:$(ROOT_DIR)/simcom/open_api/$(1)$(2)$(3)$(4)/src/$(API_MAP_FILE_NAME).c $(ROOT_DIR)/simcom/open_api/$(1)$(2)$(3)$(4)/inc/$(API_MAP_FILE_NAME).h
$(ROOT_DIR)/simcom/open_api/$(1)$(2)$(3)$(4)/inc/$(API_MAP_FILE_NAME).h: $(ROOT_DIR)/simcom/open_api/$(1)$(2)$(3)$(4)/src/$(API_MAP_FILE_NAME).c
endif

ifneq (,$(findstring _QL_, _$(4)_))
$(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(APP_NAME).elf:$(APP_DIR)/ql_lib/$(1)$(2)$(3)$(4)/src/$(API_MAP_FILE_NAME).c $(APP_DIR)/ql_lib/$(1)$(2)$(3)$(4)/inc/$(API_MAP_FILE_NAME).h
$(APP_DIR)/ql_lib/$(1)$(2)$(3)$(4)/src/$(API_MAP_FILE_NAME).c:$(ROOT_DIR)/simcom/open_api_ql/$(1)$(2)$(3)$(4)/src/$(API_MAP_FILE_NAME).c
$(APP_DIR)/ql_lib/$(1)$(2)$(3)$(4)/inc/$(API_MAP_FILE_NAME).h:$(ROOT_DIR)/simcom/open_api_ql/$(1)$(2)$(3)$(4)/src/$(API_MAP_FILE_NAME).c
else
$(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(APP_NAME).elf:$(APP_DIR)/sc_lib/$(1)$(2)$(3)$(4)/src/$(API_MAP_FILE_NAME).c $(APP_DIR)/sc_lib/$(1)$(2)$(3)$(4)/inc/$(API_MAP_FILE_NAME).h
$(APP_DIR)/sc_lib/$(1)$(2)$(3)$(4)/src/$(API_MAP_FILE_NAME).c:$(ROOT_DIR)/simcom/open_api/$(1)$(2)$(3)$(4)/src/$(API_MAP_FILE_NAME).c
$(APP_DIR)/sc_lib/$(1)$(2)$(3)$(4)/inc/$(API_MAP_FILE_NAME).h:$(ROOT_DIR)/simcom/open_api/$(1)$(2)$(3)$(4)/src/$(API_MAP_FILE_NAME).c
endif

## create openSDK api map
ifneq (,$(findstring _QL_, _$(4)_))
$(ROOT_DIR)/simcom/open_api_ql/$(1)$(2)$(3)$(4)/src/$(API_MAP_FILE_NAME).c:$(API_MAP_MAKEFILE) $(API_MAP_INIT_SCRIPT)
else
$(ROOT_DIR)/simcom/open_api/$(1)$(2)$(3)$(4)/src/$(API_MAP_FILE_NAME).c:$(API_MAP_MAKEFILE) $(API_MAP_INIT_SCRIPT)
endif
	@echo ------------------------------------------
	@echo     create [$(1)$(2)$(3)$(4)] api map
	@echo ------------------------------------------
	-$(RMDIR) $(RMDIRARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$(dir $$@)))
	$(MKDIR) $(MKDIRARG) $$(subst $(BAD_SLASH),$(GOOD_SLASH),$$(dir $$(dir $$@)))
	$(MAKE) -C $$(dir $$<) -f $$(notdir $$<) API_MAP_FILE_NAME=$(API_MAP_FILE_NAME) ROOT_DIR=$(ROOT_DIR) TOOL_DIR=$(TOOL_DIR) APP_DIR=${APP_DIR} SC_MODULE=$(1) SC_HD_CFG=$(patsubst _%,%,$(2)) SC_HD_OPT=$(patsubst _%,%,$(3)) SC_USR_OPT=$(patsubst _%,%,$(4)) SC_MODULE_FULL=$(1)$(2)$(3)$(4)


################################
## kernel clean option define
################################
clean_$(1)$(2)$(3)$(4):clean_$(1)$(2)$(3)$(4)_kernel clean_$(1)$(2)$(3)$(4)_file
clean_$(1)$(2)$(3)$(4)_kernel:clean_$(1)$(2)$(3)$(4)_file
clean_file:clean_$(1)$(2)$(3)$(4)_file
clean_$(1)$(2)$(3)$(4)_file:$(API_MAP_MAKEFILE)
	-$(RMDIR) $(RMDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(KERNEL_DIR)/$(1)$(2)$(3)$(4))
	-${RM} ${RMARG} $(subst ${BAD_SLASH},${GOOD_SLASH},$(APP_DIR)/config/$(1)$(2)$(3)$(4)_linkscript.ld)
	$(MAKE) -C $$(dir $$<) -f $$(notdir $$<) clean API_MAP_FILE_NAME=$(API_MAP_FILE_NAME) ROOT_DIR=$(ROOT_DIR) TOOL_DIR=$(TOOL_DIR) APP_DIR=${APP_DIR} SC_MODULE=$(1) SC_HD_CFG=$(2) SC_HD_OPT=$(3) SC_USR_OPT=$(4) SC_MODULE_FULL=$(1)$(2)$(3)$(4)

install_$(1)$(2)$(3)$(4):$(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/$(APP_NAME).bin $(KERNEL_DIR)/$(1)$(2)$(3)$(4)/cp.bin
	$(BUILD) -C $(APP_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/ install
	@echo ------------------------------------------
	@echo     install [$(1)$(2)$(3)$(4)] success!!
	@echo ------------------------------------------

uninstall_$(1)$(2)$(3)$(4):
	-$(RMDIR) $(RMDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(ROOT_DIR)/$(RELEASE)/$(1)$(2)$(3)$(4))
## kernel clean option define end

endif  ## ifneq (,$(SIMCOM_RELEASE_SDK))

else  ## ifneq (,$(findstring _OPENSDK_,_$(3)_))
## AT version build flow

clean_$(1)$(2)$(3)$(4)_kernel:
	cd $(ROOT_DIR)/tavor/Arbel/bin && $(RM) $(RMARG) *

$(1)$(2)$(3)$(4)_kernel:
	@$(MAKE) clean_$(1)$(2)$(3)$(4)_kernel
	@$(MAKE) $(1)$(2)$(3)$(4)

$(1)$(2)$(3)$(4)_force:
	@$(MAKE) $(1)$(2)$(3)$(4)_kernel

$(1)$(2)$(3)$(4):$(ROOT_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/burn.zip

ifeq (,$(SIMCOM_RELEASE_SDK))
$(ROOT_DIR)/$(OUT)/$(1)$(2)$(3)$(4)/burn.zip:tavor/Arbel/bin/$(1)$(2)$(3)$(4)_cp.bin
endif

endif  ## ifneq (,$(findstring _OPENSDK_,_$(3)_))

## kernel/cp_lzma build for ALL version
tavor/Arbel/bin/$(1)$(2)$(3)$(4)_cp.bin:
	@echo ---------------------------
	@echo -----  build kernel   -----
	@echo ---------------------------
## Backup RD to generate work path
ifeq ($(wildcard $(PREBUILD_DIR)/RD/$(1)$(2)$(3)$(4)),)
ifeq (path.txt,$(notdir $(wildcard $(PREBUILD_DIR)/RD/$(1)$(2)$(3)/path.txt)))
	${COPY_FILE} ${COPYARG_FILE} $$(subst ${BAD_SLASH},${GOOD_SLASH},${PREBUILD_DIR}/$$(shell ${CAT} $$(subst ${BAD_SLASH},${GOOD_SLASH},${PREBUILD_DIR}/RD/$(1)$(2)$(3)/path.txt))/*) $$(subst ${BAD_SLASH},${GOOD_SLASH},$(ROOT_DIR)/tavor/Arbel/build/ReliableData/)
else
	${COPY_FILE} ${COPYARG_FILE} $$(subst ${BAD_SLASH},${GOOD_SLASH},${PREBUILD_DIR}/RD/$(1)$(2)$(3)/*) $$(subst ${BAD_SLASH},${GOOD_SLASH},$(ROOT_DIR)/tavor/Arbel/build/ReliableData/)
endif
else
ifeq (path.txt,$(notdir $(wildcard $(PREBUILD_DIR)/RD/$(1)$(2)$(3)$(4)/path.txt)))
	${COPY_FILE} ${COPYARG_FILE} $$(subst ${BAD_SLASH},${GOOD_SLASH},${PREBUILD_DIR}/$$(shell ${CAT} $$(subst ${BAD_SLASH},${GOOD_SLASH},${PREBUILD_DIR}/RD/$(1)$(2)$(3)$(4)/path.txt))/*) $$(subst ${BAD_SLASH},${GOOD_SLASH},$(ROOT_DIR)/tavor/Arbel/build/ReliableData/)
else
	${COPY_FILE} ${COPYARG_FILE} $$(subst ${BAD_SLASH},${GOOD_SLASH},${PREBUILD_DIR}/RD/$(1)$(2)$(3)$(4)/*) $$(subst ${BAD_SLASH},${GOOD_SLASH},$(ROOT_DIR)/tavor/Arbel/build/ReliableData/)
endif
endif

ifeq (,${HOLD})
ifeq (,$(wildcard $(ROOT_DIR)/tavor/Arbel/build/sc_kernel_build_$(1)$(2)$(3)$(4)))
	$(MAKE) cleanAll
endif
	-${RM} ${RMARG} $(subst ${BAD_SLASH},${GOOD_SLASH},$(ROOT_DIR)/tavor/Arbel/build/sc_kernel_build_*)
	cd. > $(subst ${BAD_SLASH},${GOOD_SLASH},$(ROOT_DIR)/tavor/Arbel/build/sc_kernel_build_$(1)$(2)$(3)$(4))
ifneq (,$(findstring _1606_,_$(3)_))
ifeq (TRUE,$(patsubst %Y,TRUE,$(2)))
	cd tavor/Arbel/build && module_lteonly_buildcust_4mram_2mflash_craneL_single_sim.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4) ${J}
else ifeq (TRUE,$(patsubst %S,TRUE,$(2)))
ifneq (,$(findstring _DS_,_$(3)_))
	cd tavor/Arbel/build && module_lteonly_buildcust_8mram_craneL.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4) ${J}
else
	cd tavor/Arbel/build && module_lteonly_buildcust_8mram_craneL_single_sim.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4) ${J}
endif
else ifeq (TRUE,$(patsubst %V,TRUE,$(2)))
ifneq (,$(findstring _DS_,_$(3)_))
	cd tavor/Arbel/build && module_lteonly_buildcust_4mram_craneL.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4) ${J}
else
ifneq (,$(findstring _TTS_,_$(4)_))
ifneq (,$(findstring _CSSMS_,_$(4)_))
	cd tavor/Arbel/build && module_lteonly_buildcust_4mram_craneL_single_sim_sms.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4) ${J}
else
	cd tavor/Arbel/build && module_lteonly_buildcust_4mram_craneL_single_sim.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4) ${J}
endif
else ifneq (,$(findstring _VOLTE_,_$(4)_))
	cd tavor/Arbel/build && module_lteonly_buildcust_4mram_craneL_single_sim_volte.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4) ${J}
else
	cd tavor/Arbel/build && module_lteonly_buildcust_4mram_craneL_single_sim_sms.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4) ${J}
endif
endif
endif
else ifneq (,$(findstring _1601_,_$(3)_))
	cd tavor/Arbel/build && module_ltegsm_buildcust.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4) ${J}
else ifneq (,$(findstring _1602_,_$(3)_))
ifeq (TRUE,$(patsubst %Y,TRUE,$(2)))
ifeq (,$(subst A7680C,,${1})$(patsubst _M%,,${2}))
## ASR5311
	cd tavor/Arbel/build && module_lteonly_buildcust_4mram_2mflash_craneLR_single_sim_agps.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4) ${J}
else
## NO_ASR5311
ifneq (,$(findstring _CSSMS_,_$(4)_))
## CSSMS
	cd tavor/Arbel/build && module_lteonly_buildcust_4mram_2mflash_craneLR_single_sim_cssms.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4) ${J}
else
	cd tavor/Arbel/build && module_lteonly_buildcust_4mram_2mflash_craneLR_single_sim.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4) ${J}
endif
endif
else ifeq (TRUE,$(patsubst %V,TRUE,$(2)))
ifneq (,$(findstring _TTS_,_$(4)_))
ifneq (,$(findstring _CSSMS_,_$(4)_))
	cd tavor/Arbel/build && module_lteonly_buildcust_4mram_4mflash_craneLR_single_sim_sms.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4) ${J}
else
	cd tavor/Arbel/build && module_lteonly_buildcust_4mram_4mflash_craneLR_single_sim.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4) ${J}
endif
else
ifneq (,$(findstring _UCVSIM_LINGDU_,$(4)_))
	cd tavor/Arbel/build && module_lteonly_buildcust_4mram_4mflash_craneLR_single_sim.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4) ${J}
else
	cd tavor/Arbel/build && module_lteonly_buildcust_4mram_4mflash_craneLR_single_sim_sms.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4) ${J}
endif
endif
endif
else ifneq (,$(findstring _1603_,_$(3)_))
ifneq (,$(findstring _LTEONLY_,_$(3)_))
	cd tavor/Arbel/build && module_lteonly_buildcust.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4) ${J}
else
ifneq (,$(findstring _CSD_,_$(4)_))
	cd tavor/Arbel/build && module_ltegsm_csd_buildcust.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4) ${J}
else
	cd tavor/Arbel/build && module_ltegsm_buildcust.bat _$(1) _$(patsubst _%,%,$(2)) _$(patsubst _%,%,$(3)) _$(patsubst _%,%,$(4)) _$(1)$(2)$(3)$(4) ${J}
endif
endif
endif
endif
	-${RM} ${RMARG} $$(subst ${BAD_SLASH},${GOOD_SLASH},$$@)
	${COPY_FILE} ${COPYARG_FILE} $$(subst ${BAD_SLASH},${GOOD_SLASH},$$(dir $$@)cp.bin) $$(subst ${BAD_SLASH},${GOOD_SLASH},$$@)

$(1)$(2)$(3)$(4)_releaseSource: tavor/Arbel/bin/$(1)$(2)$(3)$(4)_cp.bin ${MSYS2_DIR} ${APP_DIR} ${CMAKE_DIR} ${CROSS_TOOL_DIR}
	@echo ------------------------------------------
	@echo     release [$(1)$(2)$(3)$(4)] source begin ....
	@echo ------------------------------------------
	-$(MKDIR) $(MKDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),${ROOT_DIR}/${RELEASE}/buildsource)
	$(CMAKE) -B${ROOT_DIR}/${RELEASE}/buildsource \
		-DSCMODULE="$(1)$(2)$(3)$(4)_releaseSource" \
		-DTOOL_DIR="${TOOL_DIR}" \
		-DROOT_DIR="${ROOT_DIR}" \
		-DGCC_ARM_TOOLCHAIN_DIR="$(GCC_ARM_TOOLCHAIN_DIR)" \
		-DCMAKE_TOOLCHAIN_FILE="$(APP_DIR)/config/ToolChain.cmake" \
		-G $(BUILD_TYPE)
	$(BUILD) -C ${ROOT_DIR}/${RELEASE}/buildsource install
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},${ROOT_DIR}/tavor/Arbel/obj_PMD2NONE/simcom-simcom_components.lib ${ROOT_DIR}/${RELEASE}/$(1)$(2)$(3)$(4)_releaseSource/tavor/Arbel/CRANE_SDK_LIB/DM_GENERIC_LTEGSM)
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},${ROOT_DIR}/tavor/Arbel/obj_PMD2NONE/simcom-simcom_components.lib ${ROOT_DIR}/${RELEASE}/$(1)$(2)$(3)$(4)_releaseSource/tavor/Arbel/CRANE_SDK_LIB/DM_GENERIC_LTEONLY)
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},${ROOT_DIR}/tavor/Arbel/obj_PMD2NONE/simcom-simcom_components.lib ${ROOT_DIR}/${RELEASE}/$(1)$(2)$(3)$(4)_releaseSource/tavor/Arbel/CRANE_SDK_LIB/DM_THIN_LTEONLY)
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},${ROOT_DIR}/tavor/Arbel/obj_PMD2NONE/simcom-simcom_components.lib ${ROOT_DIR}/${RELEASE}/$(1)$(2)$(3)$(4)_releaseSource/tavor/Arbel/CRANE_SDK_LIB/DM_THIN_SINGLE_SIM_LTEONLY)
	${COPY_FILE} ${COPYARG_FILE} $(subst ${BAD_SLASH},${GOOD_SLASH},${ROOT_DIR}/tavor/Arbel/obj_PMD2NONE/simcom-simcom_components.lib ${ROOT_DIR}/${RELEASE}/$(1)$(2)$(3)$(4)_releaseSource/tavor/Arbel/CRANE_SDK_LIB/DM_THIN_SINGLE_SIM_LTEONLY)
	${SED} -i "/CRANE_SDK_USELIBS/asimcom\\\\\\simcom_components ^" ${ROOT_DIR}/${RELEASE}/$(1)$(2)$(3)$(4)_releaseSource/tavor/Arbel/build/SetSDKLib.bat
	@echo ------------------------------------------
	@echo     install [$(1)$(2)$(3)$(4)] source success!!
	@echo ------------------------------------------

endef  ## define user_option_process
# target create end



define user_custom_pre_process
$(eval $(call user_option_process,$(1),$(2),$(3),))

$(foreach usr_opt,$($(1)$(2)$(3)_USR_OPT_LIST),$(eval $(call user_option_process,$(1),$(2),$(3),_$(usr_opt))))

$(eval $(call get_usr_opt_list,$(1)$(2)$(3)))
endef

define hard_option_process
ifeq (,$($(1)$(2)_HD_OPT_LIST))
$$(eval $$(call user_custom_pre_process,$(1),$(2),,))
else
$$(foreach hd_opt,$($(1)$(2)_HD_OPT_LIST),$$(eval $$(call user_custom_pre_process,$(1),$(2),$$(subst _STANDARD,,_$$(hd_opt)),)))
endif
endef

define hard_config_process
ifeq (,$($(1)$(2)_HD_CFG_LIST))
$$(eval $$(call hard_option_process,$(1),))
else
$$(foreach hd_cfg,$$($(1)_HD_CFG_LIST),$$(eval $$(call hard_option_process,$(1),$$(subst _STANDARD,,_$$(hd_cfg)))))
endif
endef

ifneq (,$(SIMCOM_RELEASE_SDK))
$(foreach modules,$(SCMODULE),$(eval $(call user_option_process,$(modules),,,)))
else
################################
# info create
################################
define get_usr_opt_list
ALL_USR_OPT_LIST += $($(1)_USR_OPT_LIST)
endef

define make_usr_opt_info
$(1)_info:
	@echo -      $(1)
endef
# info create end

$(foreach modules,$(SCMODULE),$(eval $(call hard_config_process,$(modules))))

$(foreach usr_opt,$(sort $(ALL_USR_OPT_LIST)),$(eval $(call make_usr_opt_info,$(usr_opt))))
endif  ## ifneq (,$(SIMCOM_RELEASE_SDK))






################################################
##   release simcom SDK for multiple modules
################################################

ifneq (,$(SIMCOM_RELEASE_SDK))
else
KERNEL_TARGETLIST := $(sort $(notdir $(wildcard $(KERNEL_DIR)/*)))
APP_TARGETLIST := $(sort $(notdir $(wildcard $(APP_DIR)/$(OUT)/*)))
VALID_TARGETLIST := $(filter $(APP_TARGETLIST),$(KERNEL_TARGETLIST))

.PHONE: install_set uninstall_set

install_set:
	@echo ------------------------------------------
	@echo     install [${RELEASE_SET}] begin ....
	@echo ------------------------------------------
	@echo ------------------------------------------
	@echo KERNEL_TARGETLIST: $(KERNEL_TARGETLIST)
	@echo APP_TARGETLIST: $(APP_TARGETLIST)
	@echo target set list: $(VALID_TARGETLIST)
	@echo ------------------------------------------
	-$(MKDIR) $(MKDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),${ROOT_DIR}/${RELEASE}/build)
	$(CMAKE) -B${ROOT_DIR}/${RELEASE}/build \
		-DSCMODULE="${RELEASE_SET}" $(SIMCOM_SDK_MACRO) \
		-DKERNEL_DIR="$(KERNEL_DIR)" \
		-DTOOL_DIR="${TOOL_DIR}" \
		-DROOT_DIR="${ROOT_DIR}" \
		$(patsubst %,-D%=ON,$(subst _, ,${VALID_TARGETLIST})) \
		$(patsubst %,-D%=ON,${VALID_TARGETLIST}) \
		-DMODULE_LIST="${VALID_TARGETLIST}" \
		-DGCC_ARM_TOOLCHAIN_DIR="$(GCC_ARM_TOOLCHAIN_DIR)" \
		-DCMAKE_TOOLCHAIN_FILE="$(APP_DIR)/config/ToolChain.cmake" \
		-DAPP_NAME="$(APP_NAME)" \
		$(shell $(subst $(BAD_SLASH),$(GOOD_SLASH),$(BUILDCONFIG)) $(APP_DIR)/config/.config) \
		-G $(BUILD_TYPE) \
		$(APP_DIR)
	$(BUILD) -C ${ROOT_DIR}/${RELEASE}/build install
	@echo ------------------------------------------
	@echo     install [${RELEASE_SET}] success!!
	@echo ------------------------------------------

uninstall_set:
	-$(RMDIR) $(RMDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(ROOT_DIR)/$(RELEASE)/$(RELEASE_SET))
	-$(RMDIR) $(RMDIRARG) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(ROOT_DIR)/$(RELEASE)/build)

endif
