#include "material.h"

#include "render/data/texture.h"

namespace cblt::render {

/// Material Node

template<typename constantType>
CoMaterialNode<constantType>::CoMaterialNode(constantType constant): _value{constant}, _valueType{Input::kConstant} {
}

template<typename constantType>
constantType CoMaterialNode<constantType>::output() const {
    switch (_valueType) {
    case Input::kTexture :
    case Input::kConstant :
        return _value;
    }
    return constantType();
}

template class CoMaterialNode<CoSpectrum>;
template class CoMaterialNode<float>;

/// Material

namespace {
static const CoMaterial::Properties kDefaultProperties = {
    .baseColor = CoMaterialNode(CoSpectrum(CoColor(1.f, 1.f, 1.f))),
    .metallic = 0.f,
    .subsurface = 0.f,
    .ior = 1.4f,
    .specular = 0.f,
    .specularTint = 0.f,
    .specularTransmission = 0.f,
    .roughness = 0.f,
    .anisotropic = 0.f,
    .sheen = 0.f,
    .sheenTint = 0.f,
    .clearcoat = 0.f,
    .clearcoatGloss = 0.f,
};
};

CoMaterial::CoMaterial(): _parameters{kDefaultProperties} {
}

CoMaterial::CoMaterial(const CoMaterial::Properties &parameters): _parameters{parameters} {
}

CoSurfaceParams CoMaterial::surfaceParamsAtCoordinates() const {

    return CoSurfaceParams{
        .baseColor = _parameters.baseColor.output(),
        .metallic = _parameters.metallic.output(),
        .subsurface = _parameters.subsurface.output(),
        .ior = _parameters.ior.output(),
        .specular = _parameters.specular.output(),
        .specularTint = _parameters.specularTint.output(),
        .specularTransmission = _parameters.specularTransmission.output(),
        .roughness = _parameters.roughness.output(),
        .anisotropic = _parameters.anisotropic.output(),
        .sheen = _parameters.sheen.output(),
        .sheenTint = _parameters.sheenTint.output(),
        .clearcoat = _parameters.clearcoat.output(),
        .clearcoatGloss = _parameters.clearcoatGloss.output(),
    };
}

} // namespace cblt::render
