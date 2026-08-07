#include "NvBlastExtUnitySession.h"

#include "FractureSession.h"

using namespace Nv::Blast;

namespace
{

/**
    The public handle is an opaque struct so callers cannot reach into the session; internally it is
    always a FractureSession.
*/
inline FractureSession* toSession(NvBlastExtUnityFractureSession* handle)
{
    return reinterpret_cast<FractureSession*>(handle);
}

inline const FractureSession* toSession(const NvBlastExtUnityFractureSession* handle)
{
    return reinterpret_cast<const FractureSession*>(handle);
}

}  // namespace

// ─── Session lifecycle ────────────────────────────────────────────────────────

NvBlastExtUnityFractureSession* NvBlastExtUnitySessionCreate(NvBlastLog logFn)
{
    FractureSession* session = new FractureSession(logFn);
    if (!session->isValid())
    {
        delete session;
        return nullptr;
    }

    return reinterpret_cast<NvBlastExtUnityFractureSession*>(session);
}

void NvBlastExtUnitySessionRelease(NvBlastExtUnityFractureSession* session)
{
    delete toSession(session);
}

void NvBlastExtUnitySessionReset(NvBlastExtUnityFractureSession* session)
{
    if (session != nullptr)
    {
        toSession(session)->reset();
    }
}

// ─── Source meshes ────────────────────────────────────────────────────────────

int32_t NvBlastExtUnitySessionSetSourceMeshes(NvBlastExtUnityFractureSession* session, Mesh** meshes,
                                              uint32_t meshCount, const int32_t* ids)
{
    if (session == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }

    return toSession(session)->setSourceMeshes(meshes, meshCount, ids);
}

// ─── Settings ─────────────────────────────────────────────────────────────────

void NvBlastExtUnitySessionSetSeed(NvBlastExtUnityFractureSession* session, int32_t seed)
{
    if (session != nullptr)
    {
        toSession(session)->setSeed(seed);
    }
}

int32_t NvBlastExtUnitySessionGetSeed(const NvBlastExtUnityFractureSession* session)
{
    return session != nullptr ? toSession(session)->getSeed() : 0;
}

void NvBlastExtUnitySessionSetInteriorMaterialId(NvBlastExtUnityFractureSession* session, int32_t materialId)
{
    if (session != nullptr)
    {
        toSession(session)->setInteriorMaterialId(materialId);
    }
}

int32_t NvBlastExtUnitySessionGetInteriorMaterialId(const NvBlastExtUnityFractureSession* session)
{
    return session != nullptr ? toSession(session)->getInteriorMaterialId() : 0;
}

void NvBlastExtUnitySessionReplaceMaterialId(NvBlastExtUnityFractureSession* session, int32_t oldMaterialId,
                                             int32_t newMaterialId)
{
    if (session != nullptr)
    {
        toSession(session)->replaceMaterialId(oldMaterialId, newMaterialId);
    }
}

void NvBlastExtUnitySessionSetRemoveIslands(NvBlastExtUnityFractureSession* session,
                                            NvBlastExtUnityBool removeIslands)
{
    if (session != nullptr)
    {
        toSession(session)->setRemoveIslands(removeIslands != 0);
    }
}

// ─── Fracture operations ──────────────────────────────────────────────────────

int32_t NvBlastExtUnitySessionFractureVoronoi(NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                              VoronoiConfiguration config, NvBlastExtUnityBool replaceChunk)
{
    if (session == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }

    return toSession(session)->fractureVoronoi(chunkId, config, replaceChunk != 0);
}

int32_t NvBlastExtUnitySessionFractureClusteredVoronoi(NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                                       ClusteredVoronoiConfiguration config,
                                                       NvBlastExtUnityBool replaceChunk)
{
    if (session == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }

    return toSession(session)->fractureClusteredVoronoi(chunkId, config, replaceChunk != 0);
}

int32_t NvBlastExtUnitySessionFractureVoronoiInSphere(NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                                      uint32_t cellCount, float radius, NvcVec3 center,
                                                      NvBlastExtUnityBool replaceChunk)
{
    if (session == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }

    return toSession(session)->fractureVoronoiInSphere(chunkId, cellCount, radius, center, replaceChunk != 0);
}

int32_t NvBlastExtUnitySessionFractureVoronoiWithSites(NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                                       const NvcVec3* sites, uint32_t siteCount,
                                                       NvBlastExtUnityBool replaceChunk)
{
    if (session == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }

    return toSession(session)->fractureVoronoiWithSites(chunkId, sites, siteCount, replaceChunk != 0);
}

int32_t NvBlastExtUnitySessionFractureSlicing(NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                              SlicingConfiguration config, NvBlastExtUnityBool replaceChunk)
{
    if (session == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }

    return toSession(session)->fractureSlicing(chunkId, config, replaceChunk != 0);
}

int32_t NvBlastExtUnitySessionFractureCut(NvBlastExtUnityFractureSession* session, int32_t chunkId, NvcVec3 normal,
                                          NvcVec3 point, NoiseConfiguration noise, NvBlastExtUnityBool replaceChunk)
{
    if (session == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }

    return toSession(session)->fractureCut(chunkId, normal, point, noise, replaceChunk != 0);
}

int32_t NvBlastExtUnitySessionFractureCutout(NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                             const NvBlastExtUnityCutoutConfiguration* config,
                                             NvBlastExtUnityBool replaceChunk)
{
    if (session == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }
    if (config == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidArgument;
    }

    return toSession(session)->fractureCutout(chunkId, *config, replaceChunk != 0);
}

int32_t NvBlastExtUnitySessionDetectIslands(NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                            NvBlastExtUnityBool createAtNewDepth)
{
    if (session == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }

    return toSession(session)->detectIslands(chunkId, createAtNewDepth != 0);
}

// ─── Hierarchy queries ────────────────────────────────────────────────────────

uint32_t NvBlastExtUnitySessionGetChunkCount(const NvBlastExtUnityFractureSession* session)
{
    return session != nullptr ? toSession(session)->getChunkCount() : 0;
}

uint32_t NvBlastExtUnitySessionGetChunkIds(const NvBlastExtUnityFractureSession* session, int32_t* outIds,
                                           uint32_t maxIds)
{
    return session != nullptr ? toSession(session)->getChunkIds(outIds, maxIds) : 0;
}

uint32_t NvBlastExtUnitySessionGetChunkIdsAtDepth(const NvBlastExtUnityFractureSession* session, uint32_t depth,
                                                  int32_t* outIds, uint32_t maxIds)
{
    return session != nullptr ? toSession(session)->getChunkIdsAtDepth(depth, outIds, maxIds) : 0;
}

uint32_t NvBlastExtUnitySessionGetChildChunkIds(const NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                                int32_t* outIds, uint32_t maxIds)
{
    return session != nullptr ? toSession(session)->getChildChunkIds(chunkId, outIds, maxIds) : 0;
}

int32_t NvBlastExtUnitySessionGetChunkInfo(const NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                           NvBlastExtUnityChunkInfo* outInfo)
{
    if (session == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }

    return toSession(session)->getChunkInfo(chunkId, outInfo);
}

int32_t NvBlastExtUnitySessionGetChunkDepth(const NvBlastExtUnityFractureSession* session, int32_t chunkId)
{
    return session != nullptr ? toSession(session)->getChunkDepth(chunkId) : -1;
}

// ─── Chunk geometry ───────────────────────────────────────────────────────────

Mesh* NvBlastExtUnitySessionCreateChunkMesh(NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                            NvBlastExtUnityBool splitUVs)
{
    return session != nullptr ? toSession(session)->createChunkMesh(chunkId, splitUVs != 0) : nullptr;
}

// ─── Hierarchy editing ────────────────────────────────────────────────────────

int32_t NvBlastExtUnitySessionDeleteChunkSubhierarchy(NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                                      NvBlastExtUnityBool deleteRoot)
{
    if (session == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }

    return toSession(session)->deleteChunkSubhierarchy(chunkId, deleteRoot != 0);
}

void NvBlastExtUnitySessionUniteChunks(NvBlastExtUnityFractureSession* session, uint32_t threshold,
                                       uint32_t targetClusterSize, const uint32_t* chunksToMerge,
                                       uint32_t mergeChunkCount, NvBlastExtUnityBool removeOriginalChunks)
{
    if (session != nullptr)
    {
        toSession(session)->uniteChunks(threshold, targetClusterSize, chunksToMerge, mergeChunkCount,
                                        removeOriginalChunks != 0);
    }
}

int32_t NvBlastExtUnitySessionSetApproximateBonding(NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                                    NvBlastExtUnityBool useApproximateBonding)
{
    if (session == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }

    return toSession(session)->setApproximateBonding(chunkId, useApproximateBonding != 0);
}

int32_t NvBlastExtUnitySessionFitUvToRect(NvBlastExtUnityFractureSession* session, int32_t chunkId, float side)
{
    if (session == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }

    return toSession(session)->fitUvToRect(chunkId, side);
}

void NvBlastExtUnitySessionFitAllUvToRect(NvBlastExtUnityFractureSession* session, float side)
{
    if (session != nullptr)
    {
        toSession(session)->fitAllUvToRect(side);
    }
}

// ─── Support graph ────────────────────────────────────────────────────────────

int32_t NvBlastExtUnitySessionSetChunkStatic(NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                             NvBlastExtUnityBool isStatic)
{
    if (session == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }

    return toSession(session)->setChunkStatic(chunkId, isStatic != 0);
}

NvBlastExtUnityBool NvBlastExtUnitySessionGetChunkStatic(const NvBlastExtUnityFractureSession* session,
                                                         int32_t chunkId)
{
    return session != nullptr && toSession(session)->getChunkStatic(chunkId) ? 1u : 0u;
}

uint32_t NvBlastExtUnitySessionGetStaticChunkIds(const NvBlastExtUnityFractureSession* session, int32_t* outIds,
                                                 uint32_t maxIds)
{
    return session != nullptr ? toSession(session)->getStaticChunkIds(outIds, maxIds) : 0;
}

void NvBlastExtUnitySessionClearStaticChunks(NvBlastExtUnityFractureSession* session)
{
    if (session != nullptr)
    {
        toSession(session)->clearStaticChunks();
    }
}

NvBlastExtUnityBool NvBlastExtUnitySessionIsChunkSupport(const NvBlastExtUnityFractureSession* session,
                                                         int32_t chunkId, int32_t defaultSupportDepth)
{
    return session != nullptr && toSession(session)->isChunkSupport(chunkId, defaultSupportDepth) ? 1u : 0u;
}

void NvBlastExtUnitySessionSetWorldBondDirection(NvBlastExtUnityFractureSession* session, NvcVec3 direction)
{
    if (session != nullptr)
    {
        toSession(session)->setWorldBondDirection(direction);
    }
}

// ─── Finalize ─────────────────────────────────────────────────────────────────

AuthoringResult* NvBlastExtUnitySessionFinalize(NvBlastExtUnityFractureSession* session,
                                                ConvexMeshBuilder* collisionBuilder, uint32_t aggregateMaxCount,
                                                int32_t defaultSupportDepth)
{
    if (session == nullptr)
    {
        return nullptr;
    }

    return toSession(session)->finalize(collisionBuilder, aggregateMaxCount, defaultSupportDepth);
}
