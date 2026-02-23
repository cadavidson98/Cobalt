#ifndef COBALT_VEC2_H
#define COBALT_VEC2_H

#include <algorithm>

namespace cobalt {

template<typename T>
struct vec2;

template<typename T>
constexpr vec2<T> operator+(vec2<T> lhs, vec2<T> rhs);

template<typename T>
constexpr vec2<T> operator-(vec2<T> lhs, vec2<T> rhs);

template<typename T>
constexpr vec2<T> operator*(vec2<T> lhs, vec2<T> rhs);

template<typename T>
constexpr vec2<T> operator/(vec2<T> lhs, vec2<T> rhs);

template<typename T>
constexpr bool operator==(vec2<T> lhs, vec2<T> rhs);

template<typename T>
constexpr vec2<T> operator/(vec2<T> lhs, T rhs);

template<typename T>
constexpr vec2<T> operator*(vec2<T> lhs, T rhs);

template<typename T>
constexpr T lengthSqr(vec2<T> lhs);

template<typename T>
constexpr vec2<T> clamp(vec2<T> lhs, T min, T max);

template<typename T>
constexpr vec2<T> abs(vec2<T> lhs);

template<typename T>
struct vec2 {
    T x;
    T y;

    constexpr friend vec2<T> operator+ <>(vec2<T> lhs, vec2<T> rhs);
    constexpr friend vec2<T> operator- <>(vec2<T> lhs, vec2<T> rhs);
    constexpr friend vec2<T> operator* <>(vec2<T> lhs, vec2<T> rhs);
    constexpr friend vec2<T> operator/ <>(vec2<T> lhs, vec2<T> rhs);
    constexpr friend bool operator== <>(vec2<T> lhs, vec2<T> rhs);

    constexpr friend vec2<T> operator/ <>(vec2<T> lhs, T rhs);
    constexpr friend vec2<T> operator* <>(vec2<T> lhs, T rhs);
    constexpr friend T lengthSqr<>(vec2<T> lhs);
    constexpr friend vec2<T> clamp<>(vec2<T> lhs, T min, T max);
    constexpr friend vec2<T> abs<>(vec2<T> lhs);
};

template<typename T>
constexpr vec2<T> operator+(vec2<T> lhs, vec2<T> rhs) {
    return vec2<T>{.x = lhs.x + rhs.x, .y = lhs.y + rhs.y};
}

template<typename T>
constexpr vec2<T> operator-(vec2<T> lhs, vec2<T> rhs) {
    return vec2<T>{.x = lhs.x - rhs.x, .y = lhs.y - rhs.y};
}

template<typename T>
constexpr vec2<T> operator*(vec2<T> lhs, vec2<T> rhs) {
    return vec2<T>{lhs.x * rhs.x, lhs.y * rhs.y};
}

template<typename T>
constexpr vec2<T> operator/(vec2<T> lhs, vec2<T> rhs) {
    return vec2<T>{.x = lhs.x / rhs.x, .y = lhs.y / rhs.y};
}

template<typename T>
constexpr bool operator==(vec2<T> lhs, vec2<T> rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

template<typename T>
constexpr vec2<T> operator/(vec2<T> lhs, T rhs) {
    return vec2<T>{.x = lhs.x / rhs, .y = lhs.y / rhs};
}

template<typename T>
constexpr vec2<T> operator*(vec2<T> lhs, T rhs) {
    return vec2<T>{.x = lhs.x * rhs, .y = lhs.y * rhs};
}

template<typename T>
constexpr T lengthSqr(vec2<T> lhs) {
    return lhs.x * lhs.x + lhs.y * lhs.y;
}

template<typename T>
constexpr vec2<T> clamp(vec2<T> lhs, T min, T max) {
    return vec2<T>{
        .x = std::clamp<T>(lhs.x, min, max),
        .y = std::clamp<T>(lhs.y, min, max),
    };
}

template<typename T>
constexpr vec2<T> abs(vec2<T> lhs) {
    return vec2<T>{
        .x = std::abs(lhs.x),
        .y = std::abs(lhs.y),
    };
}

using vec2i = vec2<int>;
using vec2u = vec2<unsigned int>;
using vec2f = vec2<float>;

} // namespace cobalt

#endif // COBALT_VEC2_H
