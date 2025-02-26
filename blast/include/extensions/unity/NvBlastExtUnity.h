//! @file
//!
//! @brief Defines the API for the NvBlastExtUnity blast sdk extension's for Unity

#ifndef NVBLASTEXTUNITY_H
#define NVBLASTEXTUNITY_H

#include "NvBlastGlobals.h"
#include "NvBlastExtAuthoring.h"
#include "NvBlastExtAuthoringMesh.h"

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
NV_C_API AuthoringResult* NvBlastExtUnityVoronoiFractureMesh(Mesh *mesh, u_int32_t cellsCount, uint32_t aggregateMaxCount, NvBlastLog logFn);
NV_C_API uint32_t NvBlastExtUnityGetFractureChunksCount(const AuthoringResult& aResult);
NV_C_API Mesh** NvBlastExtUnityCreateMeshes(const AuthoringResult& aResult);

#endif // ifndef NVBLASTEXTUNITY_H