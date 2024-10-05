set(NV_BLAST_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/source/sdk/lowlevel/NvBlastActor.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/lowlevel/NvBlastActorSerializationBlock.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/lowlevel/NvBlastAsset.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/lowlevel/NvBlastAssetHelper.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/lowlevel/NvBlastFamily.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/lowlevel/NvBlastFamilyGraph.cpp
)

set(NV_BLAST_INCLUDES
    ${PROJECT_SOURCE_DIR}/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/include/lowlevel
    ${PROJECT_SOURCE_DIR}/source/sdk/common
)

add_library(NvBlast ${LIBRARIES_TYPE}
    ${NV_BLAST_SOURCE}
)

target_include_directories(NvBlast PRIVATE
    ${NV_BLAST_INCLUDES}
)

target_compile_definitions(NvBlast PRIVATE 
        LOG_COMPONENT="NvBlast"
)
