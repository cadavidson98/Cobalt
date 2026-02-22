#ifndef CBLT_VEC4_H
#define CBLT_VEC4_H

#include <algorithm>
#include <cstdint>

namespace cblt {

template<typename T>
struct vec4;

template<typename T>
constexpr vec4<T> operator+(vec4<T> lhs, vec4<T> rhs);

template<typename T>
constexpr vec4<T> operator-(vec4<T> lhs, vec4<T> rhs);

template<typename T>
constexpr vec4<T> operator*(vec4<T> lhs, vec4<T> rhs);

template<typename T>
constexpr vec4<T> operator/(vec4<T> lhs, vec4<T> rhs);

template<typename T>
constexpr bool operator==(vec4<T> lhs, vec4<T> rhs);

template<typename T>
constexpr vec4<T> operator/(vec4<T> lhs, T rhs);

template<typename T>
constexpr vec4<T> operator*(vec4<T> lhs, T rhs);

template<typename T>
constexpr vec4<T> operator*(T lhs, vec4<T> rhs);

template<typename T>
constexpr vec4<T> clamp(vec4<T> lhs, T min, T max);

template<typename T>
struct vec4 {
    T x;
    T y;
    T z;
    T w;

    constexpr friend vec4<T> operator+ <>(vec4<T> lhs, vec4<T> rhs);
    constexpr friend vec4<T> operator- <>(vec4<T> lhs, vec4<T> rhs);
    constexpr friend vec4<T> operator* <>(vec4<T> lhs, vec4<T> rhs);
    constexpr friend vec4<T> operator/ <>(vec4<T> lhs, vec4<T> rhs);
    constexpr friend bool operator== <>(vec4<T> lhs, vec4<T> rhs);

    constexpr vec4<T> &operator+=(const vec4<T> &rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    };

    constexpr vec4<T> &operator-=(const vec4<T> &rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    };

    constexpr vec4<T> &operator*=(const vec4<T> &rhs) {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        w *= rhs.w;
        return *this;
    };

    constexpr vec4<T> &operator/=(const vec4<T> &rhs) {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        w /= rhs.w;
        return *this;
    };

    constexpr friend vec4<T> operator/ <>(vec4<T> lhs, T rhs);
    constexpr friend vec4<T> operator* <>(vec4<T> lhs, T rhs);
    constexpr friend vec4<T> operator* <>(T lhs, vec4<T> rhs);

    constexpr friend vec4<T> clamp<>(vec4<T> lhs, T min, T max);

    static constexpr vec4<T> fill(T value);
};

template<typename T>
constexpr vec4<T> operator+(vec4<T> lhs, vec4<T> rhs) {
    return vec4<T>{
        lhs.x + rhs.x,
        lhs.y + rhs.y,
        lhs.z + rhs.z,
        lhs.w + rhs.w,
    };
}

template<typename T>
constexpr vec4<T> operator-(vec4<T> lhs, vec4<T> rhs) {
    return vec4<T>{
        lhs.x - rhs.x,
        lhs.y - rhs.y,
        lhs.z - rhs.z,
        lhs.w - rhs.w,
    };
}

template<typename T>
constexpr vec4<T> operator*(vec4<T> lhs, vec4<T> rhs) {
    return vec4<T>{
        lhs.x * rhs.x,
        lhs.y * rhs.y,
        lhs.z * rhs.z,
        lhs.w * rhs.w,
    };
}

template<typename T>
constexpr vec4<T> operator/(vec4<T> lhs, vec4<T> rhs) {
    return vec4<T>{
        lhs.x / rhs.x,
        lhs.y / rhs.y,
        lhs.z / rhs.z,
        lhs.w / rhs.w,
    };
}

template<typename T>
constexpr bool operator==(vec4<T> lhs, vec4<T> rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
}

template<typename T>
constexpr vec4<T> operator/(vec4<T> lhs, T rhs) {
    return vec4<T>{
        lhs.x / rhs,
        lhs.y / rhs,
        lhs.z / rhs,
        lhs.w / rhs,
    };
}

template<typename T>
constexpr vec4<T> operator*(vec4<T> lhs, T rhs) {
    return vec4<T>{
        lhs.x * rhs,
        lhs.y * rhs,
        lhs.z * rhs,
        lhs.w * rhs,
    };
}

template<typename T>
constexpr vec4<T> operator*(T lhs, vec4<T> rhs) {
    return rhs * lhs;
}

template<typename T>
constexpr vec4<T> clamp(vec4<T> lhs, T min, T max) {
    return vec4<T>{
        std::clamp(lhs.x, min, max),
        std::clamp(lhs.y, min, max),
        std::clamp(lhs.z, min, max),
        std::clamp(lhs.w, min, max),
    };
}

template<typename T>
constexpr vec4<T> vec4<T>::fill(T value) {
    return vec4{
        .x = value,
        .y = value,
        .z = value,
        .w = value,
    };
}

using vec4u = vec4<unsigned int>;
using vec4u8 = vec4<uint8_t>;
using vec4f = vec4<float>;

} // namespace cblt

#endif // CBLT_VEC4_H
