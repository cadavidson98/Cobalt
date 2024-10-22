#ifndef CBLT_MATERIAL_SURFACE_PARAMS_H
#define CBLT_MATERIAL_SURFACE_PARAMS_H

#include "vec4.h"

namespace cblt::material {

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

} // namespace cblt::material

#endif // CBLT_MATERIAL_SURFACE_PARAMS_H
