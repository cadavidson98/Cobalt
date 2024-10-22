#ifndef BOUNDING_VOLUME_H
#define BOUNDING_VOLUME_H

#include "ray.h"
#include "hit_info.h"
#include "bounding_box.h"

#include <functional>
#include <vector>
#include <memory>

namespace cblt {
    
    template <class T>
    class BoundingVolume {
        public:
            BoundingVolume();
            BoundingVolume(std::vector<std::shared_ptr<T>> &prims);
            BoundingVolume(std::vector<std::shared_ptr<T>> &prims, std::function<BoundingBox(T)> bounds_calc);
            bool Intersect(const Ray& ray, HitInfo &collison_pt);
            BoundingBox GetBounds() const;

        private:

            struct BoundingNode {
                BoundingBox bnds_;  //! bounding box which encapsulates the node
                //int l_child_ = -1;  //! integer offset to left child node
                int r_child_ = -1;  //! integer offset to right child node
                std::vector<std::shared_ptr<T>> prims_;  //! integer offsets to all primitives in this node
            };

            struct PrimInfo {
                PrimInfo()
                {
                };

                PrimInfo(BoundingBox bx, const std::shared_ptr<T> &prim) 
                { 
                    bnds_ = bx;
                    elem_ = prim;
                };
                BoundingBox bnds_;  //! bounding box for the primitive
                std::shared_ptr<T> elem_;  //! integer offset to the primitive in the collection
            };

            /**
              * @brief A small structure used to evaluate potential bounding volume heirarchy splits when evaluating
              * the surface area heuristic.
              * 
              */
            struct Bucket {
                BoundingBox bnds_;
                std::vector<PrimInfo> prims_;
            };

            using PrimIter = typename std::vector<PrimInfo>::iterator;

            int max_prims_in_leaf_;
            std::vector<BoundingNode> tree_;

            BoundingBox GetExtent(PrimIter prim_start, PrimIter prim_end);
            BoundingBox GetExtent(const std::vector<Vec3> &centroids);

            bool SplitSAH(PrimIter prim_start, PrimIter prim_end, PrimIter &prim_split);
            bool SplitMidpoint(PrimIter prim_start, PrimIter prim_end, PrimIter &prim_split);
            void BuildRecurse(int node_offset, PrimIter prims_start, PrimIter prims_end);
            bool IntersectIterative(const Ray &ray, HitInfo &hit);
    };
}

#include "bounding_volume.tpp"
#endif  // BOUNDING_VOLUME_H