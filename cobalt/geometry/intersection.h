#ifndef CBLT_GEOM_INTERSECTION_H
#define CBLT_GEOM_INTERSECTION_H

#include "core/size_types.h"
#include "math/math_types.h"

namespace cblt::geom {

struct AxisAlignedBoundingBox;
struct Ray;
struct Sphere;

struct IntersectionEvent {
    float timeMin = std::numeric_limits<float>::max();
    float timeMax = std::numeric_limits<float>::max();
    uint32_t geometryIndex = 0;

    vec2f localCoordinates = {0.f, 0.f};
    uint32_t primitiveIndex = 0;
};

bool raySphereIntersection(const Ray &ray, const Sphere &sphere, float &timeMin, float &timeMax);

bool rayTriangleIntersection(
    const Ray &ray,
    simd::vec3f position1,
    simd::vec3f position2,
    simd::vec3f position3,
    float &hitTime,
    vec2f &hitCoordinates
);

bool rayPatchIntersection(
    const Ray &ray,
    simd::vec3f position1,
    simd::vec3f position2,
    simd::vec3f position3,
    simd::vec3f position4,
    float &timeMin,
    float &timeMax,
    vec2f &hitCoordinates
);

bool rayAxisAlignedBoundingBoxIntersection(
    const Ray &ray,
    const AxisAlignedBoundingBox &boundingBox,
    float &minTime,
    float &maxTime
);
} // namespace cblt::geom

#endif // CBLT_GEOM_INTERSECTION_H
