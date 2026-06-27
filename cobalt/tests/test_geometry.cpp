#include "core/vertex_buffer.h"
#include "geometry/bounding_box.h"
#include "geometry/bounding_volume.h"
#include "geometry/bounding_volume_mesh_storage.h"
#include "geometry/bounding_volume_scene_storage.h"
#include "geometry/bounding_volume_types.h"
#include "geometry/intersection.h"
#include "geometry/mesh.h"
#include "geometry/ray.h"
#include "geometry/sphere.h"
#include "geometry/triangle.h"
#include "math/simd/simd_vec3.h"
#include "math/vec3.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <sstream>
#include <vector>

static constexpr float kEpsilon = 1e-4f;

namespace cobalt::geom::test {

core::VertexAttributeBuffer<simd::vec3f> makeCubeMesh(size_t width) {
    assert((width & 1) == 0);

    const size_t verticesPerRow = width + 1;

    const size_t vertexCount = verticesPerRow * verticesPerRow;
    const size_t triangleCount = width * width;
    const size_t patchCount = (width * width) / 2;

    core::VertexAttributeBuffer<simd::vec3f> buffer = {
        .vertices = std::make_shared<simd::vec3f[]>(vertexCount),
        .vertexCount = vertexCount,
        .triangleIndices = std::make_shared<vec3u[]>(triangleCount),
        .triangleCount = triangleCount,
        .patchIndices = std::make_shared<vec4u[]>(patchCount),
        .patchCount = patchCount,
    };

    for (size_t y = 0; y < verticesPerRow; ++y) {
        for (size_t x = 0; x < verticesPerRow; ++x) {
            buffer.vertices[y * verticesPerRow + x] = simd::vec3f(x, y, 0);
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

    return buffer;
}

std::shared_ptr<SceneStorage> makeScene(size_t gridSizeX, size_t gridSizeY) {
    std::shared_ptr<SceneStorage> scene = std::make_shared<cobalt::geom::SceneStorage>();

    for (uint32_t y = 0; y < gridSizeY; ++y) {
        for (uint32_t x = 0; x < gridSizeX; ++x) {
            [[maybe_unused]] geom::Primitive sphereIdx = scene->addSphere({
                .center = cobalt::simd::vec3f(float(x), float(y), 0.f),
                .radius = .5f,
            });
        }
    }

    return scene;
}

} // namespace cobalt::geom::test

class CobaltGeometryTest : public ::testing::Test {
protected:
    CobaltGeometryTest() {
        storage = cobalt::geom::test::makeScene(kGridSizeX, kGridSizeY);
        meshStorage = std::make_shared<cobalt::geom::MeshStorage>(cobalt::geom::test::makeCubeMesh(kCubeSize));
    }

    std::shared_ptr<cobalt::geom::SceneStorage> storage;
    std::shared_ptr<cobalt::geom::MeshStorage> meshStorage;

    static constexpr size_t kGridSizeX = 512;
    static constexpr size_t kGridSizeY = 512;

    static constexpr size_t kCubeSize = 8;
};

TEST_F(CobaltGeometryTest, TestBoundingBoxIntersect) {
    static const cobalt::simd::vec3f origin(0.f, 0.f, 0.f);
    static const cobalt::simd::vec3f xDir(1.f, 0.f, 0.f);
    static const cobalt::geom::Ray xRay(origin, xDir, 10.f);

    {
        // hit (in front)
        static const cobalt::simd::vec3f boxMin(2.f, -1.f, -1.f);
        static const cobalt::simd::vec3f boxMax(4.f, 1.f, 1.f);
        const cobalt::geom::AxisAlignedBoundingBox aabb(boxMin, boxMax);

        float timeMin, timeMax;
        const bool hit = cobalt::geom::rayAxisAlignedBoundingBoxIntersection(xRay, aabb, timeMin, timeMax);
        EXPECT_TRUE(hit);
        EXPECT_NEAR(timeMin, 2.f, kEpsilon);
        EXPECT_NEAR(timeMax, 4.f, kEpsilon);
    }
    {
        // hit (inside)
        static const cobalt::simd::vec3f boxMin(-1.f, -1.f, -1.f);
        static const cobalt::simd::vec3f boxMax(1.f, 1.f, 1.f);
        const cobalt::geom::AxisAlignedBoundingBox aabb(boxMin, boxMax);

        float timeMin, timeMax;
        const bool hit = cobalt::geom::rayAxisAlignedBoundingBoxIntersection(xRay, aabb, timeMin, timeMax);
        EXPECT_TRUE(hit);
        EXPECT_NEAR(timeMin, 1.f, kEpsilon);
    }
    {
        // miss (behind)
        static const cobalt::simd::vec3f boxMin(-4.f, -1.f, -1.f);
        static const cobalt::simd::vec3f boxMax(-2.f, 1.f, 1.f);
        const cobalt::geom::AxisAlignedBoundingBox aabb(boxMin, boxMax);

        float timeMin, timeMax;
        const bool hit = cobalt::geom::rayAxisAlignedBoundingBoxIntersection(xRay, aabb, timeMin, timeMax);
        EXPECT_FALSE(hit);
    }
    {
        // miss
        static const cobalt::simd::vec3f boxMin(-3.f, 4.f, -1.f);
        static const cobalt::simd::vec3f boxMax(-1.f, 6.f, 1.f);
        const cobalt::geom::AxisAlignedBoundingBox aabb(boxMin, boxMax);

        float timeMin, timeMax;
        const bool hit = cobalt::geom::rayAxisAlignedBoundingBoxIntersection(xRay, aabb, timeMin, timeMax);
        EXPECT_FALSE(hit);
    }
    {
        // miss
        static const cobalt::simd::vec3f boxMin(-.5f, -.5f, -.5f);
        static const cobalt::simd::vec3f boxMax(.5f, .5f, .5f);
        static const cobalt::geom::Ray zRay(
            cobalt::simd::vec3f(2.f, 0.f, 4.f),
            cobalt::simd::vec3f(0.f, 0.f, -1.f),
            10.f
        );
        const cobalt::geom::AxisAlignedBoundingBox aabb(boxMin, boxMax);

        float timeMin, timeMax;
        const bool hit = cobalt::geom::rayAxisAlignedBoundingBoxIntersection(zRay, aabb, timeMin, timeMax);
        EXPECT_FALSE(hit);
    }
    {
        // miss (beyond terminal dist)
        static const cobalt::simd::vec3f boxMin(12.f, -1.f, -1.f);
        static const cobalt::simd::vec3f boxMax(14.f, 1.f, 1.f);
        const cobalt::geom::AxisAlignedBoundingBox aabb(boxMin, boxMax);

        float timeMin, timeMax;
        const bool hit = cobalt::geom::rayAxisAlignedBoundingBoxIntersection(xRay, aabb, timeMin, timeMax);
        EXPECT_FALSE(hit);
    }
}

TEST_F(CobaltGeometryTest, TestSphereIntersect) {
    {
        static const cobalt::geom::Sphere sphere{
            .center = cobalt::simd::vec3f(0.f, 0.f, 0.f),
            .radius = 1.f,
        };

        static const cobalt::geom::Ray ray(
            cobalt::simd::vec3f(0.f, 0.f, -5.f),
            cobalt::simd::vec3f(0.f, 0.f, 1.f),
            10.f
        );

        float timeMin, timeMax;
        EXPECT_TRUE(cobalt::geom::raySphereIntersection(ray, sphere, timeMin, timeMax));
        EXPECT_EQ(timeMin, 4.f);
        EXPECT_EQ(timeMax, 6.f);
    }
    {
        static const cobalt::geom::Sphere sphere{
            .center = cobalt::simd::vec3f(0.f, 0.f, 0.f),
            .radius = 4.f,
        };

        static const cobalt::geom::Ray ray(
            cobalt::simd::vec3f(0.f, 0.f, 0.f),
            cobalt::simd::vec3f(1.f, 0.f, 0.f),
            10.f
        );

        float timeMin, timeMax;
        EXPECT_TRUE(cobalt::geom::raySphereIntersection(ray, sphere, timeMin, timeMax));
        EXPECT_EQ(timeMin, -4.f);
        EXPECT_EQ(timeMax, 4.f);
    }
    {
        static const cobalt::geom::Sphere sphere{
            .center = cobalt::simd::vec3f(5.f, 5.f, 5.f),
            .radius = 5.f,
        };

        static const cobalt::geom::Ray ray(
            cobalt::simd::vec3f(0.f, 10.f, 0.f),
            cobalt::simd::vec3f(0.f, -1.f, 0.f),
            10.f
        );

        float timeMin, timeMax;
        EXPECT_FALSE(cobalt::geom::raySphereIntersection(ray, sphere, timeMin, timeMax));
    }
    {
        static const cobalt::geom::Sphere sphere{
            .center = cobalt::simd::vec3f(3.f, 0.f, 3.f),
            .radius = 3.f,
        };

        static const cobalt::geom::Ray ray(
            cobalt::simd::vec3f(0.f, 0.f, 6.f),
            cobalt::simd::vec3f(0.7071f, 0, -.7071f),
            10.f
        );

        float timeMin, timeMax;
        EXPECT_TRUE(cobalt::geom::raySphereIntersection(ray, sphere, timeMin, timeMax));
        EXPECT_NEAR(timeMin, 1.24264f, 1e-4f);
        EXPECT_NEAR(timeMax, 7.24264f, 1e-4f);
    }
}

TEST_F(CobaltGeometryTest, TestTriangleIntersect) {
    {
        static const cobalt::geom::Triangle triangle{
            .position1 = cobalt::simd::vec3f{ 0.f, 1.f, 1.f},
            .position2 = cobalt::simd::vec3f{ 1.f, 0.f, 1.f},
            .position3 = cobalt::simd::vec3f{-1.f, 0.f, 1.f},
        };

        static const cobalt::geom::Ray ray =
            cobalt::geom::Ray(cobalt::simd::vec3f{0.f, .5f, 0.f}, cobalt::simd::vec3f{0.f, 0.f, 1.f}, 10.f);

        float hitTime;
        cobalt::vec2f hitCoordinates;
        const bool hit = cobalt::geom::rayTriangleIntersection(
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

TEST_F(CobaltGeometryTest, TestMesh) {
    {
        // no vertices
        std::shared_ptr<cobalt::geom::Mesh> mesh = cobalt::geom::Mesh::create({
            .positions = {
                          .vertices = nullptr,
                          .vertexCount = 0,
                          .triangleIndices = std::make_shared<cobalt::vec3u[]>(1),
                          .triangleCount = 1,
                          .patchIndices = std::make_shared<cobalt::vec4u[]>(1),
                          .patchCount = 1,
                          },
        });

        EXPECT_FALSE(mesh);
    }
    {
        // no geometry (indices)
        std::shared_ptr<cobalt::geom::Mesh> mesh = cobalt::geom::Mesh::create({
            .positions = {
                          .vertices = std::make_shared<cobalt::simd::vec3f[]>(4),
                          .vertexCount = 4,
                          .triangleIndices = nullptr,
                          .triangleCount = 0,
                          .patchIndices = nullptr,
                          .patchCount = 0,
                          },
        });

        EXPECT_FALSE(mesh);
    }
    {
        // triangle mismatch
        std::shared_ptr<cobalt::geom::Mesh> mesh = cobalt::geom::Mesh::create({
            .positions = {
                          .vertices = std::make_shared<cobalt::simd::vec3f[]>(3),
                          .vertexCount = 3,
                          .triangleIndices = std::make_shared<cobalt::vec3u[]>(1),
                          .triangleCount = 0,
                          .patchIndices = nullptr,
                          .patchCount = 0,
                          },
        });

        EXPECT_FALSE(mesh);
    }
    {
        // patch mismatch
        std::shared_ptr<cobalt::geom::Mesh> mesh = cobalt::geom::Mesh::create({
            .positions = {
                          .vertices = std::make_shared<cobalt::simd::vec3f[]>(4),
                          .vertexCount = 4,
                          .triangleIndices = nullptr,
                          .triangleCount = 0,
                          .patchIndices = std::make_shared<cobalt::vec4u[]>(1),
                          .patchCount = 0,
                          },
        });

        EXPECT_FALSE(mesh);
    }

    // valid
    std::shared_ptr<cobalt::geom::Mesh> mesh = cobalt::geom::Mesh::create({
        .positions = cobalt::geom::test::makeCubeMesh(2),
    });

    ASSERT_TRUE(mesh != nullptr);
    {
        // hit
        const cobalt::geom::Ray ray(cobalt::simd::vec3f(.5f, .5f, -5.f), cobalt::simd::vec3f(0.f, 0.f, 1.f), 10.f);

        const cobalt::geom::IntersectionResult result = mesh->intersects(ray);
        EXPECT_EQ(result.shape.type, cobalt::geom::Shape::kTriangle);
        EXPECT_EQ(result.hitTime, 5.f);
    }
    {
        // facing away
        const cobalt::geom::Ray ray(cobalt::simd::vec3f(1.f, 1.f, -5.f), cobalt::simd::vec3f(0.f, 0.f, -1.f), 10.f);

        const cobalt::geom::IntersectionResult result = mesh->intersects(ray);
        EXPECT_EQ(result.shape.type, cobalt::geom::Shape::kNone);
    }
    {
        // out of ray distance
        const cobalt::geom::Ray ray(cobalt::simd::vec3f(-4.f, 0.f, 0.f), cobalt::simd::vec3f(1.f, 0.f, 0.f), 2.f);

        const cobalt::geom::IntersectionResult result = mesh->intersects(ray);
        EXPECT_EQ(result.shape.type, cobalt::geom::Shape::kNone);
    }
}

TEST_F(CobaltGeometryTest, TestBoundingBoxPerformance) {
    static const cobalt::simd::vec3f origin(0.f, 0.f, 0.f);
    static const cobalt::simd::vec3f xDir(1.f, 0.f, 0.f);
    static const cobalt::geom::Ray xRay(origin, xDir, 10.f);

    // make a bunch of BBoxes
    static const size_t numBoxes = 100000;
    std::vector<cobalt::geom::AxisAlignedBoundingBox> boxes;
    boxes.reserve(numBoxes);
    for (size_t idx = 0; idx < numBoxes; ++idx) {
        cobalt::simd::vec3f boxMin(idx, idx, idx);
        cobalt::simd::vec3f boxMax(idx + 1, idx + 1, idx + 1);
        boxes.emplace_back(boxMin, boxMax);
    }

    float timeMin, timeMax;
    for (const auto &box : boxes) {
        cobalt::geom::rayAxisAlignedBoundingBoxIntersection(xRay, box, timeMin, timeMax);
    }
}

TEST_F(CobaltGeometryTest, TestCreateMeshStorage) {
    using namespace cobalt::geom;

    auto mortonKeyer = [](const MortonPrimitive &lhs) {
        return lhs.mortonCode;
    };

    std::vector<MortonPrimitive> mortonEncodedPrimitives = meshStorage->mortonEncodePrimitives();

    cobalt::core::radix_sort<30>(mortonEncodedPrimitives.begin(), mortonEncodedPrimitives.end(), mortonKeyer);

    // TODO: structure of array here; looks like I can "coarsen these reorders"
    meshStorage->reorder(mortonEncodedPrimitives);

    BoundingVolume<MeshStorage> boundingVolume(meshStorage, mortonEncodedPrimitives);

    for (size_t y = 0; y < kCubeSize; ++y) {
        const size_t offset = (y & 1);
        for (size_t x = 0; x < kCubeSize; ++x) {
            const cobalt::geom::Ray ray(
                cobalt::simd::vec3f(x + .25f, y + .25f, 4.f),
                cobalt::simd::vec3f(0.f, 0.f, -1.f),
                10.f
            );
            const cobalt::geom::IntersectionResult result = boundingVolume.intersects(ray);
            std::ostringstream testDescription;
            testDescription << "Mesh tile is: (" << x << ", " << y << ") of " << kCubeSize;
            ASSERT_NEAR(result.hitTime, 4.f, kEpsilon) << testDescription.view();
            // FIXME: Need to "remap" the index from the original grid values to the reordered indices in order to check
            // what box / sphere we hit
            if (((x + offset) & 1) == 1) {
                EXPECT_EQ(result.shape.type, cobalt::geom::kPatch) << testDescription.view();
            } else {
                EXPECT_EQ(result.shape.type, cobalt::geom::kTriangle) << testDescription.view();
            }
        }
    }
}

TEST_F(CobaltGeometryTest, TestCreateSceneStorage) {
    using namespace cobalt::geom;
    auto mortonKeyer = [](const MortonPrimitive &lhs) {
        return lhs.mortonCode;
    };

    std::vector<MortonPrimitive> mortonEncodedPrimitives = storage->mortonEncodePrimitives();

    cobalt::core::radix_sort<30>(mortonEncodedPrimitives.begin(), mortonEncodedPrimitives.end(), mortonKeyer);

    // TODO: structure of array here; looks like I can "coarsen these reorders"
    storage->reorder(mortonEncodedPrimitives);

    BoundingVolume<SceneStorage> boundingVolume(storage, mortonEncodedPrimitives);

    for (size_t y = 0; y < kGridSizeY; ++y) {
        for (size_t x = 0; x < kGridSizeX; ++x) {
            const cobalt::geom::Ray ray(cobalt::simd::vec3f(x, y, 4.f), cobalt::simd::vec3f(0.f, 0.f, -1.f), 10.f);
            const cobalt::geom::IntersectionResult result = boundingVolume.intersects(ray);
            ASSERT_NEAR(result.hitTime, 3.5f, kEpsilon);
            // FIXME: Need to "remap" the index from the original grid values to the reordered indices in order to check
            // what box / sphere we hit
            ASSERT_EQ(result.shape.type, cobalt::geom::kSphere);
        }
    }
}
