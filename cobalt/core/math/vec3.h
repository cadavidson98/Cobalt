#ifndef CBLT_VEC3_H
#define CBLT_VEC3_H

#include <cmath>

namespace cblt {
struct vec3f {
        float x;
        float y;
        float z;

        friend vec3f operator+(const vec3f &lhs, const vec3f &rhs) {
            return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
        };

        friend vec3f operator/(const vec3f &lhs, float rhs) {
            return {lhs.x / rhs, lhs.y / rhs, lhs.z / rhs};
        }
};

inline float dot(const vec3f &lhs, const vec3f &rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

inline float absDot(const vec3f &lhs, const vec3f &rhs) {
    return std::fabs(dot(lhs, rhs));
}

inline float length(const vec3f &vector) {
    return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;
}

inline vec3f normalize(const vec3f &vector) {
    return vector / length(vector);
}
} // namespace cblt

#endif // CBLT_VEC3_H
