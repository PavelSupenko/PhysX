#include "NvBlastExtAuthoring.h"
#include "NvBlastGlobals.h"
#include "NvBlastFracturer.h"
#include <boost/multiprecision/cpp_int.hpp>

#include "NvVec3.h"
#include "NvVec2.h"
#include "NvBounds3.h"
#include <vector>
#include <queue>
#include <map>
#include <cmath>

using namespace nvidia;

using namespace Nv::Blast;
using namespace boost::multiprecision;

/**
    Exact rational vector types.
*/
struct RVec3
{
    cpp_rational x, y, z;
    RVec3() {}

    bool isZero()
    {
        return x.is_zero() && y.is_zero() && z.is_zero();
    }

    RVec3(cpp_rational _x, cpp_rational _y, cpp_rational _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }

    RVec3(const NvcVec3& p)
    {
        x = cpp_rational(p.x);
        y = cpp_rational(p.y);
        z = cpp_rational(p.z);
    }
    NvVec3 toVec3()
    {
        return { x.convert_to<float>(), y.convert_to<float>(), z.convert_to<float>() };
    }

    void normalize()
    {
        cpp_rational sumSquares = x * x + y * y + z * z;
        double magnitude = std::sqrt(static_cast<double>(sumSquares));
        if (magnitude != 0)
        {
            x /= magnitude;
            y /= magnitude;
            z /= magnitude;
        }
    }

    RVec3 operator-(const RVec3& b) const
    {
        return RVec3(x - b.x, y - b.y, z - b.z);
    }
    RVec3 operator+(const RVec3& b) const
    {
        return RVec3(x + b.x, y + b.y, z + b.z);
    }
    RVec3 cross(const RVec3& in) const
    {
        return RVec3(y * in.z - in.y * z, in.x * z - x * in.z, x * in.y - in.x * y);
    }
    cpp_rational dot(const RVec3& in) const
    {
        return x * in.x + y * in.y + z * in.z;
    }
    RVec3 operator*(const cpp_rational& in) const
    {
        return RVec3(x * in, y * in, z * in);
    }
    RVec3 operator/(const cpp_rational& in) const
    {
        return RVec3(x / in, y / in, z / in);
    }
};

class CutOutFracturer : public Fracturer
{
private:
    NvcVec3 point;
    NvcVec3 normal;
    uint8_t* bitmap;
    uint32_t width;
    uint32_t height;

public:
    CutOutFracturer(NvcVec3 point, NvcVec3 normal, uint8_t* bitmap, uint32_t width, uint32_t height)
    {
        this->point = point;
        this->normal = normal;
        this->bitmap = bitmap;
        this->width = width;
        this->height = height;
    }

public:
    bool fracture(FractureTool* fTool, VoronoiSitesGenerator* voronoiSitesGenerator, RandomGeneratorBase* rng, uint32_t id, NvBlastLog logFn)
    {
		NVBLASTLL_LOG_DEBUG(logFn, "Cutout fracturing...");

        CutoutConfiguration cutoutConfig;
        RVec3 axis = normal;
		if (axis.isZero())
		{
			axis = RVec3(0.f, 0.f, 1.f);
		}
		axis.normalize();
		cpp_rational d = axis.dot(RVec3(0.f, 0.f, 1.f));
		NvQuat q;
		if (d < (1e-6f - 1.0f))
		{
			q = NvQuat(NvPi, NvVec3(1.f, 0.f, 0.f));
		}
		else if (d < 1.f)
		{
			float s = std::sqrt(static_cast<float>((1 + d) * 2));
			float invs = 1 / s;
			auto c = axis.cross(RVec3(0.f, 0.f, 1.f));
			q = NvQuat(static_cast<float>(c.x * invs), static_cast<float>(c.y * invs), static_cast<float>(c.z * invs), static_cast<float>(s * 0.5f));
			q.normalize();
		}
		cutoutConfig.transform.q = reinterpret_cast<NvcQuat&>(q);
		cutoutConfig.transform.p = point;
		if (bitmap != nullptr)
		{
			cutoutConfig.cutoutSet = NvBlastExtAuthoringCreateCutoutSet();
			NvBlastExtAuthoringBuildCutoutSet(*cutoutConfig.cutoutSet, bitmap, width, height, 0.001f, 1.f, false, true);
		}
		if (fTool->cutout(0, cutoutConfig, false, rng) != 0)
		{
            NVBLASTLL_LOG_ERROR(logFn, "Failed to fracture with Cutout");
            return false;
		}

        return true;
    }
};