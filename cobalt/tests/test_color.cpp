#include "color/rgb.h"
#include "color/sampled_spectrum.h"
#include "color/xyz.h"
#include "math/math_utilities.h"
#include "math/vec4.h"

#include <gtest/gtest.h>

#include <algorithm>

namespace cblt::color {

namespace {

static constexpr size_t kSampleStride = 10;

constexpr SampledSpectrum<kSampleStride> makeCobaltSpectrum() {
    // 'Cobalt' color - https://artistpigments.org/brands/golden-heavy-body/b2qc7-cobalt-blue sampled at 10 nm intervals
    constexpr size_t sampleCount = SampledSpectrum<kSampleStride>::sampleCount();

    static_assert(sampleCount == 48);

    constexpr float cobaltSamples[] = {
        0.f,    0.f,    0.f,    0.f,    0.281f, 0.42f,  0.538f, 0.594f, 0.578f, 0.567f, 0.542f, 0.487f,
        0.447f, 0.463f, 0.426f, 0.284f, 0.166f, 0.09f,  0.061f, 0.052f, 0.046f, 0.042f, 0.039f, 0.038f,
        0.039f, 0.04f,  0.042f, 0.048f, 0.06f,  0.096f, 0.193f, 0.338f, 0.531f, 0.714f, 0.798f, 0.f,
        0.f,    0.f,    0.f,    0.f,    0.f,    0.f,    0.f,    0.f,    0.f,    0.f,    0.f,    0.f,
    };

    return SampledSpectrum<kSampleStride>(cobaltSamples);
}

static constexpr float kEpsilon = 1e-5f;

void expectNear(const vec3f &value, const vec3f &expected) {
    EXPECT_NEAR(value.x, expected.x, kEpsilon);
    EXPECT_NEAR(value.y, expected.y, kEpsilon);
    EXPECT_NEAR(value.z, expected.z, kEpsilon);
}

void expectNear(const mat3f &value, const mat3f &expected) {
    expectNear(value.columns[0], expected.columns[0]);
    expectNear(value.columns[1], expected.columns[1]);
    expectNear(value.columns[2], expected.columns[2]);
}

} // anonymous namespace

TEST(CobaltColor, TestSampledSpectrum) {
    {
        // sample every 1 nm value in the array
        static constexpr float kConstant = 2.f;
        static constexpr size_t kDenseSampleCount = SampledSpectrum<>::sampleCount();
        float denseConstants[kDenseSampleCount] = {};
        std::fill_n(denseConstants, kDenseSampleCount, kConstant);

        const SampledSpectrum<> constantSpectrum(denseConstants);
        for (size_t wavelength = kSpectrumMinWavelength; wavelength <= kSpectrumMaxWavelength; ++wavelength) {
            EXPECT_EQ(constantSpectrum[wavelength], kConstant);
        }
    }
    {
        // sample every 10 nm value in the array
        static constexpr float kConstant = 2.f;
        static constexpr size_t kSampleCount = SampledSpectrum<10>::sampleCount();
        float denseConstants[kSampleCount] = {};
        std::fill_n(denseConstants, kSampleCount, kConstant);

        const SampledSpectrum<10> constantSpectrum(denseConstants);
        for (size_t wavelength = kSpectrumMinWavelength; wavelength <= kSpectrumMaxWavelength; ++wavelength) {
            EXPECT_EQ(constantSpectrum[wavelength], kConstant);
        }
    }
    {
        // sample every 1 nm value in the array

        float value = 0.f;
        auto incrementor = [&value]() -> float {
            const float previous = value;
            value += 1;
            return previous;
        };

        static constexpr size_t kDenseSampleCount = SampledSpectrum<>::sampleCount();
        float denseConstants[kDenseSampleCount] = {};
        std::generate_n(denseConstants, kDenseSampleCount, incrementor);

        const SampledSpectrum<> denseIncrementalSpectrum(denseConstants);
        float expected = 0.f;
        for (size_t wavelength = kSpectrumMinWavelength; wavelength <= kSpectrumMaxWavelength; ++wavelength) {
            EXPECT_EQ(denseIncrementalSpectrum[wavelength], expected++);
        }
    }
    {
        // sample every 10 nm in the array

        float value = 0.f;
        auto incrementor = [&value]() -> float {
            const float previous = value;
            value += 10.f;
            return previous;
        };

        static constexpr size_t kDenseSampleCount = SampledSpectrum<10>::sampleCount();
        float denseConstants[kDenseSampleCount] = {};
        std::generate_n(denseConstants, kDenseSampleCount, incrementor);

        const SampledSpectrum<10> denseStrideSpectrum(denseConstants);
        float expected = 0.f;
        for (size_t wavelength = kSpectrumMinWavelength; wavelength <= kSpectrumMaxWavelength; ++wavelength) {
            EXPECT_NEAR(denseStrideSpectrum[wavelength], expected++, kEpsilon);
        }
    }
}

TEST(CobaltColor, TestConvertXYZMonteCarlo) {
    auto makeWavelength = [](float t) -> float {
        constexpr float minWavelength = float(kSpectrumMinWavelength);
        constexpr float maxWavelength = float(kSpectrumMaxWavelength);
        return std::round(std::lerp(minWavelength, maxWavelength, t));
    };

    static constexpr float kUniformPDF = 1.f / float(SampledSpectrum<>::sampleCount());

    {
        // uniformly sample a constant spectrum
        static constexpr vec4f kConstantSamples = vec4f::fill(.5f);
        static constexpr vec4f kUniformPDFs = vec4f::fill(kUniformPDF);

        const vec4f wavelengths = {
            .x = makeWavelength(0.f),
            .y = makeWavelength(.25f),
            .z = makeWavelength(.75f),
            .w = makeWavelength(1.f),
        };

        [[maybe_unused]] const xyz::Tristimulus value = xyz::convert(kConstantSamples, wavelengths, kUniformPDFs);
    }
    {
        // uniformly sample 'zero' spectrum
        static constexpr vec4f kZeroSamples = vec4f::fill(0.f);
        static constexpr vec4f kUniformPDFs = vec4f::fill(kUniformPDF);

        const vec4f wavelengths = {
            .x = makeWavelength(0.f),
            .y = makeWavelength(.33f),
            .z = makeWavelength(.66f),
            .w = makeWavelength(1.f),
        };

        const xyz::Tristimulus value = xyz::convert(kZeroSamples, wavelengths, kUniformPDFs);

        EXPECT_NEAR(value.x, 0.f, kEpsilon);
        EXPECT_NEAR(value.y, 0.f, kEpsilon);
        EXPECT_NEAR(value.z, 0.f, kEpsilon);
    }
    {
        static constexpr SampledSpectrum<10> kCobaltSpectrum = makeCobaltSpectrum();
        static constexpr vec4f kPDFs = vec4f::fill(1.f / float(kCobaltSpectrum.sampleCount()));

        static constexpr vec4f kWavelengths = {
            .x = 400.f,
            .y = 500.f,
            .z = 600.f,
            .w = 700.f,
        };

        static constexpr vec4f kSamples = {
            .x = kCobaltSpectrum[400],
            .y = kCobaltSpectrum[500],
            .z = kCobaltSpectrum[600],
            .w = kCobaltSpectrum[700],
        };

        [[maybe_unused]] const xyz::Tristimulus value = xyz::convert(kSamples, kWavelengths, kPDFs);
    }
}

TEST(CobaltColor, TestConvertXYZToRGB) {
    {
        static constexpr mat3f kXYZToRGB = {
            {.x = +3.24096994f, .y = -0.96924364f, .z = +0.05563008f},
            {.x = -1.53738318f,  .y = +1.8759675f, .z = -0.20397696f},
            {.x = -0.49861076f, .y = +0.04155506f, .z = +1.05697151f},
        };

        constexpr mat3f xyzToRGB = rgb::convertFromXYZ(color::rgb::Colorspace::kSRGB);

        expectNear(xyzToRGB, kXYZToRGB);
    }
    {
        static constexpr mat3f kXYZToRGB = {
            {.x = +2.72539403f, .y = -0.79516803f, .z = +0.04124189f},
            {.x = -1.01800301f, .y = +1.68973205f, .z = -0.08763902f},
            {.x = -0.44016320f, .y = +0.02264719f, .z = +1.10092938f}
        };

        constexpr mat3f xyzToRGB = rgb::convertFromXYZ(color::rgb::Colorspace::kDCIP3);

        expectNear(xyzToRGB, kXYZToRGB);
    }
}

} // namespace cblt::color
