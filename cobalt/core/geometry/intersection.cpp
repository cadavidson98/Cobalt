#include "intersection.h"

#include "ray.h"
#include "sphere.h"

#include <cmath>

namespace cblt::geom {

bool raySphereIntersection(const CoRay &ray, const CoSphere &sphere, float &tMin, float &tMax) {
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
    tMin = (-B + coeff) / (2.f * A);
    tMax = (-B - coeff) / (2.f * A);
    if (tMax < tMin) {
        std::swap(tMin, tMax);
    }

    return true;
}

} // namespace cblt::geom
