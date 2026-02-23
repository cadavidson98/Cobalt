#include "math/mat3.h"
#include "math/mat4.h"
#include "math/simd/simd_mat3.h"
#include "math/simd/simd_mat4.h"
#include "math/simd/simd_vec3.h"
#include "math/simd/simd_vec4.h"
#include "math/vec2.h"
#include "math/vec3.h"
#include "math/vec4.h"

#include <gtest/gtest.h>

namespace cobalt {

namespace {

constexpr float kEpsilon = 1e-4f;

void expectNear(const vec3f &value, const vec3f &expected) {
    EXPECT_NEAR(value.x, expected.x, kEpsilon);
    EXPECT_NEAR(value.y, expected.y, kEpsilon);
    EXPECT_NEAR(value.z, expected.z, kEpsilon);
}

void expectNear(const mat3f &value, const mat3f &expected) {
    expectNear(value.columns[0], expected.columns[0]);
    expectNear(value.columns[1], expected.columns[1]);
    expectNear(value.columns[2], expected.columns[2]);
}

} // anonymous namespace

TEST(CobaltCoreMathTests, TestVec2f) {
    static constexpr vec2f x(1.f, 0.f);
    static constexpr vec2f y(0.f, 1.f);
    {
        // vector add
        const vec2f sum = x + y;
        EXPECT_EQ(sum, vec2f(1.f, 1.f));
        // vector subtract
        const vec2f diff = x - y;
        EXPECT_EQ(diff, vec2f(1.f, -1.f));
    }
    {
        // scalar div
        const vec2f scalarDiv = y / 4.f;
        EXPECT_EQ(scalarDiv, vec2f(0.f, .25f));
    }
    {
        // lengthSqr
        const vec2f z(-4.f, 5.f);
        static constexpr vec2f kZero{
            .x = 0.f,
            .y = 0.f,
        };

        const float xLength = lengthSqr(x);
        const float yLength = lengthSqr(y);
        const float zLength = lengthSqr(z);
        const float zeroLength = lengthSqr(kZero);

        EXPECT_EQ(xLength, 1.f);
        EXPECT_EQ(yLength, 1.f);
        EXPECT_NEAR(zLength, 41.f, kEpsilon);
        EXPECT_EQ(zeroLength, 0.f);
    }
    {
        // abs
        const vec2f values(-1.f, 2.f);
        const vec2f absValues = abs(values);

        EXPECT_EQ(absValues, vec2f(1.f, 2.f));

        const vec2f positives(3.f, 4.f);
        const vec2f absPositives = abs(positives);
        EXPECT_EQ(absPositives, positives);

        const vec2f negatives(-5.f, -6.f);
        const vec2f absNegatives = abs(negatives);
        EXPECT_EQ(absNegatives, vec2f(5.f, 6.f));

        const vec2f zero(0.f, 0.f);
        const vec2f absZero = abs(zero);
        EXPECT_EQ(absZero, zero);
    }
    {
        // clamp
        const vec2f values(-1.f, 2.f);

        const vec2f positiveOnly = clamp(values, 0.f, 10.f);
        EXPECT_EQ(positiveOnly, vec2f(0.f, 2.f));

        const vec2f negativeOnly = clamp(values, -10.f, 0.f);
        EXPECT_EQ(negativeOnly, vec2f(-1.f, 0.f));

        const vec2f unclamped = clamp(values, -8.f, 8.f);
        EXPECT_EQ(unclamped, values);
    }
}

TEST(CobaltCoreMathTests, TestVec3f) {
    static const vec3f x(1.f, 0.f, 0.f);
    static const vec3f y(0.f, 1.f, 0.f);
    {
        // vector add
        const vec3f sum = x + y;
        EXPECT_EQ(sum, vec3f(1.f, 1.f, 0.f));
        // vector subtract
        const vec3f diff = x - y;
        EXPECT_EQ(diff, vec3f(1.f, -1.f, 0.f));
    }
    {
        // scalar div
        const vec3f scalarDiv = y / 4.f;
        EXPECT_EQ(scalarDiv, vec3f(0.f, .25f, 0.f));
    }
    {
        // dot
        const vec3f z(1.f, -2.f, 3.f);

        const float xDotY = dot(x, y);
        const float xDotZ = dot(x, z);
        const float yDotZ = dot(y, z);

        EXPECT_EQ(xDotY, 0.f);
        EXPECT_EQ(xDotZ, 1.f);
        EXPECT_EQ(yDotZ, -2.f);

        // commutative dot product
        const float yDotX = dot(y, x);
        const float zDotX = dot(z, x);
        const float zDotY = dot(z, y);

        EXPECT_EQ(xDotY, yDotX);
        EXPECT_EQ(xDotZ, zDotX);
        EXPECT_EQ(yDotZ, zDotY);
    }
    {
        // absDot
        const vec3f z(1.f, -2.f, 3.f);

        const float xDotY = absDot(x, y);
        const float xDotZ = absDot(x, z);
        const float yDotZ = absDot(y, z);

        EXPECT_EQ(xDotY, 0.f);
        EXPECT_EQ(xDotZ, 1.f);
        EXPECT_EQ(yDotZ, 2.f);

        // commutative dot product
        const float yDotX = absDot(y, x);
        const float zDotX = absDot(z, x);
        const float zDotY = absDot(z, y);

        EXPECT_EQ(xDotY, yDotX);
        EXPECT_EQ(xDotZ, zDotX);
        EXPECT_EQ(yDotZ, zDotY);
    }
    {
        // length

        const vec3f z(-4.f, 5.f, -6.f);
        const vec3f zero(0.f, 0.f, 0.f);

        const float xLength = length(x);
        const float yLength = length(y);
        const float zLength = length(z);
        const float zeroLength = length(zero);

        EXPECT_EQ(xLength, 1.f);
        EXPECT_EQ(yLength, 1.f);
        EXPECT_NEAR(zLength, 8.77496f, kEpsilon);
        EXPECT_EQ(zeroLength, 0.f);
    }
    {
        // normalize
        const vec3f xNormalized = normalize(x);
        const vec3f yNormalized = normalize(y);
        const vec3f zNormalized = normalize(vec3f{3.f, 3.f, 3.f});

        EXPECT_EQ(xNormalized, x);
        EXPECT_EQ(yNormalized, y);
        EXPECT_NEAR(zNormalized.x, 1.f / std::sqrt(3.f), kEpsilon);
        EXPECT_NEAR(zNormalized.y, 1.f / std::sqrt(3.f), kEpsilon);
        EXPECT_NEAR(zNormalized.z, 1.f / std::sqrt(3.f), kEpsilon);
    }
}

TEST(CobaltCoreMathTests, TestVec4f) {
    static const vec4f x(1.f, 2.f, 3.f, 4.f);
    static const vec4f y(4.f, 3.f, 2.f, 1.f);
    {
        static const vec4f a(20.f, 30.f, 63.f, 96.f);
        static const vec4f b(5.f, 6.f, 7.f, 8.f);
        // add
        const vec4f sum = x + y;
        EXPECT_EQ(sum, vec4f(5.f, 5.f, 5.f, 5.f));
        // subtract
        const vec4f diff = x - y;
        EXPECT_EQ(diff, vec4f(-3.f, -1.f, 1.f, 3.f));
        // multiply
        const vec4f prod = x * y;
        EXPECT_EQ(prod, vec4f(4.f, 6.f, 6.f, 4.f));
        // divide
        const vec4f quot = a / b;
        EXPECT_EQ(quot, vec4f(4.f, 5.f, 9.f, 12.f));
    }
    {
        // scalar mult
        const vec4f scalarProd = x * 2.f;
        EXPECT_EQ(scalarProd, vec4f(2.f, 4.f, 6.f, 8.f));
        // scalar div
        const vec4f scalarDiv = y / 4.f;
        EXPECT_EQ(scalarDiv, vec4f(1.f, .75f, .5f, .25f));
        // commutatave scalar mult
        const vec4f commutativeScalarProd = 2.f * x;
        EXPECT_EQ(commutativeScalarProd, vec4f(2.f, 4.f, 6.f, 8.f));
        EXPECT_EQ(commutativeScalarProd, scalarProd);
    }
    {
        // clamp
        const vec4f negatives(-1.f, 2.f, -4.f, 8.f);

        const vec4f positiveOnly = clamp(negatives, 0.f, 10.f);
        EXPECT_EQ(positiveOnly, vec4f(0.f, 2.f, 0.f, 8.f));

        const vec4f negativeOnly = clamp(negatives, -10.f, 0.f);
        EXPECT_EQ(negativeOnly, vec4f(-1.f, 0.f, -4.f, 0.f));

        const vec4f unclamped = clamp(negatives, -8.f, 8.f);
        EXPECT_EQ(unclamped, negatives);
    }
}

TEST(CobaltCoreMathTests, TestSimdVec3f) {
    static const simd::vec3f x(1.f, 0.f, 0.f);
    static const simd::vec3f y(0.f, 1.f, 0.f);
    {
        static const simd::vec3f a(2.f, 4.f, 8.f);
        static const simd::vec3f b(10.f, 24.f, 56.f);
        // vector add
        const simd::vec3f sum = x + y;
        EXPECT_EQ(sum, simd::vec3f(1.f, 1.f, 0.f));
        // vector subtract
        const simd::vec3f diff = x - y;
        EXPECT_EQ(diff, simd::vec3f(1.f, -1.f, 0.f));
        // vector mult
        const simd::vec3f prod = a * b;
        EXPECT_EQ(prod, simd::vec3f(20.f, 96.f, 448.f));
        // vector div
        const simd::vec3f quot = b / a;
        EXPECT_EQ(quot, simd::vec3f(5.f, 6.f, 7.f));
    }
    {
        // scalar mult
        const simd::vec3f scalarProd = x * 2.f;
        EXPECT_EQ(scalarProd, simd::vec3f(2.f, 0.f, 0.f));
        // scalar div
        const simd::vec3f scalarDiv = y / 4.f;
        EXPECT_EQ(scalarDiv, simd::vec3f(0.f, .25f, 0.f));
        // scalar mult
        const simd::vec3f commutativeScalarProd = 2.f * x;
        EXPECT_EQ(commutativeScalarProd, simd::vec3f(2.f, 0.f, 0.f));
        EXPECT_EQ(commutativeScalarProd, scalarProd);
    }
    {
        // min max
        const simd::vec3f minVec = simd::min(x, y);
        const simd::vec3f maxVec = simd::max(x, y);
        EXPECT_EQ(minVec, simd::vec3f(0.f, 0.f, 0.f));
        EXPECT_EQ(maxVec, simd::vec3f(1.f, 1.f, 0.f));
    }
    {
        // reduce min max
        static const simd::vec3f a(1.f, 2.f, 3.f);
        const float minVal = simd::reduceMin(a);
        const float maxVal = simd::reduceMax(a);
        EXPECT_EQ(minVal, 1.f);
        EXPECT_EQ(maxVal, 3.f);
    }
}

TEST(CobaltCoreMathTests, TestSimdVec4f) {
    static const simd::vec4f x(1.f, 2.f, 3.f, 4.f);
    static const simd::vec4f y(4.f, 3.f, 2.f, 1.f);
    {
        static const simd::vec4f a(20.f, 30.f, 63.f, 96.f);
        static const simd::vec4f b(5.f, 6.f, 7.f, 8.f);
        // add
        const simd::vec4f sum = x + y;
        EXPECT_EQ(sum, simd::vec4f(5.f, 5.f, 5.f, 5.f));
        // subtract
        const simd::vec4f diff = x - y;
        EXPECT_EQ(diff, simd::vec4f(-3.f, -1.f, 1.f, 3.f));
        // multiply
        const simd::vec4f prod = x * y;
        EXPECT_EQ(prod, simd::vec4f(4.f, 6.f, 6.f, 4.f));
        // divide
        const simd::vec4f quot = a / b;
        EXPECT_EQ(quot, simd::vec4f(4.f, 5.f, 9.f, 12.f));
    }
    {
        // scalar mult
        const simd::vec4f scalarProd = x * 2.f;
        EXPECT_EQ(scalarProd, simd::vec4f(2.f, 4.f, 6.f, 8.f));
        // scalar div
        const simd::vec4f scalarDiv = y / 4.f;
        EXPECT_EQ(scalarDiv, simd::vec4f(1.f, .75f, .5f, .25f));
        // commutatave scalar mult
        const simd::vec4f commutativeScalarProd = 2.f * x;
        EXPECT_EQ(commutativeScalarProd, simd::vec4f(2.f, 4.f, 6.f, 8.f));
        EXPECT_EQ(commutativeScalarProd, scalarProd);
    }
    {
        // min max
        const simd::vec4f minVec = simd::min(x, y);
        const simd::vec4f maxVec = simd::max(x, y);
        EXPECT_EQ(minVec, simd::vec4f(1.f, 2.f, 2.f, 1.f));
        EXPECT_EQ(maxVec, simd::vec4f(4.f, 3.f, 3.f, 4.f));
        // reduce min max
        const float minVal = simd::reduceMin(x);
        const float maxVal = simd::reduceMax(y);
        EXPECT_EQ(minVal, 1.f);
        EXPECT_EQ(maxVal, 4.f);
    }
}

TEST(CobaltCoreMathTests, TestSimdMat3f) {
    static const simd::mat3f I(1.f);
    static const simd::mat3f A({0.f, 1.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f, 1.f});
    static const simd::mat3f B({1.f, 0.f, 0.f}, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f});

    {
        const simd::mat3f C = A * B;
        const simd::mat3f D = B * A;

        static const simd::mat3f expectedC({0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}, {1.f, 0.f, 0.f});
        static const simd::mat3f expectedD({0.f, 0.f, 1.f}, {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f});

        EXPECT_EQ(C, expectedC);
        EXPECT_EQ(D, expectedD);
    }

    {
        const simd::mat3f Sum = A + B;
        const simd::mat3f Diff = A - B;

        static const simd::mat3f expectedSum({1.f, 1.f, 0.f}, {1.f, 0.f, 1.f}, {0.f, 1.f, 1.f});
        static const simd::mat3f expectedDiff({-1.f, 1.f, 0.f}, {1.f, 0.f, -1.f}, {0.f, -1.f, 1.f});

        EXPECT_EQ(Sum, expectedSum);
        EXPECT_EQ(Diff, expectedDiff);
    }

    static const simd::vec3f a(2.f, 4.f, .5f);
    static const simd::vec3f b(0.f, 3.f, 9.f);

    {
        const simd::vec3f vecC = A * a;
        const simd::vec3f vecD = B * b;
        const simd::vec3f vecIa = I * a;
        static const simd::vec3f expectedVecC(4.f, 2.f, .5f);
        static const simd::vec3f expectedVecD(0.f, 9.f, 3.f);

        EXPECT_EQ(vecC, expectedVecC);
        EXPECT_EQ(vecD, expectedVecD);
        EXPECT_EQ(vecIa, a);
    }
}

TEST(CobaltCoreMathTests, TestSimdMat4f) {
    static const simd::mat4f I(1.f);
    static const simd::mat4f A({0.f, 1.f, 0.f, 0.f}, {1.f, 0.f, 0.f, 0.f}, {0.f, 0.f, 1.f, 0.f}, {0.f, 0.f, 0.f, 1.f});
    static const simd::mat4f B({2.f, 0.f, 0.f, 0.f}, {0.f, 3.f, 0.f, 0.f}, {0.f, 0.f, 4.f, 0.f}, {0.f, 0.f, 0.f, 1.f});

    {
        const simd::mat4f C = A * B;
        const simd::mat4f D = B * A;

        static const simd::mat4f
            expectedC({0.f, 2.f, 0.f, 0.f}, {3.f, 0.f, 0.f, 0.f}, {0.f, 0.f, 4.f, 0.f}, {0.f, 0.f, 0.f, 1.f});
        static const simd::mat4f
            expectedD({0.f, 3.f, 0.f, 0.f}, {2.f, 0.f, 0.f, 0.f}, {0.f, 0.f, 4.f, 0.f}, {0.f, 0.f, 0.f, 1.f});

        EXPECT_EQ(C, expectedC);
        EXPECT_EQ(D, expectedD);
    }

    {
        const simd::mat4f Sum = A + B;
        const simd::mat4f Diff = A - B;

        static const simd::mat4f
            expectedSum({2.f, 1.f, 0.f, 0.f}, {1.f, 3.f, 0.f, 0.f}, {0.f, 0.f, 5.f, 0.f}, {0.f, 0.f, 0.f, 2.f});
        static const simd::mat4f
            expectedDiff({-2.f, 1.f, 0.f, 0.f}, {1.f, -3.f, 0.f, 0.f}, {0.f, 0.f, -3.f, 0.f}, {0.f, 0.f, 0.f, 0.f});

        EXPECT_EQ(Sum, expectedSum);
        EXPECT_EQ(Diff, expectedDiff);
    }

    static const simd::vec4f a(2.f, 4.f, .5f, .25f);
    static const simd::vec4f b(0.f, 3.f, 9.f, 6.f);

    {
        const simd::vec4f vecC = A * a;
        const simd::vec4f vecD = B * b;
        const simd::vec4f vecIa = I * a;
        static const simd::vec4f expectedVecC(4.f, 2.f, .5f, .25f);
        static const simd::vec4f expectedVecD(0.f, 9.f, 36.f, 6.f);

        EXPECT_EQ(vecC, expectedVecC);
        EXPECT_EQ(vecD, expectedVecD);
        EXPECT_EQ(vecIa, a);
    }
}

TEST(CobaltCoreMathTests, TestInverse) {
    {
        static constexpr mat3f kIdentity(1.f);

        static constexpr Result kResult = invert(kIdentity);

        static_assert(kResult.valid);
        expectNear(kResult.inverse, kIdentity);
    }
    {
        static constexpr mat3f kTranslation({1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {7.f, 8.f, 1.f});

        static constexpr mat3f kInverseTranslation({1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {-7.f, -8.f, 1.f});

        static constexpr Result kResult = invert(kTranslation);

        static_assert(kResult.valid);
        expectNear(kResult.inverse, kInverseTranslation);
    }
    {
        static constexpr mat3f kScale(
            vec3f{
                .x = 2.f,
                .y = 4.f,
                .z = 8.f,
            }
        );
        static constexpr mat3f kInverseScale(
            vec3f{
                .x = 0.5f,
                .y = 0.25f,
                .z = 0.125f,
            }
        );

        static constexpr Result kResult = invert(kScale);

        static_assert(kResult.valid);
        expectNear(kResult.inverse, kInverseScale);
    }
    {
        static constexpr mat3f kTransform{
            {2.f, 0.f, 0.f},
            {0.f, 3.f, 0.f},
            {4.f, 9.f, 1.f},
        };

        static constexpr mat3f kInverseTransform{
            { .5f,       0.f, 0.f},
            { 0.f, 1.f / 3.f, 0.f},
            {-2.f,      -3.f, 1.f},
        };

        static constexpr Result kResult = invert(kTransform);

        static_assert(kResult.valid);
        expectNear(kResult.inverse, kInverseTransform);
    }
    {
        static constexpr mat3f kSingular(
            vec3f{.x = 0.f, .y = 0.f, .z = 0.f},
            vec3f{.x = 0.f, .y = 6.f, .z = 7.f},
            vec3f{.x = 0.f, .y = 7.f, .z = .6f}
        );

        static constexpr Result kResult = invert(kSingular);

        static_assert(!kResult.valid);
    }
    {
        static constexpr mat3f kRGBToXYZ = {
            { 0.4123908, 0.21263901, 0.01933082},
            {0.35758434, 0.71516868, 0.11919478},
            {0.18048079, 0.07219232, 0.95053215},
        };

        static constexpr mat3f kXYZToRGB = {
            { 3.24096994, -0.96924364,  0.05563008},
            {-1.53738318,  +1.8759675, -0.20397696},
            {-0.49861076, +0.04155506, +1.05697151},
        };

        static constexpr Result kResultRGBToXYZ = invert(kRGBToXYZ);
        static_assert(kResultRGBToXYZ.valid);

        expectNear(kResultRGBToXYZ.inverse, kXYZToRGB);

        static constexpr Result kResultXYZToRGB = invert(kXYZToRGB);
        static_assert(kResultXYZToRGB.valid);

        expectNear(kResultXYZToRGB.inverse, kRGBToXYZ);
    }
}

} // namespace cobalt
