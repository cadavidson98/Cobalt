#include "disney_principled.h"

namespace cblt::render {

Color evaluatePrincipledBSDF(vec3f incoming, vec3f normal, vec3f outgoing, const PrincipledParameters &parameters) {
    const vec3f halfway = normalize(incoming + outgoing);
    [[maybe_unused]] const float nDotH = dot(normal, halfway);
    [[maybe_unused]] const float nDotI = dot(normal, incoming);
    [[maybe_unused]] const float nDotO = dot(normal, outgoing);

    return parameters.baseColor;
}

} // namespace cblt::render
