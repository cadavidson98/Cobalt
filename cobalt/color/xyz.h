#ifndef CBLT_COLOR_XYZ_H
#define CBLT_COLOR_XYZ_H

#include "math/math_types.h"

namespace cblt::color::xyz {

struct Tristimulus {
    float x;
    float y;
    float z;
};

[[nodiscard]] Tristimulus convert(vec4f samples, vec4f wavelengths, vec4f pdfs);

} // namespace cblt::color::xyz

#endif // CBLT_COLOR_XYZ_H
