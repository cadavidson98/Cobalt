#ifndef BOUNDING_VOLUME_TPP
#define BOUNDING_VOLUME_TPP
#include "bounding_volume.h"
#include "math/vec.h"
#include "math/constants.h"

#include <array>
#include <algorithm>
#include <stack>
namespace cblt {
    template <class T>
    BoundingVolume<T>::BoundingVolume() {
        max_prims_in_leaf_ = 255;
        BoundingNode root;
        tree_.push_back(root);
    }

    template <class T>
    BoundingVolume<T>::BoundingVolume(std::vector<std::shared_ptr<T>> &prims)
    {
        max_prims_in_leaf_ = 16;
        // get the bounds of the primitives, and store them in primitive info structures
        std::vector<PrimInfo> prims_info;
        prims_info.reserve(prims.size());

        for (const std::shared_ptr<T> & prim : prims) {
            prims_info.emplace_back(prim->GetBounds(), prim);
        }
        BoundingNode root;
        tree_.push_back(root);
        BuildRecurse(0, prims_info.begin(), prims_info.end());
    }

    template <class T>
    BoundingVolume<T>::BoundingVolume(std::vector<std::shared_ptr<T>> &prims, std::function<BoundingBox(T)> bounds_calc)
    {
        max_prims_in_leaf_ = 4;
        // get the bounds of the primitives, and store them in primitive info structures
        std::vector<PrimInfo> prims_info;
        prims_info.reserve(prims.size());

        for (const std::shared_ptr<T> & prim : prims) {
            prims_info.emplace_back(bounds_calc(*prim), prim);
        }

        BoundingNode root;
        tree_.push_back(root);
        BuildRecurse(0, prims_info.begin(), prims_info.end());
    }
    
    /**
     * @brief Check if the ray(p, d) intersects the tree, and populate the HitInfo
     * with information about the collision point
     * 
     * @param ray ray to intersect with the bounding volume heirarchy
     * @param hit HitInfo structure which the triangle collision information is stored in
     * @return true/false If the ray has collided with a triangle
     */
    template<class T>
    bool BoundingVolume<T>::Intersect(const Ray& ray, HitInfo &collison_pt) {
        return IntersectIterative(ray, collison_pt);
    }

    template<class T>
    BoundingBox BoundingVolume<T>::GetBounds() const {
        return tree_[0].bnds_;
    }

    /**
     * Determine the smallest boundary that encapsulates all the
     * prims
     */
    template <class T>
    BoundingBox BoundingVolume<T>::GetExtent(PrimIter prim_start, PrimIter prim_end) {
        BoundingBox extent;
        for (PrimIter iter = prim_start; iter != prim_end; ++iter) {
            PrimInfo prim = *iter;
            extent.max_.x = std::max(extent.max_.x, prim.bnds_.max_.x);
            extent.max_.y = std::max(extent.max_.y, prim.bnds_.max_.y);
            extent.max_.z = std::max(extent.max_.z, prim.bnds_.max_.z);

            extent.min_.x = std::min(extent.min_.x, prim.bnds_.min_.x);
            extent.min_.y = std::min(extent.min_.y, prim.bnds_.min_.y);
            extent.min_.z = std::min(extent.min_.z, prim.bnds_.min_.z);
        }
        extent.CalculateCenter();
        return extent;
    }

    template<class T>
    BoundingBox BoundingVolume<T>::GetExtent(const std::vector<Vec3> &centroids)
    {
        BoundingBox extent;
        for (const Vec3 &cen : centroids) {

            extent.max_.x = std::max(extent.max_.x, cen.x);
            extent.max_.y = std::max(extent.max_.y, cen.y);
            extent.max_.z = std::max(extent.max_.z, cen.z);

            extent.min_.x = std::min(extent.min_.x, cen.x);
            extent.min_.y = std::min(extent.min_.y, cen.y);
            extent.min_.z = std::min(extent.min_.z, cen.z);
        }
        extent.CalculateCenter();
        return extent;
    }

    /**
     * Construct a bvh by using the Surface Area Heuristic to subdivide
     * the leaf nodes provided 
     */
    template<class T>
    void BoundingVolume<T>::BuildRecurse(int node_offset, PrimIter prim_start, PrimIter prim_end) {
        if (std::distance(prim_start, prim_end) == 2) {
            BoundingNode l_child, r_child;
            l_child.bnds_ = prim_start->bnds_;
            l_child.prims_.push_back(prim_start->elem_);

            PrimInfo next_prim = *(std::next(prim_start, 1));
            r_child.bnds_ = next_prim.bnds_;
            r_child.prims_.push_back(next_prim.elem_);

            tree_.push_back(l_child);
            tree_.push_back(r_child);

            tree_[node_offset].bnds_ = GetExtent(prim_start, prim_end);
            tree_[node_offset].r_child_ = node_offset + 2;
            return;
        }
        else if (std::distance(prim_start, prim_end) == 1) {
            BoundingNode l_child;
            l_child.bnds_ = prim_start->bnds_;
            l_child.prims_.push_back(prim_start->elem_);

            tree_.push_back(l_child);

            tree_[node_offset].bnds_ = GetExtent(prim_start, prim_end);
            return;
        }
        else if (std::distance(prim_start, prim_end) <= 0) {
            // Super Badness occurred
            return;
        }
        tree_[node_offset].bnds_ = GetExtent(prim_start, prim_end);

        // iterator to the beginning of the primitives in the second bin
        PrimIter prim_mid;
        if (!SplitSAH(prim_start, prim_end, prim_mid)) {
            // stop recursing, since the sub-child split would be worse than the current split
            tree_[node_offset].r_child_ = -1;
            for (PrimIter iter = prim_start; iter != prim_end; iter++) {
                tree_[node_offset].prims_.push_back(iter->elem_);
            }
            return;
        }
        int size_left = std::distance(prim_start, prim_mid);
        int size_right = std::distance(prim_mid, prim_end);
        // create left child
        BoundingNode new_node;
        tree_.push_back(new_node);
        int l_offset = node_offset + 1;
        BuildRecurse(l_offset, prim_start, prim_mid);

        // create right child
        tree_.push_back(new_node);
        int r_offset = static_cast<int>(tree_.size()) - 1;
        tree_[node_offset].r_child_ = r_offset;
        BuildRecurse(r_offset, prim_mid, prim_end);
    }

    template<class T>
    bool BoundingVolume<T>::SplitSAH(PrimIter prim_start, PrimIter prim_end, PrimIter &prim_split) {
        // bound the node
        BoundingBox scene_bnds = GetExtent(prim_start, prim_end);
        int prim_size = static_cast<int>(std::distance(prim_start, prim_end));
        std::vector<Vec3> centroid_pts;
        centroid_pts.reserve(prim_size);

        for(PrimIter iter = prim_start; iter != prim_end; ++iter) {
            centroid_pts.push_back(iter->bnds_.cen_);
        }

        BoundingBox centroid_bnds = GetExtent(centroid_pts);
        float range_x = centroid_bnds.max_.x - centroid_bnds.min_.x;
        float range_y = centroid_bnds.max_.y - centroid_bnds.min_.y;
        float range_z = centroid_bnds.max_.z - centroid_bnds.min_.z;
        std::array<float, 3> ranges = { range_x, range_y, range_z };
        int axis = 0;
        // parition along semi-major axis, but using centroids to mitigate
        // large triangle issues...
        for(int i = 1; i < 3; ++i) {
            if (ranges[i] > ranges[axis]) {
                axis = i;
            }
        }
        if(ranges[axis] < eps_zero_F) {
            // the range on the largest axis is still too narrow in get a good split...
            return false;
        }
        float parent_sa = scene_bnds.SurfaceArea();

        // try binning the triangles
        const int num_bins = 16;
        std::array<Bucket, num_bins> bins;
        float range = ranges[axis];      
        float centroid_max = (axis == 0) ? centroid_bnds.max_.x : (axis == 1) ? centroid_bnds.max_.y : centroid_bnds.max_.z;
        float centroid_min = (axis == 0) ? centroid_bnds.min_.x : (axis == 1) ? centroid_bnds.min_.y : centroid_bnds.min_.z;
        for (PrimIter iter = prim_start; iter != prim_end; ++iter) {
            // find out what bin this triangle belongs in
            // this can be done by manually finding the threshhold value
            // or just normalizing its value
            float centroid_pos = (axis == 0) ? iter->bnds_.cen_.x : (axis == 1) ? iter->bnds_.cen_.y : iter->bnds_.cen_.z;
            float normalized_position = (centroid_pos - centroid_min) / range;
            int bucket_index = std::min(static_cast<int>(num_bins * normalized_position), num_bins - 1);
            bins[bucket_index].prims_.push_back(*iter);
        }

        // get the size of each bin
        for(Bucket &bin : bins) {
                bin.bnds_ = GetExtent(bin.prims_.begin(), bin.prims_.end());
        }
        constexpr int buckets_minus_1 = num_bins - 1;
        std::array<float, buckets_minus_1> costs;
        // now get all the possible SAH values
        for(int i = 0; i < buckets_minus_1; ++i) {
            int j = 0;
            BoundingBox bound1, bound2;
            int num_prims1(0), num_prims2(0);
            // this is one half split
            for(; j <= i; ++j) {
                bound1 = bound1.Union(bins[j].bnds_);
                num_prims1 += static_cast<int>(bins[j].prims_.size());
            }
            // this is the other half split
            for(; j < num_bins; ++j) {
                bound2 = bound2.Union(bins[j].bnds_);
                num_prims2 += static_cast<int>(bins[j].prims_.size());
            }
            // finally, calculate the SAH!
            costs[i] = .125f + (num_prims1 * bound1.SurfaceArea() + num_prims2 * bound2.SurfaceArea()) / parent_sa;
        }
        // now that we have all the costs, we can find the best split
        float min_cost = costs[0];
        int min_split = 0;
        for (int i = 1; i < buckets_minus_1; ++i) {
            if (costs[i] < min_cost) {
                min_split = i;
            }
        }
        // Finally, determine whether or not we need to create a leaf node
        float parent_cost = .125f + prim_size;
        if(min_cost < parent_cost || prim_size > max_prims_in_leaf_) {
        // we need to subdivide
        prim_split = std::partition(prim_start, prim_end, [=](const PrimInfo & prim) {
            float centroid_pos = (axis == 0) ? prim.bnds_.cen_.x : (axis == 1) ? prim.bnds_.cen_.y : prim.bnds_.cen_.z;
            float normalized_position = (centroid_pos - centroid_min) / range;
            int bucket_index = std::min(static_cast<int>(num_bins * normalized_position), num_bins - 1);
            return bucket_index <= min_split;
        });
        // we can continue to subdivide;
        return true;
        } else {
            // this needs to become a leaf node
            return false;
        }
    }   

    template<class T>
    bool BoundingVolume<T>::SplitMidpoint(PrimIter prim_start, PrimIter prim_end, PrimIter &prim_split) {
        BoundingBox bnds = GetExtent(prim_start, prim_end); 
        std::array<float, 3> arr;
        arr[0] = std::abs(bnds.max_.x - bnds.min_.x);
        arr[1] = std::abs(bnds.max_.y - bnds.min_.y);
        arr[2] = std::abs(bnds.max_.z - bnds.min_.z);
        int index = 0;
        float max = arr[0];
        for(int i = 1; i < 3; ++i) {
            if(arr[i] > max) {
                index = i;
                max = arr[i];
            }
        }
        // now split
        if(index == 0) {
            // split on x
            std::sort(prim_start, prim_end, [](const PrimInfo& prim_1, const PrimInfo &prim_2) {
            return prim_1.bnds_.cen_.x < prim_2.bnds_.cen_.x;
            });
        }
        else if (index == 1) {
            // split on y
            std::sort(prim_start, prim_end, [](const PrimInfo& prim_1, const PrimInfo &prim_2) {
            return prim_1.bnds_.cen_.y < prim_2.bnds_.cen_.y;
            });
        }
        else {
            // split on z
            std::sort(prim_start, prim_end, [](const PrimInfo& prim_1, const PrimInfo &prim_2) {
            return prim_1.bnds_.cen_.z < prim_2.bnds_.cen_.z;
            });
        }

        // now give half the leaves to each child
        int half = static_cast<int>(std::distance(prim_start, prim_end)) / 2;
        prim_split = std::next(prim_start, half);
        return true;
    }

    /**
     * @brief Use Depth First Search to traverse through the bounding volume heirarchy
     * to find a triangle which collisdes with ray(p, d)
     * @param ray ray to intersect with the bounding volume heirarchy
     * @param hit HitInfo structure which the triangle collision information is stored in
     * @return true/false If the ray has collided with a triangle 
     */
    template <class T>
    bool BoundingVolume<T>::IntersectIterative(const Ray &ray, HitInfo &hit)
    {
        HitInfo prim_hit;
        
        int nodes[2048];
        int stack_idx = 0;
        nodes[0] = 0;
        float best_time = hit.hit_time;
        bool result = false;
        while(stack_idx >= 0)
        {
            int cur_node_idx = nodes[stack_idx--];

            BoundingNode &cur_node = tree_[cur_node_idx];
            float i_time = inf_F;
            if(!cur_node.bnds_.Intersect(ray, i_time) || i_time > best_time)
            {
                // Either this node does exist, the ray doesn't collide with it, or we have already found a closer
                // prim, so there is no reason to check this node
                continue;
            }
            
            if(cur_node.prims_.size() > 0)
            {
                // check to see if there is a collision
                for (const std::shared_ptr<T> &prim : cur_node.prims_)
                {
                    prim_hit.hit_time = inf_F;
                    result = prim->Intersect(ray, prim_hit) || result;
                    if (prim_hit.hit_time < hit.hit_time)
                    {
                        // this is a closer item than the previous one
                        hit = prim_hit;
                        best_time = hit.hit_time;
                    }
                }
            }
            else
            {
                if (cur_node.r_child_ != -1)
                {
                    nodes[++stack_idx] = cur_node.r_child_;
                }
                nodes[++stack_idx] = cur_node_idx + 1;
            }
        }
        return result;
    }
}

#endif  // BOUNDING_VOLUME_TPP