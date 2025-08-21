#include "mitsuba_bsdf.h"

namespace cblt::render::utils {
MitsubaDiffuse::MitsubaDiffuse(CoSpectrum reflectance): _reflectance{reflectance} {
}

std::string MitsubaDiffuse::referenceID() const {
    return "";
}

MitsubaBSDF::Properties MitsubaDiffuse::properties() const {
    return MitsubaBSDF::Properties{
        .baseColor = _reflectance,
        .metallic = 0.f,
        .subsurface = 0.f,
        .ior = 0.f,
        .specular = 0.f,
        .specularTint = 0.f,
        .specularTransmission = 0.f,
        .roughness = 0.f,
        .anisotropic = 0.f,
        .sheen = 0.f,
        .sheenTint = 0.f,
        .clearcoat = 0.f,
        .clearcoatGloss = 0.f,
    };
}

MitsubaDielectric::MitsubaDielectric(
    CoSpectrum specularReflectance,
    CoSpectrum specularTransmission,
    vec2f roughness,
    float interiorIOR,
    float exteriorIOR,
    bool isThin
)
    : _specularReflectance{specularReflectance}, _specularTransmission{specularTransmission}, _roughness{roughness},
      _interiorIOR{interiorIOR}, _exteriorIOR{exteriorIOR}, _isThin{isThin} {
}

MitsubaDielectric::~MitsubaDielectric() {
}

std::string MitsubaDielectric::referenceID() const {
    return "";
}

MitsubaBSDF::Properties MitsubaDielectric::properties() const {
    return MitsubaBSDF::Properties{
        .baseColor = _specularReflectance,
        .metallic = 0.f,
        .subsurface = 0.f,
        .ior = _isThin ? _exteriorIOR : _interiorIOR,
        .specular = 0.f,
        .specularTint = 0.f,
        .specularTransmission = 0.f,
        .roughness = _roughness.x,
        .anisotropic = 0.f,
        .sheen = 0.f,
        .sheenTint = 0.f,
        .clearcoat = 0.f,
        .clearcoatGloss = 0.f,
    };
}

MitsubaConductor::MitsubaConductor(CoSpectrum specularReflectance, vec2f roughness, float IOR)
    : _specularReflectance{specularReflectance}, _roughness{roughness}, _IOR{IOR} {
}

std::string MitsubaConductor::referenceID() const {
    return "";
}

MitsubaBSDF::Properties MitsubaConductor::properties() const {
    return MitsubaBSDF::Properties{
        .baseColor = _specularReflectance,
        .metallic = 0.f,
        .subsurface = 0.f,
        .ior = _IOR,
        .specular = 0.f,
        .specularTint = 0.f,
        .specularTransmission = 0.f,
        .roughness = _roughness.x,
        .anisotropic = 0.f,
        .sheen = 0.f,
        .sheenTint = 0.f,
        .clearcoat = 0.f,
        .clearcoatGloss = 0.f,
    };
}

MitsubaPlastic::MitsubaPlastic(
    CoSpectrum diffuseReflectance,
    CoSpectrum specularReflectance,
    vec2f roughness,
    float interiorIOR,
    float exteriorIOR
)
    : _diffuseReflectance{diffuseReflectance}, _specularReflectance{specularReflectance}, _roughness{roughness},
      _interiorIOR{interiorIOR}, _exteriorIOR{exteriorIOR} {
}

std::string MitsubaPlastic::referenceID() const {
    return "";
}

MitsubaBSDF::Properties MitsubaPlastic::properties() const {
    return MitsubaBSDF::Properties{
        .baseColor = _specularReflectance,
        .metallic = 0.f,
        .subsurface = 0.f,
        .ior = _exteriorIOR,
        .specular = 0.f,
        .specularTint = 0.f,
        .specularTransmission = 0.f,
        .roughness = _roughness.x,
        .anisotropic = 0.f,
        .sheen = 0.f,
        .sheenTint = 0.f,
        .clearcoat = 0.f,
        .clearcoatGloss = 0.f,
    };
}
} // namespace cblt::render::utils
