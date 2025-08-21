#include "resolver.h"

#include "color.h"
#include "disney_principled.h"

#include "core/logging.h"

namespace cblt::render {

CoResolver::CoResolver(TextureType textureType): _textureType{textureType} {
}

CoColor CoResolver::resolve(const vec3f &incoming, const MaterialData &materialData) const {
    static constexpr CoColor kEmptyColor = {0.f, 0.f, 0.f, 0.f};

    auto getBaseColor = [type = _textureType]() -> CoColor {
        switch (type) {
        case TextureType::kTexture2D : return kEmptyColor;
        case TextureType::kPTexture : return kEmptyColor;
        default : CoLogError("Unsupported Texture Type"); return kEmptyColor;
        }
    };

    const CoColor baseColor = getBaseColor();

    const CoPrincipledParameters parameters = {
        .baseColor = baseColor,
        .metallic = materialData.scalars.metallic,
        .subsurface = materialData.scalars.subsurface,
        .ior = materialData.scalars.ior,
        .specular = materialData.scalars.specular,
        .specularTint = materialData.scalars.specularTint,
        .specularTransmission = materialData.scalars.specularTransmission,
        .roughness = materialData.scalars.roughness,
        .anisotropic = materialData.scalars.anisotropic,
        .sheen = materialData.scalars.sheen,
        .sheenTint = materialData.scalars.sheenTint,
        .clearcoat = materialData.scalars.clearcoat,
        .clearcoatGloss = materialData.scalars.clearcoatGloss,
    };

    const vec3f normal = {0.f, 1.f, 0.f};
    const vec3f outgoing = reflect(incoming, normal);

    return evaluatePrincipledBSDF(incoming, normal, outgoing, parameters);
}

} // namespace cblt::render
