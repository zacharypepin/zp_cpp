#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <concepts>
#include <functional>
#include <string>
#undef min
#undef max

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
namespace zp::math
{
    struct vec2;
    struct vec3;
    struct vec4;
    struct quat;

    struct vec2
    {
        float x, y;

        // ctors
        constexpr vec2() noexcept = default;
        template <std::convertible_to<float> T> constexpr explicit vec2(T v) noexcept : x(v), y(v) {}
        template <std::convertible_to<float> T1, std::convertible_to<float> T2> constexpr vec2(T1 x_, T2 y_) noexcept : x(x_), y(y_) {}

        // element access
        constexpr float& operator[](size_t i) noexcept
        {
            return (&x)[i];
        }
        constexpr const float& operator[](size_t i) const noexcept
        {
            return (&x)[i];
        }

        // unary
        constexpr vec2 operator-() const noexcept
        {
            return {-x, -y};
        }

        // arithmetic vec–vec
        constexpr vec2 operator+(vec2 o) const noexcept
        {
            return {x + o.x, y + o.y};
        }
        constexpr vec2 operator-(vec2 o) const noexcept
        {
            return {x - o.x, y - o.y};
        }
        constexpr vec2 operator*(vec2 o) const noexcept
        {
            return {x * o.x, y * o.y};
        }
        constexpr vec2 operator/(vec2 o) const noexcept
        {
            return {x / o.x, y / o.y};
        }

        // arithmetic vec–scalar
        constexpr vec2 operator*(float s) const noexcept
        {
            return {x * s, y * s};
        }
        constexpr vec2 operator/(float s) const noexcept
        {
            return {x / s, y / s};
        }

        // compound assigns
        constexpr vec2& operator+=(vec2 o) noexcept
        {
            x += o.x;
            y += o.y;
            return *this;
        }
        constexpr vec2& operator-=(vec2 o) noexcept
        {
            x -= o.x;
            y -= o.y;
            return *this;
        }
        constexpr vec2& operator*=(vec2 o) noexcept
        {
            x *= o.x;
            y *= o.y;
            return *this;
        }
        constexpr vec2& operator/=(vec2 o) noexcept
        {
            x /= o.x;
            y /= o.y;
            return *this;
        }
        constexpr vec2& operator*=(float s) noexcept
        {
            x *= s;
            y *= s;
            return *this;
        }
        constexpr vec2& operator/=(float s) noexcept
        {
            x /= s;
            y /= s;
            return *this;
        }

        // comparisons
        constexpr bool operator==(const vec2& o) const noexcept
        {
            return x == o.x && y == o.y;
        }
        constexpr bool operator!=(const vec2& o) const noexcept
        {
            return !(*this == o);
        }
    };

    // scalar * vec
    constexpr vec2 operator*(float s, vec2 v) noexcept
    {
        return v * s;
    }

    // free functions
    constexpr float dot(vec2 a, vec2 b) noexcept
    {
        return a.x * b.x + a.y * b.y;
    }
    constexpr float cross(vec2 a, vec2 b) noexcept
    {
        return a.x * b.y - a.y * b.x;
    }
    constexpr float length2(vec2 v) noexcept
    {
        return dot(v, v);
    }
    float length(vec2 v) noexcept;
    vec2 normalize(vec2 v) noexcept;
    float distance(vec2 a, vec2 b) noexcept;
    constexpr vec2 lerp(vec2 a, vec2 b, float t) noexcept
    {
        return a + (b - a) * t;
    }
    constexpr vec2 min(vec2 a, vec2 b) noexcept
    {
        return {std::min(a.x, b.x), std::min(a.y, b.y)};
    }
    constexpr vec2 max(vec2 a, vec2 b) noexcept
    {
        return {std::max(a.x, b.x), std::max(a.y, b.y)};
    }
    constexpr vec2 clamp(vec2 v, vec2 lo, vec2 hi) noexcept
    {
        return min(max(v, lo), hi);
    }

    struct vec3
    {
        float x, y, z;

        // ctors
        constexpr vec3() noexcept = default;

        template <std::convertible_to<float> T> constexpr explicit vec3(T v) noexcept : x(v), y(v), z(v) {}

        template <std::convertible_to<float> T1, std::convertible_to<float> T2, std::convertible_to<float> T3> constexpr vec3(T1 x_, T2 y_, T3 z_) noexcept : x(x_), y(y_), z(z_) {}

        constexpr vec3(const struct vec4& v) noexcept;

        // element access
        constexpr float& operator[](size_t i) noexcept
        {
            return (&x)[i];
        }
        constexpr const float& operator[](size_t i) const noexcept
        {
            return (&x)[i];
        }

        // unary
        constexpr vec3 operator-() const noexcept
        {
            return {-x, -y, -z};
        }

        // arithmetic vec–vec
        constexpr vec3 operator+(vec3 o) const noexcept
        {
            return {x + o.x, y + o.y, z + o.z};
        }
        constexpr vec3 operator-(vec3 o) const noexcept
        {
            return {x - o.x, y - o.y, z - o.z};
        }
        constexpr vec3 operator*(vec3 o) const noexcept
        {
            return {x * o.x, y * o.y, z * o.z};
        }
        constexpr vec3 operator/(vec3 o) const noexcept
        {
            return {x / o.x, y / o.y, z / o.z};
        }

        // arithmetic vec–scalar
        constexpr vec3 operator*(float s) const noexcept
        {
            return {x * s, y * s, z * s};
        }
        constexpr vec3 operator/(float s) const noexcept
        {
            return {x / s, y / s, z / s};
        }

        // compound assigns
        constexpr vec3& operator+=(vec3 o) noexcept
        {
            x += o.x;
            y += o.y;
            z += o.z;
            return *this;
        }
        constexpr vec3& operator-=(vec3 o) noexcept
        {
            x -= o.x;
            y -= o.y;
            z -= o.z;
            return *this;
        }
        constexpr vec3& operator*=(vec3 o) noexcept
        {
            x *= o.x;
            y *= o.y;
            z *= o.z;
            return *this;
        }
        constexpr vec3& operator/=(vec3 o) noexcept
        {
            x /= o.x;
            y /= o.y;
            z /= o.z;
            return *this;
        }
        constexpr vec3& operator*=(float s) noexcept
        {
            x *= s;
            y *= s;
            z *= s;
            return *this;
        }
        constexpr vec3& operator/=(float s) noexcept
        {
            x /= s;
            y /= s;
            z /= s;
            return *this;
        }

        // comparisons
        constexpr bool operator==(const vec3& o) const noexcept
        {
            return x == o.x && y == o.y && z == o.z;
        }
        constexpr bool operator!=(const vec3& o) const noexcept
        {
            return !(*this == o);
        }
    };
    constexpr vec3 operator*(float s, vec3 v) noexcept
    {
        return v * s;
    }

    // free functions
    constexpr float dot(vec3 a, vec3 b) noexcept
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
    constexpr vec3 cross(vec3 a, vec3 b) noexcept
    {
        return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
    }
    constexpr float length2(vec3 v) noexcept
    {
        return dot(v, v);
    }
    float length(vec3 v) noexcept;
    vec3 normalize(vec3 v) noexcept;
    float distance(vec3 a, vec3 b) noexcept;
    constexpr vec3 lerp(vec3 a, vec3 b, float t) noexcept
    {
        return a + (b - a) * t;
    }
    constexpr vec3 min(vec3 a, vec3 b) noexcept
    {
        return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
    }
    constexpr vec3 max(vec3 a, vec3 b) noexcept
    {
        return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};
    }
    constexpr vec3 clamp(vec3 v, vec3 lo, vec3 hi) noexcept
    {
        return {std::min(std::max(v.x, lo.x), hi.x), std::min(std::max(v.y, lo.y), hi.y), std::min(std::max(v.z, lo.z), hi.z)};
    }

    struct vec4
    {
        float x, y, z, w;

        // ctors
        constexpr vec4() noexcept = default;
        constexpr explicit vec4(float v) noexcept : x(v), y(v), z(v), w(v) {}
        constexpr vec4(float x_, float y_, float z_, float w_) noexcept : x(x_), y(y_), z(z_), w(w_) {}
        constexpr vec4(const vec3& v, float w_) noexcept : x(v.x), y(v.y), z(v.z), w(w_) {}

        // element access
        constexpr float& operator[](size_t i) noexcept
        {
            return (&x)[i];
        }
        constexpr const float& operator[](size_t i) const noexcept
        {
            return (&x)[i];
        }

        // unary
        constexpr vec4 operator-() const noexcept
        {
            return {-x, -y, -z, -w};
        }

        // arithmetic vec–vec
        constexpr vec4 operator+(vec4 o) const noexcept
        {
            return {x + o.x, y + o.y, z + o.z, w + o.w};
        }
        constexpr vec4 operator-(vec4 o) const noexcept
        {
            return {x - o.x, y - o.y, z - o.z, w - o.w};
        }
        constexpr vec4 operator*(vec4 o) const noexcept
        {
            return {x * o.x, y * o.y, z * o.z, w * o.w};
        }
        constexpr vec4 operator/(vec4 o) const noexcept
        {
            return {x / o.x, y / o.y, z / o.z, w / o.w};
        }

        // arithmetic vec–scalar
        constexpr vec4 operator*(float s) const noexcept
        {
            return {x * s, y * s, z * s, w * s};
        }
        constexpr vec4 operator/(float s) const noexcept
        {
            return {x / s, y / s, z / s, w / s};
        }

        // compound assigns
        constexpr vec4& operator+=(vec4 o) noexcept
        {
            x += o.x;
            y += o.y;
            z += o.z;
            w += o.w;
            return *this;
        }
        constexpr vec4& operator-=(vec4 o) noexcept
        {
            x -= o.x;
            y -= o.y;
            z -= o.z;
            w -= o.w;
            return *this;
        }
        constexpr vec4& operator*=(vec4 o) noexcept
        {
            x *= o.x;
            y *= o.y;
            z *= o.z;
            w *= o.w;
            return *this;
        }
        constexpr vec4& operator/=(vec4 o) noexcept
        {
            x /= o.x;
            y /= o.y;
            z /= o.z;
            w /= o.w;
            return *this;
        }
        constexpr vec4& operator*=(float s) noexcept
        {
            x *= s;
            y *= s;
            z *= s;
            w *= s;
            return *this;
        }
        constexpr vec4& operator/=(float s) noexcept
        {
            x /= s;
            y /= s;
            z /= s;
            w /= s;
            return *this;
        }

        // comparisons
        constexpr bool operator==(const vec4& o) const noexcept
        {
            return x == o.x && y == o.y && z == o.z && w == o.w;
        }
        constexpr bool operator!=(const vec4& o) const noexcept
        {
            return !(*this == o);
        }
    };
    constexpr vec4 operator*(float s, vec4 v) noexcept
    {
        return v * s;
    }

    // free functions
    constexpr float dot(vec4 a, vec4 b) noexcept
    {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }
    constexpr float length2(vec4 v) noexcept
    {
        return dot(v, v);
    }
    float length(vec4 v) noexcept;
    vec4 normalize(vec4 v) noexcept;
    float distance(vec4 a, vec4 b) noexcept;
    constexpr vec4 lerp(vec4 a, vec4 b, float t) noexcept
    {
        return a + (b - a) * t;
    }
    constexpr vec4 min(vec4 a, vec4 b) noexcept
    {
        return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w)};
    }
    constexpr vec4 max(vec4 a, vec4 b) noexcept
    {
        return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w)};
    }
    constexpr vec4 clamp(vec4 v, vec4 lo, vec4 hi) noexcept
    {
        return {std::min(std::max(v.x, lo.x), hi.x), std::min(std::max(v.y, lo.y), hi.y), std::min(std::max(v.z, lo.z), hi.z), std::min(std::max(v.w, lo.w), hi.w)};
    }

    constexpr vec3::vec3(const vec4& v) noexcept : x(v.x), y(v.y), z(v.z) {}

    struct ivec2
    {
        int x, y;

        constexpr ivec2() noexcept = default;
        constexpr explicit ivec2(int v) noexcept : x(v), y(v) {}
        constexpr ivec2(int x_, int y_) noexcept : x(x_), y(y_) {}

        constexpr int& operator[](size_t i) noexcept
        {
            return (&x)[i];
        }
        constexpr const int& operator[](size_t i) const noexcept
        {
            return (&x)[i];
        }

        constexpr ivec2 operator-() const noexcept
        {
            return {-x, -y};
        }

        constexpr ivec2 operator+(ivec2 o) const noexcept
        {
            return {x + o.x, y + o.y};
        }
        constexpr ivec2 operator-(ivec2 o) const noexcept
        {
            return {x - o.x, y - o.y};
        }
        constexpr ivec2 operator*(ivec2 o) const noexcept
        {
            return {x * o.x, y * o.y};
        }
        constexpr ivec2 operator/(ivec2 o) const noexcept
        {
            return {x / o.x, y / o.y};
        }

        constexpr ivec2 operator*(int s) const noexcept
        {
            return {x * s, y * s};
        }
        constexpr ivec2 operator/(int s) const noexcept
        {
            return {x / s, y / s};
        }

        constexpr ivec2& operator+=(ivec2 o) noexcept
        {
            x += o.x;
            y += o.y;
            return *this;
        }
        constexpr ivec2& operator-=(ivec2 o) noexcept
        {
            x -= o.x;
            y -= o.y;
            return *this;
        }
        constexpr ivec2& operator*=(ivec2 o) noexcept
        {
            x *= o.x;
            y *= o.y;
            return *this;
        }
        constexpr ivec2& operator/=(ivec2 o) noexcept
        {
            x /= o.x;
            y /= o.y;
            return *this;
        }
        constexpr ivec2& operator*=(int s) noexcept
        {
            x *= s;
            y *= s;
            return *this;
        }
        constexpr ivec2& operator/=(int s) noexcept
        {
            x /= s;
            y /= s;
            return *this;
        }

        constexpr bool operator==(const ivec2& o) const noexcept
        {
            return x == o.x && y == o.y;
        }
        constexpr bool operator!=(const ivec2& o) const noexcept
        {
            return !(*this == o);
        }
    };
    constexpr ivec2 operator*(int s, ivec2 v) noexcept
    {
        return v * s;
    }

    struct ivec3
    {
        int x, y, z;

        constexpr ivec3() noexcept = default;
        constexpr explicit ivec3(int v) noexcept : x(v), y(v), z(v) {}
        constexpr ivec3(int x_, int y_, int z_) noexcept : x(x_), y(y_), z(z_) {}

        constexpr int& operator[](size_t i) noexcept
        {
            return (&x)[i];
        }
        constexpr const int& operator[](size_t i) const noexcept
        {
            return (&x)[i];
        }

        constexpr ivec3 operator-() const noexcept
        {
            return {-x, -y, -z};
        }

        constexpr ivec3 operator+(ivec3 o) const noexcept
        {
            return {x + o.x, y + o.y, z + o.z};
        }
        constexpr ivec3 operator-(ivec3 o) const noexcept
        {
            return {x - o.x, y - o.y, z - o.z};
        }
        constexpr ivec3 operator*(ivec3 o) const noexcept
        {
            return {x * o.x, y * o.y, z * o.z};
        }
        constexpr ivec3 operator/(ivec3 o) const noexcept
        {
            return {x / o.x, y / o.y, z / o.z};
        }

        constexpr ivec3 operator*(int s) const noexcept
        {
            return {x * s, y * s, z * s};
        }
        constexpr ivec3 operator/(int s) const noexcept
        {
            return {x / s, y / s, z / s};
        }

        constexpr ivec3& operator+=(ivec3 o) noexcept
        {
            x += o.x;
            y += o.y;
            z += o.z;
            return *this;
        }
        constexpr ivec3& operator-=(ivec3 o) noexcept
        {
            x -= o.x;
            y -= o.y;
            z -= o.z;
            return *this;
        }
        constexpr ivec3& operator*=(ivec3 o) noexcept
        {
            x *= o.x;
            y *= o.y;
            z *= o.z;
            return *this;
        }
        constexpr ivec3& operator/=(ivec3 o) noexcept
        {
            x /= o.x;
            y /= o.y;
            z /= o.z;
            return *this;
        }
        constexpr ivec3& operator*=(int s) noexcept
        {
            x *= s;
            y *= s;
            z *= s;
            return *this;
        }
        constexpr ivec3& operator/=(int s) noexcept
        {
            x /= s;
            y /= s;
            z /= s;
            return *this;
        }

        constexpr bool operator==(const ivec3& o) const noexcept
        {
            return x == o.x && y == o.y && z == o.z;
        }
        constexpr bool operator!=(const ivec3& o) const noexcept
        {
            return !(*this == o);
        }
    };
    constexpr ivec3 operator*(int s, ivec3 v) noexcept
    {
        return v * s;
    }

    struct ivec4
    {
        int x, y, z, w;

        constexpr ivec4() noexcept = default;
        constexpr explicit ivec4(int v) noexcept : x(v), y(v), z(v), w(v) {}
        constexpr ivec4(int x_, int y_, int z_, int w_) noexcept : x(x_), y(y_), z(z_), w(w_) {}

        constexpr int& operator[](size_t i) noexcept
        {
            return (&x)[i];
        }
        constexpr const int& operator[](size_t i) const noexcept
        {
            return (&x)[i];
        }

        constexpr ivec4 operator-() const noexcept
        {
            return {-x, -y, -z, -w};
        }

        constexpr ivec4 operator+(ivec4 o) const noexcept
        {
            return {x + o.x, y + o.y, z + o.z, w + o.w};
        }
        constexpr ivec4 operator-(ivec4 o) const noexcept
        {
            return {x - o.x, y - o.y, z - o.z, w - o.w};
        }
        constexpr ivec4 operator*(ivec4 o) const noexcept
        {
            return {x * o.x, y * o.y, z * o.z, w * o.w};
        }
        constexpr ivec4 operator/(ivec4 o) const noexcept
        {
            return {x / o.x, y / o.y, z / o.z, w / o.w};
        }

        constexpr ivec4 operator*(int s) const noexcept
        {
            return {x * s, y * s, z * s, w * s};
        }
        constexpr ivec4 operator/(int s) const noexcept
        {
            return {x / s, y / s, z / s, w / s};
        }

        constexpr ivec4& operator+=(ivec4 o) noexcept
        {
            x += o.x;
            y += o.y;
            z += o.z;
            w += o.w;
            return *this;
        }
        constexpr ivec4& operator-=(ivec4 o) noexcept
        {
            x -= o.x;
            y -= o.y;
            z -= o.z;
            w -= o.w;
            return *this;
        }
        constexpr ivec4& operator*=(ivec4 o) noexcept
        {
            x *= o.x;
            y *= o.y;
            z *= o.z;
            w *= o.w;
            return *this;
        }
        constexpr ivec4& operator/=(ivec4 o) noexcept
        {
            x /= o.x;
            y /= o.y;
            z /= o.z;
            w /= o.w;
            return *this;
        }
        constexpr ivec4& operator*=(int s) noexcept
        {
            x *= s;
            y *= s;
            z *= s;
            w *= s;
            return *this;
        }
        constexpr ivec4& operator/=(int s) noexcept
        {
            x /= s;
            y /= s;
            z /= s;
            w /= s;
            return *this;
        }

        constexpr bool operator==(const ivec4& o) const noexcept
        {
            return x == o.x && y == o.y && z == o.z && w == o.w;
        }
        constexpr bool operator!=(const ivec4& o) const noexcept
        {
            return !(*this == o);
        }
    };
    constexpr ivec4 operator*(int s, ivec4 v) noexcept
    {
        return v * s;
    }

    struct quat
    {
        float x, y, z, w;

        constexpr quat() noexcept = default;
        constexpr explicit quat(float val) noexcept : x(val), y(val), z(val), w(val) {}
        constexpr quat(float x_, float y_, float z_, float w_) noexcept : x(x_), y(y_), z(z_), w(w_) {}

        // Example: from axis-angle
        // quat(vec3 axis, float angle_rad) {
        //     axis = normalize(axis);
        //     float s = std::sin(angle_rad * 0.5f);
        //     x = axis.x * s;
        //     y = axis.y * s;
        //     z = axis.z * s;
        //     w = std::cos(angle_rad * 0.5f);
        // }

        constexpr float& operator[](size_t i) noexcept
        {
            return (&x)[i];
        }
        constexpr const float& operator[](size_t i) const noexcept
        {
            return (&x)[i];
        }

        constexpr quat operator-() const noexcept
        {
            return {-x, -y, -z, -w};
        }

        constexpr quat operator+(quat o) const noexcept
        {
            return {x + o.x, y + o.y, z + o.z, w + o.w};
        }
        constexpr quat operator-(quat o) const noexcept
        {
            return {x - o.x, y - o.y, z - o.z, w - o.w};
        }

        constexpr quat operator*(quat o) const noexcept
        {
            return {w * o.x + x * o.w + y * o.z - z * o.y, w * o.y - x * o.z + y * o.w + z * o.x, w * o.z + x * o.y - y * o.x + z * o.w, w * o.w - x * o.x - y * o.y - z * o.z};
        }

        constexpr quat operator*(float s) const noexcept
        {
            return {x * s, y * s, z * s, w * s};
        }
        constexpr quat operator/(float s) const noexcept
        {
            return {x / s, y / s, z / s, w / s};
        }

        constexpr quat& operator+=(quat o) noexcept
        {
            x += o.x;
            y += o.y;
            z += o.z;
            w += o.w;
            return *this;
        }
        constexpr quat& operator-=(quat o) noexcept
        {
            x -= o.x;
            y -= o.y;
            z -= o.z;
            w -= o.w;
            return *this;
        }
        constexpr quat& operator*=(quat o) noexcept
        {
            *this = *this * o;
            return *this;
        }
        constexpr quat& operator*=(float s) noexcept
        {
            x *= s;
            y *= s;
            z *= s;
            w *= s;
            return *this;
        }
        constexpr quat& operator/=(float s) noexcept
        {
            x /= s;
            y /= s;
            z /= s;
            w /= s;
            return *this;
        }

        constexpr bool operator==(const quat& o) const noexcept
        {
            return x == o.x && y == o.y && z == o.z && w == o.w;
        }
        constexpr bool operator!=(const quat& o) const noexcept
        {
            return !(*this == o);
        }
    };
    constexpr quat operator*(float s, quat q) noexcept
    {
        return q * s;
    }

    struct mat3
    {
        std::array<std::array<float, 3>, 3> f{
            {{{1.f, 0.f, 0.f}}, {{0.f, 1.f, 0.f}}, {{0.f, 0.f, 1.f}}}
        };

        mat3() = default;

        mat3(const vec3& r0, const vec3& r1, const vec3& r2);

        mat3 operator*(const mat3& rhs) const;
        vec3 operator*(const vec3& v) const;
        mat3 operator+(const mat3& rhs) const noexcept;
        mat3 operator-(const mat3& rhs) const noexcept;
        mat3 operator*(float s) const noexcept;
        mat3 operator/(float s) const noexcept;
        mat3& operator+=(const mat3& rhs) noexcept;
        mat3& operator-=(const mat3& rhs) noexcept;
        mat3& operator*=(const mat3& rhs) noexcept;
        mat3& operator*=(float s) noexcept;
        mat3& operator/=(float s) noexcept;
        bool operator==(const mat3& o) const noexcept;
        bool operator!=(const mat3& o) const noexcept;

        vec3& operator[](std::size_t i) noexcept;
        vec3 operator[](std::size_t i) const noexcept;
    };
    mat3 operator*(float s, const mat3& m) noexcept;

    struct mat4
    {
        std::array<std::array<float, 4>, 4> f = {
            {{1.f, 0.f, 0.f, 0.f}, {0.f, 1.f, 0.f, 0.f}, {0.f, 0.f, 1.f, 0.f}, {0.f, 0.f, 0.f, 1.f}}
        };

        mat4() = default;

        mat4(const vec4& r0, const vec4& r1, const vec4& r2, const vec4& r3);

        mat4 operator*(const mat4& rhs) const;
        vec4 operator*(const vec4& v) const;
        mat4 operator+(const mat4& rhs) const noexcept;
        mat4 operator-(const mat4& rhs) const noexcept;
        mat4 operator*(float s) const noexcept;
        mat4 operator/(float s) const noexcept;
        mat4& operator+=(const mat4& rhs) noexcept;
        mat4& operator-=(const mat4& rhs) noexcept;
        mat4& operator*=(const mat4& rhs) noexcept;
        mat4& operator*=(float s) noexcept;
        mat4& operator/=(float s) noexcept;
        bool operator==(const mat4& o) const noexcept;
        bool operator!=(const mat4& o) const noexcept;

        vec4& operator[](std::size_t i) noexcept;
        vec4 operator[](std::size_t i) const noexcept;

        // matrix helpers
        mat4 transpose() const noexcept;
        float determinant() const noexcept;
        mat4 inverse() const noexcept;
    };
    mat4 operator*(float s, const mat4& m) noexcept;

    // Common 4x4 builders (row-major)
    mat4 translate(vec3 t);
    mat4 scale(vec3 s);
    mat4 rotate_x(float radians);
    mat4 rotate_y(float radians);
    mat4 rotate_z(float radians);
    mat4 rotate_axis_angle(vec3 axis, float radians);
    mat4 look_at(vec3 eye, vec3 center, vec3 up);
    mat4 perspective(float fovy_radians, float aspect, float z_near, float z_far);
    mat4 ortho(float left, float right, float bottom, float top, float z_near, float z_far);

    struct bb2
    {
        vec2 min, max;
    };

    struct bb3
    {
        vec3 min, max;
    };

    struct bb4
    {
        vec4 min, max;
    };

    struct rect
    {
        vec2 xy;
        vec2 wh;
    };

    // Global axis basis uses a right-handed system: +X right, +Y forward, +Z up.
    constexpr vec3 RIGHT   = {1, 0, 0};
    constexpr vec3 FORWARD = {0, 1, 0};
    constexpr vec3 UP      = {0, 0, 1};

    std::string to_string(const vec2& v);
    std::string to_string(const vec3& v);
    std::string to_string(const vec4& v);
    std::string to_string(const ivec2& v);
    std::string to_string(const ivec3& v);
    std::string to_string(const ivec4& v);
    std::string to_string(const quat& q);
    std::string to_string(const mat3& m);
    std::string to_string(const mat4& m);
    std::string to_string(const bb2& bb);
    std::string to_string(const bb3& bb);
    std::string to_string(const bb4& bb);

    quat normalize(const quat& q);

    quat rotation_to_face(const vec3& dir_to_face);

    quat slerp(const quat& q0, const quat& q1, float f);

    float proportion(uint64_t i0, uint64_t i1);

    mat4 get_transform_mat(vec3 translate, quat rot, vec3 sca);

    mat4 get_transform_mat(vec3 translate, vec3 euler_rot, vec3 sca);

    mat3 model_to_nrm_mat(mat4 model_mat);

    ivec2 vec2_to_ivec2(vec2 vec);

    struct SphericalCoords
    {
        double radius, azimuth, polar;
    };

    SphericalCoords CartesianToPolar(vec3 cartesian);

    vec3 PolarToCartesian(SphericalCoords polar);

    bool areParallel(const vec3& v1, const vec3& v2);

    struct OrbitCameraConfig
    {
        vec3 start_pos;
        vec3 start_target_pos;
        float start_fov;
        float aspect_ratio;
    };

    class OrbitCamera
    {
        public:
        vec3 position;
        vec3 target_pos;
        float vert_fov_deg;
        float aspect_ratio;

        void init(OrbitCameraConfig config);
        mat4 get_model_mat() const;
        float get_near_plane_height(float near_plane_dist) const;
        float get_near_plane_width(float near_plane_height) const;
        vec3 get_forward_axis() const;
        vec3 get_right_axis() const;
        vec3 get_up_axis() const;
        mat4 get_view_mat() const;
        mat4 get_proj_mat(float near_dist, float far_dist) const;
        vec3 screen_point_to_near_world(vec2 nrm_point, float near_dist, float far_dist) const;
        vec3 screen_point_to_ray_dir(vec2 nrm_point, float near_dist, float far_dist) const;
        void rotate(vec2 delta, float speed);
        void pan(vec2 delta, float speed);
        void zoom(float delta, float speed);
    };

    struct IntersectionResult
    {
        bool hit;
        float distance;

        IntersectionResult();
        IntersectionResult(float dist);

        bool operator<(const IntersectionResult& other) const;
    };

    IntersectionResult checkRayAABBIntersection(const vec3& ray_position, const vec3& ray_direction, const vec3 bb_min, const vec3 bb_max);

}

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
namespace zp::math::splines
{
    std::vector<vec2> sample_bezier_tris(vec2 screen_size, std::array<vec2, 4> cp, int segments, float width_pixels);
}

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
namespace std
{
    template <> struct hash<zp::math::vec2>
    {
        size_t operator()(const zp::math::vec2& v) const noexcept;
    };

    template <> struct hash<zp::math::vec3>
    {
        size_t operator()(const zp::math::vec3& v) const noexcept;
    };

    template <> struct hash<zp::math::vec4>
    {
        size_t operator()(const zp::math::vec4& v) const noexcept;
    };

    template <> struct hash<zp::math::ivec2>
    {
        size_t operator()(const zp::math::ivec2& v) const noexcept;
    };

    template <> struct hash<zp::math::ivec3>
    {
        size_t operator()(const zp::math::ivec3& v) const noexcept;
    };

    template <> struct hash<zp::math::ivec4>
    {
        size_t operator()(const zp::math::ivec4& v) const noexcept;
    };
}
