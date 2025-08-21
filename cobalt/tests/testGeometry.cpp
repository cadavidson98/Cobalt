#include "geometry/bounding_box.h"
#include "geometry/bounding_volume.h"
#include "geometry/bounding_volume_crtp.h"
#include "geometry/bounding_volume_mesh_storage.h"
#include "geometry/bounding_volume_scene_storage.h"
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
#include <sstream>
#include <vector>

static constexpr float kEpsilon = 1e-4f;

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

std::shared_ptr<crtp::CoMeshStorage> makeCubeMesh(size_t width) {
    assert((width & 1) == 0);

    const size_t verticesPerRow = width + 1;

    const size_t vertexCount = verticesPerRow * verticesPerRow;
    const size_t triangleCount = width * width;
    const size_t patchCount = (width * width) / 2;

    crtp::CoMeshStorage::VertexBuffer buffer = {
        .positions = std::make_shared<simd::vec3f[]>(vertexCount),
        .positionCount = vertexCount,
        .triangleIndices = std::make_shared<vec3u[]>(triangleCount),
        .triangleCount = triangleCount,
        .patchIndices = std::make_shared<vec4u[]>(patchCount),
        .patchCount = patchCount,
    };

    for (size_t y = 0; y < verticesPerRow; ++y) {
        for (size_t x = 0; x < verticesPerRow; ++x) {
            buffer.positions[y * verticesPerRow + x] = simd::vec3f(x, y, 0);
        }
    }

    size_t triangleIdx = 0;
    for (size_t y = 0; y < width; ++y) {
        const size_t offset = (y & 1);
        for (size_t x = 0; x < (width / 2); ++x) {
            const size_t idx = y * verticesPerRow + (2 * x + offset);
            const size_t nextIdx = idx + 1;
            buffer.triangleIndices[triangleIdx++] = vec3u(idx, nextIdx, idx + verticesPerRow);
            buffer.triangleIndices[triangleIdx++] = vec3u(nextIdx, nextIdx + verticesPerRow, idx + verticesPerRow);
        }
    }

    size_t patchIdx = 0;
    for (size_t y = 0; y < width; ++y) {
        const size_t offset = !(y & 1);
        for (size_t x = 0; x < (width / 2); ++x) {
            const size_t idx = y * verticesPerRow + (2 * x + offset);
            const size_t nextIdx = idx + 1;
            buffer.patchIndices[patchIdx++] = vec4u(idx, nextIdx, nextIdx + verticesPerRow, idx + verticesPerRow);
        }
    }

    return std::make_shared<crtp::CoMeshStorage>(std::move(buffer));
}

std::shared_ptr<crtp::CoSceneStorage> makeScene(size_t gridSizeX, size_t gridSizeY) {
    std::shared_ptr<crtp::CoSceneStorage> scene = std::make_shared<cblt::geom::crtp::CoSceneStorage>();

    bool flip = true;
    for (uint32_t y = 0; y < gridSizeY; ++y) {
        for (uint32_t x = 0; x < gridSizeX; ++x) {
            if (flip) {
                scene->addBox({
                    .min = cblt::simd::vec3f(x - .5f, y - .5f, -.5f),
                    .max = cblt::simd::vec3f(x + .5f, y + .5f, +.5f),
                });
            } else {
                scene->addSphere({
                    .center = cblt::simd::vec3f(float(x), float(y), 0.f),
                    .radius = .5f,
                });
            }
            flip = !flip;
        }
    }

    return scene;
}

} // namespace cblt::geom::test

class CobaltGeometryTest : public ::testing::Test {
protected:
    CobaltGeometryTest() {
        storage = cblt::geom::test::makeScene(kGridSizeX, kGridSizeY);
        meshStorage = cblt::geom::test::makeCubeMesh(kCubeSize);
        old_storage = std::make_shared<cblt::geom::test::BoxStorage>(kGridSizeX, kGridSizeY);
    }

    std::shared_ptr<cblt::geom::crtp::CoSceneStorage> storage;
    std::shared_ptr<cblt::geom::crtp::CoMeshStorage> meshStorage;
    std::shared_ptr<cblt::geom::test::BoxStorage> old_storage;

    static constexpr size_t kGridSizeX = 512;
    static constexpr size_t kGridSizeY = 512;

    static constexpr size_t kCubeSize = 8;
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

TEST_F(CobaltGeometryTest, TestCreateMeshStorage) {
    using namespace cblt::geom::crtp;
    CoBoundingVolume<CoMeshStorage> boundingVolume({
        .primitives = meshStorage,
    });

    for (size_t y = 0; y < kCubeSize; ++y) {
        const size_t offset = (y & 1);
        for (size_t x = 0; x < kCubeSize; ++x) {
            cblt::geom::IntersectionEvent event;
            const cblt::geom::CoRay ray(
                cblt::simd::vec3f(x + .25f, y + .25f, 4.f),
                cblt::simd::vec3f(0.f, 0.f, -1.f),
                10.f
            );
            const cblt::geom::crtp::IntersectionResult result = boundingVolume.intersects(ray);
            std::ostringstream testDescription;
            testDescription << "Mesh tile is: (" << x << ", " << y << ") of " << kCubeSize;
            ASSERT_NEAR(result.hitTime, 4.f, kEpsilon) << testDescription.view();
            // FIXME: Need to "remap" the index from the original grid values to the reordered indices in order to check
            // what box / sphere we hit
            if (((x + offset) & 1) == 1) {
                EXPECT_EQ(result.primitive.type, cblt::geom::crtp::kPatch) << testDescription.view();
            } else {
                EXPECT_EQ(result.primitive.type, cblt::geom::crtp::kTriangle) << testDescription.view();
            }
        }
    }
}

TEST_F(CobaltGeometryTest, TestCtrp) {
    using namespace cblt::geom::crtp;
    CoBoundingVolume<CoSceneStorage> boundingVolume({
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
