#ifndef COBALT_RENDER_BLACKBODY_SPECTRUM_H
#define COBALT_RENDER_BLACKBODY_SPECTRUM_H

#include "core/size_types.h"
#include <cmath>

namespace cobalt::color {

class BlackBodySpectrum {
public:
    BlackBodySpectrum(float temperature)
      : _temperatureKelvin{temperature} {
        _normalization = _plancsLaw(_temperatureKelvin, kWeinConstant / double(_temperatureKelvin));
    }

    float operator[](size_t wavelength) const {
        const float kNanoMetersToMeters = 1e-9f;
        const float wavelengthMeters = float(wavelength) * kNanoMetersToMeters;
        const float planc = _plancsLaw(_temperatureKelvin, wavelengthMeters);
        return planc / _normalization;
    }

private:
    static constexpr float kWeinConstant = 2.8977721e-3f;
    static constexpr float kPlancConstant = 6.62606957e-34f;
    static constexpr float kBoltzmannConstant = 1.3806488e-23f;
    static constexpr float kLightSpeed = 299792458.f;

    static float _plancsLaw(float temperatureKelvin, float wavelengthMeters) {
        static constexpr float kNumerator = 2.f * kPlancConstant * kLightSpeed * kLightSpeed;
       
        const float scale =
            (kPlancConstant * kLightSpeed) / (kBoltzmannConstant * wavelengthMeters * temperatureKelvin);
        
        const float demoninator = std::powf(wavelengthMeters, 5.f) * std::expm1(scale);
        return kNumerator / demoninator;
    }

    float _temperatureKelvin;
    float _normalization;
};

}  // cobalt::color

#endif  // COBALT_RENDER_BLACKBODY_SPECTRUM_H