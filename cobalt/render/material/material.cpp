#include "material.h"

namespace cblt::render {

CoMaterial::CoMaterial(CoSurfaceParams params, Ptex::PtexTexture *texture)
: _params{params}, _texture{texture} {
    if (_texture) {
        const Ptex::PtexFilter::Options filterOptions;
        const Ptex::PtexTexture::Info textureInfo = _texture->getInfo();
        _filter = Ptex::PtexFilter::getFilter(_texture, filterOptions);
    }
}

CoSurfaceParams CoMaterial::surfaceParamsAtCoordinates(const vec2f uvCoords, uint32_t faceIdx) const {
    CoSurfaceParams faceParams = _params;
    if (_filter) {
        vec4f color{0.f, 0.f, 0.f, 1.f};
        _filter->eval(&color.x, 0, 1, faceIdx, uvCoords.x, uvCoords.y, .125f, 0.f, 0.f, .125f);
        faceParams.baseColor = vec4f{color.x, color.x, color.x, 1.f};
    }
    return faceParams;
}

CoMaterial::CoMaterial(CoMaterial&& other)
: _params{other._params}, _texture{other._texture}, _filter{other._filter} {
    other._texture = nullptr;
    other._filter = nullptr;
}

CoMaterial &CoMaterial::operator=(CoMaterial&& other) {
    _params = other._params;
    _texture = other._texture;
    _filter = other._filter;

    other._texture = nullptr;
    other._filter = nullptr;
    return *this;
}

// void CoMaterial::sampleMaterialAtCoordinate(const CoRay &ray?, vec2f localCoorindates, uint32_t faceIdx) const {}

}  // namespace cblt:material