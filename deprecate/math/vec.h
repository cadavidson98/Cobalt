#ifndef VECTOR_LIB_H
#define VECTOR_LIB_H

#include <cmath>

namespace cblt {
    class Vec2 {
        public:
            Vec2(float x = 0, float y = 0);

            union
            {
                float xy[2];
                struct
                {
                    float x;
                    float y;
                };
            };

            Vec2 friend operator /(const float& lhs, const Vec2& rhs);
            Vec2 friend operator *(const float& lhs, const Vec2& rhs);

            Vec2 operator -() const;
            Vec2 operator /(const float& rhs) const;
            Vec2 operator *(const float& rhs) const;
            Vec2 operator -(const Vec2& rhs) const;
            Vec2 operator +(const Vec2& rhs) const;
    };

    inline float Dot(const Vec2 &lhs, const Vec2 &rhs) {
        return lhs.x * rhs.x + lhs.y * rhs.y;
    }

    inline float AbsDot(const Vec2 &lhs, const Vec2 &rhs) {
        return std::abs(Dot(lhs, rhs));
    }

    inline float Magnitude(const Vec2 &vec) {
        return std::sqrt(vec.x * vec.x + vec.y * vec.y);
    }

    inline float MagnitudeSqr(const Vec2 &vec) {
        return vec.x * vec.x + vec.y * vec.y;
    }

    inline Vec2 Normalize(const Vec2 &vec) {
        float len = Magnitude(vec);
        return (vec / (len + 1e-6f));
    }

    class Vec3 {
        public:

            Vec3(float x = 0, float y = 0, float z = 0);
            Vec3(const Vec2 &xy, float z);
            union
            {
                float xyz[3];
                struct
                {
                    float x;
                    float y;
                    float z;
                };
            };

            Vec3 friend operator /(const float& lhs, const Vec3& rhs);
            Vec3 friend operator *(const float& lhs, const Vec3& rhs);

            Vec3 operator -() const;
            Vec3 operator /(const float& rhs) const;
            Vec3 operator *(const float& rhs) const;
            Vec3 operator -(const Vec3 &rhs) const;
            Vec3 operator +(const Vec3 &rhs) const;
    };

    inline float Dot(const Vec3 &lhs, const Vec3 &rhs) {
        return std::fmaf(lhs.x, rhs.x, std::fmaf(lhs.y, rhs.y, lhs.z * rhs.z));
    }

    inline float AbsDot(const Vec3 &lhs, const Vec3 &rhs) {
        return std::abs(Dot(lhs, rhs));
    }

    inline Vec3 Cross(const Vec3 &lhs, const Vec3 &rhs) {
        return Vec3(lhs.y * rhs.z - lhs.z * rhs.y,
                  - lhs.x * rhs.z + rhs.x * lhs.z,
                  + lhs.x * rhs.y - lhs.y * rhs.x);
    }

    inline float Magnitude(const Vec3 &vec) {
        return std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    }

    inline float MagnitudeSqr(const Vec3 &vec) {
        return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
    }

    inline Vec3 Normalize(const Vec3 &vec) {
        float len = Magnitude(vec);
        return (vec / (len + 1e-6f));
    }

    class Vec4 {
        public:
            Vec4(float x = 0, float y = 0, float z = 0, float w = 0);
            Vec4(const Vec2 &xy, float z, float w);
            Vec4(const Vec3 &xyz, float w);
        
            union
            {
                float xyzw[4];
                struct
                {
                    float x;
                    float y;
                    float z;
                    float w;
                };
            };

            Vec4 friend operator /(const float& lhs, const Vec4& rhs);
            Vec4 friend operator *(const float& lhs, const Vec4& rhs);

            Vec4 operator -() const;
            Vec4 operator /(const float &rhs) const;
            Vec4 operator *(const float& rhs) const;
            Vec4 operator -(const Vec4 &rhs) const;
            Vec4 operator +(const Vec4 &rhs) const;
    };

    inline float Dot(const Vec4 &lhs, const Vec4 &rhs) {
        return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * lhs.z + lhs.w * lhs.w;
    }

    inline float AbsDot(const Vec4 &lhs, const Vec4 &rhs) {
        return std::abs(Dot(lhs, rhs));
    }

    inline float Magnitude(const Vec4 &vec) {
        return std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
    }

    inline float MagnitudeSqr(const Vec4 &vec) {
        return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w;
    }

    inline Vec4 Normalize(const Vec4 &vec) {
        float len = Magnitude(vec);
        return (vec / (len + 1e-6f));
    }
}

#endif  // VECTOR_LIB_H