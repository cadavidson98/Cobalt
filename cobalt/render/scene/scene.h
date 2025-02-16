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
    struct GeometryComponent {
        std::shared_ptr<geom::CoMesh> mesh;
        mat4f transform;
    };

    struct CreateFromDataInfo {
        std::shared_ptr<CoCamera> camera;
        std::shared_ptr<CoTexture> environmentMap;
        CoDynamicArray<GeometryComponent> meshes;
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

    static constexpr CoUUID kInvalidID = CoUUID(~0);

    struct PrimitiveComponents {
        CoUUID geometryIdx = kInvalidID;
        CoUUID materialIdx = kInvalidID;
    };

    std::shared_ptr<CoMaterial> _defaultMaterial;

    std::shared_ptr<CoCamera> _camera;
    std::shared_ptr<CoTexture> _environmentMap;
    Ptex::PtexCache *_ptexTextures;

    CoDynamicArray<PrimitiveComponents> _scenePrimitives;
    CoDynamicArray<GeometryComponent> _meshes;
    CoDynamicArray<CoMaterial> _materials;

    friend class CoSceneFactory;
}; // CoScene

} // namespace render
} // namespace cblt

#endif // CBLT_RENDER_SCENE_H
