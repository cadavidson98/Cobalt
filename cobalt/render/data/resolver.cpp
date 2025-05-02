#include "resolver.h"

#include "material.h"

#include "geometry/mesh.h"

namespace cblt::render {

std::optional<CoResolvedSurface> CoProjectionResolver::resolve(
    const geom::CoMesh &mesh,
    const CoMaterial &material,
    const geom::IntersectionEvent &intersectionEvent
) const {
    if (!mesh.hasAttribute(geom::CoMesh::VertexAttribute::kUV)) {
        return std::nullopt;
    }

    const geom::CoMesh::Vertex vertex = mesh.resolveSurface(intersectionEvent);

    const CoSurfaceParams surfaceParams =
        material.surfaceParamsAtCoordinates(vertex.textureCoords, intersectionEvent.geometryIndex);
    return CoResolvedSurface{
        .surfaceParams = surfaceParams,
    };
}

std::optional<CoResolvedSurface> CoPTextureResolver::resolve(
    const geom::CoMesh &mesh,
    const CoMaterial &material,
    const geom::IntersectionEvent &intersectionEvent
) const {

    const CoSurfaceParams surfaceParams =
        material.surfaceParamsAtCoordinates(intersectionEvent.localCoordinates, intersectionEvent.geometryIndex);
    return CoResolvedSurface{
        .surfaceParams = surfaceParams,
    };
}

} // namespace cblt::render
