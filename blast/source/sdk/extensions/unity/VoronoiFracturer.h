#include "NvBlastGlobals.h"
#include "NvBlastFracturer.h"
#include "NvBlastExtUnityConfigs.h"

using namespace Nv::Blast;

class VoronoiFracturer : public Fracturer
{
private:
    VoronoiConfiguration settings;

public:
    VoronoiFracturer(VoronoiConfiguration settings)
    {
        this->settings = settings;
    }

public:
    bool fracture(FractureTool* fTool, VoronoiSitesGenerator* voronoiSitesGenerator, RandomGeneratorBase* rng, uint32_t id, NvBlastLog logFn)
    {
		NVBLASTLL_LOG_DEBUG(logFn, "Fracturing with Voronoi...");
		voronoiSitesGenerator->uniformlyGenerateSitesInMesh(settings.cellsCount);
		const NvcVec3* sites = nullptr;
		uint32_t sitesCount = voronoiSitesGenerator->getVoronoiSites(sites);
		if (fTool->voronoiFracturing(id, sitesCount, sites, false) != 0)
		{
			NVBLASTLL_LOG_ERROR(logFn, "Failed to fracture with Voronoi");
			return false;
		}

        return true;
    }
};