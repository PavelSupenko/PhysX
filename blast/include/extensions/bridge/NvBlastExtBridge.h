//! @file
//!
//! @brief Defines the API for the NvBlastExtBridge blast sdk extension — the flat C surface
//!        engine integrations bind to
//!
//! Mesh construction, collision-builder lifetime and asset serialization live here — the pieces an
//! integration needs on either side of an authoring session. Fracturing itself is the session's
//! job; see NvBlastExtBridgeSession.h.

#ifndef NVBLASTEXTBRIDGE_H
#define NVBLASTEXTBRIDGE_H

#include "NvBlastGlobals.h"
#include "NvBlastExtAuthoring.h"
#include "NvBlastExtAuthoringMesh.h"
#include "NvBlastExtBridgeConfigs.h"

using namespace Nv::Blast;

// Mesh operations
NV_C_API Mesh* NvBlastExtBridgeCreateMesh(const NvcVec3* position, const NvcVec3* normals, const NvcVec2* uv, uint32_t verticesCount, const uint32_t* triangleIndices, uint32_t indicesCount);
NV_C_API void NvBlastExtBridgeReleaseMesh(Mesh* mesh);
NV_C_API Mesh* NvBlastExtBridgeCleanMesh(Mesh* mesh, NvBlastLog logFn, NvBlastLogProgress logPrgrsFn, NvBlastLogProgressStart logPrgrsStartFn, NvBlastLogProgressEnd logPrgrsEndFn);

NV_C_API void NvBlastExtBridgeSetMaterialId(Mesh* mesh, const int32_t *materialIds);
NV_C_API void NvBlastExtBridgeSetSmoothingGroup(Mesh* mesh, const int32_t *smoothingGroups);

NV_C_API uint32_t NvBlastExtBridgeGetVerticesCount(const Mesh* mesh);
NV_C_API const Vertex* NvBlastExtBridgeGetVertices(const Mesh* mesh);

NV_C_API uint32_t NvBlastExtBridgeGetFacetCount(const Mesh* mesh);
NV_C_API const Facet* NvBlastExtBridgeGetFacets(const Mesh* mesh);

NV_C_API uint32_t NvBlastExtBridgeGetEdgesCount(const Mesh* mesh);
NV_C_API const Edge* NvBlastExtBridgeGetEdges(const Mesh* mesh);

NV_C_API void NvBlastExtBridgeReleaseAuthoringResult(ConvexMeshBuilder& collisionBuilder, AuthoringResult* ar);
NV_C_API ConvexMeshBuilder* NvBlastExtBridgeCreateCollisionBuilder();
NV_C_API void NvBlastExtBridgeReleaseCollisionBuilder(ConvexMeshBuilder* builder);

// Asset serialization
//
// The NvBlastAsset produced by fracturing is freed along with its AuthoringResult, so without
// serializing it the whole authoring result — support graph, bonds, anchors — is lost the moment
// the result is released, and there is nothing left for the runtime to load.

/**
    Serializes a Blast asset into a newly allocated buffer.

    \param[in]  asset       Asset to serialize.
    \param[out] outBuffer   Receives the buffer; release it with NvBlastExtBridgeReleaseSerializedAsset.
    \return Number of bytes written, or 0 on failure.
*/
NV_C_API uint32_t NvBlastExtBridgeSerializeAsset(const NvBlastAsset* asset, void** outBuffer);

/**
    Releases a buffer returned by NvBlastExtBridgeSerializeAsset.
*/
NV_C_API void NvBlastExtBridgeReleaseSerializedAsset(void* buffer);

/**
    Rebuilds an asset from a buffer produced by NvBlastExtBridgeSerializeAsset.
    \return The asset, owned by the caller — release it with NvBlastExtBridgeReleaseAsset. Null on failure.
*/
NV_C_API NvBlastAsset* NvBlastExtBridgeDeserializeAsset(const void* buffer, uint32_t size);

/**
    Releases an asset returned by NvBlastExtBridgeDeserializeAsset.
*/
NV_C_API void NvBlastExtBridgeReleaseAsset(NvBlastAsset* asset);

/**
    Returns the asset held by an authoring result, so it can be serialized before the result is released.
*/
NV_C_API const NvBlastAsset* NvBlastExtBridgeGetAsset(const AuthoringResult& aResult);

// Authoring result inspection
NV_C_API uint32_t NvBlastExtBridgeGetFractureChunksCount(const AuthoringResult& aResult);
NV_C_API Mesh** NvBlastExtBridgeCreateMeshes(const AuthoringResult& aResult);
NV_C_API void NvBlastExtBridgeReleaseMeshesArray(Mesh** meshes);

#endif // ifndef NVBLASTEXTBRIDGE_H