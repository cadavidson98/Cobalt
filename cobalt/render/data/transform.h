#ifndef COBALT_RENDER_TRANSFORM_H
#define COBALT_RENDER_TRANSFORM_H

#include "math/math_types.h"

namespace cobalt::render {

struct Transform {
    vec3f translation;
    vec3f scale;
    quatf rotation;
};

} // namespace cobalt::render

#endif // COBALT_RENDER_TRANSFORM_H
