#设置私有编译宏
SET(SDK_ADD_PRIV_PREMACRO
    -D__TM_ZONE=tm_zone
    -D__TM_GMTOFF=tm_gmtoff
    -D_REENT_SMALL
)

# 设置三方库
# SET(THIRD_LIB
#     "${CMAKE_CURRENT_SOURCE_DIR}/third_lib/*.a"
# )
