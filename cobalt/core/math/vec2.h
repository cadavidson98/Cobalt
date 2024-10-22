#ifndef CBLT_VEC2_H
#define CBLT_VEC2_H

namespace cblt {
struct vec2u {
        unsigned int x;
        unsigned int y;

        vec2u(unsigned int v): x{v}, y{v} {};
        vec2u(unsigned int _x, unsigned int _y): x{_x}, y{_y} {};

        friend vec2u operator/(vec2u lhs, unsigned int rhs);
        friend vec2u operator*(vec2u lhs, unsigned int rhs);
};

struct vec2f {
        float x;
        float y;
};

inline vec2u operator/(vec2u lhs, unsigned int rhs) {
    return vec2u{lhs.x / rhs, lhs.y / rhs};
}

inline vec2u operator*(vec2u lhs, unsigned int rhs) {
    return vec2u{lhs.x * rhs, lhs.y * rhs};
}
} // namespace cblt

#endif // CBLT_VEC2_H
