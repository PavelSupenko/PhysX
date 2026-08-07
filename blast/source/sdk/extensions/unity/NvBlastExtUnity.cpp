#include "NvBlastExtAuthoringMeshCleaner.h"
#include "NvBlastExtAuthoringFractureTool.h"
#include "NvBlastExtAuthoringBondGenerator.h"
#include "NvBlastExtAuthoring.h"
#include "NvBlastExtUnity.h"
#include "NvBlastPreprocessorInternal.h"
#include "ConvexHullMeshBuilder.h"
#include "FractureSession.h"
#include "NvBlastExtSerialization.h"
#include "NvBlastExtLlSerialization.h"
#include "NvBlastGlobals.h"

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
//
// A Fracturer is a descriptor: it records which operation to run and with what settings, and the
// session performs it. See NvBlastFracturer.h.

Fracturer* NvBlastExtUnityCreateVoronoiFracturer(VoronoiConfiguration settings)
{
    Fracturer* fracturer = new Fracturer();
    fracturer->type      = Fracturer::Voronoi;
    fracturer->voronoi   = settings;
    return fracturer;
}

Fracturer* NvBlastExtUnityCreateClusteredVoronoiFracturer(ClusteredVoronoiConfiguration settings)
{
    Fracturer* fracturer        = new Fracturer();
    fracturer->type             = Fracturer::ClusteredVoronoi;
    fracturer->clusteredVoronoi = settings;
    return fracturer;
}

Fracturer* NvBlastExtUnityCreateSlicingFracturer(SlicingConfiguration settings)
{
    Fracturer* fracturer = new Fracturer();
    fracturer->type      = Fracturer::Slicing;
    fracturer->slicing   = settings;
    return fracturer;
}

Fracturer* NvBlastExtUnityCreateIslandsFracturer()
{
    Fracturer* fracturer = new Fracturer();
    fracturer->type      = Fracturer::Islands;
    return fracturer;
}

Fracturer* NvBlastExtUnityCreatePlaneCutFracturer(PlaneCutConfiguration settings)
{
    Fracturer* fracturer = new Fracturer();
    fracturer->type      = Fracturer::PlaneCut;
    fracturer->planeCut  = settings;
    return fracturer;
}

Fracturer* NvBlastExtUnityCreateCutOutFracturer(CutOutConfiguration settings)
{
    Fracturer* fracturer = new Fracturer();
    fracturer->type      = Fracturer::CutOut;
    fracturer->cutOut    = settings;
    return fracturer;
}

void NvBlastExtUnityReleaseFracturer(Fracturer* fracturer)
{
    delete fracturer;
}

// ─── Collision builder ────────────────────────────────────────────────────────

ConvexMeshBuilder* NvBlastExtUnityCreateCollisionBuilder()
{
    return new ConvexHullMeshBuilder();
}

void NvBlastExtUnityReleaseCollisionBuilder(ConvexMeshBuilder* builder)
{
    if (builder) builder->release();
}

// ─── Asset serialization ──────────────────────────────────────────────────────

namespace
{

/**
    Owns a serialization manager for the duration of one call.

    A process-wide instance would be cheaper, but the manager and the codecs it holds are allocated
    through Blast's global allocator, which the host can swap — releasing the Tk framework does
    exactly that. An instance outliving such a swap is then freed by a different allocator than
    built it, which crashes rather than failing cleanly. Creating and releasing per call keeps the
    whole lifetime inside one allocator, and saving an asset is rare enough that the setup cost does
    not matter.
*/
ExtSerialization& serializationManager()
{
    // One manager for the process, deliberately never released.
    //
    // Creating and releasing one per call leaks or corrupts something inside the extension: after a
    // handful of create/release cycles the next serialization jumps to a bad address. A single
    // long-lived manager never enters that cycle. It is also what the manager is for — it holds a
    // codec registry, which has no reason to be rebuilt per save.
    static ExtSerialization* manager = []() {
        ExtSerialization* created = NvBlastExtSerializationCreate();
        if (created != nullptr)
        {
            // LoadSet also registers the family codecs, which need the Tk framework this extension
            // does not build. Those fail and log once; the asset codecs register fine.
            NvBlastExtLlSerializerLoadSet(*created);
        }
        return created;
    }();

    return *manager;
}

}  // namespace

uint32_t NvBlastExtUnitySerializeAsset(const NvBlastAsset* asset, void** outBuffer)
{
    if (asset == nullptr || outBuffer == nullptr)
    {
        return 0;
    }

    *outBuffer = nullptr;

    void* buffer = nullptr;
    const uint64_t size = NvBlastExtSerializationSerializeAssetIntoBuffer(buffer, serializationManager(), asset);

    if (size == 0 || buffer == nullptr)
    {
        return 0;
    }

    *outBuffer = buffer;
    return static_cast<uint32_t>(size);
}

void NvBlastExtUnityReleaseSerializedAsset(void* buffer)
{
    if (buffer != nullptr)
    {
        NVBLAST_FREE(buffer);
    }
}

NvBlastAsset* NvBlastExtUnityDeserializeAsset(const void* buffer, uint32_t size)
{
    if (buffer == nullptr || size == 0)
    {
        return nullptr;
    }

    return reinterpret_cast<NvBlastAsset*>(serializationManager().deserializeFromBuffer(buffer, size));
}

void NvBlastExtUnityReleaseAsset(NvBlastAsset* asset)
{
    if (asset != nullptr)
    {
        NVBLAST_FREE(asset);
    }
}

const NvBlastAsset* NvBlastExtUnityGetAsset(const AuthoringResult& aResult)
{
    return aResult.asset;
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
    // This is the one-shot convenience path: it drives a throwaway session so both APIs share one
    // implementation. Callers that need to keep fracturing — subdividing chunks, undoing, previewing
    // — should own a session directly, see NvBlastExtUnitySession.h.
    if (fracturer == nullptr)
    {
        NVBLASTLL_LOG_ERROR(logFn, "Fracture: no fracturer supplied");
        return nullptr;
    }

    {
        std::ostringstream oss;
        oss << "Fracturing " << meshesSize << " mesh(es)...";
        NVBLASTLL_LOG_DEBUG(logFn, oss.str().c_str());
    }

    FractureSession session(logFn);
    if (!session.isValid())
    {
        return nullptr;
    }

    if (session.setSourceMeshes(meshes, meshesSize, ids) != NvBlastExtUnitySessionResult_Success)
    {
        return nullptr;
    }
    NVBLASTLL_LOG_DEBUG(logFn, "Source meshes assigned to the session");

    for (uint32_t i = 0; i < meshesSize; ++i)
    {
        // Each source mesh became a root chunk under its own ID; fracture each one in turn.
        const int32_t chunkId = ids != nullptr ? ids[i] : static_cast<int32_t>(i);

        if (session.applyFracturer(chunkId, *fracturer, false) != NvBlastExtUnitySessionResult_Success)
        {
            std::ostringstream oss;
            oss << "Fracture: failed on chunk " << chunkId;
            NVBLASTLL_LOG_ERROR(logFn, oss.str().c_str());
            return nullptr;
        }
    }

    NVBLASTLL_LOG_DEBUG(logFn, "Finalizing...");
    AuthoringResult* result = session.finalize(collisionBuilder, aggregateMaxCount, -1);

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