//! @file
//!
//! @brief Persistent authoring session API — the interactive counterpart to the one-shot
//!        fracture calls in NvBlastExtUnity.h.
//!
//! The one-shot API builds a FractureTool, fractures, and tears it down inside a single call, so
//! every operation starts from the source mesh. An authoring tool needs the opposite: the artist
//! selects a chunk, fractures it, inspects the result, undoes it, fractures it differently, then
//! moves to a sibling. That requires the FractureTool to outlive individual operations, which is
//! what a session is.
//!
//! Usage:
//!
//!     session = NvBlastExtUnitySessionCreate(logFn);
//!     NvBlastExtUnitySessionSetSourceMeshes(session, meshes, count, ids);
//!     NvBlastExtUnitySessionFractureVoronoi(session, 0, voronoiConfig, false);   // depth 1
//!     NvBlastExtUnitySessionFractureVoronoi(session, 3, voronoiConfig, false);   // subdivide one chunk
//!     NvBlastExtUnitySessionDeleteChunkSubhierarchy(session, 3, false);          // undo that
//!     result = NvBlastExtUnitySessionFinalize(session, collisionBuilder, 1);
//!     NvBlastExtUnitySessionRelease(session);
//!
//! Conventions:
//!
//! - Chunks are addressed by **chunk ID**, never by info index. IDs are stable across operations;
//!   info indices are positions in an internal array and shift whenever chunks are added or removed.
//! - Booleans cross the ABI as NvBlastExtUnityBool (uint32_t, 0 or 1). C# `bool` marshals as a
//!   4-byte BOOL by default but C++ `bool` is 1 byte, and the mismatch is silent — a fixed-width
//!   integer removes the trap entirely. The same reason drives the flags field on chunk info.
//! - Every entry point tolerates a null session and returns a failure code rather than crashing.

#ifndef NVBLASTEXTUNITYSESSION_H
#define NVBLASTEXTUNITYSESSION_H

#include "NvBlastGlobals.h"
#include "NvBlastExtAuthoring.h"
#include "NvBlastExtAuthoringMesh.h"
#include "NvBlastExtAuthoringFractureTool.h"
#include "NvBlastExtUnityConfigs.h"

using namespace Nv::Blast;

/**
    Boolean marshalled across the C ABI. Use 0 for false and any non-zero value for true.
*/
typedef uint32_t NvBlastExtUnityBool;

/**
    Opaque handle to an authoring session. Created by NvBlastExtUnitySessionCreate.
*/
typedef struct NvBlastExtUnityFractureSession NvBlastExtUnityFractureSession;

/**
    Result codes returned by session operations.

    Values are negative so they never collide with the positive error codes the underlying
    FractureTool returns; those are surfaced as NvBlastExtUnitySessionResult_FractureFailed.
*/
enum NvBlastExtUnitySessionResult
{
    NvBlastExtUnitySessionResult_Success         = 0,   //!< Operation completed
    NvBlastExtUnitySessionResult_InvalidSession  = -1,  //!< Session handle was null
    NvBlastExtUnitySessionResult_NoSourceMesh    = -2,  //!< No source meshes have been set yet
    NvBlastExtUnitySessionResult_InvalidChunk    = -3,  //!< No chunk with the requested ID exists
    NvBlastExtUnitySessionResult_InvalidArgument = -4,  //!< A parameter was out of range or null
    NvBlastExtUnitySessionResult_FractureFailed  = -5,  //!< The underlying fracture call failed
};

/**
    Chunk state flags, reported in NvBlastExtUnityChunkInfo::flags.
*/
enum NvBlastExtUnityChunkFlags
{
    NvBlastExtUnityChunkFlag_None               = 0,
    NvBlastExtUnityChunkFlag_ApproximateBonding = 1 << 0,  //!< Produced by island split or merge; bonds are inexact
    NvBlastExtUnityChunkFlag_IsLeaf             = 1 << 1,  //!< Chunk has no children
    NvBlastExtUnityChunkFlag_IsChanged          = 1 << 2,  //!< Geometry changed since the last finalize
    NvBlastExtUnityChunkFlag_IsRoot             = 1 << 3,  //!< Chunk is a source mesh (no parent)
};

/**
    Flat, marshalling-friendly view of a chunk.

    Nv::Blast::ChunkInfo cannot cross the ABI directly: it holds a Mesh* and keeps its transform
    protected. Every field here is 4 bytes wide, so the struct has the same layout under any
    reasonable packing on both sides of the boundary.
*/
struct NvBlastExtUnityChunkInfo
{
    int32_t  chunkId;        //!< Stable identifier used to address this chunk
    int32_t  parentChunkId;  //!< Parent's chunk ID, or -1 for a source mesh
    int32_t  depth;          //!< 0 for source meshes, incrementing per subdivision level
    uint32_t flags;          //!< Bitwise OR of NvBlastExtUnityChunkFlags

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
struct NvBlastExtUnityCutoutConfiguration
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
    NvBlastExtUnityBool isRelativeTransform;

    /**
        If set, generated faces join the smoothing group of the face they were cut from.
    */
    NvBlastExtUnityBool useSmoothing;

    /**
        Segmentation tuning for tracing loops out of the bitmap.
    */
    float               segmentationErrorThreshold;
    float               snapThreshold;
    NvBlastExtUnityBool periodic;    //!< Treat the bitmap as tiling
    NvBlastExtUnityBool expandGaps;  //!< Close small gaps between adjacent loops

    NoiseConfiguration noise;  //!< Surface noise for the cut faces
};

// ─── Session lifecycle ────────────────────────────────────────────────────────

/**
    Creates an authoring session holding its own FractureTool and random generator.
    \param[in] logFn Log callback, may be null.
    \return Session handle; release with NvBlastExtUnitySessionRelease.
*/
NV_C_API NvBlastExtUnityFractureSession* NvBlastExtUnitySessionCreate(NvBlastLog logFn);

/**
    Destroys the session and everything it owns. Meshes returned by CreateChunkMesh and the
    AuthoringResult returned by Finalize are *not* owned by the session and outlive it.
*/
NV_C_API void NvBlastExtUnitySessionRelease(NvBlastExtUnityFractureSession* session);

/**
    Drops all chunks and source meshes, returning the session to its just-created state.
    Interior material ID and seed are preserved.
*/
NV_C_API void NvBlastExtUnitySessionReset(NvBlastExtUnityFractureSession* session);

// ─── Source meshes ────────────────────────────────────────────────────────────

/**
    Sets the meshes to fracture, discarding any existing chunk hierarchy.

    The session copies the mesh data, so the caller keeps ownership of the meshes passed in and may
    release them immediately.

    \param[in] meshes    Array of meshes, one per source object (e.g. one per submesh).
    \param[in] meshCount Number of meshes.
    \param[in] ids       Chunk IDs to assign, one per mesh. If null, IDs are allocated sequentially
                         from 0. These become the root chunk IDs.
    \return NvBlastExtUnitySessionResult_Success on success.
*/
NV_C_API int32_t NvBlastExtUnitySessionSetSourceMeshes(NvBlastExtUnityFractureSession* session,
                                                       Mesh** meshes, uint32_t meshCount, const int32_t* ids);

// ─── Settings ─────────────────────────────────────────────────────────────────

/**
    Sets the seed used by every subsequent fracture operation.

    The generator is re-seeded with this value before each operation, so repeating an operation with
    the same seed and parameters reproduces the same chunks regardless of what happened in between.
    Advancing the seed is how a tool offers the artist a different random variation of the same settings.
*/
NV_C_API void NvBlastExtUnitySessionSetSeed(NvBlastExtUnityFractureSession* session, int32_t seed);

NV_C_API int32_t NvBlastExtUnitySessionGetSeed(const NvBlastExtUnityFractureSession* session);

/**
    Sets the material ID applied to newly created interior faces.
*/
NV_C_API void NvBlastExtUnitySessionSetInteriorMaterialId(NvBlastExtUnityFractureSession* session, int32_t materialId);

NV_C_API int32_t NvBlastExtUnitySessionGetInteriorMaterialId(const NvBlastExtUnityFractureSession* session);

/**
    Replaces a material ID across all existing faces.
*/
NV_C_API void NvBlastExtUnitySessionReplaceMaterialId(NvBlastExtUnityFractureSession* session,
                                                       int32_t oldMaterialId, int32_t newMaterialId);

/**
    Enables automatic island removal during fracturing. May cause instabilities; off by default.
*/
NV_C_API void NvBlastExtUnitySessionSetRemoveIslands(NvBlastExtUnityFractureSession* session,
                                                      NvBlastExtUnityBool removeIslands);

// ─── Fracture operations ──────────────────────────────────────────────────────
//
// Each operation targets one chunk by ID and returns an NvBlastExtUnitySessionResult.
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
NV_C_API int32_t NvBlastExtUnitySessionFractureVoronoi(NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                                        VoronoiConfiguration config,
                                                        NvBlastExtUnityBool replaceChunk);

/**
    Fractures a chunk into Voronoi cells gathered into clusters, giving a less uniform break-up.
*/
NV_C_API int32_t NvBlastExtUnitySessionFractureClusteredVoronoi(NvBlastExtUnityFractureSession* session,
                                                                 int32_t chunkId,
                                                                 ClusteredVoronoiConfiguration config,
                                                                 NvBlastExtUnityBool replaceChunk);

/**
    Fractures a chunk into Voronoi cells seeded in a sphere — useful for localized impact damage.
    \param[in] center Sphere centre, in the space the source meshes were supplied in.
*/
NV_C_API int32_t NvBlastExtUnitySessionFractureVoronoiInSphere(NvBlastExtUnityFractureSession* session,
                                                                int32_t chunkId, uint32_t cellCount, float radius,
                                                                NvcVec3 center, NvBlastExtUnityBool replaceChunk);

/**
    Fractures a chunk with an explicit set of Voronoi sites, letting a tool place cells itself.
    \param[in] sites     Array of site positions, in the space the source meshes were supplied in.
    \param[in] siteCount Number of sites; must be at least 2.
*/
NV_C_API int32_t NvBlastExtUnitySessionFractureVoronoiWithSites(NvBlastExtUnityFractureSession* session,
                                                                 int32_t chunkId, const NvcVec3* sites,
                                                                 uint32_t siteCount,
                                                                 NvBlastExtUnityBool replaceChunk);

/**
    Fractures a chunk with the slicing method (a noisy grid of cuts along each axis).
*/
NV_C_API int32_t NvBlastExtUnitySessionFractureSlicing(NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                                        SlicingConfiguration config,
                                                        NvBlastExtUnityBool replaceChunk);

/**
    Splits a chunk with a single, optionally noisy plane.
    \param[in] noise Surface noise for the cut; amplitude 0 gives a flat cut.
*/
NV_C_API int32_t NvBlastExtUnitySessionFractureCut(NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                                    NvcVec3 normal, NvcVec3 point, NoiseConfiguration noise,
                                                    NvBlastExtUnityBool replaceChunk);

/**
    Cuts a chunk with a 2D pattern extracted from a bitmap, projected along a normal.

    \param[in] config Pattern source and placement, see NvBlastExtUnityCutoutConfiguration.
*/
NV_C_API int32_t NvBlastExtUnitySessionFractureCutout(NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                                       const NvBlastExtUnityCutoutConfiguration* config,
                                                       NvBlastExtUnityBool replaceChunk);

/**
    Splits disconnected pieces of a chunk into separate chunks.
    \param[in] createAtNewDepth If true, islands become children; if false they replace the chunk.
    \return Number of islands found (>= 0), or a negative NvBlastExtUnitySessionResult on failure.
*/
NV_C_API int32_t NvBlastExtUnitySessionDetectIslands(NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                                      NvBlastExtUnityBool createAtNewDepth);

// ─── Hierarchy queries ────────────────────────────────────────────────────────

/**
    Returns the total number of chunks, including source meshes.
*/
NV_C_API uint32_t NvBlastExtUnitySessionGetChunkCount(const NvBlastExtUnityFractureSession* session);

/**
    Fills the IDs of every chunk in the session.

    Two-phase call: pass null for outIds to learn the required size, then call again with a buffer.

    \param[out] outIds  Buffer to fill, or null to query the count only.
    \param[in]  maxIds  Capacity of outIds; at most this many entries are written.
    \return Total number of chunks, which may exceed maxIds.
*/
NV_C_API uint32_t NvBlastExtUnitySessionGetChunkIds(const NvBlastExtUnityFractureSession* session, int32_t* outIds,
                                                     uint32_t maxIds);

/**
    Fills the IDs of every chunk at a given depth. Same two-phase convention as GetChunkIds.
    \param[in] depth 0 selects the source meshes.
*/
NV_C_API uint32_t NvBlastExtUnitySessionGetChunkIdsAtDepth(const NvBlastExtUnityFractureSession* session,
                                                            uint32_t depth, int32_t* outIds, uint32_t maxIds);

/**
    Fills the IDs of a chunk's direct children. Same two-phase convention as GetChunkIds.
*/
NV_C_API uint32_t NvBlastExtUnitySessionGetChildChunkIds(const NvBlastExtUnityFractureSession* session,
                                                          int32_t chunkId, int32_t* outIds, uint32_t maxIds);

/**
    Retrieves a chunk's descriptor.
    \param[out] outInfo Filled on success; untouched otherwise.
    \return NvBlastExtUnitySessionResult_Success, or InvalidChunk if no such chunk exists.
*/
NV_C_API int32_t NvBlastExtUnitySessionGetChunkInfo(const NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                                     NvBlastExtUnityChunkInfo* outInfo);

/**
    Returns a chunk's depth, or -1 if it does not exist.
*/
NV_C_API int32_t NvBlastExtUnitySessionGetChunkDepth(const NvBlastExtUnityFractureSession* session, int32_t chunkId);

// ─── Chunk geometry ───────────────────────────────────────────────────────────

/**
    Builds a standalone mesh for one chunk, in the space the source meshes were supplied in.

    This is how a tool previews a chunk without finalizing the whole asset. The returned mesh is
    owned by the caller — release it with NvBlastExtUnityReleaseMesh — and is a snapshot: it does
    not track later edits to the chunk.

    \param[in] splitUVs If true, vertices are also split on differing UVs.
    \return Mesh, or null if the chunk does not exist.
*/
NV_C_API Mesh* NvBlastExtUnitySessionCreateChunkMesh(NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                                      NvBlastExtUnityBool splitUVs);

// ─── Hierarchy editing ────────────────────────────────────────────────────────

/**
    Deletes a chunk's descendants, and optionally the chunk itself. This is the undo for a
    subdivision: deleting the children of the chunk that was fractured restores it to a leaf.

    \param[in] deleteRoot If true the chunk is removed as well.
    \return NvBlastExtUnitySessionResult_Success if anything was removed, InvalidChunk otherwise.
*/
NV_C_API int32_t NvBlastExtUnitySessionDeleteChunkSubhierarchy(NvBlastExtUnityFractureSession* session,
                                                                int32_t chunkId, NvBlastExtUnityBool deleteRoot);

/**
    Rebalances a flat hierarchy into a tree with a bounded number of children per chunk, which the
    runtime traverses more cheaply.

    \param[in] threshold           Chunks with fewer children than this are left alone.
    \param[in] targetClusterSize   Desired number of children per processed chunk.
    \param[in] chunksToMerge       Candidate chunk IDs, or null for all chunks.
    \param[in] mergeChunkCount     Length of chunksToMerge when it is non-null.
    \param[in] removeOriginalChunks If true, merged chunks are removed.
*/
NV_C_API void NvBlastExtUnitySessionUniteChunks(NvBlastExtUnityFractureSession* session, uint32_t threshold,
                                                 uint32_t targetClusterSize, const uint32_t* chunksToMerge,
                                                 uint32_t mergeChunkCount,
                                                 NvBlastExtUnityBool removeOriginalChunks);

/**
    Marks a chunk as needing approximate bond detection, which is required when its geometry did not
    come from an exact cut (island splitting, merges, externally supplied meshes).
*/
NV_C_API int32_t NvBlastExtUnitySessionSetApproximateBonding(NvBlastExtUnityFractureSession* session, int32_t chunkId,
                                                              NvBlastExtUnityBool useApproximateBonding);

/**
    Rescales one chunk's interior UVs to fit a square of the given side.
*/
NV_C_API int32_t NvBlastExtUnitySessionFitUvToRect(NvBlastExtUnityFractureSession* session, int32_t chunkId, float side);

/**
    Rescales every chunk's interior UVs to fit a square of the given side, preserving relative sizes.
*/
NV_C_API void NvBlastExtUnitySessionFitAllUvToRect(NvBlastExtUnityFractureSession* session, float side);

// ─── Finalize ─────────────────────────────────────────────────────────────────

/**
    Builds the Blast asset from the current hierarchy: generates bonds, builds collision geometry,
    and produces render meshes.

    The session stays usable afterwards, so a tool can finalize for preview, keep editing, and
    finalize again.

    \param[in] collisionBuilder   Builder for collision hulls, from NvBlastExtUnityCreateCollisionBuilder.
    \param[in] aggregateMaxCount  Maximum convex hulls per chunk; values below 1 are treated as 1.
    \param[in] defaultSupportDepth Depth at which chunks become support chunks — the level the
                                  simulation treats as the breakable unit. Pass -1 to instead make
                                  every leaf a support chunk, which is the usual default.
    \return AuthoringResult owned by the caller. Release it with NvBlastExtUnityReleaseAuthoringResult
            *before* releasing the collision builder. Null on failure.
*/
NV_C_API AuthoringResult* NvBlastExtUnitySessionFinalize(NvBlastExtUnityFractureSession* session,
                                                          ConvexMeshBuilder* collisionBuilder,
                                                          uint32_t aggregateMaxCount, int32_t defaultSupportDepth);

#endif  // ifndef NVBLASTEXTUNITYSESSION_H
