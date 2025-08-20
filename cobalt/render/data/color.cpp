#include "color.h"

#include <cassert>

namespace cblt::render {

CoSpectrum::CoSpectrum() {
}

CoSpectrum::CoSpectrum(const CoColor &color): _color{color} {
}

float CoSpectrum::reflectance(float wavelengthNM) const {
    assert(false);
    return wavelengthNM;
}

CoColor CoSpectrum::rgbColor() const {
    return _color;
}

} // namespace cblt::render
