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
using MitsubaSlot = std::variant<constantType, MitsubaTexture>;

class MitsubaBSDF {
public:
    using Properties = CoPrincipledParameters<MitsubaSlot<CoSpectrum>, MitsubaSlot<float>>;

    virtual std::string referenceID() const = 0;
    virtual Properties properties() const = 0;
    virtual ~MitsubaBSDF() = default;
};

struct MitsubaCamera {
    float fov = {};
    mat4f transform = {};
    // TODO: film size?
};

struct MitsubaEmitter {
    MitsubaTexture emissionMap = {};
};

struct MitsubaMesh {
    std::string fileName = "";
    std::string fileExtension = "";
    mat4f transform = {};
    std::variant<std::shared_ptr<MitsubaBSDF>, std::string> material;
};

class MitsubaDelegate {
public:
    virtual bool readMesh(const MitsubaMesh &mesh) = 0;
    virtual bool readBsdf(std::shared_ptr<MitsubaBSDF> bsdf) = 0;
    virtual bool readEmitter(const MitsubaEmitter &emitter) = 0;
    virtual bool readSensor(const MitsubaCamera &camera) = 0;
};

bool readMitsuba(const std::string_view fileName, std::shared_ptr<MitsubaDelegate> delegate);

} // namespace cblt::render::utils

#endif // CBLT_RENDER_MITSUBA_UTILS_H
