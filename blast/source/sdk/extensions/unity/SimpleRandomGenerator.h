#ifndef SIMPLERANDOMGENERATOR_H
#define SIMPLERANDOMGENERATOR_H

#include "NvBlastExtAuthoringFractureTool.h"

#include <cstdint>

/**
    Deterministic random generator with per-instance state.

    Authoring sessions re-seed before every fracture operation so that repeating an operation with
    the same seed reproduces the same chunks. That contract requires the generator state to be local:
    an implementation built on rand()/srand() shares one global stream across every generator in the
    process, so an unrelated fracture — or any other caller of rand() — would shift the results.

    The generator is a SplitMix64 bit mixer. It is not a general-purpose statistical RNG, but it is
    stable across platforms and standard-library versions, which std::mt19937-based distributions are
    not: identical seeds must produce identical assets on every target the SDK builds for.
*/
class SimpleRandomGenerator : public Nv::Blast::RandomGeneratorBase
{
public:
    SimpleRandomGenerator() : mState(0)
    {
    }

    virtual float getRandomValue() override
    {
        // SplitMix64: advance the state, then mix it into a well-distributed 64-bit value.
        mState += UINT64_C(0x9E3779B97F4A7C15);
        uint64_t z = mState;
        z          = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
        z          = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
        z          = z ^ (z >> 31);

        // Take the high 24 bits so the result divides exactly into a float mantissa, giving a value
        // uniformly distributed over [0, 1] as the RandomGeneratorBase contract requires.
        const uint32_t bits = static_cast<uint32_t>(z >> 40);
        return static_cast<float>(bits) / static_cast<float>((1u << 24) - 1u);
    }

    virtual void seed(int32_t seed) override
    {
        mState = static_cast<uint64_t>(static_cast<uint32_t>(seed));
    }

    virtual ~SimpleRandomGenerator() override
    {
    }

private:
    uint64_t mState;
};

#endif  // ifndef SIMPLERANDOMGENERATOR_H
