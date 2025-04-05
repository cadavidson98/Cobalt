#ifndef CBLT_RENDER_LAMBERTIAN_DIFFUSE_H
#define CBLT_RENDER_LAMBERTIAN_DIFFUSE_H

#include "surface_function.h"
#include "surface_params.h"

#include <cmath>

namespace cblt::render {
class CoLamberianDiffuse : public CoSurfaceFunction {
public:
    CoColor BSDF(const vec3f &omegaI, const vec3f &omegaO, const vec3f &normal, const CoSurfaceParams &surfaceParams)
        const override {
        const float NDotO = std::max(dot(omegaO, normal), 0.f);
        return surfaceParams.baseColor;
    };

    bool isDiracDelta() const override {
        return false;
    };
};
} // namespace cblt::render

#endif // CBLT_RENDER_LAMBERTIAN_DIFFUSE_H
