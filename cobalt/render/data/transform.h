#ifndef CBLT_RENDER_TRANSFORM_H
#define CBLT_RENDER_TRANSFORM_H

#include "math/math_types.h"

namespace cblt::render {

struct Transform {
    vec3f translation;
    vec3f scale;
    quatf rotation;
};

} // namespace cblt::render

#endif // CBLT_RENDER_TRANSFORM_H
