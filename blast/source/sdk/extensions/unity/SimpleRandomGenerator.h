#include "NvBlastExtAuthoringFractureTool.h"
#include "NvBlastGlobals.h" // for cstdlib to allow us to use rand() and RAND_MAX


class SimpleRandomGenerator : public Nv::Blast::RandomGeneratorBase
{
public:
	SimpleRandomGenerator() {
		remember = false;
	};

	virtual float getRandomValue()
	{
		float r = (float)rand();
		r = r / RAND_MAX;
		return r;
	}

	virtual void seed(int32_t seed)
	{
		srand(seed);
	}

	virtual ~SimpleRandomGenerator() {};

private:
	bool remember;
};