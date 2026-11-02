include ${TOP_DIR}/script/env.mk


PACKAGE_DIRS:=$(subst \,/,$(PACKAGE_DIRS))

PACKAGE_LIBS_DIR:=$(addprefix $(BUILD_PATH)/packag-,$(notdir $(PACKAGE_DIRS)))
PACKAGE_LIBS_DIR:=$(addsuffix /$(obj),$(PACKAGE_LIBS_DIR))
PACKAGE_LIBS:=$(notdir $(PACKAGE_DIRS))
PACKAGE_LIBS:=$(addsuffix .lib,$(PACKAGE_LIBS))
PACKAGE_LIBS:=$(join $(PACKAGE_LIBS_DIR)/,$(PACKAGE_LIBS))

PACKAGE_CLEAN_FLOW:=$(patsubst %,%/package_clean,$(PACKAGE_DIRS))
PACKAGE_BUILD_FLOW:=$(patsubst %,%/package_build,$(PACKAGE_DIRS))


#-------------------------------------------------------------------------------
# standard reference library
#-------------------------------------------------------------------------------
STDLIB:=\
	$(GCC_PATH)\arm-none-eabi\lib\thumb\v7-ar\libm.a \
	$(GCC_PATH)\arm-none-eabi\lib\thumb\v7-ar\libc.a \
	$(GCC_PATH)\arm-none-eabi\lib\thumb\v7-ar\libnosys.a \
	$(GCC_PATH)\lib\gcc\arm-none-eabi\7.3.1\thumb\v7-ar\libgcc.a \

#-------------------------------------------------------------------------------
# build flags setting
#-------------------------------------------------------------------------------

#default ldfile
ldfile=app_linkscript.ld
#ldfile=app_linkscript_iFLY.ld

ifneq (,$(findstring MMI,${MODULE}))
	ifeq (LTEONLY,$(option))
		ldfile=app_linkscript_mmi_lteonly.ld
	else
		ifneq (,$(findstring MMI_SZHYT,$(MODULE)))
			ldfile=app_linkscript_mmi_SZHYT.ld
		else
			ldfile=app_linkscript_mmi.ld
		endif
	endif
endif

ifeq (A7670C_LASL_SYD_16M,$(MODULE))
ldfile=app_linkscript_SYD.ld
endif

ifeq (A7670C_FASL_MMI_NOTTS_16M,$(MODULE))
	ifeq (LTEONLY,$(option))
		ldfile=app_linkscript_mmi_notts_lteonly.ld
	else
		ldfile=app_linkscript_mmi_notts.ld
	endif
endif

ifeq (A7670C_FASL_MMI_NOTTS_YUNYI_16M,$(MODULE))
	ldfile=app_linkscript_mmi_notts_lteonly_yunyi.ld
endif

ifeq (A7672E_FASE_NAV_16M,$(MODULE))
ldfile=app_linkscript_NAV.ld
endif

ifeq (A7672SA_FASE_NAV_16M,$(MODULE))
ldfile=app_linkscript_NAV.ld
endif

ifeq (A7672E_FASE_TWS_16M,$(MODULE))
ldfile=app_linkscript_TWS.ld
endif

ifeq (A7672S_LASC_NIAGARA_8M,$(MODULE))
ldfile=app_linkscript_NIAGARA.ld
endif

ifeq (A7630C_LAAL_ST_16M,$(MODULE))
ldfile=app_linkscript_ST.ld
endif

ifeq (A7670C_LASL_CZJ_16M,$(MODULE))
ldfile=app_linkscript_CZJ.ld
endif

ifeq (A7670E_LASE_CZJ_16M,$(MODULE))
ldfile=app_linkscript_CZJ.ld
endif
ifeq (A7670C_LASE_CZJ_16M,$(MODULE))
ldfile=app_linkscript_CZJ.ld
endif
ifeq (A7670SA_LASE_CZJ_16M,$(MODULE))
ldfile=app_linkscript_CZJ.ld
endif

ifneq (,$(findstring _LSOFTSIM_,_$(MODULE)_))
ldfile=app_linkscript_LSOFTSIM.ld
endif

ifeq (A7630C_LAAS_PS_ST_8M,$(MODULE))
ldfile=app_linkscript_ST_8M.ld
endif
ifeq (A7670C_MASS_HQ_8M,$(MODULE))
ldfile=app_linkscript_HQ_8M_LTEONLY.ld
endif

ifeq (A7670SA_LASC_CD_MSJ_8M,$(MODULE))
ldfile=app_linkscript_MSJ.ld
endif

ifeq (A7670E_LASC_CD_MSJ_8M,$(MODULE))
ldfile=app_linkscript_MSJ.ld
endif

ifeq (A7670E_FASE_NWIHK_16M,$(MODULE))
ldfile=app_linkscript_NWIHK.ld
endif

ifneq (,$(findstring _BASC_WWKJ_,_$(MODULE)_)$(findstring _LASC_WWKJ_,_$(MODULE)_))
ldfile=app_linkscript_WWKJ.ld
endif

ifneq (,$(findstring A7670SA_MASC_CD_LHAY,${MODULE}))
ldfile=app_linkscript_LHAY.ld
endif

LDFLAGS:= -gc-sections -Rbuild -T$(TOP_DIR)\script\$(ldfile) -nodefaultlibs
OBJCOPYFLAGS:=


.PHONY: all clean makeDir

IMAGE:=customer_app
IMAGE_TEMP:=customer_app_temp

all:makeDir $(IMAGE)

makeDir:
	@if not exist $(BIN_PATH) (md $(subst /,\,$(BIN_PATH)))
	@if exist $(subst /,\,$(BIN_PATH))/$(IMAGE).bin (del /s/q $(subst /,\,$(BIN_PATH)/$(IMAGE).bin))
	@if exist $(subst /,\,$(BIN_PATH))/$(IMAGE).elf (del /s/q $(subst /,\,$(BIN_PATH)/$(IMAGE).elf))
	@if exist $(subst /,\,$(BIN_PATH))/$(IMAGE).map (del /s/q $(subst /,\,$(BIN_PATH)/$(IMAGE).map))

clean:$(PACKAGE_CLEAN_FLOW)
	@if exist $(subst /,\,$(BIN_PATH))/$(IMAGE).bin (del /s/q $(subst /,\,$(BIN_PATH)/$(IMAGE).bin))
	@if exist $(subst /,\,$(BIN_PATH))/$(IMAGE).elf (del /s/q $(subst /,\,$(BIN_PATH)/$(IMAGE).elf))
	@if exist $(subst /,\,$(BIN_PATH))/$(IMAGE).map (del /s/q $(subst /,\,$(BIN_PATH)/$(IMAGE).map))
	@if exist $(subst /,\,$(BUILD_PATH)) (@echo clean $(subst /,\,$(BUILD_PATH)) & rd /s/q $(subst /,\,$(BUILD_PATH)))

%/package_clean:%
	@echo clean [$<]
	gnumake -C $< -f Makefile clean TOP_DIR=$(TOP_DIR)

$(IMAGE):$(BIN_PATH)/$(IMAGE).elf
	@echo --------------objcopy [$(BIN_PATH)/$(IMAGE).bin]-------------
	$(OBJCOPY) $(OBJCOPYFLAGS) -O binary --gap-fill=0x0 $<  $(BIN_PATH)/$(IMAGE_TEMP).bin
	@echo --------------crc_set [$(BIN_PATH)/$(IMAGE).bin]-------------
	$(CRC_SET) $(BIN_PATH)/$(IMAGE_TEMP).bin $(BIN_PATH)/$(IMAGE).bin
	@echo ----------------------------------------------------
	@echo - Compiling Finished Sucessfully.
	@echo - The target image is in the '$(BIN_PATH)' directory.
	@echo ----------------------------------------------------

$(BIN_PATH)/$(IMAGE).elf:$(PACKAGE_BUILD_FLOW)
	@echo --------------link [$@]----------------------
	$(LD) $(LDFLAGS) $(U_LDFLAGS) -Map $(BIN_PATH)/$(IMAGE).map -o $@ --whole-archive $(PACKAGE_LIBS) $(DEFAULT_LIBS) --no-whole-archive $(U_LIBS) $(STDLIB)

%/package_build:%
	@echo build [$<]
	gnumake -C $< -f Makefile TOP_DIR=$(TOP_DIR)



