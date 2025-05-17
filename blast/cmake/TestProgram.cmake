set(NV_BLAST_EXT_UNITY_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/source/program/TestProgram.cpp
)

set(NV_BLAST_EXT_UNITY_INCLUDES
    ${PROJECT_SOURCE_DIR}/include/extensions/authoringCommon
    ${PROJECT_SOURCE_DIR}/include/extensions/authoring
    ${PROJECT_SOURCE_DIR}/include/extensions/unity
    ${PROJECT_SOURCE_DIR}/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/include/lowlevel
    ${PROJECT_SOURCE_DIR}/include/globals
    ${PROJECT_SOURCE_DIR}/include/extensions/assetutils
    ${PROJECT_SOURCE_DIR}/source/sdk/common
    ${PROJECT_SOURCE_DIR}/dependencies/shared/BoostMultiprecision
    ${PROJECT_SOURCE_DIR}/include/extensions/unity
)

add_executable(TestProgram
    ${NV_BLAST_EXT_UNITY_SOURCE}
)

target_include_directories(TestProgram PRIVATE
    ${NV_BLAST_EXT_UNITY_INCLUDES}
)

target_link_libraries(TestProgram
    NvBlast
    NvBlastGlobals
    NvBlastExtAuthoring
    NvBlastExtUnity
)

target_compile_definitions(NvBlastExtUnity PRIVATE 
        LOG_COMPONENT="NvBlastExtUnity"
)
