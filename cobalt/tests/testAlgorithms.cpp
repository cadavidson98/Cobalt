#include "core/algorithms.h"
#include "core/morton_encoding.h"

#include <gtest/gtest.h>

#include <array>
#include <span>
#include <vector>

namespace cblt::test {

template<typename T>
void expectArraysEqual(std::span<const T> lhs, std::span<const T> rhs) {
    ASSERT_EQ(lhs.size(), rhs.size());
    for (size_t idx = 0; idx < lhs.size(); ++idx) {
        ASSERT_EQ(lhs[idx], rhs[idx]);
    }
}

}  // namespace cblt::test

TEST(CobaltCoreAlgorithmsTests, TestMortonEncoding) {
    {
        static constexpr uint32_t kExpected = 0b00001001001001001001001001001001;
        const constexpr uint32_t kInput = 0b00000000000000000000001111111111;
        const uint32_t output = cblt::core::mortonEncode(kInput, 0, 0);
        ASSERT_EQ(output, kExpected);
    }
    {
        static constexpr uint32_t kExpected = 0b00010010010010010010010010010010;
        const constexpr uint32_t kInput = 0b00000000000000000000001111111111;
        const uint32_t output = cblt::core::mortonEncode(0, kInput, 0);
        ASSERT_EQ(output, kExpected);
    }{
        static constexpr uint32_t kExpected = 0b00100100100100100100100100100100;
        const constexpr uint32_t kInput = 0b00000000000000000000001111111111;
        const uint32_t output = cblt::core::mortonEncode(0, 0, kInput);
        ASSERT_EQ(output, kExpected);
    }
    {
        static constexpr uint32_t kExpected = 0b00111111111111111111111111111111;
        const constexpr uint32_t kInput = 0b00000000000000000000001111111111;
        const uint32_t output = cblt::core::mortonEncode(kInput, kInput, kInput);
        ASSERT_EQ(output, kExpected);
    }
}

TEST(CobaltCoreAlgorithmsTests, TestRadixSort) {
    {
        std::array<uint32_t, 4> input = {1, 4, 3, 7};
        auto keyer = [](const uint32_t &value) { 
            return value;
        };
        
        cblt::core::radix_sort(input.begin(), input.end(), keyer);

        static constexpr std::array<uint32_t, 4> kExpected = {1, 3, 4, 7};
        cblt::test::expectArraysEqual<uint32_t>(input, kExpected);
    }
    {
        std::vector<uint32_t> input = {16, 8, 4, 2, 1};
        auto keyer = [](const uint32_t &value) { 
            return value;
        };
        
        cblt::core::radix_sort(input.begin(), input.end(), keyer);

        static constexpr std::array<uint32_t, 5> kExpected = {1, 2, 4, 8, 16};
        cblt::test::expectArraysEqual<uint32_t>(input, kExpected);
    }
    {
        std::array<uint32_t, 5> input = {1, 2, 3, 4, 5};
        auto keyer = [](const uint32_t &value) { 
            return value;
        };
        
        cblt::core::radix_sort<1>(input.begin(), input.end(), keyer);

        static constexpr std::array<uint32_t, 5> kExpected = {2, 4, 1, 3, 5};
        cblt::test::expectArraysEqual<uint32_t>(input, kExpected);
    }
}