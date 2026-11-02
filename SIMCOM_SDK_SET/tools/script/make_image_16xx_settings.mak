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
ifneq (,$(findstring _1.011.069_,_${BASE_LINE}_))
PRODUCT_SIM_TYPE :=
else
ifneq (,$(findstring _DS_,$(SC_MODULE_FULL)_))
	PRODUCT_SIM_TYPE :=
else
	PRODUCT_SIM_TYPE := _SINGLE_SIM
endif
endif
else ifneq (,$(findstring _1603_,_${SC_MODULE_FULL}_))
FLASH_TARGET_PRE := CRANEM_A0
ASR_PRODUCT_TYPE := ASR_CRANEM_EVB
endif

ifneq ($(word 2,$(subst _, ,$(SC_MODULE_FULL)))$(word 2,$(subst _, ,$(SC_MODULE_FULL))),$(patsubst %S,8M,$(word 2,$(subst _, ,$(SC_MODULE_FULL))))$(patsubst %C,8M,$(word 2,$(subst _, ,$(SC_MODULE_FULL)))))
	FLASH_TARGET_SIZE := _08MB
else ifeq (TRUE,$(patsubst %V,TRUE,$(word 2,$(subst _, ,$(SC_MODULE_FULL)))))
	FLASH_TARGET_SIZE := _04MB
else ifeq (TRUE,$(patsubst %Y,TRUE,$(word 2,$(subst _, ,$(SC_MODULE_FULL)))))
	FLASH_TARGET_SIZE := _02MB
else ifeq (TRUE,$(patsubst %E,TRUE,$(word 2,$(subst _, ,$(SC_MODULE_FULL)))))
	FLASH_TARGET_SIZE := _16MB
else ifneq (,$(findstring A7682E_1603_V102,$(SC_MODULE_FULL)))
	# special product type
	FLASH_TARGET_SIZE := _16MB
endif

ifneq (,$(findstring A7680C_M,$(SC_MODULE_FULL))$(findstring M5780C,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _ASR5311
else ifneq (,$(findstring WHXA,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _WHXA
else ifneq (,$(findstring GWSD,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _GWSD
else ifneq (,$(findstring SHHD,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _SHHD
else ifneq (,$(findstring JWZD,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _APP_512K
else ifneq (,$(findstring KDWL,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _APP_512K
else ifneq (,$(findstring WHXBY,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _APP_512K
else ifneq (,$(findstring A7680C_LANS_1606_XC,$(SC_MODULE_FULL))$(findstring YHWL,${SC_MODULE_FULL}))
	FLASH_TARGET_CUSTOM := _APP_512K
else ifneq (,$(findstring A7680C_LANV_1606_V503_XC,$(SC_MODULE_FULL))$(findstring A7670C_LANV_1606_V701_XC,$(SC_MODULE_FULL))$(findstring XMWE,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _APP_320K
else ifneq (,$(findstring BL_POC_MMI,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _APP_512K
else ifneq (,$(findstring _A7670C_LANS_1606_V701_XC_,_$(SC_MODULE_FULL)_))
	FLASH_TARGET_CUSTOM := _XC
else ifneq (,$(findstring _ST,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _ST
else ifneq (,$(findstring _MSJ,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _MSJ
else ifneq (,$(findstring _HWQS,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _HWQS
else ifneq (,$(findstring _XYJ,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _XYJ
else ifneq (,$(findstring _AJBDZ,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _AJBDZ
else ifneq (,$(findstring ZS,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _ZS
else ifneq (,$(findstring GZYM,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _GZYM
else ifneq (,$(findstring QZZT_POC_MMI,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _QZZT
else ifneq (,$(findstring _HX,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _HX
else ifneq (,$(findstring _HQ,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _HQ
else ifneq (,$(findstring _LDJD,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _LDJD
else ifneq (,$(findstring _MT,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _MT
else ifneq (,$(findstring _YKKJ,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _YKKJ
else ifneq (,$(findstring QZXC_POC,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _QZXC_POC
else ifneq (,$(findstring _PWY,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _PWY
else ifneq (,$(findstring _JXTZ,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _JXTZ
else ifneq (,$(findstring A7670C_LANS_1606_V701_CZJ_POC,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _CZJ
else ifneq (,$(findstring _SJ,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _SJ
else ifneq (,$(findstring _WJA,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _WJA
else ifneq (,$(findstring _GTK,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _GTK
else ifneq (,$(findstring _BNY,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _BNY
else ifneq (,$(findstring _ZZCXNY,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _ZZCXNY
else ifneq (,$(findstring _SAFONE,$(SC_MODULE_FULL)))
	FLASH_TARGET_CUSTOM := _BT_SAFONE
else ifneq ($(word 2,$(subst _, ,$(SC_MODULE_FULL)))$(word 2,$(subst _, ,$(SC_MODULE_FULL))),$(patsubst F%,,$(word 2,$(subst _, ,$(SC_MODULE_FULL))))$(patsubst B%,,$(word 2,$(subst _, ,$(SC_MODULE_FULL)))))
	FLASH_TARGET_CUSTOM := _BT
else
	FLASH_TARGET_CUSTOM :=
endif


ifneq (,$(findstring _OPENSDK,$(SC_MODULE_BASE)))
	ifneq (,$(findstring HTTX,$(SC_MODULE_FULL)))
		ifneq (,$(findstring MANS,$(SC_MODULE_FULL)))
			FLASH_TARGET_CUSTOM := _ASR5311_HTTX
		else
			FLASH_TARGET_CUSTOM := _HTTX
		endif
	endif
	ifneq (,$(findstring A7680C_MANS,$(SC_MODULE_FULL)))
		ifneq (,$(findstring _XC,$(SC_MODULE_FULL))$(findstring _ZC,$(SC_MODULE_FULL)))
			FLASH_TARGET_CUSTOM := _ASR5311_APP_512K
		endif
		ifneq (,$(findstring _XYJ,$(SC_MODULE_FULL)))
			FLASH_TARGET_CUSTOM := _ASR5311_XYJ_768K
		endif
		ifneq (,$(findstring _LP,$(SC_MODULE_FULL)))
			FLASH_TARGET_CUSTOM := _ASR5311_LP
		endif
	endif

	ifneq (,$(findstring _WWKJ,$(SC_MODULE_FULL)))
		ifneq (,$(findstring A7680C_MANS_1606_V603_WWKJ,$(SC_MODULE_FULL))$(findstring A7680C_MANS_1606_V602_WWKJ,$(SC_MODULE_FULL)))
			FLASH_TARGET_CUSTOM := _ASR5311_WWKJ
		else
			FLASH_TARGET_CUSTOM := _WWKJ
		endif
	endif

	ifneq (,$(findstring A7680C_MANV,$(SC_MODULE_FULL)))
		ifneq (,$(findstring ZXZN,$(SC_MODULE_FULL)))
			FLASH_TARGET_CUSTOM := _ZXZN
		else
			FLASH_TARGET_CUSTOM := _ASR5311_APP_140K
		endif
	endif
	ifneq (,$(findstring _JWZD,$(SC_MODULE_FULL)))
		ifneq (,$(findstring A7670C_FANS_1606_V602_OPENSDK_JWZD,$(SC_MODULE_FULL)))
			FLASH_TARGET_CUSTOM := _BT_APP_512K
		else
			FLASH_TARGET_CUSTOM := _APP_512K
		endif
	endif
endif

ifneq (,$(findstring _OPENSDK,$(SC_MODULE_BASE)))
ASR_PRODUCT := $(FLASH_TARGET_PRE)$(PRODUCT_SIM_TYPE)$(FLASH_TARGET_SIZE)$(FLASH_TARGET_CUSTOM)_OPENSDK
else
ASR_PRODUCT := $(FLASH_TARGET_PRE)$(PRODUCT_SIM_TYPE)$(FLASH_TARGET_SIZE)$(FLASH_TARGET_CUSTOM)
endif
