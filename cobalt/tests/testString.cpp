#include "string_utilities.h"

#include <gtest/gtest.h>

#include <vector>

namespace {
    constexpr float kEpsilon = 1e-4f;
}

TEST(CobaltCoreStringTests, TestSplit) {
    {
        std::string_view testString = "abc def ghi";
        std::vector<std::string> strings = cblt::core::split(testString, ' ');
        EXPECT_EQ(strings.size(), size_t(3));
        EXPECT_EQ(strings[0], "abc");
        EXPECT_EQ(strings[1], "def");
        EXPECT_EQ(strings[2], "ghi");
    }
    {
        std::string_view testString = "1/2/3 4//5 6/7/";
        std::vector<std::string> groups = cblt::core::split<std::string>(testString, ' ');
        EXPECT_EQ(groups.size(), 3);
        EXPECT_EQ(groups[0], "1/2/3");
        EXPECT_EQ(groups[1], "4//5");
        EXPECT_EQ(groups[2], "6/7/");
        {
            std::vector<uint32_t> ints = cblt::core::split<uint32_t>(groups[0], '/');
            EXPECT_EQ(ints.size(), 3);
            EXPECT_EQ(ints[0], 1);
            EXPECT_EQ(ints[1], 2);
            EXPECT_EQ(ints[2], 3);
        }
        {
            std::vector<uint32_t> ints = cblt::core::split<uint32_t>(groups[1], '/');
            EXPECT_EQ(ints.size(), 2);
            EXPECT_EQ(ints[0], 4);
            EXPECT_EQ(ints[1], 5);
        }
        {
            std::vector<uint32_t> ints = cblt::core::split<uint32_t>(groups[2], '/');
            EXPECT_EQ(ints.size(), 2);
            EXPECT_EQ(ints[0], 6);
            EXPECT_EQ(ints[1], 7);
        }
    }
    {
        std::string_view testString = "1.5,3.4,7.4,10.3,";
        std::vector<float> floats = cblt::core::split<float>(testString, ',');
        EXPECT_EQ(floats.size(), 4);
        EXPECT_NEAR(floats[0], 1.5f, kEpsilon); 
        EXPECT_NEAR(floats[1], 3.4f, kEpsilon); 
        EXPECT_NEAR(floats[2], 7.4f, kEpsilon); 
        EXPECT_NEAR(floats[3], 10.3f, kEpsilon); 
    }
}

TEST(CobaltCoreStringTests, TestFileExtension) {
    {
        std::string_view objFile = "test.obj";
        std::string extension = cblt::core::fileExtension(objFile);
        EXPECT_EQ(extension, "obj");
    }
    {
        std::string_view pngFile = "/home/users/files/image.png";
        std::string extension = cblt::core::fileExtension(pngFile);
        EXPECT_EQ(extension, "png");
    }
    {
        std::string_view tarFile = "/tmp/data.tar.gz";
        std::string extension = cblt::core::fileExtension(tarFile);
        EXPECT_EQ(extension, "gz");
    }
    {
        std::string_view noFile = "test_no";
        std::string extension = cblt::core::fileExtension(noFile);
        EXPECT_EQ(extension, "");
    }
}