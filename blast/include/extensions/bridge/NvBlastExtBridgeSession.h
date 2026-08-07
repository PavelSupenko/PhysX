//! @file
//!
//! @brief Persistent authoring session API — the interactive counterpart to the one-shot
//!        fracture calls in NvBlastExtBridge.h.
//!
//! The one-shot API builds a FractureTool, fractures, and tears it down inside a single call, so
//! every operation starts from the source mesh. An authoring tool needs the opposite: the artist
//! selects a chunk, fractures it, inspects the result, undoes it, fractures it differently, then
//! moves to a sibling. That requires the FractureTool to outlive individual operations, which is
//! what a session is.
//!
//! Usage:
//!
//!     session = NvBlastExtBridgeSessionCreate(logFn);
//!     NvBlastExtBridgeSessionSetSourceMeshes(session, meshes, count, ids);
//!     NvBlastExtBridgeSessionFractureVoronoi(session, 0, voronoiConfig, false);   // depth 1
//!     NvBlastExtBridgeSessionFractureVoronoi(session, 3, voronoiConfig, false);   // subdivide one chunk
//!     NvBlastExtBridgeSessionDeleteChunkSubhierarchy(session, 3, false);          // undo that
//!     result = NvBlastExtBridgeSessionFinalize(session, collisionBuilder, 1);
//!     NvBlastExtBridgeSessionRelease(session);
//!
//! Conventions:
//!
//! - Chunks are addressed by **chunk ID**, never by info index. IDs are stable across operations;
//!   info indices are positions in an internal array and shift whenever chunks are added or removed.
//! - Booleans cross the ABI as NvBlastExtBridgeBool (uint32_t, 0 or 1). C# `bool` marshals as a
//!   4-byte BOOL by default but C++ `bool` is 1 byte, and the mismatch is silent — a fixed-width
//!   integer removes the trap entirely. The same reason drives the flags field on chunk info.
//! - Every entry point tolerates a null session and returns a failure code rather than crashing.

#ifndef NVBLASTEXTBRIDGESESSION_H
#define NVBLASTEXTBRIDGESESSION_H

#include "NvBlastGlobals.h"
#include "NvBlastExtAuthoring.h"
#include "NvBlastExtAuthoringMesh.h"
#include "NvBlastExtAuthoringFractureTool.h"
#include "NvBlastExtBridgeConfigs.h"

using namespace Nv::Blast;

/**
    Boolean marshalled across the C ABI. Use 0 for false and any non-zero value for true.
*/
typedef uint32_t NvBlastExtBridgeBool;

/**
    Opaque handle to an authoring session. Created by NvBlastExtBridgeSessionCreate.
*/
typedef struct NvBlastExtBridgeFractureSession NvBlastExtBridgeFractureSession;

/**
    Result codes returned by session operations.

    Values are negative so they never collide with the positive error codes the underlying
    FractureTool returns; those are surfaced as NvBlastExtBridgeSessionResult_FractureFailed.
*/
enum NvBlastExtBridgeSessionResult
{
    NvBlastExtBridgeSessionResult_Success         = 0,   //!< Operation completed
    NvBlastExtBridgeSessionResult_InvalidSession  = -1,  //!< Session handle was null
    NvBlastExtBridgeSessionResult_NoSourceMesh    = -2,  //!< No source meshes have been set yet
    NvBlastExtBridgeSessionResult_InvalidChunk    = -3,  //!< No chunk with the requested ID exists
    NvBlastExtBridgeSessionResult_InvalidArgument = -4,  //!< A parameter was out of range or null
    NvBlastExtBridgeSessionResult_FractureFailed  = -5,  //!< The underlying fracture call failed
};

/**
    Chunk state flags, reported in NvBlastExtBridgeChunkInfo::flags.
*/
enum NvBlastExtBridgeChunkFlags
{
    NvBlastExtBridgeChunkFlag_None               = 0,
    NvBlastExtBridgeChunkFlag_ApproximateBonding = 1 << 0,  //!< Produced by island split or merge; bonds are inexact
    NvBlastExtBridgeChunkFlag_IsLeaf             = 1 << 1,  //!< Chunk has no children
    NvBlastExtBridgeChunkFlag_IsChanged          = 1 << 2,  //!< Geometry changed since the last finalize
    NvBlastExtBridgeChunkFlag_IsRoot             = 1 << 3,  //!< Chunk is a source mesh (no parent)
};

/**
    Flat, marshalling-friendly view of a chunk.

    Nv::Blast::ChunkInfo cannot cross the ABI directly: it holds a Mesh* and keeps its transform
    protected. Every field here is 4 bytes wide, so the struct has the same layout under any
    reasonable packing on both sides of the boundary.
*/
struct NvBlastExtBridgeChunkInfo
{
    int32_t  chunkId;        //!< Stable identifier used to address this chunk
    int32_t  parentChunkId;  //!< Parent's chunk ID, or -1 for a source mesh
    int32_t  depth;          //!< 0 for source meshes, incrementing per subdivision level
    uint32_t flags;          //!< Bitwise OR of NvBlastExtBridgeChunkFlags

    /**
        The chunk's mesh is stored fitted to a unit cube; this transform maps it back to the space
        the source mesh was supplied in. Meshes handed out by CreateChunkMesh already have it applied.
    */
    NvcVec3 worldTranslation;
    float   worldScale;
};

/**
    Cutout fracture parameters.

    This is the full parameter set Nv::Blast::CutoutConfiguration exposes, flattened for the ABI:
    the plane is given as a point and normal rather than a transform (a tool has those directly from
    a picked surface), and the bitmap that defines the pattern is carried alongside instead of a
    prebuilt CutoutSet, which the session builds and destroys internally.
*/
struct NvBlastExtBridgeCutoutConfiguration
{
    NvcVec3 point;   //!< Point on the projection plane
    NvcVec3 normal;  //!< Projection direction; the pattern is projected along it

    /**
        8-bit-per-pixel image whose non-zero regions become the cutout loops. Must not be null.
    */
    const uint8_t* bitmap;
    uint32_t       width;
    uint32_t       height;

    /**
        Pattern size. An unscaled pattern covers (1, 1). Negative components centre the pattern on
        the chunk and scale it to the chunk's bounding box, which is the useful default.
    */
    NvcVec2 scale;

    float aperture;  //!< Conic aperture in degrees; 0 gives a cylindrical (straight) cutout

    /**
        If set, `point` is a displacement from the chunk centre rather than an absolute position.
    */
    NvBlastExtBridgeBool isRelativeTransform;

    /**
        If set, generated faces join the smoothing group of the face they were cut from.
    */
    NvBlastExtBridgeBool useSmoothing;

    /**
        Segmentation tuning for tracing loops out of the bitmap.
    */
    float               segmentationErrorThreshold;
    float               snapThreshold;
    NvBlastExtBridgeBool periodic;    //!< Treat the bitmap as tiling
    NvBlastExtBridgeBool expandGaps;  //!< Close small gaps between adjacent loops

    NoiseConfiguration noise;  //!< Surface noise for the cut faces
};

// ─── Session lifecycle ────────────────────────────────────────────────────────

/**
    Creates an authoring session holding its own FractureTool and random generator.
    \param[in] logFn Log callback, may be null.
    \return Session handle; release with NvBlastExtBridgeSessionRelease.
*/
NV_C_API NvBlastExtBridgeFractureSession* NvBlastExtBridgeSessionCreate(NvBlastLog logFn);

/**
    Destroys the session and everything it owns. Meshes returned by CreateChunkMesh and the
    AuthoringResult returned by Finalize are *not* owned by the session and outlive it.
*/
NV_C_API void NvBlastExtBridgeSessionRelease(NvBlastExtBridgeFractureSession* session);

/**
    Drops all chunks and source meshes, returning the session to its just-created state.
    Interior material ID and seed are preserved.
*/
NV_C_API void NvBlastExtBridgeSessionReset(NvBlastExtBridgeFractureSession* session);

// ─── Source meshes ────────────────────────────────────────────────────────────

/**
    Sets the meshes to fracture, discarding any existing chunk hierarchy.

    The session copies the mesh data, so the caller keeps ownership of the meshes passed in and may
    release them immediately.

    \param[in] meshes    Array of meshes, one per source object (e.g. one per submesh).
    \param[in] meshCount Number of meshes.
    \param[in] ids       Chunk IDs to assign, one per mesh. If null, IDs are allocated sequentially
                         from 0. These become the root chunk IDs.
    \return NvBlastExtBridgeSessionResult_Success on success.
*/
NV_C_API int32_t NvBlastExtBridgeSessionSetSourceMeshes(NvBlastExtBridgeFractureSession* session,
                                                       Mesh** meshes, uint32_t meshCount, const int32_t* ids);

// ─── Settings ─────────────────────────────────────────────────────────────────

/**
    Sets the seed used by every subsequent fracture operation.

    The generator is re-seeded with this value before each operation, so repeating an operation with
    the same seed and parameters reproduces the same chunks regardless of what happened in between.
    Advancing the seed is how a tool offers the artist a different random variation of the same settings.
*/
NV_C_API void NvBlastExtBridgeSessionSetSeed(NvBlastExtBridgeFractureSession* session, int32_t seed);

NV_C_API int32_t NvBlastExtBridgeSessionGetSeed(const NvBlastExtBridgeFractureSession* session);

/**
    Sets the material ID applied to newly created interior faces.
*/
NV_C_API void NvBlastExtBridgeSessionSetInteriorMaterialId(NvBlastExtBridgeFractureSession* session, int32_t materialId);

NV_C_API int32_t NvBlastExtBridgeSessionGetInteriorMaterialId(const NvBlastExtBridgeFractureSession* session);

/**
    Replaces a material ID across all existing faces.
*/
NV_C_API void NvBlastExtBridgeSessionReplaceMaterialId(NvBlastExtBridgeFractureSession* session,
                                                       int32_t oldMaterialId, int32_t newMaterialId);

/**
    Enables automatic island removal during fracturing. May cause instabilities; off by default.
*/
NV_C_API void NvBlastExtBridgeSessionSetRemoveIslands(NvBlastExtBridgeFractureSession* session,
                                                      NvBlastExtBridgeBool removeIslands);

// ─── Fracture operations ──────────────────────────────────────────────────────
//
// Each operation targets one chunk by ID and returns an NvBlastExtBridgeSessionResult.
//
// replaceChunk == false places the new chunks one depth level below the target, keeping the target
// as their parent — this is the subdivision the artist expects when drilling into a chunk.
// replaceChunk == true swaps the new chunks in for the target at its own level; it is rejected for
// root chunks, which must remain to anchor the hierarchy.
//
// Re-fracturing a chunk that already has children discards those children first, so repeating an
// operation with new settings replaces the previous result rather than compounding with it.

/**
    Fractures a chunk into uniformly distributed Voronoi cells.
*/
NV_C_API int32_t NvBlastExtBridgeSessionFractureVoronoi(NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                                        VoronoiConfiguration config,
                                                        NvBlastExtBridgeBool replaceChunk);

/**
    Fractures a chunk into Voronoi cells gathered into clusters, giving a less uniform break-up.
*/
NV_C_API int32_t NvBlastExtBridgeSessionFractureClusteredVoronoi(NvBlastExtBridgeFractureSession* session,
                                                                 int32_t chunkId,
                                                                 ClusteredVoronoiConfiguration config,
                                                                 NvBlastExtBridgeBool replaceChunk);

/**
    Fractures a chunk into Voronoi cells seeded in a sphere — useful for localized impact damage.
    \param[in] center Sphere centre, in the space the source meshes were supplied in.
*/
NV_C_API int32_t NvBlastExtBridgeSessionFractureVoronoiInSphere(NvBlastExtBridgeFractureSession* session,
                                                                int32_t chunkId, uint32_t cellCount, float radius,
                                                                NvcVec3 center, NvBlastExtBridgeBool replaceChunk);

/**
    Fractures a chunk with an explicit set of Voronoi sites, letting a tool place cells itself.
    \param[in] sites     Array of site positions, in the space the source meshes were supplied in.
    \param[in] siteCount Number of sites; must be at least 2.
*/
NV_C_API int32_t NvBlastExtBridgeSessionFractureVoronoiWithSites(NvBlastExtBridgeFractureSession* session,
                                                                 int32_t chunkId, const NvcVec3* sites,
                                                                 uint32_t siteCount,
                                                                 NvBlastExtBridgeBool replaceChunk);

/**
    Fractures a chunk with the slicing method (a noisy grid of cuts along each axis).
*/
NV_C_API int32_t NvBlastExtBridgeSessionFractureSlicing(NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                                        SlicingConfiguration config,
                                                        NvBlastExtBridgeBool replaceChunk);

/**
    Splits a chunk with a single, optionally noisy plane.
    \param[in] noise Surface noise for the cut; amplitude 0 gives a flat cut.
*/
NV_C_API int32_t NvBlastExtBridgeSessionFractureCut(NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                                    NvcVec3 normal, NvcVec3 point, NoiseConfiguration noise,
                                                    NvBlastExtBridgeBool replaceChunk);

/**
    Cuts a chunk with a 2D pattern extracted from a bitmap, projected along a normal.

    \param[in] config Pattern source and placement, see NvBlastExtBridgeCutoutConfiguration.
*/
NV_C_API int32_t NvBlastExtBridgeSessionFractureCutout(NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                                       const NvBlastExtBridgeCutoutConfiguration* config,
                                                       NvBlastExtBridgeBool replaceChunk);

/**
    Splits disconnected pieces of a chunk into separate chunks.
    \param[in] createAtNewDepth If true, islands become children; if false they replace the chunk.
    \return Number of islands found (>= 0), or a negative NvBlastExtBridgeSessionResult on failure.
*/
NV_C_API int32_t NvBlastExtBridgeSessionDetectIslands(NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                                      NvBlastExtBridgeBool createAtNewDepth);

// ─── Hierarchy queries ────────────────────────────────────────────────────────

/**
    Returns the total number of chunks, including source meshes.
*/
NV_C_API uint32_t NvBlastExtBridgeSessionGetChunkCount(const NvBlastExtBridgeFractureSession* session);

/**
    Fills the IDs of every chunk in the session.

    Two-phase call: pass null for outIds to learn the required size, then call again with a buffer.

    \param[out] outIds  Buffer to fill, or null to query the count only.
    \param[in]  maxIds  Capacity of outIds; at most this many entries are written.
    \return Total number of chunks, which may exceed maxIds.
*/
NV_C_API uint32_t NvBlastExtBridgeSessionGetChunkIds(const NvBlastExtBridgeFractureSession* session, int32_t* outIds,
                                                     uint32_t maxIds);

/**
    Fills the IDs of every chunk at a given depth. Same two-phase convention as GetChunkIds.
    \param[in] depth 0 selects the source meshes.
*/
NV_C_API uint32_t NvBlastExtBridgeSessionGetChunkIdsAtDepth(const NvBlastExtBridgeFractureSession* session,
                                                            uint32_t depth, int32_t* outIds, uint32_t maxIds);

/**
    Fills the IDs of a chunk's direct children. Same two-phase convention as GetChunkIds.
*/
NV_C_API uint32_t NvBlastExtBridgeSessionGetChildChunkIds(const NvBlastExtBridgeFractureSession* session,
                                                          int32_t chunkId, int32_t* outIds, uint32_t maxIds);

/**
    Retrieves a chunk's descriptor.
    \param[out] outInfo Filled on success; untouched otherwise.
    \return NvBlastExtBridgeSessionResult_Success, or InvalidChunk if no such chunk exists.
*/
NV_C_API int32_t NvBlastExtBridgeSessionGetChunkInfo(const NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                                     NvBlastExtBridgeChunkInfo* outInfo);

/**
    Returns a chunk's depth, or -1 if it does not exist.
*/
NV_C_API int32_t NvBlastExtBridgeSessionGetChunkDepth(const NvBlastExtBridgeFractureSession* session, int32_t chunkId);

// ─── Chunk geometry ───────────────────────────────────────────────────────────

/**
    Builds a standalone mesh for one chunk, in the space the source meshes were supplied in.

    This is how a tool previews a chunk without finalizing the whole asset. The returned mesh is
    owned by the caller — release it with NvBlastExtBridgeReleaseMesh — and is a snapshot: it does
    not track later edits to the chunk.

    \param[in] splitUVs If true, vertices are also split on differing UVs.
    \return Mesh, or null if the chunk does not exist.
*/
NV_C_API Mesh* NvBlastExtBridgeSessionCreateChunkMesh(NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                                      NvBlastExtBridgeBool splitUVs);

// ─── Hierarchy editing ────────────────────────────────────────────────────────

/**
    Deletes a chunk's descendants, and optionally the chunk itself. This is the undo for a
    subdivision: deleting the children of the chunk that was fractured restores it to a leaf.

    \param[in] deleteRoot If true the chunk is removed as well.
    \return NvBlastExtBridgeSessionResult_Success if anything was removed, InvalidChunk otherwise.
*/
NV_C_API int32_t NvBlastExtBridgeSessionDeleteChunkSubhierarchy(NvBlastExtBridgeFractureSession* session,
                                                                int32_t chunkId, NvBlastExtBridgeBool deleteRoot);

/**
    Rebalances a flat hierarchy into a tree with a bounded number of children per chunk, which the
    runtime traverses more cheaply.

    \param[in] threshold           Chunks with fewer children than this are left alone.
    \param[in] targetClusterSize   Desired number of children per processed chunk.
    \param[in] chunksToMerge       Candidate chunk IDs, or null for all chunks.
    \param[in] mergeChunkCount     Length of chunksToMerge when it is non-null.
    \param[in] removeOriginalChunks If true, merged chunks are removed.
*/
NV_C_API void NvBlastExtBridgeSessionUniteChunks(NvBlastExtBridgeFractureSession* session, uint32_t threshold,
                                                 uint32_t targetClusterSize, const uint32_t* chunksToMerge,
                                                 uint32_t mergeChunkCount,
                                                 NvBlastExtBridgeBool removeOriginalChunks);

/**
    Marks a chunk as needing approximate bond detection, which is required when its geometry did not
    come from an exact cut (island splitting, merges, externally supplied meshes).
*/
NV_C_API int32_t NvBlastExtBridgeSessionSetApproximateBonding(NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                                              NvBlastExtBridgeBool useApproximateBonding);

/**
    Rescales one chunk's interior UVs to fit a square of the given side.
*/
NV_C_API int32_t NvBlastExtBridgeSessionFitUvToRect(NvBlastExtBridgeFractureSession* session, int32_t chunkId, float side);

/**
    Rescales every chunk's interior UVs to fit a square of the given side, preserving relative sizes.
*/
NV_C_API void NvBlastExtBridgeSessionFitAllUvToRect(NvBlastExtBridgeFractureSession* session, float side);

// ─── Support graph ────────────────────────────────────────────────────────────
//
// Which chunks are support chunks — the level the simulation treats as breakable — follows the
// depth rule given to Finalize. What that rule cannot express is anchoring: without a bond to the
// world, a structure has nothing holding it up and collapses on the first simulated frame.
//
// Marking a chunk static gives it that bond. Only support chunks can carry one, so a chunk marked
// static that the depth rule did not make support is reported and skipped at finalize time —
// use IsChunkSupport to check before marking.

/**
    Marks a chunk as anchored to the world, or clears the mark.

    The mark is remembered per chunk ID and survives fracturing elsewhere in the hierarchy; marks on
    chunks that are later deleted are simply never applied.

    \return NvBlastExtBridgeSessionResult_Success, or InvalidChunk if no such chunk exists.
*/
NV_C_API int32_t NvBlastExtBridgeSessionSetChunkStatic(NvBlastExtBridgeFractureSession* session, int32_t chunkId,
                                                       NvBlastExtBridgeBool isStatic);

/**
    Returns non-zero if the chunk is marked as anchored to the world.
*/
NV_C_API NvBlastExtBridgeBool NvBlastExtBridgeSessionGetChunkStatic(const NvBlastExtBridgeFractureSession* session,
                                                                   int32_t chunkId);

/**
    Fills the IDs of every chunk marked static. Same two-phase convention as GetChunkIds.
*/
NV_C_API uint32_t NvBlastExtBridgeSessionGetStaticChunkIds(const NvBlastExtBridgeFractureSession* session,
                                                           int32_t* outIds, uint32_t maxIds);

/**
    Clears every static mark.
*/
NV_C_API void NvBlastExtBridgeSessionClearStaticChunks(NvBlastExtBridgeFractureSession* session);

/**
    Reports whether a chunk would be a support chunk under the given depth rule.

    This mirrors what Finalize will decide, so a tool can show the artist which chunks form the
    support layer — and therefore which ones can be anchored — without finalizing first.

    \param[in] defaultSupportDepth Same value that will be passed to Finalize; -1 makes leaves support.
    \return Non-zero if the chunk would be a support chunk.
*/
NV_C_API NvBlastExtBridgeBool NvBlastExtBridgeSessionIsChunkSupport(const NvBlastExtBridgeFractureSession* session,
                                                                   int32_t chunkId, int32_t defaultSupportDepth);

/**
    Sets the direction of the bonds tying static chunks to the world. Defaults to (0, -1, 0), which
    reads as "held from below" for an object standing on the ground.
*/
NV_C_API void NvBlastExtBridgeSessionSetWorldBondDirection(NvBlastExtBridgeFractureSession* session,
                                                           NvcVec3 direction);

// ─── Finalize ─────────────────────────────────────────────────────────────────

/**
    Builds the Blast asset from the current hierarchy: generates bonds, builds collision geometry,
    and produces render meshes.

    The session stays usable afterwards, so a tool can finalize for preview, keep editing, and
    finalize again.

    \param[in] collisionBuilder   Builder for collision hulls, from NvBlastExtBridgeCreateCollisionBuilder.
    \param[in] aggregateMaxCount  Maximum convex hulls per chunk; values below 1 are treated as 1.
    \param[in] defaultSupportDepth Depth at which chunks become support chunks — the level the
                                  simulation treats as the breakable unit. Pass -1 to instead make
                                  every leaf a support chunk, which is the usual default.
    \return AuthoringResult owned by the caller. Release it with NvBlastExtBridgeReleaseAuthoringResult
            *before* releasing the collision builder. Null on failure.
*/
NV_C_API AuthoringResult* NvBlastExtBridgeSessionFinalize(NvBlastExtBridgeFractureSession* session,
                                                          ConvexMeshBuilder* collisionBuilder,
                                                          uint32_t aggregateMaxCount, int32_t defaultSupportDepth);

#endif  // ifndef NVBLASTEXTBRIDGESESSION_H
