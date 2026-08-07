#include "NvBlastExtBridgeSession.h"

#include "FractureSession.h"

using namespace Nv::Blast;

namespace
{

/**
    The public handle is an opaque struct so callers cannot reach into the session; internally it is
    always a FractureSession.
*/
inline FractureSession* toSession(NvBlastExtBridgeFractureSession* handle)
{
    return reinterpret_cast<FractureSession*>(handle);
}

inline const FractureSession* toSession(const NvBlastExtBridgeFractureSession* handle)
{
    return reinterpret_cast<const FractureSession*>(handle);
}

}  // namespace

// ─── Session lifecycle ────────────────────────────────────────────────────────

NvBlastExtBridgeFractureSession* NvBlastExtBridgeSessionCreate(NvBlastLog logFn)
{
    FractureSession* session = new FractureSession(logFn);
    if (!session->isValid())
    {
        delete session;
        return nullptr;
    }

    return reinterpret_cast<NvBlastExtBridgeFractureSession*>(session);
}

void NvBlastExtBridgeSessionRelease(NvBlastExtBridgeFractureSession* session)
{
    delete toSession(session);
}

void NvBlastExtBridgeSessionReset(NvBlastExtBridgeFractureSession* session)
{
    if (session != nullptr)
    {
        toSession(session)->reset();
    }
}

// ─── Source meshes ────────────────────────────────────────────────────────────

int32_t NvBlastExtBridgeSessionSetSourceMeshes(NvBlastExtBridgeFractureSession* session, Mesh** meshes,
                                              uint32_t meshCount, const int32_t* ids)
{
    if (session == nullptr)
    {
        return NvBlastExtBridgeSessionResult_InvalidSession;
    }

    return toSession(session)->setSourceMeshes(meshes, meshCount, ids);
}

// ─── Settings ─────────────────────────────────────────────────────────────────

void NvBlastExtBridgeSessionSetSeed(NvBlastExtBridgeFractureSession* session, int32_t seed)
{
    if (session != nullptr)
    {
        toSession(session)->setSeed(seed);
    }
}

int32_t NvBlastExtBridgeSessionGetSeed(const NvBlastExtBridgeFractureSession* session)
{
    return session != nullptr ? toSession(session)->getSeed() : 0;
}

void NvBlastExtBridgeSessionSetInteriorMaterialId(NvBlastExtBridgeFractureSession* session, int32_t materialId)
{
    if (session != nullptr)
    {
        toSession(session)->setInteriorMaterialId(materialId);
    }
}

int32_t NvBlastExtBridgeSessionGetInteriorMaterialId(const NvBlastExtBridgeFractureSession* session)
{
    return session != nullptr ? toSession(session)->getInteriorMaterialId() : 0;
}

void NvBlastExtBridgeSessionReplaceMaterialId(NvBlastExtBridgeFractureSession* session, int32_t oldMaterialId,
                                             int32_t newMaterialId)
{
    if (session != nullptr)
    {
        toSession(session)->replaceMaterialId(oldMaterialId, newMaterialId);
    }
}

void NvBlastExtBridgeSessionSetRemoveIslands(NvBlastExtBridgeFractureSession* session,
                                            NvBlastExtBridgeBool removeIslands)
{
    if (session != nullptr)
    {
        toSession(session)->setRemoveIslands(removeIslands != 0);
    }
}

// ─── Fracture operations ──────────────────────────────────────────────────────

int32_t NvBlastExtBridgeSessionFractureVoronoi(NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                              VoronoiConfiguration config, NvBlastExtBridgeBool replaceChunk)
{
    if (session == nullptr)
    {
        return NvBlastExtBridgeSessionResult_InvalidSession;
    }

    return toSession(session)->fractureVoronoi(chunkId, config, replaceChunk != 0);
}

int32_t NvBlastExtBridgeSessionFractureClusteredVoronoi(NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                                       ClusteredVoronoiConfiguration config,
                                                       NvBlastExtBridgeBool replaceChunk)
{
    if (session == nullptr)
    {
        return NvBlastExtBridgeSessionResult_InvalidSession;
    }

    return toSession(session)->fractureClusteredVoronoi(chunkId, config, replaceChunk != 0);
}

int32_t NvBlastExtBridgeSessionFractureVoronoiInSphere(NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                                      uint32_t cellCount, float radius, NvcVec3 center,
                                                      NvBlastExtBridgeBool replaceChunk)
{
    if (session == nullptr)
    {
        return NvBlastExtBridgeSessionResult_InvalidSession;
    }

    return toSession(session)->fractureVoronoiInSphere(chunkId, cellCount, radius, center, replaceChunk != 0);
}

int32_t NvBlastExtBridgeSessionFractureVoronoiWithSites(NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                                       const NvcVec3* sites, uint32_t siteCount,
                                                       NvBlastExtBridgeBool replaceChunk)
{
    if (session == nullptr)
    {
        return NvBlastExtBridgeSessionResult_InvalidSession;
    }

    return toSession(session)->fractureVoronoiWithSites(chunkId, sites, siteCount, replaceChunk != 0);
}

int32_t NvBlastExtBridgeSessionFractureSlicing(NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                              SlicingConfiguration config, NvBlastExtBridgeBool replaceChunk)
{
    if (session == nullptr)
    {
        return NvBlastExtBridgeSessionResult_InvalidSession;
    }

    return toSession(session)->fractureSlicing(chunkId, config, replaceChunk != 0);
}

int32_t NvBlastExtBridgeSessionFractureCut(NvBlastExtBridgeFractureSession* session, int32_t chunkId, NvcVec3 normal,
                                          NvcVec3 point, NoiseConfiguration noise, NvBlastExtBridgeBool replaceChunk)
{
    if (session == nullptr)
    {
        return NvBlastExtBridgeSessionResult_InvalidSession;
    }

    return toSession(session)->fractureCut(chunkId, normal, point, noise, replaceChunk != 0);
}

int32_t NvBlastExtBridgeSessionFractureCutout(NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                             const NvBlastExtBridgeCutoutConfiguration* config,
                                             NvBlastExtBridgeBool replaceChunk)
{
    if (session == nullptr)
    {
        return NvBlastExtBridgeSessionResult_InvalidSession;
    }
    if (config == nullptr)
    {
        return NvBlastExtBridgeSessionResult_InvalidArgument;
    }

    return toSession(session)->fractureCutout(chunkId, *config, replaceChunk != 0);
}

int32_t NvBlastExtBridgeSessionDetectIslands(NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                            NvBlastExtBridgeBool createAtNewDepth)
{
    if (session == nullptr)
    {
        return NvBlastExtBridgeSessionResult_InvalidSession;
    }

    return toSession(session)->detectIslands(chunkId, createAtNewDepth != 0);
}

// ─── Hierarchy queries ────────────────────────────────────────────────────────

uint32_t NvBlastExtBridgeSessionGetChunkCount(const NvBlastExtBridgeFractureSession* session)
{
    return session != nullptr ? toSession(session)->getChunkCount() : 0;
}

uint32_t NvBlastExtBridgeSessionGetChunkIds(const NvBlastExtBridgeFractureSession* session, int32_t* outIds,
                                           uint32_t maxIds)
{
    return session != nullptr ? toSession(session)->getChunkIds(outIds, maxIds) : 0;
}

uint32_t NvBlastExtBridgeSessionGetChunkIdsAtDepth(const NvBlastExtBridgeFractureSession* session, uint32_t depth,
                                                  int32_t* outIds, uint32_t maxIds)
{
    return session != nullptr ? toSession(session)->getChunkIdsAtDepth(depth, outIds, maxIds) : 0;
}

uint32_t NvBlastExtBridgeSessionGetChildChunkIds(const NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                                int32_t* outIds, uint32_t maxIds)
{
    return session != nullptr ? toSession(session)->getChildChunkIds(chunkId, outIds, maxIds) : 0;
}

int32_t NvBlastExtBridgeSessionGetChunkInfo(const NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                           NvBlastExtBridgeChunkInfo* outInfo)
{
    if (session == nullptr)
    {
        return NvBlastExtBridgeSessionResult_InvalidSession;
    }

    return toSession(session)->getChunkInfo(chunkId, outInfo);
}

int32_t NvBlastExtBridgeSessionGetChunkDepth(const NvBlastExtBridgeFractureSession* session, int32_t chunkId)
{
    return session != nullptr ? toSession(session)->getChunkDepth(chunkId) : -1;
}

// ─── Chunk geometry ───────────────────────────────────────────────────────────

Mesh* NvBlastExtBridgeSessionCreateChunkMesh(NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                            NvBlastExtBridgeBool splitUVs)
{
    return session != nullptr ? toSession(session)->createChunkMesh(chunkId, splitUVs != 0) : nullptr;
}

// ─── Hierarchy editing ────────────────────────────────────────────────────────

int32_t NvBlastExtBridgeSessionDeleteChunkSubhierarchy(NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                                      NvBlastExtBridgeBool deleteRoot)
{
    if (session == nullptr)
    {
        return NvBlastExtBridgeSessionResult_InvalidSession;
    }

    return toSession(session)->deleteChunkSubhierarchy(chunkId, deleteRoot != 0);
}

void NvBlastExtBridgeSessionUniteChunks(NvBlastExtBridgeFractureSession* session, uint32_t threshold,
                                       uint32_t targetClusterSize, const uint32_t* chunksToMerge,
                                       uint32_t mergeChunkCount, NvBlastExtBridgeBool removeOriginalChunks)
{
    if (session != nullptr)
    {
        toSession(session)->uniteChunks(threshold, targetClusterSize, chunksToMerge, mergeChunkCount,
                                        removeOriginalChunks != 0);
    }
}

int32_t NvBlastExtBridgeSessionSetApproximateBonding(NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                                    NvBlastExtBridgeBool useApproximateBonding)
{
    if (session == nullptr)
    {
        return NvBlastExtBridgeSessionResult_InvalidSession;
    }

    return toSession(session)->setApproximateBonding(chunkId, useApproximateBonding != 0);
}

int32_t NvBlastExtBridgeSessionFitUvToRect(NvBlastExtBridgeFractureSession* session, int32_t chunkId, float side)
{
    if (session == nullptr)
    {
        return NvBlastExtBridgeSessionResult_InvalidSession;
    }

    return toSession(session)->fitUvToRect(chunkId, side);
}

void NvBlastExtBridgeSessionFitAllUvToRect(NvBlastExtBridgeFractureSession* session, float side)
{
    if (session != nullptr)
    {
        toSession(session)->fitAllUvToRect(side);
    }
}

// ─── Support graph ────────────────────────────────────────────────────────────

int32_t NvBlastExtBridgeSessionSetChunkStatic(NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                             NvBlastExtBridgeBool isStatic)
{
    if (session == nullptr)
    {
        return NvBlastExtBridgeSessionResult_InvalidSession;
    }

    return toSession(session)->setChunkStatic(chunkId, isStatic != 0);
}

NvBlastExtBridgeBool NvBlastExtBridgeSessionGetChunkStatic(const NvBlastExtBridgeFractureSession* session,
                                                         int32_t chunkId)
{
    return session != nullptr && toSession(session)->getChunkStatic(chunkId) ? 1u : 0u;
}

uint32_t NvBlastExtBridgeSessionGetStaticChunkIds(const NvBlastExtBridgeFractureSession* session, int32_t* outIds,
                                                 uint32_t maxIds)
{
    return session != nullptr ? toSession(session)->getStaticChunkIds(outIds, maxIds) : 0;
}

void NvBlastExtBridgeSessionClearStaticChunks(NvBlastExtBridgeFractureSession* session)
{
    if (session != nullptr)
    {
        toSession(session)->clearStaticChunks();
    }
}

NvBlastExtBridgeBool NvBlastExtBridgeSessionIsChunkSupport(const NvBlastExtBridgeFractureSession* session,
                                                         int32_t chunkId, int32_t defaultSupportDepth)
{
    return session != nullptr && toSession(session)->isChunkSupport(chunkId, defaultSupportDepth) ? 1u : 0u;
}

void NvBlastExtBridgeSessionSetWorldBondDirection(NvBlastExtBridgeFractureSession* session, NvcVec3 direction)
{
    if (session != nullptr)
    {
        toSession(session)->setWorldBondDirection(direction);
    }
}

// ─── Finalize ─────────────────────────────────────────────────────────────────

AuthoringResult* NvBlastExtBridgeSessionFinalize(NvBlastExtBridgeFractureSession* session,
                                                ConvexMeshBuilder* collisionBuilder, uint32_t aggregateMaxCount,
                                                int32_t defaultSupportDepth)
{
    if (session == nullptr)
    {
        return nullptr;
    }

    return toSession(session)->finalize(collisionBuilder, aggregateMaxCount, defaultSupportDepth);
}
