#ifndef CBLT_RENDER_MITSUBA_BSDF_H
#define CBLT_RENDER_MITSUBA_BSDF_H

#include "mitsuba_utilities.h"

namespace cblt::render::utils {

/// Mitsuba materials
class MitsubaDiffuse final : public MitsubaBSDF {
public:
    MitsubaDiffuse(CoSpectrum reflectance);
    virtual ~MitsubaDiffuse() = default;

    virtual std::string referenceID() const override;
    virtual Properties properties() const override;

private:
    CoSpectrum _reflectance;
};

struct MitsubaDielectric final : public MitsubaBSDF {
public:
    MitsubaDielectric(
        CoSpectrum specularReflectance,
        CoSpectrum specularTransmission,
        vec2f roughness,
        float interiorIOR,
        float exteriorIOR,
        bool isThin
    );
    virtual ~MitsubaDielectric();

    virtual std::string referenceID() const override;
    virtual Properties properties() const override;

private:
    CoSpectrum _specularReflectance;
    CoSpectrum _specularTransmission;
    // anisotropic roughness coefficients (alpha in distribution)
    vec2f _roughness;
    float _interiorIOR;
    float _exteriorIOR;
    // use thin material approximation
    bool _isThin;
};

struct MitsubaConductor final : public MitsubaBSDF {
public:
    MitsubaConductor(CoSpectrum specularReflectance, vec2f roughness, float IOR);
    virtual ~MitsubaConductor() = default;

    virtual std::string referenceID() const override;
    virtual Properties properties() const override;

private:
    CoSpectrum _specularReflectance;
    // anisotropic roughness coefficients (alpha in distribution)
    vec2f _roughness;
    // TODO: complex values for conductor ior
    float _IOR;
};

struct MitsubaPlastic final : public MitsubaBSDF {
public:
    MitsubaPlastic(
        CoSpectrum diffuseReflectance,
        CoSpectrum specularReflectance,
        vec2f roughness,
        float interiorIOR,
        float exteriorIOR
    );
    virtual ~MitsubaPlastic() = default;

    virtual std::string referenceID() const override;
    virtual Properties properties() const override;

private:
    CoSpectrum _diffuseReflectance;
    CoSpectrum _specularReflectance;
    // anisotropic roughness coefficients (alpha in distribution)
    vec2f _roughness;
    // TODO: need to return interior based on the incoming direction
    [[maybe_unused]] float _interiorIOR;
    float _exteriorIOR;
};

} // namespace cblt::render::utils

#endif // CBLT_RENDER_MITSUBA_BSDF_H
