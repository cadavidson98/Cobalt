#ifndef CBLT_RENDER_RESOLVER_H
#define CBLT_RENDER_RESOLVER_H

#include "material.h"

#include "geometry/intersection.h"

#include <optional>

namespace cblt::geom {

class CoMesh;
struct IntersectionEvent;

} // namespace cblt::geom

namespace cblt::render {

struct CoResolvedSurface {
    CoSurfaceParams surfaceParams = {};
};

struct CoResolver {

    virtual std::optional<CoResolvedSurface>
    resolve(const geom::CoMesh &mesh, const CoMaterial &material, const geom::IntersectionEvent &intersectionEvent)
        const = 0;
};

struct CoProjectionResolver : CoResolver {

    std::optional<CoResolvedSurface>
    resolve(const geom::CoMesh &mesh, const CoMaterial &material, const geom::IntersectionEvent &intersectionEvent)
        const override;
};

struct CoPTextureResolver : CoResolver {

    std::optional<CoResolvedSurface>
    resolve(const geom::CoMesh &mesh, const CoMaterial &material, const geom::IntersectionEvent &intersectionEvent)
        const override;
};

} // namespace cblt::render

#endif // CBLT_RENDER_RESOLVER_H
