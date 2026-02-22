#ifndef CBLT_RENDER_DISNEY_PRINCIPLED
#define CBLT_RENDER_DISNEY_PRINCIPLED

#include "color.h"

#include "math/math_types.h"

namespace cblt::render {

struct PrincipledParameters {
    Color baseColor;
    float metallic;
    float subsurface;
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

Color evaluatePrincipledBSDF(vec3f incoming, vec3f normal, vec3f outgoing, const PrincipledParameters &parameters);

} // namespace cblt::render

#endif // CBLT_RENDER_DISNEY_PRINCIPLED
