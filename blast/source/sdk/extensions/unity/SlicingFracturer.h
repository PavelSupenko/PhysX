#include "NvBlastGlobals.h"
#include "NvBlastFracturer.h"

using namespace Nv::Blast;

class SlicingFracturer : public Fracturer
{
private:
    int32_t x_slices;
    int32_t y_slices;
    int32_t z_slices;
    float angleVariation;
    float offsetVariation;

public:
    SlicingFracturer(int32_t x_slices = 1, int32_t y_slices = 1, int32_t z_slices = 1, float angleVariation = 0.0f, float offsetVariation = 0.0f)
    {
        this->x_slices = x_slices;
        this->y_slices = y_slices;
        this->z_slices = z_slices;
        this->angleVariation = angleVariation;
        this->offsetVariation = offsetVariation;
    }

public:
    bool fracture(FractureTool* fTool, VoronoiSitesGenerator* voronoiSitesGenerator, RandomGeneratorBase* rng, uint32_t id, NvBlastLog logFn)
    {
		NVBLASTLL_LOG_DEBUG(logFn, "Fracturing with Slicing...");

        // todo: in future create slicing config in managed code
        SlicingConfiguration slConfig;
        slConfig.x_slices = x_slices;
        slConfig.y_slices = y_slices;
        slConfig.z_slices = z_slices;
        slConfig.angle_variations = angleVariation;
        slConfig.offset_variations = offsetVariation;
        if (fTool->slicing(0, slConfig, false, rng) != 0)
        {
			NVBLASTLL_LOG_ERROR(logFn, "Failed to fracture with Slicing");
			return false;
        }

        return true;
    }
};