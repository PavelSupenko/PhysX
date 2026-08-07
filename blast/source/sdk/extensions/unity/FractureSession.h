#ifndef FRACTURESESSION_H
#define FRACTURESESSION_H

#include "NvBlastExtUnitySession.h"
#include "NvBlastFracturer.h"
#include "SimpleRandomGenerator.h"

#include <unordered_set>
#include <vector>

namespace Nv
{
namespace Blast
{

/**
    Owns a FractureTool across many operations so a tool can fracture, inspect, undo and re-fracture
    individual chunks. See NvBlastExtUnitySession.h for the contract this implements; this class is
    the C++ side of it and is not part of the public ABI.

    Every method addresses chunks by ID and validates them, so an out-of-date ID from a UI returns a
    result code instead of corrupting the hierarchy.
*/
class FractureSession
{
public:
    explicit FractureSession(NvBlastLog logFn);
    ~FractureSession();

    FractureSession(const FractureSession&)            = delete;
    FractureSession& operator=(const FractureSession&) = delete;

    bool isValid() const { return mTool != nullptr; }

    // ─── Source meshes ────────────────────────────────────────────────────────

    int32_t setSourceMeshes(Mesh** meshes, uint32_t meshCount, const int32_t* ids);
    void    reset();

    // ─── Settings ─────────────────────────────────────────────────────────────

    void    setSeed(int32_t seed) { mSeed = seed; }
    int32_t getSeed() const { return mSeed; }

    void    setInteriorMaterialId(int32_t materialId);
    int32_t getInteriorMaterialId() const;
    void    replaceMaterialId(int32_t oldMaterialId, int32_t newMaterialId);
    void    setRemoveIslands(bool removeIslands);

    // ─── Fracture operations ──────────────────────────────────────────────────

    int32_t fractureVoronoi(int32_t chunkId, const VoronoiConfiguration& config, bool replaceChunk);
    int32_t fractureClusteredVoronoi(int32_t chunkId, const ClusteredVoronoiConfiguration& config, bool replaceChunk);
    int32_t fractureVoronoiInSphere(int32_t chunkId, uint32_t cellCount, float radius, const NvcVec3& center,
                                    bool replaceChunk);
    int32_t fractureVoronoiWithSites(int32_t chunkId, const NvcVec3* sites, uint32_t siteCount, bool replaceChunk);
    int32_t fractureSlicing(int32_t chunkId, const SlicingConfiguration& config, bool replaceChunk);
    int32_t fractureCut(int32_t chunkId, const NvcVec3& normal, const NvcVec3& point, const NoiseConfiguration& noise,
                        bool replaceChunk);
    int32_t fractureCutout(int32_t chunkId, const NvBlastExtUnityCutoutConfiguration& config, bool replaceChunk);
    int32_t detectIslands(int32_t chunkId, bool createAtNewDepth);

    /**
        Dispatches a Fracturer descriptor to the matching operation above. This is what lets the
        one-shot API keep its Fracturer-based signatures while running through a session.
    */
    int32_t applyFracturer(int32_t chunkId, const Fracturer& fracturer, bool replaceChunk);

    // ─── Queries ──────────────────────────────────────────────────────────────

    uint32_t getChunkCount() const;
    uint32_t getChunkIds(int32_t* outIds, uint32_t maxIds) const;
    uint32_t getChunkIdsAtDepth(uint32_t depth, int32_t* outIds, uint32_t maxIds) const;
    uint32_t getChildChunkIds(int32_t chunkId, int32_t* outIds, uint32_t maxIds) const;
    int32_t  getChunkInfo(int32_t chunkId, NvBlastExtUnityChunkInfo* outInfo) const;
    int32_t  getChunkDepth(int32_t chunkId) const;

    // ─── Geometry ─────────────────────────────────────────────────────────────

    Mesh* createChunkMesh(int32_t chunkId, bool splitUVs);

    // ─── Hierarchy editing ────────────────────────────────────────────────────

    int32_t deleteChunkSubhierarchy(int32_t chunkId, bool deleteRoot);
    void    uniteChunks(uint32_t threshold, uint32_t targetClusterSize, const uint32_t* chunksToMerge,
                        uint32_t mergeChunkCount, bool removeOriginalChunks);
    int32_t setApproximateBonding(int32_t chunkId, bool useApproximateBonding);
    int32_t fitUvToRect(int32_t chunkId, float side);
    void    fitAllUvToRect(float side);

    // ─── Support graph ────────────────────────────────────────────────────────

    int32_t  setChunkStatic(int32_t chunkId, bool isStatic);
    bool     getChunkStatic(int32_t chunkId) const;
    uint32_t getStaticChunkIds(int32_t* outIds, uint32_t maxIds) const;
    void     clearStaticChunks();

    /** Mirrors the rule finalize applies, so a tool can show the support layer before finalizing. */
    bool isChunkSupport(int32_t chunkId, int32_t defaultSupportDepth) const;

    void setWorldBondDirection(const NvcVec3& direction) { mWorldBondDirection = direction; }

    // ─── Finalize ─────────────────────────────────────────────────────────────

    AuthoringResult* finalize(ConvexMeshBuilder* collisionBuilder, uint32_t aggregateMaxCount,
                              int32_t defaultSupportDepth);

private:
    /**
        Checks that the chunk exists and that the operation is legal for it, returning a session
        result code. Root chunks cannot be replaced: doing so would leave the hierarchy without the
        top-level chunk every asset needs.
    */
    int32_t validateTarget(int32_t chunkId, bool replaceChunk) const;

    /**
        Re-seeds the generator so an operation depends only on its seed and parameters, never on how
        many operations preceded it.
    */
    void reseed();

    /**
        Builds a sites generator bound to one chunk's mesh. Both outputs must be released by the
        caller, the generator first: it holds the mesh by pointer and does not own it.

        Sites land in the space the source meshes were supplied in, which is what the fracture calls
        expect — they map into the chunk's local unit-cube space themselves.
    */
    bool createSitesGenerator(int32_t chunkId, VoronoiSitesGenerator*& outGenerator, Mesh*& outMesh);

    /** Shared tail of the voronoi operations: hand the generated sites to the fracture tool. */
    int32_t applyVoronoiSites(int32_t chunkId, const NvcVec3* sites, uint32_t siteCount, bool replaceChunk);

    /**
        Rewrites the result's asset with world bonds on every static chunk that is actually a
        support chunk. Returns the number of anchors applied.
    */
    uint32_t applyWorldBonds(AuthoringResult& result);

    FractureTool*         mTool;
    SimpleRandomGenerator mRng;
    int32_t               mSeed;
    NvBlastLog            mLogFn;

    /** Chunk IDs the artist marked as anchored to the world. */
    std::unordered_set<int32_t> mStaticChunks;

    NvcVec3 mWorldBondDirection;
};

}  // namespace Blast
}  // namespace Nv

#endif  // ifndef FRACTURESESSION_H
