#ifndef CBLT_VEC4_H
#define CBLT_VEC4_H

#include <cstdint>

namespace cblt {

template<typename T>
struct vec4 {
        T x;
        T y;
        T z;
        T w;

        template<typename T>
        friend vec4<T> operator+(vec4<T> lhs, vec4<T> rhs);
        template<typename T>
        friend vec4<T> operator-(vec4<T> lhs, vec4<T> rhs);
        template<typename T>
        friend vec4<T> operator*(vec4<T> lhs, vec4<T> rhs);
        template<typename T>
        friend vec4<T> operator/(vec4<T> lhs, vec4<T> rhs);

        template<typename T>
        vec4<T> &operator+=(const vec4<T> &rhs) {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            w += rhs.w;
            return *this;
        };

        template<typename T>
        vec4<T> &operator-=(const vec4<T> &rhs) {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            w -= rhs.w;
            return *this;
        };

        template<typename T>
        vec4<T> &operator*=(const vec4<T> &rhs) {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            w *= rhs.w;
            return *this;
        };

        template<typename T>
        vec4<T> &operator/=(const vec4<T> &rhs) {
            x /= rhs.x;
            y /= rhs.y;
            z /= rhs.z;
            w /= rhs.w;
            return *this;
        };

        template<typename T>
        friend vec4<T> operator/(vec4<T> lhs, T rhs);
        template<typename T>
        friend vec4<T> operator*(vec4<T> lhs, T rhs);
};

template<typename T>
inline vec4<T> operator+(vec4<T> lhs, vec4<T> rhs) {
    return vec4<T>{lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w};
}

template<typename T>
inline vec4<T> operator-(vec4<T> lhs, vec4<T> rhs) {
    return vec4<T>{lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w};
}

template<typename T>
inline vec4<T> operator*(vec4<T> lhs, vec4<T> rhs) {
    return vec4<T>{lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w};
}

template<typename T>
inline vec4<T> operator/(vec4<T> lhs, vec4<T> rhs) {
    return vec4<T>{lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w};
}

template<typename T>
inline vec4<T> operator/(vec4<T> lhs, T rhs) {
    return vec4<T>{lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs};
}

template<typename T>
inline vec4<T> operator*(vec4<T> lhs, T rhs) {
    return vec4<T>{lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs};
}

using vec4u = vec4<unsigned int>;
using vec4u8 = vec4<uint8_t>;
using vec4f = vec4<float>;

} // namespace cblt

#endif // CBLT_VEC4_H
