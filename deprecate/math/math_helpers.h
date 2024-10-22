#ifndef CBLT_MATH_HELPERS_H
#define CBLT_MATH_HELPERS_H

namespace cblt {
    template <class T>
    inline T toRadians(T x) {
        return x * PI_f / 180.f;
    }

    template <class T>
    inline T toDegrees(T x) {
        return x * 180.f / PI_f;
    }

    template <class T>
    inline T sqr(T x) {
        return x * x;
    }

    template <typename T>
    inline T lerp(T x, T y, float t) {
        return x * (1.f - t) + y * t;
    }
}
#endif  // CBLT_MATH_HELPERS_H