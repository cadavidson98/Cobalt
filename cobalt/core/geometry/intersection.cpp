#include "intersection.h"

#include "bounding_box.h"
#include "quad.h"
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

bool rayTriangleIntersection(
    const CoRay &ray,
    simd::vec3f position1,
    simd::vec3f position2,
    simd::vec3f position3,
    IntersectionEvent &intersectionEvent
) {
    const simd::vec3f AToB = position2 - position1;
    const simd::vec3f AToC = position3 - position1;

    const simd::vec3f triangleNormal = simd::cross(AToB, AToC);

    const float determinant = simd::dot(ray.dir, triangleNormal);
    if (!(std::fabs(determinant) > std::numeric_limits<float>::epsilon())) {
        return false;
    }

    const float invDeterminant = 1.f / determinant;

    const simd::vec3f rayToTriangle = position1 - ray.pos; // Q
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

// ray quad intersection, from Inigo Quilez
// TODO: link here
bool rayQuadIntersection(const CoRay &ray, const CoQuad &quad, IntersectionEvent &intersectionEvent) {
    static constexpr std::array<int, 4> lookupTable = {1, 2, 0, 1};
    static constexpr float kZero = -1e-4f;
    static constexpr float kOne = 1 + 1e-4;

    const simd::vec3f edge1 = quad.position2 - quad.position1;
    const simd::vec3f edge2 = quad.position4 - quad.position1;
    const simd::vec3f edge3 = quad.position3 - quad.position1;
    const simd::vec3f toQuad = ray.pos - quad.position1;

    simd::vec3f planeNormal = simd::cross(edge1, edge2);
    const float hitTime = -simd::dot(planeNormal, toQuad) / simd::dot(ray.dir, planeNormal);
    if (hitTime < kZero) {
        return false;
    }

    const simd::vec3f hitPosition = toQuad + hitTime * ray.dir;
    // project onto quad plane
    const simd::vec3f absPlane = simd::abs(planeNormal);
    std::array<float, 3> planeValues = {absPlane.x, absPlane.y, absPlane.z};
    const int maxDimension = planeValues[0] > planeValues[1] && planeValues[0] > planeValues[2] ? 0
                             : planeValues[1] > planeValues[2]                                  ? 1
                                                                                                : 2;

    const int uID = lookupTable[maxDimension];
    const int vID = lookupTable[maxDimension + 1];

    // project to 2D
    const simd::vec3f projectedPosition = simd::shuffle(hitPosition, uID, vID, 2);
    const simd::vec3f projectedEdge1 = simd::shuffle(edge1, uID, vID, 2);
    const simd::vec3f projectedEdge2 = simd::shuffle(edge2, uID, vID, 2);
    const simd::vec3f projectedEdge3 = simd::shuffle(edge3, uID, vID, 2);

    // find barycentric coordinates
    const simd::vec3f projectedSum = projectedEdge3 - projectedEdge2 - projectedEdge1;

    auto crossProduct2D = [](const simd::vec3f &lhs, const simd::vec3f &rhs) {
        const simd::vec3f cross3D = simd::cross(lhs, rhs);
        return cross3D.z;
    };

    const float cross0 = crossProduct2D(projectedPosition, projectedEdge2);
    const float cross2 = crossProduct2D(projectedEdge3 - projectedEdge2, projectedEdge1);
    const float cross1 = crossProduct2D(projectedPosition, projectedSum) - planeNormal[maxDimension];

    float u = 0;
    float v = 0;
    if (!(std::fabs(cross2) > std::numeric_limits<float>::epsilon())) {
        u = crossProduct2D(projectedPosition, projectedEdge1) / cross1;
        v = -cross0 / cross1;
    } else {
        const float w = cross1 * cross1 - 4.f * cross0 * cross2;
        if (w < kZero) {
            return false;
        }
        const float sqrtW = std::sqrt(w);
        const float inv2A = 1.f / (2.f * cross2);
        v = (-cross1 - sqrtW) * inv2A;
        if (std::clamp(v, kZero, kOne) != v) {
            v = (-cross1 + sqrtW) * inv2A;
        }

        u = (projectedPosition.y - projectedEdge1.y * v) / (projectedEdge2.y + projectedSum.y * v);
        if (std::clamp(u, kZero, kOne) != u) {
            u = (projectedPosition.x - projectedEdge1.x * v) / (projectedEdge2.x + projectedSum.x * v);
        }
    }

    if (std::clamp(u, kZero, kOne) != u || std::clamp(v, kZero, kOne) != v) {
        return false;
    }

    intersectionEvent.timeMin = hitTime;
    intersectionEvent.localCoordinates = vec2f{u, v};
    return true;
}

bool rayQuadIntersection(
    const CoRay &ray,
    simd::vec3f position1,
    simd::vec3f position2,
    simd::vec3f position3,
    simd::vec3f position4,
    IntersectionEvent &IntersectionEvent
) {
    if (rayTriangleIntersection(ray, position1, position2, position3, IntersectionEvent)) {
        return true;
    }

    return rayTriangleIntersection(ray, position1, position3, position4, IntersectionEvent);
}

bool rayAxisAlignedBoundingBoxIntersection(
    const CoRay &ray,
    const CoAxisAlignedBoundingBox &aabb,
    IntersectionEvent &intersectionEvent
) {
    const simd::vec3f minIntersectTimes = (aabb.min - ray.pos) * ray.invDir;
    const simd::vec3f maxIntersectTimes = (aabb.max - ray.pos) * ray.invDir;

    const simd::vec3f closestTimes = simd::min(minIntersectTimes, maxIntersectTimes);
    const simd::vec3f farthestTimes = simd::max(minIntersectTimes, maxIntersectTimes);

    intersectionEvent.timeMin = simd::reduceMax(closestTimes);
    intersectionEvent.timeMax = simd::reduceMin(farthestTimes);

    if (intersectionEvent.timeMax < intersectionEvent.timeMin) {
        std::swap(intersectionEvent.timeMax, intersectionEvent.timeMin);
    }

    if (intersectionEvent.timeMin <= 0.f) {
        intersectionEvent.timeMin = intersectionEvent.timeMax;
    }
    return intersectionEvent.timeMax >= 0.f && intersectionEvent.timeMin < ray.maxDist;
}

} // namespace cblt::geom
