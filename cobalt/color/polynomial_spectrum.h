#ifndef CBLT_RENDER_POLYNOMIAL_SPECTRUM_H
#define CBLT_RENDER_POLYNOMIAL_SPECTRUM_H

#include "core/size_types.h"
#include "math/math_types.h"
#include "math/vec4.h"

namespace cblt::color {

struct PolynomialSpectrum {
    static constexpr size_t kCoefficientsCount = 3;

    std::array<float, kCoefficientsCount> coefficients;

    constexpr float operator[](float lambda) const {
        return coefficients[0] * lambda * lambda + coefficients[1] * lambda + coefficients[2];
    };

    constexpr vec4f operator[](vec4f lambdas) const {
        return coefficients[0] * lambdas * lambdas + coefficients[1] * lambdas + vec4f::fill(coefficients[2]);
    }
};

} // namespace cblt::color

#endif // CBLT_RENDER_POLYNOMIAL_SPECTRUM_H
