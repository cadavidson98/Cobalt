#ifndef CBLT_GEOM_INTERSECTION_H
#define CBLT_GEOM_INTERSECTION_H

#include "simd/simd_vec3.h"
#include "vec2.h"

namespace cblt::geom {

struct CoAxisAlignedBoundingBox;
struct CoRay;
struct CoSphere;

struct IntersectionEvent {
        float timeMin = 0.f;
        float timeMax = 0.f;
        vec2f localCoordinates = {0.f, 0.f};
};

bool raySphereIntersection(const CoRay &ray, const CoSphere &sphere, IntersectionEvent &intersectionEvent);

bool rayTriangleIntersection(
    const CoRay &ray,
    simd::vec3f position1,
    simd::vec3f position2,
    simd::vec3f position3,
    IntersectionEvent &IntersectionEvent
);

bool rayQuadIntersection(
    const CoRay &ray,
    simd::vec3f position1,
    simd::vec3f position2,
    simd::vec3f position3,
    simd::vec3f position4,
    IntersectionEvent &IntersectionEvent
);

bool rayPatchIntersection(
    const CoRay &ray,
    simd::vec3f position1,
    simd::vec3f position2,
    simd::vec3f position3,
    simd::vec3f position4,
    IntersectionEvent &intersectionEvent
);

bool rayAxisAlignedBoundingBoxIntersection(
    const CoRay &ray,
    const CoAxisAlignedBoundingBox &boundingBox,
    IntersectionEvent &intersectionEvent
);
} // namespace cblt::geom

#endif // CBLT_GEOM_INTERSECTION_H
