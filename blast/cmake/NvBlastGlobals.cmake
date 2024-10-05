set(NV_BLAST_GLOBALS_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/source/sdk/globals/NvBlastGlobals.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/globals/NvBlastInternalProfiler.cpp
)

set(NV_BLAST_GLOBALS_INCLUDES
    ${PROJECT_SOURCE_DIR}/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/include/lowlevel
    ${PROJECT_SOURCE_DIR}/include/globals
    ${PROJECT_SOURCE_DIR}/source/sdk/common
    ${PROJECT_SOURCE_DIR}/source/shared/NsFoundation/include
)

add_library(NvBlastGlobals ${LIBRARIES_TYPE}
    ${NV_BLAST_GLOBALS_SOURCE}
)

target_include_directories(NvBlastGlobals PRIVATE
    ${NV_BLAST_GLOBALS_INCLUDES}
)

target_compile_definitions(NvBlastGlobals PRIVATE 
        LOG_COMPONENT="NvBlastGlobals"
)
