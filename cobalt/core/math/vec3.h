#ifndef CBLT_VEC3_H
#define CBLT_VEC3_H

#include <cmath>

namespace cblt {
template<typename T>
struct vec3 {
        T x;
        T y;
        T z;

        template<typename T>
        friend vec3<T> operator+(const vec3<T> &lhs, const vec3<T> &rhs);
        template<typename T>
        friend vec3<T> operator-(const vec3<T> &lhs, const vec3<T> &rhs);
        template<typename T>
        friend vec3<T> operator/(const vec3<T> &lhs, T rhs);
        template<typename T>
        friend bool operator==(const vec3<T> lhs, const vec3<T> rhs);
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

using vec3f = vec3<float>;
using vec3i = vec3<int>;

} // namespace cblt

#endif // CBLT_VEC3_H
