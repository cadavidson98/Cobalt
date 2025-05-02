#ifndef CBLT_RENDER_MATERIAL_H
#define CBLT_RENDER_MATERIAL_H

#include "surface_function.h"
#include "surface_params.h"

#include "math/math_types.h"

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
    // CoMaterialNode(std::shared_ptr<class CoTexture> texture);

    constantType output(vec2f uv, uint32_t faceIdx) const;

private:
    constantType _value;
    Input _valueType;
};

class CoMaterial {
public:
    using Properties = CoPrincipledParameters<CoMaterialNode<CoSpectrum>, CoMaterialNode<float>>;

    CoMaterial();
    CoMaterial(const Properties &parameters);
    CoSurfaceParams surfaceParamsAtCoordinates(const vec2f uvCoords, uint32_t faceIdx) const;

    // void sampleMaterialAtCoordinate(const CoRay &ray?, vec2f localCoorindates, uint32_t faceIdx) const;
private:
    Properties _parameters;
};

} // namespace cblt::render
#endif // CBLT_RENDER_MATERIAL_H
