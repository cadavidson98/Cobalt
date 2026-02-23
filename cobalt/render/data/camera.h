#ifndef COBALT_RENDER_CAMERA_H
#define COBALT_RENDER_CAMERA_H

#include "core/size_types.h"
#include "geometry/ray.h"
#include "math/math_types.h"
#include "math/math_utilities.h"

namespace cobalt::render {

class Camera {
public:
    struct CreateFromProjectionInfo {
        float hFov = cobalt::utils::toRadians(35.f);
        float vFov = cobalt::utils::toRadians(35.f);
        vec2f filmSize = {2.f, 2.f};
        mat4f cameraToWorld = cobalt::utils::translationMatrix({0.f, 0.f, -5.f});
    };

    Camera(const CreateFromProjectionInfo &createInfo);

    struct Sample {
        vec4f wavelengths;
        vec4f pdfs;
    };

    Sample sampleWavelengths(float uniformValue) const;
    geom::Ray createRay(vec2f pixelPos) const;

private:
    mat4f _viewportToWorld;
    vec3f _cameraPos;
    vec2f _filmSize;
};

} // namespace cobalt::render

#endif // COBALT_RENDER_CAMERA_H
