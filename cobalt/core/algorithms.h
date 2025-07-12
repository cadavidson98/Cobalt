#ifndef CBLT_CORE_ALGORITHMS_H
#define CBLT_CORE_ALGORITHMS_H

#include "size_types.h"

#include <iterator>
#include <span>
#include <vector>

namespace cblt::core {

template<size_t numPasses = 32, class Iterator, typename KeyFetcher>
    requires std::random_access_iterator<Iterator>
void radix_sort(Iterator begin, Iterator end, KeyFetcher keyFetcher) {
    using ValueType = std::iterator_traits<Iterator>::value_type;

    std::vector<ValueType> scratch(std::distance(begin, end));

    std::span<ValueType> in(begin, end);
    std::span<ValueType> out(scratch);

    for (size_t pass = 0; pass < numPasses; ++pass) {
        const uint32_t bitMask = 1 << pass;
        uint32_t zeroBucket = 0;
        uint32_t onesBucket = 0;
        // compute bucket offsets
        for (const ValueType &v : in) {
            const uint32_t key = keyFetcher(v);
            onesBucket += bool(key & bitMask);
        }

        onesBucket = std::distance(begin, end) - onesBucket;

        for (const ValueType &v : in) {

            if (keyFetcher(v) & bitMask) {
                out[onesBucket++] = v;
            } else {
                out[zeroBucket++] = v;
            }
        }

        std::swap(in, out);
    }

    if constexpr (numPasses % 2 == 1) {
        std::move(std::make_move_iterator(scratch.begin()), std::make_move_iterator(scratch.end()), begin);
    }
}

} // namespace cblt::core

#endif // CBLT_CORE_ALGORITHMS_H
