#ifndef NVBLASTEXTUNITYCONFIGS_H
#define NVBLASTEXTUNITYCONFIGS_H

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

struct PlaneCutConfiguration
{
    NvcVec3 point;
    NvcVec3 normal;

    PlaneCutConfiguration(NvcVec3 point = {0, 0, 0}, NvcVec3 normal = {1, 0, 0})
    {
        this->point = point;
        this->normal = normal;
    }
};

struct CutOutConfiguration
{
    NvcVec3 point;
    NvcVec3 normal;
    uint8_t* bitmap;
    uint32_t width;
    uint32_t height;

    CutOutConfiguration(NvcVec3 point = {0, 0, 0}, NvcVec3 normal = {1, 0, 0}, uint8_t* bitmap = nullptr, uint32_t width = 0, uint32_t height = 0)
    {
        this->point = point;
        this->normal = normal;
        this->bitmap = bitmap;
        this->width = width;
        this->height = height;
    }
};

#endif