set(NV_BLAST_EXT_BRIDGE_SOURCE
    ${COMMON_SOURCES}
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/bridge/NvBlastExtBridge.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/bridge/NvBlastExtBridgeSession.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/bridge/FractureSession.cpp
    ${PROJECT_SOURCE_DIR}/source/sdk/extensions/bridge/ConvexHullMeshBuilder.cpp
)

set(NV_BLAST_EXT_BRIDGE_INCLUDES
    ${PROJECT_SOURCE_DIR}/include/extensions/authoringCommon
    ${PROJECT_SOURCE_DIR}/include/extensions/authoring
    ${PROJECT_SOURCE_DIR}/include/extensions/bridge
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

add_library(NvBlastExtBridge ${LIBRARIES_TYPE}
    ${NV_BLAST_EXT_BRIDGE_SOURCE}
)

target_include_directories(NvBlastExtBridge PRIVATE
    ${NV_BLAST_EXT_BRIDGE_INCLUDES}
)

target_link_libraries(NvBlastExtBridge
    NvBlast
    NvBlastGlobals
    NvBlastExtAuthoring
    # NvBlastExtAssetUtilsAddExternalBonds — anchors static chunks to the world at finalize time.
    NvBlastExtAssetUtils
    # Serializing the authored asset, so the runtime has something to load.
    NvBlastExtSerialization
)

target_compile_definitions(NvBlastExtBridge PRIVATE 
        LOG_COMPONENT="NvBlastExtBridge"
)
