#ifndef COBALT_COLOR_XYZ_H
#define COBALT_COLOR_XYZ_H

#include "math/math_types.h"

namespace cobalt::color::xyz {

struct Tristimulus {
    float x;
    float y;
    float z;
};

[[nodiscard]] constexpr vec2f chromaticity(Tristimulus xyz) {
    return {
        .x = xyz.x / (xyz.x + xyz.y + xyz.z),
        .y = xyz.y / (xyz.x + xyz.y + xyz.z),
    };
}

// TODO: should this 'also' be a 'static constexpr' factory? would read like this:
// const Tristimulus value = color::xyz::Tristimulus::create(samples, wavelengths, pdfs);
// const Tristimulus value = color::xyz::Tristimulus::create(chromaticity);
// personally, I'm still leaning towards the 'treat structs as C structs' convention
[[nodiscard]] Tristimulus convert(vec4f samples, vec4f wavelengths, vec4f pdfs);

} // namespace cobalt::color::xyz

#endif // COBALT_COLOR_XYZ_H
