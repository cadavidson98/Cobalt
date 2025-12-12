#include "color_space_xyz.h"

#include "assets.h"
#include "spectrum.h"

#include "core/logging.h"
#include "math/math_types.h"

#include <array>
#include <fstream>
#include <memory>

namespace cblt::color::xyz {

namespace {

struct Samples {
    Spectrum x;
    Spectrum y;
    Spectrum z;
};

[[nodiscard]] static std::shared_ptr<const Samples> createSamples() {
    static std::shared_ptr<Samples> samples = nullptr;

    if (!samples) {
        std::filesystem::path filePath = color::asset::pathForResource("cie_1931_xyz_samples.bin");

        std::ifstream fin(filePath.c_str(), std::ios::binary);
        if (!fin) {
            CoLogError("Failed to load CIE color space samples");
            return nullptr;
        }

        samples = std::make_shared<Samples>();

        fin.read(reinterpret_cast<char *>(samples.get()), sizeof(Samples));
    }

    return samples;
}

} // anonymous namespace

[[nodiscard]] vec3f convert(const Spectrum &spectrum) {
    const std::shared_ptr<const Samples> samples = createSamples();
    if (!samples) {
        return {};
    }

    static constexpr float kYIntegral = 106.856917101172f;

    vec3f total = {};

    for (size_t wavelength = kSpectrumMinWavelength; wavelength < kSpectrumMaxWavelength; ++wavelength) {
        const float sample = spectrum[wavelength];
        const float blueSample = samples->z[wavelength];
        total.x += sample * samples->x[wavelength];
        total.y += sample * samples->y[wavelength];
        total.z += sample * blueSample;
    }

    return total / kYIntegral;
}

[[nodiscard]] vec3f toLinearSRGB(vec3f xyz) {
    // shamelessly copy & pasted from http://www.brucelindbloom.com/
    static const mat3f kXYZToSRGB = {
        vec3f(3.2404542f, -1.5371385f, -.4985314f),
        vec3f(-.969266f, 1.8760108f, .041556f),
        vec3f(.0556434f, -.2040259f, 1.0572252f),
    };

    return kXYZToSRGB * xyz;
}

} // namespace cblt::color::xyz
