//! @file
//!
//! @brief Defines the API for the NvBlastExtUnity blast sdk extension's for Unity

#ifndef NVBLASTEXTUNITY_H
#define NVBLASTEXTUNITY_H

#include "NvBlastGlobals.h"
#include "NvBlastExtAuthoring.h"
#include "NvBlastExtAuthoringMesh.h"
#include "NvBlastFracturer.h"

using namespace Nv::Blast;

// Mesh operations
NV_C_API Mesh* NvBlastExtUnityCreateMesh(const NvcVec3* position, const NvcVec3* normals, const NvcVec2* uv, uint32_t verticesCount, const uint32_t* triangleIndices, uint32_t indicesCount);
NV_C_API void NvBlastExtUnityReleaseMesh(Mesh* mesh);
NV_C_API Mesh* NvBlastExtUnityCleanMesh(Mesh* mesh);

NV_C_API uint32_t NvBlastExtUnityGetVerticesCount(const Mesh* mesh);
NV_C_API const Vertex* NvBlastExtUnityGetVertices(const Mesh* mesh);

NV_C_API uint32_t NvBlastExtUnityGetFacetCount(const Mesh* mesh);
NV_C_API const Facet* NvBlastExtUnityGetFacets(const Mesh* mesh);

NV_C_API uint32_t NvBlastExtUnityGetEdgesCount(const Mesh* mesh);
NV_C_API const Edge* NvBlastExtUnityGetEdges(const Mesh* mesh);

// Fracture operations
NV_C_API Fracturer* NvBlastExtUnityCreateIslandsFracturer();
NV_C_API Fracturer* NvBlastExtUnityCreateVoronoiFracturer(u_int32_t cellsCount);
NV_C_API Fracturer* NvBlastExtUnityCreateClusteredVoronoiFracturer(uint32_t cellsCount, uint32_t clusterCount, float clusterRad);
NV_C_API Fracturer* NvBlastExtUnityCreateSlicingFracturer(int32_t x_slices, int32_t y_slices, int32_t z_slices, float angleVariation, float offsetVariation);

NV_C_API AuthoringResult* NvBlastExtUnityFractureMesh(Mesh *mesh, uint32_t aggregateMaxCount, Fracturer* fracturer, NvBlastLog logFn);
NV_C_API AuthoringResult* NvBlastExtUnityFractureMeshes(Mesh **meshes, uint32_t meshesSize, const int32_t *ids, uint32_t aggregateMaxCount, Fracturer* fracturer, NvBlastLog logFn);
NV_C_API uint32_t NvBlastExtUnityGetFractureChunksCount(const AuthoringResult& aResult);
NV_C_API Mesh** NvBlastExtUnityCreateMeshes(const AuthoringResult& aResult);

#endif // ifndef NVBLASTEXTUNITY_H