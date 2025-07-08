#ifndef CBLT_GEOM_BOUNDING_VOLUME_TYPES_H
#define CBLT_GEOM_BOUNDING_VOLUME_TYPES_H

#include "bounding_box.h"

#include "core/morton_encoding.h"
#include "core/size_types.h"

#include <span>
#include <vector>

namespace cblt::geom::crtp {

    enum PrimitiveType : uint32_t {
        kNone = 0,
        kSphere = 1,
        kTriangle = 2,
        kPatch = 4,
        kBox = 8,
        kMesh = 16,
    };
    
    using PrimitiveTypes = uint32_t;
    
    struct Primitive {
        PrimitiveType type;
        CoAxisAlignedBoundingBox boundingBox;
        uint32_t index;
    };
    
    struct MortonPrimitive {
        uint32_t mortonCode;
        Primitive primitive;
    };
    
    template<class Derived>
    struct CoPrimitiveStorageBase {
    public:
    
        std::vector<MortonPrimitive> mortonEncodePrimitives() const {
            const CoAxisAlignedBoundingBox primitiveBounds = bounds();
            const simd::vec3f boundsExtent = primitiveBounds.Scales();
            const simd::vec3f boundsMin = primitiveBounds.min;
            std::vector<Primitive> storagePrimitives = primitives();
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
    
        std::vector<Primitive> primitives() const {
            return static_cast<const Derived *>(this)->primitives();
        }
    
        geom::CoAxisAlignedBoundingBox bounds() const {
            return static_cast<const Derived *>(this)->bounds();
        }
    
        void reorder(std::span<const MortonPrimitive> primitives) {
            return static_cast<Derived *>(this)->reorder(primitives);
        }
    
    protected:
        CoPrimitiveStorageBase() = default;
    };
    
    template<typename T>
    struct primitive_types {
        static const PrimitiveTypes value = PrimitiveType::kNone;
    };
    
    template<typename BoundingVolumeStorage>
    concept isPrimitiveStorage =
        requires { std::is_base_of_v<CoPrimitiveStorageBase<BoundingVolumeStorage>, BoundingVolumeStorage>; };

}  // namespace cblt::geom::crtp

#endif  // CBLT_GEOM_BOUNDING_VOLUME_TYPES_H