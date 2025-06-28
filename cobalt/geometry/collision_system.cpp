#include "collision_system.h"

#include "bounding_box.h"
#include "intersection.h"
#include "ray.h"

#include <algorithm>

namespace cblt::geom {

// CollisionPrimitiveStorage

size_t CoCollisionSystem::PrimitiveStorage::numPrimitives() const {
    return _sphereCount + _meshCount;
}

CoAxisAlignedBoundingBox CoCollisionSystem::PrimitiveStorage::primitiveBounds(size_t startIdx, size_t endIdx) const {
    CoAxisAlignedBoundingBox bounds = _primitives[startIdx].boundingBox;

    for (size_t idx = startIdx + 1; idx < endIdx; ++idx) {
        CoAxisAlignedBoundingBox::Union(bounds, _primitives[idx].boundingBox);
    }

    return bounds;
}

size_t CoCollisionSystem::PrimitiveStorage::reorder(
    size_t startIdx,
    size_t endIdx,
    std::function<bool(const CoAxisAlignedBoundingBox &)> comparator
) {

    size_t splitIdx = endIdx - 1;
    for (size_t idx = startIdx; idx < splitIdx;) {
        if (comparator(_bounds[idx])) {
            ++idx;
        } else {
            --splitIdx;
            std::swap(_indices[idx], _indices[splitIdx]);
            std::swap(_bounds[idx], _bounds[splitIdx]);
            std::swap(_primitiveIndices[idx], _primitiveIndices[splitIdx]);
        }
    }

    return 0;
}

bool CoCollisionSystem::PrimitiveNode::primitivesIntersect(const CoRay &ray, IntersectionEvent &event) const {
    bool intersects = false;

    for (size_t idx = 0; idx < _sphereCount; ++idx) {
        intersects = intersects || raySphereIntersection(ray, _spheres[idx], event);
    }

    for (size_t idx = 0; idx < _meshCount; ++idx) {
        intersects = intersects || _meshes[idx].intersects(ray, event);
    }

    return intersects;
}

// CoCollisionSystem

CoCollisionSystem::CollisionData CoCollisionSystem::computeCollisions(CollisionInput input) {
    CollisionData collisionData;

    for (const CoRay &ray : input.rays) {
        CollisionEvent collisionEvent;
        IntersectionEvent intersectionEvent;

        for (const Component &component : input.components) {
            if (component.mesh->intersects(ray, intersectionEvent) &&
                intersectionEvent.timeMin < collisionEvent.timeMin) {
                collisionEvent.componentID = component.id;
                collisionEvent.localCoordinates = intersectionEvent.localCoordinates;
                collisionEvent.primitiveID = intersectionEvent.primitiveIndex;
                collisionEvent.timeMin = intersectionEvent.timeMin;
                collisionEvent.timeMax = intersectionEvent.timeMax;
            }
        }

        collisionData.events.push_back(collisionEvent);
    }
}

} // namespace cblt::geom
