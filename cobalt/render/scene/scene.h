#ifndef CBLT_RENDER_SCENE_H
#define CBLT_RENDER_SCENE_H

#include "camera.h"
#include "material.h"
#include "dynamic_array.h"

#include "Ptexture.h"

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
                CoCamera camera;
                std::shared_ptr<geom::CoMesh> mesh;
                std::shared_ptr<CoTexture> environmentMap;
        };

        static std::shared_ptr<CoScene> Create(const CreateFromDataInfo &createInfo);

        bool closestIntersection(const geom::CoRay &ray, geom::IntersectionEvent &intersectionEvent) const;

        const CoMaterial *materialForPrimitive(CoUUID primitiveID) const;
        CoColor environment(const geom::CoRay &ray) const;

        ~CoScene();

    private:
        CoScene();

        struct PrimitiveComponents{
            uint32_t materialIdx;
        };

        CoCamera _camera;
        std::shared_ptr<CoTexture> _environmentMap;
        Ptex::PtexCache *_ptexTextures;
        DynamicArray<geom::CoMesh> _meshs;
        DynamicArray<CoMaterial> _materials;
        DynamicArray<PrimitiveComponents> _scenePrimitives;
}; // CoScene

} // namespace render
} // namespace cblt

#endif // CBLT_RENDER_SCENE_H
