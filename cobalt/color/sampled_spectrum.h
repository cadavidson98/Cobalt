#ifndef CBLT_COLOR_SAMPLED_SPECTRUM_H
#define CBLT_COLOR_SAMPLED_SPECTRUM_H

#include "core/size_types.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <span>

namespace cblt::color {

static constexpr size_t kSpectrumMinWavelength = 360;
static constexpr size_t kSpectrumMaxWavelength = 830;

template<size_t interval = 1>
class SampledSpectrum {
public:
    constexpr SampledSpectrum(std::span<const float> values) {
        std::copy_n(values.begin(), kSpectrumWavelengthCount, samples);
    }

    constexpr float operator[](size_t wavelength) const {
        if (wavelength < kSpectrumMinWavelength || wavelength > kSpectrumMaxWavelength) {
            return 0.f;
        }

        if constexpr (interval == 1) {
            return samples[wavelength - kSpectrumMinWavelength];
        }

        const size_t startIdx = (wavelength - kSpectrumMinWavelength) / interval;
        const size_t remainder = (wavelength - kSpectrumMinWavelength) % interval;

        const size_t endIdx = std::min(startIdx + 1, kSpectrumWavelengthCount - 1);

        const float normalizedValue = float(remainder) / float(interval);

        return std::lerp(samples[startIdx], samples[endIdx], normalizedValue);
    }

    static constexpr size_t sampleCount() {
        return kSpectrumWavelengthCount;
    }

private:
    static constexpr size_t kSpectrumWavelengthCount = (kSpectrumMaxWavelength - kSpectrumMinWavelength) / interval + 1;

    float samples[kSpectrumWavelengthCount] = {};
};

} // namespace cblt::color

#endif // CBLT_COLOR_SAMPLED_SPECTRUM_H
