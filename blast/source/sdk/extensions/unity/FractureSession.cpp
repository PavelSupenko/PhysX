#include "FractureSession.h"

#include "NvBlastExtAuthoringBondGenerator.h"
#include "NvBlastExtAuthoringCutout.h"
#include "NvBlastExtAssetUtils.h"
#include "NvBlastPreprocessorInternal.h"
#include "NvBlastGlobals.h"

#include <cmath>
#include <sstream>

namespace Nv
{
namespace Blast
{

namespace
{

/**
    Builds the rotation taking +Z onto `normal`.

    Cutout patterns are authored in the XY plane facing +Z, and CutoutConfiguration::transform is
    what orients that pattern in the world, so the rotation must map the pattern's own axis onto the
    requested projection direction.
*/
NvcQuat quatFromZToNormal(const NvcVec3& normal)
{
    const float lengthSq = normal.x * normal.x + normal.y * normal.y + normal.z * normal.z;
    if (lengthSq < 1e-12f)
    {
        return { 0.0f, 0.0f, 0.0f, 1.0f };  // Degenerate direction — leave the pattern in XY
    }

    const float invLength = 1.0f / std::sqrt(lengthSq);
    const NvcVec3 to      = { normal.x * invLength, normal.y * invLength, normal.z * invLength };

    // dot(+Z, to) reduces to to.z, and cross(+Z, to) to (-to.y, to.x, 0).
    const float d = to.z;

    if (d < -1.0f + 1e-6f)
    {
        return { 1.0f, 0.0f, 0.0f, 0.0f };  // Antiparallel: half turn about any axis normal to Z
    }

    const float s    = std::sqrt((1.0f + d) * 2.0f);
    const float invS = 1.0f / s;
    return { -to.y * invS, to.x * invS, 0.0f, s * 0.5f };
}

}  // namespace

// ─── Lifetime ─────────────────────────────────────────────────────────────────

FractureSession::FractureSession(NvBlastLog logFn)
    : mTool(NvBlastExtAuthoringCreateFractureTool())
    , mSeed(0)
    , mLogFn(logFn)
    , mWorldBondDirection({ 0.0f, -1.0f, 0.0f })  // Held from below, the usual case
{
    if (mTool == nullptr)
    {
        NVBLASTLL_LOG_ERROR(mLogFn, "FractureSession: failed to create the fracture tool");
    }
}

FractureSession::~FractureSession()
{
    if (mTool != nullptr)
    {
        mTool->release();
        mTool = nullptr;
    }
}

// ─── Source meshes ────────────────────────────────────────────────────────────

int32_t FractureSession::setSourceMeshes(Mesh** meshes, uint32_t meshCount, const int32_t* ids)
{
    if (mTool == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }
    if (meshes == nullptr || meshCount == 0)
    {
        NVBLASTLL_LOG_ERROR(mLogFn, "SetSourceMeshes: no meshes supplied");
        return NvBlastExtUnitySessionResult_InvalidArgument;
    }

    for (uint32_t i = 0; i < meshCount; ++i)
    {
        if (meshes[i] == nullptr)
        {
            std::ostringstream oss;
            oss << "SetSourceMeshes: mesh " << i << " is null";
            NVBLASTLL_LOG_ERROR(mLogFn, oss.str().c_str());
            return NvBlastExtUnitySessionResult_InvalidArgument;
        }
    }

    // The tool deep-copies each mesh, so the caller keeps ownership of what it passed in.
    if (!mTool->setSourceMeshes(meshes, meshCount, ids))
    {
        NVBLASTLL_LOG_ERROR(mLogFn, "SetSourceMeshes: the fracture tool rejected the meshes");
        return NvBlastExtUnitySessionResult_InvalidArgument;
    }

    return NvBlastExtUnitySessionResult_Success;
}

void FractureSession::reset()
{
    if (mTool != nullptr)
    {
        mTool->reset();
    }
}

// ─── Settings ─────────────────────────────────────────────────────────────────

void FractureSession::setInteriorMaterialId(int32_t materialId)
{
    if (mTool != nullptr)
    {
        mTool->setInteriorMaterialId(materialId);
    }
}

int32_t FractureSession::getInteriorMaterialId() const
{
    return mTool != nullptr ? mTool->getInteriorMaterialId() : 0;
}

void FractureSession::replaceMaterialId(int32_t oldMaterialId, int32_t newMaterialId)
{
    if (mTool != nullptr)
    {
        mTool->replaceMaterialId(oldMaterialId, newMaterialId);
    }
}

void FractureSession::setRemoveIslands(bool removeIslands)
{
    if (mTool != nullptr)
    {
        mTool->setRemoveIslands(removeIslands);
    }
}

// ─── Internal helpers ─────────────────────────────────────────────────────────

void FractureSession::reseed()
{
    mRng.seed(mSeed);
}

int32_t FractureSession::validateTarget(int32_t chunkId, bool replaceChunk) const
{
    if (mTool == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }
    if (mTool->getChunkCount() == 0)
    {
        NVBLASTLL_LOG_ERROR(mLogFn, "Fracture: no source meshes have been set");
        return NvBlastExtUnitySessionResult_NoSourceMesh;
    }

    const int32_t infoIndex = mTool->getChunkInfoIndex(chunkId);
    if (infoIndex < 0)
    {
        std::ostringstream oss;
        oss << "Fracture: no chunk with ID " << chunkId;
        NVBLASTLL_LOG_ERROR(mLogFn, oss.str().c_str());
        return NvBlastExtUnitySessionResult_InvalidChunk;
    }

    // Checking for a missing parent rather than for ID 0 catches every source mesh, not just the
    // first: multi-submesh assets have one root per submesh and none of them may be replaced.
    if (replaceChunk && mTool->getChunkInfo(infoIndex).parentChunkId == -1)
    {
        std::ostringstream oss;
        oss << "Fracture: chunk " << chunkId << " is a source mesh and cannot be replaced";
        NVBLASTLL_LOG_ERROR(mLogFn, oss.str().c_str());
        return NvBlastExtUnitySessionResult_InvalidArgument;
    }

    return NvBlastExtUnitySessionResult_Success;
}

bool FractureSession::createSitesGenerator(int32_t chunkId, VoronoiSitesGenerator*& outGenerator, Mesh*& outMesh)
{
    outGenerator = nullptr;
    outMesh      = nullptr;

    const int32_t infoIndex = mTool->getChunkInfoIndex(chunkId);
    if (infoIndex < 0)
    {
        return false;
    }

    // Sites must be generated against the chunk being fractured, not the source mesh, or a
    // subdivision would scatter cells over the whole object instead of the selected piece.
    outMesh = mTool->createChunkMesh(infoIndex, false);
    if (outMesh == nullptr)
    {
        NVBLASTLL_LOG_ERROR(mLogFn, "Fracture: could not build the chunk mesh for site generation");
        return false;
    }

    outGenerator = NvBlastExtAuthoringCreateVoronoiSitesGenerator(outMesh, &mRng);
    if (outGenerator == nullptr)
    {
        NVBLASTLL_LOG_ERROR(mLogFn, "Fracture: could not create the voronoi sites generator");
        outMesh->release();
        outMesh = nullptr;
        return false;
    }

    return true;
}

int32_t FractureSession::applyVoronoiSites(int32_t chunkId, const NvcVec3* sites, uint32_t siteCount,
                                           bool replaceChunk)
{
    if (sites == nullptr || siteCount < 2)
    {
        NVBLASTLL_LOG_ERROR(mLogFn, "Fracture: voronoi fracturing needs at least 2 sites");
        return NvBlastExtUnitySessionResult_InvalidArgument;
    }

    if (mTool->voronoiFracturing(static_cast<uint32_t>(chunkId), siteCount, sites, replaceChunk) != 0)
    {
        NVBLASTLL_LOG_ERROR(mLogFn, "Fracture: voronoi fracturing failed");
        return NvBlastExtUnitySessionResult_FractureFailed;
    }

    return NvBlastExtUnitySessionResult_Success;
}

// ─── Fracture operations ──────────────────────────────────────────────────────

int32_t FractureSession::fractureVoronoi(int32_t chunkId, const VoronoiConfiguration& config, bool replaceChunk)
{
    const int32_t validation = validateTarget(chunkId, replaceChunk);
    if (validation != NvBlastExtUnitySessionResult_Success)
    {
        return validation;
    }

    reseed();

    VoronoiSitesGenerator* generator = nullptr;
    Mesh*                  chunkMesh = nullptr;
    if (!createSitesGenerator(chunkId, generator, chunkMesh))
    {
        return NvBlastExtUnitySessionResult_FractureFailed;
    }

    generator->uniformlyGenerateSitesInMesh(config.cellsCount);

    const NvcVec3* sites     = nullptr;
    const uint32_t siteCount = generator->getVoronoiSites(sites);
    const int32_t  result    = applyVoronoiSites(chunkId, sites, siteCount, replaceChunk);

    generator->release();
    chunkMesh->release();
    return result;
}

int32_t FractureSession::fractureClusteredVoronoi(int32_t chunkId, const ClusteredVoronoiConfiguration& config,
                                                  bool replaceChunk)
{
    const int32_t validation = validateTarget(chunkId, replaceChunk);
    if (validation != NvBlastExtUnitySessionResult_Success)
    {
        return validation;
    }

    reseed();

    VoronoiSitesGenerator* generator = nullptr;
    Mesh*                  chunkMesh = nullptr;
    if (!createSitesGenerator(chunkId, generator, chunkMesh))
    {
        return NvBlastExtUnitySessionResult_FractureFailed;
    }

    // clusteredSitesGeneration takes (clusterCount, sitesPerCluster, radius) in that order.
    generator->clusteredSitesGeneration(config.clusterCount, config.cellsCount, config.clusterRad);

    const NvcVec3* sites     = nullptr;
    const uint32_t siteCount = generator->getVoronoiSites(sites);
    const int32_t  result    = applyVoronoiSites(chunkId, sites, siteCount, replaceChunk);

    generator->release();
    chunkMesh->release();
    return result;
}

int32_t FractureSession::fractureVoronoiInSphere(int32_t chunkId, uint32_t cellCount, float radius,
                                                 const NvcVec3& center, bool replaceChunk)
{
    const int32_t validation = validateTarget(chunkId, replaceChunk);
    if (validation != NvBlastExtUnitySessionResult_Success)
    {
        return validation;
    }

    reseed();

    VoronoiSitesGenerator* generator = nullptr;
    Mesh*                  chunkMesh = nullptr;
    if (!createSitesGenerator(chunkId, generator, chunkMesh))
    {
        return NvBlastExtUnitySessionResult_FractureFailed;
    }

    generator->generateInSphere(cellCount, radius, center);

    const NvcVec3* sites     = nullptr;
    const uint32_t siteCount = generator->getVoronoiSites(sites);
    const int32_t  result    = applyVoronoiSites(chunkId, sites, siteCount, replaceChunk);

    generator->release();
    chunkMesh->release();
    return result;
}

int32_t FractureSession::fractureVoronoiWithSites(int32_t chunkId, const NvcVec3* sites, uint32_t siteCount,
                                                  bool replaceChunk)
{
    const int32_t validation = validateTarget(chunkId, replaceChunk);
    if (validation != NvBlastExtUnitySessionResult_Success)
    {
        return validation;
    }

    // No generator needed: the caller placed the sites itself.
    return applyVoronoiSites(chunkId, sites, siteCount, replaceChunk);
}

int32_t FractureSession::fractureSlicing(int32_t chunkId, const SlicingConfiguration& config, bool replaceChunk)
{
    const int32_t validation = validateTarget(chunkId, replaceChunk);
    if (validation != NvBlastExtUnitySessionResult_Success)
    {
        return validation;
    }

    reseed();

    if (mTool->slicing(static_cast<uint32_t>(chunkId), config, replaceChunk, &mRng) != 0)
    {
        NVBLASTLL_LOG_ERROR(mLogFn, "Fracture: slicing failed");
        return NvBlastExtUnitySessionResult_FractureFailed;
    }

    return NvBlastExtUnitySessionResult_Success;
}

int32_t FractureSession::fractureCut(int32_t chunkId, const NvcVec3& normal, const NvcVec3& point,
                                     const NoiseConfiguration& noise, bool replaceChunk)
{
    const int32_t validation = validateTarget(chunkId, replaceChunk);
    if (validation != NvBlastExtUnitySessionResult_Success)
    {
        return validation;
    }

    reseed();

    if (mTool->cut(static_cast<uint32_t>(chunkId), normal, point, noise, replaceChunk, &mRng) != 0)
    {
        NVBLASTLL_LOG_ERROR(mLogFn, "Fracture: plane cut failed");
        return NvBlastExtUnitySessionResult_FractureFailed;
    }

    return NvBlastExtUnitySessionResult_Success;
}

int32_t FractureSession::fractureCutout(int32_t chunkId, const NvBlastExtUnityCutoutConfiguration& config,
                                        bool replaceChunk)
{
    const int32_t validation = validateTarget(chunkId, replaceChunk);
    if (validation != NvBlastExtUnitySessionResult_Success)
    {
        return validation;
    }

    if (config.bitmap == nullptr || config.width == 0 || config.height == 0)
    {
        NVBLASTLL_LOG_ERROR(mLogFn, "Fracture: cutout requires a non-empty bitmap");
        return NvBlastExtUnitySessionResult_InvalidArgument;
    }

    reseed();

    CutoutConfiguration cutoutConfig;
    cutoutConfig.transform.q         = quatFromZToNormal(config.normal);
    cutoutConfig.transform.p         = config.point;
    cutoutConfig.scale               = config.scale;
    cutoutConfig.aperture            = config.aperture;
    cutoutConfig.isRelativeTransform = config.isRelativeTransform != 0;
    cutoutConfig.useSmoothing        = config.useSmoothing != 0;
    cutoutConfig.noise               = config.noise;

    cutoutConfig.cutoutSet = NvBlastExtAuthoringCreateCutoutSet();
    if (cutoutConfig.cutoutSet == nullptr)
    {
        NVBLASTLL_LOG_ERROR(mLogFn, "Fracture: could not create the cutout set");
        return NvBlastExtUnitySessionResult_FractureFailed;
    }

    NvBlastExtAuthoringBuildCutoutSet(*cutoutConfig.cutoutSet, config.bitmap, config.width, config.height,
                                      config.segmentationErrorThreshold, config.snapThreshold, config.periodic != 0,
                                      config.expandGaps != 0);

    const int32_t cutoutResult = mTool->cutout(static_cast<uint32_t>(chunkId), cutoutConfig, replaceChunk, &mRng);

    cutoutConfig.cutoutSet->release();

    if (cutoutResult != 0)
    {
        NVBLASTLL_LOG_ERROR(mLogFn, "Fracture: cutout failed");
        return NvBlastExtUnitySessionResult_FractureFailed;
    }

    return NvBlastExtUnitySessionResult_Success;
}

int32_t FractureSession::detectIslands(int32_t chunkId, bool createAtNewDepth)
{
    // Island detection rewrites the chunk in place rather than replacing it in its parent, so the
    // replace-chunk restriction does not apply and roots are legal targets.
    const int32_t validation = validateTarget(chunkId, false);
    if (validation != NvBlastExtUnitySessionResult_Success)
    {
        return validation;
    }

    return mTool->islandDetectionAndRemoving(chunkId, createAtNewDepth);
}

// ─── Queries ──────────────────────────────────────────────────────────────────

uint32_t FractureSession::getChunkCount() const
{
    return mTool != nullptr ? mTool->getChunkCount() : 0;
}

uint32_t FractureSession::getChunkIds(int32_t* outIds, uint32_t maxIds) const
{
    if (mTool == nullptr)
    {
        return 0;
    }

    const uint32_t count = mTool->getChunkCount();
    if (outIds != nullptr)
    {
        const uint32_t writeCount = count < maxIds ? count : maxIds;
        for (uint32_t i = 0; i < writeCount; ++i)
        {
            outIds[i] = mTool->getChunkId(static_cast<int32_t>(i));
        }
    }

    return count;
}

uint32_t FractureSession::getChunkIdsAtDepth(uint32_t depth, int32_t* outIds, uint32_t maxIds) const
{
    if (mTool == nullptr)
    {
        return 0;
    }

    int32_t*       ids   = nullptr;
    const uint32_t count = mTool->getChunksIdAtDepth(depth, ids);

    if (outIds != nullptr && ids != nullptr)
    {
        const uint32_t writeCount = count < maxIds ? count : maxIds;
        for (uint32_t i = 0; i < writeCount; ++i)
        {
            outIds[i] = ids[i];
        }
    }

    // getChunksIdAtDepth allocates the buffer with new[] and hands over ownership.
    delete[] ids;

    return count;
}

uint32_t FractureSession::getChildChunkIds(int32_t chunkId, int32_t* outIds, uint32_t maxIds) const
{
    if (mTool == nullptr)
    {
        return 0;
    }

    uint32_t       count      = 0;
    const uint32_t chunkCount = mTool->getChunkCount();
    for (uint32_t i = 0; i < chunkCount; ++i)
    {
        if (mTool->getChunkInfo(static_cast<int32_t>(i)).parentChunkId != chunkId)
        {
            continue;
        }

        if (outIds != nullptr && count < maxIds)
        {
            outIds[count] = mTool->getChunkId(static_cast<int32_t>(i));
        }
        ++count;
    }

    return count;
}

int32_t FractureSession::getChunkInfo(int32_t chunkId, NvBlastExtUnityChunkInfo* outInfo) const
{
    if (mTool == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }
    if (outInfo == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidArgument;
    }

    const int32_t infoIndex = mTool->getChunkInfoIndex(chunkId);
    if (infoIndex < 0)
    {
        return NvBlastExtUnitySessionResult_InvalidChunk;
    }

    const ChunkInfo&   info = mTool->getChunkInfo(infoIndex);
    const TransformST& tm   = info.getTmToWorld();

    outInfo->chunkId       = info.chunkId;
    outInfo->parentChunkId = info.parentChunkId;
    outInfo->depth         = mTool->getChunkDepth(chunkId);

    uint32_t flags = NvBlastExtUnityChunkFlag_None;
    if ((info.flags & ChunkInfo::APPROXIMATE_BONDING) != 0)
    {
        flags |= NvBlastExtUnityChunkFlag_ApproximateBonding;
    }
    if (info.isLeaf)
    {
        flags |= NvBlastExtUnityChunkFlag_IsLeaf;
    }
    if (info.isChanged)
    {
        flags |= NvBlastExtUnityChunkFlag_IsChanged;
    }
    if (info.parentChunkId == -1)
    {
        flags |= NvBlastExtUnityChunkFlag_IsRoot;
    }
    outInfo->flags = flags;

    outInfo->worldTranslation = tm.t;
    outInfo->worldScale       = tm.s;

    return NvBlastExtUnitySessionResult_Success;
}

int32_t FractureSession::getChunkDepth(int32_t chunkId) const
{
    return mTool != nullptr ? mTool->getChunkDepth(chunkId) : -1;
}

// ─── Geometry ─────────────────────────────────────────────────────────────────

Mesh* FractureSession::createChunkMesh(int32_t chunkId, bool splitUVs)
{
    if (mTool == nullptr)
    {
        return nullptr;
    }

    const int32_t infoIndex = mTool->getChunkInfoIndex(chunkId);
    if (infoIndex < 0)
    {
        return nullptr;
    }

    return mTool->createChunkMesh(infoIndex, splitUVs);
}

// ─── Hierarchy editing ────────────────────────────────────────────────────────

int32_t FractureSession::deleteChunkSubhierarchy(int32_t chunkId, bool deleteRoot)
{
    if (mTool == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }
    if (mTool->getChunkInfoIndex(chunkId) < 0)
    {
        return NvBlastExtUnitySessionResult_InvalidChunk;
    }

    return mTool->deleteChunkSubhierarchy(chunkId, deleteRoot) ? NvBlastExtUnitySessionResult_Success
                                                               : NvBlastExtUnitySessionResult_InvalidChunk;
}

void FractureSession::uniteChunks(uint32_t threshold, uint32_t targetClusterSize, const uint32_t* chunksToMerge,
                                  uint32_t mergeChunkCount, bool removeOriginalChunks)
{
    if (mTool == nullptr)
    {
        return;
    }

    mTool->uniteChunks(threshold, targetClusterSize, chunksToMerge, mergeChunkCount, nullptr, 0, removeOriginalChunks);
}

int32_t FractureSession::setApproximateBonding(int32_t chunkId, bool useApproximateBonding)
{
    if (mTool == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }

    const int32_t infoIndex = mTool->getChunkInfoIndex(chunkId);
    if (infoIndex < 0)
    {
        return NvBlastExtUnitySessionResult_InvalidChunk;
    }

    return mTool->setApproximateBonding(static_cast<uint32_t>(infoIndex), useApproximateBonding)
               ? NvBlastExtUnitySessionResult_Success
               : NvBlastExtUnitySessionResult_InvalidChunk;
}

int32_t FractureSession::fitUvToRect(int32_t chunkId, float side)
{
    if (mTool == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }
    if (mTool->getChunkInfoIndex(chunkId) < 0)
    {
        return NvBlastExtUnitySessionResult_InvalidChunk;
    }

    mTool->fitUvToRect(side, static_cast<uint32_t>(chunkId));
    return NvBlastExtUnitySessionResult_Success;
}

void FractureSession::fitAllUvToRect(float side)
{
    if (mTool != nullptr)
    {
        mTool->fitAllUvToRect(side);
    }
}

// ─── Support graph ────────────────────────────────────────────────────────────

int32_t FractureSession::setChunkStatic(int32_t chunkId, bool isStatic)
{
    if (mTool == nullptr)
    {
        return NvBlastExtUnitySessionResult_InvalidSession;
    }
    if (mTool->getChunkInfoIndex(chunkId) < 0)
    {
        return NvBlastExtUnitySessionResult_InvalidChunk;
    }

    if (isStatic)
    {
        mStaticChunks.insert(chunkId);
    }
    else
    {
        mStaticChunks.erase(chunkId);
    }

    return NvBlastExtUnitySessionResult_Success;
}

bool FractureSession::getChunkStatic(int32_t chunkId) const
{
    return mStaticChunks.find(chunkId) != mStaticChunks.end();
}

uint32_t FractureSession::getStaticChunkIds(int32_t* outIds, uint32_t maxIds) const
{
    if (outIds != nullptr)
    {
        uint32_t written = 0;
        for (int32_t chunkId : mStaticChunks)
        {
            if (written >= maxIds)
            {
                break;
            }
            outIds[written++] = chunkId;
        }
    }

    return static_cast<uint32_t>(mStaticChunks.size());
}

void FractureSession::clearStaticChunks()
{
    mStaticChunks.clear();
}

bool FractureSession::isChunkSupport(int32_t chunkId, int32_t defaultSupportDepth) const
{
    if (mTool == nullptr)
    {
        return false;
    }

    const int32_t infoIndex = mTool->getChunkInfoIndex(chunkId);
    if (infoIndex < 0)
    {
        return false;
    }

    // Mirrors NvBlastExtAuthoringProcessFracture: above the requested depth every leaf is support,
    // at the requested depth every chunk is.
    const int32_t depth = mTool->getChunkDepth(chunkId);

    if (defaultSupportDepth < 0 || depth < defaultSupportDepth)
    {
        return mTool->getChunkInfo(infoIndex).isLeaf;
    }

    return depth == defaultSupportDepth;
}

uint32_t FractureSession::applyWorldBonds(AuthoringResult& result)
{
    if (mStaticChunks.empty() || result.asset == nullptr)
    {
        return 0;
    }

    // Static marks are held as chunk IDs, but the asset addresses chunks by its own index, and the
    // two differ because finalizing reorders chunks. assetToFractureChunkIdMap is that mapping.
    std::vector<uint32_t> anchoredChunks;
    anchoredChunks.reserve(mStaticChunks.size());

    for (uint32_t assetIndex = 0; assetIndex < result.chunkCount; ++assetIndex)
    {
        const int32_t chunkId = static_cast<int32_t>(result.assetToFractureChunkIdMap[assetIndex]);

        if (mStaticChunks.find(chunkId) == mStaticChunks.end())
        {
            continue;
        }

        // Only support chunks can carry an external bond; anything else is silently ignored by the
        // asset builder, so report it instead of letting the anchor quietly go missing.
        if ((result.chunkDescs[assetIndex].flags & NvBlastChunkDesc::SupportFlag) == 0)
        {
            std::ostringstream oss;
            oss << "Finalize: chunk " << chunkId
                << " is marked static but is not a support chunk, so it cannot be anchored";
            NVBLASTLL_LOG_ERROR(mLogFn, oss.str().c_str());
            continue;
        }

        anchoredChunks.push_back(assetIndex);
    }

    if (anchoredChunks.empty())
    {
        return 0;
    }

    std::vector<NvcVec3> directions(anchoredChunks.size(), mWorldBondDirection);

    NvBlastAsset* anchored = NvBlastExtAssetUtilsAddExternalBonds(
        result.asset, anchoredChunks.data(), static_cast<uint32_t>(anchoredChunks.size()),
        directions.data(), nullptr);

    if (anchored == nullptr)
    {
        NVBLASTLL_LOG_ERROR(mLogFn, "Finalize: failed to add world bonds; the asset is left unanchored");
        return 0;
    }

    // AddExternalBonds returns a freshly allocated asset and leaves the original untouched, so the
    // old one has to be released or it leaks for the lifetime of the result.
    NVBLAST_FREE(result.asset);
    result.asset = anchored;

    return static_cast<uint32_t>(anchoredChunks.size());
}

// ─── Finalize ─────────────────────────────────────────────────────────────────

AuthoringResult* FractureSession::finalize(ConvexMeshBuilder* collisionBuilder, uint32_t aggregateMaxCount,
                                           int32_t defaultSupportDepth)
{
    if (mTool == nullptr || collisionBuilder == nullptr)
    {
        NVBLASTLL_LOG_ERROR(mLogFn, "Finalize: session or collision builder is null");
        return nullptr;
    }
    if (mTool->getChunkCount() == 0)
    {
        NVBLASTLL_LOG_ERROR(mLogFn, "Finalize: nothing to finalize, no source meshes have been set");
        return nullptr;
    }

    BlastBondGenerator* bondGenerator = NvBlastExtAuthoringCreateBondGenerator(collisionBuilder);
    if (bondGenerator == nullptr)
    {
        NVBLASTLL_LOG_ERROR(mLogFn, "Finalize: could not create the bond generator");
        return nullptr;
    }

    ConvexDecompositionParams collisionParams;
    collisionParams.maximumNumberOfHulls = aggregateMaxCount > 0 ? aggregateMaxCount : 1;
    collisionParams.voxelGridResolution  = 0;

    // ProcessFracture calls finalizeFracturing itself, and leaves the tool usable afterwards, so the
    // session can keep being edited and finalized again.
    AuthoringResult* result =
        NvBlastExtAuthoringProcessFracture(*mTool, *bondGenerator, *collisionBuilder, collisionParams,
                                           defaultSupportDepth);

    bondGenerator->release();

    if (result == nullptr)
    {
        NVBLASTLL_LOG_ERROR(mLogFn, "Finalize: ProcessFracture returned no result");
        return nullptr;
    }

    const uint32_t anchored = applyWorldBonds(*result);
    if (anchored > 0)
    {
        std::ostringstream oss;
        oss << "Finalize: anchored " << anchored << " chunk(s) to the world";
        NVBLASTLL_LOG_DEBUG(mLogFn, oss.str().c_str());
    }

    return result;
}

}  // namespace Blast
}  // namespace Nv
