#ifndef CBLT_COLOR_COLORSPACE_XYZ_H
#define CBLT_COLOR_COLORSPACE_XYZ_H

#include "jakob.h"

#include "math/math_types.h"

namespace cblt::color::xyz {

[[nodiscard]] vec3f convert(const PolynomialSpectrum &spectrum);

[[nodiscard]] vec3f toLinearSRGB(vec3f xyz);

} // namespace cblt::color::xyz

#endif // CBLT_COLOR_COLORSPACE_XYZ_H
