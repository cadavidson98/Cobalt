#include "geometry/bounding_box.h"
#include "geometry/bounding_volume.h"
#include "geometry/intersection.h"
#include "geometry/ray.h"
#include "geometry/sphere.h"
#include "geometry/triangle.h"

#include <chrono>
#include <gtest/gtest.h>
#include <iostream>
#include <vector>

namespace {
static constexpr float kEpsilon = 1e-4f;
}

TEST(CobaltCoreGeometryTests, TestBoundingBoxIntersect) {
    static const cblt::simd::vec3f origin(0.f, 0.f, 0.f);
    static const cblt::simd::vec3f xDir(1.f, 0.f, 0.f);
    static const cblt::geom::CoRay xRay(origin, xDir, 10.f);

    {
        // hit (in front)
        static const cblt::simd::vec3f boxMin(2.f, -1.f, -1.f);
        static const cblt::simd::vec3f boxMax(4.f, 1.f, 1.f);
        const cblt::geom::CoAxisAlignedBoundingBox aabb(boxMin, boxMax);

        cblt::geom::IntersectionEvent event;
        const bool hit = cblt::geom::rayAxisAlignedBoundingBoxIntersection(xRay, aabb, event);
        EXPECT_TRUE(hit);
        EXPECT_NEAR(event.timeMin, 2.f, kEpsilon);
        EXPECT_NEAR(event.timeMax, 4.f, kEpsilon);
    }
    {
        // hit (inside)
        static const cblt::simd::vec3f boxMin(-1.f, -1.f, -1.f);
        static const cblt::simd::vec3f boxMax(1.f, 1.f, 1.f);
        const cblt::geom::CoAxisAlignedBoundingBox aabb(boxMin, boxMax);

        cblt::geom::IntersectionEvent event;
        const bool hit = cblt::geom::rayAxisAlignedBoundingBoxIntersection(xRay, aabb, event);
        EXPECT_TRUE(hit);
        EXPECT_NEAR(event.timeMin, 1.f, kEpsilon);
    }
    {
        // miss (behind)
        static const cblt::simd::vec3f boxMin(-4.f, -1.f, -1.f);
        static const cblt::simd::vec3f boxMax(-2.f, 1.f, 1.f);
        const cblt::geom::CoAxisAlignedBoundingBox aabb(boxMin, boxMax);

        cblt::geom::IntersectionEvent event;
        const bool hit = cblt::geom::rayAxisAlignedBoundingBoxIntersection(xRay, aabb, event);
        EXPECT_FALSE(hit);
    }
    {
        // miss
        static const cblt::simd::vec3f boxMin(-1.f, 4.f, -1.f);
        static const cblt::simd::vec3f boxMax(1.f, 6.f, 1.f);
        const cblt::geom::CoAxisAlignedBoundingBox aabb(boxMin, boxMax);

        cblt::geom::IntersectionEvent event;
        const bool hit = cblt::geom::rayAxisAlignedBoundingBoxIntersection(xRay, aabb, event);
        EXPECT_FALSE(hit);
    }
    {
        // miss (beyond terminal dist)
        static const cblt::simd::vec3f boxMin(12.f, -1.f, -1.f);
        static const cblt::simd::vec3f boxMax(14.f, 1.f, 1.f);
        const cblt::geom::CoAxisAlignedBoundingBox aabb(boxMin, boxMax);

        cblt::geom::IntersectionEvent event;
        const bool hit = cblt::geom::rayAxisAlignedBoundingBoxIntersection(xRay, aabb, event);
        EXPECT_FALSE(hit);
    }
}

TEST(CobaltCoreGeometryTests, TestSphereIntersect) {
    {
        static const cblt::geom::CoSphere sphere{
            .center = cblt::simd::vec3f(0.f, 0.f, 0.f),
            .radius = 1.f,
        };

        static const cblt::geom::CoRay ray(cblt::simd::vec3f(0.f, 0.f, -5.f), cblt::simd::vec3f(0.f, 0.f, 1.f), 10.f);

        cblt::geom::IntersectionEvent event;
        EXPECT_TRUE(cblt::geom::raySphereIntersection(ray, sphere, event));
        EXPECT_EQ(event.timeMin, 4.f);
        EXPECT_EQ(event.timeMax, 6.f);
    }
    {
        static const cblt::geom::CoSphere sphere{
            .center = cblt::simd::vec3f(0.f, 0.f, 0.f),
            .radius = 4.f,
        };

        static const cblt::geom::CoRay ray(cblt::simd::vec3f(0.f, 0.f, 0.f), cblt::simd::vec3f(1.f, 0.f, 0.f), 10.f);

        cblt::geom::IntersectionEvent event;
        EXPECT_TRUE(cblt::geom::raySphereIntersection(ray, sphere, event));
        EXPECT_EQ(event.timeMin, -4.f);
        EXPECT_EQ(event.timeMax, 4.f);
    }
    {
        static const cblt::geom::CoSphere sphere{
            .center = cblt::simd::vec3f(5.f, 5.f, 5.f),
            .radius = 5.f,
        };

        static const cblt::geom::CoRay ray(cblt::simd::vec3f(0.f, 10.f, 0.f), cblt::simd::vec3f(0.f, -1.f, 0.f), 10.f);

        cblt::geom::IntersectionEvent event;
        EXPECT_FALSE(cblt::geom::raySphereIntersection(ray, sphere, event));
    }
    {
        static const cblt::geom::CoSphere sphere{
            .center = cblt::simd::vec3f(3.f, 0.f, 3.f),
            .radius = 3.f,
        };

        static const cblt::geom::CoRay ray(
            cblt::simd::vec3f(0.f, 0.f, 6.f),
            cblt::simd::vec3f(0.7071f, 0, -.7071f),
            10.f
        );

        cblt::geom::IntersectionEvent event;
        EXPECT_TRUE(cblt::geom::raySphereIntersection(ray, sphere, event));
        EXPECT_NEAR(event.timeMin, 1.24264f, 1e-4f);
        EXPECT_NEAR(event.timeMax, 7.24264f, 1e-4f);
    }
}

TEST(CobaltCoreGeometryTests, TestTriangleIntersect) {
    {
        static const cblt::geom::CoTriangle triangle{
            .position1 = cblt::simd::vec3f{0.f, 1.f, 1.f},
            .position2 = cblt::simd::vec3f{1.f, 0.f, 1.f},
            .position3 = cblt::simd::vec3f{-1.f, 0.f, 1.f},
        };

        static const cblt::geom::CoRay ray =
            cblt::geom::CoRay(cblt::simd::vec3f{0.f, .5f, 0.f}, cblt::simd::vec3f{0.f, 0.f, 1.f}, 10.f);

        cblt::geom::IntersectionEvent event;
        const bool hit = cblt::geom::rayTriangleIntersection(ray, triangle, event);
        EXPECT_TRUE(hit);
        EXPECT_NEAR(event.timeMin, 1.f, 1e-4f);
    }
}

TEST(CobaltCoreGeometryTests, TestBoundingBoxPerformance) {
    static const cblt::simd::vec3f origin(0.f, 0.f, 0.f);
    static const cblt::simd::vec3f xDir(1.f, 0.f, 0.f);
    static const cblt::geom::CoRay xRay(origin, xDir, 10.f);

    // make a bunch of BBoxes
    static const size_t numBoxes = 100000;
    std::vector<cblt::geom::CoAxisAlignedBoundingBox> boxes;
    boxes.reserve(numBoxes);
    for (size_t idx = 0; idx < numBoxes; ++idx) {
        cblt::simd::vec3f boxMin(idx, idx, idx);
        cblt::simd::vec3f boxMax(idx + 1, idx + 1, idx + 1);
        boxes.emplace_back(boxMin, boxMax);
    }

    auto s = std::chrono::high_resolution_clock::now();
    cblt::geom::IntersectionEvent event;
    for (const auto &box : boxes) {
        cblt::geom::rayAxisAlignedBoundingBoxIntersection(xRay, box, event);
    }
    auto e = std::chrono::high_resolution_clock::now();
    std::cout << "Time elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(e - s).count() << std::endl;
}

TEST(CobaltCoreGeometryTests, TestCreateBoundingVolume) {
    std::vector<cblt::geom::CoAxisAlignedBoundingBox> boxes;
    static const size_t numBoxes = 10;
    boxes.reserve(numBoxes);

    for (size_t idx = 0; idx < numBoxes; ++idx) {
        cblt::simd::vec3f boxMin(idx, idx, idx);
        cblt::simd::vec3f boxMax(idx + 1, idx + 1, idx + 1);
        boxes.emplace_back(boxMin, boxMax);
    }

    cblt::geom::CoBoundingVolume::CreateWithBoundingBoxesInfo createInfo{
        .boxes = boxes,
        .maxPrimsInLeaf = 1,
        .partitionMethod = cblt::geom::CoBoundingVolume::PartitionMethod::Midpoint,
    };

    cblt::geom::CoBoundingVolume boundingVolume(createInfo);

    {
        cblt::geom::CoRay hitRay({5.f, 5.f, 0.f}, {0.f, 0.f, 1.f}, 10.f);
        EXPECT_TRUE(boundingVolume.IntersectClosest(hitRay));
    }
}
