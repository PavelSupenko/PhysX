#include "NvBlastGlobals.h"
#include "NvBlastFracturer.h"

using namespace Nv::Blast;

class ClusteredVoronoiFracturer : public Fracturer
{
private:
    uint32_t cellsCount;
    uint32_t clusterCount;
    float clusterRad;

public:
    ClusteredVoronoiFracturer(uint32_t cellsCount = 5, uint32_t clusterCount = 5, float clusterRad = 1.0f)
    {
        this->cellsCount = cellsCount;
        this->clusterCount = clusterCount;
        this->clusterRad = clusterRad;
    }

public:
    bool fracture(FractureTool* fTool, VoronoiSitesGenerator* voronoiSitesGenerator, RandomGeneratorBase* rng, uint32_t id, NvBlastLog logFn)
    {
		NVBLASTLL_LOG_DEBUG(logFn, "Fracturing with Clustered Voronoi...");
        voronoiSitesGenerator->clusteredSitesGeneration(cellsCount, clusterCount, clusterRad);
		const NvcVec3* sites = nullptr;
		uint32_t sitesCount = voronoiSitesGenerator->getVoronoiSites(sites);
		if (fTool->voronoiFracturing(id, sitesCount, sites, false) != 0)
		{
			NVBLASTLL_LOG_ERROR(logFn, "Failed to fracture with Clustered Voronoi");
			return false;
		}

        return true;
    }
};