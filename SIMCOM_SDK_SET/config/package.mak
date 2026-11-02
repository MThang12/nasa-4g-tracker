include ${ROOT_DIR}/config/ToolChain.mak
include ${KERNEL_DIR}/${SCMODULE}/KernelDefinitions.mak

SRC_FILES:=$(subst \,/,$(SRC_FILES))
INC_DIRS:=$(subst \,/,$(INC_DIRS))

SRC_DIRS:=$(dir $(SRC_FILES))
MAKE_PATH:=$(patsubst %/,%,$(dir $(abspath $(firstword $(MAKEFILE_LIST)))))
TARGET_NAME:=$(lastword $(subst /, ,$(subst \, ,$(MAKE_PATH))))
BUILD_PATH:=$(BUILD_PATH)
OBJ_PATH:=$(BUILD_PATH)/packag-$(TARGET_NAME)

OBJS:=$(SRC_FILES:.c=.o)
OBJS:=$(OBJS:.cpp=.o)
OBJS:=$(OBJS:.S=.o)
OBJS:=$(OBJS:.s=.o)
OBJS:=$(patsubst %,$(OBJ_PATH)/%,$(OBJS))

DEF_FILE_TEMP:=$(SRC_FILES:.c=.d)
DEF_FILE_TEMP:=$(DEF_FILE_TEMP:.cpp=.d)
DEF_FILE_TEMP:=$(patsubst %,$(subst $(strip \),/,$(OBJ_PATH))/%,$(DEF_FILE_TEMP))
DEF_FILE:=$(DEF_FILE_TEMP)

#-------------------------------------------------------------------------------
# build flags setting
#-------------------------------------------------------------------------------
FNO_BUILTIN_FLAGS := -fno-builtin-printf -fno-builtin-malloc -fno-builtin-free -fno-builtin-time -fno-builtin-gmtime -fno-builtin-gettimeofday -fno-builtin-mktime -fno-builtin-puts -fno-builtin-signal

SFLAGS := -c -mlong-calls -march=armv7-r -mcpu=cortex-r4 -mlittle-endian -mthumb -mthumb-interwork -mfloat-abi=soft -Wstrict-prototypes -Os -fdiagnostics-color=always

CFLAGS := -c -MMD -mlong-calls -mcpu=cortex-r4 -mlittle-endian -mthumb -mfloat-abi=soft -Wall -ffunction-sections -fdata-sections -fgnu89-inline ${FNO_BUILTIN_FLAGS} -D__OCPU_COMPILER_GCC__ -D_WANT_USE_LONG_TIME_T -std=c99 -Os -fdiagnostics-color=always

CPPFLAGS := -c -MMD -mlong-calls  -mcpu=cortex-r4  -mlittle-endian -mthumb -mthumb-interwork -mfloat-abi=soft -Wall -ffunction-sections -fdata-sections -fgnu89-inline ${FNO_BUILTIN_FLAGS} -D__OCPU_COMPILER_GCC__ -D_WANT_USE_LONG_TIME_T -std=c++11 -Os -fdiagnostics-color=always

ARFLAG := -rc


.PHONY: all clean makeDir

TARGET := $(LIB_PATH)/cuslib$(TARGET_NAME).a

all:makeDir $(sort $(dir ${OBJS})) $(TARGET)

makeDir:
	-${MKDIR} ${MKDIRARG} $(subst ${BAD_SLASH},${GOOD_SLASH},$(BUILD_PATH))
	-${MKDIR} ${MKDIRARG} $(subst ${BAD_SLASH},${GOOD_SLASH},$(OBJ_PATH))
	-${MKDIR} ${MKDIRARG} $(subst ${BAD_SLASH},${GOOD_SLASH},$(LIB_PATH))

$(sort $(dir ${OBJS})): makeDir
	-${MKDIR} ${MKDIRARG} $(subst ${BAD_SLASH},${GOOD_SLASH},$@)

clean:
	-${RMDIR} ${RMDIRARG} $(subst ${BAD_SLASH},${GOOD_SLASH},$(BUILD_PATH))
	-${RMDIR} ${RMDIRARG} $(subst ${BAD_SLASH},${GOOD_SLASH},$(TARGET))

$(TARGET):$(OBJS)
	@echo --- AR $(subst \,/,$@)
	@$(AR) $(ARFLAG) $@ $^

$(OBJ_PATH)/%.o: %.S
	@echo --- CC ${MAKE_PATH}/$<
	@$(CC) $(SFLAGS) -o $@ $<

$(OBJ_PATH)/%.o: %.c
	@echo --- CC ${MAKE_PATH}/$<
	@$(CC) $(CFLAGS) ${SDK_KERNEL_PREMACRO} $(DFLAG) $(DFLAGS) $(U_FLAGS) $(INC_DIRS) -o $@ $<

$(OBJ_PATH)/%.o: %.cpp
	@echo --- CC ${MAKE_PATH}/$<
	@$(CPP) $(CPPFLAGS) ${SDK_KERNEL_PREMACRO} $(DFLAG) $(DFLAGS) $(U_FLAGS) $(INC_DIRS)  -o $@ $<

-include $(DEF_FILE)
