#include "bounding_box.h"
#include "bounding_volume.h"
#include "ray.h"

#include <chrono>
#include <gtest/gtest.h>
#include <iostream>
#include <vector>

TEST(CobaltCoreGeometryTests, TestBoundingBoxIntersect) {
    static const cblt::simd::vec4f origin(0.f, 0.f, 0.f, 1.f);
    static const cblt::simd::vec4f xDir(1.f, 0.f, 0.f, 0.f);
    static const cblt::geom::CoRay xRay(origin, xDir, 10.f);

    {
        // hit (in front)
        static const cblt::simd::vec4f boxMin(2.f, -1.f, -1.f, 1.f);
        static const cblt::simd::vec4f boxMax(4.f, 1.f, 1.f, 1.f);
        const cblt::geom::CoAxisAlignedBoundingBox aabb(boxMin, boxMax);

        float timeMin, timeMax;
        const bool hit = cblt::geom::CoAxisAlignedBoundingBox::intersect(xRay, aabb, timeMin, timeMax);
        EXPECT_TRUE(hit);
        EXPECT_EQ(timeMin, 2.f);
        EXPECT_EQ(timeMax, 4.f);
    }
    {
        // hit (inside)
        static const cblt::simd::vec4f boxMin(-1.f, -1.f, -1.f, 1.f);
        static const cblt::simd::vec4f boxMax(1.f, 1.f, 1.f, 1.f);
        const cblt::geom::CoAxisAlignedBoundingBox aabb(boxMin, boxMax);

        float timeMin, timeMax;
        const bool hit = cblt::geom::CoAxisAlignedBoundingBox::intersect(xRay, aabb, timeMin, timeMax);
        EXPECT_TRUE(hit);
        EXPECT_EQ(timeMin, 1.f);
    }
    {
        // miss (behind)
        static const cblt::simd::vec4f boxMin(-4.f, -1.f, -1.f, 1.f);
        static const cblt::simd::vec4f boxMax(-2.f, 1.f, 1.f, 1.f);
        const cblt::geom::CoAxisAlignedBoundingBox aabb(boxMin, boxMax);

        float timeMin, timeMax;
        const bool hit = cblt::geom::CoAxisAlignedBoundingBox::intersect(xRay, aabb, timeMin, timeMax);
        EXPECT_FALSE(hit);
    }
    {
        // miss
        static const cblt::simd::vec4f boxMin(-1.f, 4.f, -1.f, 1.f);
        static const cblt::simd::vec4f boxMax(1.f, 6.f, 1.f, 1.f);
        const cblt::geom::CoAxisAlignedBoundingBox aabb(boxMin, boxMax);

        float timeMin, timeMax;
        const bool hit = cblt::geom::CoAxisAlignedBoundingBox::intersect(xRay, aabb, timeMin, timeMax);
        EXPECT_FALSE(hit);
    }
    {
        // miss (beyond terminal dist)
        static const cblt::simd::vec4f boxMin(12.f, -1.f, -1.f, 1.f);
        static const cblt::simd::vec4f boxMax(14.f, 1.f, 1.f, 1.f);
        const cblt::geom::CoAxisAlignedBoundingBox aabb(boxMin, boxMax);

        float timeMin, timeMax;
        const bool hit = cblt::geom::CoAxisAlignedBoundingBox::intersect(xRay, aabb, timeMin, timeMax);
        EXPECT_FALSE(hit);
    }
}

TEST(CobaltCoreGeometryTests, TestBoundingBoxPerformance) {
    static const cblt::simd::vec4f origin(0.f, 0.f, 0.f, 1.f);
    static const cblt::simd::vec4f xDir(1.f, 0.f, 0.f, 0.f);
    static const cblt::geom::CoRay xRay(origin, xDir, 10.f);

    // make a bunch of BBoxes
    static const size_t numBoxes = 100000;
    std::vector<cblt::geom::CoAxisAlignedBoundingBox> boxes;
    boxes.reserve(numBoxes);
    for (size_t idx = 0; idx < numBoxes; ++idx) {
        cblt::simd::vec4f boxMin(idx, idx, idx, 1);
        cblt::simd::vec4f boxMax(idx + 1, idx + 1, idx + 1, 1);
        boxes.emplace_back(boxMin, boxMax);
    }

    auto s = std::chrono::high_resolution_clock::now();
    float timeMin, timeMax;
    for (const auto &box : boxes) {
        cblt::geom::CoAxisAlignedBoundingBox::intersect(xRay, box, timeMin, timeMax);
    }
    auto e = std::chrono::high_resolution_clock::now();
    std::cout << "Time elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(e - s).count() << std::endl;
}

TEST(CobaltCoreGeometryTests, TestCreateBoundingVolume) {
    std::vector<cblt::geom::CoAxisAlignedBoundingBox> boxes;
    static const size_t numBoxes = 10;
    boxes.reserve(numBoxes);

    for (size_t idx = 0; idx < numBoxes; ++idx) {
        cblt::simd::vec4f boxMin(idx, idx, idx, 1);
        cblt::simd::vec4f boxMax(idx + 1, idx + 1, idx + 1, 1);
        boxes.emplace_back(boxMin, boxMax);
    }

    cblt::geom::CoBoundingVolume::CreateWithBoundingBoxesInfo createInfo{
        .boxes = boxes,
        .maxPrimsInLeaf = 1,
        .partitionMethod = cblt::geom::CoBoundingVolume::PartitionMethod::Midpoint,
    };

    cblt::geom::CoBoundingVolume boundingVolume(createInfo);

    {
        cblt::geom::CoRay hitRay({5.f, 5.f, 0.f, 1.f}, {0.f, 0.f, 1.f, 0.f}, 10.f);
        EXPECT_TRUE(boundingVolume.IntersectClosest(hitRay));
    }
}
