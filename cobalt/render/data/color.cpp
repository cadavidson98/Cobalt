#include "color.h"

namespace cblt::render {

CoSpectrum::CoSpectrum() {
}

CoSpectrum::CoSpectrum(const CoColor &color): _color{color} {
}

float CoSpectrum::reflectance(float wavelengthNM) const {
    return 0.f;
}

CoColor CoSpectrum::rgbColor() const {
    return _color;
}

} // namespace cblt::render
