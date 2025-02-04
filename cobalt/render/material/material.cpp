#include "material.h"

namespace cblt::render {

CoMaterial::CoMaterial(CoMaterialParams params)
: _params{params} {
    if (std::holds_alternative<Ptex::PtexTexture *>(params.baseColor)) {
        Ptex::PtexTexture *texture = std::get<Ptex::PtexTexture *>(params.baseColor);
        const Ptex::PtexFilter::Options filterOptions;
        const Ptex::PtexTexture::Info textureInfo = texture->getInfo();
        _filter = Ptex::PtexFilter::getFilter(texture, filterOptions);
    }
}

CoSurfaceParams CoMaterial::surfaceParamsAtCoordinates(const vec2f uvCoords, uint32_t faceIdx) const {
    vec4f color{0.f, 0.f, 0.f, 1.f};
    if (_filter) {
        _filter->eval(&color.x, 0, 1, faceIdx, uvCoords.x, uvCoords.y, .125f, 0.f, 0.f, .125f);
    }

    return CoSurfaceParams{
        .baseColor = color,
        .metallic = _params.metallic,
        .subsurface = _params.subsurface,
        .ior = _params.ior,
        .specular = _params.specular,
        .specularTint = _params.specularTint,
        .specularTransmission = _params.specularTransmission,
        .roughness = _params.roughness,
        .anisotropic = _params.anisotropic,
        .sheen = _params.sheen,
        .sheenTint = _params.sheenTint,
        .clearcoat = _params.clearcoat,
        .clearcoatGloss = _params.clearcoatGloss,
    };
}

CoMaterial::CoMaterial(CoMaterial&& other)
: _params{other._params},  _filter{other._filter} {
    other._filter = nullptr;
}

CoMaterial &CoMaterial::operator=(CoMaterial&& other) {
    _params = other._params;
    _filter = other._filter;

    other._filter = nullptr;
    return *this;
}

// void CoMaterial::sampleMaterialAtCoordinate(const CoRay &ray?, vec2f localCoorindates, uint32_t faceIdx) const {}

}  // namespace cblt:material