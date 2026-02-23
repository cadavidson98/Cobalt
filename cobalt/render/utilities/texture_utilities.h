#ifndef COBALT_RENDER_TEXTURE_UTILITIES_H
#define COBALT_RENDER_TEXTURE_UTILITIES_H

#include "math/math_types.h"
#include "math/math_utilities.h"

#include <cmath>

namespace cobalt::render {

template<typename T, typename valueType>
static inline valueType tentFilter(vec2<T> position, vec2<T> center, const valueType &value) {
    const vec2<T> distance = position - center;
    const vec2<T> weights = clamp(vec2<T>(T(1), T(1)) - abs(distance), T(0), T(1));
    return value * weights.x * weights.y;
}

} // namespace cobalt::render

#endif // COBALT_RENDER_TEXTURE_UTILITIES_H
