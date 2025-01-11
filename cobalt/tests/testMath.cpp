#include "mat4.h"
#include "simd/simd_mat3.h"
#include "simd/simd_mat4.h"
#include "simd/simd_vec3.h"
#include "simd/simd_vec4.h"
#include "vec2.h"
#include "vec3.h"
#include "vec4.h"

#include <cstring>
#include <gtest/gtest.h>

namespace {

static constexpr float kEpsilon = 1e-4f;

} // anonymous namespace

TEST(CobaltCoreMathTests, TestVec2f) {
    static const cblt::vec2f x(1.f, 0.f);
    static const cblt::vec2f y(0.f, 1.f);
    {
        static const cblt::vec2f a(2.f, 4.f);
        static const cblt::vec2f b(10.f, 24.f);
        // vector add
        const cblt::vec2f sum = x + y;
        EXPECT_EQ(sum, cblt::vec2f(1.f, 1.f));
        // vector subtract
        const cblt::vec2f diff = x - y;
        EXPECT_EQ(diff, cblt::vec2f(1.f, -1.f));
    }
    {
        // scalar div
        const cblt::vec2f scalarDiv = y / 4.f;
        EXPECT_EQ(scalarDiv, cblt::vec2f(0.f, .25f));
    }
    {
        // lengthSqr
        const cblt::vec2f z(-4.f, 5.f);
        const cblt::vec2f zero(0.f);

        const float xLength = cblt::lengthSqr(x);
        const float yLength = cblt::lengthSqr(y);
        const float zLength = cblt::lengthSqr(z);
        const float zeroLength = cblt::lengthSqr(zero);

        EXPECT_EQ(xLength, 1.f);
        EXPECT_EQ(yLength, 1.f);
        EXPECT_NEAR(zLength, 41.f, kEpsilon);
        EXPECT_EQ(zeroLength, 0.f);
    }
    {
        // abs
        const cblt::vec2f values(-1.f, 2.f);
        const cblt::vec2f absValues = cblt::abs(values);

        EXPECT_EQ(absValues, cblt::vec2f(1.f, 2.f));

        const cblt::vec2f positives(3.f, 4.f);
        const cblt::vec2f absPositives = cblt::abs(positives);
        EXPECT_EQ(absPositives, positives);

        const cblt::vec2f negatives(-5.f, -6.f);
        const cblt::vec2f absNegatives = cblt::abs(negatives);
        EXPECT_EQ(absNegatives, cblt::vec2f(5.f, 6.f));

        const cblt::vec2f zero(0.f, 0.f);
        const cblt::vec2f absZero = cblt::abs(zero);
        EXPECT_EQ(absZero, zero);
    }
    {
        // clamp
        const cblt::vec2f values(-1.f, 2.f);

        const cblt::vec2f positiveOnly = cblt::clamp(values, 0.f, 10.f);
        EXPECT_EQ(positiveOnly, cblt::vec2f(0.f, 2.f));

        const cblt::vec2f negativeOnly = cblt::clamp(values, -10.f, 0.f);
        EXPECT_EQ(negativeOnly, cblt::vec2f(-1.f, 0.f));

        const cblt::vec2f unclamped = cblt::clamp(values, -8.f, 8.f);
        EXPECT_EQ(unclamped, values);
    }
}

TEST(CobaltCoreMathTests, TestVec3f) {
    static const cblt::vec3f x(1.f, 0.f, 0.f);
    static const cblt::vec3f y(0.f, 1.f, 0.f);
    {
        static const cblt::vec3f a(2.f, 4.f, 8.f);
        static const cblt::vec3f b(10.f, 24.f, 56.f);
        // vector add
        const cblt::vec3f sum = x + y;
        EXPECT_EQ(sum, cblt::vec3f(1.f, 1.f, 0.f));
        // vector subtract
        const cblt::vec3f diff = x - y;
        EXPECT_EQ(diff, cblt::vec3f(1.f, -1.f, 0.f));
    }
    {
        // scalar div
        const cblt::vec3f scalarDiv = y / 4.f;
        EXPECT_EQ(scalarDiv, cblt::vec3f(0.f, .25f, 0.f));
    }
    {
        // dot
        const cblt::vec3f z(1.f, -2.f, 3.f);

        const float xDotY = cblt::dot(x, y);
        const float xDotZ = cblt::dot(x, z);
        const float yDotZ = cblt::dot(y, z);

        EXPECT_EQ(xDotY, 0.f);
        EXPECT_EQ(xDotZ, 1.f);
        EXPECT_EQ(yDotZ, -2.f);

        // commutative dot product
        const float yDotX = cblt::dot(y, x);
        const float zDotX = cblt::dot(z, x);
        const float zDotY = cblt::dot(z, y);

        EXPECT_EQ(xDotY, yDotX);
        EXPECT_EQ(xDotZ, zDotX);
        EXPECT_EQ(yDotZ, zDotY);
    }
    {
        // absDot
        const cblt::vec3f z(1.f, -2.f, 3.f);

        const float xDotY = cblt::absDot(x, y);
        const float xDotZ = cblt::absDot(x, z);
        const float yDotZ = cblt::absDot(y, z);

        EXPECT_EQ(xDotY, 0.f);
        EXPECT_EQ(xDotZ, 1.f);
        EXPECT_EQ(yDotZ, 2.f);

        // commutative dot product
        const float yDotX = cblt::absDot(y, x);
        const float zDotX = cblt::absDot(z, x);
        const float zDotY = cblt::absDot(z, y);

        EXPECT_EQ(xDotY, yDotX);
        EXPECT_EQ(xDotZ, zDotX);
        EXPECT_EQ(yDotZ, zDotY);
    }
    {
        // length

        const cblt::vec3f z(-4.f, 5.f, -6.f);
        const cblt::vec3f zero(0.f);

        const float xLength = cblt::length(x);
        const float yLength = cblt::length(y);
        const float zLength = cblt::length(z);
        const float zeroLength = cblt::length(zero);

        EXPECT_EQ(xLength, 1.f);
        EXPECT_EQ(yLength, 1.f);
        EXPECT_NEAR(zLength, 8.77496f, kEpsilon);
        EXPECT_EQ(zeroLength, 0.f);
    }
    {
        // normalize
        const cblt::vec3f xNormalized = cblt::normalize(x);
        const cblt::vec3f yNormalized = cblt::normalize(y);
        const cblt::vec3f zNormalized = cblt::normalize(cblt::vec3f{3.f, 3.f, 3.f});

        EXPECT_EQ(xNormalized, x);
        EXPECT_EQ(yNormalized, y);
        EXPECT_NEAR(zNormalized.x, 1.f / std::sqrt(3.f), kEpsilon);
        EXPECT_NEAR(zNormalized.y, 1.f / std::sqrt(3.f), kEpsilon);
        EXPECT_NEAR(zNormalized.z, 1.f / std::sqrt(3.f), kEpsilon);
    }
}

TEST(CobaltCoreMathTests, TestVec4f) {
    static const cblt::vec4f x(1.f, 2.f, 3.f, 4.f);
    static const cblt::vec4f y(4.f, 3.f, 2.f, 1.f);
    {
        static const cblt::vec4f a(20.f, 30.f, 63.f, 96.f);
        static const cblt::vec4f b(5.f, 6.f, 7.f, 8.f);
        // add
        const cblt::vec4f sum = x + y;
        EXPECT_EQ(sum, cblt::vec4f(5.f, 5.f, 5.f, 5.f));
        // subtract
        const cblt::vec4f diff = x - y;
        EXPECT_EQ(diff, cblt::vec4f(-3.f, -1.f, 1.f, 3.f));
        // multiply
        const cblt::vec4f prod = x * y;
        EXPECT_EQ(prod, cblt::vec4f(4.f, 6.f, 6.f, 4.f));
        // divide
        const cblt::vec4f quot = a / b;
        EXPECT_EQ(quot, cblt::vec4f(4.f, 5.f, 9.f, 12.f));
    }
    {
        // scalar mult
        const cblt::vec4f scalarProd = x * 2.f;
        EXPECT_EQ(scalarProd, cblt::vec4f(2.f, 4.f, 6.f, 8.f));
        // scalar div
        const cblt::vec4f scalarDiv = y / 4.f;
        EXPECT_EQ(scalarDiv, cblt::vec4f(1.f, .75f, .5f, .25f));
        // commutatave scalar mult
        const cblt::vec4f commutativeScalarProd = 2.f * x;
        EXPECT_EQ(commutativeScalarProd, cblt::vec4f(2.f, 4.f, 6.f, 8.f));
        EXPECT_EQ(commutativeScalarProd, scalarProd);
    }
    {
        // clamp
        const cblt::vec4f negatives(-1.f, 2.f, -4.f, 8.f);

        const cblt::vec4f positiveOnly = cblt::clamp(negatives, 0.f, 10.f);
        EXPECT_EQ(positiveOnly, cblt::vec4f(0.f, 2.f, 0.f, 8.f));

        const cblt::vec4f negativeOnly = cblt::clamp(negatives, -10.f, 0.f);
        EXPECT_EQ(negativeOnly, cblt::vec4f(-1.f, 0.f, -4.f, 0.f));

        const cblt::vec4f unclamped = cblt::clamp(negatives, -8.f, 8.f);
        EXPECT_EQ(unclamped, negatives);
    }
}

TEST(CobaltCoreMathTests, TestSimdVec3f) {
    static const cblt::simd::vec3f x(1.f, 0.f, 0.f);
    static const cblt::simd::vec3f y(0.f, 1.f, 0.f);
    {
        static const cblt::simd::vec3f a(2.f, 4.f, 8.f);
        static const cblt::simd::vec3f b(10.f, 24.f, 56.f);
        // vector add
        const cblt::simd::vec3f sum = x + y;
        EXPECT_EQ(sum, cblt::simd::vec3f(1.f, 1.f, 0.f));
        // vector subtract
        const cblt::simd::vec3f diff = x - y;
        EXPECT_EQ(diff, cblt::simd::vec3f(1.f, -1.f, 0.f));
        // vector mult
        const cblt::simd::vec3f prod = a * b;
        EXPECT_EQ(prod, cblt::simd::vec3f(20.f, 96.f, 448.f));
        // vector div
        const cblt::simd::vec3f quot = b / a;
        EXPECT_EQ(quot, cblt::simd::vec3f(5.f, 6.f, 7.f));
    }
    {
        // scalar mult
        const cblt::simd::vec3f scalarProd = x * 2.f;
        EXPECT_EQ(scalarProd, cblt::simd::vec3f(2.f, 0.f, 0.f));
        // scalar div
        const cblt::simd::vec3f scalarDiv = y / 4.f;
        EXPECT_EQ(scalarDiv, cblt::simd::vec3f(0.f, .25f, 0.f));
        // scalar mult
        const cblt::simd::vec3f commutativeScalarProd = 2.f * x;
        EXPECT_EQ(commutativeScalarProd, cblt::simd::vec3f(2.f, 0.f, 0.f));
        EXPECT_EQ(commutativeScalarProd, scalarProd);
    }
    {
        // min max
        const cblt::simd::vec3f minVec = cblt::simd::min(x, y);
        const cblt::simd::vec3f maxVec = cblt::simd::max(x, y);
        EXPECT_EQ(minVec, cblt::simd::vec3f(0.f, 0.f, 0.f));
        EXPECT_EQ(maxVec, cblt::simd::vec3f(1.f, 1.f, 0.f));
    }
    {
        // reduce min max
        static const cblt::simd::vec3f a(1.f, 2.f, 3.f);
        const float minVal = cblt::simd::reduceMin(a);
        const float maxVal = cblt::simd::reduceMax(a);
        EXPECT_EQ(minVal, 1.f);
        EXPECT_EQ(maxVal, 3.f);
    }
}

TEST(CobaltCoreMathTests, TestSimdVec4f) {
    static const cblt::simd::vec4f x(1.f, 2.f, 3.f, 4.f);
    static const cblt::simd::vec4f y(4.f, 3.f, 2.f, 1.f);
    {
        static const cblt::simd::vec4f a(20.f, 30.f, 63.f, 96.f);
        static const cblt::simd::vec4f b(5.f, 6.f, 7.f, 8.f);
        // add
        const cblt::simd::vec4f sum = x + y;
        EXPECT_EQ(sum, cblt::simd::vec4f(5.f, 5.f, 5.f, 5.f));
        // subtract
        const cblt::simd::vec4f diff = x - y;
        EXPECT_EQ(diff, cblt::simd::vec4f(-3.f, -1.f, 1.f, 3.f));
        // multiply
        const cblt::simd::vec4f prod = x * y;
        EXPECT_EQ(prod, cblt::simd::vec4f(4.f, 6.f, 6.f, 4.f));
        // divide
        const cblt::simd::vec4f quot = a / b;
        EXPECT_EQ(quot, cblt::simd::vec4f(4.f, 5.f, 9.f, 12.f));
    }
    {
        // scalar mult
        const cblt::simd::vec4f scalarProd = x * 2.f;
        EXPECT_EQ(scalarProd, cblt::simd::vec4f(2.f, 4.f, 6.f, 8.f));
        // scalar div
        const cblt::simd::vec4f scalarDiv = y / 4.f;
        EXPECT_EQ(scalarDiv, cblt::simd::vec4f(1.f, .75f, .5f, .25f));
        // commutatave scalar mult
        const cblt::simd::vec4f commutativeScalarProd = 2.f * x;
        EXPECT_EQ(commutativeScalarProd, cblt::simd::vec4f(2.f, 4.f, 6.f, 8.f));
        EXPECT_EQ(commutativeScalarProd, scalarProd);
    }
    {
        // min max
        const cblt::simd::vec4f minVec = cblt::simd::min(x, y);
        const cblt::simd::vec4f maxVec = cblt::simd::max(x, y);
        EXPECT_EQ(minVec, cblt::simd::vec4f(1.f, 2.f, 2.f, 1.f));
        EXPECT_EQ(maxVec, cblt::simd::vec4f(4.f, 3.f, 3.f, 4.f));
        // reduce min max
        const float minVal = cblt::simd::reduceMin(x);
        const float maxVal = cblt::simd::reduceMax(y);
        EXPECT_EQ(minVal, 1.f);
        EXPECT_EQ(maxVal, 4.f);
    }
}

TEST(CobaltCoreMathTests, TestSimdMat3f) {
    static const cblt::simd::mat3f I(1.f);
    static const cblt::simd::mat3f A({0.f, 1.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f, 1.f});
    static const cblt::simd::mat3f B({1.f, 0.f, 0.f}, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f});

    {
        const cblt::simd::mat3f C = A * B;
        const cblt::simd::mat3f D = B * A;

        static const cblt::simd::mat3f expectedC({0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}, {1.f, 0.f, 0.f});
        static const cblt::simd::mat3f expectedD({0.f, 0.f, 1.f}, {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f});

        EXPECT_EQ(C, expectedC);
        EXPECT_EQ(D, expectedD);
    }

    {
        const cblt::simd::mat3f Sum = A + B;
        const cblt::simd::mat3f Diff = A - B;

        static const cblt::simd::mat3f expectedSum({1.f, 1.f, 0.f}, {1.f, 0.f, 1.f}, {0.f, 1.f, 1.f});
        static const cblt::simd::mat3f expectedDiff({-1.f, 1.f, 0.f}, {1.f, 0.f, -1.f}, {0.f, -1.f, 1.f});

        EXPECT_EQ(Sum, expectedSum);
        EXPECT_EQ(Diff, expectedDiff);
    }

    static const cblt::simd::vec3f a(2.f, 4.f, .5f);
    static const cblt::simd::vec3f b(0.f, 3.f, 9.f);

    {
        const cblt::simd::vec3f vecC = A * a;
        const cblt::simd::vec3f vecD = B * b;
        const cblt::simd::vec3f vecIa = I * a;
        static const cblt::simd::vec3f expectedVecC(4.f, 2.f, .5f);
        static const cblt::simd::vec3f expectedVecD(0.f, 9.f, 3.f);

        EXPECT_EQ(vecC, expectedVecC);
        EXPECT_EQ(vecD, expectedVecD);
        EXPECT_EQ(vecIa, a);
    }
}

TEST(CobaltCoreMathTests, TestSimdMat4f) {
    static const cblt::simd::mat4f I(1.f);
    static const cblt::simd::mat4f
        A({0.f, 1.f, 0.f, 0.f}, {1.f, 0.f, 0.f, 0.f}, {0.f, 0.f, 1.f, 0.f}, {0.f, 0.f, 0.f, 1.f});
    static const cblt::simd::mat4f
        B({2.f, 0.f, 0.f, 0.f}, {0.f, 3.f, 0.f, 0.f}, {0.f, 0.f, 4.f, 0.f}, {0.f, 0.f, 0.f, 1.f});

    {
        const cblt::simd::mat4f C = A * B;
        const cblt::simd::mat4f D = B * A;

        static const cblt::simd::mat4f
            expectedC({0.f, 2.f, 0.f, 0.f}, {3.f, 0.f, 0.f, 0.f}, {0.f, 0.f, 4.f, 0.f}, {0.f, 0.f, 0.f, 1.f});
        static const cblt::simd::mat4f
            expectedD({0.f, 3.f, 0.f, 0.f}, {2.f, 0.f, 0.f, 0.f}, {0.f, 0.f, 4.f, 0.f}, {0.f, 0.f, 0.f, 1.f});

        EXPECT_EQ(C, expectedC);
        EXPECT_EQ(D, expectedD);
    }

    {
        const cblt::simd::mat4f Sum = A + B;
        const cblt::simd::mat4f Diff = A - B;

        static const cblt::simd::mat4f
            expectedSum({2.f, 1.f, 0.f, 0.f}, {1.f, 3.f, 0.f, 0.f}, {0.f, 0.f, 5.f, 0.f}, {0.f, 0.f, 0.f, 2.f});
        static const cblt::simd::mat4f
            expectedDiff({-2.f, 1.f, 0.f, 0.f}, {1.f, -3.f, 0.f, 0.f}, {0.f, 0.f, -3.f, 0.f}, {0.f, 0.f, 0.f, 0.f});

        EXPECT_EQ(Sum, expectedSum);
        EXPECT_EQ(Diff, expectedDiff);
    }

    static const cblt::simd::vec4f a(2.f, 4.f, .5f, .25f);
    static const cblt::simd::vec4f b(0.f, 3.f, 9.f, 6.f);

    {
        const cblt::simd::vec4f vecC = A * a;
        const cblt::simd::vec4f vecD = B * b;
        const cblt::simd::vec4f vecIa = I * a;
        static const cblt::simd::vec4f expectedVecC(4.f, 2.f, .5f, .25f);
        static const cblt::simd::vec4f expectedVecD(0.f, 9.f, 36.f, 6.f);

        EXPECT_EQ(vecC, expectedVecC);
        EXPECT_EQ(vecD, expectedVecD);
        EXPECT_EQ(vecIa, a);
    }
}
