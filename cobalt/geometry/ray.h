#ifndef CBLT_GEOM_RAY_H
#define CBLT_GEOM_RAY_H

#include "math/math_types.h"

namespace {
const cblt::simd::vec3f kOne(1.f, 1.f, 1.f);
const cblt::simd::vec3f kZero(1e-6f, 1e-6f, 1e-6f);
} // namespace

namespace cblt::geom {

struct CoRay {

    CoRay(): pos(0.f), dir(0.f), invDir(0.f), maxDist(0.f) {};

    CoRay(const simd::vec3f &_pos, const simd::vec3f &_dir, float _maxDist)
        : pos{_pos.xyz}, dir{_dir.xyz}, invDir{kOne / (dir)}, maxDist{_maxDist} {
    }

    simd::vec3f pos;
    simd::vec3f dir;
    simd::vec3f invDir;

    float maxDist;
};

} // namespace cblt::geom

#endif // CBLT_GEOM_RAY_H
