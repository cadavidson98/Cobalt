#include "vec.h"

namespace cblt
{
    Vec2::Vec2(float _x, float _y) : x(_x), y(_y) {

    }

    Vec2 Vec2::operator-() const {
        return Vec2(-x, -y);
    }

    Vec2 operator *(const float &lhs, const Vec2& rhs) {
        return Vec2(lhs * rhs.x, lhs * rhs.y);
    }

    Vec2 operator /(const float &lhs, const Vec2& rhs) {
        return Vec2(lhs / rhs.x, lhs / rhs.y);
    }

    Vec2 Vec2::operator /(const float& rhs) const {
        return Vec2(x / rhs, y / rhs);
    }
    
    Vec2 Vec2::operator *(const float& rhs) const {
        return Vec2(x * rhs, y * rhs);
    }
    
    Vec2 Vec2::operator -(const Vec2& rhs) const {
        return Vec2(x - rhs.x, y - rhs.y);
    }
    
    Vec2 Vec2::operator +(const Vec2& rhs) const {
        return Vec2(x + rhs.x, y + rhs.y);
    }

    Vec3::Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {

    }

    Vec3::Vec3(const Vec2 &_xy, float _z) : x(_xy.x), y(_xy.y), z(_z) {

    }

    Vec3 Vec3::operator-() const {
        return Vec3(-x, -y, -z);
    }

    Vec3 operator /(const float& lhs, const Vec3& rhs) {
        return Vec3(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z);
    }

    Vec3 operator *(const float& lhs, const Vec3& rhs) {
        return Vec3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
    }

    Vec3 Vec3::operator /(const float& rhs) const {
        return Vec3(x / rhs, y / rhs, z / rhs);
    }
    
    Vec3 Vec3::operator *(const float& rhs) const {
        return Vec3(x * rhs, y * rhs, z * rhs);
    }
    
    Vec3 Vec3::operator -(const Vec3& rhs) const {
        return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
    }
    
    Vec3 Vec3::operator +(const Vec3& rhs) const {
        return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    Vec4::Vec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {

    }

    Vec4::Vec4(const Vec2 &_xy, float _z, float _w) : x(_xy.x), y(_xy.y), z(_z), w(_w) {

    }

    Vec4::Vec4(const Vec3 &_xyz, float _w) : x(_xyz.x), y(_xyz.y), z(_xyz.z), w(_w) {

    }

    Vec4 Vec4::operator-() const {
        return Vec4(-x, -y, -z, -w);
    }

    Vec4 Vec4::operator /(const float& rhs) const {
        return Vec4(x / rhs, y / rhs, z / rhs, w / rhs);
    }
    
    Vec4 Vec4::operator *(const float& rhs) const {
        return Vec4(x * rhs, y * rhs, z * rhs, w * rhs);
    }
    
    Vec4 Vec4::operator -(const Vec4& rhs) const {
        return Vec4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
    }
    
    Vec4 Vec4::operator +(const Vec4& rhs) const {
        return Vec4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
    }

    Vec4 operator /(const float& lhs, const Vec4& rhs) {
        return Vec4(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z, lhs / rhs.w);
    }

    Vec4 operator *(const float& lhs, const Vec4& rhs) {
        return Vec4(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
    }
}