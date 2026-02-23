#ifndef COBALT_COLOR_XYZ_H
#define COBALT_COLOR_XYZ_H

#include "math/math_types.h"

namespace cobalt::color::xyz {

struct Tristimulus {
    float x;
    float y;
    float z;
};

[[nodiscard]] Tristimulus convert(vec4f samples, vec4f wavelengths, vec4f pdfs);

} // namespace cobalt::color::xyz

#endif // COBALT_COLOR_XYZ_H
