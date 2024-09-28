set(NV_BLAST_TK_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/toolkit/NvBlastTkActorImpl.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/toolkit/NvBlastTkAssetImpl.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/toolkit/NvBlastTkFamilyImpl.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/toolkit/NvBlastTkFrameworkImpl.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/toolkit/NvBlastTkGroupImpl.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/toolkit/NvBlastTkJointImpl.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/toolkit/NvBlastTkTask.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/toolkit/NvBlastTkTaskImpl.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/toolkit/NvBlastTkTaskManager.cpp
)

set(NV_BLAST_TK_INCLUDES
    ${PROJECT_SOURCE_DIR}/blast/include
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common
    ${PROJECT_SOURCE_DIR}/blast/include/toolkit
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/toolkit
    ${PROJECT_SOURCE_DIR}/blast/include/lowlevel
    ${PROJECT_SOURCE_DIR}/blast/include/globals
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/globals
    ${PROJECT_SOURCE_DIR}/blast/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/blast/source/shared/NsFoundation/include
    ${PROJECT_SOURCE_DIR}/blast/source/shared/NsFileBuffer/include
    ${PROJECT_SOURCE_DIR}/blast/source/shared/NvTask/include
    
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/msvc/VC/Tools/MSVC/14.16.27023/include
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

link_directories(
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/winsdk/lib/um/x64
)

target_link_libraries(NvBlastTk
    # ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/winsdk/lib/um/x64/Rpcrt4.Lib
    Rpcrt4.Lib # location mentioned above was added via link_directories
)

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
