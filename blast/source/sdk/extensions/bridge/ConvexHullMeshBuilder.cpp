#include "ConvexHullMeshBuilder.h"

#include "btConvexHullComputer.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace Nv
{
namespace Blast
{

namespace
{

/** HullPolygon addresses vertices with 16-bit fields, so a hull cannot exceed this many indices. */
constexpr size_t kMaxHullIndices = 0xFFFF;

/**
    Builds an axis-aligned box hull around the points.

    Used for degenerate input that has no volume to wrap — a flat sliver or a line of points — where
    a convex hull is undefined but dropping the chunk's collision entirely would be worse.
*/
CollisionHull* buildAabbHull(uint32_t verticesCount, const NvcVec3* vertexData)
{
    NvcVec3 bbMin = vertexData[0];
    NvcVec3 bbMax = vertexData[0];

    for (uint32_t i = 1; i < verticesCount; ++i)
    {
        bbMin.x = std::min(bbMin.x, vertexData[i].x);
        bbMin.y = std::min(bbMin.y, vertexData[i].y);
        bbMin.z = std::min(bbMin.z, vertexData[i].z);
        bbMax.x = std::max(bbMax.x, vertexData[i].x);
        bbMax.y = std::max(bbMax.y, vertexData[i].y);
        bbMax.z = std::max(bbMax.z, vertexData[i].z);
    }

    CollisionHull* hull     = new CollisionHull;
    hull->pointsCount       = 8;
    hull->indicesCount      = 24;  // 6 quads
    hull->polygonDataCount  = 6;
    hull->points            = new NvcVec3[8];
    hull->indices           = new uint32_t[24];
    hull->polygonData       = new HullPolygon[6];

    hull->points[0] = bbMin;
    hull->points[1] = { bbMax.x, bbMin.y, bbMin.z };
    hull->points[2] = { bbMax.x, bbMax.y, bbMin.z };
    hull->points[3] = { bbMin.x, bbMax.y, bbMin.z };
    hull->points[4] = { bbMin.x, bbMin.y, bbMax.z };
    hull->points[5] = { bbMax.x, bbMin.y, bbMax.z };
    hull->points[6] = { bbMax.x, bbMax.y, bbMax.z };
    hull->points[7] = { bbMin.x, bbMax.y, bbMax.z };

    // Faces wound so their normals point outwards, matching the plane equations below.
    const uint32_t faceIndices[24] = {
        0, 4, 7, 3,  // -X
        1, 2, 6, 5,  // +X
        0, 1, 5, 4,  // -Y
        3, 7, 6, 2,  // +Y
        0, 3, 2, 1,  // -Z
        4, 5, 6, 7   // +Z
    };
    std::copy(faceIndices, faceIndices + 24, hull->indices);

    // plane is (nx, ny, nz, d) with n·p + d = 0 for any p on the face.
    const float planes[6][4] = {
        { -1.0f, 0.0f, 0.0f, bbMin.x },  { 1.0f, 0.0f, 0.0f, -bbMax.x },
        { 0.0f, -1.0f, 0.0f, bbMin.y },  { 0.0f, 1.0f, 0.0f, -bbMax.y },
        { 0.0f, 0.0f, -1.0f, bbMin.z },  { 0.0f, 0.0f, 1.0f, -bbMax.z }
    };

    for (uint32_t f = 0; f < 6; ++f)
    {
        hull->polygonData[f].vertexCount = 4;
        hull->polygonData[f].indexBase   = static_cast<uint16_t>(f * 4);
        std::copy(planes[f], planes[f] + 4, hull->polygonData[f].plane);
    }

    return hull;
}

}  // namespace

void ConvexHullMeshBuilder::release()
{
    delete this;
}

CollisionHull* ConvexHullMeshBuilder::buildCollisionGeometry(uint32_t verticesCount, const NvcVec3* vertexData)
{
    if (verticesCount == 0 || vertexData == nullptr)
    {
        return nullptr;
    }

    btConvexHullComputer computer;
    computer.compute(&vertexData[0].x, static_cast<int32_t>(sizeof(NvcVec3)), static_cast<int32_t>(verticesCount),
                     0.0f, 0.0f);

    // A solid hull needs at least a tetrahedron. Anything less means the input had no volume.
    if (computer.vertices.size() < 4 || computer.faces.size() < 4)
    {
        return buildAabbHull(verticesCount, vertexData);
    }

    const int32_t pointCount = computer.vertices.size();

    std::vector<NvcVec3> points(static_cast<size_t>(pointCount));
    NvcVec3              center = { 0.0f, 0.0f, 0.0f };

    for (int32_t i = 0; i < pointCount; ++i)
    {
        const btVector3& v = computer.vertices[i];
        points[static_cast<size_t>(i)] = { static_cast<float>(v.x()), static_cast<float>(v.y()),
                                           static_cast<float>(v.z()) };
        center.x += points[static_cast<size_t>(i)].x;
        center.y += points[static_cast<size_t>(i)].y;
        center.z += points[static_cast<size_t>(i)].z;
    }

    center.x /= static_cast<float>(pointCount);
    center.y /= static_cast<float>(pointCount);
    center.z /= static_cast<float>(pointCount);

    std::vector<uint32_t>    indices;
    std::vector<HullPolygon> polygons;
    polygons.reserve(static_cast<size_t>(computer.faces.size()));

    for (int32_t f = 0; f < computer.faces.size(); ++f)
    {
        const size_t base = indices.size();

        const btConvexHullComputer::Edge* firstEdge = &computer.edges[computer.faces[f]];
        const btConvexHullComputer::Edge* edge      = firstEdge;

        do
        {
            indices.push_back(static_cast<uint32_t>(edge->getTargetVertex()));
            edge = edge->getNextEdgeOfFace();
        } while (edge != firstEdge);

        const size_t vertexCount = indices.size() - base;

        if (vertexCount < 3 || indices.size() > kMaxHullIndices)
        {
            indices.resize(base);
            continue;
        }

        // Newell's method rather than a cross product of three picked vertices: hull faces are
        // n-gons whose consecutive vertices are often nearly collinear, which makes a single cross
        // product numerically unstable or outright zero.
        double nx = 0.0, ny = 0.0, nz = 0.0;
        NvcVec3 faceCenter = { 0.0f, 0.0f, 0.0f };

        for (size_t i = 0; i < vertexCount; ++i)
        {
            const NvcVec3& a = points[indices[base + i]];
            const NvcVec3& b = points[indices[base + (i + 1) % vertexCount]];

            nx += static_cast<double>(a.y - b.y) * static_cast<double>(a.z + b.z);
            ny += static_cast<double>(a.z - b.z) * static_cast<double>(a.x + b.x);
            nz += static_cast<double>(a.x - b.x) * static_cast<double>(a.y + b.y);

            faceCenter.x += a.x;
            faceCenter.y += a.y;
            faceCenter.z += a.z;
        }

        faceCenter.x /= static_cast<float>(vertexCount);
        faceCenter.y /= static_cast<float>(vertexCount);
        faceCenter.z /= static_cast<float>(vertexCount);

        const double length = std::sqrt(nx * nx + ny * ny + nz * nz);
        if (length < 1e-12)
        {
            indices.resize(base);  // Zero-area face contributes nothing to the hull
            continue;
        }

        nx /= length;
        ny /= length;
        nz /= length;

        // Orient outwards. The winding the computer reports is consistent, but which way it faces
        // is not something to assume — compare against the direction from the hull's centre.
        const double outward = nx * static_cast<double>(faceCenter.x - center.x) +
                               ny * static_cast<double>(faceCenter.y - center.y) +
                               nz * static_cast<double>(faceCenter.z - center.z);

        if (outward < 0.0)
        {
            nx = -nx;
            ny = -ny;
            nz = -nz;
            std::reverse(indices.begin() + static_cast<std::ptrdiff_t>(base), indices.end());
        }

        HullPolygon polygon;
        polygon.vertexCount = static_cast<uint16_t>(vertexCount);
        polygon.indexBase   = static_cast<uint16_t>(base);
        polygon.plane[0]    = static_cast<float>(nx);
        polygon.plane[1]    = static_cast<float>(ny);
        polygon.plane[2]    = static_cast<float>(nz);
        polygon.plane[3]    = -static_cast<float>(nx * static_cast<double>(faceCenter.x) +
                                                  ny * static_cast<double>(faceCenter.y) +
                                                  nz * static_cast<double>(faceCenter.z));

        polygons.push_back(polygon);
    }

    if (polygons.size() < 4)
    {
        return buildAabbHull(verticesCount, vertexData);
    }

    CollisionHull* hull    = new CollisionHull;
    hull->pointsCount      = static_cast<uint32_t>(points.size());
    hull->indicesCount     = static_cast<uint32_t>(indices.size());
    hull->polygonDataCount = static_cast<uint32_t>(polygons.size());
    hull->points           = new NvcVec3[points.size()];
    hull->indices          = new uint32_t[indices.size()];
    hull->polygonData      = new HullPolygon[polygons.size()];

    std::copy(points.begin(), points.end(), hull->points);
    std::copy(indices.begin(), indices.end(), hull->indices);
    std::copy(polygons.begin(), polygons.end(), hull->polygonData);

    return hull;
}

void ConvexHullMeshBuilder::releaseCollisionHull(CollisionHull* hull) const
{
    if (hull == nullptr)
    {
        return;
    }

    delete[] hull->points;
    delete[] hull->indices;
    delete[] hull->polygonData;
    delete hull;
}

}  // namespace Blast
}  // namespace Nv
