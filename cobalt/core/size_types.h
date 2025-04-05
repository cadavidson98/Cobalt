#ifndef CBLT_CORE_SIZE_TYPES_H
#define CBLT_CORE_SIZE_TYPES_H

#if __has_include(<cstdint>)
#include <cstdint>
#else
using int8_t = char;
using int16_t = short;
using int32_t = int;
using int64_t = long long;

using uint8_t = unsigned char;
using uint16_t = unsigned short;
using uint32_t = unsigned int;
using uint64_t = unsigned long long;
#endif

#if __has_include(<cstddef>)
#include <cstddef>
#else
using size_t = unsigned long long;
#endif

// TODO: half type here
using float16_t = _Float16;
using float32_t = float;
using float64_t = double;
#endif // CBLT_CORE_SIZE_TYPES_H
