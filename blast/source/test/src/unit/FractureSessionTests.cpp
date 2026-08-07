// Tests for the persistent authoring session (NvBlastExtBridgeSession.h).
//
// These exercise the C API rather than the FractureSession class, because the C API is the contract
// engine integrations are written against — a regression there breaks Unity and Unreal silently,
// where a C++-level one would at least fail to compile.

#include "NvBlastExtBridge.h"
#include "NvBlastExtBridgeSession.h"
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

    return NvBlastExtBridgeCreateMesh(positions, normals, uvs, verticesCount, indices, indicesCount);
}

class FractureSessionTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_session = NvBlastExtBridgeSessionCreate(silentLog);
        ASSERT_NE(m_session, nullptr);

        m_mesh = createCubeMesh();
        ASSERT_NE(m_mesh, nullptr);
    }

    void TearDown() override
    {
        if (m_session != nullptr)
        {
            NvBlastExtBridgeSessionRelease(m_session);
        }
        if (m_mesh != nullptr)
        {
            NvBlastExtBridgeReleaseMesh(m_mesh);
        }
    }

    /** Loads the cube as chunk 0. The session copies it, so m_mesh stays owned by the fixture. */
    void setCubeSource()
    {
        int32_t ids[] = { 0 };
        ASSERT_EQ(NvBlastExtBridgeSessionSetSourceMeshes(m_session, &m_mesh, 1, ids),
                  NvBlastExtBridgeSessionResult_Success);
    }

    std::vector<int32_t> childrenOf(int32_t chunkId)
    {
        const uint32_t count = NvBlastExtBridgeSessionGetChildChunkIds(m_session, chunkId, nullptr, 0);
        std::vector<int32_t> ids(count);
        if (count > 0)
        {
            NvBlastExtBridgeSessionGetChildChunkIds(m_session, chunkId, ids.data(), count);
        }
        return ids;
    }

    NvBlastExtBridgeFractureSession* m_session = nullptr;
    Mesh*                           m_mesh    = nullptr;
};

// ─── Lifecycle and source meshes ──────────────────────────────────────────────

TEST_F(FractureSessionTest, NewSessionHasNoChunks)
{
    EXPECT_EQ(NvBlastExtBridgeSessionGetChunkCount(m_session), 0u);
}

TEST_F(FractureSessionTest, SetSourceMeshesCreatesRootChunk)
{
    setCubeSource();

    EXPECT_EQ(NvBlastExtBridgeSessionGetChunkCount(m_session), 1u);
    EXPECT_EQ(NvBlastExtBridgeSessionGetChunkDepth(m_session, 0), 0);

    NvBlastExtBridgeChunkInfo info = {};
    ASSERT_EQ(NvBlastExtBridgeSessionGetChunkInfo(m_session, 0, &info), NvBlastExtBridgeSessionResult_Success);
    EXPECT_EQ(info.chunkId, 0);
    EXPECT_EQ(info.parentChunkId, -1);
    EXPECT_NE(info.flags & NvBlastExtBridgeChunkFlag_IsRoot, 0u);
    EXPECT_NE(info.flags & NvBlastExtBridgeChunkFlag_IsLeaf, 0u);
}

TEST_F(FractureSessionTest, SetSourceMeshesRejectsEmptyInput)
{
    EXPECT_EQ(NvBlastExtBridgeSessionSetSourceMeshes(m_session, nullptr, 0, nullptr),
              NvBlastExtBridgeSessionResult_InvalidArgument);

    Mesh* nullMesh[] = { nullptr };
    EXPECT_EQ(NvBlastExtBridgeSessionSetSourceMeshes(m_session, nullMesh, 1, nullptr),
              NvBlastExtBridgeSessionResult_InvalidArgument);
}

TEST_F(FractureSessionTest, ResetClearsHierarchyButKeepsSettings)
{
    setCubeSource();
    NvBlastExtBridgeSessionSetSeed(m_session, 42);
    NvBlastExtBridgeSessionSetInteriorMaterialId(m_session, 7);

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);
    ASSERT_GT(NvBlastExtBridgeSessionGetChunkCount(m_session), 1u);

    NvBlastExtBridgeSessionReset(m_session);

    EXPECT_EQ(NvBlastExtBridgeSessionGetChunkCount(m_session), 0u);
    EXPECT_EQ(NvBlastExtBridgeSessionGetSeed(m_session), 42);
}

TEST_F(FractureSessionTest, NullSessionIsHandled)
{
    // The C# side can hold a stale handle; every entry point must degrade rather than crash.
    EXPECT_EQ(NvBlastExtBridgeSessionGetChunkCount(nullptr), 0u);
    EXPECT_EQ(NvBlastExtBridgeSessionGetChunkDepth(nullptr, 0), -1);
    EXPECT_EQ(NvBlastExtBridgeSessionCreateChunkMesh(nullptr, 0, 1), nullptr);
    EXPECT_EQ(NvBlastExtBridgeSessionFinalize(nullptr, nullptr, 1, -1), nullptr);

    VoronoiConfiguration voronoi(5);
    EXPECT_EQ(NvBlastExtBridgeSessionFractureVoronoi(nullptr, 0, voronoi, 0),
              NvBlastExtBridgeSessionResult_InvalidSession);

    NvBlastExtBridgeSessionRelease(nullptr);  // must not crash
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

    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);

    EXPECT_EQ(childrenOf(0).size(), 8u);
    EXPECT_EQ(NvBlastExtBridgeSessionGetChunkCount(m_session), 9u);  // root + 8
}

TEST_F(FractureSessionTest, VoronoiProducesChildrenAtNextDepth)
{
    setCubeSource();

    VoronoiConfiguration voronoi(5);
    ASSERT_EQ(NvBlastExtBridgeSessionFractureVoronoi(m_session, 0, voronoi, 0),
              NvBlastExtBridgeSessionResult_Success);

    const std::vector<int32_t> children = childrenOf(0);
    ASSERT_GT(children.size(), 1u);

    for (int32_t child : children)
    {
        EXPECT_EQ(NvBlastExtBridgeSessionGetChunkDepth(m_session, child), 1);
    }

    // The root is no longer a leaf now that it has children.
    NvBlastExtBridgeChunkInfo rootInfo = {};
    ASSERT_EQ(NvBlastExtBridgeSessionGetChunkInfo(m_session, 0, &rootInfo), NvBlastExtBridgeSessionResult_Success);
    EXPECT_EQ(rootInfo.flags & NvBlastExtBridgeChunkFlag_IsLeaf, 0u);
}

// This is the operation the whole session exists for: drilling into an already-fractured chunk.
TEST_F(FractureSessionTest, ChunkCanBeSubdividedFurther)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);

    const std::vector<int32_t> firstLevel = childrenOf(0);
    ASSERT_FALSE(firstLevel.empty());

    const int32_t  target        = firstLevel.front();
    const uint32_t countBefore   = NvBlastExtBridgeSessionGetChunkCount(m_session);

    VoronoiConfiguration voronoi(4);
    ASSERT_EQ(NvBlastExtBridgeSessionFractureVoronoi(m_session, target, voronoi, 0),
              NvBlastExtBridgeSessionResult_Success);

    const std::vector<int32_t> secondLevel = childrenOf(target);
    ASSERT_GT(secondLevel.size(), 1u);
    EXPECT_GT(NvBlastExtBridgeSessionGetChunkCount(m_session), countBefore);

    for (int32_t grandchild : secondLevel)
    {
        EXPECT_EQ(NvBlastExtBridgeSessionGetChunkDepth(m_session, grandchild), 2);
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
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);
    ASSERT_EQ(childrenOf(0).size(), 8u);

    // Re-running with different settings must replace the previous result, not add to it —
    // otherwise tweaking a slider in a tool would pile up chunks.
    slicing.x_slices = 2;
    slicing.y_slices = 1;
    slicing.z_slices = 1;
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);

    EXPECT_EQ(childrenOf(0).size(), 12u);  // 3 x 2 x 2
    EXPECT_EQ(NvBlastExtBridgeSessionGetChunkCount(m_session), 13u);
}

TEST_F(FractureSessionTest, FractureRejectsUnknownChunk)
{
    setCubeSource();

    VoronoiConfiguration voronoi(5);
    EXPECT_EQ(NvBlastExtBridgeSessionFractureVoronoi(m_session, 999, voronoi, 0),
              NvBlastExtBridgeSessionResult_InvalidChunk);
}

TEST_F(FractureSessionTest, FractureRequiresSourceMeshes)
{
    VoronoiConfiguration voronoi(5);
    EXPECT_EQ(NvBlastExtBridgeSessionFractureVoronoi(m_session, 0, voronoi, 0),
              NvBlastExtBridgeSessionResult_NoSourceMesh);
}

TEST_F(FractureSessionTest, RootChunkCannotBeReplaced)
{
    setCubeSource();

    // Replacing a source mesh would leave the asset without its top-level chunk.
    VoronoiConfiguration voronoi(5);
    EXPECT_EQ(NvBlastExtBridgeSessionFractureVoronoi(m_session, 0, voronoi, 1),
              NvBlastExtBridgeSessionResult_InvalidArgument);
}

TEST_F(FractureSessionTest, ReplaceChunkKeepsChildrenAtTheSameDepth)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);

    const std::vector<int32_t> firstLevel = childrenOf(0);
    ASSERT_FALSE(firstLevel.empty());
    const int32_t target = firstLevel.front();

    VoronoiConfiguration voronoi(4);
    ASSERT_EQ(NvBlastExtBridgeSessionFractureVoronoi(m_session, target, voronoi, 1),
              NvBlastExtBridgeSessionResult_Success);

    // The replaced chunk is gone and its pieces sit where it used to, still under the root.
    EXPECT_EQ(NvBlastExtBridgeSessionGetChunkDepth(m_session, target), -1);
    const std::vector<int32_t> afterReplace = childrenOf(0);
    EXPECT_GT(afterReplace.size(), firstLevel.size());
    for (int32_t chunk : afterReplace)
    {
        EXPECT_EQ(NvBlastExtBridgeSessionGetChunkDepth(m_session, chunk), 1);
    }
}

// ─── Determinism ──────────────────────────────────────────────────────────────

TEST_F(FractureSessionTest, SameSeedReproducesTheSameFracture)
{
    setCubeSource();
    NvBlastExtBridgeSessionSetSeed(m_session, 1234);

    VoronoiConfiguration voronoi(8);
    ASSERT_EQ(NvBlastExtBridgeSessionFractureVoronoi(m_session, 0, voronoi, 0),
              NvBlastExtBridgeSessionResult_Success);
    const uint32_t firstRun = NvBlastExtBridgeSessionGetChunkCount(m_session);

    // A second session, seeded identically, must land on the same hierarchy — this is what lets a
    // tool store a seed in a project file and rebuild the same asset later.
    NvBlastExtBridgeFractureSession* other = NvBlastExtBridgeSessionCreate(silentLog);
    ASSERT_NE(other, nullptr);
    int32_t ids[] = { 0 };
    ASSERT_EQ(NvBlastExtBridgeSessionSetSourceMeshes(other, &m_mesh, 1, ids), NvBlastExtBridgeSessionResult_Success);
    NvBlastExtBridgeSessionSetSeed(other, 1234);
    ASSERT_EQ(NvBlastExtBridgeSessionFractureVoronoi(other, 0, voronoi, 0), NvBlastExtBridgeSessionResult_Success);

    EXPECT_EQ(NvBlastExtBridgeSessionGetChunkCount(other), firstRun);

    NvBlastExtBridgeSessionRelease(other);
}

TEST_F(FractureSessionTest, ReseedingIsIndependentOfOperationHistory)
{
    setCubeSource();
    NvBlastExtBridgeSessionSetSeed(m_session, 99);

    VoronoiConfiguration voronoi(6);
    ASSERT_EQ(NvBlastExtBridgeSessionFractureVoronoi(m_session, 0, voronoi, 0),
              NvBlastExtBridgeSessionResult_Success);
    const size_t firstAttempt = childrenOf(0).size();

    // Running unrelated work in between must not shift the generator: the operation is re-seeded,
    // so repeating it with the same seed gives the same answer.
    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);

    ASSERT_EQ(NvBlastExtBridgeSessionFractureVoronoi(m_session, 0, voronoi, 0),
              NvBlastExtBridgeSessionResult_Success);
    EXPECT_EQ(childrenOf(0).size(), firstAttempt);
}

// ─── Hierarchy editing ────────────────────────────────────────────────────────

TEST_F(FractureSessionTest, DeleteSubhierarchyUndoesAFracture)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);
    ASSERT_GT(NvBlastExtBridgeSessionGetChunkCount(m_session), 1u);

    ASSERT_EQ(NvBlastExtBridgeSessionDeleteChunkSubhierarchy(m_session, 0, 0),
              NvBlastExtBridgeSessionResult_Success);

    EXPECT_EQ(NvBlastExtBridgeSessionGetChunkCount(m_session), 1u);
    EXPECT_TRUE(childrenOf(0).empty());

    NvBlastExtBridgeChunkInfo info = {};
    ASSERT_EQ(NvBlastExtBridgeSessionGetChunkInfo(m_session, 0, &info), NvBlastExtBridgeSessionResult_Success);
    EXPECT_NE(info.flags & NvBlastExtBridgeChunkFlag_IsLeaf, 0u);
}

TEST_F(FractureSessionTest, DeleteSubhierarchyRejectsUnknownChunk)
{
    setCubeSource();
    EXPECT_EQ(NvBlastExtBridgeSessionDeleteChunkSubhierarchy(m_session, 999, 0),
              NvBlastExtBridgeSessionResult_InvalidChunk);
}

// ─── Queries ──────────────────────────────────────────────────────────────────

TEST_F(FractureSessionTest, ChunkIdQueriesFollowTheTwoPhaseConvention)
{
    setCubeSource();

    SlicingConfiguration slicing;
    slicing.x_slices = 1;
    slicing.y_slices = 1;
    slicing.z_slices = 1;
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);

    // Passing a null buffer reports the size without writing anything.
    const uint32_t total = NvBlastExtBridgeSessionGetChunkIds(m_session, nullptr, 0);
    EXPECT_EQ(total, 9u);

    std::vector<int32_t> ids(total);
    EXPECT_EQ(NvBlastExtBridgeSessionGetChunkIds(m_session, ids.data(), total), total);

    // An undersized buffer is filled as far as it goes and still reports the true total.
    std::vector<int32_t> small(3, -1);
    EXPECT_EQ(NvBlastExtBridgeSessionGetChunkIds(m_session, small.data(), 3), total);
    for (int32_t id : small)
    {
        EXPECT_NE(id, -1);
    }

    EXPECT_EQ(NvBlastExtBridgeSessionGetChunkIdsAtDepth(m_session, 0, nullptr, 0), 1u);
    EXPECT_EQ(NvBlastExtBridgeSessionGetChunkIdsAtDepth(m_session, 1, nullptr, 0), 8u);
}

TEST_F(FractureSessionTest, ChunkInfoReportsUnknownChunk)
{
    setCubeSource();

    NvBlastExtBridgeChunkInfo info = {};
    EXPECT_EQ(NvBlastExtBridgeSessionGetChunkInfo(m_session, 999, &info), NvBlastExtBridgeSessionResult_InvalidChunk);
    EXPECT_EQ(NvBlastExtBridgeSessionGetChunkInfo(m_session, 0, nullptr), NvBlastExtBridgeSessionResult_InvalidArgument);
}

TEST_F(FractureSessionTest, InteriorMaterialIdRoundTrips)
{
    setCubeSource();
    NvBlastExtBridgeSessionSetInteriorMaterialId(m_session, 13);
    EXPECT_EQ(NvBlastExtBridgeSessionGetInteriorMaterialId(m_session), 13);
}

// ─── Chunk geometry ───────────────────────────────────────────────────────────

TEST_F(FractureSessionTest, CreateChunkMeshReturnsPreviewableGeometry)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);

    const std::vector<int32_t> children = childrenOf(0);
    ASSERT_FALSE(children.empty());

    Mesh* chunkMesh = NvBlastExtBridgeSessionCreateChunkMesh(m_session, children.front(), 1);
    ASSERT_NE(chunkMesh, nullptr);
    EXPECT_GT(NvBlastExtBridgeGetVerticesCount(chunkMesh), 0u);
    EXPECT_GT(NvBlastExtBridgeGetFacetCount(chunkMesh), 0u);

    // The mesh is a caller-owned snapshot; releasing it must not disturb the session.
    NvBlastExtBridgeReleaseMesh(chunkMesh);
    EXPECT_GT(NvBlastExtBridgeSessionGetChunkCount(m_session), 1u);
}

TEST_F(FractureSessionTest, CreateChunkMeshRejectsUnknownChunk)
{
    setCubeSource();
    EXPECT_EQ(NvBlastExtBridgeSessionCreateChunkMesh(m_session, 999, 1), nullptr);
}

// ─── Finalize ─────────────────────────────────────────────────────────────────

TEST_F(FractureSessionTest, FinalizeProducesAnAsset)
{
    setCubeSource();

    SlicingConfiguration slicing;
    slicing.x_slices = 1;
    slicing.y_slices = 1;
    slicing.z_slices = 1;
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);

    ConvexMeshBuilder* collisionBuilder = NvBlastExtBridgeCreateCollisionBuilder();
    ASSERT_NE(collisionBuilder, nullptr);

    AuthoringResult* result = NvBlastExtBridgeSessionFinalize(m_session, collisionBuilder, 1, -1);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->chunkCount, 9u);
    EXPECT_GT(result->bondCount, 0u);
    EXPECT_NE(result->asset, nullptr);

    // Ordering matters: the result holds hulls the builder must still be alive to free.
    NvBlastExtBridgeReleaseAuthoringResult(*collisionBuilder, result);
    NvBlastExtBridgeReleaseCollisionBuilder(collisionBuilder);
}

TEST_F(FractureSessionTest, SessionStaysEditableAfterFinalize)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);

    ConvexMeshBuilder* collisionBuilder = NvBlastExtBridgeCreateCollisionBuilder();
    AuthoringResult*   first            = NvBlastExtBridgeSessionFinalize(m_session, collisionBuilder, 1, -1);
    ASSERT_NE(first, nullptr);
    NvBlastExtBridgeReleaseAuthoringResult(*collisionBuilder, first);

    // Finalizing is a preview step, not a teardown: the artist keeps working afterwards.
    const std::vector<int32_t> children = childrenOf(0);
    ASSERT_FALSE(children.empty());

    VoronoiConfiguration voronoi(4);
    EXPECT_EQ(NvBlastExtBridgeSessionFractureVoronoi(m_session, children.front(), voronoi, 0),
              NvBlastExtBridgeSessionResult_Success);

    AuthoringResult* second = NvBlastExtBridgeSessionFinalize(m_session, collisionBuilder, 1, -1);
    ASSERT_NE(second, nullptr);
    EXPECT_GT(second->chunkCount, first == nullptr ? 0u : 9u);

    NvBlastExtBridgeReleaseAuthoringResult(*collisionBuilder, second);
    NvBlastExtBridgeReleaseCollisionBuilder(collisionBuilder);
}

TEST_F(FractureSessionTest, FinalizeWithoutSourceMeshesFails)
{
    ConvexMeshBuilder* collisionBuilder = NvBlastExtBridgeCreateCollisionBuilder();
    EXPECT_EQ(NvBlastExtBridgeSessionFinalize(m_session, collisionBuilder, 1, -1), nullptr);
    NvBlastExtBridgeReleaseCollisionBuilder(collisionBuilder);
}

// ─── Support graph ────────────────────────────────────────────────────────────

TEST_F(FractureSessionTest, StaticMarkRoundTrips)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);

    const std::vector<int32_t> children = childrenOf(0);
    ASSERT_FALSE(children.empty());

    EXPECT_EQ(NvBlastExtBridgeSessionGetChunkStatic(m_session, children.front()), 0u);
    ASSERT_EQ(NvBlastExtBridgeSessionSetChunkStatic(m_session, children.front(), 1),
              NvBlastExtBridgeSessionResult_Success);
    EXPECT_NE(NvBlastExtBridgeSessionGetChunkStatic(m_session, children.front()), 0u);

    EXPECT_EQ(NvBlastExtBridgeSessionGetStaticChunkIds(m_session, nullptr, 0), 1u);

    ASSERT_EQ(NvBlastExtBridgeSessionSetChunkStatic(m_session, children.front(), 0),
              NvBlastExtBridgeSessionResult_Success);
    EXPECT_EQ(NvBlastExtBridgeSessionGetStaticChunkIds(m_session, nullptr, 0), 0u);
}

TEST_F(FractureSessionTest, StaticMarkRejectsUnknownChunk)
{
    setCubeSource();
    EXPECT_EQ(NvBlastExtBridgeSessionSetChunkStatic(m_session, 999, 1),
              NvBlastExtBridgeSessionResult_InvalidChunk);
}

TEST_F(FractureSessionTest, ClearStaticChunksDropsEveryMark)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);

    for (int32_t chunkId : childrenOf(0))
    {
        ASSERT_EQ(NvBlastExtBridgeSessionSetChunkStatic(m_session, chunkId, 1),
                  NvBlastExtBridgeSessionResult_Success);
    }
    ASSERT_GT(NvBlastExtBridgeSessionGetStaticChunkIds(m_session, nullptr, 0), 0u);

    NvBlastExtBridgeSessionClearStaticChunks(m_session);
    EXPECT_EQ(NvBlastExtBridgeSessionGetStaticChunkIds(m_session, nullptr, 0), 0u);
}

TEST_F(FractureSessionTest, SupportPredictionMatchesTheDepthRule)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);

    // With -1 every leaf is support, so the children are and the root is not.
    EXPECT_EQ(NvBlastExtBridgeSessionIsChunkSupport(m_session, 0, -1), 0u);
    for (int32_t chunkId : childrenOf(0))
    {
        EXPECT_NE(NvBlastExtBridgeSessionIsChunkSupport(m_session, chunkId, -1), 0u);
    }

    // Pinning the depth to 0 moves the support layer up to the root.
    EXPECT_NE(NvBlastExtBridgeSessionIsChunkSupport(m_session, 0, 0), 0u);
    for (int32_t chunkId : childrenOf(0))
    {
        EXPECT_EQ(NvBlastExtBridgeSessionIsChunkSupport(m_session, chunkId, 0), 0u);
    }
}

TEST_F(FractureSessionTest, AnchoringAddsExternalBondsToTheAsset)
{
    setCubeSource();

    SlicingConfiguration slicing;
    slicing.x_slices = 1;
    slicing.y_slices = 1;
    slicing.z_slices = 1;
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);

    ConvexMeshBuilder* collisionBuilder = NvBlastExtBridgeCreateCollisionBuilder();
    ASSERT_NE(collisionBuilder, nullptr);

    AuthoringResult* plain = NvBlastExtBridgeSessionFinalize(m_session, collisionBuilder, 1, -1);
    ASSERT_NE(plain, nullptr);
    const uint32_t bondsWithoutAnchors = NvBlastAssetGetBondCount(plain->asset, nullptr);
    NvBlastExtBridgeReleaseAuthoringResult(*collisionBuilder, plain);

    // Anchor two of the leaves; each one adds a bond to the external body.
    const std::vector<int32_t> children = childrenOf(0);
    ASSERT_GE(children.size(), 2u);
    ASSERT_EQ(NvBlastExtBridgeSessionSetChunkStatic(m_session, children[0], 1),
              NvBlastExtBridgeSessionResult_Success);
    ASSERT_EQ(NvBlastExtBridgeSessionSetChunkStatic(m_session, children[1], 1),
              NvBlastExtBridgeSessionResult_Success);

    AuthoringResult* anchored = NvBlastExtBridgeSessionFinalize(m_session, collisionBuilder, 1, -1);
    ASSERT_NE(anchored, nullptr);

    EXPECT_EQ(NvBlastAssetGetBondCount(anchored->asset, nullptr), bondsWithoutAnchors + 2);

    NvBlastExtBridgeReleaseAuthoringResult(*collisionBuilder, anchored);
    NvBlastExtBridgeReleaseCollisionBuilder(collisionBuilder);
}

TEST_F(FractureSessionTest, AnchoringANonSupportChunkIsRefusedNotSilent)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);

    // The root is not a support chunk under the leaf rule, so anchoring it cannot work. Finalize
    // must still succeed — the anchor is reported and skipped rather than failing the whole build.
    ASSERT_EQ(NvBlastExtBridgeSessionSetChunkStatic(m_session, 0, 1),
              NvBlastExtBridgeSessionResult_Success);

    ConvexMeshBuilder* collisionBuilder = NvBlastExtBridgeCreateCollisionBuilder();
    AuthoringResult*   result           = NvBlastExtBridgeSessionFinalize(m_session, collisionBuilder, 1, -1);

    ASSERT_NE(result, nullptr);
    EXPECT_GT(result->chunkCount, 1u);

    NvBlastExtBridgeReleaseAuthoringResult(*collisionBuilder, result);
    NvBlastExtBridgeReleaseCollisionBuilder(collisionBuilder);
}

TEST_F(FractureSessionTest, StaticMarksSurviveFurtherFracturing)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);

    const std::vector<int32_t> children = childrenOf(0);
    ASSERT_GE(children.size(), 2u);

    ASSERT_EQ(NvBlastExtBridgeSessionSetChunkStatic(m_session, children[0], 1),
              NvBlastExtBridgeSessionResult_Success);

    // Fracturing an unrelated sibling must not disturb the mark.
    VoronoiConfiguration voronoi(4);
    ASSERT_EQ(NvBlastExtBridgeSessionFractureVoronoi(m_session, children[1], voronoi, 0),
              NvBlastExtBridgeSessionResult_Success);

    EXPECT_NE(NvBlastExtBridgeSessionGetChunkStatic(m_session, children[0]), 0u);
}

// ─── Asset serialization ──────────────────────────────────────────────────────

TEST_F(FractureSessionTest, AssetSurvivesASerializationRoundTrip)
{
    setCubeSource();

    SlicingConfiguration slicing;
    slicing.x_slices = 1;
    slicing.y_slices = 1;
    slicing.z_slices = 1;
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);

    ConvexMeshBuilder* collisionBuilder = NvBlastExtBridgeCreateCollisionBuilder();
    AuthoringResult*   result           = NvBlastExtBridgeSessionFinalize(m_session, collisionBuilder, 1, -1);
    ASSERT_NE(result, nullptr);

    const uint32_t originalChunks = NvBlastAssetGetChunkCount(result->asset, nullptr);
    const uint32_t originalBonds  = NvBlastAssetGetBondCount(result->asset, nullptr);

    void*          buffer = nullptr;
    const uint32_t size   = NvBlastExtBridgeSerializeAsset(result->asset, &buffer);

    ASSERT_GT(size, 0u);
    ASSERT_NE(buffer, nullptr);

    // The authoring result owns the asset and frees it on release, so the serialized copy has to
    // stand on its own — that is the whole point of saving it.
    NvBlastExtBridgeReleaseAuthoringResult(*collisionBuilder, result);
    NvBlastExtBridgeReleaseCollisionBuilder(collisionBuilder);

    NvBlastAsset* restored = NvBlastExtBridgeDeserializeAsset(buffer, size);
    ASSERT_NE(restored, nullptr);

    EXPECT_EQ(NvBlastAssetGetChunkCount(restored, nullptr), originalChunks);
    EXPECT_EQ(NvBlastAssetGetBondCount(restored, nullptr), originalBonds);

    NvBlastExtBridgeReleaseAsset(restored);
    NvBlastExtBridgeReleaseSerializedAsset(buffer);
}

TEST_F(FractureSessionTest, RepeatedSerializationIsStable)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);

    ConvexMeshBuilder* collisionBuilder = NvBlastExtBridgeCreateCollisionBuilder();
    AuthoringResult*   result           = NvBlastExtBridgeSessionFinalize(m_session, collisionBuilder, 1, -1);
    ASSERT_NE(result, nullptr);

    // Saving repeatedly is the normal editor workflow, and each save builds and tears down its own
    // serialization manager — this is the loop that has to stay stable.
    for (int pass = 0; pass < 5; ++pass)
    {
        void*          buffer = nullptr;
        const uint32_t size   = NvBlastExtBridgeSerializeAsset(result->asset, &buffer);
        ASSERT_GT(size, 0u) << "pass " << pass;

        NvBlastAsset* restored = NvBlastExtBridgeDeserializeAsset(buffer, size);
        ASSERT_NE(restored, nullptr) << "pass " << pass;

        NvBlastExtBridgeReleaseAsset(restored);
        NvBlastExtBridgeReleaseSerializedAsset(buffer);
    }

    NvBlastExtBridgeReleaseAuthoringResult(*collisionBuilder, result);
    NvBlastExtBridgeReleaseCollisionBuilder(collisionBuilder);
}

TEST_F(FractureSessionTest, SerializingNothingIsRefused)
{
    void* buffer = nullptr;
    EXPECT_EQ(NvBlastExtBridgeSerializeAsset(nullptr, &buffer), 0u);
    EXPECT_EQ(NvBlastExtBridgeDeserializeAsset(nullptr, 0), nullptr);

    // Releasing null must be safe — the C# side runs these from finalizers.
    NvBlastExtBridgeReleaseSerializedAsset(nullptr);
    NvBlastExtBridgeReleaseAsset(nullptr);
}

TEST_F(FractureSessionTest, AnchorsSurviveSerialization)
{
    setCubeSource();

    SlicingConfiguration slicing;
    ASSERT_EQ(NvBlastExtBridgeSessionFractureSlicing(m_session, 0, slicing, 0),
              NvBlastExtBridgeSessionResult_Success);

    const std::vector<int32_t> children = childrenOf(0);
    ASSERT_FALSE(children.empty());
    ASSERT_EQ(NvBlastExtBridgeSessionSetChunkStatic(m_session, children.front(), 1),
              NvBlastExtBridgeSessionResult_Success);

    ConvexMeshBuilder* collisionBuilder = NvBlastExtBridgeCreateCollisionBuilder();
    AuthoringResult*   result           = NvBlastExtBridgeSessionFinalize(m_session, collisionBuilder, 1, -1);
    ASSERT_NE(result, nullptr);

    const uint32_t bondsWithAnchor = NvBlastAssetGetBondCount(result->asset, nullptr);

    void*          buffer = nullptr;
    const uint32_t size   = NvBlastExtBridgeSerializeAsset(result->asset, &buffer);
    ASSERT_GT(size, 0u);

    NvBlastExtBridgeReleaseAuthoringResult(*collisionBuilder, result);
    NvBlastExtBridgeReleaseCollisionBuilder(collisionBuilder);

    NvBlastAsset* restored = NvBlastExtBridgeDeserializeAsset(buffer, size);
    ASSERT_NE(restored, nullptr);

    // The world bond is what anchors the asset; losing it in serialization would leave the runtime
    // with a structure that collapses immediately.
    EXPECT_EQ(NvBlastAssetGetBondCount(restored, nullptr), bondsWithAnchor);

    NvBlastExtBridgeReleaseAsset(restored);
    NvBlastExtBridgeReleaseSerializedAsset(buffer);
}

}  // namespace
