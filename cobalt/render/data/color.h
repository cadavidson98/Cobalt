#ifndef CBLT_RENDER_COLOR_H
#define CBLT_RENDER_COLOR_H

#include <span>
#include <vector>

namespace cblt::render {

struct CoColor {
    float r = 0.f;
    float g = 0.f;
    float b = 0.f;
    float a = 1.f;
};

class CoSpectrum {
public:
    struct Sample {
        float wavelengthNM;
        float reflectance;
    };

    CoSpectrum();
    CoSpectrum(const CoColor &color);
    float reflectance(float wavelengthNM) const;
    CoColor rgbColor() const;

private:
    std::vector<Sample> samples;
};

} // namespace cblt::render

#endif // CBLT_RENDER_COLOR_H
