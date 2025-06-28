#ifndef CBLT_RENDER_SCENE_H
#define CBLT_RENDER_SCENE_H

#include "render/data/camera.h"
#include "render/material/material.h"


#include <atomic>
#include <memory>
#include <string>
#include <vector>

namespace cblt {

namespace geom {
class CoMesh;
class CoRay;
struct IntersectionEvent;
} // namespace geom

namespace render {

class CoColor;
struct CoResolver;
class CoTexture;
class CoTextureCache;

using CoUUID = uint32_t;

class CoScene {
public:
    std::shared_ptr<CoCamera> camera() const;

    bool closestIntersection(const geom::CoRay &ray, geom::IntersectionEvent &intersectionEvent) const;

    CoColor environment(const geom::CoRay &ray) const;

    ~CoScene();

private:
    struct GeometryComponent {
        std::shared_ptr<geom::CoMesh> mesh;
        mat4f transform;
    };

    struct MaterialComponent {
        std::shared_ptr<CoMaterial> surface;
        std::shared_ptr<CoResolver> resolver;
    };

    struct Primitive {
        CoUUID geometryIdx = kInvalidID;
        CoUUID materialIdx = kInvalidID;
    };

    struct CreateOptions {
        std::string baseDirectory = {};
    };

    struct CreateInfo {
        std::shared_ptr<CoCamera> camera;
        std::shared_ptr<CoTexture> environmentMap;
        std::span<Primitive> primitives;
        std::span<GeometryComponent> meshes;
        std::span<MaterialComponent> materials;
        CreateOptions options = {};
    };

    static std::shared_ptr<CoScene> create(const CreateInfo &createInfo);

    CoScene(const CreateInfo &createInfo);
    CoScene() = delete;

    // TODO: try using '0' as the invalid ID instead of 2^32 - 1; ZII reasons, or bool reasons?
    static constexpr CoUUID kInvalidID = CoUUID(~0);
    std::atomic<CoUUID> _nextGeometryID;
    std::atomic<CoUUID> _nextMaterialID;

    std::shared_ptr<CoCamera> _camera;
    std::shared_ptr<CoTexture> _environmentMap;
    std::unique_ptr<CoTextureCache> _textureCache;

    std::vector<Primitive> _scenePrimitives;
    std::vector<GeometryComponent> _geometries;
    std::vector<MaterialComponent> _materials;

    friend class CoSceneFactory;
    friend class CoSceneFactoryDelegate;
}; // CoScene

} // namespace render
} // namespace cblt

#endif // CBLT_RENDER_SCENE_H
