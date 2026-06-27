#ifndef COBALT_IO_MITSUBA_H
#define COBALT_IO_MITSUBA_H

#include "math/math_types.h"

#include <memory>
#include <string>
#include <string_view>

namespace cobalt::io::mitsuba {

struct BlackBody {
    float minWavelength;
    float maxWavelength;
    float temperature;
};

struct RGB {
    float r;
    float g;
    float b;
};

class Spectrum {
public:
    enum class Type {
        kNone,
        kRGB,
        kBlackBody,
    };

    Spectrum()
      : _type{Type::kNone} {
    }

    Spectrum(BlackBody &&blackbody)
      : blackbody{blackbody}, _type{Type::kBlackBody} {
    }

    Spectrum(RGB &&rgb)
      : rgb{rgb}, _type{Type::kRGB} {
    }

    operator Type() const {
        return _type;
    };

    union {
        BlackBody blackbody;
        RGB rgb;
    };

private:
    Type _type;
};

struct Texture {
    std::string fileName = "";
    std::string fileExtension = "";

    explicit operator bool() const {
        return fileName.length() > 0 && fileExtension.length() > 0;
    }
};

struct Camera {
    float fov = {};
    mat4f transform = {};
    // TODO: film size?
};

struct Emitter {
    Spectrum radiance;
    Texture emissionMap = {};
};

template<typename ShapeType>
struct Shape {
    ShapeType shape;
    // todo: this isn't correct: needs to be a 'formal' BSDF and 'formal' emitter
    Spectrum spectrum;
    mat4f transform = {};
};

struct Sphere {
    vec3f center;
    float radius;
};

struct Mesh {
    std::string fileName;
    std::string fileExtension;
};

class FileReaderDelegate {
public:
    [[nodiscard]] virtual bool readSphere(const Shape<Sphere> &sphere) = 0;
    [[nodiscard]] virtual bool readMesh(const Shape<Mesh> &mesh) = 0;
    [[nodiscard]] virtual bool readEmitter(const Emitter &emitter) = 0;
    [[nodiscard]] virtual bool readSensor(const Camera &camera) = 0;
};

[[nodiscard]] bool read(const std::string_view fileName, std::shared_ptr<FileReaderDelegate> delegate);

} // namespace cobalt::io::mitsuba

#endif // COBALT_IO_MITSUBA_H
