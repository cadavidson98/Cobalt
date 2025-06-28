#ifndef CBLT_GEOM_COLLISION_SYSTEM_H
#define CBLT_GEOM_COLLISION_SYSTEM_H

#include "bounding_box.h"
#include "bounding_volume.h"
#include "mesh.h"
#include "ray.h"
#include "sphere.h"

#include "core/size_types.h"
#include "math/simd/simd_mat4.h"

#include <limits>
#include <memory>
#include <span>
#include <vector>

namespace cblt::geom {

class CoCollisionSystem {

public:

    struct Component {
        PrimitiveType type;
        std::shared_ptr<CoMesh> mesh;
        simd::mat4f localToWorld;

        union {
            uint32_t sphereID;
            uint32_t meshID;
        };
    };

    struct CollisionInput {
        std::span<const Component> components;
        std::span<const CoRay> rays;
    };

    struct CollisionEvent {
        float timeMin = std::numeric_limits<float>::max(); // ray time for exit intersection
        float timeMax = std::numeric_limits<float>::max(); // ray time for exit intersection
        vec2f localCoordinates = {0.f, 0.f};               // 2D local coordinates for the hit
        size_t componentID = 0;                            // id for the component hit at this point
        size_t primitiveID = 0;                            // id for the primitive hit in a mesh
    };

    struct CollisionData {
        std::vector<CollisionEvent> events;
    };

    CollisionData computeCollisions(CollisionInput input);

private:
    class PrimitiveStorage : public CoPrimitiveStorage {
    public:


        struct HeapArray {
            Primitive *ptr;
            size_t count;
        };

        size_t numPrimitives() const;
        CoAxisAlignedBoundingBox primitiveBounds(size_t startIdx, size_t endIdx) const;

        size_t
        reorder(size_t startIdx, size_t endIdx, std::function<bool(const CoAxisAlignedBoundingBox &)> comparator);

        // caller assumes ownership of HeadArray
        HeapArray makePrimitives() const;
        void reorder(const HeapArray &primitives);

    private:

        Primitive *_primitives;

        CoSphere *_spheres;
        size_t _sphereCount;

        CoMesh *_meshes;
        size_t _meshCount;
    };

    class PrimitiveNode : public CoBoundingVolumeLeafNode {
    public:
        bool primitivesIntersect(const CoRay &ray, IntersectionEvent &event) const;

    private:
        CoSphere *_spheres;
        size_t _sphereCount;

        CoMesh *_meshes;
        size_t _meshCount;
    };

    CoBoundingVolume<PrimitiveStorage> _accelerator;
};

} // namespace cblt::geom

#endif // CBLT_GEOM_COLLISION_SYSTEM_H
