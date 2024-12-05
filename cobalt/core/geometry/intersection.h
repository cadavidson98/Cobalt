#ifndef CBLT_GEOM_INTERSECTION_H
#define CBLT_GEOM_INTERSECTION_H

#include "vec2.h"

namespace cblt::geom {

struct CoRay;
struct CoAxisAlignedBoundingBox;
struct CoTriangle;
struct CoSphere;

struct IntersectionEvent {
        float timeMin = 0.f;
        float timeMax = 0.f;
        vec2f localCoordinates = {0.f, 0.f};
};

bool raySphereIntersection(const CoRay &ray, const CoSphere &sphere, IntersectionEvent &intersectionEvent);

bool rayTriangleIntersection(const CoRay &ray, const CoTriangle &triangle, IntersectionEvent &intersectionEvent);

bool rayAxisAlignedBoundingBoxIntersection(
    const CoRay &ray,
    const CoAxisAlignedBoundingBox &boundingBox,
    IntersectionEvent &intersectionEvent
);
} // namespace cblt::geom

#endif // CBLT_GEOM_INTERSECTION_H
