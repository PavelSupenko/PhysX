#include "NvBlastExtAuthoringMeshCleaner.h"
#include "NvBlastExtAuthoringFractureTool.h"
#include "NvBlastExtAuthoringBondGenerator.h"
#include "NvBlastExtAuthoring.h"
#include "NvBlastExtBridge.h"
#include "NvBlastPreprocessorInternal.h"
#include "ConvexHullMeshBuilder.h"
#include "NvBlastExtSerialization.h"
#include "NvBlastExtLlSerialization.h"
#include "NvBlastGlobals.h"

using namespace Nv::Blast;

// ─── Mesh creation and release ────────────────────────────────────────────────

Mesh* NvBlastExtBridgeCreateMesh(const NvcVec3* position, const NvcVec3* normals, const NvcVec2* uv,
    uint32_t verticesCount, const uint32_t* triangleIndices, uint32_t indicesCount)
{
    return NvBlastExtAuthoringCreateMesh(position, normals, uv, verticesCount, triangleIndices, indicesCount);
}

void NvBlastExtBridgeReleaseMesh(Mesh* mesh)
{
    if (mesh) mesh->release();
}

void NvBlastExtBridgeSetMaterialId(Mesh* mesh, const int32_t* materialIds)
{
    mesh->setMaterialId(materialIds);
}

void NvBlastExtBridgeSetSmoothingGroup(Mesh* mesh, const int32_t* smoothingGroups)
{
    mesh->setSmoothingGroup(smoothingGroups);
}

// ─── Mesh data accessors ──────────────────────────────────────────────────────

uint32_t NvBlastExtBridgeGetVerticesCount(const Mesh* mesh)
{
    return mesh->getVerticesCount();
}

const Vertex* NvBlastExtBridgeGetVertices(const Mesh* mesh)
{
    return mesh->getVertices();
}

uint32_t NvBlastExtBridgeGetFacetCount(const Mesh* mesh)
{
    return mesh->getFacetCount();
}

const Facet* NvBlastExtBridgeGetFacets(const Mesh* mesh)
{
    return mesh->getFacetsBuffer();
}

uint32_t NvBlastExtBridgeGetEdgesCount(const Mesh* mesh)
{
    return mesh->getEdgesCount();
}

const Edge* NvBlastExtBridgeGetEdges(const Mesh* mesh)
{
    return mesh->getEdges();
}

// ─── Mesh cleaning ────────────────────────────────────────────────────────────

Mesh* NvBlastExtBridgeCleanMesh(Mesh* mesh,
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

// ─── Collision builder ────────────────────────────────────────────────────────

ConvexMeshBuilder* NvBlastExtBridgeCreateCollisionBuilder()
{
    return new ConvexHullMeshBuilder();
}

void NvBlastExtBridgeReleaseCollisionBuilder(ConvexMeshBuilder* builder)
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

uint32_t NvBlastExtBridgeSerializeAsset(const NvBlastAsset* asset, void** outBuffer)
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

void NvBlastExtBridgeReleaseSerializedAsset(void* buffer)
{
    if (buffer != nullptr)
    {
        NVBLAST_FREE(buffer);
    }
}

NvBlastAsset* NvBlastExtBridgeDeserializeAsset(const void* buffer, uint32_t size)
{
    if (buffer == nullptr || size == 0)
    {
        return nullptr;
    }

    return reinterpret_cast<NvBlastAsset*>(serializationManager().deserializeFromBuffer(buffer, size));
}

void NvBlastExtBridgeReleaseAsset(NvBlastAsset* asset)
{
    if (asset != nullptr)
    {
        NVBLAST_FREE(asset);
    }
}

const NvBlastAsset* NvBlastExtBridgeGetAsset(const AuthoringResult& aResult)
{
    return aResult.asset;
}

// ─── Authoring result helpers ─────────────────────────────────────────────────

uint32_t NvBlastExtBridgeGetFractureChunksCount(const AuthoringResult& aResult)
{
    return aResult.chunkCount;
}

Mesh** NvBlastExtBridgeCreateMeshes(const AuthoringResult& aResult)
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

void NvBlastExtBridgeReleaseMeshesArray(Mesh** meshes)
{
    // Releases only the pointer array itself.
    // Each individual Mesh* must already have been released via NvBlastExtBridgeReleaseMesh.
    delete[] meshes;
}

void NvBlastExtBridgeReleaseAuthoringResult(ConvexMeshBuilder& collisionBuilder, AuthoringResult* ar)
{
    NvBlastExtAuthoringReleaseAuthoringResult(collisionBuilder, ar);
}