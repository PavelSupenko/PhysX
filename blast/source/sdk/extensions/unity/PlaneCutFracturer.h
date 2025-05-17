#include "NvBlastGlobals.h"
#include "NvBlastFracturer.h"

using namespace Nv::Blast;

class PlaneCutFracturer : public Fracturer
{
private:
    PlaneCutConfiguration settings;

public:
    PlaneCutFracturer(PlaneCutConfiguration settings)
    {
        this->settings = settings;
    }

public:
    bool fracture(FractureTool* fTool, VoronoiSitesGenerator* voronoiSitesGenerator, RandomGeneratorBase* rng, uint32_t id, NvBlastLog logFn)
    {
		NVBLASTLL_LOG_DEBUG(logFn, "Plane cut fracturing...");

		NoiseConfiguration noise;
        if (fTool->cut(0, settings.normal, settings.point, noise, false, rng) != 0)
        {
			NVBLASTLL_LOG_ERROR(logFn, "Failed to fracture with Cutout (in half-space, plane cut)");
			return false;
        }

        return true;
    }
};