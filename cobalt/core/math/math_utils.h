#ifndef CBLT_MATH_UTILS_H
#define CBLT_MATH_UTILS_H

namespace cblt {

template<typename T>
constexpr T sqr(const T value) {
    return value * value;
}

template<typename T>
constexpr T divUp(const T lhs, T rhs) {
    return (lhs + T(1)) / rhs;
}

} // namespace cblt

#endif // CBLT_MATH_UTILS_H
