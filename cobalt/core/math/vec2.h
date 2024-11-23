#ifndef CBLT_VEC2_H
#define CBLT_VEC2_H

#include <algorithm>

namespace cblt {

template<typename T>
struct vec2 {
        T x;
        T y;

        vec2(T v = T(0)): x{v}, y{v} {};
        vec2(T _x, T _y): x{_x}, y{_y} {};

        template<typename T>
        friend vec2<T> operator+(vec2<T> lhs, vec2<T> rhs);
        template<typename T>
        friend vec2<T> operator-(vec2<T> lhs, vec2<T> rhs);
        template<typename T>
        friend vec2<T> operator*(vec2<T> lhs, vec2<T> rhs);
        template<typename T>
        friend vec2<T> operator/(vec2<T> lhs, vec2<T> rhs);
        template<typename T>
        friend vec2<T> operator/(vec2<T> lhs, T rhs);
        template<typename T>
        friend vec2<T> operator*(vec2<T> lhs, T rhs);
        template<typename T>
        friend T lengthSqr(vec2<T> lhs);
        template<typename T>
        friend vec2<T> clamp(vec2<T> lhs, T min, T max);
        template<typename T>
        friend vec2<T> abs(vec2<T> lhs);
};

template<typename T>
inline vec2<T> operator+(vec2<T> lhs, vec2<T> rhs) {
    return vec2<T>{lhs.x + rhs.x, lhs.y + rhs.y};
}

template<typename T>
inline vec2<T> operator-(vec2<T> lhs, vec2<T> rhs) {
    return vec2<T>{lhs.x - rhs.x, lhs.y - rhs.y};
}

template<typename T>
inline vec2<T> operator*(vec2<T> lhs, vec2<T> rhs) {
    return vec2<T>{lhs.x * rhs.x, lhs.y * rhs.y};
}

template<typename T>
inline vec2<T> operator/(vec2<T> lhs, vec2<T> rhs) {
    return vec2<T>{lhs.x / rhs.x, lhs.y / rhs.y};
}

template<typename T>
inline vec2<T> operator/(vec2<T> lhs, T rhs) {
    return vec2<T>{lhs.x / rhs, lhs.y / rhs};
}

template<typename T>
inline vec2<T> operator*(vec2<T> lhs, T rhs) {
    return vec2<T>{lhs.x * rhs, lhs.y * rhs};
}

template<typename T>
inline T lengthSqr(vec2<T> lhs) {
    return lhs.x * lhs.x + lhs.y * lhs.y;
}

template<typename T>
inline vec2<T> clamp(vec2<T> lhs, T min, T max) {
    return vec2<T>{
        std::clamp<T>(lhs.x, min, max),
        std::clamp<T>(lhs.y, min, max),
    };
}

template<typename T>
inline vec2<T> abs(vec2<T> lhs) {
    return vec2<T>{
        std::abs(lhs.x),
        std::abs(lhs.y),
    };
}

using vec2i = vec2<int>;
using vec2u = vec2<unsigned int>;
using vec2f = vec2<float>;

} // namespace cblt

#endif // CBLT_VEC2_H
