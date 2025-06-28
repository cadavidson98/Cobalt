#ifndef CBLT_VEC3_H
#define CBLT_VEC3_H

#include <cmath>
#include <cstdint>

namespace cblt {

template<typename T>
struct vec3;

template<typename T>
vec3<T> operator+(const vec3<T> &lhs, const vec3<T> &rhs);

template<typename T>
vec3<T> operator-(const vec3<T> &lhs, const vec3<T> &rhs);

template<typename T>
vec3<T> operator*(T lhs, const vec3<T> &rhs);

template<typename T>
vec3<T> operator/(const vec3<T> &lhs, T rhs);

template<typename T>
bool operator==(vec3<T> lhs, vec3<T> rhs);

// TODO: SFINAE or concepts here?

template<typename T>
T dot(const vec3<T> &lhs, const vec3<T> &rhs);

template<typename T>
T absDot(const vec3<T> &lhs, const vec3<T> &rhs);

template<typename T>
T length(const vec3<T> &vector);

template<typename T>
vec3<T> normalize(const vec3<T> &vector);

// project A onto B
template<typename T>
vec3<T> projection(const vec3<T> &A, const vec3<T> &B);

template<typename T>
vec3<T> reflect(const vec3<T> &direction, const vec3<T> &normal);

template<typename T>
struct vec3 {
    T x;
    T y;
    T z;

    friend vec3<T> operator+ <>(const vec3<T> &lhs, const vec3<T> &rhs);
    friend vec3<T> operator- <>(const vec3<T> &lhs, const vec3<T> &rhs);
    friend vec3<T> operator* <>(T lhs, const vec3<T> &rhs);
    friend vec3<T> operator/ <>(const vec3<T> &lhs, T rhs);

    friend bool operator== <>(const vec3<T> lhs, const vec3<T> rhs);
};

template<typename T>
inline vec3<T> operator+(const vec3<T> &lhs, const vec3<T> &rhs) {
    return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

template<typename T>
inline vec3<T> operator-(const vec3<T> &lhs, const vec3<T> &rhs) {
    return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

template<typename T>
vec3<T> operator*(T lhs, const vec3<T> &rhs) {
    return {lhs * rhs.x, lhs * rhs.y, lhs * rhs.z};
}

template<typename T>
inline vec3<T> operator/(const vec3<T> &lhs, T rhs) {
    return {lhs.x / rhs, lhs.y / rhs, lhs.z / rhs};
}

template<typename T>
inline bool operator==(vec3<T> lhs, vec3<T> rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

template<typename T>
inline T dot(const vec3<T> &lhs, const vec3<T> &rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

template<typename T>
inline T absDot(const vec3<T> &lhs, const vec3<T> &rhs) {
    return std::fabs(dot(lhs, rhs));
}

template<typename T>
inline T length(const vec3<T> &vector) {
    return std::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}

template<typename T>
inline vec3<T> normalize(const vec3<T> &vector) {
    return vector / length(vector);
}

template<typename T>
inline vec3<T> projection(const vec3<T> &A, const vec3<T> &B) {
    return dot(A, B) / dot(B, B) * B;
}

template<typename T>
inline vec3<T> reflect(const vec3<T> &direction, const vec3<T> &normal) {
    return direction - (T(2) * projection(direction, normal));
}

using vec3f = vec3<float>;
using vec3i = vec3<int>;
using vec3u = vec3<uint32_t>;

} // namespace cblt

#endif // CBLT_VEC3_H
