#ifndef CBLT_RENDER_MATERIAL_H
#define CBLT_RENDER_MATERIAL_H

#include "surface_function.h"
#include "surface_params.h"

#include "math/math_types.h"

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

    constantType output() const;

private:
    constantType _value;
    Input _valueType;
};

class CoMaterial {
public:
    using Properties = CoPrincipledParameters<CoMaterialNode<CoSpectrum>, CoMaterialNode<float>>;

    CoMaterial();
    CoMaterial(const Properties &parameters);
    CoSurfaceParams surfaceParamsAtCoordinates() const;

private:
    Properties _parameters;
};

} // namespace cblt::render
#endif // CBLT_RENDER_MATERIAL_H
