#include "NvBlastGlobals.h"
#include "NvBlastFracturer.h"

using namespace Nv::Blast;

class PlaneCutFracturer : public Fracturer
{
private:
    NvcVec3 point;
    NvcVec3 normal;

public:
    PlaneCutFracturer(NvcVec3 point, NvcVec3 normal)
    {
        this->point = point;
        this->normal = normal;
    }

public:
    bool fracture(FractureTool* fTool, VoronoiSitesGenerator* voronoiSitesGenerator, RandomGeneratorBase* rng, uint32_t id, NvBlastLog logFn)
    {
		NVBLASTLL_LOG_DEBUG(logFn, "Plane cut fracturing...");

		NoiseConfiguration noise;
        if (fTool->cut(0, normal, point, noise, false, rng) != 0)
        {
			NVBLASTLL_LOG_ERROR(logFn, "Failed to fracture with Cutout (in half-space, plane cut)");
			return false;
        }

        return true;
    }
};