#include "geometry/bounding_box.h"
#include "geometry/bounding_volume.h"
#include "geometry/bounding_volume_crtp.h"
#include "geometry/intersection.h"
#include "geometry/ray.h"
#include "geometry/sphere.h"
#include "geometry/triangle.h"
#include "math/simd/simd_vec3.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

static constexpr float kEpsilon = 1e-4f;

namespace cblt::geom::crtp {

class CoStorageMock : public CoPrimitiveStorageBase<CoStorageMock> {
    public:
    CoStorageMock() {
        for (int y = 0; y < 1024; ++y) {
            for (int x = 0; x < 1024; ++x) {
                boxes.push_back(cblt::geom::CoAxisAlignedBoundingBox{
                    .min = simd::vec3f(x - 1.f, y - 1.f, -1.f),
                    .max = simd::vec3f(x + 1.f, y + 1.f, +1.f),
                });
            }
        }
    }
    
    geom::CoAxisAlignedBoundingBox bounds() const {
        return geom::CoAxisAlignedBoundingBox{
            .min = boxes.front().min,
            .max = boxes.back().max,
        };
    }
    
    void reorder(std::span<const MortonPrimitive> primitives) {
        const auto scratch = boxes;
        uint32_t idx = 0;
        for (const auto &mortonPrimitive : primitives) {
            boxes[idx] = scratch[mortonPrimitive.primitive.index];
        }
    }
    
    std::vector<Primitive> primitives() const {
        std::vector<Primitive> prims;
        for (size_t index = 0; index < boxes.size(); ++index) {
            const auto &box = boxes[index];
            prims.push_back(Primitive{
                .type = PrimitiveType::kBox,
                .boundingBox = box,
                .index = uint32_t(index),
            });
        }
        return prims;
    }
    
    private:
    std::vector<cblt::geom::CoAxisAlignedBoundingBox> boxes;
};

template<>
struct primitive_types<CoStorageMock> {
    static const PrimitiveTypes value = PrimitiveType::kBox;
};

}  // namespace cblt::geom::crtp

namespace cblt::geom::test {
struct BoxStorage {
    std::vector<CoAxisAlignedBoundingBox> boxes;
    BoxStorage() {
        static const size_t numBoxes = 10;
        boxes.reserve(numBoxes);

        for (size_t idx = 0; idx < numBoxes; ++idx) {
            simd::vec3f boxMin(idx, idx, idx);
            simd::vec3f boxMax(idx + 1, idx + 1, idx + 1);
            boxes.emplace_back(boxMin, boxMax);
        }
    }

    size_t NumPrimitives() const {
        return boxes.size();
    }

    CoAxisAlignedBoundingBox PrimitiveBounds(size_t startIdx, size_t endIdx) const {
        static constexpr float minFloat = std::numeric_limits<float>::lowest();
        static constexpr float maxFloat = std::numeric_limits<float>::max();
        CoAxisAlignedBoundingBox regionBounds = {
            .min = simd::vec3f(maxFloat, maxFloat, maxFloat),
            .max = simd::vec3f(minFloat, minFloat, minFloat),
        };

        for (size_t idx = startIdx; idx < endIdx; ++idx) {
            regionBounds.min = simd::min(regionBounds.min, boxes[idx].min);
            regionBounds.max = simd::max(regionBounds.max, boxes[idx].max);
        }

        return regionBounds;
    }

    size_t Reorder(size_t startIdx, size_t endIdx, std::function<bool(const CoAxisAlignedBoundingBox &)> comparator) {
        auto start = boxes.begin() + startIdx;
        auto end = boxes.begin() + endIdx;
        auto split = std::partition(start, end, comparator);
        return startIdx + std::distance(start, split);
    }

    bool PrimitivesIntersect(const CoRay &ray, size_t startIdx, size_t endIdx, IntersectionEvent &event) const {
        bool hit = false;
        for (size_t idx = startIdx; idx < endIdx; ++idx) {
            hit = rayAxisAlignedBoundingBoxIntersection(ray, boxes[idx], event.timeMin, event.timeMax) || hit;
        }
        return hit;
    }
};
} // namespace cblt::geom::test

TEST(CobaltGeometryTests, TestBoundingBoxIntersect) {
    static const cblt::simd::vec3f origin(0.f, 0.f, 0.f);
    static const cblt::simd::vec3f xDir(1.f, 0.f, 0.f);
    static const cblt::geom::CoRay xRay(origin, xDir, 10.f);

    {
        // hit (in front)
        static const cblt::simd::vec3f boxMin(2.f, -1.f, -1.f);
        static const cblt::simd::vec3f boxMax(4.f, 1.f, 1.f);
        const cblt::geom::CoAxisAlignedBoundingBox aabb(boxMin, boxMax);

        float timeMin, timeMax;
        const bool hit = cblt::geom::rayAxisAlignedBoundingBoxIntersection(xRay, aabb, timeMin, timeMax);
        EXPECT_TRUE(hit);
        EXPECT_NEAR(timeMin, 2.f, kEpsilon);
        EXPECT_NEAR(timeMax, 4.f, kEpsilon);
    }
    {
        // hit (inside)
        static const cblt::simd::vec3f boxMin(-1.f, -1.f, -1.f);
        static const cblt::simd::vec3f boxMax(1.f, 1.f, 1.f);
        const cblt::geom::CoAxisAlignedBoundingBox aabb(boxMin, boxMax);

        float timeMin, timeMax;
        const bool hit = cblt::geom::rayAxisAlignedBoundingBoxIntersection(xRay, aabb, timeMin, timeMax);
        EXPECT_TRUE(hit);
        EXPECT_NEAR(timeMin, 1.f, kEpsilon);
    }
    {
        // miss (behind)
        static const cblt::simd::vec3f boxMin(-4.f, -1.f, -1.f);
        static const cblt::simd::vec3f boxMax(-2.f, 1.f, 1.f);
        const cblt::geom::CoAxisAlignedBoundingBox aabb(boxMin, boxMax);

        float timeMin, timeMax;
        const bool hit = cblt::geom::rayAxisAlignedBoundingBoxIntersection(xRay, aabb, timeMin, timeMax);
        EXPECT_FALSE(hit);
    }
    {
        // miss
        static const cblt::simd::vec3f boxMin(-3.f, 4.f, -1.f);
        static const cblt::simd::vec3f boxMax(-1.f, 6.f, 1.f);
        const cblt::geom::CoAxisAlignedBoundingBox aabb(boxMin, boxMax);

        float timeMin, timeMax;
        const bool hit = cblt::geom::rayAxisAlignedBoundingBoxIntersection(xRay, aabb, timeMin, timeMax);
        EXPECT_FALSE(hit);
    }
    {
        // miss (beyond terminal dist)
        static const cblt::simd::vec3f boxMin(12.f, -1.f, -1.f);
        static const cblt::simd::vec3f boxMax(14.f, 1.f, 1.f);
        const cblt::geom::CoAxisAlignedBoundingBox aabb(boxMin, boxMax);

        float timeMin, timeMax;
        const bool hit = cblt::geom::rayAxisAlignedBoundingBoxIntersection(xRay, aabb, timeMin, timeMax);
        EXPECT_FALSE(hit);
    }
}

TEST(CobaltGeometryTests, TestSphereIntersect) {
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

TEST(CobaltGeometryTests, TestTriangleIntersect) {
    {
        static const cblt::geom::CoTriangle triangle{
            .position1 = cblt::simd::vec3f{ 0.f, 1.f, 1.f},
            .position2 = cblt::simd::vec3f{ 1.f, 0.f, 1.f},
            .position3 = cblt::simd::vec3f{-1.f, 0.f, 1.f},
        };

        static const cblt::geom::CoRay ray =
            cblt::geom::CoRay(cblt::simd::vec3f{0.f, .5f, 0.f}, cblt::simd::vec3f{0.f, 0.f, 1.f}, 10.f);

        float hitTime;
        cblt::vec2f hitCoordinates;
        const bool hit = cblt::geom::rayTriangleIntersection(
            ray,
            triangle.position1,
            triangle.position2,
            triangle.position3,
            hitTime,
            hitCoordinates
        );
        EXPECT_TRUE(hit);
        EXPECT_NEAR(hitTime, 1.f, 1e-4f);
    }
}

TEST(CobaltGeometryTests, TestBoundingBoxPerformance) {
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
    float timeMin, timeMax;
    for (const auto &box : boxes) {
        cblt::geom::rayAxisAlignedBoundingBoxIntersection(xRay, box, timeMin, timeMax);
    }
    auto e = std::chrono::high_resolution_clock::now();
    std::cout << "Time elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(e - s).count() << std::endl;
}

TEST(CobaltGeometryTests, TestCreateBoundingVolume) {

    using BoxAccelerator = cblt::geom::CoBoundingVolume<cblt::geom::test::BoxStorage>;

    std::shared_ptr<cblt::geom::test::BoxStorage> primitives =
        std::shared_ptr<cblt::geom::test::BoxStorage>(new cblt::geom::test::BoxStorage);

    BoxAccelerator::CreateWithPrimitivesInfo createInfo{
        .primitives = primitives,
        .maxPrimsInLeaf = 1,
        .partitionMethod = BoxAccelerator::PartitionMethod::Midpoint,
    };

    BoxAccelerator boundingVolume(createInfo);

    {
        cblt::geom::IntersectionEvent event;
        cblt::geom::CoRay hitRay({5.f, 5.f, 0.f}, {0.f, 0.f, 1.f}, 10.f);
        EXPECT_TRUE(boundingVolume.IntersectClosest(hitRay, event));
    }
}

TEST(CobaltGeometryTests, TestCtrp) {
    using namespace cblt::geom::crtp;
    std::shared_ptr<CoStorageMock> storage = std::make_shared<CoStorageMock>();
    CoBoundingVolume<CoStorageMock> bvh({
        .primitives = storage,
    });
}
