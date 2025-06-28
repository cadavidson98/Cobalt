#include "resolver.h"

#include "color.h"
#include "disney_principled.h"

#include "core/logging.h"
#include "geometry/mesh.h"

namespace cblt::render {

CoResolver::CoResolver(TextureType textureType): _textureType{textureType} {
}

CoColor CoResolver::resolve(
    const vec3f &incoming,
    const geom::CoMesh &mesh,
    const MaterialData &materialData,
    const Interpolant &interpolant
) const {
    static constexpr CoColor kEmptyColor = {0.f, 0.f, 0.f, 0.f};

    const geom::CoMesh::Vertex vertex = mesh.interpolateAttributes({interpolant.faceIdx, interpolant.localCoordinates});

    const vec3f outgoing = reflect(incoming, vertex.normal);

    CoColor baseColor;

    switch (_textureType) {
    case TextureType::kTexture2D :
        if (!mesh.hasAttribute(geom::CoMesh::VertexAttribute::kUV)) {
            CoLogError("Missing UV coordinate attribute on mesh with texture 2D material");
            return kEmptyColor;
        }
        baseColor = materialData.textures.baseColor.sample(vertex.textureCoords);
        break;
    case TextureType::kPTexture :
        baseColor = materialData.textures.baseColor.sample(interpolant.localCoordinates);
        break;
    default : CoLogError("Unsupported Texture Type"); return kEmptyColor;
    }

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

    return evaluatePrincipledBSDF(incoming, outgoing, vertex.normal, parameters);
}

} // namespace cblt::render
