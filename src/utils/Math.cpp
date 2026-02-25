/**
 * @file Math.cpp
 * @brief Implementation of mathematical structures and their operators.
 */

#include "utils/Math.h"

Math::Vec2::Vec2(const Vec3& vec3) :
    x(vec3.x),
    y(vec3.y) {}

Math::Vec2::Vec2(const Vec4& vec4) :
    x(vec4.x),
    y(vec4.y) {}

Math::Vec2& Math::Vec2::operator=(const Vec2& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
    }
    return *this;
}

Math::Vec2& Math::Vec2::operator=(const Vec3& other) {
    x = other.x;
    y = other.y;
    return *this;
}

Math::Vec2& Math::Vec2::operator=(const Vec4& other) {
    x = other.x;
    y = other.y;
    return *this;
}

bool Math::Vec2::operator==(const Vec2& other) const {
    return x == other.x && y == other.y;
}

bool Math::Vec2::operator!=(const Vec2& other) const {
    return !(*this == other);
}

Math::Vec2 Math::Vec2::operator+(const Vec2& other) const {
    return Vec2(x + other.x, y + other.y);
}

Math::Vec2 Math::Vec2::operator-(const Vec2& other) const {
    return Vec2(x - other.x, y - other.y);
}

Math::Mat2 Math::Vec2::operator*(const Vec2& other) const {
    return Mat2(
        x * other.x, x * other.y,
        y * other.x, y * other.y
    );
}

Math::Vec2& Math::Vec2::operator+=(const Vec2& other) {
    x += other.x;
    y += other.y;
    return *this;
}

Math::Vec2& Math::Vec2::operator-=(const Vec2& other) {
    x -= other.x;
    y -= other.y;
    return *this;
}

Math::Vec2 Math::Vec2::operator+(const float scalar) const {
    return Vec2(x + scalar, y + scalar);
}

Math::Vec2 Math::Vec2::operator-(const float scalar) const {
    return Vec2(x - scalar, y - scalar);
}

Math::Vec2 Math::Vec2::operator*(const float scalar) const {
    return Vec2(x * scalar, y * scalar);
}

Math::Vec2 Math::Vec2::operator/(const float scalar) const {
    return Vec2(x / scalar, y / scalar);
}

Math::Vec2& Math::Vec2::operator+=(const float scalar) {
    x += scalar;
    y += scalar;
    return *this;
}

Math::Vec2& Math::Vec2::operator-=(const float scalar) {
    x -= scalar;
    y -= scalar;
    return *this;
}

Math::Vec2& Math::Vec2::operator*=(const float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
}

Math::Vec2& Math::Vec2::operator/=(const float scalar) {
    x /= scalar;
    y /= scalar;
    return *this;
}

float Math::Vec2::operator[](size_t index) const {
    if (index == 0)
        return x;
    else if (index == 1)
        return y;
    else
        return 0.0f;
}

Math::Vec3::Vec3(const Vec4& vec4) :
    x(vec4.x),
    y(vec4.y),
    z(vec4.z) {}

Math::Vec3& Math::Vec3::operator=(const Vec3& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
        z = other.z;
    }
    return *this;
}

Math::Vec3& Math::Vec3::operator=(const Vec2& other) {
    x = other.x;
    y = other.y;
    z = 0.0f;
    return *this;
}

Math::Vec3& Math::Vec3::operator=(const Vec4& other) {
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
}

bool Math::Vec3::operator==(const Vec3& other) const {
    return x == other.x && y == other.y && z == other.z;
}

bool Math::Vec3::operator!=(const Vec3& other) const {
    return !(*this == other);
}

Math::Vec3 Math::Vec3::operator+(const Vec3& other) const {
    return Vec3(x + other.x, y + other.y, z + other.z);
}

Math::Vec3 Math::Vec3::operator-(const Vec3& other) const {
    return Vec3(x - other.x, y - other.y, z - other.z);
}

Math::Mat3 Math::Vec3::operator*(const Vec3& other) const {
    return Mat3(
        x * other.x, x * other.y, x * other.z,
        y * other.x, y * other.y, y * other.z,
        z * other.x, z * other.y, z * other.z
    );
}

Math::Vec3& Math::Vec3::operator+=(const Vec3& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Math::Vec3& Math::Vec3::operator-=(const Vec3& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

Math::Vec3 Math::Vec3::operator+(const float scalar) const {
    return Vec3(x + scalar, y + scalar, z + scalar);
}

Math::Vec3 Math::Vec3::operator-(const float scalar) const {
    return Vec3(x - scalar, y - scalar, z - scalar);
}

Math::Vec3 Math::Vec3::operator*(const float scalar) const {
    return Vec3(x * scalar, y * scalar, z * scalar);
}

Math::Vec3 Math::Vec3::operator/(const float scalar) const {
    return Vec3(x / scalar, y / scalar, z / scalar);
}

Math::Vec3& Math::Vec3::operator+=(const float scalar) {
    x += scalar;
    y += scalar;
    z += scalar;
    return *this;
}

Math::Vec3& Math::Vec3::operator-=(const float scalar) {
    x -= scalar;
    y -= scalar;
    z -= scalar;
    return *this;
}

Math::Vec3& Math::Vec3::operator*=(const float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

Math::Vec3& Math::Vec3::operator/=(const float scalar) {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
}

float Math::Vec3::operator[](size_t index) const {
    if (index == 0)
        return x;
    else if (index == 1)
        return y;
    else if (index == 2)
        return z;
    else
        return 0.0f;
}

Math::Vec4& Math::Vec4::operator=(const Vec4& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
        z = other.z;
        w = other.w;
    }
    return *this;
}

Math::Vec4& Math::Vec4::operator=(const Vec2& other) {
    x = other.x;
    y = other.y;
    z = 0.0f;
    w = 0.0f;
    return *this;
}

Math::Vec4& Math::Vec4::operator=(const Vec3& other) {
    x = other.x;
    y = other.y;
    z = other.z;
    w = 0.0f;
    return *this;
}

bool Math::Vec4::operator==(const Vec4& other) const {
    return x == other.x && y == other.y && z == other.z && w == other.w;
}

bool Math::Vec4::operator!=(const Vec4& other) const {
    return !(*this == other);
}

Math::Vec4 Math::Vec4::operator+(const Vec4& other) const {
    return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
}

Math::Vec4 Math::Vec4::operator-(const Vec4& other) const {
    return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
}

Math::Mat4 Math::Vec4::operator*(const Vec4& other) const {
    return Mat4(
        x * other.x, x * other.y, x * other.z, x * other.w,
        y * other.x, y * other.y, y * other.z, y * other.w,
        z * other.x, z * other.y, z * other.z, z * other.w,
        w * other.x, w * other.y, w * other.z, w * other.w
    );
}

Math::Vec4& Math::Vec4::operator+=(const Vec4& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
}

Math::Vec4& Math::Vec4::operator-=(const Vec4& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
}

Math::Vec4 Math::Vec4::operator+(const float scalar) const {
    return Vec4(x + scalar, y + scalar, z + scalar, w + scalar);
}

Math::Vec4 Math::Vec4::operator-(const float scalar) const {
    return Vec4(x - scalar, y - scalar, z - scalar, w - scalar);
}

Math::Vec4 Math::Vec4::operator*(const float scalar) const {
    return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
}

Math::Vec4 Math::Vec4::operator/(const float scalar) const {
    return Vec4(x / scalar, y / scalar, z / scalar, w / scalar);
}

Math::Vec4& Math::Vec4::operator+=(const float scalar) {
    x += scalar;
    y += scalar;
    z += scalar;
    w += scalar;
    return *this;
}

Math::Vec4& Math::Vec4::operator-=(const float scalar) {
    x -= scalar;
    y -= scalar;
    z -= scalar;
    w -= scalar;
    return *this;
}

Math::Vec4& Math::Vec4::operator*=(const float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
}

Math::Vec4& Math::Vec4::operator/=(const float scalar) {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
    return *this;
}

float Math::Vec4::operator[](size_t index) const {
    if (index == 0)
        return x;
    else if (index == 1)
        return y;
    else if (index == 2)
        return z;
    else if (index == 3)
        return w;
    else
        return 0.0f;
}

Math::Mat2::Mat2(const Mat3& other) :
    xx(other.xx), xy(other.xy),
    yx(other.yx), yy(other.yy) {}

Math::Mat2::Mat2(const Mat4& other) :
    xx(other.xx), xy(other.xy),
    yx(other.yx), yy(other.yy) {}

Math::Mat2& Math::Mat2::operator=(const Mat2& other) {
    if (this != &other) {
        xx = other.xx;
        xy = other.xy;
        yx = other.yx;
        yy = other.yy;
    }
    return *this;
}

Math::Mat2& Math::Mat2::operator=(const Mat3& other) {
    xx = other.xx;
    xy = other.xy;
    yx = other.yx;
    yy = other.yy;
    return *this;
}

Math::Mat2& Math::Mat2::operator=(const Mat4& other) {
    xx = other.xx;
    xy = other.xy;
    yx = other.yx;
    yy = other.yy;
    return *this;
}

bool Math::Mat2::operator==(const Mat2& other) const {
    bool result = xx == other.xx && xy == other.xy;
    result &= yx == other.yx && yy == other.yy;
    return result;
}

bool Math::Mat2::operator!=(const Mat2& other) const {
    return !(*this == other);
}

Math::Mat2 Math::Mat2::operator+(const Mat2& other) const {
    return Mat2(
        xx + other.xx, xy + other.xy,
        yx + other.yx, yy + other.yy
    );
}

Math::Mat2 Math::Mat2::operator-(const Mat2& other) const {
    return Mat2(
        xx - other.xx, xy - other.xy,
        yx - other.yx, yy - other.yy
    );
}

Math::Mat2 Math::Mat2::operator*(const Mat2& other) const {
    return Mat2(
        xx * other.xx + xy * other.yx, xx * other.xy + xy * other.yy,
        yx * other.xx + yy * other.yx, yx * other.xy + yy * other.yy
    );
}

Math::Mat2& Math::Mat2::operator+=(const Mat2& other) {
    xx += other.xx;
    xy += other.xy;
    yx += other.yx;
    yy += other.yy;
    return *this;
}

Math::Mat2& Math::Mat2::operator-=(const Mat2& other) {
    xx -= other.xx;
    xy -= other.xy;
    yx -= other.yx;
    yy -= other.yy;
    return *this;
}

Math::Mat2& Math::Mat2::operator*=(const Mat2& other) {
    *this = *this * other;
    return *this;
}

Math::Mat2 Math::Mat2::operator+(float scalar) const {
    return Mat2
    (
        xx + scalar, xy + scalar,
        yx + scalar, yy + scalar
    );
}

Math::Mat2 Math::Mat2::operator-(float scalar) const {
    return Mat2(
        xx - scalar, xy - scalar,
        yx - scalar, yy - scalar
    );
}

Math::Mat2 Math::Mat2::operator*(float scalar) const {
    return Mat2(
        xx * scalar, xy * scalar,
        yx * scalar, yy * scalar
    );
}

Math::Mat2 Math::Mat2::operator/(float scalar) const {
    return Mat2(
        xx / scalar, xy / scalar,
        yx / scalar, yy / scalar
    );
}

Math::Mat2& Math::Mat2::operator+=(float scalar) {
    xx += scalar;
    xy += scalar;
    yx += scalar;
    yy += scalar;
    return *this;
}

Math::Mat2& Math::Mat2::operator-=(float scalar) {
    xx -= scalar;
    xy -= scalar;
    yx -= scalar;
    yy -= scalar;
    return *this;
}

Math::Mat2& Math::Mat2::operator*=(float scalar) {
    xx *= scalar;
    xy *= scalar;
    yx *= scalar;
    yy *= scalar;
    return *this;
}

Math::Mat2& Math::Mat2::operator/=(float scalar) {
    xx /= scalar;
    xy /= scalar;
    yx /= scalar;
    yy /= scalar;
    return *this;
}

Math::Vec2 Math::Mat2::operator*(const Vec2& vec) const {
    return Vec2(
        xx * vec.x + xy * vec.y,
        yx * vec.x + yy * vec.y
    );
}

Math::Vec2 Math::Mat2::operator[](size_t index) const {
    if (index == 0)
        return Vec2(xx, yx);
    else if (index == 1)
        return Vec2(xy, yy);
    else
        return Vec2(0.0f, 0.0f);
}

Math::Mat3::Mat3(const Mat4& other) :
    xx(other.xx), xy(other.xy), xz(other.xz),
    yx(other.yx), yy(other.yy), yz(other.yz),
    zx(other.zx), zy(other.zy), zz(other.zz) {}

Math::Mat3& Math::Mat3::operator=(const Mat3& other) {
    if (this != &other) {
        xx = other.xx; xy = other.xy; xz = other.xz;
        yx = other.yx; yy = other.yy; yz = other.yz;
        zx = other.zx; zy = other.zy; zz = other.zz;
    }
    return *this;
}

Math::Mat3& Math::Mat3::operator=(const Mat2& other) {
    xx = other.xx; xy = other.xy; xz = 0.0f;
    yx = other.yx; yy = other.yy; yz = 0.0f;
    zx = 0.0f;     zy = 0.0f;     zz = 0.0f;
    return *this;
}

Math::Mat3& Math::Mat3::operator=(const Mat4& other) {
    xx = other.xx; xy = other.xy; xz = other.xz;
    yx = other.yx; yy = other.yy; yz = other.yz;
    zx = other.zx; zy = other.zy; zz = other.zz;
    return *this;
}

bool Math::Mat3::operator==(const Mat3& other) const {
    bool result = xx == other.xx && xy == other.xy && xz == other.xz;
    result &= yx == other.yx && yy == other.yy && yz == other.yz;
    result &= zx == other.zx && zy == other.zy && zz == other.zz;
    return result;
}

bool Math::Mat3::operator!=(const Mat3& other) const {
    return !(*this == other);
}

Math::Mat3 Math::Mat3::operator+(const Mat3& other) const {
    return Mat3(
        xx + other.xx, xy + other.xy, xz + other.xz,
        yx + other.yx, yy + other.yy, yz + other.yz,
        zx + other.zx, zy + other.zy, zz + other.zz
    );
}

Math::Mat3 Math::Mat3::operator-(const Mat3& other) const {
    return Mat3(
        xx - other.xx, xy - other.xy, xz - other.xz,
        yx - other.yx, yy - other.yy, yz - other.yz,
        zx - other.zx, zy - other.zy, zz - other.zz
    );
}

Math::Mat3 Math::Mat3::operator*(const Mat3& other) const {
    return Mat3(
        xx * other.xx + xy * other.yx + xz * other.zx,
        xx * other.xy + xy * other.yy + xz * other.zy,
        xx * other.xz + xy * other.yz + xz * other.zz,

        yx * other.xx + yy * other.yx + yz * other.zx,
        yx * other.xy + yy * other.yy + yz * other.zy,
        yx * other.xz + yy * other.yz + yz * other.zz,

        zx * other.xx + zy * other.yx + zz * other.zx,
        zx * other.xy + zy * other.yy + zz * other.zy,
        zx * other.xz + zy * other.yz + zz * other.zz
    );
}

Math::Mat3& Math::Mat3::operator+=(const Mat3& other) {
    xx += other.xx; xy += other.xy; xz += other.xz;
    yx += other.yx; yy += other.yy; yz += other.yz;
    zx += other.zx; zy += other.zy; zz += other.zz;
    return *this;
}

Math::Mat3& Math::Mat3::operator-=(const Mat3& other) {
    xx -= other.xx; xy -= other.xy; xz -= other.xz;
    yx -= other.yx; yy -= other.yy; yz -= other.yz;
    zx -= other.zx; zy -= other.zy; zz -= other.zz;
    return *this;
}

Math::Mat3& Math::Mat3::operator*=(const Mat3& other) {
    *this = *this * other;
    return *this;
}

Math::Mat3 Math::Mat3::operator+(float scalar) const {
    return Mat3(
        xx + scalar, xy + scalar, xz + scalar,
        yx + scalar, yy + scalar, yz + scalar,
        zx + scalar, zy + scalar, zz + scalar
    );
}

Math::Mat3 Math::Mat3::operator-(float scalar) const {
    return Mat3(
        xx - scalar, xy - scalar, xz - scalar,
        yx - scalar, yy - scalar, yz - scalar,
        zx - scalar, zy - scalar, zz - scalar
    );
}

Math::Mat3 Math::Mat3::operator*(float scalar) const {
    return Mat3(
        xx * scalar, xy * scalar, xz * scalar,
        yx * scalar, yy * scalar, yz * scalar,
        zx * scalar, zy * scalar, zz * scalar
    );
}

Math::Mat3 Math::Mat3::operator/(float scalar) const {
    return Mat3(
        xx / scalar, xy / scalar, xz / scalar,
        yx / scalar, yy / scalar, yz / scalar,
        zx / scalar, zy / scalar, zz / scalar
    );
}

Math::Mat3& Math::Mat3::operator+=(float scalar) {
    xx += scalar; xy += scalar; xz += scalar;
    yx += scalar; yy += scalar; yz += scalar;
    zx += scalar; zy += scalar; zz += scalar;
    return *this;
}

Math::Mat3& Math::Mat3::operator-=(float scalar) {
    xx -= scalar; xy -= scalar; xz -= scalar;
    yx -= scalar; yy -= scalar; yz -= scalar;
    zx -= scalar; zy -= scalar; zz -= scalar;
    return *this;
}

Math::Mat3& Math::Mat3::operator*=(float scalar) {
    xx *= scalar; xy *= scalar; xz *= scalar;
    yx *= scalar; yy *= scalar; yz *= scalar;
    zx *= scalar; zy *= scalar; zz *= scalar;
    return *this;
}

Math::Mat3& Math::Mat3::operator/=(float scalar) {
    xx /= scalar; xy /= scalar; xz /= scalar;
    yx /= scalar; yy /= scalar; yz /= scalar;
    zx /= scalar; zy /= scalar; zz /= scalar;
    return *this;
}

Math::Vec3 Math::Mat3::operator*(const Vec3& vec) const {
    return Vec3(
        xx * vec.x + xy * vec.y + xz * vec.z,
        yx * vec.x + yy * vec.y + yz * vec.z,
        zx * vec.x + zy * vec.y + zz * vec.z
    );
}

Math::Vec3 Math::Mat3::operator[](size_t index) const {
    if (index == 0)
        return Vec3(xx, yx, zx);
    else if (index == 1)
        return Vec3(xy, yy, zy);
    else if (index == 2)
        return Vec3(xz, yz, zz);
    else
        return Vec3(0.0f, 0.0f, 0.0f);
}

Math::Mat4& Math::Mat4::operator=(const Mat4& other) {
    if (this != &other) {
        xx = other.xx; xy = other.xy; xz = other.xz; xw = other.xw;
        yx = other.yx; yy = other.yy; yz = other.yz; yw = other.yw;
        zx = other.zx; zy = other.zy; zz = other.zz; zw = other.zw;
        wx = other.wx; wy = other.wy; wz = other.wz; ww = other.ww;
    }
    return *this;
}

Math::Mat4& Math::Mat4::operator=(const Mat2& other) {
    xx = other.xx; xy = other.xy; xz = 0.0f;   xw = 0.0f;
    yx = other.yx; yy = other.yy; yz = 0.0f;   yw = 0.0f;
    zx = 0.0f;     zy = 0.0f;     zz = 0.0f;   zw = 0.0f;
    wx = 0.0f;     wy = 0.0f;     wz = 0.0f;   ww = 0.0f;
    return *this;
}

Math::Mat4& Math::Mat4::operator=(const Mat3& other) {
    xx = other.xx; xy = other.xy; xz = other.xz; xw = 0.0f;
    yx = other.yx; yy = other.yy; yz = other.yz; yw = 0.0f;
    zx = other.zx; zy = other.zy; zz = other.zz; zw = 0.0f;
    wx = 0.0f;     wy = 0.0f;     wz = 0.0f;     ww = 0.0f;
    return *this;
}

bool Math::Mat4::operator==(const Mat4& other) const {
    bool result = xx == other.xx && xy == other.xy && xz == other.xz && xw == other.xw;
    result &= yx == other.yx && yy == other.yy && yz == other.yz && yw == other.yw;
    result &= zx == other.zx && zy == other.zy && zz == other.zz && zw == other.zw;
    result &= wx == other.wx && wy == other.wy && wz == other.wz && ww == other.ww;
    return result;
}

bool Math::Mat4::operator!=(const Mat4& other) const {
    return !(*this == other);
}

Math::Mat4 Math::Mat4::operator+(const Mat4& other) const {
    return Mat4(
        xx + other.xx, xy + other.xy, xz + other.xz, xw + other.xw,
        yx + other.yx, yy + other.yy, yz + other.yz, yw + other.yw,
        zx + other.zx, zy + other.zy, zz + other.zz, zw + other.zw,
        wx + other.wx, wy + other.wy, wz + other.wz, ww + other.ww
    );
}

Math::Mat4 Math::Mat4::operator-(const Mat4& other) const {
    return Mat4(
        xx - other.xx, xy - other.xy, xz - other.xz, xw - other.xw,
        yx - other.yx, yy - other.yy, yz - other.yz, yw - other.yw,
        zx - other.zx, zy - other.zy, zz - other.zz, zw - other.zw,
        wx - other.wx, wy - other.wy, wz - other.wz, ww - other.ww
    );
}

Math::Mat4 Math::Mat4::operator*(const Mat4& other) const {
    return Mat4(
        xx * other.xx + xy * other.yx + xz * other.zx + xw * other.wx,
        xx * other.xy + xy * other.yy + xz * other.zy + xw * other.wy,
        xx * other.xz + xy * other.yz + xz * other.zz + xw * other.wz,
        xx * other.xw + xy * other.yw + xz * other.zw + xw * other.ww,

        yx * other.xx + yy * other.yx + yz * other.zx + yw * other.wx,
        yx * other.xy + yy * other.yy + yz * other.zy + yw * other.wy,
        yx * other.xz + yy * other.yz + yz * other.zz + yw * other.wz,
        yx * other.xw + yy * other.yw + yz * other.zw + yw * other.ww,

        zx * other.xx + zy * other.yx + zz * other.zx + zw * other.wx,
        zx * other.xy + zy * other.yy + zz * other.zy + zw * other.wy,
        zx * other.xz + zy * other.yz + zz * other.zz + zw * other.wz,
        zx * other.xw + zy * other.yw + zz * other.zw + zw * other.ww,

        wx * other.xx + wy * other.yx + wz * other.zx + ww * other.wx,
        wx * other.xy + wy * other.yy + wz * other.zy + ww * other.wy,
        wx * other.xz + wy * other.yz + wz * other.zz + ww * other.wz,
        wx * other.xw + wy * other.yw + wz * other.zw + ww * other.ww
    );
}

Math::Mat4& Math::Mat4::operator+=(const Mat4& other) {
    xx += other.xx; xy += other.xy; xz += other.xz; xw += other.xw;
    yx += other.yx; yy += other.yy; yz += other.yz; yw += other.yw;
    zx += other.zx; zy += other.zy; zz += other.zz; zw += other.zw;
    wx += other.wx; wy += other.wy; wz += other.wz; ww += other.ww;
    return *this;
}

Math::Mat4& Math::Mat4::operator-=(const Mat4& other) {
    xx -= other.xx; xy -= other.xy; xz -= other.xz; xw -= other.xw;
    yx -= other.yx; yy -= other.yy; yz -= other.yz; yw -= other.yw;
    zx -= other.zx; zy -= other.zy; zz -= other.zz; zw -= other.zw;
    wx -= other.wx; wy -= other.wy; wz -= other.wz; ww -= other.ww;
    return *this;
}

Math::Mat4& Math::Mat4::operator*=(const Mat4& other) {
    *this = *this * other;
    return *this;
}

Math::Mat4 Math::Mat4::operator+(float scalar) const {
    return Mat4(
        xx + scalar, xy + scalar, xz + scalar, xw + scalar,
        yx + scalar, yy + scalar, yz + scalar, yw + scalar,
        zx + scalar, zy + scalar, zz + scalar, zw + scalar,
        wx + scalar, wy + scalar, wz + scalar, ww + scalar
    );
}

Math::Mat4 Math::Mat4::operator-(float scalar) const {
    return Mat4(
        xx - scalar, xy - scalar, xz - scalar, xw - scalar,
        yx - scalar, yy - scalar, yz - scalar, yw - scalar,
        zx - scalar, zy - scalar, zz - scalar, zw - scalar,
        wx - scalar, wy - scalar, wz - scalar, ww - scalar
    );
}

Math::Mat4 Math::Mat4::operator*(float scalar) const {
    return Mat4(
        xx * scalar, xy * scalar, xz * scalar, xw * scalar,
        yx * scalar, yy * scalar, yz * scalar, yw * scalar,
        zx * scalar, zy * scalar, zz * scalar, zw * scalar,
        wx * scalar, wy * scalar, wz * scalar, ww * scalar
    );
}

Math::Mat4 Math::Mat4::operator/(float scalar) const {
    return Mat4(
        xx / scalar, xy / scalar, xz / scalar, xw / scalar,
        yx / scalar, yy / scalar, yz / scalar, yw / scalar,
        zx / scalar, zy / scalar, zz / scalar, zw / scalar,
        wx / scalar, wy / scalar, wz / scalar, ww / scalar
    );
}

Math::Mat4& Math::Mat4::operator+=(float scalar) {
    xx += scalar; xy += scalar; xz += scalar; xw += scalar;
    yx += scalar; yy += scalar; yz += scalar; yw += scalar;
    zx += scalar; zy += scalar; zz += scalar; zw += scalar;
    wx += scalar; wy += scalar; wz += scalar; ww += scalar;
    return *this;
}

Math::Mat4& Math::Mat4::operator-=(float scalar) {
    xx -= scalar; xy -= scalar; xz -= scalar; xw -= scalar;
    yx -= scalar; yy -= scalar; yz -= scalar; yw -= scalar;
    zx -= scalar; zy -= scalar; zz -= scalar; zw -= scalar;
    wx -= scalar; wy -= scalar; wz -= scalar; ww -= scalar;
    return *this;
}

Math::Mat4& Math::Mat4::operator*=(float scalar) {
    xx *= scalar; xy *= scalar; xz *= scalar; xw *= scalar;
    yx *= scalar; yy *= scalar; yz *= scalar; yw *= scalar;
    zx *= scalar; zy *= scalar; zz *= scalar; zw *= scalar;
    wx *= scalar; wy *= scalar; wz *= scalar; ww *= scalar;
    return *this;
}

Math::Mat4& Math::Mat4::operator/=(float scalar) {
    xx /= scalar; xy /= scalar; xz /= scalar; xw /= scalar;
    yx /= scalar; yy /= scalar; yz /= scalar; yw /= scalar;
    zx /= scalar; zy /= scalar; zz /= scalar; zw /= scalar;
    wx /= scalar; wy /= scalar; wz /= scalar; ww /= scalar;
    return *this;
}

Math::Vec4 Math::Mat4::operator*(const Vec4& vec) const {
    return Vec4(
        xx * vec.x + xy * vec.y + xz * vec.z + xw * vec.w,
        yx * vec.x + yy * vec.y + yz * vec.z + yw * vec.w,
        zx * vec.x + zy * vec.y + zz * vec.z + zw * vec.w,
        wx * vec.x + wy * vec.y + wz * vec.z + ww * vec.w
    );
}

Math::Vec4 Math::Mat4::operator[](size_t index) const {
    if (index == 0)
        return Vec4(xx, yx, zx, wx);
    else if (index == 1)
        return Vec4(xy, yy, zy, wy);
    else if (index == 2)
        return Vec4(xz, yz, zz, wz);
    else if (index == 3)
        return Vec4(xw, yw, zw, ww);
    else
        return Vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

Math::Vec3 Math::cross(const Vec3& a, const Vec3& b) {
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

Math::Mat2 Math::transpose(const Mat2& mat) {
    return Mat2(
        mat.xx, mat.yx,
        mat.xy, mat.yy
    );
}

Math::Mat3 Math::transpose(const Mat3& mat) {
    return Mat3(
        mat.xx, mat.yx, mat.zx,
        mat.xy, mat.yy, mat.zy,
        mat.xz, mat.yz, mat.zz
    );
}

Math::Mat4 Math::transpose(const Mat4& mat) {
    return Mat4(
        mat.xx, mat.yx, mat.zx, mat.wx,
        mat.xy, mat.yy, mat.zy, mat.wy,
        mat.xz, mat.yz, mat.zz, mat.wz,
        mat.xw, mat.yw, mat.zw, mat.ww
    );
}

float Math::determinant(const Mat2& mat) {
    return mat.xx * mat.yy - mat.xy * mat.yx;
}

float Math::determinant(const Mat3& mat) {
    float result = mat.xx * (mat.yy * mat.zz - mat.yz * mat.zy);
    result -= mat.xy * (mat.yx * mat.zz - mat.yz * mat.zx);
    result += mat.xz * (mat.yx * mat.zy - mat.yy * mat.zx);
    return result;
}

float Math::determinant(const Mat4& mat) {
    float det = 0.0f;

    float detXX =
        mat.yy * (mat.zz * mat.ww - mat.zw * mat.wz) -
        mat.yz * (mat.zy * mat.ww - mat.zw * mat.wy) +
        mat.yw * (mat.zy * mat.wz - mat.zz * mat.wy);
    det += mat.xx * detXX;

    float detXY =
        mat.yx * (mat.zz * mat.ww - mat.zw * mat.wz) -
        mat.yz * (mat.zx * mat.ww - mat.zw * mat.wx) +
        mat.yw * (mat.zx * mat.wz - mat.zz * mat.wx);
    det -= mat.xy * detXY;

    float detXZ =
        mat.yx * (mat.zy * mat.ww - mat.zw * mat.wy) -
        mat.yy * (mat.zx * mat.ww - mat.zw * mat.wx) +
        mat.yw * (mat.zx * mat.wy - mat.zy * mat.wx);
    det += mat.xz * detXZ;

    float detXW =
        mat.yx * (mat.zy * mat.wz - mat.zz * mat.wy) -
        mat.yy * (mat.zx * mat.wz - mat.zz * mat.wx) +
        mat.yz * (mat.zx * mat.wy - mat.zy * mat.wx);
    det -= mat.xw * detXW;

    return det;
}

Math::Mat2 Math::inverse(const Mat2& mat) {
    float det = determinant(mat);
    if (det == 0.0f)
        return Mat2(1.0);
    float invDet = 1.0f / det;
    return Mat2(
        mat.yy * invDet, -mat.xy * invDet,
        -mat.yx * invDet, mat.xx * invDet
    );
}

Math::Mat3 Math::inverse(const Mat3& mat) {
    float det = determinant(mat);
    if (det == 0.0f)
        return Mat3(1.0);

    float invDet = 1.0f / det;

    return Mat3(
        (mat.yy * mat.zz - mat.yz * mat.zy) * invDet,
        (mat.xz * mat.zy - mat.xy * mat.zz) * invDet,
        (mat.xy * mat.yz - mat.xz * mat.yy) * invDet,

        (mat.yz * mat.zx - mat.yx * mat.zz) * invDet,
        (mat.xx * mat.zz - mat.xz * mat.zx) * invDet,
        (mat.xz * mat.yx - mat.xx * mat.yz) * invDet,

        (mat.yx * mat.zy - mat.yy * mat.zx) * invDet,
        (mat.xy * mat.zx - mat.xx * mat.zy) * invDet,
        (mat.xx * mat.yy - mat.xy * mat.yx) * invDet
    );
}

Math::Mat4 Math::inverse(const Mat4& mat) {
    float det = determinant(mat);
    if (det == 0.0f)
        return Mat4(1.0f);

    float invDet = 1.0f / det;
    Mat4 adj;

    const Mat4& a = mat;

    auto det2 = [](float a, float b, float c, float d) {
        return a * d - b * c;
        };

    adj.xx =
        det2(a.yy, a.yz, a.zy, a.zz) * a.ww +
        det2(a.yz, a.yw, a.zz, a.zw) * a.wy +
        det2(a.yw, a.yy, a.zw, a.zy) * a.wz;
    adj.xy =
        -det2(a.xy, a.xz, a.zy, a.zz) * a.ww -
        det2(a.xz, a.xw, a.zz, a.zw) * a.wy -
        det2(a.xw, a.xy, a.zw, a.zy) * a.wz;
    adj.xz =
        det2(a.xy, a.xz, a.yy, a.yz) * a.ww +
        det2(a.xz, a.xw, a.yz, a.yw) * a.wy +
        det2(a.xw, a.xy, a.yw, a.yy) * a.wz;
    adj.xw =
        -det2(a.xy, a.xz, a.yy, a.yz) * a.wz -
        det2(a.xz, a.xw, a.yz, a.yw) * a.wy -
        det2(a.xw, a.xy, a.yw, a.yy) * a.ww;

    adj.yx =
        -det2(a.yx, a.yz, a.zx, a.zz) * a.ww -
        det2(a.yz, a.yw, a.zz, a.zw) * a.wx -
        det2(a.yw, a.yx, a.zw, a.zx) * a.wz;
    adj.yy =
        det2(a.xx, a.xz, a.zx, a.zz) * a.ww +
        det2(a.xz, a.xw, a.zz, a.zw) * a.wx +
        det2(a.xw, a.xx, a.zw, a.zx) * a.wz;
    adj.yz =
        -det2(a.xx, a.xz, a.yx, a.yz) * a.ww -
        det2(a.xz, a.xw, a.yz, a.yw) * a.wx -
        det2(a.xw, a.xx, a.yw, a.yx) * a.wz;
    adj.yw =
        det2(a.xx, a.xz, a.yx, a.yz) * a.wz +
        det2(a.xz, a.xw, a.yz, a.yw) * a.wx +
        det2(a.xw, a.xx, a.yw, a.yx) * a.ww;

    adj.zx =
        det2(a.yx, a.yy, a.zx, a.zy) * a.ww +
        det2(a.yy, a.yw, a.zy, a.zw) * a.wx +
        det2(a.yw, a.yx, a.zw, a.zx) * a.wy;
    adj.zy =
        -det2(a.xx, a.xy, a.zx, a.zy) * a.ww -
        det2(a.xy, a.xw, a.zy, a.zw) * a.wx -
        det2(a.xw, a.xx, a.zw, a.zx) * a.wy;
    adj.zz =
        det2(a.xx, a.xy, a.yx, a.yy) * a.ww +
        det2(a.xy, a.xw, a.yy, a.yw) * a.wx +
        det2(a.xw, a.xx, a.yw, a.yx) * a.wy;
    adj.zw =
        -det2(a.xx, a.xy, a.yx, a.yy) * a.wy -
        det2(a.xy, a.xw, a.yy, a.yw) * a.wx -
        det2(a.xw, a.xx, a.yw, a.yx) * a.ww;

    adj.wx =
        -det2(a.yx, a.yy, a.zx, a.zy) * a.wz -
        det2(a.yy, a.yz, a.zy, a.zz) * a.wx -
        det2(a.yz, a.yx, a.zz, a.zx) * a.wy;
    adj.wy =
        det2(a.xx, a.xy, a.zx, a.zy) * a.wz +
        det2(a.xy, a.xz, a.zy, a.zz) * a.wx +
        det2(a.xz, a.xx, a.zz, a.zx) * a.wy;
    adj.wz =
        -det2(a.xx, a.xy, a.yx, a.yy) * a.wz -
        det2(a.xy, a.xz, a.yy, a.yz) * a.wx -
        det2(a.xz, a.xx, a.yz, a.yx) * a.wy;
    adj.ww =
        det2(a.xx, a.xy, a.yx, a.yy) * a.zz +
        det2(a.xy, a.xz, a.yy, a.yz) * a.zx +
        det2(a.xz, a.xx, a.yz, a.yx) * a.zy;

    for (int i = 0; i < 16; i++)
        (&adj.xx)[i] *= invDet;

    return adj;
}

Math::Mat3 Math::scale(const Mat3& mat, const Vec2& scale) {
    Mat3 scaleMat(
        scale.x, 0.0f, 0.0f,
        0.0f, scale.y, 0.0f,
        0.0f, 0.0f, 1.0f
    );
    return mat * scaleMat;
}

Math::Mat4 Math::scale(const Mat4& mat, const Vec3& scale) {
    Mat4 scaleMat(
        scale.x, 0.0f, 0.0f, 0.0f,
        0.0f, scale.y, 0.0f, 0.0f,
        0.0f, 0.0f, scale.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    return mat * scaleMat;
}

Math::Mat3 Math::translate(const Mat3& mat, const Vec2& translation) {
    Mat3 translationMat(
        1.0f, 0.0f, translation.x,
        0.0f, 1.0f, translation.y,
        0.0f, 0.0f, 1.0f
    );
    return mat * translationMat;
}

Math::Mat4 Math::translate(const Mat4& mat, const Vec3& translation) {
    Mat4 translationMat(
        1.0f, 0.0f, 0.0f, translation.x,
        0.0f, 1.0f, 0.0f, translation.y,
        0.0f, 0.0f, 1.0f, translation.z,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    return mat * translationMat;
}

Math::Mat3 Math::rotate(const Mat3& mat, float angle, const Vec3& axis) {
    float radians = angle * PI / 180.0f;
    float c = cos(radians);
    float s = sin(radians);
    float t = 1.0f - c;
    Vec3 normAxis = normalize(axis);
    float x = normAxis.x;
    float y = normAxis.y;
    float z = normAxis.z;
    Mat3 rotation(
        t * x * x + c, t * x * y - s * z, t * x * z + s * y,
        t * x * y + s * z, t * y * y + c, t * y * z - s * x,
        t * x * z - s * y, t * y * z + s * x, t * z * z + c
    );
    return mat * rotation;
}

Math::Mat4 Math::rotate(const Mat4& mat, float angle, const Vec3& axis) {
    float radians = angle * PI / 180.0f;
    float c = cos(radians);
    float s = sin(radians);
    float t = 1.0f - c;
    Vec3 normAxis = normalize(axis);
    float x = normAxis.x;
    float y = normAxis.y;
    float z = normAxis.z;
    Mat4 rotation(
        t * x * x + c, t * x * y - s * z, t * x * z + s * y, 0.0f,
        t * x * y + s * z, t * y * y + c, t * y * z - s * x, 0.0f,
        t * x * z - s * y, t * y * z + s * x, t * z * z + c, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    return mat * rotation;
}

Math::Mat3 Math::scale(const Mat3& mat, float scale) {
    Mat3 scaleMat(
        scale, 0.0f, 0.0f,
        0.0f, scale, 0.0f,
        0.0f, 0.0f, 1.0f
    );
    return mat * scaleMat;
}

Math::Mat4 Math::scale(const Mat4& mat, float scale) {
    Mat4 scaleMat(
        scale, 0.0f, 0.0f, 0.0f,
        0.0f, scale, 0.0f, 0.0f,
        0.0f, 0.0f, scale, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    return mat * scaleMat;
}

Math::Mat4 Math::lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Vec3 f = normalize(center - eye);
    Vec3 s = normalize(cross(f, up));
    Vec3 u = cross(s, f);
    Mat4 result(1.0f);
    result.xx = s.x;
    result.xy = s.y;
    result.xz = s.z;
    result.yx = u.x;
    result.yy = u.y;
    result.yz = u.z;
    result.zx = -f.x;
    result.zy = -f.y;
    result.zz = -f.z;
    result.xw = -dot(s, eye);
    result.yw = -dot(u, eye);
    result.zw = dot(f, eye);
    return result;
}

Math::Mat4 Math::perspective
(
    float fov,
    float aspect,
    float near,
    float far,
    DepthRange depthRange
) {
    float tanHalfFov = tan(fov / 2.0f);
    Mat4 result(0.0f);
    result.xx = 1.0f / (aspect * tanHalfFov);
    result.yy = 1.0f / tanHalfFov;
    if (depthRange == DepthRange::ZERO_TO_ONE) {
        result.zz = far / (near - far);
        result.zw = -(far * near) / (far - near);
    } else { // DepthRange::MINUS_ONE_TO_ONE
        result.zz = -(far + near) / (far - near);
        result.zw = -(2.0f * far * near) / (far - near);
    }
    result.wz = -1.0f;
    return result;
}

Math::Mat4 Math::orthographic(float left, float right, float bottom, float top, float near, float far) {
    Mat4 result(1.0f);
    result.xx = 2.0f / (right - left);
    result.yy = 2.0f / (top - bottom);
    result.zz = -2.0f / (far - near);
    result.xw = -(right + left) / (right - left);
    result.yw = -(top + bottom) / (top - bottom);
    result.zw = -(far + near) / (far - near);
    return result;
}
