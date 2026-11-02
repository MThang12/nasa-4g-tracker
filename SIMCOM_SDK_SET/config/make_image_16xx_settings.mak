#
# 16xx settings
#

FLASH_TARGET_PRE := CRANEL_A0

ifneq (,$(findstring _1606_,_${SC_MODULE_FULL}_))
ASR_PRODUCT_TYPE := ASR_CRANEL_EVB
#
# Special process
# In SDK_1.011.069, only have dual SIM product type, so ignore to check
# In SDK_1.011.078 and later version, ASR1606L only have use signal SIM product
#
ifneq (,$(findstring _DS_,$(SC_MODULE_FULL)_))
	PRODUCT_SIM_TYPE :=
else
	PRODUCT_SIM_TYPE := _SINGLE_SIM
endif
else ifneq (,$(findstring _1602_,_${SC_MODULE_FULL}_))
FLASH_TARGET_PRE := ASR1602
ASR_PRODUCT_TYPE := ASR1602_EVB
else ifneq (,$(findstring _1603_,_${SC_MODULE_FULL}_))
FLASH_TARGET_PRE := CRANEM_A0
ASR_PRODUCT_TYPE := ASR_CRANEM_EVB
else ifneq (,$(findstring _1601_,_${SC_MODULE_FULL}_))
FLASH_TARGET_PRE := ASR1601
ASR_PRODUCT_TYPE := ASR1601_EVB
endif

ifneq ($(word 2,$(subst _, ,$(SC_MODULE_FULL)))$(word 2,$(subst _, ,$(SC_MODULE_FULL))),$(patsubst %S,8M,$(word 2,$(subst _, ,$(SC_MODULE_FULL))))$(patsubst %C,8M,$(word 2,$(subst _, ,$(SC_MODULE_FULL)))))
	FLASH_TARGET_SIZE := _08MB
else ifeq (TRUE,$(patsubst %V,TRUE,$(word 2,$(subst _, ,$(SC_MODULE_FULL)))))
	FLASH_TARGET_SIZE := _04MB
else ifeq (TRUE,$(patsubst %Y,TRUE,$(word 2,$(subst _, ,$(SC_MODULE_FULL)))))
	FLASH_TARGET_SIZE := _02MB
else ifeq (TRUE,$(patsubst %E,TRUE,$(word 2,$(subst _, ,$(SC_MODULE_FULL)))))
	FLASH_TARGET_SIZE := _16MB
else ifeq (TRUE,$(patsubst %L,TRUE,$(word 2,$(subst _, ,$(SC_MODULE_FULL)))))
	FLASH_TARGET_SIZE := _16MB
else ifneq (,$(findstring A7682E_1603_V102,$(SC_MODULE_FULL))$(findstring A7662E_1603_V102,$(SC_MODULE_FULL)))
	# special product type
	FLASH_TARGET_SIZE := _16MB
else ifneq (,$(findstring A7682E_1603_R_V102,$(SC_MODULE_FULL)))
	# special product type
	FLASH_TARGET_SIZE := _08MB
else ifneq (,$(findstring A7672E_R_1603_V101,$(SC_MODULE_FULL)))
	# special product type
	FLASH_TARGET_SIZE := _08MB
endif

#PARSE FLASH_TARGET_HD START
#BT ADD
ifneq ($(word 2,$(subst _, ,$(SC_MODULE_FULL)))$(word 2,$(subst _, ,$(SC_MODULE_FULL))),$(patsubst F%,,$(word 2,$(subst _, ,$(SC_MODULE_FULL))))$(patsubst B%,,$(word 2,$(subst _, ,$(SC_MODULE_FULL)))))
	FLASH_TARGET_HD := _BT
endif

#NVM ADD
#ASR5311 ADD
ifneq ($(word 2,$(subst _, ,$(SC_MODULE_FULL)))$(word 2,$(subst _, ,$(SC_MODULE_FULL))),$(patsubst F%,,$(word 2,$(subst _, ,$(SC_MODULE_FULL))))$(patsubst M%,,$(word 2,$(subst _, ,$(SC_MODULE_FULL)))))
	ifeq (,$(findstring _A7672E_FASE_1603_V201_,_$(SC_MODULE_FULL)_)$(findstring _A7670U_MAMS_1606_V201_,_$(SC_MODULE_FULL)_)$(findstring _A7670NA_MAMS_1606_V201_,_$(SC_MODULE_FULL)_)$(findstring _A7670G_MAMS_1606_V201_,_$(SC_MODULE_FULL)_)$(findstring _A7670E_MAMS_1606_V201_,_$(SC_MODULE_FULL)_)$(findstring _A7670SA_MAMS_1606_V201_,_$(SC_MODULE_FULL)_)$(findstring _A7670G_MNMY_1602_UB_V101_,_$(SC_MODULE_FULL)_)$(findstring _A7672SA_FASE_1603_V201_,_$(SC_MODULE_FULL)_))
		ifneq (,$(findstring A7670,$(SC_MODULE_FULL))$(findstring A7672,$(SC_MODULE_FULL))$(findstring A7677,$(SC_MODULE_FULL))$(findstring A7605,$(SC_MODULE_FULL)))
			ifeq (_08MB,$(FLASH_TARGET_SIZE))
				FLASH_TARGET_HD := $(FLASH_TARGET_HD)_NVM
			endif
			ifeq (_16MB,$(FLASH_TARGET_SIZE))
				FLASH_TARGET_HD := $(FLASH_TARGET_HD)_NVM
			endif
		else
			FLASH_TARGET_HD := $(FLASH_TARGET_HD)_ASR5311
		endif
	else
		FLASH_TARGET_HD := $(FLASH_TARGET_HD)_ASR5311
	endif
endif
#PARSE FLASH_TARGET_HD END


# STANDARD FLASH_TARGET_CUSTOM START
ifneq (,$(findstring _VOLTE_,_$(SC_MODULE_FULL)_))
	ifeq (TRUE,$(patsubst %V,TRUE,$(word 2,$(subst _, ,$(SC_MODULE_FULL)))))
		FLASH_TARGET_CUSTOM := _VOLTE
	endif
else ifneq (,$(findstring _WHXA_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _WHXA
else ifneq (,$(findstring _GWSD_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _GWSD
else ifneq (,$(findstring _SHHD_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _SHHD
else ifneq (,$(findstring _JWZD_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _APP_512K
else ifneq (,$(findstring _YHWL_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _APP_512K
# else ifneq (,$(findstring _KDWL_,_$(SC_MODULE_FULL)_))
# 	FLASH_TARGET_CUSTOM := _APP_512K
else ifneq (,$(findstring _SHYJ_,_$(SC_MODULE_FULL)_))
	ifneq (,$(findstring _A7670C_MANS_1606_V702_,_$(SC_MODULE_FULL)_))
		FLASH_TARGET_CUSTOM := _APP_512K
	endif
else ifneq (,$(findstring _WHXBY_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _APP_512K
else ifneq (,$(findstring _A7680C_LANS_1606_XC_,_$(SC_MODULE_FULL)_)$(findstring _A7670C_LANS_1606_V701_XC_,_$(SC_MODULE_FULL)_)$(findstring _AJBDZ_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _APP_512K
else ifneq (,$(findstring _A7680C_LANV_1606_V503_XC_,_$(SC_MODULE_FULL)_)$(findstring _A7670C_LANV_1606_V701_XC_,_$(SC_MODULE_FULL)_)$(findstring _XMWE_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _APP_320K
else ifneq (,$(findstring _BL_POC_MMI_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _APP_512K
else ifneq (,$(findstring _ST_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _ST
else ifneq (,$(findstring _XYJ_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _XYJ
else ifneq (,$(findstring _ZS_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _ZS
else ifneq (,$(findstring _GZYM_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _GZYM
else ifneq (,$(findstring _QZZT_POC_MMI_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _QZZT
else ifneq (,$(findstring _HX_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _HX
else ifneq (,$(findstring _HQ_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _HQ
else ifneq (,$(findstring _LDJD_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _LDJD
else ifneq (,$(findstring _MT_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _MT
else ifneq (,$(findstring _YKKJ_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _YKKJ
else ifneq (,$(findstring _QZXC_POC_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _QZXC_POC
else ifneq (,$(findstring _PWY_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _PWY
else ifneq (,$(findstring _JXTZ_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _JXTZ
else ifneq (,$(findstring _A7670C_LANS_1606_V701_CZJ_POC_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _CZJ
else ifneq (,$(findstring _SJ_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _SJ
else ifneq (,$(findstring _DYDZ_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _DYDZ
else ifneq (,$(findstring _TOPFLY_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _TOPFLY
else ifneq (,$(findstring _UBIA_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _UBIA
else ifneq (,$(findstring _DF_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _DF
else ifneq (,$(findstring _VOLTE_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _VOLTE
else ifneq (,$(findstring _TTS_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _TTS
else ifneq (,$(findstring _LARGENVM_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _LARGENVM
else ifneq (,$(findstring _ONOMONDO_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _ONOMONDO
else ifneq (,$(findstring _ZM_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _ZM
else ifneq (,$(findstring _RU_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _RU
else ifneq (,$(findstring _SAFONE_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _SAFONE
else ifneq (,$(findstring _A7680C_LANS_1606_V506_CQCY_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _CQCY
else ifneq (,$(findstring _A7672E_LASE_1603_V101_OPENSDK_CSD_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _APP_640K
else ifneq (,$(findstring _A7680C_LNNV_1602_V801_OPENSDK_YJ_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _YJ
else ifneq (,$(findstring _FOTA_,_$(SC_MODULE_FULL)_))
    ifneq (,$(findstring _1603_,_${SC_MODULE_FULL}_))
        FLASH_TARGET_CUSTOM := _FOTA
    endif
else ifneq (,$(findstring _CUSAUDIO_,_$(SC_MODULE_FULL)_))
    ifneq (,$(findstring _A7680C_MANV_,_${SC_MODULE_FULL}_))
        FLASH_TARGET_CUSTOM := _AUDIO
    endif
else
	FLASH_TARGET_CUSTOM :=
endif
# STANDARD FLASH_TARGET_CUSTOM END

# OPENSDK FLASH_TARGET_CUSTOM START
ifneq (,$(findstring _OPENSDK,$(SC_MODULE_BASE)))
	ifneq (,$(findstring _IMSSMS_,_$(SC_MODULE_FULL)_))
		FLASH_TARGET_CUSTOM := _IMSSMS
	endif

	ifneq (,$(findstring _XC_,_$(SC_MODULE_FULL)_)$(findstring _ZC_,_$(SC_MODULE_FULL)_))
		FLASH_TARGET_CUSTOM := _APP_512K
	endif
	ifneq (,$(findstring _XYJ_,_$(SC_MODULE_FULL)_))
		FLASH_TARGET_CUSTOM := _XYJ_768K
	endif
	ifneq (,$(findstring _LP_,_$(SC_MODULE_FULL)_))
		FLASH_TARGET_CUSTOM := _LP
	endif
	ifneq (,$(findstring _WWKJ_,_$(SC_MODULE_FULL)_))
		FLASH_TARGET_CUSTOM := _WWKJ
	endif
	ifneq (,$(findstring _ZXZN_,_$(SC_MODULE_FULL)_))
		FLASH_TARGET_CUSTOM := _ZXZN
	endif
	ifneq (,$(findstring _JWZD_,_$(SC_MODULE_FULL)_))
		ifneq (,$(findstring _1606_,_${SC_MODULE_FULL}_))
			FLASH_TARGET_CUSTOM := _APP_512K
		else ifneq (,$(findstring _1602_,_${SC_MODULE_FULL}_))
			FLASH_TARGET_CUSTOM := _JWZD
		else
		endif
	endif

	ifneq (,$(findstring _A7677S_MANS_1606_V702_OPENSDK_ADSUN_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _ADSUN
	endif

	ifneq (,$(findstring _FILESYSTEM_,_$(SC_MODULE_FULL)_))
		FLASH_TARGET_CUSTOM := _FS
	endif

	ifneq (,$(findstring _TBDTU_,_$(SC_MODULE_FULL)_))
		FLASH_TARGET_CUSTOM := _TBDTU
	endif

	ifneq (,$(findstring _WALNUT_,_$(SC_MODULE_FULL)_))
		ifneq (,$(findstring _A7677S_LANS_,_$(SC_MODULE_FULL)_)$(findstring _A7672S_LASC_,_$(SC_MODULE_FULL)_)$(findstring _A7677S_LANV_,_$(SC_MODULE_FULL)_))
			ifneq (,$(findstring _APP768K_,_${SC_MODULE_FULL}_))
				FLASH_TARGET_CUSTOM := _APP_768K
			else
				FLASH_TARGET_CUSTOM := _WALNUT
			endif
		endif
	endif

	ifneq (,$(findstring _YUNHAI_,_${SC_MODULE_FULL}_))
		ifeq (4M,$(patsubst %V,4M,$(SC_MODULE_FULL)))
			FLASH_TARGET_CUSTOM := _YUNHAI
		endif
	endif

	ifneq (,$(findstring _GTK_,_${SC_MODULE_FULL}_))
		FLASH_TARGET_CUSTOM := _GTK
	endif
	ifneq (,$(findstring _HTTX_,_${SC_MODULE_FULL}_))
		ifneq (,$(findstring _1602_,_${SC_MODULE_FULL}_))
			ifeq (TRUE,$(patsubst %V,TRUE,$(word 2,$(subst _, ,$(SC_MODULE_FULL)))))
				FLASH_TARGET_CUSTOM := _HTTX
			endif
		endif
	endif
endif

# OPENSDK FLASH_TARGET_CUSTOM END

ifneq (,$(findstring _OPENSDK,$(SC_MODULE_BASE)))
ASR_PRODUCT := $(FLASH_TARGET_PRE)$(PRODUCT_SIM_TYPE)$(FLASH_TARGET_SIZE)$(FLASH_TARGET_HD)$(FLASH_TARGET_CUSTOM)_OPENSDK
else
ASR_PRODUCT := $(FLASH_TARGET_PRE)$(PRODUCT_SIM_TYPE)$(FLASH_TARGET_SIZE)$(FLASH_TARGET_HD)$(FLASH_TARGET_CUSTOM)
endif
