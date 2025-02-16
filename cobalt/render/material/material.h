#ifndef CBLT_RENDER_MATERIAL_H
#define CBLT_RENDER_MATERIAL_H

#include "surface_function.h"
#include "surface_params.h"

#include "geometry/interpolation.h"
#include "math/vec2.h"

#include <memory>
#include <variant>

namespace cblt::render {

// tagged union to represent various material arguments, as opposed to using inheritance
template<typename constantType>
class CoMaterialNode {
public:
    enum class Input {
        kConstant,
        kTexture,
    };

    CoMaterialNode(constantType constant);
    CoMaterialNode(std::shared_ptr<class CoTexture> texture);

    constantType output(vec2f uv, uint32_t faceIdx) const;

private:
    std::variant<std::shared_ptr<class CoTexture>, constantType> _value;
    Input _valueType;
};

class CoMaterial {
public:
    using Properties = CoPrincipledParameters<CoMaterialNode<CoSpectrum>, CoMaterialNode<float>>;

    CoMaterial(const Properties &parameters);
    CoSurfaceParams surfaceParamsAtCoordinates(const vec2f uvCoords, uint32_t faceIdx) const;

    CoMaterial(CoMaterial &&other);
    CoMaterial &operator=(CoMaterial &&other);

    // void sampleMaterialAtCoordinate(const CoRay &ray?, vec2f localCoorindates, uint32_t faceIdx) const;
private:
    CoMaterial(CoMaterial &) = delete;
    CoMaterial &operator=(CoMaterial &) = delete;

    Properties _parameters;
};

} // namespace cblt::render
#endif // CBLT_RENDER_MATERIAL_H
