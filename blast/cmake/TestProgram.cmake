set(NV_BLAST_EXT_BRIDGE_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/source/program/TestProgram.cpp
)

set(NV_BLAST_EXT_BRIDGE_INCLUDES
    ${PROJECT_SOURCE_DIR}/include/extensions/authoringCommon
    ${PROJECT_SOURCE_DIR}/include/extensions/authoring
    ${PROJECT_SOURCE_DIR}/include/extensions/bridge
    ${PROJECT_SOURCE_DIR}/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/include/lowlevel
    ${PROJECT_SOURCE_DIR}/include/globals
    ${PROJECT_SOURCE_DIR}/include/extensions/assetutils
    ${PROJECT_SOURCE_DIR}/source/sdk/common
    ${PROJECT_SOURCE_DIR}/dependencies/shared/BoostMultiprecision
    ${PROJECT_SOURCE_DIR}/include/extensions/bridge
)

add_executable(TestProgram
    ${NV_BLAST_EXT_BRIDGE_SOURCE}
)

target_include_directories(TestProgram PRIVATE
    ${NV_BLAST_EXT_BRIDGE_INCLUDES}
)

target_link_libraries(TestProgram
    NvBlast
    NvBlastGlobals
    NvBlastExtAuthoring
    NvBlastExtBridge
)

target_compile_definitions(NvBlastExtBridge PRIVATE 
        LOG_COMPONENT="NvBlastExtBridge"
)
