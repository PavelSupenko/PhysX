set(LIBRARIES_TYPE SHARED)

## === General === ##

set(COMMON_SOURCES
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common/NvBlastAssert.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common/NvBlastAtomic.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common/NvBlastTime.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common/NvBlastTimers.cpp
)

## === NvBlast === ##

set(NV_BLAST_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel/NvBlastActor.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel/NvBlastActorSerializationBlock.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel/NvBlastAsset.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel/NvBlastAssetHelper.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel/NvBlastFamily.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel/NvBlastFamilyGraph.cpp
)

set(NV_BLAST_INCLUDES
    ${PROJECT_SOURCE_DIR}/blast/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/blast/include/lowlevel
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common
)

add_library(NvBlast ${LIBRARIES_TYPE}
    ${NV_BLAST_SOURCE}
)

target_include_directories(NvBlast PRIVATE
    ${NV_BLAST_INCLUDES}
)

## === NvBlastGlobals === ##

set(NV_BLAST_GLOBALS_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/globals/NvBlastGlobals.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/globals/NvBlastInternalProfiler.cpp
)

set(NV_BLAST_GLOBALS_INCLUDES
    ${PROJECT_SOURCE_DIR}/blast/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/blast/include/lowlevel
    ${PROJECT_SOURCE_DIR}/blast/include/globals
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common
    ${PROJECT_SOURCE_DIR}/blast/source/shared/NsFoundation/include
)

add_library(NvBlastGlobals ${LIBRARIES_TYPE}
    ${NV_BLAST_GLOBALS_SOURCE}
)

target_include_directories(NvBlastGlobals PRIVATE
    ${NV_BLAST_GLOBALS_INCLUDES}
)

## === NvBlastExtAssetUtils === ##

set(NV_BLAST_EXT_ASSET_UTILS_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/assetutils/NvBlastExtAssetUtils.cpp
)

set(NV_BLAST_EXT_ASSET_UTILS_INCLUDES
    ${PROJECT_SOURCE_DIR}/blast/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/blast/include/lowlevel
    ${PROJECT_SOURCE_DIR}/blast/include/globals
    ${PROJECT_SOURCE_DIR}/blast/include/extensions/assetutils
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common
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

## === NvBlastExtAuthoring === ##

set(NV_BLAST_EXT_AUTHORING_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common/NvBlastAssert.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common/NvBlastAtomic.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common/NvBlastTime.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common/NvBlastTimers.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/NvBlastExtApexSharedParts.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/NvBlastExtAuthoring.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/NvBlastExtAuthoringBondGeneratorImpl.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/NvBlastExtAuthoringBooleanToolImpl.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/NvBlastExtAuthoringCollisionBuilderImpl.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/NvBlastExtAuthoringCutoutImpl.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/NvBlastExtAuthoringFractureToolImpl.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/NvBlastExtAuthoringMeshCleanerImpl.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/NvBlastExtAuthoringMeshNoiser.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/NvBlastExtAuthoringMeshUtils.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/NvBlastExtAuthoringPatternGeneratorImpl.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/NvBlastExtAuthoringTriangulator.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/NvBlastExtTriangleProcessor.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/VHACD/src/FloatMath.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/VHACD/src/VHACD-ASYNC.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/VHACD/src/VHACD.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/VHACD/src/btAlignedAllocator.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/VHACD/src/btConvexHullComputer.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/VHACD/src/vhacdICHull.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/VHACD/src/vhacdManifoldMesh.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/VHACD/src/vhacdMesh.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/VHACD/src/vhacdRaycastMesh.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/VHACD/src/vhacdVolume.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoringCommon/NvBlastExtAuthoringAcceleratorImpl.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoringCommon/NvBlastExtAuthoringMeshImpl.cpp
)

set(NV_BLAST_EXT_AUTHORING_INCLUDES
    ${PROJECT_SOURCE_DIR}/blast/include
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common
    ${PROJECT_SOURCE_DIR}/blast/include/extensions/authoring
    ${PROJECT_SOURCE_DIR}/blast/include/lowlevel
    ${PROJECT_SOURCE_DIR}/blast/include/globals
    ${PROJECT_SOURCE_DIR}/blast/include/extensions/assetutils
    ${PROJECT_SOURCE_DIR}/blast/include/extensions/authoringCommon
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoringCommon
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/VHACD/inc
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/authoring/VHACD/public
    ${PROJECT_SOURCE_DIR}/blast/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/blast/source/shared/NsFoundation/include
    ${PROJECT_SOURCE_DIR}/blast/_build/target-deps/BoostMultiprecision
    ${PROJECT_SOURCE_DIR}/blast/_build/target-deps/BoostMultiprecision
    ${PROJECT_SOURCE_DIR}/blast/_build/target-deps

    # std::roundf and std::max used from '#include <algorithm>'
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/msvc/VC/Tools/MSVC/14.16.27023/include
)

add_library(NvBlastExtAuthoring ${LIBRARIES_TYPE}
    ${NV_BLAST_EXT_AUTHORING_SOURCE}
)

target_include_directories(NvBlastExtAuthoring PRIVATE
    ${NV_BLAST_EXT_AUTHORING_INCLUDES}
)

target_link_libraries(NvBlastExtAuthoring
    NvBlast
    NvBlastGlobals
)


## === NvBlastExtSerialization === ##

set(NV_BLAST_EXT_SERIALIZATION_SOURCE
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
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/DTO/ActorDTO.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/DTO/AssetDTO.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/DTO/FamilyDTO.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/DTO/FamilyGraphDTO.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/DTO/NvBlastBondDTO.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/DTO/NvBlastChunkDTO.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/DTO/NvBlastIDDTO.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/NvBlastExtInputStream.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/NvBlastExtLlSerialization.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/NvBlastExtOutputStream.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/NvBlastExtSerialization.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/generated/NvBlastExtLlSerialization-capn.c++

)

set(NV_BLAST_EXT_SERIALIZATION_INCLUDES
    ${PROJECT_SOURCE_DIR}/blast/include
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common
    ${PROJECT_SOURCE_DIR}/blast/include/extensions/serialization
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/DTO
    ${PROJECT_SOURCE_DIR}/blast/include/lowlevel
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel
    ${PROJECT_SOURCE_DIR}/blast/include/globals
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/CapnProto/src
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization/generated
    ${PROJECT_SOURCE_DIR}/blast/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/blast/source/shared/NsFoundation/include
    ${PROJECT_SOURCE_DIR}/blast/source/shared/NsFileBuffer/include
    ${PROJECT_SOURCE_DIR}/blast/source/shared/NvTask/include

    # include <typeinfo.h> used
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/msvc/VC/Tools/MSVC/14.16.27023/include
)

add_library(NvBlastExtSerialization ${LIBRARIES_TYPE}
    ${NV_BLAST_EXT_SERIALIZATION_SOURCE}
)

target_include_directories(NvBlastExtSerialization PRIVATE
    ${NV_BLAST_EXT_SERIALIZATION_INCLUDES}
)

target_link_libraries(NvBlastExtSerialization
    NvBlast
    NvBlastGlobals
)

## === NvBlastExtShaders === ##

set(NV_BLAST_EXT_SHADERS_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/shaders/NvBlastExtDamageAcceleratorAABBTree.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/shaders/NvBlastExtDamageAccelerators.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/shaders/NvBlastExtDamageShaders.cpp

)

set(NV_BLAST_EXT_SHADERS_INCLUDES
    ${PROJECT_SOURCE_DIR}/blast/include
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common
    ${PROJECT_SOURCE_DIR}/blast/include/extensions/shaders
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/shaders
    ${PROJECT_SOURCE_DIR}/blast/include/lowlevel
    ${PROJECT_SOURCE_DIR}/blast/include/globals
    ${PROJECT_SOURCE_DIR}/blast/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/blast/source/shared/NsFoundation/include

    # include <typeinfo.h> used
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/msvc/VC/Tools/MSVC/14.16.27023/include
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

## === NvBlastExtStress === ##

set(NV_BLAST_EXT_STRESS_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/stress/NvBlastExtStressSolver.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/shared/stress_solver/stress.cpp
)

set(NV_BLAST_EXT_STRESS_INCLUDES
    ${PROJECT_SOURCE_DIR}/blast/include
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common
    ${PROJECT_SOURCE_DIR}/blast/include/extensions/stress
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/stress
    ${PROJECT_SOURCE_DIR}/blast/include/lowlevel
    ${PROJECT_SOURCE_DIR}/blast/include/globals
    ${PROJECT_SOURCE_DIR}/blast/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/blast/source/shared/NsFoundation/include
    ${PROJECT_SOURCE_DIR}/blast/source/shared/stress_solver
    
    # include <typeinfo.h> used
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/msvc/VC/Tools/MSVC/14.16.27023/include
)

add_library(NvBlastExtStress ${LIBRARIES_TYPE}
    ${NV_BLAST_EXT_STRESS_SOURCE}
)

target_include_directories(NvBlastExtStress PRIVATE
    ${NV_BLAST_EXT_STRESS_INCLUDES}
)

target_link_libraries(NvBlastExtStress
    NvBlast
    NvBlastGlobals
)

## === NvBlastTk === ##

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

## === NvBlastExtTkSerialization === ##

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

## === UnitTests === ##

set(TESTS_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel/NvBlastActor.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel/NvBlastActorSerializationBlock.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel/NvBlastAsset.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel/NvBlastFamily.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel/NvBlastFamilyGraph.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/toolkit/NvBlastTkTaskManager.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/shared/utils/AssetGenerator.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/unit/APITests.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/unit/ActorTests.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/unit/AssetTests.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/unit/CoreTests.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/unit/FamilyGraphTests.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/unit/MultithreadingTests.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/unit/TkCompositeTests.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/unit/TkTests.cpp
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/utils/TestAssets.cpp
)

set(TESTS_INCLUDES
    ${PROJECT_SOURCE_DIR}/blast/include
    ${PROJECT_SOURCE_DIR}/blast/include/globals
    ${PROJECT_SOURCE_DIR}/blast/include/lowlevel
    ${PROJECT_SOURCE_DIR}/blast/include/toolkit
    ${PROJECT_SOURCE_DIR}/blast/include/extensions/assetutils
    ${PROJECT_SOURCE_DIR}/blast/include/extensions/shaders
    ${PROJECT_SOURCE_DIR}/blast/include/extensions/serialization
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/common
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/globals
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/lowlevel
    ${PROJECT_SOURCE_DIR}/blast/source/sdk/extensions/serialization
    ${PROJECT_SOURCE_DIR}/blast/source/test/src
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/unit
    ${PROJECT_SOURCE_DIR}/blast/source/test/src/utils
    ${PROJECT_SOURCE_DIR}/blast/source/shared/filebuf/include
    ${PROJECT_SOURCE_DIR}/blast/source/shared/utils
    ${PROJECT_SOURCE_DIR}/blast/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/blast/source/shared/NsFoundation/include
    ${PROJECT_SOURCE_DIR}/blast/source/shared/NsFileBuffer/include
    ${PROJECT_SOURCE_DIR}/blast/source/shared/NvTask/include

    ${PROJECT_SOURCE_DIR}/blast/_build/target-deps/googletest/include
    ${PROJECT_SOURCE_DIR}/blast/_build/host-deps/msvc/VC/Tools/MSVC/14.16.27023/include
)

add_executable(UnitTests
    ${TESTS_SOURCE}
)

target_include_directories(UnitTests PRIVATE
    ${TESTS_INCLUDES}
)

target_link_libraries(UnitTests
    NvBlast
    NvBlastTk
    NvBlastGlobals
    NvBlastExtAssetUtils
    NvBlastExtShaders
    NvBlastExtSerialization
    NvBlastExtTkSerialization
)

target_link_directories(UnitTests BEFORE PRIVATE
    # use Release for release builds of libs because it causes error LNK2038: mismatch detected for '_ITERATOR_DEBUG_LEVEL'
    ${PROJECT_SOURCE_DIR}/blast/_build/target-deps/googletest/lib/vc14win64-cmake/Release
)

target_link_libraries(UnitTests
    gtest_main.lib
    gtest.lib
)
