set(NV_BLAST_TK_SER_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/CapnProto/src/capnp/arena.c++
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/CapnProto/src/capnp/blob.c++
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/CapnProto/src/capnp/layout.c++
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/CapnProto/src/capnp/message.c++
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/CapnProto/src/capnp/serialize.c++
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/CapnProto/src/kj/array.c++
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/CapnProto/src/kj/common.c++
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/CapnProto/src/kj/debug.c++
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/CapnProto/src/kj/exception.c++
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/CapnProto/src/kj/io.c++
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/CapnProto/src/kj/mutex.c++
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/CapnProto/src/kj/string.c++
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/CapnProto/src/kj/units.c++
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common/NvBlastAssert.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common/NvBlastAtomic.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common/NvBlastTime.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common/NvBlastTimers.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/DTO/AssetDTO.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/DTO/NvBlastBondDTO.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/DTO/NvBlastChunkDTO.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/DTO/NvBlastIDDTO.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/DTO/NvVec3DTO.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/DTO/TkAssetDTO.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/DTO/TkAssetJointDescDTO.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/NvBlastExtInputStream.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/NvBlastExtOutputStream.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/NvBlastExtTkSerialization.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/NvBlastExtTkSerializerRAW.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/generated/NvBlastExtLlSerialization-capn.c++
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/generated/NvBlastExtTkSerialization-capn.c++
)

set(NV_BLAST_TK_SER_INCLUDES
    ${PROJECT_SOURCE_DIR}/blast/include
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common
    ${PROJECT_SOURCE_DIR}/blast/include/extensions/serialization
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/DTO
    ${PROJECT_SOURCE_DIR}/blast/include/lowlevel
    ${PROJECT_SOURCE_DIR}/blast/include/toolkit
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel
    ${PROJECT_SOURCE_DIR}/blast/include/globals
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/CapnProto/src
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/generated
    ${PROJECT_SOURCE_DIR}/blast/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/blast/source/shared/NsFoundation/include
    ${PROJECT_SOURCE_DIR}/blast/source/shared/NsFileBuffer/include
    ${PROJECT_SOURCE_DIR}/blast/source/shared/NvTask/include
    
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/msvc/VC/Tools/MSVC/14.16.27023/include
)

add_library(NvBlastExtTkSerialization ${LIBRARIES_TYPE}
    ${NV_BLAST_TK_SER_SOURCE}
)

target_include_directories(NvBlastExtTkSerialization PRIVATE
    ${NV_BLAST_TK_SER_INCLUDES}
)

target_link_libraries(NvBlastExtTkSerialization
    NvBlast
    NvBlastTk
    NvBlastGlobals
)

if(NV_CONFIGURATION_TYPE STREQUAL DEBUG)
    target_compile_definitions(NvBlastExtTkSerialization PRIVATE 
        LOG_COMPONENT="NvBlastExtTkSerialization"
        _CRT_NONSTDC_NO_DEPRECATE
        BOOST_USE_WINDOWS_H=1
        _DEBUG
        CARB_DEBUG=1
        KJ_HEADER_WARNINGS=0
    )
else()
    target_compile_definitions(NvBlastExtTkSerialization PRIVATE 
        LOG_COMPONENT="NvBlastExtTkSerialization"
        _CRT_NONSTDC_NO_DEPRECATE
        BOOST_USE_WINDOWS_H=1
        NDEBUG
        CARB_DEBUG=0
        KJ_HEADER_WARNINGS=0
)
endif()
