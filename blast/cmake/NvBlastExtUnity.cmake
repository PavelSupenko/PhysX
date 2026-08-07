set(NV_BLAST_EXT_UNITY_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/unity/NvBlastExtUnity.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/unity/NvBlastExtUnitySession.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/unity/FractureSession.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/unity/ConvexHullMeshBuilder.cpp
)

set(NV_BLAST_EXT_UNITY_INCLUDES
    ${PROJECT_SOURCE_DIR}/include/extensions/authoringCommon
    ${PROJECT_SOURCE_DIR}/include/extensions/authoring
    ${PROJECT_SOURCE_DIR}/include/extensions/unity
    ${PROJECT_SOURCE_DIR}/include/shared/NvFoundation
    ${PROJECT_SOURCE_DIR}/include/lowlevel
    ${PROJECT_SOURCE_DIR}/include/globals
    ${PROJECT_SOURCE_DIR}/include/extensions/assetutils
    ${PROJECT_SOURCE_DIR}/include/extensions/serialization
    ${PROJECT_SOURCE_DIR}/source/sdk/common
    # btConvexHullComputer — the quickhull already compiled into NvBlastExtAuthoring as part of
    # V-HACD, which ConvexHullMeshBuilder uses to build real hulls without pulling in PhysX.
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/authoring/VHACD/inc
)

add_library(NvBlastExtUnity ${LIBRARIES_TYPE}
    ${NV_BLAST_EXT_UNITY_SOURCE}
)

target_include_directories(NvBlastExtUnity PRIVATE
    ${NV_BLAST_EXT_UNITY_INCLUDES}
)

target_link_libraries(NvBlastExtUnity
    NvBlast
    NvBlastGlobals
    NvBlastExtAuthoring
    # NvBlastExtAssetUtilsAddExternalBonds — anchors static chunks to the world at finalize time.
    NvBlastExtAssetUtils
    # Serializing the authored asset, so the runtime has something to load.
    NvBlastExtSerialization
)

target_compile_definitions(NvBlastExtUnity PRIVATE 
        LOG_COMPONENT="NvBlastExtUnity"
)
