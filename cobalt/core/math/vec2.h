#ifndef CBLT_VEC2_H
#define CBLT_VEC2_H

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

using vec2u = vec2<unsigned int>;
using vec2f = vec2<float>;

} // namespace cblt

#endif // CBLT_VEC2_H
