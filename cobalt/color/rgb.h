#ifndef COBALT_COLOR_RGB_H
#define COBALT_COLOR_RGB_H

#include "xyz.h"

#include "math/math_types.h"

#include <concepts>
#include <ranges>

namespace cobalt::color::rgb {

struct Value {
    float r;
    float g;
    float b;

    [[nodiscard]] static constexpr Value create(vec3f rgb) {
        return Value{
            .r = rgb.x,
            .g = rgb.y,
            .b = rgb.z,
        };
    }

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

// converted from xyY to XYZ
constexpr xyz::Tristimulus kD65Whitepoint = {
    .x = 0.95045593f,
    .y = 1.f,
    .z = 1.08905775f,
};

constexpr xyz::Tristimulus kDCIP3Whitepoint = {
    .x = 0.89458689,
    .y = 1.0f,
    .z = 0.95441595,
};

// todo: is there a nice language feature I can use to bundle
// enum, all cases, and total number of cases in a single 'object'?
enum class Colorspace {
    kSRGB,
    kDCIP3,
};

constexpr Colorspace kAllColorspaces[] = {
    Colorspace::kSRGB,
    Colorspace::kDCIP3,
};

constexpr size_t kColorspaceCount = std::size(kAllColorspaces);

template<Colorspace colorspace>
struct colorspace_traits {
    static constexpr xyz::Tristimulus kRed = {
        .x = 0.f,
        .y = 0.f,
        .z = 0.f,
    };

    static constexpr xyz::Tristimulus kGreen = {
        .x = 0.f,
        .y = 0.f,
        .z = 0.f,
    };

    static constexpr xyz::Tristimulus kBlue = {
        .x = 0.f,
        .y = 0.f,
        .z = 0.f,
    };

    static constexpr xyz::Tristimulus kWhitePoint = {
        .x = 0.f,
        .y = 0.f,
        .z = 0.f,
    };
};

template<>
struct colorspace_traits<Colorspace::kSRGB> {
    static constexpr xyz::Tristimulus kRed = {
        .x = .64f,
        .y = .33f,
        .z = .03f,
    };

    static constexpr xyz::Tristimulus kGreen = {
        .x = .30f,
        .y = .60f,
        .z = .10f,
    };

    static constexpr xyz::Tristimulus kBlue = {
        .x = .15f,
        .y = .06f,
        .z = .79f,
    };

    static constexpr xyz::Tristimulus kWhitePoint = kD65Whitepoint;
};

template<>
struct colorspace_traits<Colorspace::kDCIP3> {
    static constexpr xyz::Tristimulus kRed = {
        .x = .680f,
        .y = .320f,
        .z = .000f,
    };

    static constexpr xyz::Tristimulus kGreen = {
        .x = .265f,
        .y = .690f,
        .z = .045f,
    };

    static constexpr xyz::Tristimulus kBlue = {
        .x = .15f,
        .y = .06f,
        .z = .79f,
    };

    static constexpr xyz::Tristimulus kWhitePoint = kDCIP3Whitepoint;
};

template<Colorspace colorspace>
constexpr mat3f rgbToXYZTransform() {
    constexpr mat3f primaries(
        {
            .x = colorspace_traits<colorspace>::kRed.x,
            .y = colorspace_traits<colorspace>::kRed.y,
            .z = colorspace_traits<colorspace>::kRed.z,
        },
        {
            .x = colorspace_traits<colorspace>::kGreen.x,
            .y = colorspace_traits<colorspace>::kGreen.y,
            .z = colorspace_traits<colorspace>::kGreen.z,
        },
        {
            .x = colorspace_traits<colorspace>::kBlue.x,
            .y = colorspace_traits<colorspace>::kBlue.y,
            .z = colorspace_traits<colorspace>::kBlue.z,
        }
    );

    constexpr Result inversePrimaries = invert(primaries);
    static_assert(inversePrimaries.valid);

    constexpr vec3f adjustedWhitepoint = inversePrimaries.inverse *
                                         vec3f{
                                             .x = colorspace_traits<colorspace>::kWhitePoint.x,
                                             .y = colorspace_traits<colorspace>::kWhitePoint.y,
                                             .z = colorspace_traits<colorspace>::kWhitePoint.z,
                                         };

    constexpr mat3f rgbToXYZ = primaries * mat3f(adjustedWhitepoint);
    return rgbToXYZ;
}

template<Colorspace colorspace>
constexpr mat3f xyzToRGBTransform() {
    constexpr mat3f rgbToXYZ = rgbToXYZTransform<colorspace>();
    constexpr Result xyzToRGB = invert(rgbToXYZ);

    static_assert(xyzToRGB.valid);
    return xyzToRGB.inverse;
}

constexpr mat3f convertFromXYZ(Colorspace colorspace) {
    switch (colorspace) {
    case Colorspace::kSRGB :
        return xyzToRGBTransform<Colorspace::kSRGB>();
    case Colorspace::kDCIP3 :
        return xyzToRGBTransform<Colorspace::kDCIP3>();
    default :
        assert(false);
        return mat3f(1.f);
    };
}

} // namespace cobalt::color::rgb

#endif // COBALT_COLOR_RGB_H
