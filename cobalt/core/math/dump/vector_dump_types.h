#ifndef CBLT_MATH_VECTOR_DUMP_TYPES_H
#define CBLT_MATH_VECTOR_DUMP_TYPES_H

namespace cblt::utils {
class CoXMLElement;
}

namespace cblt {
template<typename T>
class Vec3Dump {
        T x;
        T y;
        T z;

        Vec3Dump(const CoXMLElement &attribute);
};

using Vec3fDump = Vec3Dump<float>;
using Vec3iDump = Vec3Dump<int>;

template<typename T>
class Vec4Dump {
        T x;
        T y;
        T z;
        T w;

        Vec4Dump(const CoXMLElement &element);
};

using Vec4fDump = Vec4Dump<float>;
using Vec4iDump = Vec4Dump<int>;
} // namespace cblt

#endif // CBLT_MATH_VECTOR_DUMP_TYPES_H
