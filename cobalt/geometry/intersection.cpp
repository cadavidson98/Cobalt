#include "intersection.h"

#include "bounding_box.h"
#include "quad.h"
#include "ray.h"
#include "sphere.h"
#include "triangle.h"

#include "core/size_types.h"
#include "math/simd/simd_vec3.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace cblt::geom {

bool raySphereIntersection(const Ray &ray, const Sphere &sphere, float &timeMin, float &timeMax) {
    simd::vec3f centerToPoint = ray.pos - sphere.center;
    // solve quadratic equation At^2 + Bt + C = 0
    const float A = simd::dot(ray.dir, ray.dir);
    const float B = 2.f * simd::dot(centerToPoint, ray.dir);
    float C = simd::dot(centerToPoint, centerToPoint) - sphere.radius * sphere.radius;

    const float discriminant = B * B - 4.f * A * C;
    if (discriminant < 0.f) {
        return false;
    }

    const float coeff = std::sqrt(discriminant);
    timeMin = (-B + coeff) / (2.f * A);
    timeMax = (-B - coeff) / (2.f * A);
    if (timeMin > timeMax) {
        std::swap(timeMin, timeMax);
    }

    return true;
}

bool rayTriangleIntersection(
    const Ray &ray,
    simd::vec3f position1,
    simd::vec3f position2,
    simd::vec3f position3,
    float &hitTime,
    vec2f &hitCoordinates
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

    hitTime = simd::dot(rayToTriangle, triangleNormal) * invDeterminant;
    hitCoordinates = {u, v};

    return hitTime <= ray.maxDist;
}

bool rayPatchIntersection(
    const Ray &ray,
    simd::vec3f position1,
    simd::vec3f position2,
    simd::vec3f position3,
    simd::vec3f position4,
    float &timeMin,
    float &timeMax,
    vec2f &hitCoordinates
) {
    const simd::vec3f edge21 = position2 - position1;
    const simd::vec3f edge43 = position4 - position3;
    const simd::vec3f edge41 = position4 - position1;
    const simd::vec3f edge32 = position3 - position2;
    const simd::vec3f patchNormal = simd::cross(edge21, edge43);

    const float quadraticA = simd::dot(patchNormal, ray.dir);
    const float quadraticC = simd::dot(simd::cross(position1 - ray.pos, ray.dir), edge41);
    const float quadraticB = simd::dot(simd::cross(position2 - ray.pos, ray.dir), edge32) - (quadraticA + quadraticC);

    float u1 = 0.f;
    float u2 = 0.f;

    if (quadraticA == 0.f) {
        if (quadraticB == 0.f) {
            return false;
        }
        u1 = -quadraticC / quadraticB;
        u2 = u1;
    } else {
        const float discriminant = quadraticB * quadraticB - 4.f * quadraticA * quadraticC;

        if (discriminant < 0.f) {
            return false;
        }

        const float rootDiscriminant = std::sqrt(discriminant);
        const float Q = -.5f * (quadraticB + std::copysign(rootDiscriminant, quadraticB));
        u1 = Q / quadraticA;
        u2 = quadraticC / Q;
    }

    struct PatchValues {
        float time;
        float v;
    };

    auto computePatchValues = [&ray, &position1, &position2, &position3, &position4](float u) -> PatchValues {
        const simd::vec3f Fx = simd::lerp(position1, position2, u);
        const simd::vec3f Fy = simd::lerp(position4, position3, u);
        const simd::vec3f directionV = Fy - Fx;

        const simd::vec3f rayToFx = Fx - ray.pos;
        const simd::vec3f normal = simd::cross(ray.dir, directionV);
        const float normalLengthSquared = simd::dot(normal, normal);

        // use scalar triple product for determinant of 3x3 matrix
        const float v1 = simd::dot(rayToFx, simd::cross(ray.dir, normal)) / normalLengthSquared;
        const float t1 = simd::dot(rayToFx, simd::cross(directionV, normal)) / normalLengthSquared;

        return PatchValues{
            .time = t1,
            .v = v1,
        };
    };

    float hitTimeU = std::numeric_limits<float>::max();

    vec2f localCoordinatesU = {0.f, 0.f};
    const bool u1Valid = 0.f <= u1 && u1 <= 1.f;
    if (u1Valid) {
        const PatchValues patchValues = computePatchValues(u1);
        if (0.f < patchValues.time && 0.f <= patchValues.v && patchValues.v <= 1.f) {
            hitTimeU = patchValues.time;
            localCoordinatesU = {u1, patchValues.v};
        }
    }

    float hitTimeV = std::numeric_limits<float>::max();

    vec2f localCoordinatesV = {0.f, 0.f};
    const bool u2Valid = 0.f <= u2 && u2 <= 1.f;
    if (u2Valid) {
        const PatchValues patchValues = computePatchValues(u2);
        if (0.f < patchValues.time && 0.f <= patchValues.v && patchValues.v <= 1.f) {
            hitTimeV = patchValues.time;
            localCoordinatesV = {u2, patchValues.v};
        }
    }

    timeMin = hitTimeU;
    timeMax = hitTimeV;
    vec2f localCoordinates = localCoordinatesU;

    if (timeMax < timeMin) {
        std::swap(timeMin, timeMax);
        localCoordinates = localCoordinatesV;
    }

    if (timeMin <= ray.maxDist) {
        hitCoordinates = localCoordinates;
        return true;
    }

    return false;
}

bool rayAxisAlignedBoundingBoxIntersection(
    const Ray &ray,
    const AxisAlignedBoundingBox &aabb,
    float &minTime,
    float &maxTime
) {
    const simd::vec3f minIntersectTimes =
        simd::maskNaN((aabb.min - ray.pos) * ray.invDir, std::numeric_limits<float>::lowest());
    const simd::vec3f maxIntersectTimes =
        simd::maskNaN((aabb.max - ray.pos) * ray.invDir, std::numeric_limits<float>::max());

    const simd::vec3f closestTimes = simd::min(minIntersectTimes, maxIntersectTimes);
    const simd::vec3f farthestTimes = simd::max(minIntersectTimes, maxIntersectTimes);

    minTime = simd::reduceMax(closestTimes);
    maxTime = simd::reduceMin(farthestTimes);

    if (maxTime < minTime) {
        return false;
    }

    if (minTime < 0.f) {
        minTime = maxTime;
    }
    return maxTime >= 0.f && minTime <= ray.maxDist;
}

} // namespace cblt::geom
