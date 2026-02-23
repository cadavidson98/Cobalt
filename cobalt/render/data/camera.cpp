#include "camera.h"

#include "math/math_types.h"
#include "math/math_utilities.h"

#include <cmath>

namespace cobalt::render {

namespace {

constexpr float A = 0.0072f;
constexpr float B = 538.f;

vec4f visibleWavelengthsPDF(vec4f wavelengths) {
    auto pdf = [](float u) -> float {
        static constexpr float kNormalization = 1.f / 253.81971839f;
        return kNormalization / utils::sqr(std::cosh(A * (u - B)));
    };

    return vec4f{
        .x = pdf(wavelengths.x),
        .y = pdf(wavelengths.y),
        .z = pdf(wavelengths.z),
        .w = pdf(wavelengths.w),
    };
}

vec4f visibleWavelengths(vec4f wavelengths) {
    auto value = [](float u) -> float {
        return B - 138.8889f * std::atanh(.8569f - 1.8275f * u);
    };

    return vec4f{
        .x = value(wavelengths.x),
        .y = value(wavelengths.y),
        .z = value(wavelengths.z),
        .w = value(wavelengths.w),
    };
}

} // anonymous namespace

Camera::Camera(const CreateFromProjectionInfo &createInfo) {
    const vec4f cameraTranslation = createInfo.cameraToWorld[3];
    _cameraPos = vec3f(cameraTranslation.x, cameraTranslation.y, cameraTranslation.z);
    _filmSize = createInfo.filmSize;

    _viewportToWorld = createInfo.cameraToWorld *
                       utils::perspectiveProjectionInv(.01f, 1000.f, createInfo.hFov, createInfo.vFov);
}

Camera::Sample Camera::sampleWavelengths(float uniformValue) const {
    static constexpr float kStepSize = .1f;

    auto nextStep = [uniformValue](float idx) -> float {
        const float value = uniformValue + idx * kStepSize;
        return value > 1.f ? value - 1.f : value;
    };

    const vec4f samples = {
        .x = uniformValue,
        .y = nextStep(1.f),
        .z = nextStep(2.f),
        .w = nextStep(3.f),
    };

    const vec4f wavelengths = visibleWavelengths(samples);

    return Camera::Sample{
        .wavelengths = wavelengths,
        .pdfs = visibleWavelengthsPDF(wavelengths),
    };
}

geom::Ray Camera::createRay(vec2f ndcPos) const {
    const vec2 filmPos = ndcPos * _filmSize * 0.5f;

    const vec4f pixelWorldPos = _viewportToWorld * vec4f(filmPos.x, filmPos.y, 0.f, 1.f);
    const vec3f rayOrigin = vec3f(pixelWorldPos.x, pixelWorldPos.y, pixelWorldPos.z) / pixelWorldPos.w;

    const vec3f cameraDir = normalize(rayOrigin - _cameraPos);
    return geom::Ray(
        simd::vec3f(rayOrigin.x, rayOrigin.y, rayOrigin.z),
        simd::vec3f(cameraDir.x, cameraDir.y, cameraDir.z),
        100.f
    );
}
} // namespace cobalt::render
