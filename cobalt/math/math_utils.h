#ifndef CBLT_MATH_UTILS_H
#define CBLT_MATH_UTILS_H

#include "constants.h"

namespace cblt {

template<typename T>
constexpr T sqr(const T value) {
    return value * value;
}

template<typename T>
constexpr T divUp(const T lhs, T rhs) {
    return (lhs + T(1)) / rhs;
}

template<typename T>
constexpr T toRadians(const T degrees) {
    return degrees * T(kPI) / T(180);
}

template<typename T>
constexpr T toDegrees(const T radians) {
    return radians * T(180) / T(kPI);
}

} // namespace cblt

#endif // CBLT_MATH_UTILS_H
