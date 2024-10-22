#ifndef CBLT_MATERIAL_SURFACE_FUNC_H
#define CBLT_MATERIAL_SURFACE_FUNC_H

#include "surface_params.h"
#include "vec3.h"
#include "vec4.h"

namespace cblt::material {

namespace {
using CoColor = vec4f;
}

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

} // namespace cblt::material

#endif // CBLT_MATERIAL_SURFACE_FUNC_H
