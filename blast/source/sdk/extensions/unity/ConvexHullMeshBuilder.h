#ifndef CONVEXHULLMESHBUILDER_H
#define CONVEXHULLMESHBUILDER_H

#include "NvBlastExtAuthoringConvexMeshBuilder.h"
#include "NvBlastExtAuthoringTypes.h"  // CollisionHull / HullPolygon — only forward-declared above

namespace Nv
{
namespace Blast
{

/**
    Builds real convex hulls, with no dependency on PhysX.

    Blast's own ConvexMeshBuilder implementation lives in NvBlastExtPhysX, which this project does
    not build, so collision geometry used to fall back to an axis-aligned box per chunk. Boxes make
    fractured pieces collide as if they were crates: they never interlock and always leave gaps.

    The hull is computed with btConvexHullComputer, the quickhull already compiled into
    NvBlastExtAuthoring as part of V-HACD — so this costs no new dependency.

    Degenerate input (fewer than four points, or points that are collinear or coplanar) has no
    volume and cannot form a hull. Rather than fail, such chunks fall back to their bounding box,
    which keeps a sliver of geometry collidable instead of dropping it out of the simulation.
*/
class ConvexHullMeshBuilder : public ConvexMeshBuilder
{
public:
    virtual ~ConvexHullMeshBuilder() {}

    virtual void release() override;

    virtual CollisionHull* buildCollisionGeometry(uint32_t verticesCount, const NvcVec3* vertexData) override;

    virtual void releaseCollisionHull(CollisionHull* hull) const override;
};

}  // namespace Blast
}  // namespace Nv

#endif  // ifndef CONVEXHULLMESHBUILDER_H
