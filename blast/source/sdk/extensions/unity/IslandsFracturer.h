#include "NvBlastGlobals.h"
#include "NvBlastFracturer.h"

using namespace Nv::Blast;

class IslandsFracturer : public Fracturer
{
public:
	IslandsFracturer()
    {
    }

public:
	bool fracture(FractureTool* fTool, VoronoiSitesGenerator* voronoiSitesGenerator, RandomGeneratorBase* rng, uint32_t id, NvBlastLog logFn)
	{
		NVBLASTLL_LOG_DEBUG(logFn, "Generate chunks from islands...");
		fTool->islandDetectionAndRemoving(0, true);
		return true;
	}
};