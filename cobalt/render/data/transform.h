#ifndef CBLT_TRANSFORM_H
#define CBLT_TRANSFORM_H

#include "math/quat.h"
#include "math/vec3.h"

namespace cblt::render {

struct CoTransform {
    vec3f translation;
    vec3f scale;
    quatf rotation;
};

} // namespace
  // cblt::render

#endif // CBLT_TRANSFORM_H
