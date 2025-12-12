#ifndef CBLT_COLOR_SPECTRUM_H
#define CBLT_COLOR_SPECTRUM_H

#include "core/size_types.h"

#include <array>
#include <cassert>

namespace cblt::color {

static constexpr size_t kSpectrumMinWavelength = 360;
static constexpr size_t kSpectrumMaxWavelength = 830;
static constexpr size_t kSpectrumSampleCount = kSpectrumMaxWavelength - kSpectrumMinWavelength + 1;

struct Spectrum {
    std::array<float, kSpectrumSampleCount> samples;

    [[nodiscard]] float operator[](const size_t wavelength) const {
        assert(kSpectrumMinWavelength <= wavelength && wavelength <= kSpectrumMaxWavelength);
        const size_t normalizedIdx = wavelength - kSpectrumMinWavelength;
        return samples[normalizedIdx];
    };
};

} // namespace cblt::color

#endif // CBLT_COLOR_SPECTRUM_H
