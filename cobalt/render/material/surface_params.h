#ifndef CBLT_RENDER_SURFACE_PARAMS_H
#define CBLT_RENDER_SURFACE_PARAMS_H

#include "math/vec4.h"
#include "render/data/color.h"

namespace cblt::render {

// Material parameters based on the Disney Principled BSDF
template<typename colorType, typename scalarType>
struct CoPrincipledParameters {
    colorType baseColor;
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

using CoSurfaceParams = CoPrincipledParameters<CoSpectrum, float>;

} // namespace cblt::render

#endif // CBLT_RENDER_SURFACE_PARAMS_H
