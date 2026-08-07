#ifndef NVBLASTEXTBRIDGECONFIGS_H
#define NVBLASTEXTBRIDGECONFIGS_H

#include "NvBlastGlobals.h"

using namespace Nv::Blast;

struct VoronoiConfiguration
{
    uint32_t cellsCount;

    VoronoiConfiguration(uint32_t cellsCount = 5)
    {
        this->cellsCount = cellsCount;
    }
};


struct ClusteredVoronoiConfiguration
{
    uint32_t cellsCount;
    uint32_t clusterCount;
    float clusterRad;

    ClusteredVoronoiConfiguration(uint32_t cellsCount = 5, uint32_t clusterCount = 5, float clusterRad = 1.0f)
    {
        this->cellsCount = cellsCount;
        this->clusterCount = clusterCount;
        this->clusterRad = clusterRad;
    }
};

// Plane cut and cutout take their parameters from the session API rather than from a struct here:
// both need noise settings, and cutout needs the full placement set, which the flat
// NvBlastExtBridgeCutoutConfiguration in NvBlastExtBridgeSession.h carries.

#endif