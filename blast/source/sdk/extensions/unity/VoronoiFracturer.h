#include "NvBlastGlobals.h"
#include "NvBlastFracturer.h"

using namespace Nv::Blast;

class VoronoiFracturer : public Fracturer
{
private:
    uint32_t cellsCount;

public:
    VoronoiFracturer(uint32_t cellsCount = 5)
    {
        this->cellsCount = cellsCount;
    }

public:
    bool fracture(FractureTool* fTool, VoronoiSitesGenerator* voronoiSitesGenerator, RandomGeneratorBase* rng, uint32_t id, NvBlastLog logFn)
    {
		NVBLASTLL_LOG_DEBUG(logFn, "Fracturing with Voronoi...");
		voronoiSitesGenerator->uniformlyGenerateSitesInMesh(cellsCount);
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