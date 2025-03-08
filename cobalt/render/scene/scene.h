#ifndef CBLT_RENDER_SCENE_H
#define CBLT_RENDER_SCENE_H

#include "Ptexture.h"

#include "core/dynamic_array.h"
#include "render/data/camera.h"
#include "render/material/material.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

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
    std::shared_ptr<CoCamera> camera() const;

    bool closestIntersection(const geom::CoRay &ray, geom::IntersectionEvent &intersectionEvent) const;

    CoSurfaceParams resolveSurfaceAtInteraction(const geom::IntersectionEvent &intersectionEvent) const;
    CoColor environment(const geom::CoRay &ray) const;

    ~CoScene();

private:
    struct PrimitiveComponents {
        CoUUID geometryIdx = kInvalidID;
        CoUUID materialIdx = kInvalidID;
    };

    struct GeometryComponent {
        std::shared_ptr<geom::CoMesh> mesh;
        mat4f transform;
    };

    struct CreateFromDataInfo {
        std::shared_ptr<CoCamera> camera;
        std::shared_ptr<CoTexture> environmentMap;
        std::span<CoMaterial> materials;
        std::span<PrimitiveComponents> primitives;
        std::span<GeometryComponent> meshes;
        Ptex::PtexCache *ptexTextures;
    };

    static CoUUID nextUUID();
    static std::shared_ptr<CoScene> create(const CreateFromDataInfo &createInfo);

    CoScene();

    // TODO: try using '0' as the invalid ID instead of 2^32 - 1; ZII reasons, or bool reasons?
    static constexpr CoUUID kInvalidID = CoUUID(~0);
    static std::atomic<CoUUID> _nextID;

    std::shared_ptr<CoMaterial> _defaultMaterial;

    std::shared_ptr<CoCamera> _camera;
    std::shared_ptr<CoTexture> _environmentMap;
    Ptex::PtexCache *_ptexTextures = nullptr;

    std::vector<PrimitiveComponents> _scenePrimitives;
    std::vector<GeometryComponent> _meshes;
    std::vector<CoMaterial> _materials;

    friend class CoSceneFactory;
    friend class CoSceneFactoryDelegate;
}; // CoScene

} // namespace render
} // namespace cblt

#endif // CBLT_RENDER_SCENE_H
