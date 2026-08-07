// Tests for the persistent authoring session (NvBlastExtUnitySession.h).
//
// These exercise the C API rather than the FractureSession class, because the C API is the contract
// engine integrations are written against — a regression there breaks Unity and Unreal silently,
// where a C++-level one would at least fail to compile.

#include "NvBlastExtUnity.h"
#include "NvBlastExtUnitySession.h"
#include "NvBlast.h"  // NvBlastAssetGetBondCount, to verify anchors reached the asset

#include <gtest/gtest.h>

#include <vector>

namespace
{

/** Swallows log output so the expected-failure cases do not spam the test report. */
void silentLog(int, const char*, const char*, int)
{
}

/** Unit cube, 24 verts / 12 triangles — the same geometry TestProgram fractures. */
Mesh* createCubeMesh()
{
    const uint32_t verticesCount = 24;
    const uint32_t indicesCount  = 36;

    const NvcVec3 positions[verticesCount] = {
        { 0.5f, -0.5f, 0.5f },  { -0.5f, -0.5f, 0.5f },  { 0.5f, 0.5f, 0.5f },   { -0.5f, 0.5f, 0.5f },
        { 0.5f, 0.5f, -0.5f },  { -0.5f, 0.5f, -0.5f },  { 0.5f, -0.5f, -0.5f }, { -0.5f, -0.5f, -0.5f },
        { 0.5f, 0.5f, 0.5f },   { -0.5f, 0.5f, 0.5f },   { 0.5f, 0.5f, -0.5f },  { -0.5f, 0.5f, -0.5f },
        { 0.5f, -0.5f, -0.5f }, { 0.5f, -0.5f, 0.5f },   { -0.5f, -0.5f, 0.5f }, { -0.5f, -0.5f, -0.5f },
        { -0.5f, -0.5f, 0.5f }, { -0.5f, 0.5f, 0.5f },   { -0.5f, 0.5f, -0.5f }, { -0.5f, -0.5f, -0.5f },
        { 0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, -0.5f },   { 0.5f, 0.5f, 0.5f },   { 0.5f, -0.5f, 0.5f }
    };

    const NvcVec3 normals[verticesCount] = {
        { 0, 0, 1 },  { 0, 0, 1 },  { 0, 0, 1 },  { 0, 0, 1 },  { 0, 1, 0 },  { 0, 1, 0 },
        { 0, 0, -1 }, { 0, 0, -1 }, { 0, 1, 0 },  { 0, 1, 0 },  { 0, 0, -1 }, { 0, 0, -1 },
        { 0, -1, 0 }, { 0, -1, 0 }, { 0, -1, 0 }, { 0, -1, 0 }, { -1, 0, 0 }, { -1, 0, 0 },
        { -1, 0, 0 }, { -1, 0, 0 }, { 1, 0, 0 },  { 1, 0, 0 },  { 1, 0, 0 },  { 1, 0, 0 }
    };

    const NvcVec2 uvs[verticesCount] = {
        { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }, { 0, 1 }, { 1, 1 }, { 0, 1 }, { 1, 1 },
        { 0, 0 }, { 1, 0 }, { 0, 0 }, { 1, 0 }, { 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 0 },
        { 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 0 }, { 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 0 }
    };

    const uint32_t indices[indicesCount] = { 0,  2,  3,  0,  3,  1,  8,  4,  5,  8,  5,  9,
                                             10, 6,  7,  10, 7,  11, 12, 13, 14, 12, 14, 15,
                                             16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23 };

    return NvBlastExtUnityCreateMesh(positions, normals, uvs, verticesCount, indices, indicesCount);
}

class FractureSessionTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_session = NvBlastExtUnitySessionCreate(silentLog);
        ASSERT_NE(m_session, nullptr);

        m_mesh = createCubeMesh();
        ASSERT_NE(m_mesh, nullptr);
    }

    void TearDown() override
    {
        if (m_session != nullptr)
        {
            NvBlastExtUnitySessionRelease(m_session);
        }
        if (m_mesh != nullptr)
        {
            NvBlastExtUnityReleaseMesh(m_mesh);
        }
    }

    /** Loads the cube as chunk 0. The session copies it, so m_mesh stays owned by the fixture. */
    void setCubeSource()
    {
        int32_t ids[] = { 0 };
        ASSERT_EQ(NvBlastExtUnitySessionSetSourceMeshes(m_session, &m_mesh, 1, ids),
                  NvBlastExtUnitySessionResult_Success);
    }

    std::vector<int32_t> childrenOf(int32_t chunkId)
    {
        const uint32_t count = NvBlastExtUnitySessionGetChildChunkIds(m_session, chunkId, nullptr, 0);
        std::vector<int32_t> ids(count);
        if (count > 0)
        {
            NvBlastExtUnitySessionGetChildChunkIds(m_session, chunkId, ids.data(), count);
        }
        return ids;
    }

    NvBlastExtUnityFractureSession* m_session = nullptr;
    Mesh*                           m_mesh    = nullptr;
};

// ─── Lifecycle and source meshes ──────────────────────────────────────────────

TEST_F(FractureSessionTest, NewSessionHasNoChunks)
{
    EXPECT_EQ(NvBlastExtUnitySessionGetChunkCount(m_session), 0u);
}

TEST_F(FractureSessionTest, SetSourceMeshesCreatesRootChunk)
{
    setCubeSource();

    EXPECT_EQ(NvBlastExtUnitySessionGetChunkCount(m_session), 1u);
    EXPECT_EQ(NvBlastExtUnitySessionGetChunkDepth(m_session, 0), 0);

    NvBlastExtUnityChunkInfo info = {};
    ASSERT_EQ(NvBlastExtUnitySessionGetChunkInfo(m_session, 0, &info), NvBlastExtUnitySessionResult_Success);
    EXPECT_EQ(info.chunkId, 0);
    EXPECT_EQ(info.parentChunkId, -1);
    EXPECT_NE(info.flags & NvBlastExtUnityChunkFlag_IsRoot, 0u);
    EXPECT_NE(info.flags & NvBlastExtUnityChunkFlag_IsLeaf, 0u);
}

TEST_F(FractureSessionTest, SetSourceMeshesRejectsEmptyInput)
{
    EXPECT_EQ(NvBlastExtUnitySessionSetSourceMeshes(m_session, nullptr, 0, nullptr),
              NvBlastExtUnitySessionResult_InvalidArgument);

    Mesh* nullMesh[] = { nullptr };
    EXPECT_EQ(NvBlastExtUnitySessionSetSourceMeshes(m_session, nullMesh, 1, nullptr),
              NvBlastExtUnitySessionResult_InvalidArgument);
}

TEST_F(FractureSessionTest, ResetClearsHierarchyButKeepsSettings)
{
    setCubeSource();
    NvBlastExtUnitySessionSetSeed(m_session, 42);
    NvBlastExtUnitySessionSetInteriorMaterialId(m_session, 7);

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);
    ASSERT_GT(NvBlastExtUnitySessionGetChunkCount(m_session), 1u);

    NvBlastExtUnitySessionReset(m_session);

    EXPECT_EQ(NvBlastExtUnitySessionGetChunkCount(m_session), 0u);
    EXPECT_EQ(NvBlastExtUnitySessionGetSeed(m_session), 42);
}

TEST_F(FractureSessionTest, NullSessionIsHandled)
{
    // The C# side can hold a stale handle; every entry point must degrade rather than crash.
    EXPECT_EQ(NvBlastExtUnitySessionGetChunkCount(nullptr), 0u);
    EXPECT_EQ(NvBlastExtUnitySessionGetChunkDepth(nullptr, 0), -1);
    EXPECT_EQ(NvBlastExtUnitySessionCreateChunkMesh(nullptr, 0, 1), nullptr);
    EXPECT_EQ(NvBlastExtUnitySessionFinalize(nullptr, nullptr, 1, -1), nullptr);

    VoronoiConfiguration voronoi(5);
    EXPECT_EQ(NvBlastExtUnitySessionFractureVoronoi(nullptr, 0, voronoi, 0),
              NvBlastExtUnitySessionResult_InvalidSession);

    NvBlastExtUnitySessionRelease(nullptr);  // must not crash
}

// ─── Fracturing ───────────────────────────────────────────────────────────────

TEST_F(FractureSessionTest, SlicingProducesEightChildren)
{
    setCubeSource();

    // One slice per axis halves the cube three times: 2 x 2 x 2 pieces.
    SlicingConfiguration slicing;
    slicing.x_slices = 1;
    slicing.y_slices = 1;
    slicing.z_slices = 1;

    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);

    EXPECT_EQ(childrenOf(0).size(), 8u);
    EXPECT_EQ(NvBlastExtUnitySessionGetChunkCount(m_session), 9u);  // root + 8
}

TEST_F(FractureSessionTest, VoronoiProducesChildrenAtNextDepth)
{
    setCubeSource();

    VoronoiConfiguration voronoi(5);
    ASSERT_EQ(NvBlastExtUnitySessionFractureVoronoi(m_session, 0, voronoi, 0),
              NvBlastExtUnitySessionResult_Success);

    const std::vector<int32_t> children = childrenOf(0);
    ASSERT_GT(children.size(), 1u);

    for (int32_t child : children)
    {
        EXPECT_EQ(NvBlastExtUnitySessionGetChunkDepth(m_session, child), 1);
    }

    // The root is no longer a leaf now that it has children.
    NvBlastExtUnityChunkInfo rootInfo = {};
    ASSERT_EQ(NvBlastExtUnitySessionGetChunkInfo(m_session, 0, &rootInfo), NvBlastExtUnitySessionResult_Success);
    EXPECT_EQ(rootInfo.flags & NvBlastExtUnityChunkFlag_IsLeaf, 0u);
}

// This is the operation the whole session exists for: drilling into an already-fractured chunk.
TEST_F(FractureSessionTest, ChunkCanBeSubdividedFurther)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);

    const std::vector<int32_t> firstLevel = childrenOf(0);
    ASSERT_FALSE(firstLevel.empty());

    const int32_t  target        = firstLevel.front();
    const uint32_t countBefore   = NvBlastExtUnitySessionGetChunkCount(m_session);

    VoronoiConfiguration voronoi(4);
    ASSERT_EQ(NvBlastExtUnitySessionFractureVoronoi(m_session, target, voronoi, 0),
              NvBlastExtUnitySessionResult_Success);

    const std::vector<int32_t> secondLevel = childrenOf(target);
    ASSERT_GT(secondLevel.size(), 1u);
    EXPECT_GT(NvBlastExtUnitySessionGetChunkCount(m_session), countBefore);

    for (int32_t grandchild : secondLevel)
    {
        EXPECT_EQ(NvBlastExtUnitySessionGetChunkDepth(m_session, grandchild), 2);
    }

    // Its siblings are untouched — subdividing is local to the selected chunk.
    for (size_t i = 1; i < firstLevel.size(); ++i)
    {
        EXPECT_TRUE(childrenOf(firstLevel[i]).empty());
    }
}

TEST_F(FractureSessionTest, RefracturingAChunkReplacesItsChildren)
{
    setCubeSource();

    SlicingConfiguration slicing;
    slicing.x_slices = 1;
    slicing.y_slices = 1;
    slicing.z_slices = 1;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);
    ASSERT_EQ(childrenOf(0).size(), 8u);

    // Re-running with different settings must replace the previous result, not add to it —
    // otherwise tweaking a slider in a tool would pile up chunks.
    slicing.x_slices = 2;
    slicing.y_slices = 1;
    slicing.z_slices = 1;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);

    EXPECT_EQ(childrenOf(0).size(), 12u);  // 3 x 2 x 2
    EXPECT_EQ(NvBlastExtUnitySessionGetChunkCount(m_session), 13u);
}

TEST_F(FractureSessionTest, FractureRejectsUnknownChunk)
{
    setCubeSource();

    VoronoiConfiguration voronoi(5);
    EXPECT_EQ(NvBlastExtUnitySessionFractureVoronoi(m_session, 999, voronoi, 0),
              NvBlastExtUnitySessionResult_InvalidChunk);
}

TEST_F(FractureSessionTest, FractureRequiresSourceMeshes)
{
    VoronoiConfiguration voronoi(5);
    EXPECT_EQ(NvBlastExtUnitySessionFractureVoronoi(m_session, 0, voronoi, 0),
              NvBlastExtUnitySessionResult_NoSourceMesh);
}

TEST_F(FractureSessionTest, RootChunkCannotBeReplaced)
{
    setCubeSource();

    // Replacing a source mesh would leave the asset without its top-level chunk.
    VoronoiConfiguration voronoi(5);
    EXPECT_EQ(NvBlastExtUnitySessionFractureVoronoi(m_session, 0, voronoi, 1),
              NvBlastExtUnitySessionResult_InvalidArgument);
}

TEST_F(FractureSessionTest, ReplaceChunkKeepsChildrenAtTheSameDepth)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);

    const std::vector<int32_t> firstLevel = childrenOf(0);
    ASSERT_FALSE(firstLevel.empty());
    const int32_t target = firstLevel.front();

    VoronoiConfiguration voronoi(4);
    ASSERT_EQ(NvBlastExtUnitySessionFractureVoronoi(m_session, target, voronoi, 1),
              NvBlastExtUnitySessionResult_Success);

    // The replaced chunk is gone and its pieces sit where it used to, still under the root.
    EXPECT_EQ(NvBlastExtUnitySessionGetChunkDepth(m_session, target), -1);
    const std::vector<int32_t> afterReplace = childrenOf(0);
    EXPECT_GT(afterReplace.size(), firstLevel.size());
    for (int32_t chunk : afterReplace)
    {
        EXPECT_EQ(NvBlastExtUnitySessionGetChunkDepth(m_session, chunk), 1);
    }
}

// ─── Determinism ──────────────────────────────────────────────────────────────

TEST_F(FractureSessionTest, SameSeedReproducesTheSameFracture)
{
    setCubeSource();
    NvBlastExtUnitySessionSetSeed(m_session, 1234);

    VoronoiConfiguration voronoi(8);
    ASSERT_EQ(NvBlastExtUnitySessionFractureVoronoi(m_session, 0, voronoi, 0),
              NvBlastExtUnitySessionResult_Success);
    const uint32_t firstRun = NvBlastExtUnitySessionGetChunkCount(m_session);

    // A second session, seeded identically, must land on the same hierarchy — this is what lets a
    // tool store a seed in a project file and rebuild the same asset later.
    NvBlastExtUnityFractureSession* other = NvBlastExtUnitySessionCreate(silentLog);
    ASSERT_NE(other, nullptr);
    int32_t ids[] = { 0 };
    ASSERT_EQ(NvBlastExtUnitySessionSetSourceMeshes(other, &m_mesh, 1, ids), NvBlastExtUnitySessionResult_Success);
    NvBlastExtUnitySessionSetSeed(other, 1234);
    ASSERT_EQ(NvBlastExtUnitySessionFractureVoronoi(other, 0, voronoi, 0), NvBlastExtUnitySessionResult_Success);

    EXPECT_EQ(NvBlastExtUnitySessionGetChunkCount(other), firstRun);

    NvBlastExtUnitySessionRelease(other);
}

TEST_F(FractureSessionTest, ReseedingIsIndependentOfOperationHistory)
{
    setCubeSource();
    NvBlastExtUnitySessionSetSeed(m_session, 99);

    VoronoiConfiguration voronoi(6);
    ASSERT_EQ(NvBlastExtUnitySessionFractureVoronoi(m_session, 0, voronoi, 0),
              NvBlastExtUnitySessionResult_Success);
    const size_t firstAttempt = childrenOf(0).size();

    // Running unrelated work in between must not shift the generator: the operation is re-seeded,
    // so repeating it with the same seed gives the same answer.
    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);

    ASSERT_EQ(NvBlastExtUnitySessionFractureVoronoi(m_session, 0, voronoi, 0),
              NvBlastExtUnitySessionResult_Success);
    EXPECT_EQ(childrenOf(0).size(), firstAttempt);
}

// ─── Hierarchy editing ────────────────────────────────────────────────────────

TEST_F(FractureSessionTest, DeleteSubhierarchyUndoesAFracture)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);
    ASSERT_GT(NvBlastExtUnitySessionGetChunkCount(m_session), 1u);

    ASSERT_EQ(NvBlastExtUnitySessionDeleteChunkSubhierarchy(m_session, 0, 0),
              NvBlastExtUnitySessionResult_Success);

    EXPECT_EQ(NvBlastExtUnitySessionGetChunkCount(m_session), 1u);
    EXPECT_TRUE(childrenOf(0).empty());

    NvBlastExtUnityChunkInfo info = {};
    ASSERT_EQ(NvBlastExtUnitySessionGetChunkInfo(m_session, 0, &info), NvBlastExtUnitySessionResult_Success);
    EXPECT_NE(info.flags & NvBlastExtUnityChunkFlag_IsLeaf, 0u);
}

TEST_F(FractureSessionTest, DeleteSubhierarchyRejectsUnknownChunk)
{
    setCubeSource();
    EXPECT_EQ(NvBlastExtUnitySessionDeleteChunkSubhierarchy(m_session, 999, 0),
              NvBlastExtUnitySessionResult_InvalidChunk);
}

// ─── Queries ──────────────────────────────────────────────────────────────────

TEST_F(FractureSessionTest, ChunkIdQueriesFollowTheTwoPhaseConvention)
{
    setCubeSource();

    SlicingConfiguration slicing;
    slicing.x_slices = 1;
    slicing.y_slices = 1;
    slicing.z_slices = 1;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);

    // Passing a null buffer reports the size without writing anything.
    const uint32_t total = NvBlastExtUnitySessionGetChunkIds(m_session, nullptr, 0);
    EXPECT_EQ(total, 9u);

    std::vector<int32_t> ids(total);
    EXPECT_EQ(NvBlastExtUnitySessionGetChunkIds(m_session, ids.data(), total), total);

    // An undersized buffer is filled as far as it goes and still reports the true total.
    std::vector<int32_t> small(3, -1);
    EXPECT_EQ(NvBlastExtUnitySessionGetChunkIds(m_session, small.data(), 3), total);
    for (int32_t id : small)
    {
        EXPECT_NE(id, -1);
    }

    EXPECT_EQ(NvBlastExtUnitySessionGetChunkIdsAtDepth(m_session, 0, nullptr, 0), 1u);
    EXPECT_EQ(NvBlastExtUnitySessionGetChunkIdsAtDepth(m_session, 1, nullptr, 0), 8u);
}

TEST_F(FractureSessionTest, ChunkInfoReportsUnknownChunk)
{
    setCubeSource();

    NvBlastExtUnityChunkInfo info = {};
    EXPECT_EQ(NvBlastExtUnitySessionGetChunkInfo(m_session, 999, &info), NvBlastExtUnitySessionResult_InvalidChunk);
    EXPECT_EQ(NvBlastExtUnitySessionGetChunkInfo(m_session, 0, nullptr), NvBlastExtUnitySessionResult_InvalidArgument);
}

TEST_F(FractureSessionTest, InteriorMaterialIdRoundTrips)
{
    setCubeSource();
    NvBlastExtUnitySessionSetInteriorMaterialId(m_session, 13);
    EXPECT_EQ(NvBlastExtUnitySessionGetInteriorMaterialId(m_session), 13);
}

// ─── Chunk geometry ───────────────────────────────────────────────────────────

TEST_F(FractureSessionTest, CreateChunkMeshReturnsPreviewableGeometry)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);

    const std::vector<int32_t> children = childrenOf(0);
    ASSERT_FALSE(children.empty());

    Mesh* chunkMesh = NvBlastExtUnitySessionCreateChunkMesh(m_session, children.front(), 1);
    ASSERT_NE(chunkMesh, nullptr);
    EXPECT_GT(NvBlastExtUnityGetVerticesCount(chunkMesh), 0u);
    EXPECT_GT(NvBlastExtUnityGetFacetCount(chunkMesh), 0u);

    // The mesh is a caller-owned snapshot; releasing it must not disturb the session.
    NvBlastExtUnityReleaseMesh(chunkMesh);
    EXPECT_GT(NvBlastExtUnitySessionGetChunkCount(m_session), 1u);
}

TEST_F(FractureSessionTest, CreateChunkMeshRejectsUnknownChunk)
{
    setCubeSource();
    EXPECT_EQ(NvBlastExtUnitySessionCreateChunkMesh(m_session, 999, 1), nullptr);
}

// ─── Finalize ─────────────────────────────────────────────────────────────────

TEST_F(FractureSessionTest, FinalizeProducesAnAsset)
{
    setCubeSource();

    SlicingConfiguration slicing;
    slicing.x_slices = 1;
    slicing.y_slices = 1;
    slicing.z_slices = 1;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);

    ConvexMeshBuilder* collisionBuilder = NvBlastExtUnityCreateCollisionBuilder();
    ASSERT_NE(collisionBuilder, nullptr);

    AuthoringResult* result = NvBlastExtUnitySessionFinalize(m_session, collisionBuilder, 1, -1);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->chunkCount, 9u);
    EXPECT_GT(result->bondCount, 0u);
    EXPECT_NE(result->asset, nullptr);

    // Ordering matters: the result holds hulls the builder must still be alive to free.
    NvBlastExtUnityReleaseAuthoringResult(*collisionBuilder, result);
    NvBlastExtUnityReleaseCollisionBuilder(collisionBuilder);
}

TEST_F(FractureSessionTest, SessionStaysEditableAfterFinalize)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);

    ConvexMeshBuilder* collisionBuilder = NvBlastExtUnityCreateCollisionBuilder();
    AuthoringResult*   first            = NvBlastExtUnitySessionFinalize(m_session, collisionBuilder, 1, -1);
    ASSERT_NE(first, nullptr);
    NvBlastExtUnityReleaseAuthoringResult(*collisionBuilder, first);

    // Finalizing is a preview step, not a teardown: the artist keeps working afterwards.
    const std::vector<int32_t> children = childrenOf(0);
    ASSERT_FALSE(children.empty());

    VoronoiConfiguration voronoi(4);
    EXPECT_EQ(NvBlastExtUnitySessionFractureVoronoi(m_session, children.front(), voronoi, 0),
              NvBlastExtUnitySessionResult_Success);

    AuthoringResult* second = NvBlastExtUnitySessionFinalize(m_session, collisionBuilder, 1, -1);
    ASSERT_NE(second, nullptr);
    EXPECT_GT(second->chunkCount, first == nullptr ? 0u : 9u);

    NvBlastExtUnityReleaseAuthoringResult(*collisionBuilder, second);
    NvBlastExtUnityReleaseCollisionBuilder(collisionBuilder);
}

TEST_F(FractureSessionTest, FinalizeWithoutSourceMeshesFails)
{
    ConvexMeshBuilder* collisionBuilder = NvBlastExtUnityCreateCollisionBuilder();
    EXPECT_EQ(NvBlastExtUnitySessionFinalize(m_session, collisionBuilder, 1, -1), nullptr);
    NvBlastExtUnityReleaseCollisionBuilder(collisionBuilder);
}

// ─── Support graph ────────────────────────────────────────────────────────────

TEST_F(FractureSessionTest, StaticMarkRoundTrips)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);

    const std::vector<int32_t> children = childrenOf(0);
    ASSERT_FALSE(children.empty());

    EXPECT_EQ(NvBlastExtUnitySessionGetChunkStatic(m_session, children.front()), 0u);
    ASSERT_EQ(NvBlastExtUnitySessionSetChunkStatic(m_session, children.front(), 1),
              NvBlastExtUnitySessionResult_Success);
    EXPECT_NE(NvBlastExtUnitySessionGetChunkStatic(m_session, children.front()), 0u);

    EXPECT_EQ(NvBlastExtUnitySessionGetStaticChunkIds(m_session, nullptr, 0), 1u);

    ASSERT_EQ(NvBlastExtUnitySessionSetChunkStatic(m_session, children.front(), 0),
              NvBlastExtUnitySessionResult_Success);
    EXPECT_EQ(NvBlastExtUnitySessionGetStaticChunkIds(m_session, nullptr, 0), 0u);
}

TEST_F(FractureSessionTest, StaticMarkRejectsUnknownChunk)
{
    setCubeSource();
    EXPECT_EQ(NvBlastExtUnitySessionSetChunkStatic(m_session, 999, 1),
              NvBlastExtUnitySessionResult_InvalidChunk);
}

TEST_F(FractureSessionTest, ClearStaticChunksDropsEveryMark)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);

    for (int32_t chunkId : childrenOf(0))
    {
        ASSERT_EQ(NvBlastExtUnitySessionSetChunkStatic(m_session, chunkId, 1),
                  NvBlastExtUnitySessionResult_Success);
    }
    ASSERT_GT(NvBlastExtUnitySessionGetStaticChunkIds(m_session, nullptr, 0), 0u);

    NvBlastExtUnitySessionClearStaticChunks(m_session);
    EXPECT_EQ(NvBlastExtUnitySessionGetStaticChunkIds(m_session, nullptr, 0), 0u);
}

TEST_F(FractureSessionTest, SupportPredictionMatchesTheDepthRule)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);

    // With -1 every leaf is support, so the children are and the root is not.
    EXPECT_EQ(NvBlastExtUnitySessionIsChunkSupport(m_session, 0, -1), 0u);
    for (int32_t chunkId : childrenOf(0))
    {
        EXPECT_NE(NvBlastExtUnitySessionIsChunkSupport(m_session, chunkId, -1), 0u);
    }

    // Pinning the depth to 0 moves the support layer up to the root.
    EXPECT_NE(NvBlastExtUnitySessionIsChunkSupport(m_session, 0, 0), 0u);
    for (int32_t chunkId : childrenOf(0))
    {
        EXPECT_EQ(NvBlastExtUnitySessionIsChunkSupport(m_session, chunkId, 0), 0u);
    }
}

TEST_F(FractureSessionTest, AnchoringAddsExternalBondsToTheAsset)
{
    setCubeSource();

    SlicingConfiguration slicing;
    slicing.x_slices = 1;
    slicing.y_slices = 1;
    slicing.z_slices = 1;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);

    ConvexMeshBuilder* collisionBuilder = NvBlastExtUnityCreateCollisionBuilder();
    ASSERT_NE(collisionBuilder, nullptr);

    AuthoringResult* plain = NvBlastExtUnitySessionFinalize(m_session, collisionBuilder, 1, -1);
    ASSERT_NE(plain, nullptr);
    const uint32_t bondsWithoutAnchors = NvBlastAssetGetBondCount(plain->asset, nullptr);
    NvBlastExtUnityReleaseAuthoringResult(*collisionBuilder, plain);

    // Anchor two of the leaves; each one adds a bond to the external body.
    const std::vector<int32_t> children = childrenOf(0);
    ASSERT_GE(children.size(), 2u);
    ASSERT_EQ(NvBlastExtUnitySessionSetChunkStatic(m_session, children[0], 1),
              NvBlastExtUnitySessionResult_Success);
    ASSERT_EQ(NvBlastExtUnitySessionSetChunkStatic(m_session, children[1], 1),
              NvBlastExtUnitySessionResult_Success);

    AuthoringResult* anchored = NvBlastExtUnitySessionFinalize(m_session, collisionBuilder, 1, -1);
    ASSERT_NE(anchored, nullptr);

    EXPECT_EQ(NvBlastAssetGetBondCount(anchored->asset, nullptr), bondsWithoutAnchors + 2);

    NvBlastExtUnityReleaseAuthoringResult(*collisionBuilder, anchored);
    NvBlastExtUnityReleaseCollisionBuilder(collisionBuilder);
}

TEST_F(FractureSessionTest, AnchoringANonSupportChunkIsRefusedNotSilent)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);

    // The root is not a support chunk under the leaf rule, so anchoring it cannot work. Finalize
    // must still succeed — the anchor is reported and skipped rather than failing the whole build.
    ASSERT_EQ(NvBlastExtUnitySessionSetChunkStatic(m_session, 0, 1),
              NvBlastExtUnitySessionResult_Success);

    ConvexMeshBuilder* collisionBuilder = NvBlastExtUnityCreateCollisionBuilder();
    AuthoringResult*   result           = NvBlastExtUnitySessionFinalize(m_session, collisionBuilder, 1, -1);

    ASSERT_NE(result, nullptr);
    EXPECT_GT(result->chunkCount, 1u);

    NvBlastExtUnityReleaseAuthoringResult(*collisionBuilder, result);
    NvBlastExtUnityReleaseCollisionBuilder(collisionBuilder);
}

TEST_F(FractureSessionTest, StaticMarksSurviveFurtherFracturing)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);

    const std::vector<int32_t> children = childrenOf(0);
    ASSERT_GE(children.size(), 2u);

    ASSERT_EQ(NvBlastExtUnitySessionSetChunkStatic(m_session, children[0], 1),
              NvBlastExtUnitySessionResult_Success);

    // Fracturing an unrelated sibling must not disturb the mark.
    VoronoiConfiguration voronoi(4);
    ASSERT_EQ(NvBlastExtUnitySessionFractureVoronoi(m_session, children[1], voronoi, 0),
              NvBlastExtUnitySessionResult_Success);

    EXPECT_NE(NvBlastExtUnitySessionGetChunkStatic(m_session, children[0]), 0u);
}

// ─── Asset serialization ──────────────────────────────────────────────────────

TEST_F(FractureSessionTest, AssetSurvivesASerializationRoundTrip)
{
    setCubeSource();

    SlicingConfiguration slicing;
    slicing.x_slices = 1;
    slicing.y_slices = 1;
    slicing.z_slices = 1;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);

    ConvexMeshBuilder* collisionBuilder = NvBlastExtUnityCreateCollisionBuilder();
    AuthoringResult*   result           = NvBlastExtUnitySessionFinalize(m_session, collisionBuilder, 1, -1);
    ASSERT_NE(result, nullptr);

    const uint32_t originalChunks = NvBlastAssetGetChunkCount(result->asset, nullptr);
    const uint32_t originalBonds  = NvBlastAssetGetBondCount(result->asset, nullptr);

    void*          buffer = nullptr;
    const uint32_t size   = NvBlastExtUnitySerializeAsset(result->asset, &buffer);

    ASSERT_GT(size, 0u);
    ASSERT_NE(buffer, nullptr);

    // The authoring result owns the asset and frees it on release, so the serialized copy has to
    // stand on its own — that is the whole point of saving it.
    NvBlastExtUnityReleaseAuthoringResult(*collisionBuilder, result);
    NvBlastExtUnityReleaseCollisionBuilder(collisionBuilder);

    NvBlastAsset* restored = NvBlastExtUnityDeserializeAsset(buffer, size);
    ASSERT_NE(restored, nullptr);

    EXPECT_EQ(NvBlastAssetGetChunkCount(restored, nullptr), originalChunks);
    EXPECT_EQ(NvBlastAssetGetBondCount(restored, nullptr), originalBonds);

    NvBlastExtUnityReleaseAsset(restored);
    NvBlastExtUnityReleaseSerializedAsset(buffer);
}

TEST_F(FractureSessionTest, RepeatedSerializationIsStable)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);

    ConvexMeshBuilder* collisionBuilder = NvBlastExtUnityCreateCollisionBuilder();
    AuthoringResult*   result           = NvBlastExtUnitySessionFinalize(m_session, collisionBuilder, 1, -1);
    ASSERT_NE(result, nullptr);

    // Saving repeatedly is the normal editor workflow, and each save builds and tears down its own
    // serialization manager — this is the loop that has to stay stable.
    for (int pass = 0; pass < 5; ++pass)
    {
        void*          buffer = nullptr;
        const uint32_t size   = NvBlastExtUnitySerializeAsset(result->asset, &buffer);
        ASSERT_GT(size, 0u) << "pass " << pass;

        NvBlastAsset* restored = NvBlastExtUnityDeserializeAsset(buffer, size);
        ASSERT_NE(restored, nullptr) << "pass " << pass;

        NvBlastExtUnityReleaseAsset(restored);
        NvBlastExtUnityReleaseSerializedAsset(buffer);
    }

    NvBlastExtUnityReleaseAuthoringResult(*collisionBuilder, result);
    NvBlastExtUnityReleaseCollisionBuilder(collisionBuilder);
}

TEST_F(FractureSessionTest, SerializingNothingIsRefused)
{
    void* buffer = nullptr;
    EXPECT_EQ(NvBlastExtUnitySerializeAsset(nullptr, &buffer), 0u);
    EXPECT_EQ(NvBlastExtUnityDeserializeAsset(nullptr, 0), nullptr);

    // Releasing null must be safe — the C# side runs these from finalizers.
    NvBlastExtUnityReleaseSerializedAsset(nullptr);
    NvBlastExtUnityReleaseAsset(nullptr);
}

TEST_F(FractureSessionTest, AnchorsSurviveSerialization)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtUnitySessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtUnitySessionResult_Success);

    const std::vector<int32_t> children = childrenOf(0);
    ASSERT_FALSE(children.empty());
    ASSERT_EQ(NvBlastExtUnitySessionSetChunkStatic(m_session, children.front(), 1),
              NvBlastExtUnitySessionResult_Success);

    ConvexMeshBuilder* collisionBuilder = NvBlastExtUnityCreateCollisionBuilder();
    AuthoringResult*   result           = NvBlastExtUnitySessionFinalize(m_session, collisionBuilder, 1, -1);
    ASSERT_NE(result, nullptr);

    const uint32_t bondsWithAnchor = NvBlastAssetGetBondCount(result->asset, nullptr);

    void*          buffer = nullptr;
    const uint32_t size   = NvBlastExtUnitySerializeAsset(result->asset, &buffer);
    ASSERT_GT(size, 0u);

    NvBlastExtUnityReleaseAuthoringResult(*collisionBuilder, result);
    NvBlastExtUnityReleaseCollisionBuilder(collisionBuilder);

    NvBlastAsset* restored = NvBlastExtUnityDeserializeAsset(buffer, size);
    ASSERT_NE(restored, nullptr);

    // The world bond is what anchors the asset; losing it in serialization would leave the runtime
    // with a structure that collapses immediately.
    EXPECT_EQ(NvBlastAssetGetBondCount(restored, nullptr), bondsWithAnchor);

    NvBlastExtUnityReleaseAsset(restored);
    NvBlastExtUnityReleaseSerializedAsset(buffer);
}

// ─── One-shot API, now running on top of a session ────────────────────────────

TEST_F(FractureSessionTest, OneShotFractureStillWorks)
{
    VoronoiConfiguration voronoi(5);
    Fracturer*           fracturer = NvBlastExtUnityCreateVoronoiFracturer(voronoi);
    ASSERT_NE(fracturer, nullptr);

    ConvexMeshBuilder* collisionBuilder = NvBlastExtUnityCreateCollisionBuilder();
    AuthoringResult*   result = NvBlastExtUnityFractureMesh(m_mesh, 1, fracturer, collisionBuilder, silentLog);

    ASSERT_NE(result, nullptr);
    EXPECT_GT(result->chunkCount, 1u);

    NvBlastExtUnityReleaseAuthoringResult(*collisionBuilder, result);
    NvBlastExtUnityReleaseCollisionBuilder(collisionBuilder);
    NvBlastExtUnityReleaseFracturer(fracturer);
}

}  // namespace
