#include "zp_cpp/math.hpp"

#include <format>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace zp::math;

// ================================================================================================================
// ================================================================================================================
// to_vec*: Converts GLM vector, quaternion, and matrix types into their zp::math counterparts for internal use.
// ================================================================================================================
// ================================================================================================================
// clang-format off
vec2 to_vec2(const glm::vec2& v) { return vec2{v.x, v.y}; }
vec3 to_vec3(const glm::vec3& v) { return vec3{v.x, v.y, v.z}; }
vec4 to_vec4(const glm::vec4& v) { return vec4{v.x, v.y, v.z, v.w}; }
ivec2 to_ivec2(const glm::ivec2& v) { return ivec2{v.x, v.y}; }
ivec3 to_ivec3(const glm::ivec3& v) { return ivec3{v.x, v.y, v.z}; }
ivec4 to_ivec4(const glm::ivec4& v) { return ivec4{v.x, v.y, v.z, v.w}; }
quat to_quat(const glm::quat& q) { return quat{q.x, q.y, q.z, q.w}; }
mat3 to_mat3(const glm::mat3& m)
{
    mat3 out;
    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            out.f[row][col] = m[col][row];
        }
    }
    return out;
}

mat4 to_mat4(const glm::mat4& m)
{
    mat4 out;
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            out.f[row][col] = m[col][row];
        }
    }
    return out;
}
// clang-format on

// ================================================================================================================
// ================================================================================================================
// from_vec*: Adapts zp::math vector, quaternion, and matrix types back into GLM for interop with helper helpers.
// ================================================================================================================
// ================================================================================================================
// clang-format off
glm::vec2 from_vec2(const vec2& v) { return glm::vec2{v.x, v.y}; }
glm::vec3 from_vec3(const vec3& v) { return glm::vec3{v.x, v.y, v.z}; }
glm::vec4 from_vec4(const vec4& v) { return glm::vec4{v.x, v.y, v.z, v.w}; }
glm::ivec2 from_ivec2(const ivec2& v) { return glm::ivec2{v.x, v.y}; }
glm::ivec3 from_ivec3(const ivec3& v) { return glm::ivec3{v.x, v.y, v.z}; }
glm::ivec4 from_ivec4(const ivec4& v) { return glm::ivec4{v.x, v.y, v.z, v.w}; }
glm::quat from_quat(const quat& q) { return glm::quat{q.w, q.x, q.y, q.z}; }
// clang-format on

// ================================================================================================================
// ================================================================================================================
// from_mat3: Builds a GLM mat3 from a zp::math::mat3 while preserving column-major layout semantics.
// ================================================================================================================
// ================================================================================================================
glm::mat3 from_mat3(const mat3& m)
{
    glm::mat3 out(0.0f);
    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            out[col][row] = m.f[row][col];
        }
    }
    return out;
}

// ================================================================================================================
// ================================================================================================================
// from_mat4: Converts a zp::math::mat4 into the equivalent GLM mat4 by reassembling column vectors.
// ================================================================================================================
// ================================================================================================================
glm::mat4 from_mat4(const mat4& m)
{
    glm::mat4 out(0.0f);
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            out[col][row] = m.f[row][col];
        }
    }
    return out;
}

// ================================================================================================================
// ================================================================================================================
// Vector utilities: Runtime helpers declared in math.hpp.
// ================================================================================================================
// ================================================================================================================
float zp::math::length(vec2 v) noexcept
{
    return std::sqrt(length2(v));
}

zp::math::vec2 zp::math::normalize(vec2 v) noexcept
{
    const float l = length(v);
    return l > 0.f ? v / l : vec2{0.f};
}

float zp::math::distance(vec2 a, vec2 b) noexcept
{
    return length(a - b);
}

float zp::math::length(vec3 v) noexcept
{
    return std::sqrt(length2(v));
}

zp::math::vec3 zp::math::normalize(vec3 v) noexcept
{
    const float l = length(v);
    return l > 0.f ? v / l : vec3{0.f};
}

float zp::math::distance(vec3 a, vec3 b) noexcept
{
    return length(a - b);
}

float zp::math::length(vec4 v) noexcept
{
    return std::sqrt(length2(v));
}

zp::math::vec4 zp::math::normalize(vec4 v) noexcept
{
    const float l = length(v);
    return l > 0.f ? v / l : vec4{0.f};
}

float zp::math::distance(vec4 a, vec4 b) noexcept
{
    return length(a - b);
}

// ================================================================================================================
// ================================================================================================================
// mat3: Runtime operations declared in math.hpp.
// ================================================================================================================
// ================================================================================================================
zp::math::mat3::mat3(const vec3& r0, const vec3& r1, const vec3& r2)
    : f{
          {{{r0.x, r0.y, r0.z}}, {{r1.x, r1.y, r1.z}}, {{r2.x, r2.y, r2.z}}}
}
{
}

zp::math::mat3 zp::math::mat3::operator*(const mat3& rhs) const
{
    mat3 res;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            float sum = 0.0f;
            for (int k = 0; k < 3; ++k)
            {
                sum += f[i][k] * rhs.f[k][j];
            }
            res.f[i][j] = sum;
        }
    }
    return res;
}

zp::math::vec3 zp::math::mat3::operator*(const vec3& v) const
{
    return {
        f[0][0] * v.x + f[0][1] * v.y + f[0][2] * v.z,
        f[1][0] * v.x + f[1][1] * v.y + f[1][2] * v.z,
        f[2][0] * v.x + f[2][1] * v.y + f[2][2] * v.z,
    };
}

zp::math::mat3 zp::math::mat3::operator+(const mat3& rhs) const noexcept
{
    mat3 res;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            res.f[i][j] = f[i][j] + rhs.f[i][j];
        }
    }
    return res;
}

zp::math::mat3 zp::math::mat3::operator-(const mat3& rhs) const noexcept
{
    mat3 res;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            res.f[i][j] = f[i][j] - rhs.f[i][j];
        }
    }
    return res;
}

zp::math::mat3 zp::math::mat3::operator*(float s) const noexcept
{
    mat3 res;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            res.f[i][j] = f[i][j] * s;
        }
    }
    return res;
}

zp::math::mat3 zp::math::mat3::operator/(float s) const noexcept
{
    const float inv_s = 1.0f / s;
    mat3 res;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            res.f[i][j] = f[i][j] * inv_s;
        }
    }
    return res;
}

zp::math::mat3& zp::math::mat3::operator+=(const mat3& rhs) noexcept
{
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            f[i][j] += rhs.f[i][j];
        }
    }
    return *this;
}

zp::math::mat3& zp::math::mat3::operator-=(const mat3& rhs) noexcept
{
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            f[i][j] -= rhs.f[i][j];
        }
    }
    return *this;
}

zp::math::mat3& zp::math::mat3::operator*=(const mat3& rhs) noexcept
{
    *this = *this * rhs;
    return *this;
}

zp::math::mat3& zp::math::mat3::operator*=(float s) noexcept
{
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            f[i][j] *= s;
        }
    }
    return *this;
}

zp::math::mat3& zp::math::mat3::operator/=(float s) noexcept
{
    const float inv_s = 1.0f / s;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            f[i][j] *= inv_s;
        }
    }
    return *this;
}

bool zp::math::mat3::operator==(const mat3& o) const noexcept
{
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            if (f[i][j] != o.f[i][j])
            {
                return false;
            }
        }
    }
    return true;
}

bool zp::math::mat3::operator!=(const mat3& o) const noexcept
{
    return !(*this == o);
}

zp::math::vec3& zp::math::mat3::operator[](std::size_t i) noexcept
{
    return *reinterpret_cast<vec3*>(f[i].data());
}

zp::math::vec3 zp::math::mat3::operator[](std::size_t i) const noexcept
{
    return {f[i][0], f[i][1], f[i][2]};
}

zp::math::mat3 zp::math::operator*(float s, const mat3& m) noexcept
{
    return m * s;
}

// ================================================================================================================
// ================================================================================================================
// mat4: Runtime operations declared in math.hpp.
// ================================================================================================================
// ================================================================================================================
zp::math::mat4::mat4(const vec4& r0, const vec4& r1, const vec4& r2, const vec4& r3)
    : f{
          {{r0.x, r0.y, r0.z, r0.w}, {r1.x, r1.y, r1.z, r1.w}, {r2.x, r2.y, r2.z, r2.w}, {r3.x, r3.y, r3.z, r3.w}}
}
{
}

zp::math::mat4 zp::math::mat4::operator*(const mat4& rhs) const
{
    mat4 res;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k)
            {
                sum += f[i][k] * rhs.f[k][j];
            }
            res.f[i][j] = sum;
        }
    }
    return res;
}

zp::math::vec4 zp::math::mat4::operator*(const vec4& v) const
{
    return {
        f[0][0] * v.x + f[0][1] * v.y + f[0][2] * v.z + f[0][3] * v.w,
        f[1][0] * v.x + f[1][1] * v.y + f[1][2] * v.z + f[1][3] * v.w,
        f[2][0] * v.x + f[2][1] * v.y + f[2][2] * v.z + f[2][3] * v.w,
        f[3][0] * v.x + f[3][1] * v.y + f[3][2] * v.z + f[3][3] * v.w,
    };
}

zp::math::mat4 zp::math::mat4::operator+(const mat4& rhs) const noexcept
{
    mat4 res;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            res.f[i][j] = f[i][j] + rhs.f[i][j];
        }
    }
    return res;
}

zp::math::mat4 zp::math::mat4::operator-(const mat4& rhs) const noexcept
{
    mat4 res;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            res.f[i][j] = f[i][j] - rhs.f[i][j];
        }
    }
    return res;
}

zp::math::mat4 zp::math::mat4::operator*(float s) const noexcept
{
    mat4 res;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            res.f[i][j] = f[i][j] * s;
        }
    }
    return res;
}

zp::math::mat4 zp::math::mat4::operator/(float s) const noexcept
{
    const float inv_s = 1.0f / s;
    mat4 res;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            res.f[i][j] = f[i][j] * inv_s;
        }
    }
    return res;
}

zp::math::mat4& zp::math::mat4::operator+=(const mat4& rhs) noexcept
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            f[i][j] += rhs.f[i][j];
        }
    }
    return *this;
}

zp::math::mat4& zp::math::mat4::operator-=(const mat4& rhs) noexcept
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            f[i][j] -= rhs.f[i][j];
        }
    }
    return *this;
}

zp::math::mat4& zp::math::mat4::operator*=(const mat4& rhs) noexcept
{
    *this = *this * rhs;
    return *this;
}

zp::math::mat4& zp::math::mat4::operator*=(float s) noexcept
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            f[i][j] *= s;
        }
    }
    return *this;
}

zp::math::mat4& zp::math::mat4::operator/=(float s) noexcept
{
    const float inv_s = 1.0f / s;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            f[i][j] *= inv_s;
        }
    }
    return *this;
}

bool zp::math::mat4::operator==(const mat4& o) const noexcept
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            if (f[i][j] != o.f[i][j])
            {
                return false;
            }
        }
    }
    return true;
}

bool zp::math::mat4::operator!=(const mat4& o) const noexcept
{
    return !(*this == o);
}

zp::math::vec4& zp::math::mat4::operator[](std::size_t i) noexcept
{
    return *reinterpret_cast<vec4*>(f[i].data());
}

zp::math::vec4 zp::math::mat4::operator[](std::size_t i) const noexcept
{
    return {f[i][0], f[i][1], f[i][2], f[i][3]};
}

zp::math::mat4 zp::math::mat4::transpose() const noexcept
{
    mat4 r;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            r.f[i][j] = f[j][i];
        }
    }
    return r;
}

float zp::math::mat4::determinant() const noexcept
{
    const float a00 = f[0][0], a01 = f[0][1], a02 = f[0][2], a03 = f[0][3];
    const float a10 = f[1][0], a11 = f[1][1], a12 = f[1][2], a13 = f[1][3];
    const float a20 = f[2][0], a21 = f[2][1], a22 = f[2][2], a23 = f[2][3];
    const float a30 = f[3][0], a31 = f[3][1], a32 = f[3][2], a33 = f[3][3];

    const float m00 = a11 * (a22 * a33 - a23 * a32) - a12 * (a21 * a33 - a23 * a31) + a13 * (a21 * a32 - a22 * a31);
    const float m01 = a10 * (a22 * a33 - a23 * a32) - a12 * (a20 * a33 - a23 * a30) + a13 * (a20 * a32 - a22 * a30);
    const float m02 = a10 * (a21 * a33 - a23 * a31) - a11 * (a20 * a33 - a23 * a30) + a13 * (a20 * a31 - a21 * a30);
    const float m03 = a10 * (a21 * a32 - a22 * a31) - a11 * (a20 * a32 - a22 * a30) + a12 * (a20 * a31 - a21 * a30);

    return a00 * m00 - a01 * m01 + a02 * m02 - a03 * m03;
}

zp::math::mat4 zp::math::mat4::inverse() const noexcept
{
    const float a00 = f[0][0], a01 = f[0][1], a02 = f[0][2], a03 = f[0][3];
    const float a10 = f[1][0], a11 = f[1][1], a12 = f[1][2], a13 = f[1][3];
    const float a20 = f[2][0], a21 = f[2][1], a22 = f[2][2], a23 = f[2][3];
    const float a30 = f[3][0], a31 = f[3][1], a32 = f[3][2], a33 = f[3][3];

    const float c00     = (a11 * (a22 * a33 - a23 * a32) - a12 * (a21 * a33 - a23 * a31) + a13 * (a21 * a32 - a22 * a31));
    const float c01     = -(a10 * (a22 * a33 - a23 * a32) - a12 * (a20 * a33 - a23 * a30) + a13 * (a20 * a32 - a22 * a30));
    const float c02     = (a10 * (a21 * a33 - a23 * a31) - a11 * (a20 * a33 - a23 * a30) + a13 * (a20 * a31 - a21 * a30));
    const float c03     = -(a10 * (a21 * a32 - a22 * a31) - a11 * (a20 * a32 - a22 * a30) + a12 * (a20 * a31 - a21 * a30));

    const float c10     = -(a01 * (a22 * a33 - a23 * a32) - a02 * (a21 * a33 - a23 * a31) + a03 * (a21 * a32 - a22 * a31));
    const float c11     = (a00 * (a22 * a33 - a23 * a32) - a02 * (a20 * a33 - a23 * a30) + a03 * (a20 * a32 - a22 * a30));
    const float c12     = -(a00 * (a21 * a33 - a23 * a31) - a01 * (a20 * a33 - a23 * a30) + a03 * (a20 * a31 - a21 * a30));
    const float c13     = (a00 * (a21 * a32 - a22 * a31) - a01 * (a20 * a32 - a22 * a30) + a02 * (a20 * a31 - a21 * a30));

    const float c20     = (a01 * (a12 * a33 - a13 * a32) - a02 * (a11 * a33 - a13 * a31) + a03 * (a11 * a32 - a12 * a31));
    const float c21     = -(a00 * (a12 * a33 - a13 * a32) - a02 * (a10 * a33 - a13 * a30) + a03 * (a10 * a32 - a12 * a30));
    const float c22     = (a00 * (a11 * a33 - a13 * a31) - a01 * (a10 * a33 - a13 * a30) + a03 * (a10 * a31 - a11 * a30));
    const float c23     = -(a00 * (a11 * a32 - a12 * a31) - a01 * (a10 * a32 - a12 * a30) + a02 * (a10 * a31 - a11 * a30));

    const float c30     = -(a01 * (a12 * a23 - a13 * a22) - a02 * (a11 * a23 - a13 * a21) + a03 * (a11 * a22 - a12 * a21));
    const float c31     = (a00 * (a12 * a23 - a13 * a22) - a02 * (a10 * a23 - a13 * a20) + a03 * (a10 * a22 - a12 * a20));
    const float c32     = -(a00 * (a11 * a23 - a13 * a21) - a01 * (a10 * a23 - a13 * a20) + a03 * (a10 * a21 - a11 * a20));
    const float c33     = (a00 * (a11 * a22 - a12 * a21) - a01 * (a10 * a22 - a12 * a20) + a02 * (a10 * a21 - a11 * a20));

    const float det     = a00 * c00 + a01 * c01 + a02 * c02 + a03 * c03;
    const float inv_det = 1.0f / det;

    mat4 r;
    r.f[0][0] = c00 * inv_det;
    r.f[0][1] = c10 * inv_det;
    r.f[0][2] = c20 * inv_det;
    r.f[0][3] = c30 * inv_det;
    r.f[1][0] = c01 * inv_det;
    r.f[1][1] = c11 * inv_det;
    r.f[1][2] = c21 * inv_det;
    r.f[1][3] = c31 * inv_det;
    r.f[2][0] = c02 * inv_det;
    r.f[2][1] = c12 * inv_det;
    r.f[2][2] = c22 * inv_det;
    r.f[2][3] = c32 * inv_det;
    r.f[3][0] = c03 * inv_det;
    r.f[3][1] = c13 * inv_det;
    r.f[3][2] = c23 * inv_det;
    r.f[3][3] = c33 * inv_det;
    return r;
}

zp::math::mat4 zp::math::operator*(float s, const mat4& m) noexcept
{
    return m * s;
}

// ================================================================================================================
// ================================================================================================================
// translate: Builds a row-major transform matrix that translates by the supplied vector.
// ================================================================================================================
// ================================================================================================================
zp::math::mat4 zp::math::translate(vec3 t)
{
    glm::mat4 m(1.0f);
    m = glm::translate(m, from_vec3(t));
    return to_mat4(m);
}

// ================================================================================================================
// ================================================================================================================
// scale: Produces a row-major scale matrix using the provided scale factors.
// ================================================================================================================
// ================================================================================================================
zp::math::mat4 zp::math::scale(vec3 s)
{
    glm::mat4 m(1.0f);
    m = glm::scale(m, from_vec3(s));
    return to_mat4(m);
}

// ================================================================================================================
// ================================================================================================================
// rotate_x: Generates a rotation matrix for a rotation about the X axis in radians.
// ================================================================================================================
// ================================================================================================================
zp::math::mat4 zp::math::rotate_x(float radians)
{
    return to_mat4(glm::rotate(glm::mat4(1.0f), radians, glm::vec3(1.0f, 0.0f, 0.0f)));
}

// ================================================================================================================
// ================================================================================================================
// rotate_y: Generates a rotation matrix for a rotation about the Y axis in radians.
// ================================================================================================================
// ================================================================================================================
zp::math::mat4 zp::math::rotate_y(float radians)
{
    return to_mat4(glm::rotate(glm::mat4(1.0f), radians, glm::vec3(0.0f, 1.0f, 0.0f)));
}

// ================================================================================================================
// ================================================================================================================
// rotate_z: Generates a rotation matrix for a rotation about the Z axis in radians.
// ================================================================================================================
// ================================================================================================================
zp::math::mat4 zp::math::rotate_z(float radians)
{
    return to_mat4(glm::rotate(glm::mat4(1.0f), radians, glm::vec3(0.0f, 0.0f, 1.0f)));
}

// ================================================================================================================
// ================================================================================================================
// rotate_axis_angle: Builds a rotation matrix around an arbitrary axis by the specified angle.
// ================================================================================================================
// ================================================================================================================
zp::math::mat4 zp::math::rotate_axis_angle(vec3 axis, float radians)
{
    return to_mat4(glm::rotate(glm::mat4(1.0f), radians, from_vec3(axis)));
}

// ================================================================================================================
// ================================================================================================================
// look_at: Produces a view matrix matching GLM's right-handed lookAt helper using +X/+Y/+Z axes.
// ================================================================================================================
// ================================================================================================================
zp::math::mat4 zp::math::look_at(vec3 eye, vec3 center, vec3 up)
{
    return to_mat4(glm::lookAt(from_vec3(eye), from_vec3(center), from_vec3(up)));
}

// ================================================================================================================
// ================================================================================================================
// perspective: Generates a perspective projection matrix given vertical FOV, aspect ratio, and clip planes.
// ================================================================================================================
// ================================================================================================================
zp::math::mat4 zp::math::perspective(float fovy_radians, float aspect, float z_near, float z_far)
{
    return to_mat4(glm::perspective(fovy_radians, aspect, z_near, z_far));
}

// ================================================================================================================
// ================================================================================================================
// ortho: Builds a row-major orthographic projection matrix using the specified volume.
// ================================================================================================================
// ================================================================================================================
zp::math::mat4 zp::math::ortho(float left, float right, float bottom, float top, float z_near, float z_far)
{
    return to_mat4(glm::ortho(left, right, bottom, top, z_near, z_far));
}

// ================================================================================================================
// ================================================================================================================
// to_string(vec2): Formats a 2D vector as an "(x, y)" tuple string.
// ================================================================================================================
// ================================================================================================================
std::string zp::math::to_string(const vec2& v)
{
    return std::format("({}, {})", v.x, v.y);
}

// ================================================================================================================
// ================================================================================================================
// to_string(vec3): Formats a 3D vector as an "(x, y, z)" tuple string.
// ================================================================================================================
// ================================================================================================================
std::string zp::math::to_string(const vec3& v)
{
    return std::format("({}, {}, {})", v.x, v.y, v.z);
}

// ================================================================================================================
// ================================================================================================================
// to_string(vec4): Formats a homogeneous vector as an "(x, y, z, w)" tuple string.
// ================================================================================================================
// ================================================================================================================
std::string zp::math::to_string(const vec4& v)
{
    return std::format("({}, {}, {}, {})", v.x, v.y, v.z, v.w);
}

// ================================================================================================================
// ================================================================================================================
// to_string(ivec2): Formats an integer 2D vector as an "(x, y)" tuple string.
// ================================================================================================================
// ================================================================================================================
std::string zp::math::to_string(const ivec2& v)
{
    return std::format("({}, {})", v.x, v.y);
}

// ================================================================================================================
// ================================================================================================================
// to_string(ivec3): Formats an integer 3D vector as an "(x, y, z)" tuple string.
// ================================================================================================================
// ================================================================================================================
std::string zp::math::to_string(const ivec3& v)
{
    return std::format("({}, {}, {})", v.x, v.y, v.z);
}

// ================================================================================================================
// ================================================================================================================
// to_string(ivec4): Formats an integer 4D vector as an "(x, y, z, w)" tuple string.
// ================================================================================================================
// ================================================================================================================
std::string zp::math::to_string(const ivec4& v)
{
    return std::format("({}, {}, {}, {})", v.x, v.y, v.z, v.w);
}

// ================================================================================================================
// ================================================================================================================
// to_string(quat): Formats a quaternion into an "(x, y, z, w)" tuple string.
// ================================================================================================================
// ================================================================================================================
std::string zp::math::to_string(const quat& q)
{
    return std::format("({}, {}, {}, {})", q.x, q.y, q.z, q.w);
}

// ================================================================================================================
// ================================================================================================================
// to_string(mat3): Formats a 3x3 matrix row-wise for debugging display.
// ================================================================================================================
// ================================================================================================================
std::string zp::math::to_string(const mat3& m)
{
    return std::format("(({}, {}, {}) - ({}, {}, {}) - ({}, {}, {}))", m.f[0][0], m.f[0][1], m.f[0][2], m.f[1][0], m.f[1][1], m.f[1][2], m.f[2][0], m.f[2][1], m.f[2][2]);
}

// ================================================================================================================
// ================================================================================================================
// to_string(mat4): Formats a 4x4 matrix row-wise for debugging display.
// ================================================================================================================
// ================================================================================================================
std::string zp::math::to_string(const mat4& m)
{
    return std::format("(({}, {}, {}, {}) - ({}, {}, {}, {}) - ({}, {}, {}, {}) - ({}, {}, {}, {}))", m.f[0][0], m.f[0][1], m.f[0][2], m.f[0][3], m.f[1][0], m.f[1][1], m.f[1][2], m.f[1][3], m.f[2][0], m.f[2][1], m.f[2][2], m.f[2][3], m.f[3][0], m.f[3][1], m.f[3][2], m.f[3][3]);
}

// ================================================================================================================
// ================================================================================================================
// to_string(bb2): Formats a 2D bounding box as min/max tuples.
// ================================================================================================================
// ================================================================================================================
std::string zp::math::to_string(const bb2& bb)
{
    return std::format("(({}, {}) - ({}, {}))", bb.min.x, bb.min.y, bb.max.x, bb.max.y);
}

// ================================================================================================================
// ================================================================================================================
// to_string(bb3): Formats a 3D bounding box as min/max tuples.
// ================================================================================================================
// ================================================================================================================
std::string zp::math::to_string(const bb3& bb)
{
    return std::format("(({}, {}, {}) - ({}, {}, {}))", bb.min.x, bb.min.y, bb.min.z, bb.max.x, bb.max.y, bb.max.z);
}

// ================================================================================================================
// ================================================================================================================
// to_string(bb4): Formats a 4D bounding box as min/max tuples.
// ================================================================================================================
// ================================================================================================================
std::string zp::math::to_string(const bb4& bb)
{
    return std::format("(({}, {}, {}, {}) - ({}, {}, {}, {}))", bb.min.x, bb.min.y, bb.min.z, bb.min.w, bb.max.x, bb.max.y, bb.max.z, bb.max.w);
}

// ================================================================================================================
// ================================================================================================================
// normalize: Returns a unit-length quaternion, defaulting to identity when input is zero length.
// ================================================================================================================
// ================================================================================================================
quat zp::math::normalize(const quat& q)
{
    return to_quat(glm::normalize(from_quat(q)));
}

// ================================================================================================================
// ================================================================================================================
// rotation_to_face: Computes a quaternion that rotates FORWARD to align with the requested direction.
// ================================================================================================================
// ================================================================================================================
quat zp::math::rotation_to_face(const vec3& dir_to_face_zmath)
{
    glm::vec3 target_dir_glm = from_vec3(dir_to_face_zmath);

    // Handle zero direction vector: return identity or current rotation.
    if (glm::length2(target_dir_glm) < glm::epsilon<float>() * glm::epsilon<float>())
    {
        return zp::math::quat{0, 0, 0, 1};
    }
    target_dir_glm                           = glm::normalize(target_dir_glm);

    // --- Define coordinate system conventions ---
    // Your object's inherent "forward" direction when unrotated
    const glm::vec3 ZMATH_LOCAL_FORWARD_GLM  = from_vec3(zp::math::FORWARD); // Should be (0,1,0)
    // Your object's inherent "up" direction when unrotated
    const glm::vec3 ZMATH_LOCAL_UP_GLM       = from_vec3(zp::math::UP); // Should be (0,0,1)

    // The "up" direction in the world, used by lookAt to determine roll
    const glm::vec3 WORLD_UP_FOR_LOOKAT_GLM  = from_vec3(zp::math::UP); // (0,0,1)

    // glm::quatLookAtRH assumes the object's canonical forward is -Z and canonical up is +Y
    const glm::vec3 GLM_CANONICAL_FORWARD_RH = glm::vec3(0.0f, 0.0f, -1.0f);
    const glm::vec3 GLM_CANONICAL_UP_RH      = glm::vec3(0.0f, 1.0f, 0.0f);

    // --- Calculate the correction quaternion ---
    // This quaternion rotates an object from ZMath's local system to GLM's canonical system.
    // It needs to rotate ZMATH_LOCAL_FORWARD_GLM to GLM_CANONICAL_FORWARD_RH
    // and ZMATH_LOCAL_UP_GLM to GLM_CANONICAL_UP_RH.

    // Method 1: Construct from axes (more robust if axes are not simple 90-deg rotations)
    // glm::mat3 zmath_basis_inv = glm::inverse(glm::mat3(
    //    glm::cross(ZMATH_LOCAL_UP_GLM, ZMATH_LOCAL_FORWARD_GLM), // zmath local right
    //    ZMATH_LOCAL_UP_GLM,                                     // zmath local up
    //    -ZMATH_LOCAL_FORWARD_GLM                                // zmath local back (to align with typical matrix basis)
    // ));
    // glm::mat3 glm_basis = glm::mat3(
    //    glm::cross(GLM_CANONICAL_UP_RH, GLM_CANONICAL_FORWARD_RH), // glm canonical right
    //    GLM_CANONICAL_UP_RH,                                      // glm canonical up
    //    -GLM_CANONICAL_FORWARD_RH                                 // glm canonical back
    // );
    // glm::quat correction_q_complex = glm::quat_cast(glm_basis * zmath_basis_inv);

    // Method 2: For your specific case (ZMath Fwd=+Y, Up=+Z to GLM Fwd=-Z, Up=+Y)
    // Rotate ZMath_Forward (0,1,0) to GLM_Canonical_Forward (0,0,-1).
    // This is a -90 degree rotation around World X-axis.
    // (1,0,0) (X-axis)
    glm::quat correction_q                   = glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    // Verify this correction:
    // glm::vec3 corrected_forward = correction_q * ZMATH_LOCAL_FORWARD_GLM; // should be (0,0,-1)
    // glm::vec3 corrected_up = correction_q * ZMATH_LOCAL_UP_GLM;       // should be (0,1,0)
    // If ZMATH_LOCAL_FORWARD_GLM = (0,1,0) and ZMATH_LOCAL_UP_GLM = (0,0,1):
    //   corrected_forward is indeed (0,0,-1)
    //   corrected_up is indeed (0,1,0)
    // So this simple correction_q works for your specific Fwd/Up convention.

    // --- Calculate the LookAt quaternion ---
    // This quaternion rotates an object (that's in GLM's canonical orientation)
    // to face target_dir_glm, using WORLD_UP_FOR_LOOKAT_GLM to orient its roll.
    glm::quat look_at_q;

    // Handle cases where target_dir_glm is collinear with WORLD_UP_FOR_LOOKAT_GLM
    // glm::quatLookAtRH might use a default secondary axis (like world X or Y)
    // which might not be what you want if your world up is Z.
    float dot_target_worldup = glm::dot(target_dir_glm, WORLD_UP_FOR_LOOKAT_GLM);
    if (std::abs(dot_target_worldup) > 1.0f - glm::epsilon<float>())
    {
        // Target is looking straight up or straight down along WORLD_UP_FOR_LOOKAT_GLM.
        // glm::quatLookAtRH will use a fallback 'up' (often world Y {0,1,0}).
        // If WORLD_UP_FOR_LOOKAT_GLM is Z {0,0,1}, and we look along Z,
        // GLM's fallback using Y-up for quatLookAtRH is fine. It will establish a "right" vector.
        look_at_q = glm::quatLookAtRH(target_dir_glm, WORLD_UP_FOR_LOOKAT_GLM);
        // If GLM's default fallback for this case is not desired, you might need to
        // construct the quaternion manually using a chosen "right" vector.
        // For example, if looking straight up/down Z, you might want your local X to align with world X.
        // glm::vec3 right_if_looking_z = glm::vec3(1,0,0);
        // glm::vec3 forward_if_looking_z = target_dir_glm; // (0,0,1) or (0,0,-1)
        // glm::vec3 up_if_looking_z = glm::cross(right_if_looking_z, forward_if_looking_z);
        // look_at_q = glm::quatLookAt(forward_if_looking_z, up_if_looking_z); // This is not a GLM function name
        // look_at_q = glm::conjugate(glm::toQuat(glm::lookAt(glm::vec3(0), target_dir_glm, an_alternative_up)));
    }
    else
    {
        look_at_q = glm::quatLookAtRH(target_dir_glm, WORLD_UP_FOR_LOOKAT_GLM);
    }

    // The final quaternion:
    // correction_q aligns ZMath's default orientation to GLM's canonical orientation.
    // look_at_q then rotates this canonical object to face the target.
    // The combined effect is (look_at_q * correction_q) applied to an object in ZMath's default orientation.
    glm::quat final_q_glm = look_at_q * correction_q;

    return to_quat(final_q_glm);
}

// ================================================================================================================
// ================================================================================================================
// slerp: Performs spherical linear interpolation between two quaternions.
// ================================================================================================================
// ================================================================================================================
quat zp::math::slerp(const quat& q0, const quat& q1, float f)
{
    return to_quat(glm::slerp(from_quat(q0), from_quat(q1), f));
}

// ================================================================================================================
// ================================================================================================================
// proportion: Returns the floating-point ratio of i0 to i1 without defensive guards.
// ================================================================================================================
// ================================================================================================================
float zp::math::proportion(uint64_t i0, uint64_t i1)
{
    return float(i0) / float(i1);
}

// ================================================================================================================
// ================================================================================================================
// get_transform_mat: Composes translation, quaternion rotation, and scale into a row-major matrix.
// ================================================================================================================
// ================================================================================================================
zp::math::mat4 zp::math::get_transform_mat(vec3 translate, quat rot, vec3 sca)
{
    glm::mat4 model_mat = glm::mat4(1.f);
    model_mat           = glm::translate(model_mat, from_vec3(translate));
    model_mat           = model_mat * glm::mat4_cast(from_quat(rot));
    model_mat           = glm::scale(model_mat, from_vec3(sca));
    return to_mat4(model_mat);
}

// ================================================================================================================
// ================================================================================================================
// get_transform_mat: Overload accepting Euler angles that internally converts to a quaternion.
// ================================================================================================================
// ================================================================================================================
zp::math::mat4 zp::math::get_transform_mat(vec3 translate, vec3 euler_rot, vec3 sca)
{
    glm::vec3 _rot = from_vec3(euler_rot);
    glm::quat _q   = glm::quat(_rot);
    return get_transform_mat(translate, to_quat(_q), sca);
}

// ====================================================================================================================
// ====================================================================================================================
// model_to_nrm_mat: Derives the normal matrix as transpose-inverse of the model's linear part.
// ====================================================================================================================
// ====================================================================================================================
mat3 zp::math::model_to_nrm_mat(mat4 model_mat)
{
    return to_mat3(glm::transpose(glm::inverse(from_mat4(model_mat))));
}

// ====================================================================================================================
// ====================================================================================================================
// vec2_to_ivec2: Rounds a vec2 to the nearest integer components.
// ====================================================================================================================
// ====================================================================================================================
ivec2 zp::math::vec2_to_ivec2(vec2 v)
{
    return to_ivec2(glm::ivec2(glm::round(from_vec2(v))));
}

// ================================================================================================================
// ================================================================================================================
// CartesianToPolar: Converts Cartesian coordinates into spherical coordinates (radius, azimuth, polar).
// ================================================================================================================
// ================================================================================================================
zp::math::SphericalCoords zp::math::CartesianToPolar(vec3 _cartesian)
{
    glm::vec3 cartesian = from_vec3(_cartesian);

    float radius        = glm::length(cartesian);
    float azimuth       = glm::atan(cartesian.y, cartesian.x);
    float polar         = glm::acos(cartesian.z / radius);

    return {radius, azimuth, polar};
}

// ================================================================================================================
// ================================================================================================================
// PolarToCartesian: Converts spherical coordinates back into Cartesian vector form.
// ================================================================================================================
// ================================================================================================================
vec3 zp::math::PolarToCartesian(SphericalCoords polar)
{

    float radius          = polar.radius;
    float azimuth         = polar.azimuth;
    float polarAngle      = polar.polar;

    float sin_polar_angle = glm::sin(polarAngle);
    float x               = radius * sin_polar_angle * cos(azimuth);
    float y               = radius * sin_polar_angle * glm::sin(azimuth);
    float z               = radius * cos(polarAngle);

    return vec3(x, y, z);
}

// ================================================================================================================
// ================================================================================================================
// areParallel: Returns true when two vectors are parallel or degenerate (zero length).
// ================================================================================================================
// ================================================================================================================
bool zp::math::areParallel(const vec3& _v0, const vec3& _v1)
{
    glm::vec3 v0 = from_vec3(_v0);
    glm::vec3 v1 = from_vec3(_v1);

    if (glm::length(v0) < 1e-6f || glm::length(v1) < 1e-6f) return true;
    glm::vec3 n0 = glm::normalize(v0);
    glm::vec3 n1 = glm::normalize(v1);
    return glm::length(glm::cross(n0, n1)) < 1e-6f;
}

// ================================================================================================================
// ================================================================================================================
// OrbitCamera::init: Seeds camera state from the provided configuration without validation.
// ================================================================================================================
// ================================================================================================================
void zp::math::OrbitCamera::init(OrbitCameraConfig config)
{
    position     = config.start_pos;
    target_pos   = config.start_target_pos;
    vert_fov_deg = config.start_fov;
    aspect_ratio = config.aspect_ratio;
}

// ================================================================================================================
// ================================================================================================================
// OrbitCamera::get_model_mat: Returns the camera model matrix combining axes and position.
// ================================================================================================================
// ================================================================================================================
mat4 zp::math::OrbitCamera::get_model_mat() const
{
    const vec3 r = get_right_axis();
    const vec3 f = get_forward_axis();
    const vec3 u = get_up_axis();

    return mat4(vec4(r.x, f.x, u.x, position.x), vec4(r.y, f.y, u.y, position.y), vec4(r.z, f.z, u.z, position.z), vec4(0.0f, 0.0f, 0.0f, 1.0f));
}

// ================================================================================================================
// ================================================================================================================
// OrbitCamera::get_near_plane_height: Computes near plane height from current FOV.
// ================================================================================================================
// ================================================================================================================
float zp::math::OrbitCamera::get_near_plane_height(float near_plane_dist) const
{
    return 2.0f * glm::tan(glm::radians(vert_fov_deg) / 2.0f) * near_plane_dist;
}

// ================================================================================================================
// ================================================================================================================
// OrbitCamera::get_near_plane_width: Derives near plane width using aspect ratio.
// ================================================================================================================
// ================================================================================================================
float zp::math::OrbitCamera::get_near_plane_width(float near_plane_height) const
{
    return aspect_ratio * near_plane_height;
}

// ====================================================================================================================
// ====================================================================================================================
// OrbitCamera::get_forward_axis: Normalized direction vector from position to target.
// ====================================================================================================================
// ====================================================================================================================
vec3 zp::math::OrbitCamera::get_forward_axis() const
{
    return normalize(target_pos - position);
}

// ====================================================================================================================
// ====================================================================================================================
// OrbitCamera::get_right_axis: Computes the right axis via forward × global up.
// ====================================================================================================================
// ====================================================================================================================
vec3 zp::math::OrbitCamera::get_right_axis() const
{
    vec3 forward = get_forward_axis();
    return to_vec3(glm::normalize((glm::cross(from_vec3(forward), from_vec3(UP)))));
}

// ====================================================================================================================
// ====================================================================================================================
// OrbitCamera::get_up_axis: Recomputes an orthonormal up axis using right × forward.
// ====================================================================================================================
// ====================================================================================================================
vec3 zp::math::OrbitCamera::get_up_axis() const
{
    vec3 forward = get_forward_axis();
    vec3 right   = get_right_axis();
    return to_vec3(glm::normalize(glm::cross(from_vec3(right), from_vec3(forward))));
}

// ====================================================================================================================
// ====================================================================================================================
// OrbitCamera::get_view_mat: Produces the view matrix via GLM lookAt using camera state.
// ====================================================================================================================
// ====================================================================================================================
mat4 zp::math::OrbitCamera::get_view_mat() const
{
    return to_mat4(glm::lookAt(from_vec3(position), from_vec3(target_pos), from_vec3(UP)));
}

// ====================================================================================================================
// ====================================================================================================================
// OrbitCamera::get_proj_mat: Generates a perspective projection matrix for the configured FOV/aspect.
// ====================================================================================================================
// ====================================================================================================================
mat4 zp::math::OrbitCamera::get_proj_mat(float near_dist, float far_dist) const
{
    return to_mat4(glm::perspective(glm::radians(vert_fov_deg), aspect_ratio, near_dist, far_dist));
}

// ====================================================================================================================
// ====================================================================================================================
// OrbitCamera::screen_point_to_near_world: Unprojects a normalized screen coordinate onto the near plane.
// ====================================================================================================================
// ====================================================================================================================
vec3 zp::math::OrbitCamera::screen_point_to_near_world(vec2 nrm_point, float near_dist, float far_dist) const
{
    // build matrices
    glm::mat4 view       = from_mat4(get_view_mat());
    glm::mat4 proj       = from_mat4(get_proj_mat(near_dist, far_dist));

    // unproject to world
    glm::vec3 near_world = glm::unProject(glm::vec3(nrm_point.x, 1 - nrm_point.y, 0.0f), view, proj, glm::vec4(0, 0, 1, 1));

    return to_vec3(near_world);
}

// ====================================================================================================================
// ====================================================================================================================
// OrbitCamera::screen_point_to_ray_dir: Returns a normalized world ray passing through the screen point.
// ====================================================================================================================
// ====================================================================================================================
vec3 zp::math::OrbitCamera::screen_point_to_ray_dir(vec2 nrm_point, float near_dist, float far_dist) const
{
    // build matrices
    glm::mat4 view       = from_mat4(get_view_mat());
    glm::mat4 proj       = from_mat4(get_proj_mat(near_dist, far_dist));

    // unproject to world
    glm::vec3 near_world = glm::unProject(glm::vec3(nrm_point.x, 1 - nrm_point.y, 0.0f), view, proj, glm::vec4(0, 0, 1, 1));
    glm::vec3 far_world  = glm::unProject(glm::vec3(nrm_point.x, 1 - nrm_point.y, 1.0f), view, proj, glm::vec4(0, 0, 1, 1));

    // ray direction
    return to_vec3(glm::normalize(far_world - near_world));
}

// ================================================================================================================
// ================================================================================================================
// OrbitCamera::rotate: Orbits the camera around the target using spherical coordinate adjustments.
// ================================================================================================================
// ================================================================================================================
void zp::math::OrbitCamera::rotate(vec2 delta, float speed)
{
    auto polar    = CartesianToPolar(position - target_pos);
    polar.azimuth = polar.azimuth - delta.x * speed;
    polar.polar   = polar.polar - delta.y * speed;
    polar.polar   = glm::clamp(polar.polar, 0.1, glm::pi<float>() - 0.1);
    position      = PolarToCartesian(polar) + target_pos;
}

// ================================================================================================================
// ================================================================================================================
// OrbitCamera::pan: Slides target and position using the camera up/right axes.
// ================================================================================================================
// ================================================================================================================
void zp::math::OrbitCamera::pan(vec2 delta, float speed)
{
    const vec3& up     = get_up_axis();
    const vec3& right  = get_right_axis();
    target_pos        += up * delta.y * speed;
    target_pos        -= right * delta.x * speed;
    position          += up * delta.y * speed;
    position          -= right * delta.x * speed;
}

// ================================================================================================================
// ================================================================================================================
// OrbitCamera::zoom: Moves the camera along the view direction clamped to a minimal distance.
// ================================================================================================================
// ================================================================================================================
void zp::math::OrbitCamera::zoom(float delta, float speed)
{
    vec3 dir       = normalize(position - target_pos);
    vec3 move_vec  = -dir * delta * speed;
    vec3 new_pos   = position + move_vec;
    float new_dist = length(new_pos - target_pos);
    if (new_dist < 0.01f && delta > 0)
    {
        return;
    }
    position = new_pos;
}

// ================================================================================================================
// ================================================================================================================
// IntersectionResult constructors and helpers.
// ================================================================================================================
// ================================================================================================================
zp::math::IntersectionResult::IntersectionResult() : hit(false), distance(0.0f) {}

zp::math::IntersectionResult::IntersectionResult(float dist) : hit(true), distance(dist) {}

bool zp::math::IntersectionResult::operator<(const IntersectionResult& other) const
{
    return distance < other.distance;
}

// ================================================================================================================
// ================================================================================================================
// checkRayAABBIntersection: Computes slab-based ray-box intersection returning the nearest hit distance.
// ================================================================================================================
// ================================================================================================================
IntersectionResult zp::math::checkRayAABBIntersection(const vec3& ray_position, const vec3& ray_direction, const vec3 bb_min, const vec3 bb_max)
{
    vec3 inv_direction = {1.0f / ray_direction.x, 1.0f / ray_direction.y, 1.0f / ray_direction.z};

    float t_min_x      = (bb_min.x - ray_position.x) * inv_direction.x;
    float t_max_x      = (bb_max.x - ray_position.x) * inv_direction.x;
    if (t_min_x > t_max_x) std::swap(t_min_x, t_max_x);

    float t_min_y = (bb_min.y - ray_position.y) * inv_direction.y;
    float t_max_y = (bb_max.y - ray_position.y) * inv_direction.y;
    if (t_min_y > t_max_y) std::swap(t_min_y, t_max_y);

    float t_min_z = (bb_min.z - ray_position.z) * inv_direction.z;
    float t_max_z = (bb_max.z - ray_position.z) * inv_direction.z;
    if (t_min_z > t_max_z) std::swap(t_min_z, t_max_z);

    float t_near_overall = std::max({t_min_x, t_min_y, t_min_z});
    float t_far_overall  = std::min({t_max_x, t_max_y, t_max_z});

    if (t_near_overall > t_far_overall || t_far_overall < 0.0f)
    {
        return IntersectionResult();
    }

    return IntersectionResult(t_near_overall);
}

// ================================================================================================================
// ================================================================================================================
// std::hash specialisations for zp::math vector types.
// ================================================================================================================
// ================================================================================================================
size_t std::hash<zp::math::vec2>::operator()(const zp::math::vec2& v) const noexcept
{
    size_t seed  = 0;
    seed        ^= std::hash<float>{}(v.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed        ^= std::hash<float>{}(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}

size_t std::hash<zp::math::vec3>::operator()(const zp::math::vec3& v) const noexcept
{
    size_t seed  = 0;
    seed        ^= std::hash<float>{}(v.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed        ^= std::hash<float>{}(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed        ^= std::hash<float>{}(v.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}

size_t std::hash<zp::math::vec4>::operator()(const zp::math::vec4& v) const noexcept
{
    size_t seed  = 0;
    seed        ^= std::hash<float>{}(v.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed        ^= std::hash<float>{}(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed        ^= std::hash<float>{}(v.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed        ^= std::hash<float>{}(v.w) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}

size_t std::hash<zp::math::ivec2>::operator()(const zp::math::ivec2& v) const noexcept
{
    size_t seed  = 0;
    seed        ^= std::hash<int>{}(v.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed        ^= std::hash<int>{}(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}

size_t std::hash<zp::math::ivec3>::operator()(const zp::math::ivec3& v) const noexcept
{
    size_t seed  = 0;
    seed        ^= std::hash<int>{}(v.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed        ^= std::hash<int>{}(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed        ^= std::hash<int>{}(v.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}

size_t std::hash<zp::math::ivec4>::operator()(const zp::math::ivec4& v) const noexcept
{
    size_t seed  = 0;
    seed        ^= std::hash<int>{}(v.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed        ^= std::hash<int>{}(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed        ^= std::hash<int>{}(v.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed        ^= std::hash<int>{}(v.w) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}
