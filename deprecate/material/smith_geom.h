#ifndef CBLT_SMITH_GEOM
#define CBLT_SMITH_GEOM

#include <cmath>

namespace cblt
{
    inline float SmithPartialGeomGGX(float cos_theta, float alpha)
    {
        float alpha_sqr = alpha * alpha;
        return 2.f / (1.f + std::sqrtf(alpha_sqr + (1.f - alpha_sqr) * cos_theta * cos_theta));
    }
}
#endif  // CBLT_SMITH_GEOM