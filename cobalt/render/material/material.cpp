#include "material.h"

#include "render/data/texture.h"

namespace cblt::render {

/// Material Node

template<typename constantType>
CoMaterialNode<constantType>::CoMaterialNode(constantType constant): _value{constant}, _valueType{Input::kConstant} {
}

// template<typename constantType>
// CoMaterialNode<constantType>::CoMaterialNode(std::shared_ptr<class CoTexture> texture)
//     : _value{texture}, _valueType{Input::kTexture} {
// }

template<typename constantType>
constantType CoMaterialNode<constantType>::output(vec2f uv, uint32_t faceIdx) const {
    switch (_valueType) {
    case Input::kTexture :
    case Input::kConstant :
        return _value;
        // std::shared_ptr<CoTexture> texture = std::get<0>(_value);
        // return texture->sample(uv);
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

CoSurfaceParams CoMaterial::surfaceParamsAtCoordinates(const vec2f uvCoords, uint32_t faceIdx) const {

    return CoSurfaceParams{
        .baseColor = _parameters.baseColor.output(uvCoords, faceIdx),
        .metallic = _parameters.metallic.output(uvCoords, faceIdx),
        .subsurface = _parameters.subsurface.output(uvCoords, faceIdx),
        .ior = _parameters.ior.output(uvCoords, faceIdx),
        .specular = _parameters.specular.output(uvCoords, faceIdx),
        .specularTint = _parameters.specularTint.output(uvCoords, faceIdx),
        .specularTransmission = _parameters.specularTransmission.output(uvCoords, faceIdx),
        .roughness = _parameters.roughness.output(uvCoords, faceIdx),
        .anisotropic = _parameters.anisotropic.output(uvCoords, faceIdx),
        .sheen = _parameters.sheen.output(uvCoords, faceIdx),
        .sheenTint = _parameters.sheenTint.output(uvCoords, faceIdx),
        .clearcoat = _parameters.clearcoat.output(uvCoords, faceIdx),
        .clearcoatGloss = _parameters.clearcoatGloss.output(uvCoords, faceIdx),
    };
}

// void CoMaterial::sampleMaterialAtCoordinate(const CoRay &ray?, vec2f localCoorindates, uint32_t faceIdx) const {}

} // namespace cblt::render
