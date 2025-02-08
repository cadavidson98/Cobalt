#ifndef CBLT_RENDER_CAMERA_H
#define CBLT_RENDER_CAMERA_H

#include "core/size_types.h"
#include "geometry/ray.h"
#include "math/mat4.h"
#include "math/math_utilities.h"
#include "math/vec2.h"
#include "math/vec3.h"

namespace cblt::render {

class CoCamera {
public:
    struct CreateFromProjectionInfo {
        float hFov = cblt::utils::toRadians(35.f);
        float vFov = cblt::utils::toRadians(35.f);
        vec2f filmSize = {2.f, 2.f};
        mat4f cameraToWorld = cblt::utils::translationMatrix({0.f, 0.f, -5.f});
    };

    CoCamera(const CreateFromProjectionInfo &createInfo);

    geom::CoRay createRay(vec2f pixelPos) const;

private:
    mat4f _viewportToWorld;
    vec3f _cameraPos;
    vec2f _filmSize;
};

} // namespace cblt::render

#endif // CBLT_RENDER_CAMERA_H
