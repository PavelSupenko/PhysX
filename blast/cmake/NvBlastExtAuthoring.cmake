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

if(NV_CONFIGURATION_TYPE STREQUAL DEBUG)
    target_compile_definitions(NvBlastExtAuthoring PRIVATE 
        LOG_COMPONENT="NvBlastExtAuthoring"
        _CRT_NONSTDC_NO_DEPRECATE
        BOOST_USE_WINDOWS_H=1
        _DEBUG
        CARB_DEBUG=1
    )
else()
    target_compile_definitions(NvBlastExtAuthoring PRIVATE 
        LOG_COMPONENT="NvBlastExtAuthoring"
        _CRT_NONSTDC_NO_DEPRECATE
        BOOST_USE_WINDOWS_H=1
        NDEBUG
        CARB_DEBUG=0
)
endif()