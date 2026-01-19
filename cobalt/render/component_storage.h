#ifndef CBLT_RENDER_COMPONENT_H
#define CBLT_RENDER_COMPONENT_H

#include "color/jakob.h"
#include "core/size_types.h"
#include "geometry/bounding_volume_types.h"

#include <cassert>

namespace cblt::render {

struct Component {
    uint32_t materialIdx;
};

struct ComponentStorage {
    std::vector<Component> meshIndices = {};
    std::vector<Component> sphereIndices = {};
    std::vector<color::PolynomialSpectrum> spectrums = {};

    Component operator()(const geom::Primitive primitive) {
        switch (primitive.type) {
        case geom::kMesh :
            return meshIndices[primitive.index];
        case geom::kSphere :
            return sphereIndices[primitive.index];
        case geom::kTriangle :
            [[fallthrough]];
        case geom::kPatch :
            [[fallthrough]];
        default :
            assert(false);
            return Component{};
        }
    }

    void reorder(std::span<const geom::MortonPrimitive> primitives) {
        std::vector<Component> meshCopy = meshIndices;
        std::vector<Component> sphereCopy = sphereIndices;

        uint32_t meshIdx = 0;
        uint32_t sphereIdx = 0;

        for (const geom::MortonPrimitive &mortonPrimitive : primitives) {
            const geom::Primitive &primitive = mortonPrimitive.primitive;
            switch (primitive.type) {
            case geom::PrimitiveType::kMesh :
                meshIndices[meshIdx++] = meshCopy[primitive.index];
                break;
            case geom::PrimitiveType::kSphere :
                sphereIndices[sphereIdx++] = sphereCopy[primitive.index];
                break;
            case geom::PrimitiveType::kTriangle :
                [[fallthrough]];
            case geom::PrimitiveType::kPatch :
                [[fallthrough]];
            default :
                assert(false && "Unsupported type in scene storage");
                break;
            }
        }
    }

    void addSpectrum(const geom::Primitive primitive, const color::PolynomialSpectrum spectrum) {
        spectrums.push_back(spectrum);
        const uint32_t spectrumIdx = spectrums.size() - 1;
        switch (primitive.type) {
        case geom::kMesh :
            meshIndices.push_back({
                .materialIdx = spectrumIdx,
            });
            return;
        case geom::kSphere :
            sphereIndices.push_back({
                .materialIdx = spectrumIdx,
            });
            return;
        case geom::kTriangle :
            [[fallthrough]];
        case geom::kPatch :
            [[fallthrough]];
        default :
            assert(false);
        }
    }
};

} // namespace cblt::render

#endif // CBLT_RENDER_COMPONENT_H
