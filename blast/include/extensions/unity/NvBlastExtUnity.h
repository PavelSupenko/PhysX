//! @file
//!
//! @brief Defines the API for the NvBlastExtUnity blast sdk extension's for Unity

#ifndef NVBLASTEXTUNITY_H
#define NVBLASTEXTUNITY_H

#include "NvBlastGlobals.h"
#include "NvBlastExtAuthoring.h"
#include "NvBlastExtAuthoringMesh.h"
#include "NvBlastFracturer.h"
#include "NvBlastExtUnityConfigs.h"

using namespace Nv::Blast;

// Mesh operations
NV_C_API Mesh* NvBlastExtUnityCreateMesh(const NvcVec3* position, const NvcVec3* normals, const NvcVec2* uv, uint32_t verticesCount, const uint32_t* triangleIndices, uint32_t indicesCount);
NV_C_API void NvBlastExtUnityReleaseMesh(Mesh* mesh);
NV_C_API Mesh* NvBlastExtUnityCleanMesh(Mesh* mesh, NvBlastLog logFn, NvBlastLogProgress logPrgrsFn, NvBlastLogProgressStart logPrgrsStartFn, NvBlastLogProgressEnd logPrgrsEndFn);

NV_C_API void NvBlastExtUnitySetMaterialId(Mesh* mesh, const int32_t *materialIds);
NV_C_API void NvBlastExtUnitySetSmoothingGroup(Mesh* mesh, const int32_t *smoothingGroups);

NV_C_API uint32_t NvBlastExtUnityGetVerticesCount(const Mesh* mesh);
NV_C_API const Vertex* NvBlastExtUnityGetVertices(const Mesh* mesh);

NV_C_API uint32_t NvBlastExtUnityGetFacetCount(const Mesh* mesh);
NV_C_API const Facet* NvBlastExtUnityGetFacets(const Mesh* mesh);

NV_C_API uint32_t NvBlastExtUnityGetEdgesCount(const Mesh* mesh);
NV_C_API const Edge* NvBlastExtUnityGetEdges(const Mesh* mesh);

// Fracture operations
NV_C_API Fracturer* NvBlastExtUnityCreateIslandsFracturer();
NV_C_API Fracturer* NvBlastExtUnityCreateVoronoiFracturer(VoronoiConfiguration settings);
NV_C_API Fracturer* NvBlastExtUnityCreateClusteredVoronoiFracturer(ClusteredVoronoiConfiguration settings);
NV_C_API Fracturer* NvBlastExtUnityCreateSlicingFracturer(SlicingConfiguration settings);
NV_C_API Fracturer* NvBlastExtUnityCreatePlaneCutFracturer(PlaneCutConfiguration settings);
NV_C_API Fracturer* NvBlastExtUnityCreateCutOutFracturer(CutOutConfiguration settings);

NV_C_API void NvBlastExtUnityReleaseFracturer(Fracturer* fracturer);

NV_C_API void NvBlastExtUnityReleaseAuthoringResult(ConvexMeshBuilder& collisionBuilder, AuthoringResult* ar);
NV_C_API ConvexMeshBuilder* NvBlastExtUnityCreateCollisionBuilder();
NV_C_API void NvBlastExtUnityReleaseCollisionBuilder(ConvexMeshBuilder* builder);

NV_C_API AuthoringResult* NvBlastExtUnityFractureMesh(Mesh *mesh, uint32_t aggregateMaxCount, Fracturer* fracturer, ConvexMeshBuilder* collisionBuilder, NvBlastLog logFn);
NV_C_API AuthoringResult* NvBlastExtUnityFractureMeshes(Mesh **meshes, uint32_t meshesSize, const int32_t *ids, uint32_t aggregateMaxCount, Fracturer* fracturer, ConvexMeshBuilder* collisionBuilder, NvBlastLog logFn);
NV_C_API uint32_t NvBlastExtUnityGetFractureChunksCount(const AuthoringResult& aResult);
NV_C_API Mesh** NvBlastExtUnityCreateMeshes(const AuthoringResult& aResult);
NV_C_API void NvBlastExtUnityReleaseMeshesArray(Mesh** meshes);
NV_C_API void NvBlastExtUnityReleaseMesh(Mesh* mesh);

#endif // ifndef NVBLASTEXTUNITY_H