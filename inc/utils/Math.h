/**
 * @file Math.h
 * @brief Header file defining mathematical structures.
 */

#pragma once

#include "UtilsCommon.h"

namespace Math {

constexpr float PI = 3.14159265358979323846f;

struct Mat2;
struct Mat3;
struct Mat4;

struct Vec3;
struct Vec4;

struct Vec2 {
    union {
        struct { float x, y; };
        struct { float r, g; };
        struct { float s, t; };
    };

    Vec2() :
        x(0.0f),
        y(0.0f) {};
    Vec2(float x, float y) :
        x(x),
        y(y) {};
    Vec2(float scalar) :
        x(scalar),
        y(scalar) {};
    Vec2(const Vec3& vec3);
    Vec2(const Vec4& vec4);

    Vec2(const Vec2& other) :
        x(other.x),
        y(other.y) {};

    Vec2& operator=(const Vec2& other);
    Vec2& operator=(const Vec3& other);
    Vec2& operator=(const Vec4& other);

    bool operator==(const Vec2& other) const;
    bool operator!=(const Vec2& other) const;

    Vec2 operator+(const Vec2& other) const;
    Vec2 operator-(const Vec2& other) const;
    Mat2 operator*(const Vec2& other) const;

    Vec2& operator+=(const Vec2& other);
    Vec2& operator-=(const Vec2& other);

    Vec2 operator+(const float scalar) const;
    Vec2 operator-(const float scalar) const;
    Vec2 operator*(const float scalar) const;
    Vec2 operator/(const float scalar) const;

    Vec2& operator+=(const float scalar);
    Vec2& operator-=(const float scalar);
    Vec2& operator*=(const float scalar);
    Vec2& operator/=(const float scalar);

    float operator[](size_t index) const;
};

struct Vec3 {
    union {
        struct { float x, y, z; };
        struct { float r, g, b; };
        struct { float s, t, p; };
    };

    Vec3() :
        x(0.0f),
        y(0.0f),
        z(0.0f) {};
    Vec3(float x, float y, float z) :
        x(x),
        y(y),
        z(z) {};
    Vec3(float scalar) :
        x(scalar),
        y(scalar),
        z(scalar) {};
    Vec3(const Vec2& vec2, float z = 0.0f) :
        x(vec2.x),
        y(vec2.y),
        z(z) {};
    Vec3(float x, const Vec2& vec2) :
        x(x),
        y(vec2.x),
        z(vec2.y) {};
    Vec3(const Vec4& vec4);

    Vec3(const Vec3& other) :
        x(other.x),
        y(other.y),
        z(other.z) {};

    Vec3& operator=(const Vec3& other);
    Vec3& operator=(const Vec2& other);
    Vec3& operator=(const Vec4& other);

    bool operator==(const Vec3& other) const;
    bool operator!=(const Vec3& other) const;

    Vec3 operator+(const Vec3& other) const;
    Vec3 operator-(const Vec3& other) const;
    Mat3 operator*(const Vec3& other) const;

    Vec3& operator+=(const Vec3& other);
    Vec3& operator-=(const Vec3& other);

    Vec3 operator+(const float scalar) const;
    Vec3 operator-(const float scalar) const;
    Vec3 operator*(const float scalar) const;
    Vec3 operator/(const float scalar) const;

    Vec3& operator+=(const float scalar);
    Vec3& operator-=(const float scalar);
    Vec3& operator*=(const float scalar);
    Vec3& operator/=(const float scalar);

    float operator[](size_t index) const;
};

struct Vec4 {
    union {
        struct { float x, y, z, w; };
        struct { float r, g, b, a; };
        struct { float s, t, p, q; };
    };

    Vec4() :
        x(0.0f),
        y(0.0f),
        z(0.0f),
        w(0.0f) {};
    Vec4(float x, float y, float z, float w) :
        x(x),
        y(y),
        z(z),
        w(w) {};
    Vec4(float scalar) :
        x(scalar),
        y(scalar),
        z(scalar),
        w(scalar) {};
    Vec4(const Vec2& vec2, float z = 0.0f, float w = 0.0f) :
        x(vec2.x),
        y(vec2.y),
        z(z),
        w(w) {};
    Vec4(float x, const Vec2& vec2, float w) :
        x(x),
        y(vec2.x),
        z(vec2.y),
        w(w) {};
    Vec4(float x, float y, const Vec2& vec2) :
        x(x),
        y(y),
        z(vec2.x),
        w(vec2.y) {};
    Vec4(const Vec2& vec2_1, const Vec2& vec2_2) :
        x(vec2_1.x),
        y(vec2_1.y),
        z(vec2_2.x),
        w(vec2_2.y) {};
    Vec4(const Vec3& vec3, float w = 0.0f) :
        x(vec3.x),
        y(vec3.y),
        z(vec3.z),
        w(w) {};
    Vec4(float x, const Vec3& vec3) :
        x(x),
        y(vec3.x),
        z(vec3.y),
        w(vec3.z) {};

    Vec4(const Vec4& other) :
        x(other.x),
        y(other.y),
        z(other.z),
        w(other.w) {};

    Vec4& operator=(const Vec4& other);
    Vec4& operator=(const Vec2& other);
    Vec4& operator=(const Vec3& other);

    bool operator==(const Vec4& other) const;
    bool operator!=(const Vec4& other) const;

    Vec4 operator+(const Vec4& other) const;
    Vec4 operator-(const Vec4& other) const;
    Mat4 operator*(const Vec4& other) const;

    Vec4& operator+=(const Vec4& other);
    Vec4& operator-=(const Vec4& other);

    Vec4 operator+(const float scalar) const;
    Vec4 operator-(const float scalar) const;
    Vec4 operator*(const float scalar) const;
    Vec4 operator/(const float scalar) const;

    Vec4& operator+=(const float scalar);
    Vec4& operator-=(const float scalar);
    Vec4& operator*=(const float scalar);
    Vec4& operator/=(const float scalar);

    float operator[](size_t index) const;
};

template<typename VecType>
float dot(const VecType& a, const VecType& b) {
    if constexpr (std::is_same_v<VecType, Vec2>)
        return a.x * b.x + a.y * b.y;
    else if constexpr (std::is_same_v<VecType, Vec3>)
        return a.x * b.x + a.y * b.y + a.z * b.z;
    else if constexpr (std::is_same_v<VecType, Vec4>)
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    else
        return 0.0f;
}

Vec3 cross(const Vec3& a, const Vec3& b);

template<typename VecType>
float length(const VecType& vec) {
    return std::sqrt(dot(vec, vec));
}

template<typename VecType>
VecType normalize(const VecType& vec) {
    float len = length(vec);
    if (len == 0.0f)
        return vec; // Avoid division by zero
    return vec / len;
}

struct Mat2 {
    float xx = 0.0f, yx = 0.0f;
    float xy = 0.0f, yy = 0.0f;

    Mat2() = default;
    Mat2(
        float xx, float xy,
        float yx, float yy
    ) :
        xx(xx), xy(xy),
        yx(yx), yy(yy) {};
    Mat2(float diagonal) :
        xx(diagonal), xy(0.0f),
        yx(0.0f), yy(diagonal) {};
    Mat2(const Vec2& col1, const Vec2& col2) :
        xx(col1.x), xy(col2.x),
        yx(col1.y), yy(col2.y) {};

    Mat2(const Mat2& other) :
        xx(other.xx), xy(other.xy),
        yx(other.yx), yy(other.yy) {};

    Mat2(const Mat3& other);
    Mat2(const Mat4& other);

    Mat2& operator=(const Mat2& other);
    Mat2& operator=(const Mat3& other);
    Mat2& operator=(const Mat4& other);

    bool operator==(const Mat2& other) const;
    bool operator!=(const Mat2& other) const;

    Mat2 operator+(const Mat2& other) const;
    Mat2 operator-(const Mat2& other) const;
    Mat2 operator*(const Mat2& other) const;

    Mat2& operator+=(const Mat2& other);
    Mat2& operator-=(const Mat2& other);
    Mat2& operator*=(const Mat2& other);

    Mat2 operator+(float scalar) const;
    Mat2 operator-(float scalar) const;
    Mat2 operator*(float scalar) const;
    Mat2 operator/(float scalar) const;

    Mat2& operator+=(float scalar);
    Mat2& operator-=(float scalar);
    Mat2& operator*=(float scalar);
    Mat2& operator/=(float scalar);

    Vec2 operator*(const Vec2& vec) const;

    Vec2 operator[](size_t index) const;
};

struct Mat3 {
    float xx = 0.0f, yx = 0.0f, zx = 0.0f;
    float xy = 0.0f, yy = 0.0f, zy = 0.0f;
    float xz = 0.0f, yz = 0.0f, zz = 0.0f;

    Mat3() = default;
    Mat3(
        float xx, float xy, float xz,
        float yx, float yy, float yz,
        float zx, float zy, float zz
    ) :
        xx(xx), xy(xy), xz(xz),
        yx(yx), yy(yy), yz(yz),
        zx(zx), zy(zy), zz(zz) {};
    Mat3(float diagonal) :
        xx(diagonal), xy(0.0f), xz(0.0f),
        yx(0.0f), yy(diagonal), yz(0.0f),
        zx(0.0f), zy(0.0f), zz(diagonal) {};
    Mat3(const Vec3& col1, const Vec3& col2, const Vec3& col3) :
        xx(col1.x), xy(col2.x), xz(col3.x),
        yx(col1.y), yy(col2.y), yz(col3.y),
        zx(col1.z), zy(col2.z), zz(col3.z) {};

    Mat3(const Mat3& other) :
        xx(other.xx), xy(other.xy), xz(other.xz),
        yx(other.yx), yy(other.yy), yz(other.yz),
        zx(other.zx), zy(other.zy), zz(other.zz) {};

    Mat3(const Mat2& other) :
        xx(other.xx), xy(other.xy), xz(0.0f),
        yx(other.yx), yy(other.yy), yz(0.0f),
        zx(0.0f), zy(0.0f), zz(0.0f) {};
    Mat3(const Mat4& other);

    Mat3& operator=(const Mat3& other);
    Mat3& operator=(const Mat2& other);
    Mat3& operator=(const Mat4& other);

    bool operator==(const Mat3& other) const;
    bool operator!=(const Mat3& other) const;

    Mat3 operator+(const Mat3& other) const;
    Mat3 operator-(const Mat3& other) const;
    Mat3 operator*(const Mat3& other) const;

    Mat3& operator+=(const Mat3& other);
    Mat3& operator-=(const Mat3& other);
    Mat3& operator*=(const Mat3& other);

    Mat3 operator+(float scalar) const;
    Mat3 operator-(float scalar) const;
    Mat3 operator*(float scalar) const;
    Mat3 operator/(float scalar) const;

    Mat3& operator+=(float scalar);
    Mat3& operator-=(float scalar);
    Mat3& operator*=(float scalar);
    Mat3& operator/=(float scalar);

    Vec3 operator*(const Vec3& vec) const;

    Vec3 operator[](size_t index) const;
};

struct Mat4 {
    float xx = 0.0f, yx = 0.0f, zx = 0.0f, wx = 0.0f;
    float xy = 0.0f, yy = 0.0f, zy = 0.0f, wy = 0.0f;
    float xz = 0.0f, yz = 0.0f, zz = 0.0f, wz = 0.0f;
    float xw = 0.0f, yw = 0.0f, zw = 0.0f, ww = 0.0f;

    Mat4() = default;
    Mat4(
        float xx, float xy, float xz, float xw,
        float yx, float yy, float yz, float yw,
        float zx, float zy, float zz, float zw,
        float wx, float wy, float wz, float ww
    ) :
        xx(xx), xy(xy), xz(xz), xw(xw),
        yx(yx), yy(yy), yz(yz), yw(yw),
        zx(zx), zy(zy), zz(zz), zw(zw),
        wx(wx), wy(wy), wz(wz), ww(ww) {};
    Mat4(float diagonal) :
        xx(diagonal), xy(0.0f), xz(0.0f), xw(0.0f),
        yx(0.0f), yy(diagonal), yz(0.0f), yw(0.0f),
        zx(0.0f), zy(0.0f), zz(diagonal), zw(0.0f),
        wx(0.0f), wy(0.0f), wz(0.0f), ww(diagonal) {};
    Mat4(const Vec4& col1, const Vec4& col2, const Vec4& col3, const Vec4& col4) :
        xx(col1.x), xy(col2.x), xz(col3.x), xw(col4.x),
        yx(col1.y), yy(col2.y), yz(col3.y), yw(col4.y),
        zx(col1.z), zy(col2.z), zz(col3.z), zw(col4.z),
        wx(col1.w), wy(col2.w), wz(col3.w), ww(col4.w) {};

    Mat4(const Mat4& other) :
        xx(other.xx), xy(other.xy), xz(other.xz), xw(other.xw),
        yx(other.yx), yy(other.yy), yz(other.yz), yw(other.yw),
        zx(other.zx), zy(other.zy), zz(other.zz), zw(other.zw),
        wx(other.wx), wy(other.wy), wz(other.wz), ww(other.ww) {};
    Mat4(const Mat2& other) :
        xx(other.xx), xy(other.xy), xz(0.0f), xw(0.0f),
        yx(other.yx), yy(other.yy), yz(0.0f), yw(0.0f),
        zx(0.0f), zy(0.0f), zz(0.0f), zw(0.0f),
        wx(0.0f), wy(0.0f), wz(0.0f), ww(0.0f) {};
    Mat4(const Mat3& other) :
        xx(other.xx), xy(other.xy), xz(other.xz), xw(0.0f),
        yx(other.yx), yy(other.yy), yz(other.yz), yw(0.0f),
        zx(other.zx), zy(other.zy), zz(other.zz), zw(0.0f),
        wx(0.0f), wy(0.0f), wz(0.0f), ww(0.0f) {};

    Mat4& operator=(const Mat4& other);
    Mat4& operator=(const Mat2& other);
    Mat4& operator=(const Mat3& other);

    bool operator==(const Mat4& other) const;
    bool operator!=(const Mat4& other) const;

    Mat4 operator+(const Mat4& other) const;
    Mat4 operator-(const Mat4& other) const;
    Mat4 operator*(const Mat4& other) const;

    Mat4& operator+=(const Mat4& other);
    Mat4& operator-=(const Mat4& other);
    Mat4& operator*=(const Mat4& other);

    Mat4 operator+(float scalar) const;
    Mat4 operator-(float scalar) const;
    Mat4 operator*(float scalar) const;
    Mat4 operator/(float scalar) const;

    Mat4& operator+=(float scalar);
    Mat4& operator-=(float scalar);
    Mat4& operator*=(float scalar);
    Mat4& operator/=(float scalar);

    Vec4 operator*(const Vec4& vec) const;

    Vec4 operator[](size_t index) const;
};

Mat2 transpose(const Mat2& mat);
Mat3 transpose(const Mat3& mat);
Mat4 transpose(const Mat4& mat);

float determinant(const Mat2& mat);
float determinant(const Mat3& mat);
float determinant(const Mat4& mat);

Mat2 inverse(const Mat2& mat);
Mat3 inverse(const Mat3& mat);
Mat4 inverse(const Mat4& mat);

Mat3 scale(const Mat3& mat, const Vec2& scale);
Mat4 scale(const Mat4& mat, const Vec3& scale);

Mat3 translate(const Mat3& mat, const Vec2& translation);
Mat4 translate(const Mat4& mat, const Vec3& translation);

Mat3 rotate(const Mat3& mat, float angle, const Vec3& axis);
Mat4 rotate(const Mat4& mat, float angle, const Vec3& axis);

Mat3 scale(const Mat3& mat, float scale);
Mat4 scale(const Mat4& mat, float scale);

Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up);
enum class DepthRange {
    ZERO_TO_ONE,
    MINUS_ONE_TO_ONE
};
Mat4 perspective(
    float fov,
    float aspect,
    float near,
    float far,
    DepthRange depthRange = DepthRange::MINUS_ONE_TO_ONE
);
Mat4 orthographic(float left, float right, float bottom, float top, float near, float far);

} // namespace Math
