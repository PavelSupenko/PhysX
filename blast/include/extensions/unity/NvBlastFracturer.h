#ifndef NVBLASTFRACTURER_H
#define NVBLASTFRACTURER_H

#include "NvBlastExtAuthoringFractureTool.h"

namespace Nv
{
namespace Blast
{

/**
Base class for all fracturers.
*/
class Fracturer
{
public:
    virtual bool fracture(FractureTool* fTool, VoronoiSitesGenerator* voronoiSitesGenerator, RandomGeneratorBase* rng, uint32_t id, NvBlastLog logFn) = 0;
};

} // namespace Blast
} // namespace Nv


#endif // ifndef NVBLASTFRACTURER_H
