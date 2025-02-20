set(NV_BLAST_TK_SER_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/dependencies/shared/CapnProto/src/capnp/arena.c++
    ${PROJECT_SOURCE_DIR}/dependencies/shared/CapnProto/src/capnp/blob.c++
    ${PROJECT_SOURCE_DIR}/dependencies/shared/CapnProto/src/capnp/layout.c++
    ${PROJECT_SOURCE_DIR}/dependencies/shared/CapnProto/src/capnp/message.c++
    ${PROJECT_SOURCE_DIR}/dependencies/shared/CapnProto/src/capnp/serialize.c++
    ${PROJECT_SOURCE_DIR}/dependencies/shared/CapnProto/src/kj/array.c++
    ${PROJECT_SOURCE_DIR}/dependencies/shared/CapnProto/src/kj/common.c++
    ${PROJECT_SOURCE_DIR}/dependencies/shared/CapnProto/src/kj/debug.c++
    ${PROJECT_SOURCE_DIR}/dependencies/shared/CapnProto/src/kj/exception.c++
    ${PROJECT_SOURCE_DIR}/dependencies/shared/CapnProto/src/kj/io.c++
    ${PROJECT_SOURCE_DIR}/dependencies/shared/CapnProto/src/kj/mutex.c++
    ${PROJECT_SOURCE_DIR}/dependencies/shared/CapnProto/src/kj/string.c++
    ${PROJECT_SOURCE_DIR}/dependencies/shared/CapnProto/src/kj/units.c++
    ${PROJECT_SOURCE_DIR}/source/sdk/common/NvBlastAssert.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/common/NvBlastAtomic.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/common/NvBlastTime.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/common/NvBlastTimers.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/serialization/DTO/AssetDTO.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/serialization/DTO/NvBlastBondDTO.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/serialization/DTO/NvBlastChunkDTO.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/serialization/DTO/NvBlastIDDTO.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/serialization/DTO/NvVec3DTO.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/serialization/DTO/TkAssetDTO.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/serialization/DTO/TkAssetJointDescDTO.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/serialization/NvBlastExtInputStream.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/serialization/NvBlastExtOutputStream.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/serialization/NvBlastExtTkSerialization.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/serialization/NvBlastExtTkSerializerRAW.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/serialization/generated/NvBlastExtLlSerialization-capn.c++
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/serialization/generated/NvBlastExtTkSerialization-capn.c++
)

set(NV_BLAST_TK_SER_INCLUDES
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/source/sdk/common
    ${PROJECT_SOURCE_DIR}/include/extensions/serialization
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/serialization
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/serialization/DTO
    ${PROJECT_SOURCE_DIR}/include/lowlevel
    ${PROJECT_SOURCE_DIR}/include/toolkit
    ${PROJECT_SOURCE_DIR}/source/sdk/lowlevel
    ${PROJECT_SOURCE_DIR}/include/globals
    ${PROJECT_SOURCE_DIR}/dependencies/shared/CapnProto/src
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/serialization/generated
    ${PROJECT_SOURCE_DIR}/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/source/shared/NsFoundation/include
    ${PROJECT_SOURCE_DIR}/source/shared/NsFileBuffer/include
    ${PROJECT_SOURCE_DIR}/source/shared/NvTask/include
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


target_compile_definitions(NvBlastExtTkSerialization PRIVATE 
    LOG_COMPONENT="NvBlastExtTkSerialization"
    KJ_HEADER_WARNINGS=0
)
