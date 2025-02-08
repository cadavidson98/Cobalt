#ifndef CBLT_RENDER_SCENE_H
#define CBLT_RENDER_SCENE_H

#include "Ptexture.h"

#include "core/dynamic_array.h"
#include "render/data/camera.h"
#include "render/material/material.h"

#include <memory>
#include <optional>
#include <string>

namespace cblt {

namespace geom {
class CoMesh;
class CoRay;
struct IntersectionEvent;
} // namespace geom

namespace render {

class CoTexture;
class CoColor;

using CoUUID = uint32_t;

class CoScene {
public:
    struct CreateFromDataInfo {
        std::shared_ptr<CoCamera> camera;
        std::shared_ptr<geom::CoMesh> mesh;
        std::shared_ptr<CoTexture> environmentMap;
        CoDynamicArray<CoMaterial> materials;
        Ptex::PtexCache *ptexTextures;
    };

    static std::shared_ptr<CoScene> create(CreateFromDataInfo &createInfo);

    std::shared_ptr<CoCamera> camera() const;

    bool closestIntersection(const geom::CoRay &ray, geom::IntersectionEvent &intersectionEvent) const;

    CoSurfaceParams resolveSurfaceAtInteraction(const geom::IntersectionEvent &intersectionEvent) const;
    CoColor environment(const geom::CoRay &ray) const;

    ~CoScene();

private:
    CoScene();

    struct PrimitiveComponents {
        uint32_t materialIdx;
    };

    static constexpr CoUUID kInvalidID = CoUUID(~0);

    std::shared_ptr<CoMaterial> _defaultMaterial;

    std::shared_ptr<CoCamera> _camera;
    std::shared_ptr<CoTexture> _environmentMap;
    Ptex::PtexCache *_ptexTextures;
    std::shared_ptr<geom::CoMesh> _mesh;
    CoDynamicArray<CoMaterial> _materials;
    CoDynamicArray<PrimitiveComponents> _scenePrimitives;
}; // CoScene

} // namespace render
} // namespace cblt

#endif // CBLT_RENDER_SCENE_H
