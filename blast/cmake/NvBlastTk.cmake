set(NV_BLAST_TK_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/source/sdk/toolkit/NvBlastTkActorImpl.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/toolkit/NvBlastTkAssetImpl.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/toolkit/NvBlastTkFamilyImpl.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/toolkit/NvBlastTkFrameworkImpl.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/toolkit/NvBlastTkGroupImpl.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/toolkit/NvBlastTkJointImpl.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/toolkit/NvBlastTkTask.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/toolkit/NvBlastTkTaskImpl.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/toolkit/NvBlastTkTaskManager.cpp
)

set(NV_BLAST_TK_INCLUDES
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/source/sdk/common
    ${PROJECT_SOURCE_DIR}/include/toolkit
    ${PROJECT_SOURCE_DIR}/source/sdk/toolkit
    ${PROJECT_SOURCE_DIR}/include/lowlevel
    ${PROJECT_SOURCE_DIR}/include/globals
    ${PROJECT_SOURCE_DIR}/source/sdk/globals
    ${PROJECT_SOURCE_DIR}/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/source/shared/NsFoundation/include
    ${PROJECT_SOURCE_DIR}/source/shared/NsFileBuffer/include
    ${PROJECT_SOURCE_DIR}/source/shared/NvTask/include
)

add_library(NvBlastTk ${LIBRARIES_TYPE}
    ${NV_BLAST_TK_SOURCE}
)

target_include_directories(NvBlastTk PRIVATE
    ${NV_BLAST_TK_INCLUDES}
)

target_link_libraries(NvBlastTk
    NvBlast
    NvBlastGlobals
)

if (MSVC)
    target_link_libraries(NvBlastTk
        Rpcrt4.Lib
    )
endif()

if(NV_CONFIGURATION_TYPE STREQUAL DEBUG)
    target_compile_definitions(NvBlastTk PRIVATE 
        LOG_COMPONENT="NvBlastTk"
        _CRT_NONSTDC_NO_DEPRECATE
        BOOST_USE_WINDOWS_H=1
        _DEBUG
        CARB_DEBUG=1
    )
else()
    target_compile_definitions(NvBlastTk PRIVATE 
        LOG_COMPONENT="NvBlastTk"
        _CRT_NONSTDC_NO_DEPRECATE
        BOOST_USE_WINDOWS_H=1
        NDEBUG
        CARB_DEBUG=0
)
endif()
