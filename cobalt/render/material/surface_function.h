#ifndef CBLT_RENDER_SURFACE_FUNC_H
#define CBLT_RENDER_SURFACE_FUNC_H

#include "surface_params.h"

#include "math/math_types.h"
#include "render/data/color.h"

namespace cblt::render {

class CoSurfaceFunction {
public:
    virtual CoColor BSDF(
        const vec3f &omegaI,
        const vec3f &omegaO,
        const vec3f &normal,
        const CoColor &incidentRadiance,
        const CoSurfaceParams &surfaceParams
    ) const = 0;
    virtual bool isDiracDelta() const = 0;

private:
    CoSurfaceFunction();
};

} // namespace cblt::render

#endif // CBLT_RENDER_SURFACE_FUNC_H
