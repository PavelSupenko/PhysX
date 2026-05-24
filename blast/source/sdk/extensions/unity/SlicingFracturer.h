#include "NvBlastGlobals.h"
#include "NvBlastFracturer.h"

using namespace Nv::Blast;

class SlicingFracturer : public Fracturer
{
private:
    SlicingConfiguration settings;

public:
    SlicingFracturer(SlicingConfiguration settings)
    {
        this->settings = settings;
    }

public:
    bool fracture(FractureTool* fTool, VoronoiSitesGenerator* voronoiSitesGenerator, RandomGeneratorBase* rng, uint32_t id, NvBlastLog logFn)
    {
		NVBLASTLL_LOG_DEBUG(logFn, "Fracturing with Slicing...");

        if (fTool->slicing(0, settings, false, rng) != 0)
        {
			NVBLASTLL_LOG_ERROR(logFn, "Failed to fracture with Slicing");
			return false;
        }

        return true;
    }
};