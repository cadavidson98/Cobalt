#ifndef COBALT_VEC3_H
#define COBALT_VEC3_H

#include <cmath>
#include <cstdint>

namespace cobalt {

template<typename T>
struct vec3;

template<typename T>
constexpr vec3<T> operator+(const vec3<T> &lhs, const vec3<T> &rhs);

template<typename T>
constexpr vec3<T> operator-(const vec3<T> &lhs, const vec3<T> &rhs);

template<typename T>
constexpr vec3<T> operator*(T lhs, const vec3<T> &rhs);

template<typename T>
constexpr vec3<T> operator/(const vec3<T> &lhs, T rhs);

template<typename T>
constexpr bool operator==(vec3<T> lhs, vec3<T> rhs);

// TODO: SFINAE or concepts here?

template<typename T>
constexpr T dot(const vec3<T> &lhs, const vec3<T> &rhs);

template<typename T>
constexpr T absDot(const vec3<T> &lhs, const vec3<T> &rhs);

template<typename T>
constexpr T length(const vec3<T> &vector);

template<typename T>
constexpr vec3<T> normalize(const vec3<T> &vector);

// project A onto B
template<typename T>
constexpr vec3<T> projection(const vec3<T> &A, const vec3<T> &B);

template<typename T>
constexpr vec3<T> reflect(const vec3<T> &direction, const vec3<T> &normal);

template<typename T>
struct vec3 {
    T x;
    T y;
    T z;

    constexpr friend vec3<T> operator+ <>(const vec3<T> &lhs, const vec3<T> &rhs);
    constexpr friend vec3<T> operator- <>(const vec3<T> &lhs, const vec3<T> &rhs);
    constexpr friend vec3<T> operator* <>(T lhs, const vec3<T> &rhs);
    constexpr friend vec3<T> operator/ <>(const vec3<T> &lhs, T rhs);
    constexpr friend bool operator== <>(const vec3<T> lhs, const vec3<T> rhs);

    constexpr vec3<T> &operator+=(const vec3<T> &rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    };

    constexpr vec3<T> &operator-=(const vec3<T> &rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    };

    constexpr vec3<T> &operator*=(const vec3<T> &rhs) {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    };

    constexpr vec3<T> &operator/=(const vec3<T> &rhs) {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        return *this;
    };
};

template<typename T>
constexpr vec3<T> operator+(const vec3<T> &lhs, const vec3<T> &rhs) {
    return {
        .x = lhs.x + rhs.x,
        .y = lhs.y + rhs.y,
        .z = lhs.z + rhs.z,
    };
}

template<typename T>
constexpr vec3<T> operator-(const vec3<T> &lhs, const vec3<T> &rhs) {
    return {
        .x = lhs.x - rhs.x,
        .y = lhs.y - rhs.y,
        .z = lhs.z - rhs.z,
    };
}

template<typename T>
constexpr vec3<T> operator*(T lhs, const vec3<T> &rhs) {
    return {
        .x = lhs * rhs.x,
        .y = lhs * rhs.y,
        .z = lhs * rhs.z,
    };
}

template<typename T>
constexpr vec3<T> operator*(const vec3<T> &lhs, T rhs) {
    return {
        .x = lhs.x * rhs,
        .y = lhs.y * rhs,
        .z = lhs.z * rhs,
    };
}

template<typename T>
constexpr vec3<T> operator/(const vec3<T> &lhs, T rhs) {
    return {
        .x = lhs.x / rhs,
        .y = lhs.y / rhs,
        .z = lhs.z / rhs,
    };
}

template<typename T>
constexpr bool operator==(vec3<T> lhs, vec3<T> rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

template<typename T>
constexpr T dot(const vec3<T> &lhs, const vec3<T> &rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

template<typename T>
constexpr T absDot(const vec3<T> &lhs, const vec3<T> &rhs) {
    return std::fabs(dot(lhs, rhs));
}

template<typename T>
constexpr T length(const vec3<T> &vector) {
    return std::sqrt(dot(vector, vector));
}

template<typename T>
constexpr vec3<T> normalize(const vec3<T> &vector) {
    return vector / length(vector);
}

template<typename T>
constexpr vec3<T> projection(const vec3<T> &A, const vec3<T> &B) {
    return dot(A, B) / dot(B, B) * B;
}

template<typename T>
constexpr vec3<T> reflect(const vec3<T> &direction, const vec3<T> &normal) {
    return direction - (T(2) * projection(direction, normal));
}

using vec3f = vec3<float>;
using vec3i = vec3<int>;
using vec3u = vec3<uint32_t>;

} // namespace cobalt

#endif // COBALT_VEC3_H
