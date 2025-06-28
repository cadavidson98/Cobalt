#include "disney_principled.h"

namespace cblt::render {

CoColor evaluatePrincipledBSDF(vec3f incoming, vec3f normal, vec3f outgoing, const CoPrincipledParameters &parameters) {
    return parameters.baseColor;
}

} // namespace cblt::render
