#ifndef COBALT_RENDER_COMPONENT_H
#define COBALT_RENDER_COMPONENT_H

#include "color/blackbody_spectrum.h"
#include "color/polynomial_spectrum.h"
#include "color/spectrum.h"
#include "core/size_types.h"
#include "geometry/bounding_volume_types.h"
#include "geometry/mesh.h"
#include "geometry/sphere.h"

#include <cassert>

namespace cobalt::render {

struct Component {
    uint32_t materialIdx;
};

// TODO: why is this just an index; is is because the raw data is owned by the BVH?
// perhaps as an exercise, you can review why the bvh 'owns' instead of 'shares' geometry
// okay, well obviously reordering is a key reason: bvh needs to reorder, which in turn
// changes the logical indices we have. Unless.... we reorder as a service.
// but anyways, that reorder is probably why ownership.

// also, keep in mind due to access patterns, reording for geometry won't necessarily benefit
// other components (materials)
// but if that is the case, then a Primitive must be defined as a struct of indicies (which it is)
// also keep in mind, a primtive may not have ALL components (ex: env doesn't have geometry)
// so it is a structure of OPTIONALS!
// using size_t(-1) as an invalid idx is probably best

// okay, so does BVH return a primitive idx, or a geometry idx?
// really what it returns is an arbitrary piece of user data -> could be index or hash or ptr
// I guess, can you reason about a scenario where the geometry isn't unique (ex instancing)
// so you are back to mapping v reverse mapping

// I think it is trivial to reason geometry is unique (even with instancing, yes)
// what that means is __technically__ you could 'find' the primitive that corresponds to geometry x
// I could imagine a 'fancy' solution where you have a cache with hashes; but I don't think it will be useful in practice
// binary search is a 'no go' too. So the only question is: why not just have the relationship be 'implicit?' (idx)
// what about 'explicit' (pointer) (can't use ptr, tried)
// the key is 'wavefront rendering' yes? Addresses are fine, indices are fine, everything else is not.
// If I'm being completely honest, I want to do the most 'C' thing possible
// I think the key thing to 'change' about your current approach is enum & index == offset into array

// some basic questions:
// 1) single uber array? then index is just index (could be size_t 'address' or byte index) -> do you 'downcast?'
// 2) what do you actually need FROM the data? What fields are required to access? For Geom, nothing? what about blackbodies?
// 2.a) To be honest, I think there is some 'poor mans virtualization' here; I don't NEED anything, just compute some equation
// 2.b) The most obvious benefit to that is it means I could just say: "compute all polynomial, sampled, then blackbodies"
// 2.c) Which means in the memory usage becomes and array of indices per type, given that each 'primitive' knows the type -> so I do need 'TypedIndex'

template <typename T>
struct TypedIndex {
    T type;
    uint32_t idx;
};

class ComponentStorage {
public:
    enum Tag: uint8_t {
        kGeometry = 1,
        kSpectrum = 2,
    };

    struct Primitive {
        uint8_t components;
        TypedIndex<geom::Shape> geometryIdx;
        TypedIndex<color::SpectrumType> spectrumIdx;
    };

    Component component(const Primitive primitive, Tag tag) {
        if ((primitive.components & tag) == 0) {
            return;
        }

        if (tag & Tag::kGeometry) {

        };

        if (tag & Tag::kSpectrum) {

        };

        primitive.components[primitive.];
    }

private:

    std::vector<Component> meshIndices = {};
    std::vector<Component> sphereIndices = {};
    std::vector<color::PolynomialSpectrum> spectrums = {};

    Component operator()(const geom::Primitive primitive) {
        switch (primitive.type) {
        case geom::Shape::kMesh :
            return meshIndices[primitive.index];
        case geom::Shape::kSphere :
            return sphereIndices[primitive.index];
        case geom::Shape::kTriangle :
            [[fallthrough]];
        case geom::Shape::kPatch :
            [[fallthrough]];
        case geom::Shape::kNone :
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
            case geom::Shape::kMesh :
                meshIndices[meshIdx++] = meshCopy[primitive.index];
                break;
            case geom::Shape::kSphere :
                sphereIndices[sphereIdx++] = sphereCopy[primitive.index];
                break;
            case geom::Shape::kTriangle :
                [[fallthrough]];
            case geom::Shape::kPatch :
                [[fallthrough]];
            case geom::Shape::kNone :
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
        case geom::kNone :
            [[fallthrough]];
        default :
            assert(false);
        }
    }
};

} // namespace cobalt::render

#endif // COBALT_RENDER_COMPONENT_H
