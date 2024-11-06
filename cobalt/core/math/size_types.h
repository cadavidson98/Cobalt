#ifndef CBLT_MATH_SIZE_TYPES_H
#define CBLT_MATH_SIZE_TYPES_H

#include "vec2.h"

namespace cblt {

struct CoSize {
        float width = 0.f;
        float height = 0.f;
};

struct CoRect {
        vec2f offset;
        CoSize size;
};
}; // namespace cblt

#endif // CBLT_MATH_SIZE_TYPES_H
