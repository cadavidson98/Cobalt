#include "camera.h"

#include "math/math_types.h"
#include "math/math_utilities.h"

namespace cblt::render {
CoCamera::CoCamera(const CreateFromProjectionInfo &createInfo) {
    const vec4f cameraTranslation = createInfo.cameraToWorld[3];
    _cameraPos = vec3f(cameraTranslation.x, cameraTranslation.y, cameraTranslation.z);
    _filmSize = createInfo.filmSize;

    _viewportToWorld = createInfo.cameraToWorld *
                       utils::perspectiveProjectionInv(.01f, 1000.f, createInfo.hFov, createInfo.vFov);
}

geom::CoRay CoCamera::createRay(vec2f ndcPos) const {
    const vec2 filmPos = ndcPos * _filmSize * 0.5f;

    const vec4f pixelWorldPos = _viewportToWorld * vec4f(filmPos.x, filmPos.y, 0.f, 1.f);
    const vec3f rayOrigin = vec3f(pixelWorldPos.x, pixelWorldPos.y, pixelWorldPos.z) / pixelWorldPos.w;

    const vec3f cameraDir = normalize(rayOrigin - _cameraPos);
    return geom::CoRay(
        simd::vec3f(rayOrigin.x, rayOrigin.y, rayOrigin.z),
        simd::vec3f(cameraDir.x, cameraDir.y, cameraDir.z),
        100.f
    );
}
} // namespace cblt::render
