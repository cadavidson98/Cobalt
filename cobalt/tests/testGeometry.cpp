#include "core/morton_encoding.h"

#include "geometry/bounding_box.h"
#include "geometry/bounding_volume.h"
#include "geometry/bounding_volume_crtp.h"
#include "geometry/bounding_volume_types.h"
#include "geometry/intersection.h"
#include "geometry/ray.h"
#include "geometry/sphere.h"
#include "geometry/triangle.h"
#include "math/simd/simd_vec3.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>

static constexpr float kEpsilon = 1e-4f;

namespace cblt::geom::crtp {

template<>
struct primitive_types<class CoStorageMock> {
    static constexpr PrimitiveTypes value = (PrimitiveType::kBox | PrimitiveType::kSphere);
};

class CoStorageMock : public CoPrimitiveStorageBase<CoStorageMock> {
public:
    CoStorageMock(size_t gridX, size_t gridY) : gridBounds{
        .min = simd::vec3f(-1.f, -1.f, -1.f),
        .max = simd::vec3f(gridX, gridY, 1.f)
    } {
        bool flip = true;
        boxPrimitives.reserve(gridX * gridY);
        for (uint32_t y = 0; y < gridY; ++y) {
            for (uint32_t x = 0; x < gridX; ++x) {
                if (flip) {
                    boxPrimitives.emplace_back(simd::vec3f(x - .5f, y - .5f, -.5f), simd::vec3f(x + .5f, y + .5f, +.5f));
                } else {
                    spherePrimitives.emplace_back(simd::vec3f(float(x), float(y), 0.f), .5f);
                }
                flip = !flip;
            }
        }
    }

    geom::CoAxisAlignedBoundingBox bounds() const {
        return gridBounds;
    }

    void reorder(std::span<MortonPrimitive> primitives) {
        uint32_t boxIdx = 0;
        uint32_t sphereIdx = 0;

        std::vector<CoAxisAlignedBoundingBox> boxesCopy = boxPrimitives;
        std::vector<CoSphere> spheresCopy = spherePrimitives;

        for (MortonPrimitive &mortonPrimitive : primitives) {
            switch (mortonPrimitive.primitive.type) {
            case PrimitiveType::kBox : {
                const size_t newIdx = boxIdx++;
                boxPrimitives[newIdx] = boxesCopy[mortonPrimitive.primitive.index];
                mortonPrimitive.primitive.index = newIdx;
                continue;
            }
            case PrimitiveType::kSphere : {
                const size_t newIdx = sphereIdx++;
                spherePrimitives[newIdx] = spheresCopy[mortonPrimitive.primitive.index];
                mortonPrimitive.primitive.index = newIdx;
                continue;
            }
            case PrimitiveType::kMesh : [[fallthrough]];
            case PrimitiveType::kPatch : [[fallthrough]];
            case PrimitiveType::kTriangle : [[fallthrough]];
            default : ASSERT_TRUE(false);
            }
        }
    }

    std::vector<MortonPrimitive> mortonEncodePrimitives() const {
        const CoAxisAlignedBoundingBox primitiveBounds = bounds();
        const simd::vec3f boundsExtent = primitiveBounds.Scales();
        const simd::vec3f boundsMin = primitiveBounds.min;
        std::vector<Primitive> storagePrimitives = primitivesAll();
        auto encodePrimitive = [&boundsExtent, &boundsMin](const Primitive &primitive) {
            static constexpr float kFloatToUint = float((1 << 10) - 1);
            const simd::vec3f normalizedPosition = (primitive.boundingBox.Center() - boundsMin) / boundsExtent;
            const std::array<float, 4> values = (kFloatToUint * normalizedPosition).Values();
            return MortonPrimitive{
                .mortonCode = core::mortonEncode(values[0], values[1], values[2]),
                .primitive = primitive,
            };
        };

        std::vector<MortonPrimitive> mortonEncodedPrimitives(storagePrimitives.size());
        std::transform(
            storagePrimitives.begin(),
            storagePrimitives.end(),
            mortonEncodedPrimitives.begin(),
            encodePrimitive
        );

        return mortonEncodedPrimitives;
    }

    std::span<const CoSphere> spheres(size_t start, size_t count) const {
        return {spherePrimitives.begin() + start, spherePrimitives.begin() + (start + count) };

        static_assert(primitive_types<CoStorageMock>::value & kSphere, "not supported");
    }

    std::span<const CoAxisAlignedBoundingBox> boxes(size_t start, size_t count) const {
        return {boxPrimitives.begin() + start, boxPrimitives.begin() + (start + count) };

        static_assert(primitive_types<CoStorageMock>::value != kNone, "not supported");
    }

private:
    const CoAxisAlignedBoundingBox gridBounds;

    std::vector<cblt::geom::CoAxisAlignedBoundingBox> boxPrimitives;
    std::vector<cblt::geom::CoSphere> spherePrimitives;

    std::vector<Primitive> primitivesAll() const {
        std::vector<Primitive> prims;
        for (size_t index = 0; index < boxPrimitives.size(); ++index) {
            const CoAxisAlignedBoundingBox &box = boxPrimitives[index];
            prims.push_back(Primitive{
                .type = PrimitiveType::kBox,
                .boundingBox = box,
                .index = uint32_t(index),
            });
        }

        for (size_t index = 0; index < spherePrimitives.size(); ++index) {
            const CoSphere &sphere = spherePrimitives[index];
            const CoAxisAlignedBoundingBox boundingBox = {
                .min = sphere.center - simd::vec3f(sphere.radius),
                .max = sphere.center + simd::vec3f(sphere.radius),
            };

            prims.push_back(Primitive{
                .type = PrimitiveType::kSphere,
                .boundingBox = boundingBox,
                .index = uint32_t(index),
            });
        }

        return prims;
    }
};

} // namespace cblt::geom::crtp

namespace cblt::geom::test {
struct BoxStorage {
    std::vector<CoAxisAlignedBoundingBox> boxes;
    BoxStorage(size_t gridX, size_t gridY) {
        boxes.reserve(gridX * gridY);

        for (uint32_t y = 0; y < gridY; ++y) {
            for (uint32_t x = 0; x < gridX; ++x) {
                simd::vec3f boxMin(x - 1, y - 1, -1);
                simd::vec3f boxMax(x + 1, y + 1, +1);
                boxes.emplace_back(boxMin, boxMax);
            }
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

class CobaltGeometryTest : public ::testing::Test {
protected:
    CobaltGeometryTest() {
        storage = std::make_shared<cblt::geom::crtp::CoStorageMock>(kGridSizeX, kGridSizeY);
        old_storage = std::make_shared<cblt::geom::test::BoxStorage>(kGridSizeX, kGridSizeY);
    }

    std::shared_ptr<cblt::geom::crtp::CoStorageMock> storage;
    std::shared_ptr<cblt::geom::test::BoxStorage> old_storage;

    static constexpr size_t kGridSizeX = 512;
    static constexpr size_t kGridSizeY = 512;
};

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
        // miss
        static const cblt::simd::vec3f boxMin(-.5f, -.5f, -.5f);
        static const cblt::simd::vec3f boxMax(.5f, .5f, .5f);
        static const cblt::geom::CoRay zRay(cblt::simd::vec3f(2.f, 0.f, 4.f), cblt::simd::vec3f(0.f, 0.f, -1.f), 10.f);
        const cblt::geom::CoAxisAlignedBoundingBox aabb(boxMin, boxMax);

        float timeMin, timeMax;
        const bool hit = cblt::geom::rayAxisAlignedBoundingBoxIntersection(zRay, aabb, timeMin, timeMax);
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

        float timeMin, timeMax;
        EXPECT_TRUE(cblt::geom::raySphereIntersection(ray, sphere, timeMin, timeMax));
        EXPECT_EQ(timeMin, 4.f);
        EXPECT_EQ(timeMax, 6.f);
    }
    {
        static const cblt::geom::CoSphere sphere{
            .center = cblt::simd::vec3f(0.f, 0.f, 0.f),
            .radius = 4.f,
        };

        static const cblt::geom::CoRay ray(cblt::simd::vec3f(0.f, 0.f, 0.f), cblt::simd::vec3f(1.f, 0.f, 0.f), 10.f);

        float timeMin, timeMax;
        EXPECT_TRUE(cblt::geom::raySphereIntersection(ray, sphere, timeMin, timeMax));
        EXPECT_EQ(timeMin, -4.f);
        EXPECT_EQ(timeMax, 4.f);
    }
    {
        static const cblt::geom::CoSphere sphere{
            .center = cblt::simd::vec3f(5.f, 5.f, 5.f),
            .radius = 5.f,
        };

        static const cblt::geom::CoRay ray(cblt::simd::vec3f(0.f, 10.f, 0.f), cblt::simd::vec3f(0.f, -1.f, 0.f), 10.f);

        float timeMin, timeMax;
        EXPECT_FALSE(cblt::geom::raySphereIntersection(ray, sphere, timeMin, timeMax));
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

        float timeMin, timeMax;
        EXPECT_TRUE(cblt::geom::raySphereIntersection(ray, sphere, timeMin, timeMax));
        EXPECT_NEAR(timeMin, 1.24264f, 1e-4f);
        EXPECT_NEAR(timeMax, 7.24264f, 1e-4f);
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

TEST_F(CobaltGeometryTest, TestBoundingBoxPerformance) {
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

    cblt::geom::IntersectionEvent event;
    float timeMin, timeMax;
    for (const auto &box : boxes) {
        cblt::geom::rayAxisAlignedBoundingBoxIntersection(xRay, box, timeMin, timeMax);
    }
}

TEST_F(CobaltGeometryTest, TestCreateBoundingVolume) {

    using BoxAccelerator = cblt::geom::CoBoundingVolume<cblt::geom::test::BoxStorage>;
    
    BoxAccelerator::CreateWithPrimitivesInfo createInfo{
        .primitives = old_storage,
        .maxPrimsInLeaf = 1,
        .partitionMethod = BoxAccelerator::PartitionMethod::Midpoint,
    };
    
    BoxAccelerator boundingVolume(createInfo);

    for (size_t y = 0; y < kGridSizeY; ++y) {
        for (size_t x = 0; x < kGridSizeX; ++x) {
            cblt::geom::IntersectionEvent event;
            const cblt::geom::CoRay ray(cblt::simd::vec3f(x, y, 4.f), cblt::simd::vec3f(0.f, 0.f, -1.f), 10.f);
            ASSERT_TRUE(boundingVolume.IntersectClosest(ray, event)) << "missed box:" << x << ' ' << y;
            ASSERT_NEAR(event.timeMin, 3.f, kEpsilon);
        }
    }
}

TEST_F(CobaltGeometryTest, TestCtrp) {
    using namespace cblt::geom::crtp;
    CoBoundingVolume<CoStorageMock> boundingVolume({
        .primitives = storage,
    });

    for (size_t y = 0; y < kGridSizeY; ++y) {
        for (size_t x = 0; x < kGridSizeX; ++x) {
            cblt::geom::IntersectionEvent event;
            const cblt::geom::CoRay ray(cblt::simd::vec3f(x, y, 4.f), cblt::simd::vec3f(0.f, 0.f, -1.f), 10.f);
            const cblt::geom::crtp::IntersectionResult result = boundingVolume.intersects(ray);
            ASSERT_NEAR(result.hitTime, 3.5f, kEpsilon);
            // FIXME: Need to "remap" the index from the original grid values to the reordered indices in order to check
            // what box / sphere we hit
            if ((x & 1) == 1) {
                ASSERT_EQ(result.primitive.type, cblt::geom::crtp::kSphere);
            } else {
                ASSERT_EQ(result.primitive.type, cblt::geom::crtp::kBox);
            }
        }
    }
}
