#!/bin/bash
MODULE=$1
APP_NAME=$2
APP_TOP_DIR=${PWD}
echo "APP_TOP_DIR : ${APP_TOP_DIR}"
ABOOT=${APP_TOP_DIR}/tools/linux/aboot
APP_BIN_DIR=${APP_TOP_DIR}/out/${MODULE}
SC_IMAGES=${APP_TOP_DIR}/tools/win32/aboot/images/sc_images/${MODULE}

ARELEASE=${ABOOT}/arelease

TargetConfigDir=${ABOOT}
TargetPath=${APP_TOP_DIR}/out/${MODULE}
Targetname=SC_${MODULE}.zip

cp_image=${APP_TOP_DIR}/kernel/${MODULE}/cp.bin
rd_image=${SC_IMAGES}/ReliableData.bin
rf_image=${SC_IMAGES}/rf.bin
app_image=${APP_BIN_DIR}/${APP_NAME}.bin

if [[ $MODULE =~ 8M ]]
then
    if [[ $MODULE =~ A7680C_M ]]; then
        FLASH_TARGET=CRANEL_A0_8M_ASR5311
    elif [[ $MODULE =~ _F ]]; then
        FLASH_TARGET=CRANEL_A0_8M_BT
    elif [[ $MODULE =~ _B ]]; then
        FLASH_TARGET=CRANEL_A0_8M_BT
    else
        FLASH_TARGET=CRANEL_A0_8M
    fi
fi

if [[ $MODULE =~ 4M ]]
then
    if [[ $MODULE =~ A7680C_M ]]; then
        FLASH_TARGET=CRANEL_A0_4M_ASR5311
    elif [[ $MODULE =~ _F ]]; then
        FLASH_TARGET=CRANEL_A0_4M_BT
    elif [[ $MODULE =~ _B ]]; then
        FLASH_TARGET=CRANEL_A0_4M_BT
    else
        FLASH_TARGET=CRANEL_A0_4M
    fi
fi

echo FLASH_TARGET: $FLASH_TARGET
Images="customer_app=${app_image},cp=${cp_image},rd=${rd_image},rfbin=${rf_image}"
${ABOOT}/arelease -c ${TargetConfigDir}  -g -p ASR_CRANEL_EVB -v ${FLASH_TARGET} -i ${Images}  ${TargetPath}/${Targetname}