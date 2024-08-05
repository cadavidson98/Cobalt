#ifndef CBLT_GEOM_RAY_H
#define CBLT_GEOM_RAY_H

#include "simd/simd_vec4.h"

namespace {
    static const cblt::simd::vec4f one(1.f, 1.f, 1.f, 1.f);
    static const cblt::simd::vec4f zero(1e-6f, 1e-6f, 1e-6f, 1e-6f);
}

namespace cblt::geom {

struct CoRay {

CoRay(const simd::vec4f &_pos, const simd::vec4f &_dir, float _maxDist) 
    : pos{ _pos.xyzw }, dir{ _dir.xyzw }, invDir{ one / simd::max(dir, zero) }, maxDist{ _maxDist } {
}

simd::vec4f pos;
simd::vec4f dir;
simd::vec4f invDir;

float maxDist;

};

}  // namespace cblt::geom

#endif  // CBLT_GEOM_RAY_H