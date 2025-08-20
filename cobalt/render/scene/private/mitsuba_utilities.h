#ifndef CBLT_RENDER_MITSUBA_UTILS_H
#define CBLT_RENDER_MITSUBA_UTILS_H

#include "surface_params.h"

#include "math/math_types.h"

#include <memory>
#include <string>
#include <variant>

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

template<typename ShapeType>
struct MitsubaShape {
    ShapeType shape;
    mat4f transform = {};
    std::variant<std::shared_ptr<MitsubaBSDF>, std::string> material;
};

struct MitsubaSphere {
    vec3f center;
    float radius;
};

struct MitsubaMesh {
    std::string fileName;
    std::string fileExtension;
};

class MitsubaDelegate {
public:
    [[nodiscard]] virtual bool readSphere(const MitsubaShape<MitsubaSphere> &sphere) = 0;
    [[nodiscard]] virtual bool readMesh(const MitsubaShape<MitsubaMesh> &mesh) = 0;
    [[nodiscard]] virtual bool readBsdf(std::shared_ptr<MitsubaBSDF> bsdf) = 0;
    [[nodiscard]] virtual bool readEmitter(const MitsubaEmitter &emitter) = 0;
    [[nodiscard]] virtual bool readSensor(const MitsubaCamera &camera) = 0;
};

[[nodiscard]] bool readMitsuba(const std::string_view fileName, std::shared_ptr<MitsubaDelegate> delegate);

} // namespace cblt::render::utils

#endif // CBLT_RENDER_MITSUBA_UTILS_H
