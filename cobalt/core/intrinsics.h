#ifndef COBALT_CORE_INTRINSICS_H
#define COBALT_CORE_INTRINSICS_H

namespace cobalt::core {

inline unsigned int countLeadingZeros(unsigned int i) {
    return __builtin_clz(i);
}

} // namespace cobalt::core

#endif // COBALT_CORE_INTRINSICS_H
