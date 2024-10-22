#ifndef CBLT_MATERIAL_H
#define CBLT_MATERIAL_H

#include "surface_function.h"
#include "surface_params.h"
#include "variant"
#include "vec2.h"
#include "vec4.h"

namespace cblt {
namespace {
class CoTexture;
using CoColor = vec4f;
} // namespace

namespace material {

class CoMaterial {
    public:
        bool isVisible(vec2f uv);
        CoSurfaceParams Resolve(vec2f uv);

    private:
        std::variant<CoColor, CoTexture *> baseColor;
        std::variant<float, CoTexture *> metallic;
        float subsurface;
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
} // namespace material
} // namespace cblt
#endif // CBLT_MATERIAL_H
