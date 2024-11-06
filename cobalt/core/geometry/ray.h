#ifndef CBLT_GEOM_RAY_H
#define CBLT_GEOM_RAY_H

#include "math/simd/simd_vec3.h"

namespace {
const cblt::simd::vec3f one(1.f, 1.f, 1.f);
} // namespace

namespace cblt::geom {

struct CoRay {

        CoRay(const simd::vec3f &_pos, const simd::vec3f &_dir, float _maxDist)
            : pos{_pos.xyz}, dir{_dir.xyz}, invDir{one / dir}, maxDist{_maxDist} {
        }

        simd::vec3f pos;
        simd::vec3f dir;
        simd::vec3f invDir;

        float maxDist;
};

} // namespace cblt::geom

#endif // CBLT_GEOM_RAY_H
