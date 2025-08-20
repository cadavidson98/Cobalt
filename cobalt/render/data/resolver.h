#ifndef CBLT_RENDER_RESOLVER_H
#define CBLT_RENDER_RESOLVER_H

#include "color.h"

#include "math/math_types.h"

namespace cblt::geom {

class CoMesh;

} // namespace cblt::geom

namespace cblt::render {

enum class TextureType {
    kTexture2D,
    kPTexture,
};

class Texture2DView {
public:
    CoColor sample(vec2f textureCoords) const;

private:
    const void *_texture;
};

struct ScalarData {
    CoColor baseColor;
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

struct TextureData {
    Texture2DView baseColor;
};

struct MaterialData {
    ScalarData scalars;
    TextureData textures;
};

class CoResolver {
public:
    CoResolver(TextureType type);

    struct Interpolant {
        vec3f position;
        vec2f localCoordinates;
        size_t faceIdx;
    };

    CoColor resolve(
        const vec3f &incoming,
        const MaterialData &material
    ) const;

private:
    TextureType _textureType;
};

} // namespace cblt::render

#endif // CBLT_RENDER_RESOLVER_H
