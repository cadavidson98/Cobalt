#ifndef CBLT_RENDER_MITSUBA_UTILS_H
#define CBLT_RENDER_MITSUBA_UTILS_H

#include "surface_params.h"

#include "math/mat4.h"
#include "math/vec2.h"

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace cblt::render::utils {

struct MitsubaTexture {
    std::string fileName = "";
    std::string fileExtension = "";

    explicit operator bool() const {
        return fileName.length() > 0 && fileExtension.length() > 0;
    }
};

template<typename constantType>
using MitsubaSlot = std::variant<MitsubaTexture, constantType>;

class MitsubaBSDF {
public:
    using Properties = CoPrincipledParameters<MitsubaSlot<CoSpectrum>, MitsubaSlot<float>>;

    virtual Properties properties() const = 0;
    virtual ~MitsubaBSDF() = default;
};

class MitsubaDiffuse final : public MitsubaBSDF {
public:
    MitsubaDiffuse(CoSpectrum reflectance);
    virtual ~MitsubaDiffuse() = default;

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
    virtual ~MitsubaDielectric() = default;

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

    virtual Properties properties() const override;

private:
    CoSpectrum _diffuseReflectance;
    CoSpectrum _specularReflectance;
    // anisotropic roughness coefficients (alpha in distribution)
    vec2f _roughness;
    float _interiorIOR;
    float _exteriorIOR;
};

struct MitsubaCamera {
    float fov = {};
    mat4f transform = {};
    // TODO: film size?
};

struct MitsubaMesh {
    std::string fileName = "";
    std::string fileExtension = "";
    mat4f transform = {};
    std::shared_ptr<MitsubaBSDF> material = nullptr;
};

struct MitsubaScene {
    MitsubaCamera camera;
    std::vector<std::shared_ptr<MitsubaBSDF>> materials;
    std::vector<MitsubaMesh> meshes;
    MitsubaTexture environmentMap;
};

std::optional<MitsubaScene> readMitsuba(const std::string &fileName, const std::string &parentDirectory);

} // namespace cblt::render::utils

#endif // CBLT_RENDER_MITSUBA_UTILS_H
