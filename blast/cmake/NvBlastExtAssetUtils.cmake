set(NV_BLAST_EXT_ASSET_UTILS_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/assetutils/NvBlastExtAssetUtils.cpp
)

set(NV_BLAST_EXT_ASSET_UTILS_INCLUDES
    ${PROJECT_SOURCE_DIR}/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/include/lowlevel
    ${PROJECT_SOURCE_DIR}/include/globals
    ${PROJECT_SOURCE_DIR}/include/extensions/assetutils
    ${PROJECT_SOURCE_DIR}/source/sdk/common
)

add_library(NvBlastExtAssetUtils ${LIBRARIES_TYPE}
    ${NV_BLAST_EXT_ASSET_UTILS_SOURCE}
)

target_include_directories(NvBlastExtAssetUtils PRIVATE
    ${NV_BLAST_EXT_ASSET_UTILS_INCLUDES}
)

target_link_libraries(NvBlastExtAssetUtils
    NvBlast
    NvBlastGlobals
)

target_compile_definitions(NvBlastExtAssetUtils PRIVATE 
        LOG_COMPONENT="NvBlastExtAssetUtils"
)
