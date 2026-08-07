// Tests for the PhysX-free convex hull builder behind NvBlastExtUnityCreateCollisionBuilder.
//
// Exercised through the public C API rather than the class, since that is what the fracture
// pipeline and the engine integrations actually call.

#include "NvBlastExtUnity.h"
#include "NvBlastExtAuthoringConvexMeshBuilder.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

namespace
{

/**
    Slack allowed when testing whether a point sits inside a face plane.

    The hull's vertices are single-precision, so a face with four or more of them is never exactly
    planar and no single plane can contain all of them. The builder fits each plane through the face
    centroid, which spreads that residual around rather than pinning it to one vertex, but it cannot
    remove it. At the unit-ish coordinates these tests use, the leftover is a few 1e-4 — far below
    anything a collision solver would notice.
*/
const float kPlaneTolerance = 1e-3f;

/** Signed distance of a point from a polygon's plane. Negative is inside the hull. */
float planeDistance(const HullPolygon& polygon, const NvcVec3& point)
{
    return polygon.plane[0] * point.x + polygon.plane[1] * point.y + polygon.plane[2] * point.z + polygon.plane[3];
}

NvcVec3 hullCentroid(const CollisionHull& hull)
{
    NvcVec3 center = { 0.0f, 0.0f, 0.0f };
    for (uint32_t i = 0; i < hull.pointsCount; ++i)
    {
        center.x += hull.points[i].x;
        center.y += hull.points[i].y;
        center.z += hull.points[i].z;
    }

    const float inv = 1.0f / static_cast<float>(hull.pointsCount);
    return { center.x * inv, center.y * inv, center.z * inv };
}

class ConvexHullTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_builder = NvBlastExtUnityCreateCollisionBuilder();
        ASSERT_NE(m_builder, nullptr);
    }

    void TearDown() override
    {
        if (m_hull != nullptr)
        {
            m_builder->releaseCollisionHull(m_hull);
            m_hull = nullptr;
        }
        NvBlastExtUnityReleaseCollisionBuilder(m_builder);
    }

    CollisionHull* build(const std::vector<NvcVec3>& points)
    {
        m_hull = m_builder->buildCollisionGeometry(static_cast<uint32_t>(points.size()), points.data());
        return m_hull;
    }

    /**
        Every vertex must lie inside or on every face plane. This is the defining property of a
        convex hull, and it catches a mis-oriented normal or a wrong plane offset — the two ways
        this code can be subtly wrong while still producing plausible-looking geometry.
    */
    void expectAllPointsInside(const CollisionHull& hull, float tolerance = kPlaneTolerance)
    {
        for (uint32_t p = 0; p < hull.polygonDataCount; ++p)
        {
            for (uint32_t v = 0; v < hull.pointsCount; ++v)
            {
                EXPECT_LE(planeDistance(hull.polygonData[p], hull.points[v]), tolerance)
                    << "point " << v << " lies outside face " << p;
            }
        }
    }

    /** Face normals must point away from the hull's interior, or contacts push the wrong way. */
    void expectNormalsOutward(const CollisionHull& hull)
    {
        const NvcVec3 center = hullCentroid(hull);

        for (uint32_t p = 0; p < hull.polygonDataCount; ++p)
        {
            EXPECT_LT(planeDistance(hull.polygonData[p], center), 0.0f)
                << "face " << p << " has its normal pointing inwards";
        }
    }

    void expectIndicesInRange(const CollisionHull& hull)
    {
        for (uint32_t p = 0; p < hull.polygonDataCount; ++p)
        {
            const HullPolygon& polygon = hull.polygonData[p];
            EXPECT_GE(polygon.vertexCount, 3);
            EXPECT_LE(static_cast<uint32_t>(polygon.indexBase) + polygon.vertexCount, hull.indicesCount);

            for (uint32_t i = 0; i < polygon.vertexCount; ++i)
            {
                EXPECT_LT(hull.indices[polygon.indexBase + i], hull.pointsCount);
            }
        }
    }

    static std::vector<NvcVec3> cubeCorners()
    {
        return { { -1, -1, -1 }, { 1, -1, -1 }, { 1, 1, -1 }, { -1, 1, -1 },
                 { -1, -1, 1 },  { 1, -1, 1 },  { 1, 1, 1 },  { -1, 1, 1 } };
    }

    ConvexMeshBuilder* m_builder = nullptr;
    CollisionHull*     m_hull    = nullptr;
};

// ─── Basic shapes ─────────────────────────────────────────────────────────────

TEST_F(ConvexHullTest, CubeGivesEightPointsAndSixFaces)
{
    CollisionHull* hull = build(cubeCorners());
    ASSERT_NE(hull, nullptr);

    EXPECT_EQ(hull->pointsCount, 8u);
    EXPECT_EQ(hull->polygonDataCount, 6u);  // quads, not triangles

    expectAllPointsInside(*hull);
    expectNormalsOutward(*hull);
    expectIndicesInRange(*hull);
}

// The point of this builder: a shape that is not a box must not come back as one.
TEST_F(ConvexHullTest, TetrahedronIsNotABoundingBox)
{
    CollisionHull* hull = build({ { 0, 0, 0 }, { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } });
    ASSERT_NE(hull, nullptr);

    EXPECT_EQ(hull->pointsCount, 4u);
    EXPECT_EQ(hull->polygonDataCount, 4u);

    expectAllPointsInside(*hull);
    expectNormalsOutward(*hull);
    expectIndicesInRange(*hull);
}

TEST_F(ConvexHullTest, InteriorPointsAreDiscarded)
{
    std::vector<NvcVec3> points = cubeCorners();
    points.push_back({ 0.0f, 0.0f, 0.0f });     // dead centre
    points.push_back({ 0.5f, -0.25f, 0.1f });   // somewhere inside

    CollisionHull* hull = build(points);
    ASSERT_NE(hull, nullptr);

    // A hull is defined by its extreme points; interior ones must not survive into the geometry.
    EXPECT_EQ(hull->pointsCount, 8u);
    expectAllPointsInside(*hull);
}

TEST_F(ConvexHullTest, HullWrapsAnIrregularCloud)
{
    std::vector<NvcVec3> points = {
        { 0.0f, 2.0f, 0.0f },  { -1.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, -1.0f },
        { 1.0f, 0.0f, 1.0f },  { -1.0f, 0.0f, 1.0f },  { 0.0f, -0.5f, 0.0f }
    };

    CollisionHull* hull = build(points);
    ASSERT_NE(hull, nullptr);

    EXPECT_GE(hull->polygonDataCount, 4u);
    expectAllPointsInside(*hull);
    expectNormalsOutward(*hull);
    expectIndicesInRange(*hull);

    // Every input point must be enclosed, not just the ones that became hull vertices.
    for (const NvcVec3& point : points)
    {
        for (uint32_t p = 0; p < hull->polygonDataCount; ++p)
        {
            EXPECT_LE(planeDistance(hull->polygonData[p], point), kPlaneTolerance);
        }
    }
}

// ─── Degenerate input ─────────────────────────────────────────────────────────

TEST_F(ConvexHullTest, CoplanarPointsFallBackToABox)
{
    // A flat quad has no volume, so no hull exists. Falling back keeps the chunk collidable
    // instead of dropping it out of the simulation entirely.
    CollisionHull* hull = build({ { -1, 0, -1 }, { 1, 0, -1 }, { 1, 0, 1 }, { -1, 0, 1 } });
    ASSERT_NE(hull, nullptr);

    EXPECT_EQ(hull->pointsCount, 8u);
    EXPECT_EQ(hull->polygonDataCount, 6u);
    expectIndicesInRange(*hull);
}

TEST_F(ConvexHullTest, CollinearPointsFallBackToABox)
{
    CollisionHull* hull = build({ { 0, 0, 0 }, { 1, 0, 0 }, { 2, 0, 0 }, { 3, 0, 0 } });
    ASSERT_NE(hull, nullptr);
    EXPECT_EQ(hull->pointsCount, 8u);
}

TEST_F(ConvexHullTest, TooFewPointsFallBackToABox)
{
    CollisionHull* hull = build({ { 0, 0, 0 }, { 1, 1, 1 } });
    ASSERT_NE(hull, nullptr);
    EXPECT_EQ(hull->pointsCount, 8u);
}

TEST_F(ConvexHullTest, EmptyInputReturnsNull)
{
    EXPECT_EQ(m_builder->buildCollisionGeometry(0, nullptr), nullptr);

    const NvcVec3 point = { 0, 0, 0 };
    EXPECT_EQ(m_builder->buildCollisionGeometry(0, &point), nullptr);
}

TEST_F(ConvexHullTest, ReleasingNullHullIsSafe)
{
    m_builder->releaseCollisionHull(nullptr);
}

// ─── Through the fracture pipeline ────────────────────────────────────────────

TEST_F(ConvexHullTest, FracturedChunksGetHullsThatAreNotAllBoxes)
{
    // A cube sliced into 8 pieces yields box-shaped chunks, so slice unevenly and use voronoi to
    // get chunks that a bounding box would visibly misrepresent.
    const uint32_t verticesCount = 24;
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
    const uint32_t indices[36] = { 0,  2,  3,  0,  3,  1,  8,  4,  5,  8,  5,  9,
                                   10, 6,  7,  10, 7,  11, 12, 13, 14, 12, 14, 15,
                                   16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23 };

    Mesh* mesh = NvBlastExtUnityCreateMesh(positions, normals, uvs, verticesCount, indices, 36);
    ASSERT_NE(mesh, nullptr);

    VoronoiConfiguration config(8);
    Fracturer* fracturer = NvBlastExtUnityCreateVoronoiFracturer(config);
    ASSERT_NE(fracturer, nullptr);

    ConvexMeshBuilder* builder = NvBlastExtUnityCreateCollisionBuilder();
    AuthoringResult*   result  = NvBlastExtUnityFractureMesh(mesh, 1, fracturer, builder, nullptr);

    ASSERT_NE(result, nullptr);
    ASSERT_GT(result->chunkCount, 1u);
    ASSERT_NE(result->collisionHull, nullptr);

    const uint32_t hullCount = result->collisionHullOffset[result->chunkCount];
    ASSERT_GT(hullCount, 0u);

    uint32_t nonBoxHulls = 0;
    for (uint32_t i = 0; i < hullCount; ++i)
    {
        const CollisionHull* hull = result->collisionHull[i];
        ASSERT_NE(hull, nullptr);
        EXPECT_GE(hull->pointsCount, 4u);
        expectAllPointsInside(*hull);
        expectNormalsOutward(*hull);

        // A box has exactly 6 faces; voronoi cells generally have more.
        if (hull->polygonDataCount != 6 || hull->pointsCount != 8)
        {
            ++nonBoxHulls;
        }
    }

    EXPECT_GT(nonBoxHulls, 0u) << "every chunk came back as a box — hulls are not being built";

    NvBlastExtUnityReleaseAuthoringResult(*builder, result);
    NvBlastExtUnityReleaseCollisionBuilder(builder);
    NvBlastExtUnityReleaseFracturer(fracturer);
    NvBlastExtUnityReleaseMesh(mesh);
}

}  // namespace
