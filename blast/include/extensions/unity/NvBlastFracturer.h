#ifndef NVBLASTFRACTURER_H
#define NVBLASTFRACTURER_H

#include "NvBlastExtAuthoringFractureTool.h"
#include "NvBlastExtUnityConfigs.h"

namespace Nv
{
namespace Blast
{

/**
    A fracture operation and its parameters, with no execution logic of its own.

    This used to be an abstract strategy holding a fracture() method that drove a FractureTool. That
    shape only fits the one-shot pipeline, where the tool is created, driven once and destroyed
    inside a single call: it takes the tool as an argument, so it cannot be applied to a session's
    long-lived tool, and it hard-codes which chunk it fractures.

    Describing the operation instead of performing it lets the same value drive both paths — the
    one-shot API keeps its signatures, and a session applies the same descriptor to any chunk.
*/
struct Fracturer
{
    enum Type
    {
        Voronoi,
        ClusteredVoronoi,
        Slicing,
        PlaneCut,
        CutOut,
        Islands,
    };

    Type type;

    // Only the member matching `type` is meaningful. These are small aggregates and a session may
    // outlive several operations, so they are stored side by side rather than in a union — the
    // memory saved would not justify hand-managing the active member.
    VoronoiConfiguration          voronoi;
    ClusteredVoronoiConfiguration clusteredVoronoi;
    SlicingConfiguration          slicing;
    PlaneCutConfiguration         planeCut;
    CutOutConfiguration           cutOut;

    Fracturer() : type(Voronoi) {}
};

}  // namespace Blast
}  // namespace Nv

#endif  // ifndef NVBLASTFRACTURER_H
