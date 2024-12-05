#include "intersection.h"

#include "bounding_box.h"
#include "ray.h"
#include "sphere.h"
#include "triangle.h"

#include <algorithm>
#include <cmath>

namespace cblt::geom {

bool raySphereIntersection(const CoRay &ray, const CoSphere &sphere, IntersectionEvent &intersectionEvent) {
    simd::vec3f centerToPoint = ray.pos - sphere.center;
    // solve quadratic equation At^2 + Bt + C = 0
    float A = simd::dot(ray.dir, ray.dir);
    float B = 2.f * simd::dot(centerToPoint, ray.dir);
    float C = simd::dot(centerToPoint, centerToPoint) - sphere.radius * sphere.radius;

    float magicNumber = B * B - 4.f * A * C;
    if (magicNumber < 0.f) {
        return false;
    }

    float coeff = std::sqrt(magicNumber);
    intersectionEvent.timeMin = (-B + coeff) / (2.f * A);
    intersectionEvent.timeMax = (-B - coeff) / (2.f * A);
    if (intersectionEvent.timeMin > intersectionEvent.timeMax) {
        std::swap(intersectionEvent.timeMin, intersectionEvent.timeMax);
    }

    return true;
}

bool rayTriangleIntersection(const CoRay &ray, const CoTriangle &triangle, IntersectionEvent &intersectionEvent) {
    const simd::vec3f AToB = triangle.position2 - triangle.position1;
    const simd::vec3f AToC = triangle.position3 - triangle.position1;

    const simd::vec3f triangleNormal = simd::cross(AToB, AToC);

    const float determinant = simd::dot(ray.dir, triangleNormal);
    if (!(std::fabs(determinant) > std::numeric_limits<float>::epsilon())) {
        return false;
    }

    const float invDeterminant = 1.f / determinant;

    const simd::vec3f rayToTriangle = triangle.position1 - ray.pos; // Q
    const simd::vec3f DCrossQ = simd::cross(ray.dir, rayToTriangle);
    const float u = -simd::dot(DCrossQ, AToC) * invDeterminant;
    const float v = simd::dot(DCrossQ, AToB) * invDeterminant;

    if (std::clamp(u, 0.f, 1.f) != u || std::clamp(v, 0.f, 1.f) != v || u + v > 1.f) {
        return false;
    }

    intersectionEvent.timeMin = simd::dot(rayToTriangle, triangleNormal) * invDeterminant;
    intersectionEvent.localCoordinates = {u, v};

    return intersectionEvent.timeMin <= ray.maxDist;
}

bool rayAxisAlignedBoundingBoxIntersection(
    const CoRay &ray,
    const CoAxisAlignedBoundingBox &aabb,
    IntersectionEvent &intersectionEvent
) {
    const simd::vec3f minIntersectTimes = (aabb.min - ray.pos) * ray.invDir;
    const simd::vec3f maxIntersectTimes = (aabb.max - ray.pos) * ray.invDir;

    simd::vec3f closestTimes = simd::min(minIntersectTimes, maxIntersectTimes);
    simd::vec3f farthestTimes = simd::max(minIntersectTimes, maxIntersectTimes);

    intersectionEvent.timeMin = simd::reduceMax(closestTimes);
    intersectionEvent.timeMax = simd::reduceMin(farthestTimes);

    if (intersectionEvent.timeMin < 0.f) {
        intersectionEvent.timeMin = intersectionEvent.timeMax;
    }
    return intersectionEvent.timeMax > 0.f && intersectionEvent.timeMin < ray.maxDist &&
           intersectionEvent.timeMax >= intersectionEvent.timeMin;
}

} // namespace cblt::geom
