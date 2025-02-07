#ifndef CBLT_RENDER_GGX_H
#define CBLT_RENDER_GGX_H

#include "surface_function.h"
#include "surface_params.h"
#include "vec3.h"

namespace cblt::render {

class CoMicrofacetSpecular : public CoSurfaceFunction {
public:
    CoColor BSDF(
        const vec3f &omegaI,
        const vec3f &omegaO,
        const vec3f &normal,
        const CoColor &incidentRadiance,
        const CoSurfaceParams &surfaceParams
    ) const override;

    bool isDiracDelta() const override {
        return false;
    };
};

} // namespace cblt::render

#endif // CBLT_RENDER_GGX_H
