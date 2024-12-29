#ifndef CBLT_RENDER_SCENE_H
#define CBLT_RENDER_SCENE_H

#include "camera.h"

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

class CoScene {
    public:
        struct CreateFromDataInfo {
                CoCamera camera;
                std::shared_ptr<geom::CoMesh> mesh;
                std::shared_ptr<CoTexture> environmentMap;
        };

        static std::shared_ptr<CoScene> Create(const CreateFromDataInfo &createInfo);

        bool closestIntersection(const geom::CoRay &ray, geom::IntersectionEvent &intersectionEvent) const;

        CoColor environment(const geom::CoRay &ray) const;

        ~CoScene();

    private:
        CoScene();

        CoCamera _camera;
        std::shared_ptr<CoTexture> _environmentMap;
        std::shared_ptr<geom::CoMesh> _mesh;

}; // CoScene

} // namespace render
} // namespace cblt

#endif // CBLT_RENDER_SCENE_H
