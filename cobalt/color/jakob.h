#ifndef CBLT_RENDER_JAKOB_H
#define CBLT_RENDER_JAKOB_H

#include "core/size_types.h"

#include <array>

namespace cblt::color {

struct PolynomialSpectrum {
    static constexpr size_t kNumCoefficients = 3;

    std::array<float, kNumCoefficients> coefficients;

    float operator[](float lambda) const {
        return coefficients[0] * lambda * lambda + coefficients[1] * lambda + coefficients[2];
    };
};

} // namespace cblt::color

#endif // CBLT_RENDER_JAKOB_H
