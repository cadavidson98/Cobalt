#ifndef CBLT_CAMERA_H
#define CBLT_CAMERA_H

#include "mat4.h"
#include "math_utilities.h"
#include "math_utils.h"
#include "ray.h"
#include "size_types.h"
#include "vec2.h"
#include "vec3.h"

namespace cblt::render {

class CoCamera {
    public:
        struct CreateFromProjectionInfo {
                float hFov = toRadians(35.f);
                float vFov = toRadians(35.f);
                vec2f filmSize = {2.f, 2.f};
                mat4f cameraToWorld = utils::translationMatrix({0.f, 0.f, -5.f});
        };

        CoCamera(const CreateFromProjectionInfo &createInfo);

        geom::CoRay CreateRay(vec2f pixelPos) const;

    private:
        mat4f _viewportToWorld;
        vec3f _cameraPos;
        vec2f _filmSize;
};

} // namespace
  // cblt::render

#endif // CBLT_CAMERA_H
