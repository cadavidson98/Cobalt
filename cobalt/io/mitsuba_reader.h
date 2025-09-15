#ifndef CBLT_IO_MITSUBA_READER_H
#define CBLT_IO_MITSUBA_READER_H

#include "math/math_types.h"

#include <memory>
#include <string>
#include <string_view>

namespace cblt::io::mitsuba {

struct Spectrum {
    float red;
    float green;
    float blue;
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
    Texture emissionMap = {};
};

template<typename ShapeType>
struct Shape {
    ShapeType shape;
    mat4f transform = {};
    Spectrum spectrum = {};
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

} // namespace cblt::io::mitsuba

#endif // CBLT_IO_MITSUBA_READER_H
