#ifndef COBALT_COLOR_RGB
#define COBALT_COLOR_RGB

#include "xyz.h"

#include "math/math_types.h"

namespace cobalt::color::rgb {

enum class Colorspace {
    kSRGB,
    kDCIP3,
};

struct Value {
    float r;
    float g;
    float b;

    friend Value operator+(const Value &lhs, const Value &rhs);
    friend Value operator*(float lhs, const Value &rhs);

    Value &operator+=(const Value &rhs) {
        r += rhs.r;
        g += rhs.g;
        b += rhs.b;
        return *this;
    }
};

inline Value operator+(const Value &lhs, const Value &rhs) {
    return Value{
        .r = lhs.r + rhs.r,
        .g = lhs.g + rhs.g,
        .b = lhs.b + rhs.b,
    };
}

inline Value operator*(float lhs, const Value &rhs) {
    return Value{
        .r = lhs * rhs.r,
        .g = lhs * rhs.g,
        .b = lhs * rhs.b,
    };
}

namespace {

// converted from xyY to XYZ
constexpr vec3f kD65Whitepoint = {
    .x = 0.95045593f,
    .y = 1.f,
    .z = 1.08905775f,
};

constexpr vec3f kDCIP3Whitepoint = {
    .x = 0.89458689,
    .y = 1.0f,
    .z = 0.95441595,
};

template<Colorspace colorspace>
struct colorspace_traits {
    static constexpr mat3f rgbPrimaries = {0.f};

    static constexpr vec3f whitePoint = {
        .x = 0.f,
        .y = 0.f,
        .z = 0.f,
    };
};

template<>
struct colorspace_traits<Colorspace::kSRGB> {
    static constexpr mat3f rgbPrimaries = {
        {.x = .64f, .y = .33f, .z = .03f},
        {.x = .30f, .y = .60f, .z = .10f},
        {.x = .15f, .y = .06f, .z = .79f},
    };

    static constexpr vec3f whitePoint = kD65Whitepoint;
};

template<>
struct colorspace_traits<Colorspace::kDCIP3> {
    static constexpr mat3f rgbPrimaries = {
        {.x = .680f, .y = .32f, .z = 0.00f},
        {.x = .265f, .y = .69f, .z = .045f},
        {.x = .150f, .y = .06f, .z = 0.79f},
    };

    static constexpr vec3f whitePoint = kDCIP3Whitepoint;
};

template<Colorspace colorspace>
constexpr mat3f rgbToXYZTransform() {
    constexpr Result inversePrimaries = invert(colorspace_traits<colorspace>::rgbPrimaries);
    static_assert(inversePrimaries.valid);

    constexpr vec3f adjustedWhitepoint = inversePrimaries.inverse * colorspace_traits<colorspace>::whitePoint;

    constexpr mat3f rgbToXYZ = colorspace_traits<colorspace>::rgbPrimaries * mat3f(adjustedWhitepoint);
    return rgbToXYZ;
}

template<Colorspace colorspace>
constexpr mat3f xyzToRGBTransform() {
    constexpr mat3f rgbToXYZ = rgbToXYZTransform<colorspace>();
    constexpr Result xyzToRGB = invert(rgbToXYZ);

    static_assert(xyzToRGB.valid);
    return xyzToRGB.inverse;
}

} // anonymous namespace

constexpr mat3f convertFromXYZ(Colorspace colorspace) {
    switch (colorspace) {
    case Colorspace::kSRGB :
        return xyzToRGBTransform<Colorspace::kSRGB>();
    case Colorspace::kDCIP3 :
        return xyzToRGBTransform<Colorspace::kDCIP3>();
    default :
        assert(false);
    };
}

} // namespace cobalt::color::rgb

#endif // COBALT_COLOR_RGB
