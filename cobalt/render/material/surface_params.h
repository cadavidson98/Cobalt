#ifndef CBLT_RENDER_SURFACE_PARAMS_H
#define CBLT_RENDER_SURFACE_PARAMS_H

#include "vec4.h"

namespace cblt::render {

// Material parameters based on the Disney Principled BSDF
struct CoSurfaceParams {
        vec4f baseColor;
        float metallic;
        float subsurface;
        // TODO: sus! should use to derive IoR
        float ior;
        float specular;
        float specularTint;
        float specularTransmission;
        float roughness;
        float anisotropic;
        float sheen;
        float sheenTint;
        float clearcoat;
        float clearcoatGloss;
};

} // namespace cblt::render

#endif // CBLT_RENDER_SURFACE_PARAMS_H
