#include "NvBlastExtAuthoringMeshCleaner.h"
#include "NvBlastExtAuthoringFractureTool.h"
#include "NvBlastExtAuthoringBondGenerator.h"
#include "NvBlastExtAuthoring.h"
#include "SimpleRandomGenerator.h"
#include "NvBlastExtUnity.h"
#include "NvBlastPreprocessorInternal.h"
#include "BoundingBoxConvexMeshBuilder.h"

#include "VoronoiFracturer.h"
#include "IslandsFracturer.h"
#include "ClusteredVoronoiFracturer.h"
#include "SlicingFracturer.h"
#include "PlaneCutFracturer.h"
#include "CutOutFracturer.h"

#include <sstream>

using namespace Nv::Blast;

// ─── Mesh creation and release ────────────────────────────────────────────────

Mesh* NvBlastExtUnityCreateMesh(const NvcVec3* position, const NvcVec3* normals, const NvcVec2* uv,
    uint32_t verticesCount, const uint32_t* triangleIndices, uint32_t indicesCount)
{
    return NvBlastExtAuthoringCreateMesh(position, normals, uv, verticesCount, triangleIndices, indicesCount);
}

void NvBlastExtUnityReleaseMesh(Mesh* mesh)
{
    if (mesh) mesh->release();
}

void NvBlastExtUnitySetMaterialId(Mesh* mesh, const int32_t* materialIds)
{
    mesh->setMaterialId(materialIds);
}

void NvBlastExtUnitySetSmoothingGroup(Mesh* mesh, const int32_t* smoothingGroups)
{
    mesh->setSmoothingGroup(smoothingGroups);
}

// ─── Mesh data accessors ──────────────────────────────────────────────────────

uint32_t NvBlastExtUnityGetVerticesCount(const Mesh* mesh)
{
    return mesh->getVerticesCount();
}

const Vertex* NvBlastExtUnityGetVertices(const Mesh* mesh)
{
    return mesh->getVertices();
}

uint32_t NvBlastExtUnityGetFacetCount(const Mesh* mesh)
{
    return mesh->getFacetCount();
}

const Facet* NvBlastExtUnityGetFacets(const Mesh* mesh)
{
    return mesh->getFacetsBuffer();
}

uint32_t NvBlastExtUnityGetEdgesCount(const Mesh* mesh)
{
    return mesh->getEdgesCount();
}

const Edge* NvBlastExtUnityGetEdges(const Mesh* mesh)
{
    return mesh->getEdges();
}

// ─── Mesh cleaning ────────────────────────────────────────────────────────────

Mesh* NvBlastExtUnityCleanMesh(Mesh* mesh,
    NvBlastLog logFn, NvBlastLogProgress logPrgrsFn,
    NvBlastLogProgressStart logPrgrsStartFn, NvBlastLogProgressEnd logPrgrsEndFn)
{
    MeshCleaner* cleaner = NvBlastExtAuthoringCreateMeshCleaner();
    Mesh* cleanedMesh = cleaner->cleanMesh(mesh, logFn, logPrgrsFn, logPrgrsStartFn, logPrgrsEndFn);
    cleaner->release();

    // Original mesh is always released here — the C# side uses DetachPointer() to
    // ensure it does not call ReleaseMesh on the same pointer afterwards.
    mesh->release();

    return cleanedMesh;
}

// ─── Fracturers ───────────────────────────────────────────────────────────────

Fracturer* NvBlastExtUnityCreateVoronoiFracturer(VoronoiConfiguration settings)
{
    return new VoronoiFracturer(settings);
}

Fracturer* NvBlastExtUnityCreateClusteredVoronoiFracturer(ClusteredVoronoiConfiguration settings)
{
    return new ClusteredVoronoiFracturer(settings);
}

Fracturer* NvBlastExtUnityCreateSlicingFracturer(SlicingConfiguration settings)
{
    return new SlicingFracturer(settings);
}

Fracturer* NvBlastExtUnityCreateIslandsFracturer()
{
    return new IslandsFracturer();
}

Fracturer* NvBlastExtUnityCreatePlaneCutFracturer(PlaneCutConfiguration settings)
{
    return new PlaneCutFracturer(settings);
}

Fracturer* NvBlastExtUnityCreateCutOutFracturer(CutOutConfiguration settings)
{
    return new CutOutFracturer(settings);
}

void NvBlastExtUnityReleaseFracturer(Fracturer* fracturer)
{
    delete fracturer;
}

// ─── Collision builder ────────────────────────────────────────────────────────

ConvexMeshBuilder* NvBlastExtUnityCreateCollisionBuilder()
{
    return new BoundingBoxConvexMeshBuilder();
}

void NvBlastExtUnityReleaseCollisionBuilder(ConvexMeshBuilder* builder)
{
    if (builder) builder->release();
}

// ─── Fracture pipeline ────────────────────────────────────────────────────────

AuthoringResult* NvBlastExtUnityFractureMesh(Mesh* mesh, uint32_t aggregateMaxCount,
    Fracturer* fracturer, ConvexMeshBuilder* collisionBuilder, NvBlastLog logFn)
{
    Mesh* meshes[] = { mesh };
    int32_t ids[]  = { 0 };
    return NvBlastExtUnityFractureMeshes(meshes, 1, ids, aggregateMaxCount, fracturer, collisionBuilder, logFn);
}

AuthoringResult* NvBlastExtUnityFractureMeshes(Mesh** meshes, uint32_t meshesSize, const int32_t* ids,
    uint32_t aggregateMaxCount, Fracturer* fracturer, ConvexMeshBuilder* collisionBuilder, NvBlastLog logFn)
{
    {
        std::ostringstream oss;
        oss << "Fracturing " << meshesSize << " mesh(es)...";
        NVBLASTLL_LOG_DEBUG(logFn, oss.str().c_str());
    }

    FractureTool*       fTool         = NvBlastExtAuthoringCreateFractureTool();
    BlastBondGenerator* bondGenerator = NvBlastExtAuthoringCreateBondGenerator(collisionBuilder);

    ConvexDecompositionParams collisionParams;
    collisionParams.maximumNumberOfHulls = aggregateMaxCount > 0 ? aggregateMaxCount : 1;
    collisionParams.voxelGridResolution  = 0;

    SimpleRandomGenerator rng;
    rng.seed(0);

    fTool->setSourceMeshes(meshes, meshesSize, ids);
    NVBLASTLL_LOG_DEBUG(logFn, "Source meshes assigned to FractureTool");

    for (uint32_t i = 0; i < meshesSize; ++i)
    {
        Mesh*    mesh = meshes[i];
        int32_t  id   = ids[i];

        if (mesh == nullptr)
        {
            std::ostringstream oss;
            oss << "Mesh with id " << id << " is null — skipping";
            NVBLASTLL_LOG_ERROR(logFn, oss.str().c_str());
            continue;
        }

        VoronoiSitesGenerator* sitesGen = NvBlastExtAuthoringCreateVoronoiSitesGenerator(mesh, &rng);
        if (sitesGen == nullptr)
        {
            NVBLASTLL_LOG_ERROR(logFn, "Failed to create VoronoiSitesGenerator");
            bondGenerator->release();
            fTool->release();
            return nullptr;
        }

        if (!fracturer->fracture(fTool, sitesGen, &rng, id, logFn))
        {
            NVBLASTLL_LOG_ERROR(logFn, "fracturer->fracture() failed");
            sitesGen->release();
            bondGenerator->release();
            fTool->release();
            return nullptr;
        }

        sitesGen->release();
    }

    NVBLASTLL_LOG_DEBUG(logFn, "Running NvBlastExtAuthoringProcessFracture...");
    AuthoringResult* result = NvBlastExtAuthoringProcessFracture(
        *fTool, *bondGenerator, *collisionBuilder, collisionParams);

    bondGenerator->release();
    fTool->release();

    NVBLASTLL_LOG_DEBUG(logFn, "Fracture complete");
    return result;
}

// ─── Authoring result helpers ─────────────────────────────────────────────────

uint32_t NvBlastExtUnityGetFractureChunksCount(const AuthoringResult& aResult)
{
    return aResult.chunkCount;
}

Mesh** NvBlastExtUnityCreateMeshes(const AuthoringResult& aResult)
{
    if (aResult.chunkCount == 0) return nullptr;

    uint32_t meshCount = aResult.chunkCount;
    Mesh** meshes = new Mesh*[meshCount];

    for (uint32_t i = 0; i < meshCount; ++i)
    {
        // Each chunk occupies the range [geometryOffset[i], geometryOffset[i+1]) in the geometry array
        uint32_t start         = aResult.geometryOffset[i];
        uint32_t end           = aResult.geometryOffset[i + 1];
        uint32_t triangleCount = end - start;
        uint32_t vertexCount   = triangleCount * 3;   // 3 unique vertex slots per triangle

        NvcVec3*  positions = new NvcVec3[vertexCount];
        NvcVec3*  normals   = new NvcVec3[vertexCount];
        NvcVec2*  uvs       = new NvcVec2[vertexCount];
        uint32_t* indices   = new uint32_t[vertexCount];
        int32_t*  materials = new int32_t[triangleCount];

        for (uint32_t t = 0; t < triangleCount; ++t)
        {
            const Triangle& tri = aResult.geometry[start + t];

            uint32_t a = t * 3, b = t * 3 + 1, c = t * 3 + 2;

            positions[a] = tri.a.p;  normals[a] = tri.a.n;  uvs[a] = tri.a.uv[0];  indices[a] = a;
            positions[b] = tri.b.p;  normals[b] = tri.b.n;  uvs[b] = tri.b.uv[0];  indices[b] = b;
            positions[c] = tri.c.p;  normals[c] = tri.c.n;  uvs[c] = tri.c.uv[0];  indices[c] = c;

            materials[t] = tri.materialId;
        }

        // NvBlastExtAuthoringCreateMesh copies all data into the new Mesh object
        Mesh* mesh = NvBlastExtAuthoringCreateMesh(positions, normals, uvs, vertexCount, indices, vertexCount);
        mesh->setMaterialId(materials);
        meshes[i] = mesh;

        delete[] positions;
        delete[] normals;
        delete[] uvs;
        delete[] indices;
        delete[] materials;
    }

    return meshes;
}

void NvBlastExtUnityReleaseMeshesArray(Mesh** meshes)
{
    // Releases only the pointer array itself.
    // Each individual Mesh* must already have been released via NvBlastExtUnityReleaseMesh.
    delete[] meshes;
}

void NvBlastExtUnityReleaseAuthoringResult(ConvexMeshBuilder& collisionBuilder, AuthoringResult* ar)
{
    NvBlastExtAuthoringReleaseAuthoringResult(collisionBuilder, ar);
}