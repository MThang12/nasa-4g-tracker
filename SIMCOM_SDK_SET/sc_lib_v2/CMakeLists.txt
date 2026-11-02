cmake_minimum_required(VERSION 3.10)

AUX_SOURCE_DIRECTORY(. sc_lib_src_v2)
AUX_SOURCE_DIRECTORY(./SAL sc_lib_src_v2)
AUX_SOURCE_DIRECTORY(./MAL sc_lib_src_v2)
AUX_SOURCE_DIRECTORY(./HAL sc_lib_src_v2)
AUX_SOURCE_DIRECTORY(./Pub sc_lib_src_v2)
AUX_SOURCE_DIRECTORY(./PL sc_lib_src_v2)

# Add the static library
add_library(sc_lib_v2 STATIC ${sc_lib_src_v2})
target_compile_options(sc_lib_v2 PRIVATE -Werror)
target_include_directories(sc_lib_v2 PUBLIC
    ./SAL ./MAL ./HAL ./Pub ./PL
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/sc_lib/inc
    ${CMAKE_SOURCE_DIR}/sc_lib/${SCMODULE}/inc
)

# Add the static library
add_library(sc_lib_v2_debug STATIC ${sc_lib_src_v2})
target_compile_options(sc_lib_v2_debug PRIVATE -Werror -DSIMCOM_DEBUG)
target_include_directories(sc_lib_v2_debug PUBLIC
    ./SAL ./MAL ./HAL ./Pub ./PL
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/sc_lib/inc
    ${CMAKE_SOURCE_DIR}/sc_lib/${SCMODULE}/inc
)

# SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/sc_lib_v2/lib)
