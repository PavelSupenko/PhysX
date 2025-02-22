set(NV_BLAST_EXT_PHYSX_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/physx/sync/NvBlastExtSync.cpp
)

set(NV_BLAST_EXT_PHYSX_INCLUDES
    ${PROJECT_SOURCE_DIR}/../physx/include
    ${PROJECT_SOURCE_DIR}/../physx/include/geometry
    ${PROJECT_SOURCE_DIR}/../physx/include/foundation
    ${PROJECT_SOURCE_DIR}/include/extensions/physx
    ${PROJECT_SOURCE_DIR}/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/include/lowlevel
    ${PROJECT_SOURCE_DIR}/include/globals
    ${PROJECT_SOURCE_DIR}/include/toolkit
    ${PROJECT_SOURCE_DIR}/source/sdk/common
)

add_library(NvBlastExtPhysX ${LIBRARIES_TYPE}
    ${NV_BLAST_EXT_PHYSX_SOURCE}
)

target_include_directories(NvBlastExtPhysX PRIVATE
    ${NV_BLAST_EXT_PHYSX_INCLUDES}
)

target_link_libraries(NvBlastExtPhysX
    NvBlast
    NvBlastGlobals
    NvBlastExtAuthoring
    NvBlastTk
)

target_compile_definitions(NvBlastExtPhysX PRIVATE 
        LOG_COMPONENT="NvBlastExtPhysX"
)
