#ifndef CBLT_RENDER_SURFACE_PARAMS_H
#define CBLT_RENDER_SURFACE_PARAMS_H

#include "vec4.h"

namespace cblt::render {

// Material parameters based on the Disney Principled BSDF
template<typename vectorType, typename scalarType>
struct CoPrincipledParams {
    vectorType baseColor;
    scalarType metallic;
    scalarType subsurface;
    // TODO: sus! should use to derive IoR
    scalarType ior;
    scalarType specular;
    scalarType specularTint;
    scalarType specularTransmission;
    scalarType roughness;
    scalarType anisotropic;
    scalarType sheen;
    scalarType sheenTint;
    scalarType clearcoat;
    scalarType clearcoatGloss;
};

using CoSurfaceParams = CoPrincipledParams<vec4f, float>;

} // namespace cblt::render

#endif // CBLT_RENDER_SURFACE_PARAMS_H
