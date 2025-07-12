#ifndef CBLT_CORE_INTRINSICS_H
#define CBLT_CORE_INTRINSICS_H

namespace cblt::core {

inline unsigned int countLeadingZeros(unsigned int i) {
    return __builtin_clz(i);
}

} // namespace cblt::core

#endif // CBLT_CORE_INTRINSICS_H
