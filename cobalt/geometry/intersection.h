#ifndef CBLT_GEOM_INTERSECTION_H
#define CBLT_GEOM_INTERSECTION_H

#include "simd/simd_vec3.h"
#include "vec2.h"

namespace cblt::geom {

struct CoAxisAlignedBoundingBox;
struct CoRay;
struct CoSphere;

struct IntersectionEvent {
        float timeMin = std::numeric_limits<float>::max();
        float timeMax = std::numeric_limits<float>::max();
        vec2f localCoordinates = {0.f, 0.f};
        uint32_t primitiveIndex = 0;
};

bool raySphereIntersection(const CoRay &ray, const CoSphere &sphere, IntersectionEvent &intersectionEvent);

bool rayTriangleIntersection(
    const CoRay &ray,
    simd::vec3f position1,
    simd::vec3f position2,
    simd::vec3f position3,
    float &hitTime,
    vec2f &hitCoordinates
);

bool rayPatchIntersection(
    const CoRay &ray,
    simd::vec3f position1,
    simd::vec3f position2,
    simd::vec3f position3,
    simd::vec3f position4,
    float &timeMin,
    float &timeMax,
    vec2f &hitCoordinates
);

bool rayAxisAlignedBoundingBoxIntersection(
    const CoRay &ray,
    const CoAxisAlignedBoundingBox &boundingBox,
    float &minTime,
    float &maxTime
);
} // namespace cblt::geom

#endif // CBLT_GEOM_INTERSECTION_H
