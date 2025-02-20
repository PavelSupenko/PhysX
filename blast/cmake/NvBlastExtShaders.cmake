set(NV_BLAST_EXT_SHADERS_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/shaders/NvBlastExtDamageAcceleratorAABBTree.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/shaders/NvBlastExtDamageAccelerators.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/shaders/NvBlastExtDamageShaders.cpp

)

set(NV_BLAST_EXT_SHADERS_INCLUDES
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/source/sdk/common
    ${PROJECT_SOURCE_DIR}/include/extensions/shaders
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/shaders
    ${PROJECT_SOURCE_DIR}/include/lowlevel
    ${PROJECT_SOURCE_DIR}/include/globals
    ${PROJECT_SOURCE_DIR}/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/source/shared/NsFoundation/include
)

add_library(NvBlastExtShaders ${LIBRARIES_TYPE}
    ${NV_BLAST_EXT_SHADERS_SOURCE}
)

target_include_directories(NvBlastExtShaders PRIVATE
    ${NV_BLAST_EXT_SHADERS_INCLUDES}
)

target_link_libraries(NvBlastExtShaders
    NvBlast
    NvBlastGlobals
)

target_compile_definitions(NvBlastExtShaders PRIVATE 
    LOG_COMPONENT="NvBlastExtShaders"
)
